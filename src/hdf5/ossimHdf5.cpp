//---
//
// License: MIT
//
// Description: OSSIM HDF5 utility class.
//
//---
// $Id

#include <ossim/hdf5/ossimHdf5.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimEndian.h>
#include <string>

using namespace H5;
using namespace std;

ossimHdf5::ossimHdf5()
:  m_h5File (0)
{  }

ossimHdf5::~ossimHdf5()
{
   close();
}

bool ossimHdf5::open(const ossimFilename& fullname)
{
   // Check for empty filename.
   if (fullname.empty())
      return false;

   m_filename = fullname;
   if (m_h5File && !close())
      return false;

   // H5 lib throws exceptions:
   bool success = false;
   try
   {
      // Turn off the auto-printing when failure occurs so that we can handle the errors:
      H5::Exception::dontPrint();
      if ( H5File::isHdf5( m_filename.string() ) )
      {
         m_h5File = new H5File(m_filename.string(), H5F_ACC_RDONLY);
         success  = true;
      }
   }
   catch( const H5::Exception& e )
   {
      success = false;
      ossimNotify(ossimNotifyLevel_WARN) <<e.getDetailMsg() << std::endl;
   }
   catch( ... )
   {
      ossimNotify(ossimNotifyLevel_WARN)<< "ossimH5Info::open WARNING Caught unhandled exception "
         "for file <"<< fullname <<">"<< std::endl;
      success = false;
   }

   if (!success)
   {
      delete m_h5File;
      m_h5File = 0;
   }

   return success;
}

bool ossimHdf5::close()
{
   bool success = true;
   if (m_h5File)
   {
      try
      {
         m_h5File->close();
         delete m_h5File;
         m_h5File = 0;
      }
      catch( const H5::Exception& e )
      {
         ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
         success = false;
      }
   }
   return success;
}


bool ossimHdf5::getRoot(Group& root) const
{
   if (!m_h5File)
      return false;

   bool success = true;
   try
   {
      root = m_h5File->openGroup("/");
   }
   catch( const H5::Exception& e )
   {
      ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
      success = false;
   }
   return success;

}

bool ossimHdf5::getChildGroups(H5::Group group, vector<Group>& groupList,
                               bool recursive)
{
   bool success = true;

   int numObjs = 0;

   try
   {
      numObjs = group.getNumObjs();
   }
   catch( const H5::Exception& e )
   {
      e.getDetailMsg();
      success = false;
   }
   for (int i=0; (i<numObjs) && success; ++i)
   {
      try
      {
         H5G_obj_t h5type = group.getObjTypeByIdx(i);
         if (h5type == H5G_GROUP)
         {
            string name = group.getObjnameByIdx(i);
            groupList.push_back(group.openGroup(name));
            if (recursive)
               success = getChildGroups(groupList.back(), groupList, true);
         }
      }
      catch( const H5::Exception& e )
      {
         ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
         success = false;
      }
   }
   return success;
}

bool ossimHdf5::getDatasets(H5::Group group, vector<DataSet>& datasetList,
                            bool recursive)
{
   datasetList.clear();
   vector<Group> groupList;
   if (recursive)
   {
      getChildGroups(group, groupList, true);
   }
   groupList.insert(groupList.begin(), group);

   bool success = true;
   try
   {
      vector<Group>::iterator group_iter = groupList.begin();
      while (group_iter != groupList.end())
      {
         int numObjs = group_iter->getNumObjs();
         for (int i=0; i<numObjs; ++i)
         {
            H5G_obj_t h5type = group_iter->getObjTypeByIdx(i);
            if (h5type == H5G_DATASET)
            {
               string name = group_iter->getObjnameByIdx(i);
               datasetList.push_back(group_iter->openDataSet(name));
            }
         }
         ++group_iter;
      }
   }
   catch( const H5::Exception& e )
   {
      ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
      success = false;
   }
   return success;
}

bool ossimHdf5::getNdimDatasets(H5::Group group, vector<DataSet>& datasetList,
                                bool recursive)
{
   datasetList.clear();
   vector<Group> groupList;
   if (recursive)
      getChildGroups(group, groupList, true);
   groupList.insert(groupList.begin(), group);

   bool success = true;
   try
   {
      vector<Group>::iterator group_iter = groupList.begin();
      while (group_iter != groupList.end())
      {
         int numObjs = group_iter->getNumObjs();
         for (int i=0; i<numObjs; ++i)
         {
            H5G_obj_t h5type = group_iter->getObjTypeByIdx(i);
            if (h5type == H5G_DATASET)
            {
               string name = group_iter->getObjnameByIdx(i);
               DataSet dataset (group_iter->openDataSet(name));
               DataSpace dspace (dataset.getSpace());
               if (dspace.getSimpleExtentNdims() > 1)
                  datasetList.push_back(dataset);
            }
         }
         ++group_iter;
      }
   }
   catch( const H5::Exception& e )
   {
      ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
      success = false;
   }
   return success;
}

bool ossimHdf5::getAttributes(const H5Object& obj, vector<Attribute>& attrList)
{
   attrList.clear();

   // Find the object:
   bool success = true;
   try
   {
      int numAttr = obj.getNumAttrs();
      for (int i=0; i<numAttr; ++i)
      {
         attrList.push_back(obj.openAttribute(i));
      }
   }
   catch( const H5::Exception& e )
   {
      ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
      success = false;
   }

   return success;
}


H5::Group* ossimHdf5::findGroupByName(const std::string& name, const H5::Group* parent, bool recursive)const
{
   if (name.empty())
      return NULL;

   H5::Group baseGroup;
   if (parent == NULL)
   {
      if (!getRoot(baseGroup))
         return NULL;
   }
   else
   {
      baseGroup = *parent;
   }

   H5::Group* named_group = 0;
   vector<Group> groupList;
   if (!getChildGroups(baseGroup, groupList, recursive))
      return NULL;

   try
   {
      std::vector<Group>::iterator group = groupList.begin();
      while (group != groupList.end())
      {
         // bool found;
         ossimString dsName = group->getObjName();
         if (dsName.contains(name))
         {
            named_group = new Group(*group);
            break;
         }
         ++group;
      }
   }
   catch( const H5::Exception& e )
   {
      ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
   }

   return named_group;
}

H5::DataSet* ossimHdf5::findDatasetByName(const std::string& name, const H5::Group* group,
                                          bool recursive)const
{
   if (name.empty())
      return NULL;

   H5::Group baseGroup;
   if (group == NULL)
   {
      if (!getRoot(baseGroup))
         return NULL;
   }
   else
   {
      baseGroup = *group;
   }
   H5::DataSet* named_dataset = 0;
   vector<DataSet> datasetList;
   if (!getDatasets(baseGroup, datasetList, recursive))
      return NULL;
   try
   {
      std::vector<H5::DataSet>::iterator dataset = datasetList.begin();
      while (dataset != datasetList.end())
      {
         // bool found;
         ossimString dsName = dataset->getObjName();
         if (dsName.contains(name))
         {
            named_dataset = new H5::DataSet(*dataset);
            break;
         }
         ++dataset;
      }
   }
   catch( const H5::Exception& e )
   {
      ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
   }
   return named_dataset;
}

ossimByteOrder ossimHdf5::getByteOrder( const H5::AbstractDs* obj )
{
   ossimByteOrder byteOrder = ossim::byteOrder();
   if ( obj )
   {
      try
      {

         // Get the class of the datatype that is used by the dataset.
         H5T_class_t typeClass = obj->getTypeClass();

         H5T_order_t order = H5T_ORDER_NONE;

         if ( typeClass == H5T_INTEGER )
         {
            H5::IntType intype = obj->getIntType();
            order = intype.getOrder();
         }
         else if ( typeClass == H5T_FLOAT )
         {
            H5::FloatType floatType = obj->getFloatType();
            order = floatType.getOrder();
         }

         if ( order == H5T_ORDER_LE )
         {
            byteOrder = OSSIM_LITTLE_ENDIAN;
         }
         else if ( order == H5T_ORDER_BE )
         {
            byteOrder = OSSIM_BIG_ENDIAN;
         }
      }
      catch( const H5::Exception& e )
      {
         ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
      }
   }
   return byteOrder;
}

ossimByteOrder ossimHdf5::getByteOrder( const H5::AtomType& obj )
{
   ossimByteOrder byteOrder = ossim::byteOrder();
   H5T_order_t order = obj.getOrder();
   if ( order == H5T_ORDER_LE )
   {
      byteOrder = OSSIM_LITTLE_ENDIAN;
   }
   else if ( order == H5T_ORDER_BE )
   {
      byteOrder = OSSIM_BIG_ENDIAN;
   }

   return byteOrder;
}


std::string ossimHdf5::getDatatypeClassType( ossim_int32 type )
{
   H5T_class_t classType = (H5T_class_t)type;

   std::string result;
   switch ( classType )
   {
   case H5T_INTEGER:
      result = "H5T_INTEGER";
      break;
   case H5T_FLOAT:
      result = "H5T_FLOAT";
      break;
   case H5T_TIME:
      result = "H5T_TIME";
      break;
   case H5T_STRING:
      result = "H5T_STRING";
      break;
   case H5T_BITFIELD:
      result = "H5T_BITFIELD";
      break;
   case H5T_OPAQUE:
      result = "H5T_OPAQUE";
      break;
   case H5T_COMPOUND:
      result = "H5T_COMPOUND";
      break;
   case H5T_REFERENCE:
      result = "H5T_REFERENCE";
      break;
   case H5T_ENUM:
      result = "H5T_ENUM";
      break;
   case H5T_VLEN:
      result = "H5T_VLEN";
      break;
   case H5T_ARRAY:
      result = "H5T_ARRAY";
      break;
   case H5T_NO_CLASS:
   default:
      result = "H5T_NO_CLASS";
      break;
   }
   return result;
}


void ossimHdf5::getExtents( const H5::DataSet& dataset, std::vector<ossim_uint32>& extents )
{
   extents.clear();

   try
   {
      // Get dataspace of the dataset.
      H5::DataSpace dataspace = dataset.getSpace();

      // Number of dimensions:
      int ndims = dataspace.getSimpleExtentNdims();
      if ( ndims )
      {
         //hsize_t dims_out[ndims];
         std::vector<hsize_t> dims_out(ndims);
         dataspace.getSimpleExtentDims( &dims_out.front(), 0 );
         for ( ossim_int32 i = 0; i < ndims; ++i )
         {
            extents.push_back(static_cast<ossim_uint32>(dims_out[i]));
         }
      }

      dataspace.close();
   }
   catch( const H5::Exception& e )
   {
      ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
   }
}


ossimScalarType ossimHdf5::getScalarType( const H5::DataSet& dataset )
{
   ossimScalarType scalar = OSSIM_SCALAR_UNKNOWN;

   try
   {

      H5::DataType datatype =  dataset.getDataType();
      ossim_int32 typeClass = datatype.getClass();
      if ( ( typeClass != H5T_INTEGER ) && ( typeClass != H5T_FLOAT ) )
         return scalar;

      size_t size = 0;
      hid_t mem_type_id = H5Dget_type( datatype.getId() );
      if( mem_type_id > -1 )
      {
         hid_t native_type = H5Tget_native_type(mem_type_id, H5T_DIR_DEFAULT);

         if( H5Tequal(H5T_NATIVE_CHAR, native_type) )
            scalar = OSSIM_SINT8;
         else if ( H5Tequal( H5T_NATIVE_UCHAR, native_type) )
            scalar = OSSIM_UINT8;
         else if( H5Tequal( H5T_NATIVE_SHORT, native_type) )
            scalar = OSSIM_SINT16;
         else if(H5Tequal(H5T_NATIVE_USHORT, native_type))
            scalar = OSSIM_UINT16;
         else if(H5Tequal( H5T_NATIVE_INT, native_type))
            scalar = OSSIM_SINT32;
         else if(H5Tequal( H5T_NATIVE_UINT, native_type ) )
            scalar = OSSIM_UINT32;
         else if(H5Tequal( H5T_NATIVE_LONG, native_type))
            scalar = OSSIM_SINT32;
         else if(H5Tequal( H5T_NATIVE_ULONG, native_type))
            scalar = OSSIM_UINT32;
         else if(H5Tequal( H5T_NATIVE_LLONG, native_type))
            scalar = OSSIM_SINT64;
         else if(H5Tequal( H5T_NATIVE_ULLONG, native_type))
            scalar = OSSIM_UINT64;
         else if(H5Tequal( H5T_NATIVE_FLOAT, native_type))
            scalar = OSSIM_FLOAT32;
         else if(H5Tequal( H5T_NATIVE_DOUBLE, native_type))
            scalar = OSSIM_FLOAT64;
      }
      else if ( typeClass == H5T_INTEGER )
      {
         H5::IntType intType (dataset);
         bool isSigned = intType.getSign() == H5T_SGN_NONE ? false : true;
         size = intType.getSize();
         switch (size)
         {
         case 1:
            if (isSigned)
               scalar = OSSIM_SINT8;
            else
               scalar = OSSIM_UINT8;
            break;
         case 2:
            if (isSigned)
               scalar = OSSIM_SINT16;
            else
               scalar = OSSIM_UINT16;
            break;
         case 4:
            if (isSigned)
               scalar = OSSIM_SINT32;
            else
               scalar = OSSIM_UINT32;
            break;
         case 8:
            if (isSigned)
               scalar = OSSIM_SINT64;
            else
               scalar = OSSIM_UINT64;
            break;
         default:
            break;
         }
      }
      else // float
      {
         size = datatype.getSize();
         switch (size)
         {
         case 4:
            scalar = OSSIM_FLOAT32;
            break;
         case 8:
            scalar = OSSIM_FLOAT64;
            break;
         default:
            break;
         }
      }
   }
   catch( const H5::Exception& e )
   {
      ossimNotify(ossimNotifyLevel_WARN)<<e.getDetailMsg();
   }

   return scalar;
}

bool ossimHdf5::floatTypeToString(std::string& result,
                                      const H5::FloatType& dataType,
                                      const char* dataPtr)
{
   char* buf = const_cast<char*>(dataPtr);
   ossim_uint32 dataSize = dataType.getSize();
   ossimByteOrder order = getByteOrder(dataType);
   ossimEndian endian;
   bool swapOrder = (order!=ossim::byteOrder());
   bool returnValue = true;
   switch(dataSize)
   {
      case 4:
      {
         ossim_float32* float_value=0;
         float_value = reinterpret_cast<ossim_float32*>(buf);
         if (swapOrder)
            endian.swap(*float_value);
         result = ossimString::toString(*float_value).string();
         break;
      }
      case 8:
      {
         ossim_float64* float_value=0;
         float_value = reinterpret_cast<ossim_float64*>(buf);
         if (swapOrder)
            endian.swap(*float_value);
         result = ossimString::toString(*float_value).string();
         break;
      }
      default:
      {
         returnValue = false;
         break;
      }
   }
   return returnValue;
}

bool ossimHdf5::intTypeToString(std::string& result,
                                    const H5::IntType& dataType,
                                    const char* dataPtr)
{
   char* buf = const_cast<char*>(dataPtr);
   ossim_uint32 signType = H5Tget_sign(dataType.getId());
   ossim_uint32 dataSize = dataType.getSize();
   ossimByteOrder order = getByteOrder(dataType);
   ossimEndian endian;
   bool swapOrder = (order!=ossim::byteOrder());
   bool returnValue = true;
   switch(dataSize)
   {
      case 1: // one byte integer
      {
         switch(signType)
         {
            case H5T_SGN_NONE:
            {
              ossim_uint8* intValue=0;
              intValue = reinterpret_cast<ossim_uint8*>(buf);
              result = ossimString::toString(*intValue).string();

               break;
            }
            case H5T_SGN_2:
            {
              ossim_int8* intValue=0;
              intValue = reinterpret_cast<ossim_int8*>(buf);
              result = ossimString::toString(*intValue).string();

              break;
            }
            default:
            {
               returnValue = false;
               break;
            }
         }
         break;
      }
      case 2:  // 2 byte integer
      {
         switch(signType)
         {
            case H5T_SGN_NONE: // unsigned
            {
              ossim_uint16* intValue=0;
              intValue = reinterpret_cast<ossim_uint16*>(buf);
              if (swapOrder)
                 endian.swap(*intValue);
              result = ossimString::toString(*intValue).string();

               break;
            }
            case H5T_SGN_2: // Signed
            {
              ossim_int16* intValue=0;
              intValue = reinterpret_cast<ossim_int16*>(buf);
              if (swapOrder)
                 endian.swap(*intValue);
              result = ossimString::toString(*intValue).string();
              break;
            }
            default:
            {
               returnValue = false;
               break;
            }

         }
         break;
      }
      case 4: // 4 byte integer
      {
         switch(signType)
         {
            case H5T_SGN_NONE:
            {
              ossim_uint32* intValue=0;
              intValue = reinterpret_cast<ossim_uint32*>(buf);
              if (swapOrder)
                 endian.swap(*intValue);
              result = ossimString::toString(*intValue).string();

               break;
            }
            case H5T_SGN_2:
            {
              ossim_int32* intValue=0;
              intValue = reinterpret_cast<ossim_int32*>(buf);
              if (swapOrder)
                 endian.swap(*intValue);
              result = ossimString::toString(*intValue).string();
              break;
            }
            default:
            {
               returnValue = false;
               break;
            }
         }
         break;
      }
      case 8: // 8 byte integer
      {
         switch(signType)
         {
            case H5T_SGN_NONE:
            {
              ossim_uint64* intValue=0;
              intValue = reinterpret_cast<ossim_uint64*>(buf);
              if (swapOrder)
                 endian.swap(*intValue);
              result = ossimString::toString(*intValue).string();

               break;
            }
            case H5T_SGN_2:
            {
              ossim_int64* intValue=0;
              intValue = reinterpret_cast<ossim_int64*>(buf);
              if (swapOrder)
                 endian.swap(*intValue);
              result = ossimString::toString(*intValue).string();
              break;
            }
            default:
            {
               returnValue = false;
               break;
            }
         }
         break;
      }
      default:
      {
         returnValue = false;
      }
   }

   return returnValue;
}

bool ossimHdf5::stringTypeToString(std::string& result,
                                    const H5::StrType& dataType,
                                    const char* dataPtr)
{
   bool returnValue = false;
   const char* startPtr = dataPtr;
   const char* endPtr   = dataPtr;
   const char* maxPtr   = dataPtr + dataType.getSize();
   if(dataPtr)
   {
      while((endPtr != maxPtr)&&(*endPtr!='\0')) ++endPtr;
      result = std::string(startPtr, endPtr);
      bool returnValue = true;
   }

   return returnValue;
}
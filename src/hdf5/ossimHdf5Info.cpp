//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: HDF5 Info class.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>

//---
// This includes everything!  Note the H5 includes are order dependent; hence,
// the mongo include.
//---
#include <H5Cpp.h>
#include <ossim/hdf5/ossimHdf5Info.h>
#include <ossim/hdf5/ossimHdf5.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace H5;

ossimHdf5Info::ossimHdf5Info()
   : ossimInfoBase()
{
}

ossimHdf5Info::~ossimHdf5Info()
{
   m_hdf5 = 0;
}

bool ossimHdf5Info::open(const ossimFilename& file)
{
   m_hdf5 = new ossimHdf5;
   if (!m_hdf5->open(file))
   {
      m_hdf5 = 0;
      return false;
   }
   return true;
}

// Top level print from root
ostream& ossimHdf5Info::print(ostream& out) const
{
   static const char MODULE[] = "ossimH5Info::open";
   
   if (!m_hdf5.valid())
   {
      out<<"ossimHdf5Info: No HDF5 file has been opened! Nothing to print."<<endl;
      return out;
   }

   try
   {
      Group root;
      if (!m_hdf5->getRoot(root))
         return out;
      print(out, root, "");
   }
   catch (H5::Exception& h5x)
   {
      h5x.getDetailMsg();
   }

   return out;
}

// GROUP LIST
ostream& ossimHdf5Info::printSubGroups(std::ostream& out, const H5::Group& group,
                                        const ossimString& lm) const
{
   vector<Group> groups;
   if (!m_hdf5->getChildGroups(group, groups))
      return out;
   if (!groups.empty())
   {
      for (int i=0; i<groups.size(); ++i)
         print(out, groups[i], lm);
   }
}

// ATTRIBUTE LIST
ostream& ossimHdf5Info::printAttributes(std::ostream& out, const H5::H5Object& obj,
                                        const ossimString& lm) const
{
   vector<Attribute> attributes;
   if (!m_hdf5->getAttributes(obj, attributes))
      return out;

   if (!attributes.empty())
   {
      for (int i=0; i<attributes.size(); ++i)
         print(out, attributes[i], lm);
   }
   return out;
}

// DATASET LIST
ostream& ossimHdf5Info::printDatasets(std::ostream& out, const H5::Group& group,
                                        const ossimString& lm) const
{
   vector<DataSet> datasets;
   if (!m_hdf5->getDatasets(group, datasets))
      return out;

   if (!datasets.empty())
   {
      for (int i=0; i<datasets.size(); ++i)
         print(out, datasets[i], lm);
   }
}

// GROUP
ostream& ossimHdf5Info::print(ostream& out, const H5::Group& group, const ossimString& lm) const
{
   out<<lm<<"GROUP: "<<group.getObjName()<<endl;

   // Set indent for children:
   ossimString lm2 (lm + "  ");

   // List attributes:
   printAttributes(out, group, lm2);

   // List Datasets:
   printDatasets(out, group, lm2);

   // List Child Groups:
   printSubGroups(out, group, lm2);

   return out;
}

// DATASET
ostream& ossimHdf5Info::print(ostream& out, const H5::DataSet& dataset, const ossimString& lm) const
{
   out<<lm<<"DATASET: "<<dataset.getObjName()<<endl;

   // Dump its components:
   int set_size = dataset.getSpace().getSimpleExtentNpoints();
   out<<lm<<"  number of elements: "<<set_size<<endl;
   ossimString lm2 (lm + "  ");
   print(out, dataset.getDataType(), lm2);
   print(out, dataset.getSpace(), lm2);

   // Dump dataset values for small datasets:
   H5T_class_t class_type = dataset.getDataType().getClass();
   if ((set_size < 11) && (class_type == H5T_STRING))
   {
      string values;
      dataset.read(values, dataset.getDataType());
      out<<lm<<"  values: "<<values<<endl;
   }

   return out;
}

// DATATYPE
ostream& ossimHdf5Info::print(ostream& out, const H5::DataType& datatype, const ossimString& lm) const
{
   H5T_class_t class_type = datatype.getClass();
   size_t size = datatype.getSize();
   bool isAtomic = false;
   switch (class_type)
   {
   case H5T_INTEGER:
      isAtomic = true;
      out<<lm<<"DATATYPE: integer, "<<size<<" bytes ";
      break;
   case H5T_FLOAT:
      isAtomic = true;
      out<<lm<<"DATATYPE: float, "<<size<<" bytes ";
      break;
   case H5T_TIME:
      out<<lm<<"DATATYPE: date/time, "<<size<<" bytes "<<endl;
      break;
   case H5T_STRING:
      out<<lm<<"DATATYPE: string, "<<size<<" bytes ";
      break;
   case H5T_BITFIELD:
      out<<lm<<"DATATYPE: bit-field, "<<size<<" bytes "<<endl;
      break;
   case H5T_OPAQUE:
      out<<lm<<"DATATYPE: opaque, "<<size<<" bytes "<<endl;
      break;
   case H5T_COMPOUND:
      out<<lm<<"DATATYPE: compound, "<<size<<" bytes "<<endl;
      break;
   case H5T_REFERENCE:
      out<<lm<<"DATATYPE: reference, "<<size<<" bytes "<<endl;
      break;
   case H5T_ENUM:
      out<<lm<<"DATATYPE: enumeration, "<<size<<" bytes "<<endl;
      break;
   case H5T_VLEN:
      out<<lm<<"DATATYPE: variable-length, "<<size<<" bytes "<<endl;
      break;
   case H5T_ARRAY:
      out<<lm<<"DATATYPE: array, "<<size<<" bytes "<<endl;
      break;
   default:
      out<<lm<<"DATATYPE: unknown, "<<size<<" bytes "<<endl;
   }

   if (isAtomic)
   {
      H5T_order_t order = ((AtomType&) datatype).getOrder();
      switch (order)
      {
      case H5T_ORDER_LE:
         out<<"(Little Endian)"<<endl;
         break;
      case H5T_ORDER_BE:
         out<<"(Big Endian)"<<endl;
         break;
      default:
         out<<endl;
         break;
      }
   }

   return out;
}

// DATASPACE
ostream& ossimHdf5Info::print(ostream& out, const H5::DataSpace& dataspace, const ossimString& lm) const
{
   int rank = 0;
   hsize_t* dim_sizes = 0;

   H5S_class_t classT = dataspace.getSimpleExtentType();
   switch (classT)
   {
   case H5S_SCALAR:
      out<<lm<<"DATASPACE: (scalar)"<<endl;
      break;
   case H5S_SIMPLE:
      rank = dataspace.getSimpleExtentNdims();
      dim_sizes = new hsize_t[rank];
      dataspace.getSimpleExtentDims(dim_sizes);
      out<<lm<<"DATASPACE: simple, rank: "<<rank<<"  size: ";
      for (int i=0; i<rank; i++)
      {
         int dim_size = dim_sizes[i];
         if (i > 0)
            out<<" x ";
         out<<dim_size;
      }
      out<<endl;
      delete dim_sizes;
      break;
   case H5S_NULL:
      out<<lm<<"DATASPACE: (NULL)"<<endl;
      break;
   default:
      out<<lm<<"DATASPACE: (Unknown Type)"<<endl;
   }

   return out;
}

// ATTRIBUTE
ostream& ossimHdf5Info::print(ostream& out, const H5::Attribute& attr, const ossimString& lm) const
{
   out<<lm<<"ATTRIBUTE: "<<attr.getName();

   H5T_class_t class_type = attr.getDataType().getClass();
   if (class_type == H5T_STRING)
   {
      string value;
      attr.read(attr.getDataType(), value);
      out <<" = "<<value<<endl;
   }
   else
   {
      out <<" (value not a string) "<<endl;
      ossimString lm2 (lm + "  ");
      print(out, attr.getDataType(), lm2);
      print(out, attr.getSpace(), lm2);
   }

   return out;
}


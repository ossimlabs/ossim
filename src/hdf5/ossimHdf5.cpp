//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: OSSIM HDF5 utility class.
//
//----------------------------------------------------------------------------
// $Id

#include <ossim/hdf5/ossimHdf5.h>
#include <ossim/base/ossimNotify.h>
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
      if ( H5File::isHdf5(m_filename.chars()) )
         m_h5File = new H5File(m_filename.chars(), H5F_ACC_RDONLY);
      success = true;
   }
   catch( const H5::Exception& e )
   {
      e.getDetailMsg();
   }
   catch( ... )
   {
      ossimNotify(ossimNotifyLevel_WARN)<< "ossimH5Info::open WARNING Caught unhandled exception "
            "for file <"<< fullname <<">"<< endl;
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
         e.getDetailMsg();
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
      e.getDetailMsg();
      success = false;
   }
   return success;

}

bool ossimHdf5::getChildGroups(H5::Group group, vector<Group>& groupList,
                               bool recursive) const
{
   // TODO *** NOTE: This is failing when recursive = true ***

   if (!m_h5File)
      return false;

   bool success = true;
   int numObjs = group.getNumObjs();
   for (int i=0; (i<numObjs) && success; ++i)
   {
      try
      {
         H5G_obj_t h5type = group.getObjTypeByIdx(i);
         if (h5type == H5G_GROUP)
         {
            string name = group.getObjnameByIdx(i);
            cout<<"name = <"<<name<<">"<<endl; //TODO: REMOVE
            groupList.push_back(group.openGroup(name));

            if (recursive)
               success = getChildGroups(groupList.back(), groupList, true);
         }
      }
      catch( const H5::Exception& e )
      {
         e.getDetailMsg();
         success = false;
      }
   }
   return success;
}

bool ossimHdf5::getDatasets(H5::Group group, vector<DataSet>& datasetList,
                            bool recursive) const
{
   datasetList.clear();
   if (!m_h5File)
      return false;

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
               datasetList.push_back(group_iter->openDataSet(name));
            }
         }
         ++group_iter;
      }
   }
   catch( const H5::Exception& e )
   {
      e.getDetailMsg();
      success = false;
   }
   return success;
}

bool ossimHdf5::getNdimDatasets(H5::Group group, vector<DataSet>& datasetList,
                                bool recursive) const
{
   datasetList.clear();
   if (!m_h5File)
      return false;

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
      e.getDetailMsg();
      success = false;
   }
   return success;
}

bool ossimHdf5::getAttributes(const H5Object* obj, vector<Attribute>& attrList) const
{
   attrList.clear();
   if (!m_h5File || !obj)
      return false;

   // Find the object:
   bool success = true;
   try
   {
      int numAttr = obj->getNumAttrs();
      for (int i=0; i<numAttr; ++i)
      {
         attrList.push_back(obj->openAttribute(i));
      }
   }
   catch( const H5::Exception& e )
   {
      e.getDetailMsg();
      success = false;
   }

   return success;
}


H5::DataSet* ossimHdf5::findDatasetByName(const char* name, const H5::Group* group,
                                          bool recursive)
{
   if (!m_h5File || !name)
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
   getDatasets(baseGroup, datasetList, recursive);

   std::vector<H5::DataSet>::iterator dataset = datasetList.begin();
   while (dataset != datasetList.end())
   {
      bool found;
      ossimString dsName = dataset->getObjName();
      if (dsName.contains(name))
      {
         named_dataset = new H5::DataSet(*dataset);
         break;
      }
      ++dataset;
   }
   return named_dataset;
}


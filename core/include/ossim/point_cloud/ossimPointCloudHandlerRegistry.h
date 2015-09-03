//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id$

#ifndef ossimPointCloudHandlerRegistry_HEADER
#define ossimPointCloudHandlerRegistry_HEADER 1

#include <ossim/base/ossimObjectFactory.h>
#include <ossim/base/ossimFactoryListInterface.h>
#include <ossim/base/ossimFilename.h>

#include <ossim/point_cloud/ossimPointCloudHandlerFactory.h>
#include <ossim/point_cloud/ossimPointCloudHandler.h>

class OSSIMDLLEXPORT ossimPointCloudHandlerRegistry :
      public ossimObjectFactory,
      public ossimFactoryListInterface<ossimPointCloudHandlerFactory, ossimPointCloudHandler>
{
public:
   virtual ~ossimPointCloudHandlerRegistry();

   static ossimPointCloudHandlerRegistry* instance();

   virtual ossimPointCloudHandler* open(const ossimFilename& fileName) const;

   virtual ossimPointCloudHandler* open(const ossimKeywordlist& kwl, const char* prefix = 0) const;

   virtual ossimRefPtr<ossimPointCloudHandler> open(std::istream* str,
                                                    std::streamoff restartPosition,
                                                    bool youOwnIt) const;

   /*!
    * Creates an object given a type name.
    */
   virtual ossimObject* createObject(const ossimString& typeName) const;

   /*!
    * Creates and object given a keyword list.
    */
   virtual ossimObject* createObject(const ossimKeywordlist& kwl, const char* prefix = 0) const;

   virtual void getTypeNameList(std::vector<ossimString>& typeList)const;

protected:
   ossimPointCloudHandlerRegistry();
   const ossimPointCloudHandlerRegistry& operator=(const ossimPointCloudHandlerRegistry& rhs);
   static ossimPointCloudHandlerRegistry* m_instance;

TYPE_DATA
};

#endif /* ossimPointCloudHandlerRegistry_HEADER */

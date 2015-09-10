//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id$

#ifndef ossimPointCloudHandlerFactory_HEADER
#define ossimPointCloudHandlerFactory_HEADER

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimObjectFactory.h>
#include <ossim/base/ossimFilename.h>

class ossimPointCloudHandler;

class OSSIMDLLEXPORT ossimPointCloudHandlerFactory: public ossimObjectFactory
{
public:
   virtual ~ossimPointCloudHandlerFactory() {}

   virtual ossimPointCloudHandler* open(const ossimFilename& fileName) const = 0;
   virtual ossimPointCloudHandler* open(const ossimKeywordlist& kwl, const char* prefix = 0) const = 0;

   /*!
    * Creates and object given a keyword list.
    */
   virtual ossimObject* createObject(const ossimKeywordlist& kwl, const char* prefix = 0) const
   {
      return (ossimObject*) open(kwl, prefix);
   }

   virtual ossimObject* createObject(const ossimString& typeName) const = 0;

   virtual void getSupportedExtensions(std::vector<ossimString>& extList) const = 0;

protected:

};

#endif /* ossimPointCloudHandlerFactory_HEADER */

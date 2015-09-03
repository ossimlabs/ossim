/*
 * ossimPointCloudImageHandlerFactory.h
 *
 *  Created on: Dec 8, 2014
 *      Author: okramer
 */

#ifndef ossimPointCloudImageHandlerFactory_HEADER
#define ossimPointCloudImageHandlerFactory_HEADER

#include <ossim/imaging/ossimImageHandlerFactoryBase.h>

class ossimPointCloudImageHandlerFactory: public ossimImageHandlerFactoryBase
{
public:
   ossimPointCloudImageHandlerFactory() {}
   virtual ~ossimPointCloudImageHandlerFactory();

   static ossimPointCloudImageHandlerFactory* instance();

   virtual ossimImageHandler* open(const ossimFilename& fileName,
                                   bool openOverview=true) const;
   virtual ossimImageHandler* open(const ossimKeywordlist& kwl,
                                   const char* prefix=0) const;

   virtual void
   getSupportedExtensions(ossimImageHandlerFactoryBase::UniqueStringList& extensionList) const;

   virtual ossimRefPtr<ossimImageHandler> openOverview(const ossimFilename& file ) const;

   virtual ossimObject* createObject(const ossimString& typeName)const;

   virtual ossimObject* createObject(const ossimKeywordlist& kwl,
                                     const char* prefix=0)const;

   virtual void getTypeNameList(std::vector<ossimString>& typeList)const;
   virtual void getImageHandlersBySuffix(ossimImageHandlerFactoryBase::ImageHandlerList& result, const ossimString& ext)const;
   virtual void getImageHandlersByMimeType(ossimImageHandlerFactoryBase::ImageHandlerList& result, const ossimString& mimeType)const;


private:
   static ossimPointCloudImageHandlerFactory* m_instance;

   TYPE_DATA

};

#endif /* OSSIM_PLUGINS_PDAL_ossimPointCloudImageHandlerFactory_H_ */

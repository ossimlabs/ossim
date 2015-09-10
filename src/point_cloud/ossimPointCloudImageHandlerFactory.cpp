/*
 * ossimPointCloudImageHandlerFactory.cpp
 *
 *  Created on: Dec 8, 2014
 *      Author: okramer
 */

#include <ossim/point_cloud/ossimPointCloudImageHandlerFactory.h>
#include <ossim/point_cloud/ossimPointCloudImageHandler.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimKeywordNames.h>

static const ossimTrace traceDebug("ossimPointCloudImageHandlerFactory:debug");

RTTI_DEF1(ossimPointCloudImageHandlerFactory, "ossimPointCloudImageHandlerFactory", ossimImageHandlerFactoryBase);

ossimPointCloudImageHandlerFactory* ossimPointCloudImageHandlerFactory::m_instance = 0;
ossimPointCloudImageHandlerFactory::~ossimPointCloudImageHandlerFactory()
{
   m_instance = (ossimPointCloudImageHandlerFactory*)0;
}

ossimPointCloudImageHandlerFactory* ossimPointCloudImageHandlerFactory::instance()
{
   if(!m_instance)
   {
      m_instance = new ossimPointCloudImageHandlerFactory;
   }

   return m_instance;
}

ossimImageHandler* ossimPointCloudImageHandlerFactory::open(const ossimFilename& fileName,
                                                  bool openOverview) const
{
   static const char* M = "ossimPointCloudImageHandlerFactory::open(filename) -- ";

   ossimFilename copyFilename = fileName;
   ossimRefPtr<ossimImageHandler> result = 0;

   // Check for empty file.
   copyFilename.trim();
   if (copyFilename.empty() || !copyFilename.exists())
      return 0;

   ossimString ext = copyFilename.ext().downcase();
   if(ext == "gz")
      copyFilename = copyFilename.setExtension("");

   if (traceDebug())
      ossimNotify(ossimNotifyLevel_DEBUG)<<M<< "Trying Point Cloud...\n";

   result = new ossimPointCloudImageHandler();
   result->setFilename(copyFilename);
   if (!result->open())
      result = 0;
   result->setOpenOverviewFlag(openOverview);

   return result.release();
}

ossimImageHandler* ossimPointCloudImageHandlerFactory::open(const ossimKeywordlist& kwl,
                                                  const char* prefix)const
{
   static const char* M = "ossimPointCloudImageHandlerFactory::open(kwl,prefix) -- ";
   if(traceDebug()) ossimNotify(ossimNotifyLevel_DEBUG) << M <<" entered..." << std::endl;

   ossimRefPtr<ossimImageHandler> result = 0;
   if (traceDebug()) ossimNotify(ossimNotifyLevel_DEBUG)<<M<< "Trying ossimPointCloudImageHandler...\n";
   result = new ossimPointCloudImageHandler();
   if (!result->loadState(kwl, prefix))
      result = 0;

   return result.get();
}

ossimRefPtr<ossimImageHandler> ossimPointCloudImageHandlerFactory::openOverview(
   const ossimFilename& /* file */ ) const
{
   ossimRefPtr<ossimImageHandler> result = 0;
   return result;
}

ossimObject* ossimPointCloudImageHandlerFactory::createObject(const ossimString& typeName)const
{
   if(STATIC_TYPE_NAME(ossimPointCloudImageHandler) == typeName)
   {
      return new ossimPointCloudImageHandler();
   }

   return (ossimObject*)0;
}

void ossimPointCloudImageHandlerFactory::getSupportedExtensions(ossimImageHandlerFactoryBase::UniqueStringList& extensionList)const
{
   extensionList.push_back("las");
   extensionList.push_back("gpkg");
}

void ossimPointCloudImageHandlerFactory::getImageHandlersBySuffix(ossimImageHandlerFactoryBase::ImageHandlerList& result, const ossimString& ext)const
{
   if ((ext == "las") || (ext == "gpkg"))
   {
      result.push_back(new ossimPointCloudImageHandler);
      return;
   }
}

void ossimPointCloudImageHandlerFactory::getImageHandlersByMimeType(ossimImageHandlerFactoryBase::ImageHandlerList& result, const ossimString& mimeType)const
{
   ossimString test(mimeType.begin(), mimeType.begin()+6);
   if(test == "image/")
   {
      ossimString mimeTypeTest(mimeType.begin() + 6, mimeType.end());
      getImageHandlersBySuffix(result, mimeTypeTest);
      if(mimeTypeTest == "las")
      {
         result.push_back(new ossimPointCloudImageHandler);
      }
   }
}

ossimObject* ossimPointCloudImageHandlerFactory::createObject(const ossimKeywordlist& kwl,
                                                    const char* prefix)const
{
   ossimObject* result = (ossimObject*)0;
   const char* type = kwl.find(prefix, ossimKeywordNames::TYPE_KW);

   if(type)
   {
      if (ossimString(type).trim() == STATIC_TYPE_NAME(ossimImageHandler))
      {
         const char* lookup = kwl.find(prefix, ossimKeywordNames::FILENAME_KW);

         if (lookup)
         {
            if(traceDebug())
            {
               ossimNotify(ossimNotifyLevel_DEBUG) << "BEBUG: filename " << lookup << std::endl;
            }
            // Call the open that takes a filename...
            result = this->open(kwl, prefix);//ossimFilename(lookup));
         }
      }
      else
      {
         result = createObject(ossimString(type));
         if(result)
         {
            result->loadState(kwl, prefix);
         }
      }
   }
   return result;
}

void ossimPointCloudImageHandlerFactory::getTypeNameList(std::vector<ossimString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(ossimPointCloudImageHandler));
}

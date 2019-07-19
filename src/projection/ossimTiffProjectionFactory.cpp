//*****************************************************************************
// FILE: ossimTiffProjectionFactory.cc
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains implementation of class ossimTiffProjectionFactory
//
//*****************************************************************************
//  $Id: ossimTiffProjectionFactory.cpp 17108 2010-04-15 21:08:06Z dburken $

#include <ossim/projection/ossimTiffProjectionFactory.h>
#include <ossim/support_data/ossimGeoTiff.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/imaging/ossimTiffTileSource.h>
#include <ossim/support_data/TiffHandlerState.h>
#include <fstream>

using namespace std;

ossimTiffProjectionFactory* ossimTiffProjectionFactory::theInstance = 0;

//*****************************************************************************
//  STATIC METHOD: ossimTiffProjectionFactory::instance()
//  
//*****************************************************************************
ossimTiffProjectionFactory* ossimTiffProjectionFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new ossimTiffProjectionFactory;
   }

   return (ossimTiffProjectionFactory*) theInstance;
}

ossimProjection*
ossimTiffProjectionFactory::createProjection(const ossimFilename& filename,
                                              ossim_uint32 entryIdx)const
{
   std::shared_ptr<ossim::TiffHandlerState> state = std::make_shared<ossim::TiffHandlerState>();
   if(state->loadDefaults(filename))
   {
      ossimGeoTiff geotiff;
      ossimKeywordlist kwl;
      if(geotiff.readTags(state, entryIdx))
      {
         if(geotiff.addImageGeometry(kwl))
         {
            return ossimProjectionFactoryRegistry::instance()->createProjection(kwl);
         }
      }
   }
   return 0;
}

//*****************************************************************************
//  METHOD: ossimSensorModelFactory::create(kwl, prefix)
//  
//*****************************************************************************
ossimProjection*
ossimTiffProjectionFactory::createProjection(const ossimKeywordlist& /* keywordList */,
                                             const char* /* prefix */) const
{
   return 0;
}

//*****************************************************************************
//  METHOD: ossimSensorModelFactory::create(projection_name)
//  
//*****************************************************************************
ossimProjection* ossimTiffProjectionFactory::createProjection(const ossimString& /* name */) const
{
   return 0;
}

ossimProjection* ossimTiffProjectionFactory::createProjection(ossimImageHandler* handler)const
{
   ossimTiffTileSource* tiff = dynamic_cast<ossimTiffTileSource*> (handler);
   
   if(tiff)
   {
      ossimGeoTiff geotiff;
      ossimKeywordlist kwl;
      std::shared_ptr<ossim::TiffHandlerState> state = std::dynamic_pointer_cast<ossim::TiffHandlerState>(tiff->getState());
      bool addGeometry = false;
      if(state)
      {
         addGeometry = geotiff.readTags(state, 
                                        tiff->getCurrentEntry());
      }
      else
      {
         addGeometry = geotiff.readTags(tiff->tiffPtr(), 
                                        tiff->getCurrentEntry(), false);
      }
      if(addGeometry&&geotiff.addImageGeometry(kwl))
      {
         return ossimProjectionFactoryRegistry::instance()->createProjection(kwl);
      }
   }
   
   return 0;
}

ossimObject* ossimTiffProjectionFactory::createObject(const ossimString& typeName)const
{
   return (ossimObject*)createProjection(typeName);
}

ossimObject* ossimTiffProjectionFactory::createObject(const ossimKeywordlist& kwl,
                                                     const char* prefix)const
{
   return createProjection(kwl, prefix);
}

void ossimTiffProjectionFactory::getTypeNameList(std::vector<ossimString>& /* typeList */)const
{
}

bool ossimTiffProjectionFactory::isTiff(const ossimFilename& filename)const
{
   std::ifstream in;
   unsigned char header[2];

   in.open( filename.c_str(), ios::in|ios::binary);
   if( !in)
       return false;

   in.read( (char*)header, 2);

   if( (header[0] != 'M' || header[1] != 'M')
       && (header[0] != 'I' || header[1] != 'I') )
   {
      return false;
   }

   return true;
}

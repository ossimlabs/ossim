//----------------------------------------------------------------------------
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//----------------------------------------------------------------------------
// $Id$

#include <ossim/hdf5/ossimHdf5ProjectionFactory.h>
#include <ossim/hdf5/ossimHdf5GridModel.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <ossim/projection/ossimProjection.h>

#define HDF_NAME_PRESENT ((name=="ossimH5GridModel")||(name=="ossimHdfGridModel")||(name=="ossimHdf5GridModel"))

ossimHdf5ProjectionFactory* ossimHdf5ProjectionFactory::instance()
{
   static ossimHdf5ProjectionFactory* factoryInstance =
      new ossimHdf5ProjectionFactory();

   return factoryInstance;
}
   
ossimProjection* ossimHdf5ProjectionFactory::createProjection(const ossimFilename& filename,
                                                              ossim_uint32 /*entryIdx*/)const
{
   static const char MODULE[] = "ossimHdf5ProjectionFactory::createProjection(ossimFilename& filename)";
   ossimRefPtr<ossimProjection> projection = 0;

   // Try external geom file first:
   ossimKeywordlist kwl;
   ossimRefPtr<ossimProjection> model = 0;
   ossimFilename geomFile = filename;
   geomFile = geomFile.setExtension("geom");
   
   if (!kwl.addFile(geomFile.c_str()))
      return 0;

   ossimString name = kwl.find(ossimKeywordNames::TYPE_KW);
   if( ! HDF_NAME_PRESENT )
      return 0;

   // Try external OSSIM grid file:
   ossimFilename coarseGridFile (geomFile.fileNoExtension()+"_ocg");
   if(coarseGridFile.exists())
   {
      kwl.add("grid_file_name", coarseGridFile.c_str(), true);
      projection = new ossimHdf5GridModel();
      if ( projection->loadState( kwl ) == false )
      {
         projection = 0;
      }
   }

   // Must release or pointer will self destruct when it goes out of scope.
   return projection.release();
}

ossimProjection* ossimHdf5ProjectionFactory::createProjection(const ossimString& name) const
{
   if ( HDF_NAME_PRESENT )
      return new ossimHdf5GridModel();

   return 0;
}

ossimProjection* ossimHdf5ProjectionFactory::createProjection(const ossimKeywordlist& kwl,
                                                              const char* prefix) const
{
   ossimRefPtr<ossimProjection> result = 0;
   ossimString name = kwl.find(prefix, ossimKeywordNames::TYPE_KW);
   if ( HDF_NAME_PRESENT )
   {
      result = new ossimHdf5GridModel();
      if ( !result->loadState(kwl, prefix) )
      {
         result = 0;
      }
   }

   return result.release();
}

ossimObject* ossimHdf5ProjectionFactory::createObject(
   const ossimString& typeName)const
{
   return createProjection(typeName);
}

ossimObject* ossimHdf5ProjectionFactory::createObject(
   const ossimKeywordlist& kwl, const char* prefix)const
{
   return createProjection(kwl, prefix);
}


void ossimHdf5ProjectionFactory::getTypeNameList(std::vector<ossimString>& typeList)const
{
   typeList.push_back(ossimString("ossimHdf5GridModel"));
}

ossimHdf5ProjectionFactory::ossimHdf5ProjectionFactory()
{
}

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

ossimHdf5ProjectionFactory* ossimHdf5ProjectionFactory::instance()
{
   static ossimHdf5ProjectionFactory* factoryInstance =
      new ossimHdf5ProjectionFactory();

   return factoryInstance;
}
   
ossimProjection* ossimHdf5ProjectionFactory::createProjection(const ossimFilename& filename,
                                                              ossim_uint32 entryIdx)const
{
   static const char MODULE[] = "ossimHdf5ProjectionFactory::createProjection(ossimFilename& filename)";

   // Try external geom file first:
   ossimRefPtr<ossimProjection> projection = createProjectionFromGeometryFile(filename, entryIdx);
   if (!projection.valid())
   {
      // Try something else...
   }

   // Must release or pointer will self destruct when it goes out of scope.
   return projection.release();
}

ossimProjection* ossimHdf5ProjectionFactory::createProjection(const ossimString& /*name*/) const
{
   return 0;
}

ossimProjection* ossimHdf5ProjectionFactory::createProjection(const ossimKeywordlist& /*kwl*/,
                                                              const char* /*prefix*/) const
{
   return 0;
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

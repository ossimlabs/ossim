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
   // Try external geom file first:
   ossimRefPtr<ossimProjection> projection = createProjectionFromGeometryFile(filename, entryIdx);
   if (!projection.valid())
   {
      // Try internal grid model. This may be specific to VIIRS. In any case, it is not very
      // robust as this model simply looks for "Latitude" and "Longitude" dataset names, ignoring
      // multiple entries with potentially different geometries for each. Eventually should
      // provide for the specification of specific dataset names or at least path to parent group.
      // This is not trivial with only an entry index available. (OLK 08/16)
      ossimRefPtr<ossimHdf5> hdf5 = new ossimHdf5;
      if (hdf5->open(filename))
      {
         ossimRefPtr<ossimHdf5GridModel> hdf5_grid = new ossimHdf5GridModel;
         hdf5_grid->initialize(hdf5.get());
      }
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

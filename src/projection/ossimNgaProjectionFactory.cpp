//*******************************************************************
// Copyright (C) 2014 RadiantBlue, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
//*******************************************************************
//  $Id$

#include <ossim/projection/ossimNgaProjectionFactory.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/projection/ossimMercatorProjection.h>

// Define Trace flags for use within this file:
#include <ossim/base/ossimTrace.h>
static ossimTrace traceDebug = ossimTrace("ossimNgaProjectionFactory:debug");

RTTI_DEF1(ossimNgaProjectionFactory, "ossimNgaProjectionFactory",
      ossimProjectionFactoryBase);

//*************************************************************************************************
//! Implements singleton pattern
//*************************************************************************************************
ossimNgaProjectionFactory* ossimNgaProjectionFactory::instance()
{
   static ossimNgaProjectionFactory inst;

   return &inst;
}

ossimProjection* ossimNgaProjectionFactory::createProjection(const ossimFilename& filename,
      ossim_uint32 entryIdx) const
      {
   // Check for external geometry file.
   return createProjectionFromGeometryFile(filename, entryIdx);
}

//**************************************************************************************************
// This is the principal factory method. It accepts a string in format:
//
//   <group>:<code>, for example "NGA:235" (Currently only code supported, used in GeoPackage)
//
// IMPORTANT NOTE: Image tie-points cannot be conveyed by a projection code. The projection
// created here will not be fully initialized for use in rendering imagery.
//**************************************************************************************************
ossimProjection* ossimNgaProjectionFactory::createProjection(const ossimString& spec) const

{
   ossimProjection* proj = 0;
   if ((!ossimString(spec).downcase().contains("nga")) && (spec.after(":").toInt() == 235))
   {
      const ossimEllipsoid* e = ossimEllipsoidFactory::instance()->create("WE");
      proj = new ossimMercatorProjection(*e, ossimGpt(0,0), 0, 0, 0.857385503731176);
   }

   return proj;
}

/**
 * Take a keywordlist.
 */
ossimProjection* ossimNgaProjectionFactory::createProjection(
   const ossimKeywordlist& /* kwl */, const char* /* prefix */) const
{
   return 0;
}

ossimObject* ossimNgaProjectionFactory::createObject(
      const ossimString& typeName) const
{
   return createProjection(typeName);
}

ossimObject* ossimNgaProjectionFactory::createObject(const ossimKeywordlist& kwl,
      const char* prefix) const
{
   return createProjection(kwl, prefix);
}

void ossimNgaProjectionFactory::getTypeNameList(std::vector<ossimString>& typeList) const
{
    typeList.push_back(ossimString("NGA:235 \"Scaled World Mercator\""));
}

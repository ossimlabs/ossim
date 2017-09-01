//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// Description: Unit test application for WKT Projection Factory.
//
// $Id$
//----------------------------------------------------------------------------

#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimString.h>
#include <ossim/projection/ossimWktProjectionFactory.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimCylEquAreaProjection.h>
#include <ossim/projection/ossimTransMercatorProjection.h>
#include <ossim/projection/ossimMercatorProjection.h>
#include <ossim/projection/ossimLambertConformalConicProjection.h>
#include <ossim/projection/ossimUtmProjection.h>

static const char* P1 = "PROJCS[\"NAD_1983_HARN_Lambert_Conformal_Conic\",\
GEOGCS[\"GCS_North_American_1983_HARN\",DATUM[\"NAD83_High_Accuracy_Regional_Network\",\
SPHEROID[\"GRS_1980\",6378137,298.257222101, AUTHORITY[\"EPSG\",\"7019\"]],\
AUTHORITY[\"EPSG\",\"6152\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],\
PROJECTION[\"Lambert_Conformal_Conic_2SP\"],PARAMETER[\"standard_parallel_1\",43],\
PARAMETER[\"standard_parallel_2\",45.5],PARAMETER[\"latitude_of_origin\",41.75],\
PARAMETER[\"central_meridian\",-120.5],PARAMETER[\"false_easting\",1312335.958005249],\
PARAMETER[\"false_northing\",0],UNIT[\"foot\",0.3048, AUTHORITY[\"EPSG\",\"9002\"]]]";

static const char* P2 = "PROJCS[\"WGS_1984_UTM_Zone_41N\",\
AUTHORITY[\"EPSG\",\"32641\"]],GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",\
SPHEROID[\"WGS 84\",6378137,298.2572235630016, AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],\
PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],\
PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",63],PARAMETER[\"scale_factor\",0.9996],\
PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,\
AUTHORITY[\"EPSG\",\"9001\"]]]";

static const char* P3 = "PROJCS[\"Cylindrical_Equal_Area \",\
GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",\
SPHEROID[\"WGS 84\",6378137,298.2572235630016, AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],\
PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Cylindrical_Equal_Area\"],\
PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",63],PARAMETER[\"scale_factor\",0.9996],\
PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,\
AUTHORITY[\"EPSG\",\"9001\"]]]";

static const char* P4 = "PROJCS[\"World_Equidistant_Cylindrical\",\
GEOGCS[\"GCS_WGS_1984\",DATUM[\"WGS_1984\",SPHEROID[\"WGS_1984\",6378137,298.257223563]],\
PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Equirectangular\"],\
PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],\
PARAMETER[\"Standard_Parallel_1\",60],UNIT[\"Meter\",1],AUTHORITY[\"EPSG\",\"54002\"]]";

static const char* P5 = "PROJCS[\"ETRS89 / Jersey Transverse Mercator\",\
GEOGCS[\"ETRS89\",DATUM[\"European_Terrestrial_Reference_System_1989\",\
SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],TOWGS84[0,0,0,0,0,0,0],\
AUTHORITY[\"EPSG\",\"6258\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],\
UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4258\"]],\
PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",49.225],\
PARAMETER[\"central_meridian\",-2.135],PARAMETER[\"scale_factor\",0.9999999],\
PARAMETER[\"false_easting\",40000],PARAMETER[\"false_northing\",70000],UNIT[\"metre\",1,\
AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]";

static const char* P6 = "PROJCS[\"World_Mercator\",\
GEOGCS[\"GCS_WGS_1984\",DATUM[\"WGS_1984\",SPHEROID[\"WGS_1984\",6378137,298.257223563]],\
PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Mercator_1SP\"],\
PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],\
PARAMETER[\"latitude_of_origin\",0],UNIT[\"Meter\",1]]";

int main(int argc, char* argv[])
{
   cout << "ossimWktProjectionFactory Test:" << endl;
   ossimInit::instance()->initialize(argc, argv);

   cout << "  Testing getTypeNameList()... "; cout.flush();
   std::vector<ossimString> typeList;
   ossimWktProjectionFactory::instance()->getTypeNameList(typeList);
   std::size_t size = typeList.size();
   assert(size == 2917);
   cout << "  Passed.\n" << endl;

   ossimMapProjection* proj;

//   cout << "  Testing Lambert_Conformal_Conic (P1)... "; cout.flush();
//   assert(dynamic_cast<ossimLambertConformalConicProjection*>(
//         ossimWktProjectionFactory::instance()->createProjection(ossimString(P1))));
//   cout << "  Passed.\n";

   cout << "  Testing WGS_1984_UTM_Zone_41N (P2)... "; cout.flush();
   assert(dynamic_cast<ossimUtmProjection*>(
         ossimWktProjectionFactory::instance()->createProjection(ossimString(P2))));
   cout << "  Passed.\n";

   cout << "  Testing Cylindrical_Equal_Area (P3)... "; cout.flush();
   assert(dynamic_cast<ossimCylEquAreaProjection*>(
         ossimWktProjectionFactory::instance()->createProjection(ossimString(P3))));
   cout << "  Passed.\n";

   cout << "  Testing Equidistant_Cylindrical (P4)... "; cout.flush();
   assert(dynamic_cast<ossimEquDistCylProjection*>(
         ossimWktProjectionFactory::instance()->createProjection(ossimString(P4))));
   cout << "  Passed.\n";

   cout << "  Testing Transverse_Mercator (P5)... "; cout.flush();
   assert(dynamic_cast<ossimTransMercatorProjection*>(
         ossimWktProjectionFactory::instance()->createProjection(ossimString(P5))));
   cout << "  Passed.\n";

   cout << "  Testing Mercator (P6)... "; cout.flush();
   assert(dynamic_cast<ossimMercatorProjection*>(
         ossimWktProjectionFactory::instance()->createProjection(ossimString(P6))));
   cout << "  Passed.\n";

   return 0;
}



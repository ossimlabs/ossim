//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-wkt-test.cpp
//
// Author:  David Burken
//
// Description: Contains application definition "ossim-wkt-test" app.
//----------------------------------------------------------------------------

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/init/ossimInit.h>
#include <ossim/support_data/ossimWkt.h>

#include <iostream>
#include <string>
using namespace std;

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   try
   {
      std::string wktString = "PROJCS[\"WGS 84 / UTM zone 55S\",GEOGCS[\"WGS 84\",DATUM[\"World Geodetic System_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse Mercator\",AUTHORITY[\"EPSG\",\"9807\"]],PARAMETER[\"Latitude of natural origin\",0],PARAMETER[\"Longitude of natural origin\",147],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",AUTHORITY[\"EPSG\",\"32755\"]]";

      ossimWkt wkt;
      if ( wkt.parse( wktString ) )
      {
          ossimNotify(ossimNotifyLevel_NOTICE) << wkt.getKwl() << endl;
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)  << "wkt string parse failed!" << endl;
         returnCode = 1;
      }
   }
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      returnCode = 1;
   }
   
   return returnCode;
}

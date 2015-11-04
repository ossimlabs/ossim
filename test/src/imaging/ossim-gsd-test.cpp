//----------------------------------------------------------------------------
//
// File: ossim-gsd-test.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:
//
// Test to output gsd from image geometry and then compute the gsd from center
// latitude of the image if the projecion is geographic, i.e. scale is in
// decimal degrees.
// 
//----------------------------------------------------------------------------
// $Id: ossim-gsd-test.cpp 19751 2011-06-13 15:13:07Z dburken $

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>

#include <iostream>
using namespace std;

static void usage()
{
   cout << "ossim-gsd-test <imagefile>"
        << "\nThis will compute meters per pixel from the center latitude if the\n"
        << "underlying projection scale units are in degrees." << endl;
}

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   if (argc != 2)
   {
      usage();
      return 0;
   }

   ossimFilename imageFile = argv[argc - 1];

   if ( !imageFile.exists() )
   {
      usage();
      return 1;
   }

   // Open up an image handler.
   ossimRefPtr<ossimImageHandler> ih = ossimImageHandlerRegistry::instance()->open(
      ossimFilename(argv[argc - 1]));
   if ( ih.valid() == false )
   {
      cerr << "Could not open: " << imageFile << endl;
      return 1;
   }
   
   cout << "image_file:  " << imageFile << "\n";
   
   ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
   if ( geom.valid() )
   {
      ossimDpt imageGeomGsd = geom->getMetersPerPixel();
      cout << "image_geometry_meters_per_pixel: " << imageGeomGsd << "\n";

      ossimDrect rect = ih->getImageRectangle(0);
      ossimKeywordlist kwl;
      geom->saveState(kwl);
      ossimDpt ipt = rect.midPoint();
      cout << "center_image_point: " << ipt << "\n";
      ossimGpt wpt;
      geom->localToWorld(ipt, wpt);
      cout << "center_world_point: " << wpt << "\n";

      ossimDpt scale;
      bool scaleDegrees = false;
      const char* lookup = kwl.find("projection.pixel_scale_xy");
      if ( lookup )
      {
         ossimString s = lookup;
         scale.toPoint(s);
         cout << "projection.pixel_scale_xy: " << scale << "\n";
         const char* lookup = kwl.find("projection.pixel_scale_units");
         if ( lookup )
         {
            s = lookup;
            scaleDegrees = (s == "degrees");
            cout << "projection.scale_units: " << s << "\n";
         }

         if ( scaleDegrees )
         {
            ossimDpt mpd = wpt.metersPerDegree();
            cout << "meters_per_degree_for_center: " << mpd << "\n";
            ossimDpt computeGsd;
            computeGsd.x = mpd.x * scale.x;
            computeGsd.y = mpd.y * scale.y;
            cout << "computed_meters_per_pixel_for_center_lat: " << computeGsd << "\n";
         }
      }
   }
   else
   {
      cout << "null geometry!\n";
   }

   cout << endl;
   return 0;
}


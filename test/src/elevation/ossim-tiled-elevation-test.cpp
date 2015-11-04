//----------------------------------------------------------------------------
//
// File: ossim-tiled-elevation-test.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:
//
// Test application for ossimTiledElevationDatabase class.
// 
//----------------------------------------------------------------------------
// $Id: ossim-tiled-elevation-test.cpp 22384 2013-08-30 14:09:08Z dburken $

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/elevation/ossimTiledElevationDatabase.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimTiffWriter.h>

#include <iostream>
using namespace std;

static void usage()
{
   cout << "ossim-tiled-elevation-test <elev-dir> <input-image> <output-image>"
        << "\n\nCreates a ossimTiledElevationDatabase from elev-dir for the bounding box of\ninput-image and writes output-image in tiff format.\n\nThis test the ossimTiledElevationDatabase ability to map a region of interest\nand serve it up as an elevation source.  Since this is testing elevation, the\n\"input-image\" should have a projection affected by elevation.  In other words,\nsome type of sensor model." << endl;
}

int main(int argc, char *argv[])
{
   int result = 0;

   ossimTimer::instance()->setStartTick();

   // Turn off elevation initialization as we want to use ours.
   ossimInit::instance()->setElevEnabledFlag(false);
   
   ossimInit::instance()->initialize(argc, argv);

   cout << std::setiosflags(ios::fixed) << std::setprecision(3)
        << "elapsed time after initialize: "
        << ossimTimer::instance()->time_s() << "\n";
   
   if (argc == 4)
   {
      try // Exceptions can be thrown so 
      {
         ossimFilename elevDir     = argv[1];
         ossimFilename inputImage  = argv[2];
         ossimFilename outputImage = argv[3];
         
         cout << "elev-dir: " << elevDir << "\ninput-image: " << inputImage
              << "\noutput-image: " << outputImage << "\n";
         
         ossimTiledElevationDatabase* elevdb = new ossimTiledElevationDatabase();
         
         ossimKeywordlist kwl;
         kwl.setExpandEnvVarsFlag(true);
         // kwl.add("connection_string", "$(OSSIM_DATA)/elevation/srtm/3arc");
         kwl.add("connection_string", elevDir.c_str());
         kwl.add("geoid.type", "geoid1996");
         kwl.add("max_open_cells", "50");
         kwl.add("memory_map_cells", "false");
         kwl.add("min_open_cells", "25");
         kwl.add("type", "ossimTiledElevationDatabase");
         
         if ( elevdb->loadState(kwl, 0) )
         {
            ossimRefPtr<ossimSingleImageChain> sic1 = new ossimSingleImageChain();
            sic1->setAddResamplerCacheFlag(true);
            sic1->setAddChainCacheFlag(true);

            // Open:
            if ( sic1->open( inputImage ) )
            {
               // Get the geometry:
               ossimRefPtr<ossimImageGeometry> geom = sic1->getImageGeometry();
               
               // Get the bounding rectangle:
               ossimGrect boundingRect;
               std::vector<ossimGpt> corner(4);
               if ( geom->getCornerGpts(corner[0], corner[1], corner[2], corner[3]) )
               {
                  ossimGpt ulGpt(corner[0]);
                  ossimGpt lrGpt(corner[0]);
                  cout << "corner[0]: " << corner[0] << endl;
                  for ( ossim_uint32 i = 1; i < 4; ++i )
                  {
                     cout << "corner[" << i << "]: " << corner[i] << endl;
                     if ( corner[i].lon < ulGpt.lon ) ulGpt.lon = corner[i].lon;
                     if ( corner[i].lat > ulGpt.lat ) ulGpt.lat = corner[i].lat;
                     if ( corner[i].lon > lrGpt.lon ) lrGpt.lon = corner[i].lon;
                     if ( corner[i].lat < lrGpt.lat ) lrGpt.lat = corner[i].lat;
                  }

                  //---
                  // Add a padding to ensure no nans on edges:
                  // Should check for wrap here but just test code.
                  //---
                  const ossim_float64 PADDING = 1.0/60.0; // one minute.
                  ulGpt.lon -= PADDING;
                  ulGpt.lat += PADDING;
                  lrGpt.lon += PADDING;
                  lrGpt.lat -= PADDING;
                  cout << "ulGpt: " << ulGpt
                       << "\nlrGpt: " << lrGpt
                       << endl;
                  boundingRect = ossimGrect(ulGpt, lrGpt);
               }
               else
               {
                  boundingRect.makeNan();
               }
               
               if ( boundingRect.isLonLatNan() == false )
               {
                  // Create the chain:
                  sic1->createRenderedChain();
                  
                  cout << "boundingRect: " << boundingRect << endl;
                  elevdb->mapRegion(boundingRect);

                  cout << std::setiosflags(ios::fixed) << std::setprecision(3)
                       << "elapsed time after mapping elevation: "
                       << ossimTimer::instance()->time_s() << "\n";

                  cout << "ossimTiledElevationDatabase::getMeanSpacingMeters: "
                       << elevdb->getMeanSpacingMeters() << "\n";
                  
                  cout << "mappedRect: " << elevdb->getBoundingGndRect() << endl;
                  ossimElevManager::instance()->setDefaultHeightAboveEllipsoid(0.0);
                  ossimElevManager::instance()->setUseGeoidIfNullFlag(true);
                  ossimElevManager::instance()->addDatabase(elevdb);

#if 0
                  // 35.821992089329882, 51.437673634967858
                  // Height: 1874.649761970292 MSL: 1871.642024320508
                  ossimGpt pt1(35.821992089329882, 51.437673634967858);
                  ossimGpt pt2(35.843333333333334, 51.373333333333335);
                  ossimGpt pt3(35.694166666666668, 51.598333333333336);

                  cout << "getHeightAboveEllipsoid(35.821992089329882, 51.437673634967858): "
                       << ossimElevManager::instance()->getHeightAboveEllipsoid(pt1)
                       << "\ngetHeightAboveEllipsoid(35.843333333333334, 51.373333333333335): "
                       << ossimElevManager::instance()->getHeightAboveEllipsoid(pt2)
                       << "\ngetHeightAboveEllipsoid(35.694166666666668, 51.598333333333336): "
                       << ossimElevManager::instance()->getHeightAboveEllipsoid(pt3) << endl;
#endif
                  
                  ossimRefPtr<ossimImageFileWriter> writer = new ossimTiffWriter();
                  if ( writer->open( outputImage ) )
                  {
                     // Add a listener to get percent complete.
                     ossimStdOutProgress prog(0, true);
                     writer->addListener(&prog);
                     
                     writer->connectMyInputTo(0, sic1.get());
                     writer->execute();
                     cout << std::setiosflags(ios::fixed) << std::setprecision(3)
                          << "elapsed time in seconds: "
                          << ossimTimer::instance()->time_s() << "\n";
                  }
                  else
                  {
                    cout << "Could not open: " << outputImage << endl;
                  }
               }
               else
               {
                  cout << "bounding box for " << inputImage << " has nans...\n";
               }
               
            } // if ( sic1->open( inputImage) ) )
            else
            {
               cout << "Could not open: " << inputImage << endl;
            }
            
            // ossimDrect boundingRect = elevdb->getBounding
         }

            
      }
      catch( const ossimException& e )
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         result = 1;
      }
   }
   else
   {
      usage();
   }
   return result;
}


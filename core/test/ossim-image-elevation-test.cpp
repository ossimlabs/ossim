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
// Test application for ossimImageElevationDatabase class.
// 
//----------------------------------------------------------------------------
// $Id: ossim-image-elevation-test.cpp 22197 2013-03-12 02:00:55Z dburken $

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/elevation/ossimImageElevationDatabase.h>

#include <iomanip>
#include <iostream>
using namespace std;

static void usage()
{
   cout << "ossim-image-elevation-test <elev-dir>"
        << "\nCreates a ossimImageElevationDatabase from elev-dir." << endl;
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
   
   if (argc == 2)
   {
      try // Exceptions can be thrown so 
      {
         ossimString elevDir = argv[1];
         
         cout << "elev-dir: " << elevDir << "\n";
         
         ossimImageElevationDatabase* elevdb = new ossimImageElevationDatabase();
         if ( elevdb->open(elevDir) )
         {
            std::vector<ossimGpt> pts(10);
            
            pts[0] = ossimGpt(3.5, -67.5);
            pts[1] = ossimGpt(7.5, -79.5);
            pts[2] = ossimGpt(35.694166666666668, 51.598333333333336);
            pts[3] = ossimGpt(35.821992089329882, 51.437673634967858);
            pts[4] = ossimGpt(35.843333333333334, 51.373333333333335);
            pts[5] = ossimGpt(3.25, -67.25);
            pts[6] = ossimGpt(7.5, -79.5);
            pts[7] = ossimGpt(35.821992089329882, 51.437673634967858);
            pts[8] = ossimGpt(7, -80);
            pts[9] = ossimGpt(7.9, -79.1);
            std::vector<ossimGpt>::iterator i = pts.begin();

            while ( i != pts.end() )
            {
               cout << "getHeightAboveEllipsoid(" << (*i) << "): " 
                    << elevdb->getHeightAboveEllipsoid( (*i) ) << endl;
               ++i;
            }
         }
         else
         {
            cout << "Could not open: " << elevDir << endl;
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


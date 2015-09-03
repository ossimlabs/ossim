//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Contains application for simple datum shift test.
//
// $Id: ossim-datum-shift.cpp 19900 2011-08-04 14:19:57Z dburken $
//----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <time.h>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimDatumFactory.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimNotify.h>

// Future use registry.
// #include <ossim/base/ossimDatumFactoryRegistry.h>


int main()   
{
   clock_t time_start = clock();

   // To initialize factories.
   ossimInit::instance()->initialize();
   clock_t time_init = clock();

   // WGS 84 datum:
   const ossimDatum* wgeDatum =
      ossimDatumFactory::instance()->create(ossimString("WGE"));
   if (!wgeDatum)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "WGE datum create failed!  Exiting..." << std::endl;
      exit(1);
   }
   
   // HARN NORTH AMERICAN 1983 NADCON datum code = NAR
   const ossimDatum* narDatum =
      ossimDatumFactory::instance()->create(ossimString("NAR"));
   if (!narDatum)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "HARN NAD 83 datum create failed!  Exiting..." << std::endl;
      exit(1);
   }

   // NORTH AMERICAN 1927 NADCON datum code = NAR
   const ossimDatum* nascDatum =
      ossimDatumFactory::instance()->create(ossimString("NAS-C"));
   if (!nascDatum)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "NAS C  datum create failed!  Exiting..." << std::endl;
      exit(1);
   }

   //---
   // Input point:
   // 41°51'14.6290" N, 090°11'06.0498"  W
   // 41.854063625169729 -90.185013857895839 149.6
   //---
   ossimGpt wgePt(41.854063625169729, -90.185013857895839, 149.6, wgeDatum);


   // Copy it.
   ossimGpt narPt  = wgePt;
   ossimGpt nascPt = wgePt;

   // Shift to new datum.
   narPt.changeDatum(narDatum);
   nascPt.changeDatum(nascDatum);

   ossimNotify(ossimNotifyLevel_NOTICE)
      << "WGS84 pt:      " << wgePt
      << "\nHARN NAD83 pt: " << narPt
      << "\nNAS-C pt:      " << nascPt
      << std::endl;

   double lat = 0.0;
   double lon = 0.0;
   
   do
   {
      // Walk the ellipsoid test.
      ossimGpt gp1 = ossimGpt(lat, lon, 0.0);
      ossimGpt gp2 = gp1;
      gp2.changeDatum(nascDatum); // Clark ellipsoid.
      double shiftLat = gp1.latd() - gp2.latd();
      double shiftLon = gp1.lond() - gp2.lond();

      ossimNotify(ossimNotifyLevel_NOTICE)
         << setiosflags(ios::fixed) << setprecision(15)
         << "\nWGS84 pt:      " << gp1
         << "\nNAS-C pt:      " << gp2
         << "\nShift lat (WGS8 - NASC): "
         << setiosflags(ios::fixed) << setprecision(15) 
         << shiftLat
         << "\nShift lon (WGS8 - NASC): "
         << shiftLon
         << std::endl;

      lat += 5.0;
      
   } while (lat < 65.0);

   clock_t time_end = clock();
   double init_time = (time_init - time_start)/(double)CLOCKS_PER_SEC;
   double exec_time = (time_end - time_start)/(double)CLOCKS_PER_SEC;
   cout <<"\nTiming (in seconds):\n"
      <<"    Initialization:  "<<init_time<<"\n"
      <<"    Total Execution: "<<exec_time<<"\n"<<endl;
   ofstream tfile ("ossim-datum-shift-TIMING.csv", ios_base::app|ios_base::out);
   tfile<<init_time<<", "<<exec_time<<endl;
   tfile.close();

   return 0;
}

/*****************************************************************************
*                                                                            *
*                                 O S S I M                                  *
*            Open Source, Geospatial Image Processing Project                *
*          License: MIT, see LICENSE at the top-level directory              *
*                                                                            *
******************************************************************************
$Id: ossim_header.txt 23481 2015-08-26 15:42:55Z okramer $ */

#include <iostream>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/util/ossimHlzUtil.h>

//*****************************************************************************
// Application for finding helicopter landing zones (HLZ) on a DEM given the
// final destination and max range from destination.
//*****************************************************************************
int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ap.getApplicationUsage()->setApplicationName(argv[0]);

   // While in development mode, echo the command line for logging:
   cout<<"\nCommand: ";
   for (int i=0; i<argc; ++i)
      cout<<" "<<argv[i];
   cout<<"\n"<<endl;

   double t0 = ossimTimer::instance()->time_s();
   try
   {
      // Initialize ossim stuff, factories, plugin, etc.
      ossimInit::instance()->initialize(ap);

      t0 = ossimTimer::instance()->time_s();

      ossimRefPtr<ossimHlzUtil> hlz = new ossimHlzUtil;
      if (hlz->initialize(ap))
      {
         // Add a listener for the percent complete to standard output.
         ossimStdOutProgress prog(0, true);
         hlz->addListener(&prog);
         bool success = hlz->execute();
      }
      hlz = 0;
   }
   catch  (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<e.what()<<endl;
      exit(1);
   }

   double dt = ossimTimer::instance()->time_s() - t0;
   cout << argv[0] << "Elapsed Time: " << dt << " s\n" << endl;
   exit(0);
}

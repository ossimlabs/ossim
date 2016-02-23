//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <iostream>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/util/ossimViewshedUtil.h>
#include <ossim/base/ossimException.h>

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ap.getApplicationUsage()->setApplicationName(argv[0]);

   double t0 = ossimTimer::instance()->time_s();
   try
   {
      // Initialize ossim stuff, factories, plugin, etc.
      ossimInit::instance()->initialize(ap);

      t0 = ossimTimer::instance()->time_s();

      ossimRefPtr<ossimViewshedUtil> viewshed = new ossimViewshedUtil;
      if (viewshed->initialize(ap))
      {
         // Add a listener for the percent complete to standard output.
         ossimStdOutProgress prog(0, true);
         viewshed->addListener(&prog);
         viewshed->execute();
      }
      viewshed = 0;
   }
   catch  (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<e.what()<<endl;
      exit(1);
   }
   catch( ... )
   {
      cerr << "Caught unknown exception!" << endl;
   }

   double dt = ossimTimer::instance()->time_s() - t0;
   cout << argv[0] << "Elapsed Time: " << dt << " s\n" << endl;
   exit(0);
}

//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Application to scan image and attemp to detect the valid image vertices and
// write them to a keyword list file for later use.  Note that if input
// file is "foo.tif" this will create "foo_vertices.kwl".
//
//*******************************************************************
//  $Id: ossim-viewshed.cpp 23084 2015-01-15 23:56:48Z okramer $

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

   double t0 = ossimTimer::instance()->time_s();
   try
   {
      // Initialize ossim stuff, factories, plugin, etc.
      ossimInit::instance()->initialize(ap);

      t0 = ossimTimer::instance()->time_s();

      ossimRefPtr<ossimViewshedUtil> viewshed = new ossimViewshedUtil;
      viewshed->initialize(ap);

      // Add a listener for the percent complete to standard output.
      ossimStdOutProgress prog(0, true);
      viewshed->addListener(&prog);

      // Start the viewshed process:
      viewshed->execute();
      viewshed = 0;
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

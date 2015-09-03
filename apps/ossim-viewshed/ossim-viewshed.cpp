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

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ap.getApplicationUsage()->setApplicationName(argv[0]);

   // Initialize ossim stuff, factories, plugin, etc.
   ossimInit::instance()->initialize(ap);

   double t0 = ossimTimer::instance()->time_s();

   ossimRefPtr<ossimViewshedUtil> viewshed = new ossimViewshedUtil;
   if (!viewshed->initialize(ap))
      exit(1);

   // Add a listener for the percent complete to standard output.
   ossimStdOutProgress prog(0, true);
   viewshed->addListener(&prog);
   
   // Start the viewshed process:
   bool success = viewshed->execute();
   viewshed = 0;
   
   double dt = ossimTimer::instance()->time_s() - t0;
   cout << argv[0] << "Elapsed Time: " << dt << " s\n" << endl;
   if (success)
      exit(0);
   exit(1);
}

//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Oscar Kramer
//
// Description:
//
// Application for finding helicopter landing zones (HLZ) on a DEM given the final destination and
// max range from destination.
//
//*******************************************************************
//  $Id: ossim-hlz.cpp 23167 2015-02-24 22:07:14Z okramer $

#include <iostream>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/util/ossimHLZUtil.h>

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ap.getApplicationUsage()->setApplicationName(argv[0]);

   // While in development mode, echo the command line for logging:
   cout<<"\nCommand: ";
   for (int i=0; i<argc; ++i)
      cout<<" "<<argv[i];
   cout<<"\n"<<endl;

   // Initialize ossim stuff, factories, plugin, etc.
   ossimInit::instance()->initialize(ap);

   double t0 = ossimTimer::instance()->time_s();

   ossimRefPtr<ossimHLZUtil> hlz = new ossimHLZUtil;
   if (!hlz->parseCommand(ap))
      exit(1);

   // Add a listener for the percent complete to standard output.
   ossimStdOutProgress prog(0, true);
   hlz->addListener(&prog);
   
   // Start the viewshed process:
   bool success = hlz->execute();
   hlz = 0;
   
   double dt = ossimTimer::instance()->time_s() - t0;
   cout << argv[0] << "Elapsed Time: " << dt << " s\n" << endl;
   if (success)
      exit(0);
   exit(1);
}

//---
//
// License: MIT
//
// File: ossim-foo.cpp
//
// Description: Contains application definition "ossim-foo" app.
//
// NOTE:  This is supplied for simple quick test. DO NOT checkin your test to
//        the svn repository.  Simply edit ossim-foo.cpp and run your test.
//        After completion you can do a "git checkout -- ossimfoo.cpp" if
//        you want to keep your working repository up to snuff.
//
// $Id$
//---

// System includes:
#include <cmath>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>

// Put your includes here:
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageGeometryRegistry.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/base/ossimTimer.h>

using namespace std;

int main(int argc, char *argv[])
{
   ossimTimer timer;
   // Start the timer.
   ossimTimer::Timer_t tickStart = timer.tick();
   ossimArgumentParser ap(&argc, argv);

   // Initialize ossim stuff, factories, plugin, etc.
   ossimInit::instance()->initialize(ap);
   try
   {
       /* code */
   }
   catch(const std::exception& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      exit(1);
   }
   ossimTimer::Timer_t tickEnd = timer.tick();

   ossimNotify(ossimNotifyLevel_NOTICE)
       << "elapsed time in seconds: "
       << std::setiosflags(ios::fixed)
       << std::setprecision(3)
       << timer.delta_s(tickStart, tickEnd) << endl;

   return 0;
}

//----------------------------------------------------------------------------
//
// File: ossim-chipper.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Utility class definition for processing digital elevation models(dems).
//
//----------------------------------------------------------------------------
// $Id$

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/init/ossimInit.h>
#include <ossim/util/ossimChipperUtil.h>

#include <cstdlib> /* for exit */
#include <iomanip>
#include <iostream>

using namespace std;

static ossimTrace traceDebug = ossimTrace("ossim-chipper:debug");

int main(int argc, char* argv[])
{
   // Start the timer.
   ossimTimer::instance()->setStartTick();

   //---
   // Get the arg count so we can tell if an arg was consumed by
   // ossimInit::instance()->initialize
   //---
   int originalArgCount = argc;
   
   ossimArgumentParser ap(&argc, argv);

   // Initialize ossim stuff, factories, plugin, etc.
   ossimInit::instance()->initialize(ap);

   //---
   // Avoid going on if a global option was consumed by ossimInit::initialize
   // like -V or --version option and the arg count is down to 1.
   //---
   if ( ( ap.argc() > 1 ) || ( ap.argc() == originalArgCount ) )
   {
      // Make the generator.
      ossimRefPtr<ossimChipperUtil> chipper = new ossimChipperUtil;

      try
      {      
         //---
         // NOTE: ossimChipperUtil::initialize handles the application usage which will
         // false, to end things if certain options (e.g. "--help") are provided.
         //
         // ossimChipperUtil::initialize can throw an exception.
         //---
         bool continue_after_init = chipper->initialize(ap);
         if (continue_after_init)
         {      
            // ossimChipperUtil::execute can throw an excepion.
            chipper->execute();
            
            ossimNotify(ossimNotifyLevel_NOTICE)
               << "elapsed time in seconds: "
               << std::setiosflags(ios::fixed)
               << std::setprecision(3)
               << ossimTimer::instance()->time_s() << endl;
         }
      }
      catch (const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         exit(1);
      }
      
   } // End: if ( ( ap.argc() > 1 ) ...

   exit(0);
   
} // End of main...

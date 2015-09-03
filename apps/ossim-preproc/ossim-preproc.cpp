//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: ossim-info.cpp 18619 2011-01-03 10:33:51Z dburken $

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/init/ossimInit.h>
#include <ossim/util/ossimImageUtil.h>

int main(int argc, char *argv[])
{
   // Return 0 on success, something else on error.
   enum
   {
      OK    = 0,
      ERROR = 1
   };
   
   int result = OK;
   
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
      // Make the info object.
      ossimRefPtr<ossimImageUtil> oiu = new ossimImageUtil();
      
      try
      {
         //---
         // Initialize will take the options passed in and set things to output
         // information for.
         //
         // ossimInfo::initialize can throw an exception.
         //---
         bool continue_after_init = oiu->initialize(ap);
         
         if ( continue_after_init )
         {
            // Execute the operation(s).
            result = oiu->execute();

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
         result = ERROR;
      }
      
   }  // End: if ( ( ap.argc() > 1 ) ...
   
   return result;
   
} // End of main...

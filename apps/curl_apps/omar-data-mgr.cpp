//---
// File: omar-data-mrg.cpp
//
// License: See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
// 
// Description: omar-data-mrg application for interfacing with OMAR
// dataManager services, e.g. "add", "remove" raster via libcurl.
//---

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/init/ossimInit.h>
#include "omarDataMgrUtil.h"

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
   int rv = 0; // Return value.
   
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
      ossimRefPtr<omarDataMgrUtil> dm = new omarDataMgrUtil();

      try
      {
         //---
         // Initialize will take the options passed in and set things to output
         // information for.
         //
         // omarDataMgrUtil::initialize can throw an exception.
         //---
         bool continue_after_init = dm->initialize(ap);

         if ( continue_after_init )
         {
            // This does all the work...
            rv = (int)dm->execute();
         }
      }
      catch (const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         rv = 1;
      }
      
   }  // End: if ( ( ap.argc() > 1 ) ...
   
   return rv;
   
} // End of main...

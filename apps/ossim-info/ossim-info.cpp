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
#include <ossim/init/ossimInit.h>
#include <ossim/util/ossimInfo.h>

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
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
      ossimRefPtr<ossimInfo> oi = new ossimInfo;

      try
      {
         //---
         // Initialize will take the options passed in and set things to output
         // information for.
         //
         // ossimInfo::initialize can throw an exception.
         //---
         bool continue_after_init = oi->initialize(ap);

         if ( continue_after_init )
         {
            // This will actually output the information.
            oi->execute();
         }
      }
      catch (const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         return 1;
      }
      
   }  // End: if ( ( ap.argc() > 1 ) ...
   
   return 0;
   
} // End of main...

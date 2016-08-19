//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Test application for ossimInfo class.
// 
//----------------------------------------------------------------------------
// $Id: ossim-info-test.cpp 22197 2013-03-12 02:00:55Z dburken $

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/init/ossimInit.h>
#include <ossim/support_data/ossimInfoBase.h>
#include <ossim/support_data/ossimInfoFactoryRegistry.h>
#include <ossim/util/ossimInfo.h>

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
   //---
   // Get the arg count so we can tell if an arg was consumed by
   // ossimInit::instance()->initialize
   //---
   // int originalArgCount = argc;

   ossimArgumentParser ap(&argc, argv);

   // Initialize ossim stuff, factories, plugin, etc.
   ossimInit::instance()->initialize(ap);

   if ( ap.argc() == 2 )
   {
      try
      {
         // Test the ossimInfo::getImageInfo method.
         ossimRefPtr<ossimInfo> oi = new ossimInfo;
         ossimFilename file(argv[1]);

         ossimKeywordlist kwl;
         oi->getImageInfo(file,
                          true,  // dump
                          false, // dno
                          true,  // image geom
                          true,  // image info
                          true,  // metadata
                          true,  // palette
                          kwl);
         cout << kwl << endl;

         cout << "\n\ntest info dump to a keyword list:\n";
         ossimRefPtr<ossimInfoBase> info = ossimInfoFactoryRegistry::instance()->create(file);
         if (info.valid())
         {
            info->setProcessOverviewFlag(false);

            ossimKeywordlist kwl;
            // if ( info->getKeywordlist( kwl, 9 ) )
            if ( info->getKeywordlist( kwl ) )
            {
               cout << kwl << endl;
            }
            info = 0;
         }
         else
         {
            ossimNotify(ossimNotifyLevel_INFO)
               << "No dump available for:  " << file.c_str() << std::endl;
         } 
      }
      catch (const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         return 1;
      }
      
   }  // End: if ( ( ap.argc() == 2 ) ...
   else
   {
      cout << argv[0] << " <image_file>\nTest ossimInfo class..." << endl;
   }
   
   return 0;
   
} // End of main...

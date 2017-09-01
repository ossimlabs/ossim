//---
// License: MIT
//
// File: ossim-quickbird-metadata-test.cpp
//
// Description: Test code for Quickbird/Digital Globe dot.IMD file.
//---
// $Id$

#include <ossim/base/ossimException.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/init/ossimInit.h>
#include <ossim/support_data/ossimQuickbirdMetaData.h>
#include <iostream>

using namespace std;

static void usage()
{
   cout << "ossim-quickbird-metadata-test <IMD_file>" << endl;
}

int main(int argc, char *argv[])
{
   int returnCode = 0;

   ossimInit::instance()->initialize(argc, argv);

   if (argc == 2)
   {
      try
      {
         ossimFilename file = argv[argc - 1];
         if ( file.exists() )
         {
            ossimQuickbirdMetaData md;
            if ( md.open( file ) )
            {
               md.print( cout );
            }

            ossimKeywordlist kwl;
            if ( md.getMapProjectionKwl( file, kwl ) == true )
            {
               cout << "kwl:\n" << kwl << endl;
            }
         }
         else
         {
            usage();
            returnCode = 1;
         }
      }
      catch(const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         returnCode = 1;
      }
      catch( ... )
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossim-foo caught unhandled exception!" << std::endl;
         returnCode = 1;
      }
   }
   else
   {
      usage();
   }
   
   return returnCode;
}

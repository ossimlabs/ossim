//----------------------------------------------------------------------------
//
// File: ossim-image-chain-test.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:
//
// Test application for ossimImageChain class.
// 
//----------------------------------------------------------------------------
// $Id: ossim-image-chain-test.cpp 19751 2011-06-13 15:13:07Z dburken $

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageChain.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>

#include <iostream>
using namespace std;

static void usage()
{
   cout << "ossim-image-chain-test <image>" << endl;
}

int main(int argc, char *argv[])
{
   int result = 0;
   
   ossimInit::instance()->initialize(argc, argv);

   if (argc == 2)
   {
      try // In case exceptions is thrown. 
      {
         ossimFilename image = argv[1];
         ossimRefPtr<ossimImageHandler> ih = ossimImageHandlerRegistry::instance()->open(image);
         if ( ih.valid() )
         {
            cout << "Opened image: " << image << endl;

            ossimRefPtr<ossimImageChain> ic = new ossimImageChain();
            ic->addLast( ih.get() );
            cout << "Added to image chain via ossimImageChain::addLast" << endl;

            cout << "Calling ossimImageChain::removeChild" << endl;

            if ( ic->removeChild( ih.get() ) )
            {
               cout << "Removed from chain..." << endl;
               ih = 0;
            }

            ic = 0;
         }
         else
         {
            cout << "Could not open: " << image << endl;
            result = 1;
         }
      }
      catch( const ossimException& e )
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         result = 1;
      }
   }
   else
   {
      usage();
   }

   return result;
}

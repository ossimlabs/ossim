//---
// File: ossim-chipper-test.cpp
//
// License: MIT
//
// Author:  David Burken
// 
// Description: Test application for ossimChipperUtil class.
//---
// $Id$

#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/init/ossimInit.h>
#include <ossim/util/ossimChipperUtil.h>

#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
   int status = -1;

   // Timer for elapsed time:
   ossimTimer::instance()->setStartTick();

   try
   {
      ossimInit::instance()->initialize(argc, argv);
      
      if (argc == 3 )
      {
         ossimFilename chipperOptions = argv[1];
         ossimFilename outputFile = argv[2];
         cout << "chipper_options file: " << chipperOptions
              << "output file:          " << outputFile << "\n";
         
         if ( chipperOptions.exists() )
         {
            ossimRefPtr<ossimKeywordlist> kwl = new ossimKeywordlist();
            if ( kwl->addFile( chipperOptions ) )
            {
               cout << "chipper_options:\n" << *(kwl.get()) << endl;
               
               ossimRefPtr<ossimChipperUtil> chipper = new ossimChipperUtil();
               chipper->initialize( *(kwl.get()));
               
               // ossimRefPtr<ossimImageData> chip = chipper->getChip( kwl->getMap() );
               ossimRefPtr<ossimImageData> chip = chipper->getChip( );
               cout << "chip:\n" << *(chip.get()) << endl;
               if ( chip.valid() )
               {
                  status = chip->getDataObjectStatus();
                  if ( status != (int)OSSIM_NULL )
                  {
                     if ( chip->write( outputFile ) )
                     {
                        cout << "Wrote file: " << outputFile << endl;
                        status = 0;
                     }
                     else
                     {
                        cerr << "osismImageData::write(...) failed!" << endl;
                     }
                  }
                  else
                  {
                     cerr << "returned chip status OSSIM_NULL!" << endl;
                  }
               }
               else
               {
                  cerr << "NULL chip returned from chipper->getChip(...)" << endl;
               }
            }
            else
            {
               cerr << "Could not open: " << chipperOptions << endl;
            }  
         }
         else // Match: if ( inputFile.exists() ){ ... }
         {
            cerr << "Input options file does not exists!" << endl;
         }
      }
      else // Matches: if (argc == 3 ){ ... }
      {
         cout << argv[0] << " <chipper_options> <output_file>"
              << "\n\nWrites out a single image data raster buffer.\n" << endl;
      }
   }
   catch( const ossimException& e )
   {
      cerr << "Caught exception: " << e.what() << endl;
   }
   catch( ... )
   {
      cerr << "Caught unknown exception!" << endl;
   }
   
   cout << "Elapsed time in seconds: "
        << std::setiosflags(ios::fixed)
        << std::setprecision(3)
        << ossimTimer::instance()->time_s() << "\n";
   
   return status;
   
} // End of main(...)

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

static void usage( const std::string& app )
{
   cout << app
         << " <chipper_options> <output_file>"
         << "\n\nWrites out a single image data raster buffer.\n"
         << "\nSample chipper options file that can be cut and pasted in a "
         << "\"chipper_options.kwl\" file for test.\n"
         << "hist_op: auto-minmax\n"
         << "image0.file: /data1/test/data/public/applanix/utm/17657707.tif\n"
         << "image0.entry: 0\n"
         << "operation: chip\n"
         << "thumbnail_resolution: 256\n"
         << "three_band_out: true\n"
         << "output_radiometry: U8\n"
         << "pad_thumbnail: true\n"
         << endl;
}

int main(int argc, char* argv[])
{
   int status = -1;

   if (argc != 3 )
   {
      usage( std::string( argv[0] ) );
      return 0;
   }

   ossimFilename chipperOptions = argv[1];
   ossimFilename outputFile = argv[2];
   ostringstream exceptionMsg;

   if ( !chipperOptions.exists() )
   {
      cerr << "Input options file <"<<chipperOptions<<"> does not exists!" << endl;
      return -1;
   }

   // Timer for elapsed time:
   ossimTimer::instance()->setStartTick();

   try
   {
      ossimInit::instance()->initialize(argc, argv);

      ossimFilename chipperOptions = argv[1];
      ossimFilename outputFile = argv[2];
      cout << "chipper spec file: " << chipperOptions
            << "output file:          " << outputFile << "\n";

      ossimKeywordlist kwl;
      if ( !kwl.addFile( chipperOptions ) )
      {
         exceptionMsg<<"ERROR: Could not open <"<<chipperOptions<<"> as keyword list." << ends;
         throw (ossimException(exceptionMsg.str()));
      }

      cout << "chipper input specifications:\n" << kwl << endl;

      ossimRefPtr<ossimChipperUtil> chipper = new ossimChipperUtil();
      chipper->initialize( kwl);

      ossimRefPtr<ossimImageData> chip = chipper->getChip( );
      if ( !chip.valid() )
      {
         exceptionMsg<<"ERROR: NULL chip returned from chipper->getChip()." << ends;
         throw (ossimException(exceptionMsg.str()));
      }

      status = chip->getDataObjectStatus();
      if ( status == (int)OSSIM_NULL )
      {
         exceptionMsg<<"ERROR: returned chip status OSSIM_NULL!" << ends;
         throw (ossimException(exceptionMsg.str()));
      }

      if ( !chip->write( outputFile ) )
      {
         exceptionMsg<<"ERROR: Could not write <"<<outputFile<<">." << ends;
         throw (ossimException(exceptionMsg.str()));
      }
      cout << "Wrote file: " << outputFile << endl;
      status = 0;
   }
   catch( const ossimException& e )
   {
      cerr << "Caught exception: " << e.what() << endl;
   }
   catch( ... )
   {
      cerr << "Caught unknown exception!" << endl;
   }

   cout << "Elapsed time in seconds: " << std::setiosflags(ios::fixed) << std::setprecision(3)
   << ossimTimer::instance()->time_s() << endl;

   return status;

} // End of main(...)

//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:
//
// Test app for ossimWavelength class.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/init/ossimInit.h>
#include <ossim/support_data/ossimEnviHeader.h>
#include <ossim/support_data/ossimWavelength.h>

#include <iostream>
#include <vector>
using namespace std;

static void usage()
{
   cout << "\nUsage:\nossim-wavelength-test <envi-header-file> <wavelength> <threshold_from_center>"
        << "\nWill output found closest index and wavelength."
        << "Note: Units in nanometers"
        << "\nContents of minimal header file:"
        << "\nwavelength units = Nanometers"
        << "\nwavelength = {374.323608,  382.530487,  390.737427,  398.944336,  407.150970}"
        << "\n" << endl;
}

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   if (argc != 4)
   {
      usage();
      return 0;
   }

   ossimFilename hdrFile                   = argv[argc - 3];
   ossimString   waveLengthString          = argv[argc - 2];
   ossimString   thresholdFromCenterString = argv[argc - 1];
   ossim_float64 wavelength                = waveLengthString.toFloat64();
   ossim_float64 thresholdFromCenter       = thresholdFromCenterString.toFloat64();
   

   cout << "header file: " << hdrFile
        << "\nwavelength:  " << waveLengthString
        << "\nthreshold from center: " << thresholdFromCenterString
        << "\n";

   if ( hdrFile.exists() )
   {
      ossimEnviHeader hdr;
      if ( hdr.open( hdrFile ) )
      {
         ossimWavelength wl;
         if ( wl.initialize( hdr ) )
         {
            cout << "\nossimWavelength::getRgbBands returned:\n";
            std::vector<ossim_uint32> bands;
            ossim_uint32 band = 0;
            if ( wl.getRgbBands( bands ) )
            {
               std::vector<ossim_uint32>::const_iterator i = bands.begin();
               while ( i != bands.end() )
               {
                  cout << "band[" << band << "]: " << (*i) << "\n";
                  ++band;
                  ++i;
               }
            }
            else
            {
               cout << "error...\n";
            }

            cout << "\nossimWavelength::findClosestIterator(...) returned:\n";
            ossimWavelength::WavelengthMap::const_iterator i =
               wl.findClosestIterator( wavelength, thresholdFromCenter );
            if ( i != wl.end() )
            {
               cout << "(*i).first: " << (*i).first << " (*i).second: " << (*i).second << "\n";
            }
            else
            {
               cout << "error...\n";
            }

            cout << "\nossimWavelength::findClosestIndex(...) returned:\n";
            ossim_int32 index = wl.findClosestIndex( wavelength, thresholdFromCenter );
            if ( index > -1 )
            {
               cout << "index: " << index << "\n";
            }
            else
            {
               cout << "error...\n";
            }


            
         }
         else
         {
            cerr << "ossimWavelength::initialize( ossimEnviHeader ) failed!" << endl;
         }
      }
      else
      {
         cout << "Could not open: " << hdrFile << endl;
      }
   }
   else
   {
      cerr << "File: " << hdrFile << "\nDoes not exists!" << endl;
      usage();
   }
   cout << endl;
   
   return 0;
   
} // End of main...


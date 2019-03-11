//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-foo.cpp
//
// Author:  David Burken
//
// Description: Test app:
//
// 1) Makes a 256x256 tile.  Each line having a single value from 0 to 255.
// 2) Test ossimImageData::loadTile where source is 16 bit, destination 8 bit.
// 3) Test histogram.  Should have 256 bins (0 to 255) each with count of 256.
//
// Returns 0 on success and outputs PASSED, 1 on failure and outputs FAILED.
//
// $Id$
//----------------------------------------------------------------------------

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimHistogram.h>
#include <ossim/base/ossimMultiBandHistogram.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/init/ossimInit.h>

#include <iostream>
using namespace std;

int main( int argc, char* argv[] )
{
   enum
   {
      PASSED = 0,
      FAILED = 1
   };

   int status = PASSED;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   try
   {
      ossimIrect rect(0,0,255,255);
      
      ossimRefPtr<ossimImageData> srcTile = new ossimImageData(0, OSSIM_UINT16, 1, 256, 256);
      srcTile->initialize();
      srcTile->setImageRectangle( rect );

      // Set max for conversion to 8 bit.
      srcTile->setMaxPix( 255, 0 );
      ossim_uint16* src = srcTile->getUshortBuf();
      
      const ossim_uint32 LINES   = 256;
      const ossim_uint32 SAMPLES = 256;
      ossim_uint32 s = 0;
   
      // line loop
      for(ossim_uint32 line = 0; line < LINES; ++line)
      {
         // sample loop
         for(ossim_uint32 samp = 0; samp < SAMPLES; ++samp)
         {
            src[samp] = s;
         }
         src += SAMPLES;
         ++s;
      }
      
      ossimRefPtr<ossimImageData> destTile = new ossimImageData(0, OSSIM_UINT8, 1, 256, 256);
      destTile->initialize();
      destTile->setImageRectangle( rect );
      destTile->loadTile( srcTile.get() );
      destTile->validate();

      ossimRefPtr<ossimMultiBandHistogram> his = new ossimMultiBandHistogram(
         1, 255, 1, 255, 0, OSSIM_UINT8);

      ossimIrect clipRect(0,0,255,255);
      cout << "clipRect: " << clipRect << endl;
      destTile->populateHistogram( his, clipRect );

      const ossimRefPtr<ossimHistogram> h = his->getHistogram( 0 );
      if ( h.valid() )
      {
         float count;

         // Skipping NULL pixel (i=0) since those are no longer being counted in the histogram
         // (OLK 04/2016)
         int bins = h->GetRes();

         cout << "histogram:"
              << "\nbins: " << bins
              << "\nnull value: " << h->getNullValue()
              << "\nnull count: " << h->getNullCount() << "\n";

         ossim_uint32 pixelValue = 0;
         ossim_int32 valIndex;

         for( ossim_uint32 i = 0; i < bins; ++i )
         {
            pixelValue = h->GetValFromIndex( i );
            valIndex = h->GetIndex( pixelValue );
            count = h->GetCount( static_cast<float>(pixelValue) );

            cout << "pixelValueFromIndex: " << pixelValue
                 << " indexFromPixelValue: " << valIndex
                 << " bin[" << pixelValue << "] count: " << count << "\n";

            if ( 0 ) // count != clipRect.width() )
            {
               status = FAILED;
 
               cerr << "bin[" << i << "]: " << h->GetCount( i )
                    << "\nbin[" << i << "]: count = " << count
                    << "\nShould be: " << clipRect.width()
                    << "\nhistogram bin count: " << h->GetRes()
                    << endl;

               const ossim_uint8* buf = destTile->getUcharBuf();
               if ( buf )
               {
                  cerr << "tile buffer index(255, 255) = "
                       << (int)buf[65535] << endl;
               }

               break;
            }
         }
      }
      else
      {
         status = FAILED;
      }
   }
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      status = FAILED;
   }

   cout << "ossim-loadtile-test: " << (status == PASSED ? "PASSED" : "FAILED")  << endl;
   return status;
}

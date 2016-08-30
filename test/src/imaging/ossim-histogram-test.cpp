//---
// License: MIT
//
// File: ossim-quickbird-metadata-test.cpp
//
// Description: Test code for ossim histogram stuff.
//---
// $Id$

#include <ossim/base/ossimException.h>
#include <ossim/base/ossimHistogram.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimMultiResLevelHistogram.h>
#include <ossim/base/ossimMultiBandHistogram.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/init/ossimInit.h>
#include <cmath>
#include <iostream>

using namespace std;

static void usage()
{
   cout << "ossim-histogram-test <ossim_histogram_file>" << endl;
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
            ossimRefPtr<ossimMultiResLevelHistogram> mrh = new ossimMultiResLevelHistogram();
            if ( mrh.valid() )
            {
               if ( mrh->importHistogram(file) == true )
               {
                  cout << "opened histogram: " << file << "\n";
                  
                  ossimRefPtr<ossimMultiBandHistogram> mbh = mrh->getMultiBandHistogram(0);
                  if ( mbh.valid() )
                  {
                     const ossim_uint32 BANDS = mbh->getNumberOfBands();
                     cout << "bands: " << BANDS << "\n";
                     for ( ossim_uint32 band = 0; band < BANDS; ++band)
                     {
                        ossimRefPtr<ossimHistogram> h = mbh->getHistogram(band);
                        if ( h.valid() )
                        {
                           ossim_uint32 min_idx = std::floor(h->GetMinVal());
                           ossim_uint32 max_idx = std::floor(h->GetMaxVal());
                           
                           cout << "band:            " << band << "\n"
                                << "min_val:         " << h->GetMinVal() << "\n"
                                << "max_val:         " << h->GetMaxVal() << "\n"
                                << "max_count:       " << h->GetMaxCount() << "\n"
                                << "min_val_count:   " << h->GetCounts()[min_idx] << "\n"
                                << "max_val_count:   " << h->GetCounts()[max_idx]  << "\n"
                                << "vmin:            " << h->GetRangeMin() << "\n"
                                << "vmax:            " << h->GetRangeMax() << "\n"
                                << "low_clip(0%):    " << std::floor( h->LowClipVal(0.0) ) << "\n"
                                << "high_clip(0%):   " << std::floor( h->HighClipVal(0.0) ) << "\n"
                                << "low_clip(0.1%):  " << std::floor( h->LowClipVal(0.001) ) << "\n"
                                << "high_clip(0.1%): " << std::floor( h->HighClipVal(0.001) ) << "\n"
                                << "low_clip(1%):    " << std::floor( h->LowClipVal(0.01) ) << "\n"
                                << "high_clip(1%):   " << std::floor( h->HighClipVal(0.01) ) << "\n\n";
                       }
                     }
                     cout << endl;
                  }
               }
            }
            else
            {
               cout << "could not open: " << file << endl;
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

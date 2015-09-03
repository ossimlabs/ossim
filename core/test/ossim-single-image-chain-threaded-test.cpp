//----------------------------------------------------------------------------
//
// File: ossim-single-image-chain-threaded-test.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Test application for ossimSingleImageChain with a
// ossimMultiThreadSequencer.
// 
//----------------------------------------------------------------------------
// $Id: ossim-single-image-chain-threaded-test.cpp 20269 2011-11-18 15:40:56Z dburken $

#include <iostream>
using namespace std;

#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/init/ossimInit.h>
#include <ossim/parallel/ossimMultiThreadSequencer.h>

int main(int argc, char* argv[])
{
   ossimTimer::instance()->setStartTick();
   
   ossimInit::instance()->initialize(argc, argv);

   ossimTimer::Timer_t t1 = ossimTimer::instance()->tick();
   
   cout << "elapsed time after initialize(ms): "
        << ossimTimer::instance()->time_s() << "\n";

   if (argc < 4)
   {
      cout << argv[0] << "<threads> <image_file> <output_file.tif>"
           << "\nOpens up single image chain and writes to output_file using N threads."
           << " list.\nNOTE: Hard wired tiff writer; hence, the output_file.tif." << endl;
      return 0;
   }

   ossim_uint32  threads     = ossimString(argv[1]).toUInt32();
   if ( threads == 0 )
   {
      threads = ossim::getNumberOfThreads();
   }
   ossimFilename image_file  = argv[2];
   ossimFilename output_file = argv[3];

   cout << "threads:     " << threads
        << "\nimage_file:  " << image_file
        << "\noutput_file: " << output_file
        << "\n";
   
   ossimRefPtr<ossimSingleImageChain> sic1 = new ossimSingleImageChain();
   
   if ( sic1->open( image_file ) )
   {
      sic1->createRenderedChain();

      // Establish the MT sequencer:
      ossimRefPtr<ossimMultiThreadSequencer> mts =
         new ossimMultiThreadSequencer( sic1.get(), threads );

      ossimRefPtr<ossimImageFileWriter> writer = new ossimTiffWriter();
      writer->changeSequencer( mts.get() );
      
      if ( writer->open( output_file ) )
      {
         // Add a listener to get percent complete.
         ossimStdOutProgress prog(0, true);
         writer->addListener(&prog);

         // Connect the chain to writer.
         writer->connectMyInputTo(0, sic1.get());

         // Write the file.
         writer->execute();
         
         ossimTimer::Timer_t t2 = ossimTimer::instance()->tick();
         cout << "elapsed time after write(ms): "
              << std::setiosflags(ios::fixed)
              << std::setprecision(3)
              << ossimTimer::instance()->time_s() << "\n";
         
         cout << "write time minus initialize: "
              << std::setiosflags(ios::fixed)
              << std::setprecision(3)
              << ossimTimer::instance()->delta_s(t1, t2) << "\n";
      }
   }

   return 0;
}

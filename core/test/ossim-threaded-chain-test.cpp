//**************************************************************************************************
//                          OSSIM -- Open Source Software Image Map
//
// License:  See top level LICENSE.txt file.
//
// Authors:  Oscar Kramer, David Burken
//
// Description: Test code for multi-threaded sequencer and associated logic.
//
// $Id$
//**************************************************************************************************

#include <ossim/base/ossimConstants.h>  
#include <ossim/base/ossimCommon.h>  
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimFilterResampler.h>
#include <ossim/parallel/ossimMultiThreadSequencer.h>
#include <ossim/parallel/ossimMtDebug.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/imaging/ossimImageMosaic.h>
#include <iostream>
#include <sstream>

using namespace std;

//**************************************************************************************************
// USAGE:  ossim-threaded-chain-test <num_threads> <input_file> <output_file>
//**************************************************************************************************
int main(int argc, char *argv[])
{
   ossimTimer::instance()->setStartTick();

   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->initialize(ap);

   unsigned int tempUint;
   ossimArgumentParser::ossimParameter uintParam(tempUint);
   ossimMtDebug* mt_debug = ossimMtDebug::instance();
   bool chainCacheEnabled=false;
   if (ap.read("--chain-cache"))
      chainCacheEnabled = true;
   if (ap.read("--handler-cache"))
      mt_debug->handlerCacheEnabled = true;
   if (ap.read("--faux-input"))
      mt_debug->handlerUseFauxTile = true;
   if (ap.read("--shared-handler"))
      mt_debug->chainSharedHandlers = true;
   if (ap.read("--seq-debug"))
      mt_debug->seqDebugEnabled = true;
   if (ap.read("--timer"))
      mt_debug->seqMetricsEnabled = true;
   if (ap.read("--tblocks", uintParam))
      mt_debug->seqTimedBlocksDt = tempUint;
   if (ap.read("--cache-size", uintParam))
      mt_debug->maxTileCacheSize = tempUint;

   // Parse command line "<num_threads> <input_file> <output_file>":
   if (argc < 2)
   {
      cout << "\n   "<< argv[0] << " [options] <image_file> [<output_file.tif>]\n"
         << "\n Opens up single image chain and writes to output_file using N threads."
         << "\n If N=0, this indicates a conventional (unthreaded) sequencer will be used."
         << "\n If no output file is specified, the processed tiles are dumped on the floor."
         << "\n Only TIFF file output is supported."
         << "\n"
         << "\n Options:"
         << "\n    --threads <int>     Must be nonzero for multi-threading sequencer"
         << "\n    --shared-handler    Enable shared handlers"
         << "\n    --handler-cache     Adds cache to handler adaptor (valid only with shared handler)"
         << "\n    --chain-cache       Inserts cache source in the image chain"
         << "\n    --faux-input        Use faux input tiles"
         << "\n    --seq-debug         Enable sequencer debug"
         << "\n    --timer             Enable sequencer timing metrics"
         << "\n    --tblocks <msec>    Enable timed blocks at specified interval"
         << "\n    --cache-size <int>  Specify the max tile cache factor in sequencer"
         << endl;
      return 0;
   }
   
   double t1 = (double)ossimTimer::instance()->time_m()/1000.0;

   ossim_uint32  num_threads = 0;
   if (ap.read("--threads", uintParam))
      num_threads = tempUint;

   ossimFilename image_file  = argv[1];
   ossimFilename output_file;
   if (argc > 2)
      output_file = argv[2];
   
   // Create original chain:
   ossimRefPtr<ossimSingleImageChain> singleImageChain =
      new ossimSingleImageChain(false, chainCacheEnabled, false, false, false, false);
   if (!singleImageChain->open(image_file))
   {
      cout<<"\nUnable to open image file at <"<<image_file<<">. Aborting..."<<endl;
      return 1;
   }
   singleImageChain->createRenderedChain();
   ossimRefPtr<ossimImageRenderer> renderer = singleImageChain->getImageRenderer();
   if (renderer.valid())
      renderer->getResampler()->setFilterType(ossimFilterResampler::ossimFilterResampler_GAUSSIAN);
   ossimIrect rect = singleImageChain->getBoundingRect();

   // Create complex chain of chains:
   ossimRefPtr<ossimImageChain> product_chain = new ossimImageChain;
   ossimRefPtr<ossimImageMosaic> mosaicObject = new ossimImageMosaic;
   mosaicObject->connectMyInputTo(singleImageChain.get());
   singleImageChain->changeOwner(mosaicObject.get());
   product_chain->addFirst(mosaicObject.get());

   // Make sequencer:
   ossimRefPtr<ossimMultiThreadSequencer> mts = 0;
   ossimRefPtr<ossimImageSourceSequencer> seq = 0;
   ossimRefPtr<ossimImageFileWriter> writer = 0;
   const char* noyes[2]={"no","yes"};
   if (num_threads > 0)
   {
      // Establish the MT sequencer:
      mts = new ossimMultiThreadSequencer(product_chain.get(), num_threads);
      seq = mts.get();
      cout<<"\nUsing multi-threaded sequencer..."
         << "\n   Input file:             " << image_file
         << "\n   Output file:            " << output_file
         << "\n   Number of threads:      " << num_threads
         << "\n   Using faux input tiles: " << noyes[(int)mt_debug->handlerUseFauxTile]
         << "\n   Using shared handler:   " << noyes[(int)mt_debug->chainSharedHandlers]
         << "\n   Using handler cache:    " << noyes[(int)mt_debug->handlerCacheEnabled]
         << "\n   Using chain cache:      " << noyes[(int)chainCacheEnabled];
      if ( mt_debug->seqTimedBlocksDt > 0)
         cout<< "\n   Using timed blocks:     yes (" << mt_debug->seqTimedBlocksDt<<" ms)";
      else
         cout<< "\n   Using timed blocks:     no";
      cout<<"\n   Sequencer cache size:   " << mts->maxCacheSize() << endl;
   }
   else
   {
      cout<<"\nUsing conventional (unthreaded) sequencer..."
         << "\n   Input file:             " << image_file
         << "\n   Output file:            " << output_file<<endl;
      seq = new ossimImageSourceSequencer(product_chain.get());
   }

   if (output_file.empty())
   {
      // No writer specified, so simply perform the sequencing here:
      seq->setToStartOfSequence();
      ossim_uint32 num_tiles = seq->getNumberOfTiles();
      ossim_uint32 show_every_n = num_tiles/20;
      ossim_uint32 percent_complete = 0;
      ossim_uint32 n = 0;
      cout << "  0%   \r"<<ends;
      for (ossim_uint32 tile_idx=0; tile_idx<num_tiles; ++tile_idx)
      {
         seq->getNextTile();
         if (++n >= show_every_n)
         {
            percent_complete += 5;
            cout << "  "<<percent_complete<<"%   \r"<<ends;
            n = 0;
         }
      }
      cout<<endl;
   }
   else
   {
      // Make writer object since output file was specified:
      writer = new ossimTiffWriter();
      writer->connectMyInputTo(product_chain.get());
      writer->setOutputName(output_file);
      writer->changeSequencer( seq.get() );
      if ( !writer->open( output_file ) )
      {
         cout<<"\nUnable to create output file at <"<<output_file<<">. Aborting..."<<endl;
         return 1;
      }

      // Add a listener to get percent complete and start:
      ossimStdOutProgress prog(0, true);
      writer->addListener(&prog);

      // Write the file:
      double writeStartTime = ossimTimer::instance()->time_s();
      writer->execute();
      double writeStopTime = ossimTimer::instance()->time_s();
      double writeTime = writeStopTime - writeStartTime;
      
      double bps = ((ossim_uint64)output_file.fileSize())/writeTime;
      double megaBytePerSecond = bps / ((double) 1024*1024);
      cout << "   Write MB per second:    " << setprecision(3) << megaBytePerSecond << endl;
   }
   double t2 = (double)ossimTimer::instance()->time_m()/1000.0;

   if (mts.valid())
   {
      cout << "   Max cache used:         "<< mts->d_maxCacheUsed << endl;
      cout << "   Cache emptied count:    "<< ossimString::toString(mts->d_cacheEmptyCount) << endl;
   }
   cout << setprecision(3);

   if (mts.valid())
   {
      double jgtt = mts->d_jobGetTileT;
      double jgttpj = jgtt/num_threads;
      cout << "   Time waiting on jobs:   "<<mts->d_idleTime2<<" s"<<endl;
      cout << "   Time waiting on cache:  "<<mts->d_idleTime5<<" s"<<endl;
      cout << "   Handler getTile T:      "<<mts->handlerGetTileT()<<" s"<<endl;
      cout << "   Job getTile T:          "<<jgtt<<" s"<<endl;
      cout << "   Average getTile T/job:  "<<jgttpj<<" s"<<endl;
   }
   cout << "   Execution time:         " << t2-t1  << " s" << endl;
   cout << "   Total time:             " << t2    << " s" << endl;

   writer = 0;
   return 0;
}

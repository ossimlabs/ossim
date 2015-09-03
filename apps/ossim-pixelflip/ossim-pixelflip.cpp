//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
//*******************************************************************
//  $Id: ossim-pixelflip.cpp 2644 2011-05-26 15:20:11Z oscar.kramer $


#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <ossim/ossimConfig.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimImageTypeLut.h>

#include <ossim/imaging/ossimJpegWriter.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimPixelFlipper.h>
#include <ossim/imaging/ossimRLevelFilter.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimImageWriterFactory.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimBandSelector.h>
#include <ossim/init/ossimInit.h>
#include <ossim/parallel/ossimMpi.h>
#include <ossim/parallel/ossimImageMpiMWriterSequenceConnection.h>
#include <ossim/parallel/ossimImageMpiSWriterSequenceConnection.h>

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>

#ifdef OSSIM_HAS_MPI
#  if OSSIM_HAS_MPI
#    include <mpi.h>
#  endif
#endif

static ossimTrace traceDebug("pixelflip:main");
static void usage();
static void outputWriterTypes();
int main(int argc, char* argv[])
{
   
#ifdef OSSIM_HAS_MPI
#  if OSSIM_HAS_MPI
   ossimMpi::instance()->initialize(&argc, &argv);
   if (ossimMpi::instance()->getRank() == 0)
   {
      ossimNotify(ossimNotifyLevel_INFO)
         << "MPI running with "
         << ossimMpi::instance()->getNumberOfProcessors()
         << " processors..." << std::endl;
   }
#  endif
#endif
   
   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" flips any null pixels to a valid pixel");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" [options] <output_type> <input_file> <output_file> <target_value> <replacement_value>");
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   argumentParser.getApplicationUsage()->addCommandLineOption("-o or --create-overview", "Creates and overview for the output image");
   argumentParser.getApplicationUsage()->addCommandLineOption("-c or --clamp-value", "clamp values (any pixel with value larger than input will be clamped to input)");
   argumentParser.getApplicationUsage()->addCommandLineOption("-m",  "Replacement mode (see notes below)");
   argumentParser.getApplicationUsage()->addCommandLineOption("-w",  "output tile width(only valid with tiled output types).  Must be a multiply of 16");
   
 
   static const char MODULE[] = "pixelflip:main";
   if (traceDebug()) CLOG << " Entered..." << std::endl;
   ossimInit::instance()->initialize(argc, argv);
   // Keyword list to initialize image writers with.
   ossimKeywordlist kwl;
   const char* PREFIX = "imagewriter.";
   bool create_overview = false;
   ossim_int32 tile_width = 0;
   double clamp_value = 0.0;
   bool do_clamp = false;
   ossimPixelFlipper::ReplacementMode replacement_mode =
      ossimPixelFlipper::REPLACE_BAND_IF_TARGET;


   if(argumentParser.read("-h") || argumentParser.read("--help")||(argumentParser.argc() == 1))
   {
      argumentParser.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      usage();
      exit(0);
   }
   while(argumentParser.read("-o") || argumentParser.read("--create-overview"))
   {
      create_overview = true;
   }
   while(argumentParser.read("-c", stringParam))
   {
      clamp_value = ossimString(tempString).toDouble();
      do_clamp = true;
   }
   while(argumentParser.read("-m", stringParam))
   {
      ossimString mode = tempString;
      mode.downcase();
      if (mode == "replace_band_if_target")
      {
         replacement_mode = ossimPixelFlipper::REPLACE_BAND_IF_TARGET;
      }
      else if (mode == "replace_band_if_partial_target")
      {
         replacement_mode =
            ossimPixelFlipper::REPLACE_BAND_IF_PARTIAL_TARGET;
      }
      else if (mode == "replace_all_bands_if_partial_target")
      {
         replacement_mode = ossimPixelFlipper::REPLACE_ALL_BANDS_IF_PARTIAL_TARGET;
      }
      else if (mode != "replace_all_targets")
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "Invalid mode:  "
            << mode
            << "\nMode remains:  replace_band_if_target"
            << std::endl;
      }
   }
   while(argumentParser.read("-w", stringParam))
   {
      tile_width = ossimString(tempString).toInt32();
      if (tile_width % 16)
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE
            << " NOTICE:"
            << "\nTile width must be a multiple of 16!"
            << "\nDefaulting to 128"
            << std::endl;
         tile_width = 0;
      }
   }

   if(argumentParser.argc() != 6)
   {
      ossimMpi::instance()->finalize();
      
      exit(0);
   }
   //***
   // Set the writer type and the image type.
   //***
   ossimString output_type = argumentParser.argv()[1];
   output_type.downcase();
   kwl.add(PREFIX, ossimKeywordNames::TYPE_KW, output_type.c_str());
   // Get the input file.
   ossimFilename input_file = argumentParser.argv()[2];
   // Get the output file.
   ossimFilename    output_file = argumentParser.argv()[3];
   // Get the value to replace.
   double target_value = ossimString(argumentParser.argv()[4]).toDouble();
   // Get the replacement value.
   double replacement_value = ossimString(argumentParser.argv()[5]).toDouble();
   bool master = true;

   if (ossimMpi::instance()->getRank() != 0)
   {
      master = false;
   }
   if (master)
   {
      ossimNotify(ossimNotifyLevel_INFO)
         << "pixelflip settings:"
         << "\noutput type:        " << output_type
         << "\ninput file:         " << input_file
         << "\noutput file:        " << output_file
         << "\ntarget value:       " << target_value
         << "\nreplacement value:  " << replacement_value
         << std::endl;
      if (tile_width)
      {
         ossimNotify(ossimNotifyLevel_INFO)
            << "tile width:         " << tile_width << std::endl;
      }
      if (do_clamp)
      {
          ossimNotify(ossimNotifyLevel_INFO)
             << "clamp value:        " << clamp_value << std::endl;
      }
   }
   // Get an image handler for the input file.
   ossimRefPtr<ossimImageHandler> ih
      = ossimImageHandlerRegistry::instance()->open(ossimFilename(input_file));
   if (!ih)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "Unsupported image file:  " << input_file
         << "\nExiting application..." << std::endl;
      ossimMpi::instance()->finalize();
      exit(0);
   }
   // Initialize the
   if (ih->getErrorStatus() == ossimErrorCodes::OSSIM_ERROR)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "Error reading image:  " << input_file
         << "Exiting application..." << std::endl;
      ossimMpi::instance()->finalize();
      exit(1);
   }
   ih->initialize();
   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\nImage Handler:  " << ih->getLongName()
           << std::endl;
   }
   // Capture the bounding rectangle of the image handler.
   ossimIrect output_rect = ih->getBoundingRect(0);
   // hook up the pixel flipper to the image handler.
   ossimRefPtr<ossimPixelFlipper> pf = new ossimPixelFlipper();
   pf->connectMyInputTo(ih.get());
   pf->initialize();
   // Setup the pixel flipper target/replacement values, and replace mode.
   pf->setTargetValue(target_value);
   pf->setReplacementValue(replacement_value);
   pf->setReplacementMode(replacement_mode);
   // Setup pixel flipper clamp value
   if (do_clamp)
   {
      pf->setClampValue(clamp_value);
   }
   if (master)
   {
      ossimNotify(ossimNotifyLevel_INFO)
         << "replacement mode:   " << pf->getReplacementModeString()
         << std::endl;
   }
   ossimImageSourceSequencer* sequencer = NULL;

#ifdef OSSIM_HAS_MPI
#  if OSSIM_HAS_MPI
   // only allocate the slave connection if
   // the number of processors is larger than
   // 1
   //
   if(ossimMpi::instance()->getNumberOfProcessors() > 1)
   {
      if(ossimMpi::instance()->getRank()!=0)
      {
         sequencer = new ossimImageMpiSWriterSequenceConnection(NULL, 4);
      }
      else
      {
         sequencer = new ossimImageMpiMWriterSequenceConnection();
      }
   }
   else
   {
      sequencer = new ossimImageSourceSequencer();
   }
#  else
   // we will just load a serial connection if MPI is not supported.
   sequencer = new ossimImageSourceSequencer(NULL);
#  endif
#else
   sequencer = new ossimImageSourceSequencer(NULL);
#endif
   
   ossimRefPtr<ossimImageWriter> writer
      = ossimImageWriterFactoryRegistry::instance()->createWriter(kwl, PREFIX);
   ossimImageFileWriter* fileWriter = PTR_CAST(ossimImageFileWriter, writer.get());
   if( fileWriter == NULL )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "Error making an image writer..."
         << "\nExiting application..." << std::endl;
      ossimMpi::instance()->finalize();
      exit(1);
   }
   // Change out the sequencer...
   fileWriter->changeSequencer(sequencer);
   // Hook up the pixel flipper to the file writer.
   fileWriter->connectMyInputTo(0, pf.get());
   if (tile_width)
   {
      // Set the tile size...
      fileWriter->setTileSize(ossimIpt(tile_width, tile_width));
   }
   fileWriter->open(output_file);
   // Add a listener to get percent complete.
   ossimStdOutProgress prog(0, true);
   fileWriter->addListener(&prog);
   if (fileWriter->getErrorStatus() == ossimErrorCodes::OSSIM_OK)
   {
      fileWriter->initialize();
      fileWriter->setAreaOfInterest(output_rect); // Set the output rectangle.

#ifdef OSSIM_HAS_MPI
#  if OSSIM_HAS_MPI
      double start_time = 0.0;
      if(ossimMpi::instance()->getRank() == 0)
      {
         start_time= MPI_Wtime();
      }
#  endif
#endif

      // Filter and write the file...
      fileWriter->execute();

#ifdef OSSIM_HAS_MPI
#  if OSSIM_HAS_MPI
      if(ossimMpi::instance()->getRank() == 0)
      {
         double stop_time = MPI_Wtime();
         cout << "Elapsed time: " << (stop_time-start_time) << std::endl;
      }
#  endif
#endif
   }
   else
   {
      cerr << "Error detected in the image writer..."
           << "\nExiting application..." << std::endl;
      exit(1);
   }
   if (create_overview == true)
   {
      fileWriter->writeOverviewFile();
   }

#ifdef OSSIM_HAS_MPI
#  if OSSIM_HAS_MPI   
   ossimMpi::instance()->finalize();
#  endif
#endif
   
   exit(0);
}
void usage()
{
   ossimNotify(ossimNotifyLevel_INFO)
      << "\nNOTES:"
      << "\n-m Replacement mode option explanation:"
      << "\n   Valid modes (default=replace_all_targets) :"
      << "\n     replace_partial_targets"
      << "\n     replace_partial_targets_all_bands"
      << "\n     replace_full_targets"
      << "\n     replace_all_targets"
      << " (default=all)"
      << "\n"
      << "\nIf mode is \"replace_all_targets\" (default):"
      << "\nAny pixel with dn of target will be replaced."
      << "\n"
      << "\nIf mode is \"replace_partial_targets\":"
      << "\nTarget will be replaced only at least one subpixel(band) does "
      << "\nnot have the target."
      << "\n"
      << "\nIf mode is \"replace_partial_targets_all_bands\":"
      << "\nTarget will be replaced only at least one subpixel(band) does "
      << "\nnot have the target.  All bands will be replaced."
      << "\nThis is a good Landsat7 edge fixer..."
      << "\n"
      << "\nIf mode is \"replace_full_targets\":"
      << "\nTarget will be repaced only if all subpixels(bands) have the "
      << "target."
      << "\n"
      << "\n Example:"
      << "\n target      = 0"
      << "\n replacement = 1"
      << "\n Pixel at (0, 0) r=0, g=0,  b=0"
      << "\n Pixel at (0, 1) r=0, g=30, b=21"
      << "\n"
      << "\n Mode is \"replace_all_targets\":"
      << "\n Pixel at (0, 0) becomes  r=1, g=1,  b=1"
      << "\n Pixel at (0, 1) becomes  r=1, g=30, b=21"
      << "\n"
      << "\n Mode is \"replace_partial_targets\":"
      << "\n Pixel at (0, 0) remains  r=0, g=0,  b=0"
      << "\n Pixel at (0, 1) becomes  r=1, g=30, b=21"
      << "\n"
      << "\n Mode is \"replace_partial_targets_all_bands\":"
      << "\n Pixel at (0, 0) remains  r=0, g=0,  b=0"
      << "\n Pixel at (0, 1) becomes  r=1, g=1, b=1"
      << "\n"
      << "\n Mode is \"replace_full_targets\":"
      << "\n Pixel at (0, 0) becomes  r=1, g=1,  b=1"
      << "\n Pixel at (0, 1) remains  r=0, g=30, b=21"
      << "\n"
      << "\nValid output writer types:"
      << std::endl;
   outputWriterTypes();
}

void outputWriterTypes()
{
   std::vector<ossimString> outputType;
   
   ossimImageWriterFactoryRegistry::instance()->getImageTypeList(outputType);
   std::copy(outputType.begin(),
             outputType.end(),
             std::ostream_iterator<ossimString>(std::cout, "\t\n"));
}

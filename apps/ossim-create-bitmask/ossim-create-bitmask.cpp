//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// Description:
//
// Command line application "ossim-create-bitmask" to build overviews.
//
//----------------------------------------------------------------------------
// $Id: ossim-create-bitmask.cpp 3081 2011-12-22 16:34:12Z oscar.kramer $

#include <ossim/ossimConfig.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/imaging/ossimBitMaskWriter.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageSourceSequencer.h>
#include <ossim/imaging/ossimPixelFlipper.h> // for its keywords
#include <cstdio>
#include <cstdlib> /* for exit */

//*************************************************************************************************
// FINALIZE -- Convenient location for placing debug breakpoint for catching program exit.
//*************************************************************************************************
void finalize(int code)
{
   exit (code);
}

//*************************************************************************************************
// MAIN
//*************************************************************************************************
int main(int argc, char* argv[])
{
   ossimString tempString;
   double tempDouble, tempDouble2;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser::ossimParameter doubleParam(tempDouble);
   ossimArgumentParser::ossimParameter doubleParam2(tempDouble2);

   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+
      " Generates a bit-mask given source image and target pixel range to mask out. If the input"
      " image has overviews, then masks will be generated for all R-levels.");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+
      " [options] <input file>");

   argumentParser.getApplicationUsage()->addCommandLineOption("-d", 
      "Write mask to directory specified.");

   argumentParser.getApplicationUsage()->addCommandLineOption("-e or --entry",
      "Give the entry(zero based) for which to build a mask. Only one entry can be processed. "
      "If the input is multi-entry and no entry was specified, entry 0 is assumed.");

   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help", 
      "Shows help");

   argumentParser.getApplicationUsage()->addCommandLineOption("--mask-mode <mode>",
      "Specifies how to treat multi-band imagery when determining whether pixel will be masked "
      "according to the defined target value or range. Possible modes are: "
      "\"mask_full_and_partial_targets\" (default) | \"mask_only_full_targets\".");

   argumentParser.getApplicationUsage()->addCommandLineOption("--mask-range <min> <max>", 
      "Specifies the range of pixels to target for masking out.");

   argumentParser.getApplicationUsage()->addCommandLineOption("--mask-value <target>", 
      "Specifies the unique pixel value to target for masking out.");

   argumentParser.getApplicationUsage()->addCommandLineOption("-o", 
      "Write mask to file specified.  If used on a multi-entry file, given \"foo.mask\" you will "
      "get: \"foo_e0.mask\". If none specified, the input filename is used with \".mask\" "
      "extension.");

   argumentParser.getApplicationUsage()->addCommandLineOption("--ovr-from-image", 
      "Uses exclusively the image overview data when computing subsequent overviews. "
      "Normally the mask overview from the prior level is referenced for establishing the masks at "
      "the next level.");

   argumentParser.getApplicationUsage()->addCommandLineOption("--spec-kwl <filename>", 
      "In lieu of providing mask parameters on the command line, this option specifies a keyword "
      "list filename that contains all settings. Typically used when spawning from other process.");

   argumentParser.getApplicationUsage()->addCommandLineOption("-x or --exclude-fullres", 
      "Excludes R0 mask computation. The mask file will start at R1.");

   if(argumentParser.read("-h") || argumentParser.read("--help"))
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      finalize(0);
   }

   if ( argumentParser.read("--version") || argumentParser.read("-v"))
   {
      ossimNotify(ossimNotifyLevel_NOTICE)<< argumentParser.getApplicationName().c_str() << " " 
         << ossimInit::instance()->instance()->version().c_str()<< std::endl;
      finalize(0);
   }

   // Fetch command line options:
   ossimFilename outputFile;
   if (argumentParser.read("-o", stringParam))
      outputFile = tempString.trim();

   ossimFilename outputDir;
   if (argumentParser.read("-d", stringParam))
      outputDir = tempString.trim();

   bool exclude_r0 = false;
   if (argumentParser.read("-x") || argumentParser.read("--exclude-fullres"))
      exclude_r0 = true;

   bool entry_specified = false;
   ossim_int32 entry = 0;
   if (argumentParser.read("-e", stringParam) || argumentParser.read("--entry", stringParam))
   {
      entry = ossimString(tempString).toUInt32();
      entry_specified = true;
   }

   double target_min = 0;
   double target_max = 0;
   if (argumentParser.read("--mask-range", doubleParam, doubleParam2))
   {
      target_min = tempDouble;
      target_max = tempDouble2;
   }

   if (argumentParser.read("--mask-value", doubleParam))
   {
      target_min = tempDouble;
      target_max = target_min;
   }

   ossimString mask_mode = "REPLACE_ALL_BANDS_IF_ANY_TARGET";
   ossimString mask_mode_arg;
   if (argumentParser.read("--mask-mode", stringParam))
   {
      mask_mode_arg = tempString;
      if (mask_mode_arg == "mask_full_and_partial_targets")
         mask_mode = "REPLACE_ALL_BANDS_IF_ANY_TARGET";
      else if (mask_mode_arg == "mask_only_full_targets")
         mask_mode = "REPLACE_ONLY_FULL_TARGETS";
      else
      {
         ossimNotify(ossimNotifyLevel_NOTICE)<< argumentParser.getApplicationName().c_str() << " " 
            << " Unknown mask-mode <"<<mask_mode_arg<<"> specified. See usage below." << std::endl;
         argumentParser.getApplicationUsage()->write(std::cout);
         finalize(1);
      }
   }

   bool ovr_from_image = false;
   if (argumentParser.read("--ovr-from-image"))
      ovr_from_image = true;

   ossimFilename spec_kwl_file;
   if (argumentParser.read("--spec_kwl", stringParam))
      spec_kwl_file = tempString.trim();

   // Handle bad command line:
   argumentParser.reportRemainingOptionsAsUnrecognized();
   if (argumentParser.errors())
   {
      argumentParser.writeErrorMessages(std::cout);
      finalize(1);
   }
   if ((argumentParser.argc()<2) && spec_kwl_file.empty())
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      finalize(1);
   }

   // Establish input filename:
   ossimFilename inputFile;
   ossimKeywordlist kwl;
   if (spec_kwl_file.isReadable())
   {
      kwl.addFile(spec_kwl_file);
      inputFile = kwl.find(ossimKeywordNames::IMAGE_FILE_KW);
      outputFile = kwl.find(ossimKeywordNames::OUTPUT_FILE_KW);
   }
   else
   {
      inputFile = argv[1];
   }

   // Establish the input image handler:
   ossimRefPtr<ossimImageHandler> handler = ossimImageHandlerRegistry::instance()->open(inputFile);
   if (!handler.valid())
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<argumentParser.getApplicationName().c_str()
         <<" Error encountered opening input file <"<<inputFile<<">."<<endl;
      finalize(1);
   }

   // Establish output filename:
   if (outputFile.empty())
      outputFile = handler->getFilenameWithThisExtension("mask", entry_specified);
   if (!outputDir.empty())
      outputFile.setPath(outputDir);
   else
   {
      ossimFilename path (outputFile.path());
      if (path.empty())
         outputFile.setPath(inputFile.path());
   }

   // Consider input file with multiple entries:
   std::vector<ossim_uint32> entryList;
   handler->getEntryList(entryList); 
   if (entryList.size() <= entry)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<argumentParser.getApplicationName().c_str()
         <<" Entry specified <"<<entry<<"> exceeds total number of entries available <"
         <<entryList.size()<<">. Aborting..."<<endl;
      finalize(1);
   }
   handler->setCurrentEntry(entry); 


   // Establish a keywordlist to pass to the mask builder. This KWL may have already been specified
   // on the command line with --spec_kwl option:
   kwl.add(ossimKeywordNames::OUTPUT_FILE_KW, outputFile.chars()); // may overwrite same value
   if (kwl.getSize() == 0)
   {
      kwl.add(ossimKeywordNames::IMAGE_FILE_KW, inputFile.chars());
      ossimString target_str (ossimString::toString(target_min)+" "+ossimString::toString(target_max));
      kwl.add(ossimPixelFlipper::PF_TARGET_RANGE_KW, target_str.chars());
      kwl.add(ossimPixelFlipper::PF_REPLACEMENT_MODE_KW, mask_mode.chars());
      kwl.add(ossimPixelFlipper::PF_REPLACEMENT_VALUE_KW, (int) 0);
      if (exclude_r0)
         kwl.add(ossimBitMaskWriter::BM_STARTING_RLEVEL_KW, (int) 1);
      else
         kwl.add(ossimBitMaskWriter::BM_STARTING_RLEVEL_KW, (int) 0);
   }

   // Instantiate the bit mask processor and write out the mask file:
   ossimRefPtr<ossimBitMaskWriter> mask_writer = new ossimBitMaskWriter;
   mask_writer->loadState(kwl);
   mask_writer->connectMyInputTo(handler.get());

   // Need to loop over all R-levels. Use a sequencer:
   ossimRefPtr<ossimImageSourceSequencer> sequencer =  new ossimImageSourceSequencer(handler.get());
   sequencer->initialize();
   int num_rlevels = handler->getNumberOfDecimationLevels();
   int num_tiles = sequencer->getNumberOfTiles();
   int tile_idx = 0;
   int percent_complete = 0;
   ossimRefPtr<ossimImageData> tile = 0;

   int start_res = 0;
   if (exclude_r0)
   {
      start_res = 1;
      num_tiles = (num_tiles+3)/4;
   }

   for (int r=start_res; r<num_rlevels; r++)
   {
      ossimNotify(ossimNotifyLevel_NOTICE)<<"\nProcessing R-level "<<r<<"... "<<endl;

      // Set the area of interest to the full image rectangle at this R-level:
      ossimIrect rect (handler->getBoundingRect(r));
      sequencer->setAreaOfInterest(rect);
      sequencer->setToStartOfSequence();
      do
      {
         tile = sequencer->getNextTile(r);
         mask_writer->generateMask(tile, r);

         percent_complete = 100 * tile_idx++/num_tiles;
         ossimNotify(ossimNotifyLevel_NOTICE)<<percent_complete<< "%\r";
      } 
      while(tile.valid());
      tile_idx = 0;
      num_tiles = (num_tiles+3)/4;
      if (num_tiles == 0)
         num_tiles = 1;

      // Check if additional overviews are to be generated directly from the mask at first R-level:
      if ((r == start_res) && !ovr_from_image)
      {
         ossimNotify(ossimNotifyLevel_NOTICE)<<"\nBuilding remaining overviews from initial mask..."
            <<endl;
         mask_writer->buildOverviews(num_rlevels);
         break;
      }
   }

   // Finished sequencing all levels, ready to write out the mask buffers:
   mask_writer->close();
   ossimNotify(ossimNotifyLevel_NOTICE)<<"\nSuccessfully wrote mask file to <"<<outputFile
      <<">. Finished."<<endl;
   
   finalize(0);
}

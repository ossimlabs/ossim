//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Command line application "img2rr" to build overviews.
//
//----------------------------------------------------------------------------
// $Id: ossim-img2rr.cpp 2684 2011-06-07 15:32:23Z oscar.kramer $

#include <ctime> /* for clock function */
#include <iomanip>
#include <iostream>
#include <iterator>
#include <vector>
#include <map>

#include <tiff.h> /* for compression defines */

#include <ossim/ossimConfig.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimHistogramWriter.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHistogramSource.h>
#include <ossim/imaging/ossimOverviewBuilderBase.h>
#include <ossim/imaging/ossimOverviewBuilderFactoryRegistry.h>
#include <ossim/init/ossimInit.h>
#include <ossim/parallel/ossimMpi.h>
#include <ossim/imaging/ossimPixelFlipper.h> // for its keywords
#include <ossim/imaging/ossimBitMaskWriter.h> // for its keywords

static ossimTrace traceDebug = ossimTrace("img2rr:debug");
static ossimTrace traceTime("time");

//*************************************************************************************************
// FINALIZE -- Convenient location for placing debug breakpoint for catching program exit.
//*************************************************************************************************
static void finalize(int code)
{
   exit (code);
}

static void outputWriterTypes()
{
   ossimNotify(ossimNotifyLevel_NOTICE)
      << "\nValid overview types: " << std::endl;

   std::vector<ossimString> outputType;

   ossimOverviewBuilderFactoryRegistry::instance()->getTypeNameList(outputType);
   std::copy(outputType.begin(),
      outputType.end(),
      std::ostream_iterator<ossimString>(ossimNotify(ossimNotifyLevel_NOTICE), "\t\n"));
}

static void usage(ossimApplicationUsage* au, int code)
{
   au->write(std::cout);

   outputWriterTypes();
   
   ossimNotify(ossimNotifyLevel_NOTICE)
      << "\nExample command building j2k overviews with a histogram removing/overwriting\n"
      << "any existing overviews:\n"
      << "\nossim-img2rr -r -t ossim_kakadu_nitf_j2k --create-histogram image.tif\n"
      << std::endl;

   ossimMpi::instance()->finalize();
   
   finalize( code );
}

//*************************************************************************************************
// Compute histogram after RRDSs have been established (2-pass solution)
//*************************************************************************************************
static void computeHistogram(ossimImageHandler* ih,
   const ossimFilename& histogramFile,
   ossimHistogramMode histoMode,
   ossim_float64 histoMin,
   ossim_float64 histoMax,
   ossim_uint32 histoBins)
{
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "computeHistogram entered..."
         << "\nhistogramFile: " << histogramFile
         << std::endl;
   }

   if(ih)
   {
      ossimRefPtr<ossimImageHistogramSource> histoSource = new ossimImageHistogramSource;
      ossimRefPtr<ossimHistogramWriter> writer = new ossimHistogramWriter;

      histoSource->setMaxNumberOfRLevels(1); // Currently hard coded...

      if( !ossim::isnan(histoMin) )
      {
         histoSource->setMinValueOverride(histoMin);
      }

      if( !ossim::isnan(histoMax) )
      {
         histoSource->setMaxValueOverride(histoMax);
      }

      if(histoBins > 0)
      {
         histoSource->setNumberOfBinsOverride(histoBins);
      }
      histoSource->setComputationMode(histoMode);

      histoSource->connectMyInputTo(0, ih);
      histoSource->enableSource();
      writer->connectMyInputTo(0, histoSource.get());
      writer->setFilename(histogramFile);
      theStdOutProgress.setFlushStreamFlag(true);
      writer->addListener(&theStdOutProgress);
      writer->execute();
      writer=0;
   }
}

//*************************************************************************************************
// Compute bit-mask after RRDSs have been established (2-pass solution). Returns TRUE if successful
//*************************************************************************************************
static bool computeBitMask(const ossimFilename& f, const ossimKeywordlist& bitMaskKwl)
{
   bool rtn_status = true;
   ossimFilename tempKwlFile = f;
   tempKwlFile.setExtension("maskspec.kwl");
   if (bitMaskKwl.write(tempKwlFile))
   {
      ossimString command ("ossim-create-bitmask --spec_kwl ");
      command += tempKwlFile;
      ossimNotify(ossimNotifyLevel_INFO)<< " NOTICE:"
         <<"  Launching ossim-create-bitmask process.\n"<< std::endl;
      if (system(command.chars()))
      {
         ossimNotify(ossimNotifyLevel_WARN)<<" Error code returned from "
            <<"ossim-create-bitmask utility. Mask will not be generated."<< std::endl;
         rtn_status = false;
      }
      tempKwlFile.remove();
   }
   else
   {
      ossimNotify(ossimNotifyLevel_WARN)<<" Unable to write temporary bit-mask"
         <<"spec KWL to <"<<tempKwlFile<<">. Mask will not be generated."<< std::endl;
      rtn_status = false;
   }

   return rtn_status;
}


//*************************************************************************************************
// MAIN
//*************************************************************************************************
int main(int argc, char* argv[])
{
#if OSSIM_HAS_MPI
   ossimMpi::instance()->initialize(&argc, &argv);
#endif

   double start_time = 0.0;
   if (ossimMpi::instance()->getRank() == 0)
   {
      ossimNotify(ossimNotifyLevel_INFO)
         << "MPI running with "
         << ossimMpi::instance()->getNumberOfProcessors()
         << " processors..." << std::endl;
      start_time = ossimMpi::instance()->getTime();
   }

   ossimString tempString;
   double tempDouble, tempDouble2;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser::ossimParameter doubleParam(tempDouble);
   ossimArgumentParser::ossimParameter doubleParam2(tempDouble2);

   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   ossimApplicationUsage* au = ap.getApplicationUsage();

   au->setApplicationName(ap.getApplicationName());

   au->setDescription(ap.getApplicationName()+
      " Creates overviews and optionally a histogram for the passed in image. ");

   au->setCommandLineUsage(ap.getApplicationName()+
      " [options] <input file>");

   au->addCommandLineOption("-a or --include-fullres", 
      "Wants to include full res dataset as well as reduced res sets.");

   au->addCommandLineOption("--compression-quality",
      "Compression quality varies from 0 to 100, where 100 is best.  Currently only for JPEG "
      "compression");

   au->addCommandLineOption("--compression-type",
      "Compression type can be: NONE, JPEG, PACKBITS, or DEFLATE");

   au->addCommandLineOption("--create-histogram", 
      "Computes full histogram alongside overview.");

   au->addCommandLineOption("--create-histogram-fast", 
      "Computes a histogram in fast mode which samples partial tiles.");

   au->addCommandLineOption("--create-mask", 
      "Computes and writes out an alpha mask to a separate .mask file.");

   au->addCommandLineOption("-d", 
      "Write overview to directory specified.");

   au->addCommandLineOption("-e or --entry",
      "Give the entry(zero based) to build an overview for.");

   au->addCommandLineOption("-h or --help", 
      "Shows help");

   au->addCommandLineOption("--histogram-bins", 
      "Number of histogram bins.   This will override the default for scalar type.  Also this will "
      "force a separate pass.");

   au->addCommandLineOption("--histogram-min", 
      "Minimum histogram value to use.  This will override the default for scalar type.  Also this "
      "will force a separate pass.");

   au->addCommandLineOption("--histogram-max", 
      "Maximum histogram value to use.  This will override the default for scalar type.  Also this "
      "will force a separate pass.");

   au->addCommandLineOption("--histogram-r0", 
      "Forces create-histogram code to compute a histogram using r0 instead of the starting "
      "resolution for the overview. Requires a separate pass of R0 layer.");

   au->addCommandLineOption("--list-entries",
      "Lists the entries within the image");

   au->addCommandLineOption("--mask-mode <mode>",
      "Specifies how to treat multi-band imagery when determining whether pixel will be masked "
      "according to the defined target value or range. Possible modes are: "
      "\"mask_full_and_partial_targets\" (default) | \"mask_only_full_targets\".");

   au->addCommandLineOption("--mask-range <min> <max>", 
      "Specifies the range of pixels to target for masking out.");

   au->addCommandLineOption("--mask-value <target>", 
      "Specifies the unique pixel value to target for masking out.");

   au->addCommandLineOption("-o", 
      "Write overview to file specified.  If used on a multi-entry file, given \"foo.ovr\" you will"
      "get: \"foo_e0.ovr\"");

   au->addCommandLineOption(
      "-r or --rebuild",
      "Rebuild overviews even if they are already present.");

   au->addCommandLineOption("-s", 
      "Stop dimension for overviews.  This controls how \nmany layers will be built. If set to 64 "
      "then the builder will stop when height and width for current level are less than or equal to"
      " 64.  Note a default can be set in the ossim preferences file, setting the keyword "
      "\"overview_stop_dimension\".");

   au->addCommandLineOption("--scanForMinMax",
      "Turns on min, max scanning when reading tiles.  This option assumes the null is known.");

   au->addCommandLineOption("--scanForMinMaxNull",
      "Turns on min, max, null scanning when reading tiles.  This option tries to find a null value which is useful for float data.");

   au->addCommandLineOption("--set-property",
      "key:value NOTE: separate key value by a colon.  Deprecated, use --writer-prop instead.");

   au->addCommandLineOption(
      "-t or --type",
      "see list at bottom for valid types. (default = ossim_tiff_box)");

   au->addCommandLineOption("--tile-size", 
      "Defines the tile size for the supported overview handler.");

   au->addCommandLineOption("--version", 
      "Outputs version information.");

   au->addCommandLineOption("--writer-prop", 
      "Adds a property to send to the writer. format is name=value");

   // Optional arguments.
   bool copyAllFlag       = false;
   bool useEntryIndex     = false;
   bool listEntriesFlag   = false;
   bool tileSizeFlag      = false;
   bool rebuildFlag       = false;
   bool scanForMinMax     = false;
   bool scanForMinMaxNull = false;

   // Optional histogram creation arguments:
   ossimHistogramMode histoMode = OSSIM_HISTO_MODE_UNKNOWN;
   bool histoR0Flag = false;
   bool histoFastFlag = false;
   ossim_float64 histoMin = ossim::nan();
   ossim_float64 histoMax = ossim::nan();
   ossim_uint32 histoBins = 0;


   ossimFilename inputFile  = ossimFilename::NIL;
   ossimFilename outputFile = ossimFilename::NIL;
   ossimFilename outputDir  = ossimFilename::NIL;

   std::vector<ossim_uint32> entryList;

   ossimIpt tileSize(64, 64);
   ossim_uint32 overviewStopDimension(0);

   bool listFramesFlag = false;

   //---
   // Temporary way to pass generic things to builder.
   //---
   std::vector< ossimRefPtr<ossimProperty> > propertyList(0);

   ossimString overviewType = "ossim_tiff_box";

   // Compression typedefs in tiff.h
   ossimString compressionType = "";
   ossimString compressionQuality = "75";

   if(ap.read("-h") || ap.read("--help"))
   {
      usage( au, 0 );
   }

   if ( ap.read("--version") )
   {
      ossimNotify(ossimNotifyLevel_NOTICE)
         << ap.getApplicationName().c_str() << " " 
         << ossimInit::instance()->instance()->version().c_str()
         << std::endl;
      finalize(0);
   }

   if(ap.read("--compression-quality", stringParam))
   {
      compressionQuality = tempString;
   }
   if(ap.read("--compression-type", stringParam))
   {
      compressionType = tempString;
   }
   if( ap.read("--create-histogram"))
   {
      histoMode = OSSIM_HISTO_MODE_NORMAL;
   }
   if( ap.read("--create-histogram-fast"))
   {
      histoMode = OSSIM_HISTO_MODE_FAST;
   }

   if( ap.read("--histogram-r0"))
   {
      histoR0Flag = true;
   }
   if(ap.read("--histogram-min", stringParam))
   {
      histoMin = tempString.toFloat64();
   }
   if(ap.read("--histogram-max", stringParam))
   {
      histoMax = tempString.toFloat64();
   }
   if(ap.read("--histogram-bins", stringParam))
   {
      histoBins = tempString.toUInt32();
   }

   if(ap.read("--tile-size", stringParam))
   {
      tileSize.x = tempString.toInt32();
      tileSize.y = tileSize.x;
      tileSizeFlag = true;
   }
   if( ap.read("--list-entries"))
   {
      listEntriesFlag = true;
   }

   if( ap.read("-o", stringParam) )
   {
      outputFile = tempString.trim();
   }

   if( ap.read("-d", stringParam) )
   {
      outputDir = tempString.trim();
   }

   if( ap.read("-t", stringParam) ||
      ap.read("--type", stringParam))
   {
      overviewType = tempString.trim();
   }

   if( ap.read("-s", stringParam) )
   {
      overviewStopDimension = tempString.toUInt32();
   }

   if( ap.read("--scanForMinMax" ) )
   {
      scanForMinMax = true;
   }
   if( ap.read("--scanForMinMaxNull" ) )
   {
      scanForMinMaxNull = true;
   }

   if(ap.read("-a") || ap.read("--include-fullres"))
   {
      copyAllFlag = true;
   }

   if(ap.read("-r") || ap.read("--rebuild"))
   {
      rebuildFlag = true;
   }

   while(ap.read("-e", stringParam) ||
      ap.read("--entry", stringParam))
   {
      entryList.push_back(ossimString(tempString).toUInt32());
      useEntryIndex = true;
   }

   while( ap.read( "--set-property", stringParam) )
   {
      if (tempString.size())
      {
         ossimString propertyName;
         ossimString propertyValue;
         std::vector<ossimString> v = tempString.split(":");
         if (v.size() == 2)
         {
            propertyName = v[0];
            propertyValue = v[1];
         }
         else
         {
            propertyName = tempString;
         }
         ossimRefPtr<ossimProperty> p =
            new ossimStringProperty(propertyName, propertyValue);
         propertyList.push_back(p);
      }
   }

   while(ap.read("--writer-prop", stringParam))
   {
      if (tempString.size())
      {
         ossimString propertyName;
         ossimString propertyValue;
         std::vector<ossimString> v = tempString.split("=");
         if (v.size() == 2)
         {
            propertyName = v[0];
            propertyValue = v[1];
         }
         else
         {
            propertyName = tempString;
         }
         ossimRefPtr<ossimProperty> p =
            new ossimStringProperty(propertyName, propertyValue);
         propertyList.push_back(p);
      }
   }

   // Handle Mask options:
   bool createMaskFlag   = false;
   double target_min = 0; // default
   double target_max = 0; // default
   ossimString mask_mode = "REPLACE_ALL_BANDS_IF_ANY_TARGET"; // default
   if( ap.read("--create-mask"))
   {
      createMaskFlag = true;
   }
   if (ap.read("--mask-range", doubleParam, doubleParam2))
   {
      target_min = tempDouble;
      target_max = tempDouble2;
      createMaskFlag = true;
   }

   if (ap.read("--mask-value", doubleParam))
   {
      target_min = tempDouble;
      target_max = target_min;
      createMaskFlag = true;
   }

   ossimString mask_mode_arg;
   if (ap.read("--mask-mode", stringParam))
   {
      mask_mode_arg = tempString;
      createMaskFlag = true;
      if (mask_mode_arg == "mask_full_and_partial_targets")
         mask_mode = "REPLACE_ALL_BANDS_IF_ANY_TARGET";
      else if (mask_mode_arg == "mask_only_full_targets")
         mask_mode = "REPLACE_ONLY_FULL_TARGETS";
      else
      {
         ossimNotify(ossimNotifyLevel_NOTICE)<< ap.getApplicationName().c_str() << " " 
            << " Unknown mask-mode <"<<mask_mode_arg<<"> specified. See usage below." << std::endl;
         usage( au, 1 );
      }
   }

   ap.reportRemainingOptionsAsUnrecognized();
   if (ap.errors())
   {
      ap.writeErrorMessages(std::cout);
      ossimMpi::instance()->finalize();
      finalize(1);
   }

   if(ap.argc() < 2)
   {
      usage( au, 1 );
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "\nhistoMode:        " << histoMode
         << "\nhistoMin:         " << histoMin
         << "\nhistoMax:         " << histoMax
         << "\nhistoBins:        " << histoBins
         << "\nhistoR0Flag:      " << histoR0Flag
         << "\nscanForMinMax:    " << scanForMinMax
         << "\nscanForMinMaxull: " << scanForMinMaxNull
         << std::endl;
   }

   ossim_int32 returnStatus = ossimErrorCodes::OSSIM_OK;
   
   // Keep a list of files for time command.
   std::vector<ossimFilename> overviewFiles;

   // BIG LOOP over all input files specified on command line:
   for (int i=1 ; i < ap.argc(); i++)
   {
      inputFile = ap.argv()[i];
      ossimRefPtr<ossimImageHandler> ih = ossimImageHandlerRegistry::instance()->open(inputFile);
      if (!ih.valid())
      {
         ossimNotify(ossimNotifyLevel_WARN) << "WARNING: Unable to open image file " << inputFile
            << std::endl;
         continue;
      }

      // Get the list of entries
      if (entryList.size() == 0) 
         ih->getEntryList(entryList); 

      // User wants to see entries.
      if(listEntriesFlag)
      {
         std::cout << "Entries: ";
         std::copy(entryList.begin(), entryList.end(),
            std::ostream_iterator<ossim_uint32>(std::cout, " "));
         std::cout << std::endl;
         finalize( returnStatus );
      }

      ossimRefPtr<ossimOverviewBuilderBase> ob =
         ossimOverviewBuilderFactoryRegistry::instance()->createBuilder(overviewType);
      if ( ob.valid() == false )
      {
         std::cout << "img2rr ERROR:\nCould not create builder for:  "<< overviewType << endl;
         usage( au, 1);
      }
      propertyList.push_back(new ossimStringProperty("copy_all_flag",
         ossimString::toString(copyAllFlag)));
      propertyList.push_back(new ossimStringProperty(ossimKeywordNames::COMPRESSION_TYPE_KW,
         compressionType));
      propertyList.push_back(new ossimStringProperty(ossimKeywordNames::COMPRESSION_QUALITY_KW,
         compressionQuality));

      if(tileSizeFlag)
      {
         propertyList.push_back(new ossimStringProperty(ossimKeywordNames::OUTPUT_TILE_SIZE_KW,
            tileSize.toString()));
      }

      // Generic property setting.
      if (propertyList.size())
         ob->setProperties(propertyList);

      if (overviewStopDimension)
         ob->setOverviewStopDimension(overviewStopDimension);

      // INNER LOOP over each entry in file. At least the default 0 entry is represented:
      for(ossim_uint32 idx = 0; idx < entryList.size(); ++idx)
      {
         // If image is a single entry, avoid using entry index in filename:
         if ((entryList.size() == 1) && (entryList[0] == 0))
         {
            useEntryIndex = false;
         }
         else
         {
            useEntryIndex = true;
         }

         // Explicitly set an entry number if specified:
         if (useEntryIndex)
         {
            ih->setCurrentEntry(entryList[idx]);
            std::cout << "Creating overviews for entry number: "<< entryList[idx]<< std::endl;
         }
         else
            std::cout << "Creating overviews for file: " << inputFile << std::endl;

         if (outputFile.empty())
            outputFile = ih->getFilenameWithThisExtension(ossimString(".ovr"), useEntryIndex);

         // -d "output directory option.
         if ( (outputDir != ossimFilename::NIL) && (outputDir.isDir()) )
         {
            outputFile = outputDir.dirCat(outputFile.file());

            //check if omd file exists
            ossimFilename omdFileName = inputFile.file();
            omdFileName.setExtension("omd");
            ossimFilename omdFile = outputDir.dirCat(omdFileName);
            if (omdFile.exists())
               ih->setSupplementaryDirectory(omdFile.path());
         }

         // Force a rebuild of existing OVR file. NOTE: the input image file may still contain
         // it's own internal RRDSs. 
         if (rebuildFlag) 
         { 
            ih->closeOverview(); 
            if (outputFile.exists())
               outputFile.remove();
         }

         if ( scanForMinMax )
            ob->setScanForMinMax(scanForMinMax);

         if ( scanForMinMaxNull )
            ob->setScanForMinMaxNull(scanForMinMaxNull);

         //---
         // Histogram creation:
         // Two paths:
         // 
         // 1) In conjunction with overview sequencer which is most efficient.
         // 2) Separate function call in which case the image will be scanned when
         //    building overviews and to create histo.
         //
         // Notes:
         //
         // Path # 1 is most efficient; however limitted to single process.
         //
         // If the source image has overviews and the user wants histogram from r0 we use
         // path #2.
         //
         // If options for min, max or bin count we use path 2.
         //
         // NOTE: Similar situation exists for mask creation. If partial or complete overviews 
         // already exist, we'll need to create bitmask after OVRs are generated instead of during.
         //---
         bool singlePassHistoFlag = true; 
         if ( histoMode != OSSIM_HISTO_MODE_UNKNOWN )
         {
            if ( ( ossimMpi::instance()->getNumberOfProcessors() > 1) ||
               ( ih->hasOverviews() && histoR0Flag ) ||
               ( !ossim::isnan(histoMin) ) ||
               ( !ossim::isnan(histoMax) ) ||
               ( histoBins ) ||
               ( overviewType.contains("gdal") ) )
            {
               singlePassHistoFlag = false;
            }
            if ( traceDebug() )
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                  << "singlePassHistoFlag: " << singlePassHistoFlag << std::endl;
            }
            if ( singlePassHistoFlag )
            {
               // This will tell the overview builder to make a histogram.
               ob->setHistogramMode(histoMode);
            }
         }

         ob->setOutputFile(outputFile);
         ob->setInputSource(ih.get());

         // Tell the overview to compute alpha mask according to KWL spec assembled here.
         ossimKeywordlist bitMaskKwl;
         bool singlePassMaskPerformed = false;
         if (createMaskFlag)
         {
            bitMaskKwl.add(ossimKeywordNames::IMAGE_FILE_KW, inputFile.chars());
            ossimFilename maskFileName = outputFile;
            maskFileName.setExtension("mask");
            bitMaskKwl.add(ossimKeywordNames::OUTPUT_FILE_KW, maskFileName.chars());
            ossimString target_str (ossimString::toString(target_min)+" ");
            target_str += ossimString::toString(target_max);
            bitMaskKwl.add(ossimPixelFlipper::PF_TARGET_RANGE_KW, target_str.chars());
            bitMaskKwl.add(ossimPixelFlipper::PF_REPLACEMENT_MODE_KW, mask_mode.chars());
            bitMaskKwl.add(ossimPixelFlipper::PF_REPLACEMENT_VALUE_KW, (int) 0);
            bitMaskKwl.add(ossimBitMaskWriter::BM_STARTING_RLEVEL_KW, (int) 0);

            // If the image already has no overviews, we can generate them in a single pass along 
            // with the RRDSs (similar situation to histogram path 1):
            if (!ih->hasOverviews())
            {
               singlePassMaskPerformed = true;
               ob->setBitMaskSpec(bitMaskKwl);
            }
         }

         if ( traceTime() )
            overviewFiles.push_back(ob->getOutputFile());

         // Create the overview for this entry in this file:
         bool buildOvrStatus = ob->execute();

         if ( !buildOvrStatus )
         {
            returnStatus = ossimErrorCodes::OSSIM_ERROR;
         }

         //---
         // See if we need to run create histo the old way.
         // Note if the build status is false and a histogram mode was set run
         // it the old way.  The execute may have returned false because there
         // were already required overviews.
         //---
         if ( ( (histoMode != OSSIM_HISTO_MODE_UNKNOWN) && !singlePassHistoFlag ) ||
            ( (histoMode != OSSIM_HISTO_MODE_UNKNOWN) && !buildOvrStatus ) )
         {
            ossimFilename histoFile (outputFile);
            histoFile.setExtension(".his");
            computeHistogram(ih.get(), histoFile, histoMode, histoMin, histoMax, histoBins);
         }

         // If bit-mask was requested but could not be generated in a single pass, then need to
         // spawn the create-bit-mask app to do it:
         if (createMaskFlag && !singlePassMaskPerformed)
            computeBitMask(outputFile, bitMaskKwl);

         // Specified output filename (if any) is good for only single image/single entry. After
         // that, reset the filename to null so it assumes default name. The user should never 
         // specify an output file name if multiple images are being processed.
         outputFile.clear();

      } // END of for-loop over image file's entries

      // Finished with this file's image handler:
      ih->close();

   } // end of for-loop over all input files

   if(ossimMpi::instance()->getRank() == 0)
   {
      double stop_time = ossimMpi::instance()->getTime();
      std::cout << "Elapsed time: " << (stop_time-start_time)
         << std::endl;
   }
   ossimMpi::instance()->finalize();

   if ( traceTime() )
   {
      cout << "size: " << overviewFiles.size() << endl;

      if ( overviewFiles.size() )
      {
         const ossim_float64 MB = 1048576.0; // 1024*1024
         ossim_float64 size = 0;
         std::vector<ossimFilename>::const_iterator i = overviewFiles.begin();
         while ( i < overviewFiles.end() )
         {
            size += (*i).fileSize();
            ++i;
         }
         ossim_float64 seconds = clock()/CLOCKS_PER_SEC;
         ossim_float64 megaBytes = size/MB;
         ossim_float64 megaBytesPerSecond = megaBytes/seconds;
         ossimNotify(ossimNotifyLevel_INFO)
            << std::setiosflags(ios::fixed)
            << std::setprecision(2)
            << "Wrote " << megaBytes
            << " mega bytes in " << seconds
            << " seconds.  MB/sec="
            << megaBytesPerSecond
            << std::endl;
      }
   }

   finalize( returnStatus );
}

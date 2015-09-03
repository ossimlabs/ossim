//----------------------------------------------------------------------------
// File: ossimImageUtil.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: ossimImageUtil class definition
//
// Utility class for processing image recursively.  This is for doing things like:
// building overview, histograms, compute min/max, extract vertices.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/util/ossimImageUtil.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimContainerProperty.h>
#include <ossim/base/ossimDatum.h>
#include <ossim/base/ossimDatumFactoryRegistry.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimEllipsoid.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGeoidManager.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimPropertyInterface.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimXmlDocument.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/imaging/ossimFilterResampler.h>
#include <ossim/imaging/ossimHistogramWriter.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHistogramSource.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimOverviewBuilderFactoryRegistry.h>
#include <ossim/init/ossimInit.h>
#include <ossim/plugin/ossimSharedPluginRegistry.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/support_data/ossimSupportFilesList.h>
#include <ossim/util/ossimFileWalker.h>

#include <cstdlib>
#include <iomanip>
#include <string>
#include <vector>
 
static std::string CMM_MAX_KW                  = "cmm_max"; // CMM(ComputeMinMax)
static std::string CMM_MIN_KW                  = "cmm_min";
static std::string CMM_NULL_KW                 = "cmm_null";
static std::string COPY_ALL_FLAG_KW            = "copy_all_flag";
static std::string CREATE_HISTOGRAM_KW         = "create_histogram";
static std::string CREATE_HISTOGRAM_FAST_KW    = "create_histogram_fast";
static std::string CREATE_HISTOGRAM_R0_KW      = "create_histogram_r0";
static std::string CREATE_OVERVIEWS_KW         = "create_overviews";
static std::string DUMP_FILTERED_IMAGES_KW     = "dump_filter_image";
static std::string FALSE_KW                    = "false";
static std::string FILE_KW                     = "file";
static std::string INTERNAL_OVERVIEWS_FLAG_KW  = "internal_overviews_flag";
static std::string OUTPUT_DIRECTORY_KW         = "output_directory";
static std::string OUTPUT_FILENAMES_KW         = "output_filenames";
static std::string OVERRIDE_FILTERED_IMAGES_KW = "override_filtered_images";
static std::string OVERVIEW_STOP_DIM_KW        = "overview_stop_dimension";
static std::string OVERVIEW_TYPE_KW            = "overview_type";
static std::string READER_PROP_KW              = "reader_prop";
static std::string REBUILD_HISTOGRAM_KW        = "rebuild_histogram";
static std::string REBUILD_OVERVIEWS_KW        = "rebuild_overviews";
static std::string SCAN_MIN_MAX_KW             = "scan_for_min_max";
static std::string SCAN_MIN_MAX_NULL_KW        = "scan_for_min_max_null";
static std::string THREADS_KW                  = "threads";
static std::string TILE_SIZE_KW                = "tile_size";
static std::string TRUE_KW                     = "true";
static std::string WRITER_PROP_KW              = "writer_prop";
 
// Static trace for debugging.  Use -T ossimImageUtil to turn on.
static ossimTrace traceDebug = ossimTrace("ossimImageUtil:debug");
 
ossimImageUtil::ossimImageUtil()
   :
   ossimReferenced(),
   ossimFileProcessorInterface(),
   m_kwl( new ossimKeywordlist() ),
   m_fileWalker(0),
   m_mutex(),
   m_errorStatus(0),
   m_filteredImages(0)
{
}
 
ossimImageUtil::~ossimImageUtil()
{
   if ( m_fileWalker )
   {
      delete m_fileWalker;
      m_fileWalker = 0;
   }
}
 
void ossimImageUtil::addArguments(ossimArgumentParser& ap)
{
   // Set the general usage:
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " [options] <file-or-directory-to-walk>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->addCommandLineOption("-a or --include-fullres", "Copy full res dataset to overview file as well as building reduced res sets. Option only valid with tiff overview builder. Requires -o option.");
 
   au->addCommandLineOption("--ch or --create-histogram", "Computes full histogram alongside overview.");
 
   au->addCommandLineOption("--chf or --create-histogram-fast", "Computes a histogram in fast mode which samples partial tiles.");
 
   au->addCommandLineOption("--compression-quality", "Compression quality for TIFF JPEG takes values from 0 to 100, where 100 is best.  For J2K plugin, numerically_lossless, visually_lossless, lossy");
 
   au->addCommandLineOption("--compute-min-max", "Turns on min, max scanning when reading tiles and writes a dot omd file. This option assumes the null is known.");
 
   au->addCommandLineOption("--compute-min-max-null", "Turns on min, max, null scanning when reading tiles and write a dot omd file. This option tries to find a null value which is useful for float data.");
 
   au->addCommandLineOption("--compression-type", "Compression type can be: deflate, jpeg, lzw, none or packbits");
 
   au->addCommandLineOption("--create-histogram-r0", "Forces create-histogram code to compute a histogram using r0 instead of the starting resolution for the overview builder. Can require a separate pass of R0 layer if the base image has built in overviews.");
 
   au->addCommandLineOption("-d", "<output_directory> Write overview to output directory specified.");
 
   au->addCommandLineOption("--dump-filtered-image-list", "Outputs list of filtered images.");
 
   au->addCommandLineOption("-h", "Display this information");
 
   au->addCommandLineOption("-i or --internal-overviews", "Builds internal overviews. Requires -o option. Option only valid with tiff input image and tiff overview builder. WARNING: Modifies source image and cannot be undone!");
 
   au->addCommandLineOption("--list-entries", "Lists the entries within the image");
 
   au->addCommandLineOption("--max","Overrides max value for compute-min-max option.");
 
   au->addCommandLineOption("--min","Overrides min value for compute-min-max option.");
 
   au->addCommandLineOption("--null", "<null_value> Overrides null value for compute-min-max option.  e.g. -9999.0 for float data");
 
   au->addCommandLineOption("-o", "Creates overviews. (default=ossim_tiff_box)");
 
   au->addCommandLineOption("--of or --output-files", "Output image files we can open, exluding overviews.");

   au->addCommandLineOption("--options","<options.kwl>  This can be all or part of the application options. Also used for custom prep, per file and post system commands. To get a template you can turn on trace to the ossimImageUtil class by adding \"-T ossimImageUtil\" to your command.");
 
   au->addCommandLineOption("--ot", "<overview_type> Overview type. see list at bottom for valid types. (default=ossim_tiff_box)");
 
   au->addCommandLineOption("--override-filtered-images", "Allows processing of file that is in the filtered image list.");
 
   au->addCommandLineOption("-r or --rebuild-overviews", "Rebuild overviews even if they are already present.");
 
   au->addCommandLineOption("--rebuild-histogram", "Rebuild histogram even if they are already present.");
 
   au->addCommandLineOption("--reader-prop", "Adds a property to send to the reader. format is name=value");
 
   au->addCommandLineOption("-s",  "Stop dimension for overviews.  This controls how \nmany layers will be built. If set to 64 then the builder will stop when height and width for current level are less than or equal to 64.  Note a default can be set in the ossim preferences file by setting the keyword \"overview_stop_dimension\".");
 
   au->addCommandLineOption("--tile-size", "<size> Defines the tile size for overview builder.  Tiff option only. Must be a multiple of 16. Size will be used in both x and y directions. Note a default can be set in your ossim preferences file by setting the key \"tile_size\".");
 
   au->addCommandLineOption("--threads", "<threads> The number of threads to use. (default=1) Note a default can be set in your ossim preferences file by setting the key \"ossim_threads\".");
 
   au->addCommandLineOption("--writer-prop", "Adds a property to send to the writer. format is name=value");
 
} // void ossimImageUtil::addArguments(ossimArgumentParser& ap)
 
bool ossimImageUtil::initialize(ossimArgumentParser& ap)
{
   static const char M[] = "ossimImageUtil::initialize(ossimArgumentParser&)";
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   }
 
   bool result = true;
 
   if ( (ap.argc() == 1) || ap.read("-h") || ap.read("--help") )
   {
      usage(ap);
 
      // continue_after_init to false
      result = false;
   }
   else
   {
      // Start with clean options keyword list.
      m_kwl->clear();

      // Used throughout below:
      std::string ts1;
      ossimArgumentParser::ossimParameter sp1(ts1);
      std::string ts2;
      ossimArgumentParser::ossimParameter sp2(ts2);
 
      while ( 1 ) //  While forever loop...
      {
         if( ap.read("-a") || ap.read("--include-fullres") )
         {
            setCopyAllFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--compression-quality", sp1) )
         {
            if ( ts1.size() )
            {
               setCompressionQuality( ts1 );
            }
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--compression-type", sp1) )
         {
            if ( ts1.size() )
            {
               setCompressionType( ts1 );
            }
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--ch") || ap.read("--create-histogram") )
         {
            setCreateHistogramFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--chf") || ap.read("--create-histogram-fast") )
         {
            setCreateHistogramFastFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--compute-min-max") )
         {
            setScanForMinMax( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--compute-min-max-null") )
         {
            setScanForMinMaxNull( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--create-histogram-r0") )
         {
            setCreateHistogramR0Flag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("-d", sp1) )
         {
            setOutputDirectory( ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--dump-filtered-image-list") )
         {
            setDumpFilteredImageListFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("-i") || ap.read("--internal-overviews") )
         {
            setInternalOverviewsFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--max", sp1) )
         {
            addOption( CMM_MAX_KW, ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--min", sp1) )
         {
            addOption( CMM_MIN_KW, ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--null", sp1) )
         {
            addOption( CMM_NULL_KW, ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }

         if( ap.read("--options", sp1) )
         {
            ossimFilename optionsKwl = ts1;
            if ( optionsKwl.exists() )
            {
               if ( m_kwl->addFile(optionsKwl) == false )
               {
                  std::string errMsg = "ERROR could not open options keyword list file: ";
                  errMsg += optionsKwl.string();
                  throw ossimException(errMsg);
               }
            }
            else
            {
               std::string errMsg = "ERROR options keyword list file does not exists: ";
               errMsg += optionsKwl.string();
               throw ossimException(errMsg); 
            }
         }
         
         if( ap.read("--of") || ap.read("--output-files") )
         {
            setOutputFileNamesFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("-o") )
         {
            setCreateOverviewsFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }        
 
         if( ap.read("--ot", sp1) )
         {
            setOverviewType( ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--override-filtered-images") )
         {
            setOverrideFilteredImagesFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("-r") || ap.read("--rebuild-overviews") )
         {
            setRebuildOverviewsFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--rebuild-histogram") )
         {
            setRebuildHistogramFlag( true );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         while(ap.read("--reader-prop", sp1))
         {
            if (ts1.size())
            {
               std::string key = READER_PROP_KW;
               key += ossimString::toString( getNextReaderPropIndex() ).string();
               addOption( key, ts1 );
            }
         }
         if ( ap.argc() < 2 )
         {
            break;
         }
 
 
 
         if( ap.read("-s", sp1) )
         {
            setOverviewStopDimension( ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if ( ap.read("-tile-size", sp1))
         {
            setTileSize( ossimString(ts1).toInt32() );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         if( ap.read("--threads", sp1) )
         {
            m_kwl->addPair( THREADS_KW, ts1 );
            if ( ap.argc() < 2 )
            {
               break;
            }
         }
 
         while(ap.read("--writer-prop", sp1))
         {
            if (ts1.size())
            {
               std::string key = WRITER_PROP_KW;
               key += ossimString::toString( getNextWriterPropIndex() ).string();
               addOption( key, ts1 );
            }
         }
         if ( ap.argc() < 2 )
         {
            break;
         }
 
         // End of arg parsing.
         ap.reportRemainingOptionsAsUnrecognized();
         if ( ap.errors() )
         {
            ap.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
            std::string errMsg = "Unknown option...";
            throw ossimException(errMsg);
         }
 
         break; // Break from while forever.
 
      } // End while (forever) loop.
 
      if(ap.argc() > 1)
      {
         for (ossim_int32 i = 0; i < (ap.argc()-1); ++i)
         {
            ossimString kw = FILE_KW;
            kw += ossimString::toString(i);
            std::string value = ap[i+1];
            m_kwl->addPair(kw.string(), value, true);
         }
      }
      else
      {
         if ( getDumpFilterImagesFlag() )
         {
            // Caller wants to see filtered image names:
            if ( m_filteredImages.empty() )
            {
               initializeDefaultFilterList();
            }
            dumpFilteredImageList();
         }
         else
         {
            usage(ap);
         }
 
         // continue_after_init to false
         result = false; 
 
      }
 
   } // not usage
 
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "m_kwl:\n" << *(m_kwl.get()) << "\n"
         << M << " exit result = " << (result?"true":"false")
         << "\n";
   }
 
   return result;
}
 
ossim_int32 ossimImageUtil::execute()
{
   static const char M[] = "ossimImageUtil::execute()";
 
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   }

   // Launch any prep system commands:
   executePrepCommands();
 
   if ( getDumpFilterImagesFlag() )
   {
      // Caller wants to see filtered image names:
      if ( m_filteredImages.empty() )
      {
         initializeDefaultFilterList();
      }
      dumpFilteredImageList();
   }
 
   // Get the number of "file*" keywords.
   ossim_uint32 fileCount = m_kwl->numberOf("file");
 
   if ( fileCount )
   {
      if ( !m_fileWalker )
      {
         m_fileWalker = new ossimFileWalker();
      }
 
      if ( !getOverrideFilteredImagesFlag() && m_filteredImages.empty() )
      {
         initializeDefaultFilterList();
      }
 
      m_fileWalker->initializeDefaultFilterList();
 
      m_fileWalker->setNumberOfThreads( getNumberOfThreads() );
 
      // Must set this so we can stop recursion on directory based images.
      m_fileWalker->setWaitOnDirFlag( true );
 
      // This links the file walker back to our "processFile" method.
      m_fileWalker->setFileProcessor( this );
 
      // Wrap in try catch block as excptions can be thrown under the hood.
      try
      {
         // Get the list of files passed to us:
         std::vector<ossimFilename> files;
         ossim_uint32 processedFiles = 0;;
         ossim_uint32 i = 0;
         while ( processedFiles < fileCount )
         {
            ossimString kw = FILE_KW;
            kw += ossimString::toString(i);
            std::string lookup = m_kwl->findKey( kw.string() );
            if ( lookup.size() )
            {
               files.push_back( ossimFilename(lookup) );
               ++processedFiles;
            }
 
            ++i;
            if ( i > (fileCount + 100) ) break;
         }

         // Process the files:
         m_fileWalker->walk( files ); 
      }
      catch (const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "Caught exception: " << e.what() << endl;
         setErrorStatus( ossimErrorCodes::OSSIM_ERROR );
      }
 
   } // if ( fileCount )

   // Launch any post system commands:
   executePostCommands();
   
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << M << " exit status: " << m_errorStatus << std::endl;
   }
 
   // Zero is good, non zero is bad.
   return m_errorStatus; 
}
 
//---
// This method is called back by the ossimFileWalker::walk method for each file it finds that it
// deems can be processed.
//---
void ossimImageUtil::processFile(const ossimFilename& file)
{
   static const char M[] = "ossimImageUtil::processFile";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << M << " entered...\n" << "file: " << file << "\n";
   }
 
   bool processFileFlag = true;
   if ( !getOverrideFilteredImagesFlag() )
   {
      processFileFlag = !isFiltered( file );
   }
 
   if ( processFileFlag )
   {
      ossimNotify(ossimNotifyLevel_NOTICE) << "Processing file: " << file << std::endl;
 
      m_mutex.lock();
      ossimRefPtr<ossimImageHandler> ih =
         ossimImageHandlerRegistry::instance()->open(file, true, true);
      m_mutex.unlock();
 
      if ( ih.valid() && !ih->hasError() )
      {
         if ( isDirectoryBasedImage( ih.get() ) )
         {
            // Tell the walker not to recurse this directory.
            m_mutex.lock();
            m_fileWalker->setRecurseFlag(false);
            m_mutex.unlock();
         }

         // Set any reader props:
         ossimPropertyInterface* pi = dynamic_cast<ossimPropertyInterface*>(ih.get());
         if ( pi ) setProps(pi);
 
         bool consumedHistogramOptions  = false;
         bool consumedCmmOptions = false;
 
         if ( getOutputFileNamesFlag() )
         {
            // Simply output the file name of any images we can open:
            ossimNotify(ossimNotifyLevel_NOTICE) << ih->getFilename().expand(); 
         }
 
         if ( createOverviews() )
         {
            // Skip shape files...
            if ( ih->getClassName() != "ossimOgrGdalTileSource" )
            {
               createOverview(ih, consumedHistogramOptions, consumedCmmOptions);
            }
         }
 
         // Build stand alone histogram.  Note the overview sequencer may have computed for us.
         if ( hasHistogramOption() && !consumedHistogramOptions)
         {
            createHistogram( ih );
         }
 
         // Compute/Scan for min, max.  Note the overview sequencer may have computed for us.
         if ( ( scanForMinMax() || scanForMinMaxNull() ) && !consumedCmmOptions )
         {
            computeMinMax( ih );
         }

         // Launch any file system commands.
         executeFileCommands( file );
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN) << M << "\nCould not open: " << file << std::endl;
      }
   }
   else // Matches: if ( processFileFlag )
   {
      ossimNotify(ossimNotifyLevel_NOTICE)
         << "Filtered file, not processing: " << file << std::endl;
   }
 
   if(traceDebug())
   {
      // Since ossimFileWalker is threaded output the file so we know which job exited.
      ossimNotify(ossimNotifyLevel_DEBUG) << M << "\nfile: " << file << "\nexited...\n";
   }
}

// Create overview for image:
void ossimImageUtil::createOverview( ossimRefPtr<ossimImageHandler>& ih,
                                     bool& consumedHistogramOptions,
                                     bool& consumedCmmOptions )
{
   static const char M[] = "ossimImageUtil::createOverview #1";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   }
 
   if ( ih.valid() )
   {
      // Get the entry list:
      std::vector<ossim_uint32> entryList;
      ih->getEntryList(entryList);
 
      bool useEntryIndex = false;
      if ( entryList.size() )
      {
         if ( (entryList.size() > 1) || (entryList[0] != 0) ) useEntryIndex = true;
      }

      // Create the overview builder:
      ossimString overviewType;
      getOverviewType( overviewType.string() );
      ossimRefPtr<ossimOverviewBuilderBase> ob =
         ossimOverviewBuilderFactoryRegistry::instance()->createBuilder(overviewType);
      if ( ob.valid() )
      {
         // Set up any overview builder options that don't involve histograms.
         ossimPropertyInterface* pi = dynamic_cast<ossimPropertyInterface*>( ob.get() );
         if ( pi ) setProps(pi);
 
         ossim_uint32 stopDimension = getOverviewStopDimension();
         if ( stopDimension ) ob->setOverviewStopDimension(stopDimension);
 
         if ( ( scanForMinMax() || scanForMinMaxNull() ) && !hasCmmOption() )
         {
            //---
            // If scan is set and we don't have any min, max, null overrides let the
            // overview builder scan for min, max.
            //
            // If min, max, or null option is set, scan will be performed in
            // separate function.
            //---
            ob->setScanForMinMax( scanForMinMax() );
            ob->setScanForMinMaxNull( scanForMinMaxNull() );
            consumedCmmOptions = true;
         }
 
         for(ossim_uint32 idx = 0; idx < entryList.size(); ++idx)
         {
            createOverview(ih, ob, entryList[idx], useEntryIndex, consumedHistogramOptions);
         }
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ERROR:\nCould not create builder for:  "<< overviewType << std::endl;
         outputOverviewWriterTypes();
      }
   }
 
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exited...\n";
   }
}

// Create overview for entry:
void ossimImageUtil::createOverview(ossimRefPtr<ossimImageHandler>& ih,
                                    ossimRefPtr<ossimOverviewBuilderBase>& ob,
                                    ossim_uint32 entry,
                                    bool useEntryIndex,
                                    bool& consumedHistogramOptions)
{
   static const char M[] = "ossimImageUtil::createOverview #2";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   }
 
   if ( ih.valid() && ob.valid() )
   {
      if (useEntryIndex)
      {
         // Set entry before deriving file name.
         ih->setCurrentEntry(entry);
         ossimNotify(ossimNotifyLevel_NOTICE) << "entry number: "<< entry << std::endl;
      }
 
      ossimFilename outputFile =
         ih->getFilenameWithThisExtension(ossimString(".ovr"), useEntryIndex);
 
      if ( rebuildOverviews() )
      {
         ih->closeOverview(); 
         if ( outputFile.exists() )
         {
            outputFile.remove();
         }
      }
 
      if ( getInternalOverviewsFlag() )
      {
         if ( ih->getClassName() == "ossimTiffTileSource")
         {
            //---
            // INTERNAL_OVERVIEWS_FLAG_KW is set to true:
            // Tiff reader can handle internal overviews.  Set the output file to
            // input file.  Do it after the above remove so that if there were
            // external overviews they will get removed.
            //---
            outputFile = ih->getFilename();
         }
         else 
         {
            ossimNotify(ossimNotifyLevel_NOTICE)
               << "Internal overviews not supported for reader type: "
               <<ih->getClassName()
               << "\nIgnoring option..."
               << endl;
         }
      }
 
      if ( hasRequiredOverview( ih, ob ) == false )
      {
         //---
         // Set create histogram code...
         //
         // Notes:
         // 1) Must put this logic after any removal of external overview file.
         // 
         // 2) Base file could have built in overviews, e.g. jp2 files.  So the sequensor could
         //    start at R6 even if there is no external overview file.
         //
         // 3) If user want the histogram from R0 the overview builder can do as long as
         //    ossimImageHandler::getNumberOfDecimationLevels returns 1.  If we are starting
         //    overview building at R6 then we must do the create histogram in a separate path.
         //---
         ossimHistogramMode histoMode = OSSIM_HISTO_MODE_UNKNOWN;
         if ( createHistogram() ||
              ( createHistogramR0() && ( ih->getNumberOfDecimationLevels() == 1 ) ) )
         {
            histoMode = OSSIM_HISTO_MODE_NORMAL;
         }
         else if ( createHistogramFast() )
         {
            histoMode = OSSIM_HISTO_MODE_FAST;
         }
 
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG) << "Histogram mode: " << histoMode << "\n";
         }
 
         if ( histoMode != OSSIM_HISTO_MODE_UNKNOWN )
         {
            consumedHistogramOptions = true;
            ob->setHistogramMode(histoMode);
 
            ossimNotify(ossimNotifyLevel_NOTICE)
               << "Creating overviews with histogram for file: " << ih->getFilename() << std::endl;
         }
         else
         {
            if ( histoMode != OSSIM_HISTO_MODE_UNKNOWN )
            {
               consumedHistogramOptions = false;  
               ossimNotify(ossimNotifyLevel_NOTICE)
                  << "Creating overviews for file: " << ih->getFilename() << std::endl;
            }
         }
 
         ob->setOutputFile(outputFile);
         ob->setInputSource(ih.get());

         // Create the overview for this entry in this file:
         if ( ob->execute() == false )
         {
            setErrorStatus( ossimErrorCodes::OSSIM_ERROR );
            ossimNotify(ossimNotifyLevel_WARN)
               << "Error returned creating overviews for file: " << ih->getFilename() << std::endl;
         }
      }
      else
      {
         consumedHistogramOptions = false;
         ossimNotify(ossimNotifyLevel_NOTICE)
            << "Image has required reduced resolution data sets." << std::endl;
      }
   }
 
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exited...\n";
   }
}
 
bool ossimImageUtil::hasRequiredOverview( ossimRefPtr<ossimImageHandler>& ih,
                                          ossimRefPtr<ossimOverviewBuilderBase>& ob )
{
   bool result = false;
   if ( ih.valid() && ob.valid() && ( getCopyAllFlag() == false ) )
   {
      if ( ih->getClassName() == "ossimCcfTileSource" )
      {
         // CCF reader does not use external overviews.
         result = true;
      }
      else
      {
         // Note we always have one rset
         ossim_uint32 required = 1;
 
         ossim_uint32 startingResLevel      = ih->getNumberOfDecimationLevels();
         ossim_uint32 overviewStopDimension = ob->getOverviewStopDimension();
 
         ossim_uint32 largestImageDimension =
            ih->getNumberOfSamples(0) >
            ih->getNumberOfLines(0) ?
            ih->getNumberOfSamples(0) :
            ih->getNumberOfLines(0);
 
         while(largestImageDimension > overviewStopDimension)
         {
            largestImageDimension /= 2;
            ++required;
         }
 
         if ( startingResLevel >= required )
         {
            result = true;
         }
      }
   }
   return result;
}
 
void ossimImageUtil::createHistogram(ossimRefPtr<ossimImageHandler>& ih)
{
   static const char M[] = "ossimImageUtil::createHistogram #1";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   }
 
   if ( ih.valid() )
   {
      // Get the entry list:
      std::vector<ossim_uint32> entryList;
      ih->getEntryList(entryList);
 
      bool useEntryIndex = false;
      if ( entryList.size() )
      {
         if ( (entryList.size() > 1) || (entryList[0] != 0) ) useEntryIndex = true;
      }
 
      for(ossim_uint32 idx = 0; idx < entryList.size(); ++idx)
      {
         createHistogram(ih, entryList[idx], useEntryIndex);
      }
   }
 
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exited...\n";
   }
}

// Create histogram for entry:
void ossimImageUtil::createHistogram(ossimRefPtr<ossimImageHandler>& ih,
                                     ossim_uint32 entry,
                                     bool useEntryIndex)
{
   static const char M[] = "ossimImageUtil::createHistogram #2";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   }
 
   if ( ih.valid() )
   {
      if (useEntryIndex)
      {
         // Set entry before deriving file name.
         ih->setCurrentEntry(entry);
         ossimNotify(ossimNotifyLevel_NOTICE) << "entry number: "<< entry << std::endl;
      }
 
      ossimFilename outputFile =
         ih->getFilenameWithThisExtension(ossimString(".his"), useEntryIndex);

      // Only build if needed:
      if ( (outputFile.exists() == false) || rebuildHistogram() )
      {
         ossimNotify(ossimNotifyLevel_NOTICE)
            << "Computing histogram for file: " << ih->getFilename() << std::endl;
 
         // Check handler to see if it's filtering bands.
         std::vector<ossim_uint32> originalBandList(0);
         if ( ih->isBandSelector() )
         { 
            // Capture for finalize method.
            ih->getOutputBandList( originalBandList );
 
            // Set output list to input.
            ih->setOutputToInputBandList();
         }
 
         ossimRefPtr<ossimImageHistogramSource> histoSource = new ossimImageHistogramSource;
         ossimRefPtr<ossimHistogramWriter> writer = new ossimHistogramWriter;
 
         histoSource->setMaxNumberOfRLevels(1); // Currently hard coded...
 
#if 0 /* TODO tmp drb */
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
#endif
 
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "Histogram mode: " << getHistogramMode() << "\n";
         }
 
         // Connect histogram source to image handler.
         histoSource->setComputationMode( getHistogramMode() );
         histoSource->connectMyInputTo(0, ih.get() );
         histoSource->enableSource();
 
         // Connect writer to histogram source.
         writer->connectMyInputTo(0, histoSource.get());
         writer->setFilename(outputFile);
         theStdOutProgress.setFlushStreamFlag(true);
         writer->addListener(&theStdOutProgress);
 
         // Compute...
         writer->execute();
 
         writer=0;
 
         // Reset the band list.
         if ( ih->isBandSelector() && originalBandList.size() )
         {
            ih->setOutputBandList( originalBandList );
         }
 
      } // Matches: if ( (outputFile.exists() == false) || rebuildHistogram() )
 
   } // Matches: if ( ih.valid() )
 
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exited...\n";
   }
 
} // End: ossimImageUtil::createHistogram #2
 
void ossimImageUtil::computeMinMax(ossimRefPtr<ossimImageHandler>& ih)
{
   static const char M[] = "ossimImageUtil::computeMinMax #1";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   }
 
   if ( ih.valid() )
   {
      // Get the entry list:
      std::vector<ossim_uint32> entryList;
      ih->getEntryList(entryList);
 
      bool useEntryIndex = false;
      if ( entryList.size() )
      {
         if ( (entryList.size() > 1) || (entryList[0] != 0) ) useEntryIndex = true;
      }
 
      for(ossim_uint32 idx = 0; idx < entryList.size(); ++idx)
      {
         computeMinMax(ih, entryList[idx], useEntryIndex);
      }
   }
 
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exited...\n";
   }
 
} // End: ossimImageUtil::computeMinMax( ih )
 
void ossimImageUtil::computeMinMax( ossimRefPtr<ossimImageHandler>& ih,
                                    ossim_uint32 entry,
                                    bool useEntryIndex )
{
   static const char M[] = "ossimImageUtil::computeMinMax #2";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   }
 
   if ( ih.valid() )
   {
      // Get any min, max, null overrides;
      double maxValue  = ossim::nan();
      double minValue  = ossim::nan();
      double nullValue = ossim::nan();

      ossimString value = m_kwl->findKey( CMM_MAX_KW );
      if ( value.size() )
      {
         maxValue = value.toDouble();
      }

      value = m_kwl->findKey( CMM_MIN_KW );
      if ( value.size() )
      {
         minValue = value.toDouble();
      }

      bool hasNull = false;
      value = m_kwl->findKey( CMM_NULL_KW );
      if ( value.size() )
      {
         nullValue = value.toDouble();
         if ( !ossim::isnan(nullValue) )
         {
            hasNull = true;
         }
      }
 
      // Select the entry.
      ih->setCurrentEntry( entry );
 
      // Build the .omd file name.
      ossimFilename omd_file = ih->getFilename();
 
      if ( useEntryIndex )
      {
         ossim_uint32 currentEntry = ih->getCurrentEntry();
         omd_file.setExtension("");
         omd_file += "_e";
         omd_file += ossimString::toString(currentEntry);
 
         //---
         // Note:  Set extension was not used here deliberately to avoid
         // messing
         // up a MODIS file in hdf format with multiple '.'s in the file
         // name.
         //---
         omd_file += ".omd";
      }
      else
      {
         omd_file.setExtension("omd");
      }
 
      ossimRefPtr<ossimImageSourceSequencer> is = new ossimImageSourceSequencer(ih.get());
 
      //---
      // Note: getImageTileWidth/Height will return zero if the image is not
      // intenally tiles.
      //---
      ossimIpt tileWidthHeight(ih->getImageTileWidth(), ih->getImageTileHeight());
 
      if (!tileWidthHeight.x)
      {
         //---
         // Make the sequencer read entire strips from the image handler
         // at a time.  This will speed up access time for strip images
         //---
         tileWidthHeight.x = ih->getBoundingRect().width();
      }
      if(!tileWidthHeight.y)
      {
         tileWidthHeight.y = ih->getTileHeight();
      }
 
      if ( traceDebug() )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "Sequencer tile size: " << tileWidthHeight << endl;
      }
 
      is->setTileSize(tileWidthHeight);
 
      //---
      // Make arrays of doubles to hold the min/max values.
      // Initialize mins to default maxes and maxes to default mins to be
      // safe.
      //---
      ossim_uint32 i; // for numerous iterations...
 
      const ossimScalarType ST = ih->getOutputScalarType();
 
      const double DEFAULT_MIN = ossim::defaultMin(ST);
      const double DEFAULT_MAX = ossim::defaultMax(ST);
 
      const ossim_uint32 BANDS = ih->getNumberOfInputBands();
 
      vector<double> tmin(BANDS);
      vector<double> tmax(BANDS);
      vector<double> tnull(BANDS);
      for (i = 0; i < BANDS; ++i)
      {
         tmin[i] = DEFAULT_MAX;
         tmax[i] = DEFAULT_MIN;
 
         if ( hasNull )
         {
            // User passed in with --null option so set here.
            tnull[i] = nullValue;
         }
         else
         {
            tnull[i] = ih->getNullPixelValue(i);
         }
      }
 
      // Scan the image and compute the min and max.
      const double TOTAL_TILES = is->getNumberOfTiles();
      double tile_count = 0.0;
      ossimNotify(ossimNotifyLevel_INFO)
         << setiosflags(ios::fixed) << setprecision(0);
 
      if((ossim::isnan(minValue))||
         (ossim::isnan(maxValue)))
      {
         ossimRefPtr<ossimImageData> id = is->getNextTile();
         while(id.valid())
         {
            if ( hasNull )
            {
               // Pass null to image data object so it doesn't get picked up as "min".
               id->setNullPix( nullValue );
            }
            id->computeMinMaxPix(tmin, tmax);
            id = is->getNextTile();
            ++tile_count;
            ossimNotify(ossimNotifyLevel_INFO)
               << "\r"  << setw(3)
               << (tile_count / TOTAL_TILES * 100.0) << "%"
               << flush;
         }
      }
      if(!ossim::isnan(minValue))
      {
         std::fill(tmin.begin(), tmin.end(), minValue);
      }
      if(!ossim::isnan(maxValue))
      {
         std::fill(tmax.begin(), tmax.end(), maxValue);
      }
      ossimNotify(ossimNotifyLevel_WARN)
         << "\r100%\nFinished..." << endl;
 
      ossimKeywordlist okwl(omd_file);
 
      for(i = 0; i < BANDS; ++i)
      {
         //---
         // Check to see if values got assigned.  If not flip to default and
         // issue a warning.
         //---
         if (tmin[i] == DEFAULT_MAX)
         {
            tmin[i] = DEFAULT_MIN;
            ossimNotify(ossimNotifyLevel_WARN)
               << "cmm scan for min failed! Using default min."
               << std::endl;
         }
         if (tmax[i] == DEFAULT_MIN)
         {
            tmax[i] = DEFAULT_MAX;
            ossimNotify(ossimNotifyLevel_WARN)
               << "cmm scan for max failed! Using default max."
               << std::endl;
         }
            
         ossimString band =
            ossimString("band")+ossimString::toString(i+1)+".";

         if (tmin[i] == DEFAULT_MAX)
         {
            tmin[i] = DEFAULT_MIN;
         }
         // if (tmax[i] == DEFAULT_MIN){}
            
         okwl.add(band + "min_value", tmin[i], true);
         okwl.add(band + "max_value", tmax[i], true);
         okwl.add(band + "null_value", tnull[i], true); 
         
         if( traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_INFO)
               << setiosflags(ios::fixed) << setprecision(16)
               << "band" << ossimString::toString(i+1) << ".min_value: "
               << tmin[i]
               << "\nband" << ossimString::toString(i+1) << ".max_value: "
               << tmax[i]
               << "\nband" << ossimString::toString(i+1) << ".null_value: "
               << tnull[i] << endl;
         }
         
      } // End of band loop.
      
      // Add missing info (number of bands, scalar type) to the .omd file
      okwl.add("bytes_per_pixel", ossim::scalarSizeInBytes(ST), true);
      okwl.add("number_bands", BANDS, true);
      okwl.add("scalar_type", (ossimScalarTypeLut::instance()->getEntryString(ST)), true);

      // Write the file to disk:
      okwl.write(omd_file);
      ossimNotify(ossimNotifyLevel_INFO)
         << "wrote file:  " << omd_file << endl;
         
   } // Matches: if ( ih.valid() )
   
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exited...\n";
   }
   
} // End: ossimImageUtil::computeMinMax( ih, entry, ... )

void ossimImageUtil::usage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimInit::instance()->addOptions(ap);
   
   // Set app name.
   ap.getApplicationUsage()->setApplicationName(ap.getApplicationName());

   // Add options.
   addArguments(ap);
   
   // Write usage.
   ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));

   outputOverviewWriterTypes();
   
   ossimNotify(ossimNotifyLevel_INFO)
      << "\nExample commands:\n"

      << "\n// A single image standard tiff overviews, histogram:\n"
      << "ossim-preproc -o --ch <file>\n"

      << "\n// A single image with j2k overviews(requires kakadu plugin), histogram:\n"
      << "ossim-preproc --ot ossim_kakadu_nitf_j2k --ch <file>\n"
      << "\n// j2k, histogram, 4 threads\n"

      << "\n// standard tiff overviews, full histogram, 4 threads:\n"
      << "ossim-preproc -r -o --ch --threads 4 <directory_to_walk>\n"

      << "\n// j2k, histogram (fast mode), 4 threads\n"
      << "ossim-preproc -r --ot ossim_kakadu_nitf_j2k --chf --threads 4 "
      << "<directory_to_walk>\n"

      << "\n// tiff, jpeg compression, histogram, 4 threads\n"
      << "ossim-preproc -r --ch --compression-quality 75 --compression-type "
      << "jpeg --threads 4 <directory_to_walk>\n"
      << "ossim-preproc -r --ch --compression-quality 75 --compression-type "
      << "jpeg --threads 4 <directory_to_walk>\n"

      << "\n// Process all the tiffs in a directory feeding \"prep\", \"file\", \"post\" system commands via the --options option which includes indexing file into the omar database via omar-data-mgr app:\n"
      << "ossim-preproc --options preproc-options.kwl -r -o --ch *.tif\n"
      << "\n// Contents of preproc-options.kwl used in above command:\n"
      << "prep.command0: echo start_time: %{date}\n"
      << "prep.command0.strf_time_format: %Y%m%d%H%M%S\n"
      << "prep.command1: mkdir -p %{date}\n"
      << "file.command0: mv %{file_no_ext}.* %{date}/.\n"
      << "file.command1: omar-data-mgr -u http://localhost:8080/omar add %{date}/%{basename}\n"
      << "post.command0: echo end_time: %{date}\n"
      << "post.command0.strf_time_format: %Y%m%d%H%M%S\n"
      
      << "\nNOTES:\n"
      << "\n  --ch  equals --create-histogram"
      << "\n  --chf equals --create-histogram-fast"

      << "\n\nExpanded command option variables:\n\n"
      << "%{basename}        = filename without path\n"
      << "%{basename_no_ext} = filename without path and without extention\n"
      << "%{date}            = Expanded to current zulu time. Default format=yyyymmdd\n"
      << "                     output format controlled by command0.strf_time_format key.\n"
      << "%{dirname}         = path of filename\n"
      << "%{file}            = filename being processed\n"
      << "%{file_no_ext}     = filename with no extension\n"

      << std::endl;
}

// Private method:
void ossimImageUtil::outputOverviewWriterTypes() const
{
   ossimNotify(ossimNotifyLevel_NOTICE)
      << "\nValid overview types: " << std::endl;
   
   std::vector<ossimString> outputType;
   
   ossimOverviewBuilderFactoryRegistry::instance()->getTypeNameList(outputType);
   std::copy(outputType.begin(),
             outputType.end(),
             std::ostream_iterator<ossimString>(ossimNotify(ossimNotifyLevel_NOTICE), "\t\n"));
}

// Private method:
bool ossimImageUtil::isDirectoryBasedImage(const ossimImageHandler* ih) const
{
   bool result = false;
   if ( ih )
   {
      // Get the image handler name.
      ossimString imageHandlerName = ih->getClassName();
      if ( (imageHandlerName == "ossimAdrgTileSource") ||
           (imageHandlerName == "ossimCibCadrgTileSource") )  
      {
         result = true;
      }
   }
   return result;
}

void ossimImageUtil::setCreateOverviewsFlag( bool flag )
{
   addOption( CREATE_OVERVIEWS_KW, ( flag ? TRUE_KW : FALSE_KW ) );
}

bool ossimImageUtil::createOverviews() const
{
   return  keyIsTrue( CREATE_OVERVIEWS_KW );
}

void ossimImageUtil::setRebuildOverviewsFlag( bool flag )
{
   addOption( REBUILD_OVERVIEWS_KW, ( flag ? TRUE_KW : FALSE_KW ) );
   if ( flag )
   {
      setCreateOverviewsFlag( true ); // Turn on overview building.
   }
}

bool ossimImageUtil::rebuildOverviews() const
{
   return keyIsTrue( REBUILD_OVERVIEWS_KW );
}

void ossimImageUtil::setRebuildHistogramFlag( bool flag )
{
   addOption( REBUILD_HISTOGRAM_KW, ( flag ? TRUE_KW : FALSE_KW ) );
   if ( flag )
   {
      setCreateHistogramFlag( true ); // Turn on histogram building.
   }
}

bool ossimImageUtil::rebuildHistogram() const
{
   return keyIsTrue( REBUILD_HISTOGRAM_KW );
}

void ossimImageUtil::setScanForMinMax( bool flag )
{
   addOption( SCAN_MIN_MAX_KW, ( flag ? TRUE_KW : FALSE_KW ) ); 
}

bool ossimImageUtil::scanForMinMax() const
{
   return keyIsTrue( SCAN_MIN_MAX_KW );
}

void ossimImageUtil::setScanForMinMaxNull( bool flag )
{
   addOption( SCAN_MIN_MAX_NULL_KW, ( flag ? TRUE_KW : FALSE_KW ) ); 
}

bool ossimImageUtil::scanForMinMaxNull() const
{
   return keyIsTrue( SCAN_MIN_MAX_NULL_KW );
}

void ossimImageUtil::setCompressionQuality( const std::string& quality )
{
   if ( quality.size() )
   {
      std::string key = WRITER_PROP_KW;
      key += ossimString::toString( getNextWriterPropIndex() ).string();
      std::string value = ossimKeywordNames::COMPRESSION_QUALITY_KW;
      value += "=";
      value += quality;
      addOption( key, value );
   }
}

void ossimImageUtil::setCompressionType(const std::string& type)
{
   if ( type.size() )
   {
      std::string key = WRITER_PROP_KW;
      key += ossimString::toString( getNextWriterPropIndex() ).string();
      std::string value = ossimKeywordNames::COMPRESSION_TYPE_KW;
      value += "=";
      value += type;
      addOption( key, value );
   }   
}

void ossimImageUtil::setCopyAllFlag( bool flag )
{
   // Add this for hasRequiredOverview method.
   std::string key   = COPY_ALL_FLAG_KW;
   std::string value = ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );

   // Add as a writer prop:
   key = WRITER_PROP_KW;
   key += ossimString::toString( getNextWriterPropIndex() ).string();
   value = COPY_ALL_FLAG_KW;
   value += "=";
   value += ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );
}

bool ossimImageUtil::getCopyAllFlag() const
{
   return keyIsTrue( COPY_ALL_FLAG_KW );
}

void ossimImageUtil::setDumpFilteredImageListFlag( bool flag )
{
   std::string key   = DUMP_FILTERED_IMAGES_KW;
   std::string value = ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );
}

bool ossimImageUtil::getDumpFilterImagesFlag() const
{
   return keyIsTrue( DUMP_FILTERED_IMAGES_KW );
}

void ossimImageUtil::setInternalOverviewsFlag( bool flag )
{
   // Add this for hasRequiredOverview method.
   std::string key   = INTERNAL_OVERVIEWS_FLAG_KW;
   std::string value = ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );

   // Add as a writer prop:
   key = WRITER_PROP_KW;
   key += ossimString::toString( getNextWriterPropIndex() ).string();
   value = INTERNAL_OVERVIEWS_FLAG_KW;
   value += "=";
   value += ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );
}

bool ossimImageUtil::getInternalOverviewsFlag() const
{
   return keyIsTrue( INTERNAL_OVERVIEWS_FLAG_KW );
}

void ossimImageUtil::setOutputFileNamesFlag( bool flag )
{
   std::string key   = OUTPUT_FILENAMES_KW;
   std::string value = ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );
}

bool ossimImageUtil::getOutputFileNamesFlag() const
{
   return keyIsTrue( OUTPUT_FILENAMES_KW );
}

void ossimImageUtil::setOverrideFilteredImagesFlag( bool flag )
{
   std::string key   = OVERRIDE_FILTERED_IMAGES_KW;
   std::string value = ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );
}

bool ossimImageUtil::getOverrideFilteredImagesFlag() const
{
   return keyIsTrue( OVERRIDE_FILTERED_IMAGES_KW );
}

void ossimImageUtil::setOutputDirectory( const std::string& directory )
{
   std::string key = OUTPUT_DIRECTORY_KW;
   addOption( key, directory );
}
   
void ossimImageUtil::setOverviewType( const std::string& type )
{
   std::string key = OVERVIEW_TYPE_KW;
   addOption( key, type );
   setCreateOverviewsFlag( true ); // Assume caller wants overviews.
}

void ossimImageUtil::getOverviewType(std::string& type) const
{
   std::string lookup = m_kwl->findKey(OVERVIEW_TYPE_KW);
   if ( lookup.size() )
   {
      type = lookup;
   }
   else
   {
      type = "ossim_tiff_box"; // default
   }
}

void ossimImageUtil::setProps(ossimPropertyInterface* pi) const
{
   if ( pi )
   {
      ossimString baseKey;
      if ( dynamic_cast<ossimImageHandler*>(pi) )
      {
         baseKey = READER_PROP_KW;
      }
      else
      {
         baseKey = WRITER_PROP_KW;
      }
      
      ossim_uint32 propCount = m_kwl->numberOf( baseKey.c_str() );
      if ( propCount )
      {
         ossim_uint32 foundProps = 0;
         ossim_uint32 index = 0;
         
         // (propCount+100) is to allow for holes like reader_prop0, reader_prop2...
         while ( (foundProps < propCount) && (index < (propCount+100) ) ) 
         {
            ossimString key = baseKey;
            key += ossimString::toString(index);
            std::string lookup = m_kwl->findKey( key.string() );
            if ( lookup.size() )
            {
               ossimString value = lookup;
               std::vector<ossimString> v = value.split("=");
               if (v.size() == 2)
               {
                  ossimString propertyName  = v[0];
                  ossimString propertyValue = v[1];
                  ossimRefPtr<ossimProperty> p =
                     new ossimStringProperty(propertyName, propertyValue);
                  pi->setProperty( p );
               }
               ++foundProps;
            }
            ++index;
         }
      }
   }
}

void ossimImageUtil::setOverviewStopDimension( ossim_uint32 dimension )
{
   addOption( OVERVIEW_STOP_DIM_KW, dimension );
}

void ossimImageUtil::setOverviewStopDimension( const std::string& dimension )
{
   addOption( OVERVIEW_STOP_DIM_KW, dimension );
}

void ossimImageUtil::setTileSize( ossim_uint32 tileSize )
{
   if ((tileSize % 16) == 0)
   {
      addOption( TILE_SIZE_KW, tileSize );
   }
   else
   {
      ossimNotify(ossimNotifyLevel_NOTICE)
         << "ossimImageUtil::setTileSize NOTICE:"
         << "\nTile width must be a multiple of 16!"
         << std::endl;
   }
}


ossim_uint32 ossimImageUtil::getOverviewStopDimension() const
{
   ossim_uint32 result = 0;
   std::string lookup = m_kwl->findKey( OVERVIEW_STOP_DIM_KW );
   if ( lookup.size() )
   {
      result = ossimString(lookup).toUInt32();
   }
   return result;
}

void ossimImageUtil::setCreateHistogramFlag( bool flag )
{
   addOption( CREATE_HISTOGRAM_KW, ( flag ? TRUE_KW : FALSE_KW ) );
}

bool ossimImageUtil::createHistogram() const
{
   return keyIsTrue( CREATE_HISTOGRAM_KW );
}

void ossimImageUtil::setCreateHistogramFastFlag( bool flag )
{
   addOption( CREATE_HISTOGRAM_FAST_KW, ( flag ? TRUE_KW : FALSE_KW ) );
}

bool ossimImageUtil::createHistogramFast() const
{
   return keyIsTrue( CREATE_HISTOGRAM_FAST_KW );
}

void ossimImageUtil::setCreateHistogramR0Flag( bool flag )
{
   addOption( CREATE_HISTOGRAM_R0_KW, ( flag ? TRUE_KW : FALSE_KW ) );
}

bool ossimImageUtil::createHistogramR0() const
{
   return keyIsTrue( CREATE_HISTOGRAM_R0_KW );
}

bool ossimImageUtil::hasHistogramOption() const
{
   return ( createHistogram() || createHistogramFast() || createHistogramR0() );
}

bool ossimImageUtil::hasCmmOption() const
{
   return ( keyIsTrue( CMM_MAX_KW ) || keyIsTrue( CMM_MIN_KW )|| keyIsTrue( CMM_NULL_KW ) );
}

ossimHistogramMode ossimImageUtil::getHistogramMode() const
{
   ossimHistogramMode result = OSSIM_HISTO_MODE_UNKNOWN;
   if ( createHistogram() || createHistogramR0() )
   {
      result = OSSIM_HISTO_MODE_NORMAL;
   }
   else if ( createHistogramFast() )
   {
      result = OSSIM_HISTO_MODE_FAST;
   }
   return result;
}

void ossimImageUtil::setNumberOfThreads( ossim_uint32 threads )
{
   addOption( THREADS_KW, threads );
}

void ossimImageUtil::setNumberOfThreads( const std::string& threads )
{
   addOption( THREADS_KW, threads );
}

ossim_uint32 ossimImageUtil::getNumberOfThreads() const
{
   ossim_uint32 result;
   std::string lookup = m_kwl->findKey( THREADS_KW );
   if ( lookup.size() )
   {
      result = ossimString(lookup).toUInt32();
   }
   else
   {
      result = ossim::getNumberOfThreads();
   }
   return result;
}

ossim_uint32 ossimImageUtil::getNextWriterPropIndex() const
{
   ossim_uint32 result = m_kwl->numberOf( WRITER_PROP_KW.c_str() );
   if ( result )
   {
      ossim_uint32 foundProps = 0;
      ossim_uint32 index = 0;

      //---
      // Loop until we find the last index used for WRITER_PROP_KW.
      // (result+100) is to allow for holes like writer_prop0, writer_prop2...
      //---
      while ( (foundProps < result) && (index < (result+100) ) ) 
      {
         ossimString key = WRITER_PROP_KW;
         key += ossimString::toString(index);
         std::string lookup = m_kwl->findKey( key.string() );
         if ( lookup.size() )
         {
            ++foundProps;
         }
         ++index;
      }
      result = index;
   }
   return result;
}

ossim_uint32 ossimImageUtil::getNextReaderPropIndex() const
{
   ossim_uint32 result = m_kwl->numberOf( READER_PROP_KW.c_str() );
   if ( result )
   {
      ossim_uint32 foundProps = 0;
      ossim_uint32 index = 0;
      
      //---
      // Loop until we find the last index used for WRITER_PROP_KW.
      // (result+100) is to allow for holes like reader_prop0, reader_prop2...
      //---
      while ( (foundProps < result) && (index < (result+100) ) ) 
      {
         ossimString key = READER_PROP_KW;
         key += ossimString::toString(index);
         std::string lookup = m_kwl->findKey( key.string() );
         if ( lookup.size() )
         {
            ++foundProps;
         }
         ++index;
      }
      result = index;
   }
   return result;
}

void ossimImageUtil::addOption( const std::string& key, ossim_uint32 value )
{
   addOption( key, ossimString::toString( value ).string() );
}

void ossimImageUtil::addOption(  const std::string& key, const std::string& value )
{
   m_mutex.lock();
   if ( m_kwl.valid() )
   {
      if ( key.size() && value.size() )
      {
         m_kwl->addPair( key, value );
      }
   }
   m_mutex.unlock();
}

void ossimImageUtil::setErrorStatus( ossim_int32 status )
{
   m_mutex.lock();
   m_errorStatus = status;
   m_mutex.unlock();
}

bool ossimImageUtil::isFiltered(const ossimFilename& file) const
{
   bool result = false;
   if ( file.size() )
   {
      // Strip full path to base name.
      std::string baseName = file.file().string();
      if ( baseName.size() )
      {
         std::vector<std::string>::const_iterator i = m_filteredImages.begin();
         while ( i != m_filteredImages.end() )
         {
            if ( baseName == (*i) )
            {
               result = true;
               break;
            }
            ++i;
         }
      }
   }
#if 0 /* Please leave for debug. (drb) */
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimFileWalker::isFiltered file " << (result?"filtered: ":"not filtered: ")
         << file << "\n";
   }
#endif
   
   return result;
}

bool ossimImageUtil::keyIsTrue( const std::string& key ) const
{
   bool result = false;
   if ( m_kwl.valid() )
   {
      std::string value = m_kwl->findKey( key );
      if ( value.size() )
      {
         result = ossimString(value).toBool();
      }
   }
   return result;
}

const std::vector<std::string>& ossimImageUtil::getFilteredImages() const
{
   return m_filteredImages;
}

std::vector<std::string>& ossimImageUtil::getFilteredImages()
{
   return m_filteredImages;
}

void ossimImageUtil::initializeDefaultFilterList()
{
   m_mutex.lock();

   // Common images to filter out, put most common first.
   m_filteredImages.push_back(std::string("icon.jpg"));
   m_filteredImages.push_back(std::string("logo.jpg"));
   m_filteredImages.push_back(std::string("preview.jpg"));
   
   m_mutex.unlock();
}

void ossimImageUtil::dumpFilteredImageList() const
{
   ossimNotify(ossimNotifyLevel_NOTICE) << "Filtered image list:\n";

   std::vector<std::string>::const_iterator i = m_filteredImages.begin();
   while ( i != m_filteredImages.end() )
   {
      ossimNotify(ossimNotifyLevel_NOTICE) << (*i) << "\n";
      ++i;
   }

   ossimNotify(ossimNotifyLevel_NOTICE) << std::endl;
}

void ossimImageUtil::executePrepCommands() const
{
   std::string prefix = "prep.";
   ossimFilename file = "";
   executeCommands( prefix, file );
}

void ossimImageUtil::executeFileCommands( const ossimFilename& file ) const
{
   std::string prefix = "file.";
   executeCommands( prefix, file ); 
}

void ossimImageUtil::executePostCommands() const
{
   std::string prefix = "post.";
   ossimFilename file = "";
   executeCommands( prefix, file );   
}

void ossimImageUtil::executeCommands(
   const std::string& prefix, const ossimFilename& file  ) const
{
   const std::string BASE_KEY = "command";
   
   // Get the number of test:
   const std::string REG_EXP_STR = prefix + BASE_KEY + std::string("[0-9]+");
   const ossim_uint32 NUM_COMMANDS = m_kwl->getNumberOfSubstringKeys(REG_EXP_STR);
   if ( NUM_COMMANDS )
   {
      const ossim_uint32 MAX_INDEX = NUM_COMMANDS + 1000; // for skipage...
      ossim_uint32 index = 0;
      ossim_uint32 processedIndexes = 0;
      std::string commandKey;
      ossimString command;

      while ( processedIndexes < MAX_INDEX )
      {
         commandKey = prefix + BASE_KEY + ossimString::toString( index++ ).string();
         command.string() = m_kwl->findKey( commandKey );

         if ( command.size() )
         {
            substituteCommandString( file, prefix, commandKey, command );

            ossimNotify( ossimNotifyLevel_NOTICE )
               << "executing_command: " << command << std::endl;
            
            // Launch the command:
            int status = system( command.c_str() );

            ossimNotify( ossimNotifyLevel_NOTICE )
               << "return_status: " << status << std::endl;
            
            ++processedIndexes;

            if ( processedIndexes == NUM_COMMANDS )
            {
               break;
            }
         }
      }
   }
}

void ossimImageUtil::substituteCommandString(
   const ossimFilename& file,
   const std::string& prefix,
   const std::string& commandKey,
   ossimString& command ) const
{
   // Expand any environment vars, e.g. $(env_var_name):
   command.expandEnvironmentVariable();

   gsubDate( commandKey, command );

   if ( prefix == "file." )
   {
      substituteFileStrings( file, command );
   }
}

void ossimImageUtil::substituteFileStrings( const ossimFilename& file,
                                            ossimString& command ) const
{
   const std::string BASENAME_VARIABLE = "%{basename}";
   command.gsub( BASENAME_VARIABLE, file.file().string() );
   
   const std::string BASENAME_NO_EXT_VARIABLE = "%{basename_no_ext}";
   command.gsub( BASENAME_NO_EXT_VARIABLE, file.file().string() );

   const std::string DIRNAME_VARIABLE = "%{dirname}";
   command.gsub( DIRNAME_VARIABLE, file.path().string() );

   const std::string FILE_VARIABLE = "%{file}";
   command.gsub( FILE_VARIABLE, file.string(), true );

   const std::string FILE_NO_EXT_VARIABLE = "%{file_no_ext}";
   command.gsub( FILE_NO_EXT_VARIABLE, file.noExtension().string(), true );
}

void ossimImageUtil::gsubDate( const std::string& commandKey,
                               ossimString& command ) const
{
   // Date:
   const std::string DATE_VARIABLE = "%{date}";
   if ( command.find( DATE_VARIABLE ) )
   {
      std::string key = ".strf_time_format";
      std::string strfTimeFormat = m_kwl->findKey( commandKey, key );
      if ( strfTimeFormat.empty() )
      {
         // yyyymmdd
         strfTimeFormat = "%Y%m%d";
      }
      
      //---
      // Get the date as a string, e.g 20150411.
      // true for gmt time.
      //---
      std::string date;
      ossim::getFormattedTime( strfTimeFormat, true, date );
      
      if ( date.size() )
      {
         // Sustitute:
         command.gsub( DATE_VARIABLE, date, true );
      }
   }
}

   

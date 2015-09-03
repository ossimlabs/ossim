//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:  Command line application for copying images "image copy".
// Can be used to cut images, convert formats.  Works in image space
// (no resampler).
//
//*******************************************************************
//  $Id: ossim-pc2dem.cpp 23064 2015-01-07 03:20:21Z okramer $

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimImageTypeLut.h>
#include <ossim/imaging/ossimJpegWriter.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimRLevelFilter.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimImageWriterFactory.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimCacheTileSource.h>
#include <ossim/imaging/ossimBandSelector.h>
#include <ossim/imaging/ossimCibCadrgTileSource.h>
#include <ossim/imaging/ossimRgbToGreyFilter.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/imaging/ossimMaskFilter.h>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <exception>

static ossimTrace traceDebug("icp:main");

static void usage()
{
   ossimNotify(ossimNotifyLevel_NOTICE)
      << "\nValid output writer types:\n\n";
   ossimImageWriterFactoryRegistry::instance()->
      printImageTypeList( ossimNotify(ossimNotifyLevel_NOTICE) );
}

ossimString massageQuotedValue(const ossimString& value)
{
   char quote = '\0';
   if(*value.begin() == '"')
   {
      quote = '"';
   }
   else if(*value.begin() == '\'')
   {
      quote = '\'';
   }
   
   if(quote == '\0')
   {
      return value;
   }
   std::vector<ossimString> splitString;
   value.split(splitString, quote);
   if(splitString.size() == 3)
   {
      return splitString[1];
   }
   return value;
}

//*************************************************************************************************
// FINALIZE -- Convenient location for placing debug breakpoint for catching program exit.
//*************************************************************************************************
void finalize(int code)
{
   exit (code);
}

int main(int argc, char* argv[])
{
   static const char MODULE[] = "icp:main";
   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   ossimApplicationUsage* au = ap.getApplicationUsage();
   
   au->setApplicationName(ap.getApplicationName());
   au->setDescription(ap.getApplicationName()+
      " copies any supported input image format to any supported output image format format");
   au->setCommandLineUsage(ap.getApplicationName()+
      " [options] <output_type> <input_file> <output_file>");
   au->addCommandLineOption("-h or --help",
      "Display this information");
   au->addCommandLineOption("-a or --use-scalar-remapper", 
      "Uses scalar remapper, transforms to 8-bit");
   au->addCommandLineOption("-o or --create-overview", 
      "Creates and overview for the output image");
   au->addCommandLineOption("-b or --bands <n,n...>", 
      "uses the specified bands: ex. \"1, 2, 4\" will select bands 1 2 and 4 of the input image.  "
      "Note: it is 1 based");
   au->addCommandLineOption("-c or --compression-type <type>", 
      "Uses compression.  Currently valid for only tiff output -c jpeg will use jpeg compression");
   au->addCommandLineOption("-e or --entry <n>", 
      "For multi image handlers which entry do you wish to extract");
   au->addCommandLineOption("-g", "Convert to grey scale.");
   au->addCommandLineOption("-q or --compression-quality <n>", 
      "Uses compression.  Valid for jpeg type. default is 75 where 100 is best and 1 is worst");
   au->addCommandLineOption("--pixel-type <type>", 
      "Valid values: area or point, this will determine if the tie point is upper left corner of "
      "the upper left pixel (area) or the center of the upper left corner (point), default=point. "
      "NOTE: This option will only affect the tiff writer.");
   au->addCommandLineOption("-r or --res-level <n>", 
      "Which res level to extract from the input: ex -r 1 will get res level 1");
   au->addCommandLineOption("-l or --start-line <n>", 
      "Which start line do you wish to copy from the input. If none is given then 0 is used");
   au->addCommandLineOption("-L or --end-line <n>", 
      "Which end line do you wish to copy from the input.  If none is given then max line is used");
   au->addCommandLineOption("-s or --start-sample <n>", 
      "Which start sample do you wish to copy from the input.  If none is given then 0 is used");
   au->addCommandLineOption("-p or --end-sample <n>", 
      "The end sample you wish to copy from the input. If none is given then max sample is used");
   au->addCommandLineOption("-t or --create-thumbnail <n>", 
      "Takes an argument which is the maximum pixel dimension desired.");
   au->addCommandLineOption("-w or --tile-width <n>", 
      "Defines the tile width for the handlers that support tiled output");

   au->addCommandLineOption("--reader-prop <string>", 
      "Adds a property to send to the reader. format is name=value");

   au->addCommandLineOption("--writer-prop <string>", 
      "Adds a property to send to the writer. format is name=value");

   au->addCommandLineOption("--filter-spec <fname>", 
      "This is an external file spec that describes a chain for filtering the input image.");
   au->addCommandLineOption("--use-mask [<fname>]", 
      "Optionally specify name of mask file to use for masking the input image. If no filename "
      "given, then the default mask filename is used.");
   
   
   if (traceDebug()) CLOG << " Entered..." << std::endl;
   
   // Keyword list to initialize image writers with.
   ossimKeywordlist kwl;
   const char* PREFIX = "imagewriter.";

   bool        lineEndIsSpecified       = false;
   bool        sampEndIsSpecified       = false;
   bool        lineStartIsSpecified     = false;
   bool        sampStartIsSpecified     = false;
   bool        convert_to_greyscale     = false;
   bool        create_overview          = false;
   bool        create_thumbnail         = false;
   bool        use_band_selector        = false;
   bool        use_scalar_remapper      = false;
   bool        use_mask                 = false;
   ossim_int32 tile_width               = 0;
   ossim_int32 max_thumbnail_dimension  = 0;
   ossim_int32 rr_level                 = 0;
   ossim_int32 line_start               = 0;
   ossim_int32 line_stop                = 0;
   ossim_int32 sample_start             = 0;
   ossim_int32 sample_stop              = 0;
   ossim_int32 cibcadrg_entry           = 0;
   vector<ossimString> band_list(0);
   ossimFilename filterSpec, maskFile;
   std::map<ossimString, ossimString, ossimStringLtstr> readerPropertyMap;
   std::map<ossimString, ossimString, ossimStringLtstr> writerPropertyMap;

   if (ap.read("-h") ||
       ap.read("--help")||(ap.argc() < 2))
   {
      au->write(ossimNotify(ossimNotifyLevel_NOTICE));
      usage(); // for writer output types
      finalize(0);
   }

   while(ap.read("--reader-prop", stringParam))
   {
      std::vector<ossimString> nameValue;
      ossimString(tempString).split(nameValue, "=");
      if(nameValue.size() == 2)
      {
         readerPropertyMap.insert(std::make_pair(nameValue[0], massageQuotedValue(nameValue[1])));
      }
   }   
   while(ap.read("--writer-prop", stringParam))
   {
      std::vector<ossimString> nameValue;
      ossimString(tempString).split(nameValue, "=");
      if(nameValue.size() == 2)
      {
         writerPropertyMap.insert(std::make_pair(nameValue[0], massageQuotedValue(nameValue[1])));
      }
   }
   while(ap.read("-a") ||
         ap.read("--use-scalar-remapper"))
   {
      use_scalar_remapper = true;        
   }
   while(ap.read("--filter-spec",stringParam))
   {
      filterSpec = ossimFilename(tempString);        
   }
   
   while(ap.read("--use-mask") ||
         ap.read("--use-mask",stringParam) )
   {
      maskFile = ossimFilename(tempString);     
      use_mask = true;
   }
   
   while(ap.read("-o") ||
         ap.read("--create-overview"))
   {
      create_overview = true;
   }
   
   while(ap.read("-b", stringParam) ||
         ap.read("--bands", stringParam))
   {
      use_band_selector = true;
      ossimString s = tempString;
      band_list = s.split(",");
   }
   
   while(ap.read("-c", stringParam) ||
         ap.read("--compression-type", stringParam))
   {
      ossimString s = tempString;
      s.downcase();
      kwl.add(PREFIX, ossimKeywordNames::COMPRESSION_TYPE_KW, s.c_str(), true);
   }
   
   while(ap.read("-e", stringParam) ||
         ap.read("--entry", stringParam))
   {
      cibcadrg_entry = ossimString(tempString).toInt();
   }

   if ( ap.read("-g") )
   {
      convert_to_greyscale = true;
   }   
   
   while(ap.read("-q", stringParam) ||
         ap.read("--compression-quality", stringParam))
   {
      // Set the jpeg compression quality level.
      kwl.add(PREFIX,
              ossimKeywordNames::COMPRESSION_QUALITY_KW,
              tempString.c_str(),
              true);
   }
   while(ap.read("-r", stringParam) ||
         ap.read("--res-level", stringParam))
   {
      rr_level = ossimString(tempString).toInt();
   }
   
   while(ap.read("-l", stringParam) ||
         ap.read("--start-line", stringParam))
   {
      lineStartIsSpecified = true;
      line_start = ossimString(tempString).toInt();
   }
   
   while(ap.read("-L", stringParam) ||
         ap.read("--end-line", stringParam))
   {
      lineEndIsSpecified = true;
      line_stop = ossimString(tempString).toInt();
   }
   while(ap.read("-s", stringParam) ||
         ap.read("--start-sample", stringParam))
   {
      sampStartIsSpecified = true;
      sample_start = ossimString(tempString).toInt();
   }
   
   while(ap.read("-p", stringParam) ||
         ap.read("--end-sample", stringParam))
   {
      sampEndIsSpecified = true;
      sample_stop = ossimString(tempString).toInt();
   }
   
   while(ap.read("-t", stringParam) ||
         ap.read("--create-thumbnail", stringParam))
   {
      create_thumbnail = true;
      max_thumbnail_dimension=ossimString(tempString).toInt();
   }
   
   while(ap.read("-w", stringParam) ||
         ap.read("-tile-width", stringParam))
   {
      tile_width = ossimString(tempString).toInt();
      if ((tile_width % 16) != 0)
      {
         ossimNotify(ossimNotifyLevel_NOTICE)
            << MODULE << " NOTICE:"
            << "\nTile width must be a multiple of 16!"
            << "\nDefaulting to 128"
            << std::endl;
         tile_width = 0;
      }
   }
   if (ap.read("--pixel-type", stringParam))
   {
      ossimString os = tempString;
      os.downcase();
      if (os.contains("area"))
      {
         kwl.add(PREFIX, ossimKeywordNames::PIXEL_TYPE_KW, "area", true);
      }
      else
      {
         kwl.add(PREFIX, ossimKeywordNames::PIXEL_TYPE_KW, "point", true);
 
      }
   }
   
   ap.reportRemainingOptionsAsUnrecognized();
   
   // Three required args:  output_type, input file, and output file.
   if (ap.errors())
   {
      ap.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
      finalize(0);
   }
   if (ap.argc() < 4)
   {
      au->write(ossimNotify(ossimNotifyLevel_NOTICE));
      usage(); // for writer output types
      finalize(0);
   }
   
   // Set the writer type and the image type.
   ossimString output_type = ap.argv()[ap.argc()-3];
   
   kwl.add(PREFIX, ossimKeywordNames::TYPE_KW, output_type.c_str(), true);

   // Get the input file.
   const char* input_file = argv[ap.argc()-2];
   
   // Get the output file.
   ossimFilename output_file = argv[ap.argc()-1];
   
   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\noutput type:  "
           << ap.argv()[ap.argc()-3]
           << "\ninput file:   "
           << ap.argv()[ap.argc()-2]
           << "\noutput file:  "
           << ap.argv()[ap.argc()-1]
           << std::endl;
      
      if (tile_width)
      {
         ossimNotify(ossimNotifyLevel_NOTICE) << "tile_width:  " << tile_width << std::endl;
      }
   }
   
   // Get an image handler for the input file.
   ossimRefPtr<ossimImageHandler> ih =
      ossimImageHandlerRegistry::instance()->open(ossimFilename(input_file));
   
   ossimCibCadrgTileSource* its = PTR_CAST(ossimCibCadrgTileSource, ih.get());
   
   if (its)
   {
      if (cibcadrg_entry > 0)
      {
         its->setCurrentEntry(cibcadrg_entry);
      }
   }
   
   if (!ih)
   {
      ossimNotify(ossimNotifyLevel_NOTICE) << "Unsupported image file:  " << input_file
           << "\nExiting application..." << std::endl;
      finalize(0);
   }
   if (ih->getErrorStatus() == ossimErrorCodes::OSSIM_ERROR)
   {
      ossimNotify(ossimNotifyLevel_FATAL)
         << "Error reading image:  " << input_file
         << "Exiting application..." << std::endl; 
      finalize(1);
   }

   // Set the reader properties if any.
   if ( readerPropertyMap.size() )
   {
      ossimPropertyInterface* pi = (ossimPropertyInterface*)ih.get();
      std::map<ossimString, ossimString, ossimStringLtstr>::iterator iter = readerPropertyMap.begin();
      while(iter != readerPropertyMap.end())
      {
         pi->setProperty(iter->first, iter->second);
         ++iter;
      }
   }
   
   ih->initialize();
   ossimRefPtr<ossimImageSource> source = ih.get();
   
   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\nImage Handler:  " << ih->getLongName()
           << std::endl;
   }

   // Start band selector section:

   //---
   // If image handler is band selector, start with all bands.
   // Some sources, e.g. ossimEnviTileSource can pick up default
   // bands and filter out all other bands.
   //---
   ih->setOutputToInputBandList();

   if ( use_band_selector && (source->getNumberOfOutputBands() > 1) )
   {
      // Build the band list.
      ossim_uint32 bands = source->getNumberOfOutputBands();
      vector<ossim_uint32> bl;
      ossim_uint32 i;
      for (i=0; i<band_list.size(); ++i)
      {
         bl.push_back(band_list[i].toULong()-1);
      }
      
      // Check the list.  Make sure all the bands are within range.
      for (i=0; i<bl.size(); ++i)
      {
         if (bl[i] >= bands)
         {
            ossimNotify(ossimNotifyLevel_FATAL)
               << MODULE << " ERROR:"
               << "\nBand list range error!"
               << "\nHighest available band:  " << bands
               << std::endl;
            finalize(1);
         }
      }
      
      ossimRefPtr<ossimBandSelector> bs = new ossimBandSelector();
      bs->connectMyInputTo(source.get());
      bs->setOutputBandList(bl);
      bs->enableSource();
      bs->initialize();
      source = bs.get();
      
      if (traceDebug())
      {
         CLOG << "DEBUG:"
              << "\nZero based output band list:" << std::endl;
         for (i=0; i<bl.size(); ++i)
         {
            ossimNotify(ossimNotifyLevel_NOTICE)
               << "   band[" << i << "]:  " << bl[i] << std::endl;
         }
         ossimNotify(ossimNotifyLevel_NOTICE) << std::endl;
      }
      
   } // Matches: if ( use_band_selector...

   // End of band selection section.

   if ( convert_to_greyscale )
   {
      ossimRefPtr<ossimRgbToGreyFilter> rgb2grey = new ossimRgbToGreyFilter();
      rgb2grey->connectMyInputTo( source.get() );
      source = rgb2grey.get();
   }
   
   ossimRefPtr<ossimMaskFilter> mask_filter = 0;
   if (use_mask)
   {
      if (maskFile.empty())
      {
         maskFile = ih->getFilenameWithThisExtension("mask");
      }
      
      ossimImageHandler* mh = ossimImageHandlerRegistry::instance()->open(maskFile);
      if (mh == NULL)
      {
         ossimNotify(ossimNotifyLevel_FATAL)<<"ossim-icp -- Could not open raster mask file <"
            <<maskFile<<">. Maske request will be ignored. Aborting..."<<endl;
         finalize(1);
      }
      mask_filter = new ossimMaskFilter();
      mask_filter->connectMyInputTo(source.get());
      mask_filter->setMaskSource(mh);  // assumes ownership of mask handler object
      source = mask_filter.get();
   }

   if(!filterSpec.empty()&&filterSpec.exists())
   {
      ossimKeywordlist kwl;
      if(kwl.addFile(filterSpec))
      {
         ossimRefPtr<ossimObject> input = ossimObjectFactoryRegistry::instance()->createObject(kwl);
         if(input.valid())
         {
            ossimImageSource* inputImageSource = dynamic_cast<ossimImageSource*> (input.get());
            if(inputImageSource)
            {
               inputImageSource->connectMyInputTo(source.get());
               source = inputImageSource;
            }
         }
      }
   }
   
   if (create_thumbnail == true)
   {
      // Get the rlevel that <= max thumbnail dimension.
      int max   = 0;
      int level = 0;
      
      while (level < ((ossim_int32)ih->getNumberOfDecimationLevels()-1))
      {
         int lines   = ih->getNumberOfLines(level);
         int samples = ih->getNumberOfSamples(level);
         max = lines > samples ? lines : samples;
         if (max <= max_thumbnail_dimension)
         {
            break;
         }
         ++level;
      }
      
      if (max > max_thumbnail_dimension)
      {
         ossimNotify(ossimNotifyLevel_NOTICE) << " NOTICE:"
              << "\nLowest rlevel not small enough to fulfill "
              << "max_thumbnail_dimension requirement!" << std::endl;
      }
      
      // Set the reduced res level.  This will override the -r option.
      rr_level = level;

   } // end of "if  (create_thumbnail == true)

   // Start rlevel filter section.
   if( rr_level != 0 )
   {
      //---
      // Check for a valid reduced resolution level.
      // If the operator entered an invalid rr_level with the -r option,
      // spit out a warning and set to default "0".
      //---
      if (rr_level >= (ossim_int32)(ih->getNumberOfDecimationLevels()))
      {
         ossimNotify(ossimNotifyLevel_NOTICE)
            << " WARNING:"
            << "\n\t Selected res level greater than available res levels."
            << "\n\t Defaulting to res level zero. " << std::endl;
         rr_level = 0;
      }
         
      ossimRefPtr<ossimRLevelFilter> rlevelFilter = new ossimRLevelFilter;
      
      rlevelFilter->connectMyInputTo(source.get());
      
      source = rlevelFilter.get();
      
      rlevelFilter->setCurrentRLevel(rr_level);
      if ( rr_level )
      {
         rlevelFilter->setOverrideGeometryFlag(true);
      }
      
   } // Matches: if( rr_level != 0 )
      
   ossimIrect output_rect = source->getBoundingRect(rr_level);
      
   //---
   // If any of these are true the user wants to cut the rectangle.
   //---
   if ( lineStartIsSpecified || lineEndIsSpecified ||
        sampStartIsSpecified || sampEndIsSpecified)
   {
      if (!lineStartIsSpecified) line_start   = output_rect.ul().y;
      if (!lineEndIsSpecified)   line_stop    = output_rect.lr().y;
      if (!sampStartIsSpecified) sample_start = output_rect.ul().x;
      if (!sampEndIsSpecified)   sample_stop  = output_rect.lr().x;
      
      //---
      // Check the start and stop points and make sure they are in
      // the right order; if not, swap them.
      //---
      if (line_stop < line_start)
      {
         ossimNotify(ossimNotifyLevel_NOTICE)
            << " WARNING:\n"
            << "\t Line end is less than line start, swapping."
            << std::endl;
         int tmp    = line_start;
         line_start = line_stop;
         line_stop  = tmp;
      }
      if (sample_stop < sample_start)
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << " WARNING:\n"
            << "\t Sample end is less than sample start, swapping."
            << std::endl;
         int tmp      = sample_start;
         sample_start = sample_stop;
         sample_stop  = tmp;
      }

      output_rect.set_ulx(sample_start);
      output_rect.set_lrx(sample_stop);
      output_rect.set_uly(line_start);
      output_rect.set_lry(line_stop);
         
   } // End of "if ((lineEndIsSpecified) ||..."

   if (traceDebug())
   {
      CLOG << "icp:main debug"
           << "\nrr_level:  " << rr_level
           << "\noutput_rect:   " << output_rect
           << "\nkeyword list:  " << kwl << std::endl;
   }
   
   ossimRefPtr<ossimImageFileWriter> writer =
      ossimImageWriterFactoryRegistry::instance()->createWriter(kwl, PREFIX);

   if( writer == 0 )
   {
      ossimNotify(ossimNotifyLevel_NOTICE)
         << "\nCould not create writer of type:  "
         << output_type
         << std::endl;
      usage();
      finalize(1);
   }

   writer->connectMyInputTo(0, source.get());

   if (tile_width)
   {
      // Set the tile size...
      writer->setTileSize(ossimIpt(tile_width, tile_width));
   }
   
   writer->open(output_file);
   
   // Add a listener to get percent complete.
   ossimStdOutProgress prog(0, true);
   writer->addListener(&prog);

   if (writer->getErrorStatus() == ossimErrorCodes::OSSIM_OK)
   {
      if( (ih->getOutputScalarType() != OSSIM_UCHAR) &&
          ((PTR_CAST(ossimJpegWriter, writer.get())) ||
           use_scalar_remapper))
      {
         writer->setScaleToEightBitFlag(true);
      }

      ossimRefPtr<ossimCacheTileSource> cache = new ossimCacheTileSource;
      ossimIpt tileWidthHeight(ih->getImageTileWidth(),
                               ih->getImageTileHeight());
      // only use the cache if its stripped
      if(static_cast<ossim_uint32>(tileWidthHeight.x) ==
         ih->getBoundingRect().width())
      {
         cache->connectMyInputTo(0, source.get());
         cache->setTileSize(tileWidthHeight);
         writer->connectMyInputTo(0, cache.get());
      }
      else
      {
         writer->connectMyInputTo(0, source.get());
      }
      writer->initialize();
      writer->setAreaOfInterest(output_rect); // Set the output rectangle.

      try
      {
         if ( writerPropertyMap.size() )
         {
            ossimPropertyInterface* propInterface = (ossimPropertyInterface*)writer.get();
            std::map<ossimString, ossimString, ossimStringLtstr>::iterator iter = writerPropertyMap.begin();
            while(iter!=writerPropertyMap.end())
            {
               propInterface->setProperty(iter->first, iter->second);
               ++iter;
            }
         }
         writer->execute();
      }
      catch(std::exception& e)
      {
         ossimNotify(ossimNotifyLevel_FATAL)
            << "icp:main ERROR:\n"
            << "Caught exception!\n"
            << e.what()
            << std::endl;
      }
      catch(...)
      {
         ossimNotify(ossimNotifyLevel_FATAL)
            << "icp:main ERROR:\n"
            << "Unknown exception caught!\n"
            << std::endl;
      }

      cache = 0;
   }
   else
   {
      ossimNotify(ossimNotifyLevel_FATAL)
         << "Error detected in the image writer..."
         << "\nExiting application..." << std::endl;

      finalize(1);
   }
   
   if (create_overview == true)
   {
      writer->writeOverviewFile();
   }
   
   finalize(0);
}

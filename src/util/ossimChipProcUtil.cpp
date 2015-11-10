//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGeoPolygon.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimVisitor.h>

#include <ossim/imaging/ossimBrightnessContrastSource.h>
#include <ossim/imaging/ossimBumpShadeTileSource.h>
#include <ossim/imaging/ossimFilterResampler.h>
#include <ossim/imaging/ossimFusionCombiner.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageMosaic.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/imaging/ossimImageToPlaneNormalFilter.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimSFIMFusion.h>
#include <ossim/imaging/ossimTwoColorView.h>
#include <ossim/imaging/ossimImageSourceFactoryRegistry.h>
#include <ossim/init/ossimInit.h>

#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimUtmProjection.h>

#include <ossim/support_data/ossimSrcRecord.h>
#include <ossim/util/ossimChipProcUtil.h>

#include <cmath>
#include <sstream>
#include <string>

static ossimTrace traceDebug("ossimChipProcUtil:debug");
static ossimTrace traceLog("ossimChipProcUtil:log");
static ossimTrace traceOptions("ossimChipProcUtil:options");

static const std::string BRIGHTNESS_KW           = "brightness";
static const std::string CONTRAST_KW             = "contrast";
static const std::string CLIP_WMS_BBOX_LL_KW     = "clip_wms_bbox_ll";
static const std::string CLIP_POLY_LAT_LON_KW    = "clip_poly_lat_lon";
static const std::string CUT_WMS_BBOX_KW         = "cut_wms_bbox";
static const std::string CUT_WMS_BBOX_LL_KW      = "cut_wms_bbox_ll";
static const std::string CUT_CENTER_LAT_KW       = "cut_center_lat";
static const std::string CUT_CENTER_LON_KW       = "cut_center_lon";
static const std::string CUT_RADIUS_KW           = "cut_radius";  // meters
static const std::string CUT_HEIGHT_KW           = "cut_height";  // pixels
static const std::string CUT_MAX_LAT_KW          = "cut_max_lat";
static const std::string CUT_MAX_LON_KW          = "cut_max_lon";
static const std::string CUT_MIN_LAT_KW          = "cut_min_lat";
static const std::string CUT_MIN_LON_KW          = "cut_min_lon";
static const std::string CUT_WIDTH_KW            = "cut_width";   // pixels
static const std::string HISTO_OP_KW             = "hist_op";
static const std::string IMAGE_SPACE_SCALE_X_KW  = "image_space_scale_x";
static const std::string IMAGE_SPACE_SCALE_Y_KW  = "image_space_scale_y";
static const std::string INPUT_SOURCE_KW        = "input_source";
static const std::string LUT_FILE_KW             = "lut_file";
static const std::string DEGREES_X_KW            = "degrees_x";
static const std::string DEGREES_Y_KW            = "degrees_y";
static const std::string METERS_KW               = "meters";
static const std::string NORTH_UP_KW             = "north_up"; // bool
static const std::string OUTPUT_RADIOMETRY_KW    = "output_radiometry";
static const std::string PAD_THUMBNAIL_KW        = "pad_thumbnail"; // bool
static const std::string READER_PROPERTY_KW      = "reader_property";
static const std::string RESAMPLER_FILTER_KW     = "resampler_filter";
static const std::string ROTATION_KW             = "rotation";
static const std::string SCALE_2_8_BIT_KW        = "scale_2_8_bit";
static const std::string SHARPEN_MODE_KW         = "sharpen_mode";
static const std::string SNAP_TIE_TO_ORIGIN_KW   = "snap_tie_to_origin";
static const std::string SRC_FILE_KW             = "src_file";
static const std::string SRS_KW                  = "srs";
static const std::string THREE_BAND_OUT_KW       = "three_band_out"; // bool
static const std::string THUMBNAIL_RESOLUTION_KW = "thumbnail_resolution"; // pixels
static const std::string TILE_SIZE_KW            = "tile_size"; // pixels
static const std::string TRUE_KW                 = "true";
static const std::string UP_IS_UP_KW             = "up_is_up"; // bool
static const std::string WRITER_KW               = "writer";
static const std::string WRITER_PROPERTY_KW      = "writer_property";
static const std::string COMBINER_TYPE_KW        = "combiner_type"; 

ossimChipProcUtil::ossimChipProcUtil()
{
   m_kwl.setExpandEnvVarsFlag(true);
}

ossimChipProcUtil::~ossimChipProcUtil()
{
   clear();
}

void ossimChipProcUtil::clear()
{
   // Must disonnect chains so that they destroy.
   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator i = m_srcLayers.begin();
   while ( i != m_srcLayers.end() )
   {
      (*i)->disconnect();
      (*i) = 0;
      ++i;
   }
   m_srcLayers.clear();

   if(m_writer.valid())
   {
      m_writer->disconnect();
      m_writer = 0;
   }
}

bool ossimChipProcUtil::initialize(ossimArgumentParser& ap)
{
   clear();
   if( ap.read("-h") || ap.read("--help") || (ap.argc() == 1) )
   {
      setUsage(ap);
      return false; // Indicates process should be terminated to caller.
   }

   std::string tempString1;
   ossimArgumentParser::ossimParameter stringParam1(tempString1);
   std::string tempString2;
   ossimArgumentParser::ossimParameter stringParam2(tempString2);
   std::string tempString3;
   ossimArgumentParser::ossimParameter stringParam3(tempString3);
   std::string tempString4;
   ossimArgumentParser::ossimParameter stringParam4(tempString4);
   std::string tempString5;
   ossimArgumentParser::ossimParameter stringParam5(tempString5);
   std::string tempString6;
   ossimArgumentParser::ossimParameter stringParam6(tempString6);
   double tempDouble1;
   ossimArgumentParser::ossimParameter doubleParam1(tempDouble1);
   double tempDouble2;
   ossimArgumentParser::ossimParameter doubleParam2(tempDouble2);

   ossim_uint32 srcIdx        = 0;
   ossim_uint32 readerPropIdx = 0;
   ossim_uint32 writerPropIdx = 0;
   ossimString  key           = "";

   // Extract optional arguments and stuff them in a keyword list.
   if (ap.read("-b", stringParam1) || ap.read("--bands", stringParam1))
   {
      m_kwl.addPair( std::string(ossimKeywordNames::BANDS_KW), tempString1 );
   }   

   if( ap.read("--brightness", stringParam1) )
   {
      m_kwl.addPair( BRIGHTNESS_KW, tempString1 );
   }

   if( ap.read("--central-meridian", stringParam1) )
   {
      m_kwl.addPair( std::string(ossimKeywordNames::CENTRAL_MERIDIAN_KW), tempString1 );
   }

   if( ap.read("--color-table", stringParam1) || ap.read("--lut", stringParam1) )
   {
      m_kwl.addPair( LUT_FILE_KW, tempString1 );
   }

   if( ap.read("--contrast", stringParam1) )
   {
      m_kwl.addPair( CONTRAST_KW, tempString1 );
   }

   if( ap.read("--cut-width", stringParam1) )
   {
      m_kwl.addPair( CUT_WIDTH_KW,   tempString1 );
   }
   if( ap.read("--cut-height", stringParam1) )
   {
      m_kwl.addPair( CUT_HEIGHT_KW,  tempString1 );
   }
   if( ap.read("--cut-wms-bbox", stringParam1) )
   {
      m_kwl.addPair(CUT_WMS_BBOX_KW, tempString1);
   }
   if( ap.read("--cut-wms-bbox-ll", stringParam1) )
   {
      m_kwl.addPair(CUT_WMS_BBOX_LL_KW, tempString1);
   }

   if( ap.read("--clip-wms-bbox-ll", stringParam1) )
   {
      m_kwl.addPair(CLIP_WMS_BBOX_LL_KW, tempString1);
   }

   if( ap.read("--clip-poly-lat-lon", stringParam1) )
   {
      //std::vector<ossimGpt> result;
      //ossim::toVector(result, ossimString(tempString1));
      //std::cout << result[0] << std::endl;
      //std::cout <<tempString1<<std::endl;
      //exit(0);
      m_kwl.addPair(CLIP_POLY_LAT_LON_KW, tempString1);
   }

   if( ap.read("--cut-bbox-ll", stringParam1, stringParam2, stringParam3, stringParam4) )
   {
      m_kwl.addPair( CUT_MIN_LAT_KW, tempString1 );
      m_kwl.addPair( CUT_MIN_LON_KW, tempString2 );
      m_kwl.addPair( CUT_MAX_LAT_KW, tempString3 );
      m_kwl.addPair( CUT_MAX_LON_KW, tempString4 );
   }

   if( ap.read("--cut-bbox-llwh", stringParam1, stringParam2, stringParam3,
               stringParam4, stringParam5, stringParam6) )
   {
      m_kwl.addPair( CUT_MIN_LAT_KW, tempString1 );
      m_kwl.addPair( CUT_MIN_LON_KW, tempString2 );
      m_kwl.addPair( CUT_MAX_LAT_KW, tempString3 );
      m_kwl.addPair( CUT_MAX_LON_KW, tempString4 );
      m_kwl.addPair( CUT_WIDTH_KW,   tempString5 );
      m_kwl.addPair( CUT_HEIGHT_KW,  tempString6 );
   }

   if( ap.read("--cut-center-llwh", stringParam1, stringParam2, stringParam3, stringParam4) )
   {
      m_kwl.addPair( CUT_CENTER_LAT_KW, tempString1 );
      m_kwl.addPair( CUT_CENTER_LON_KW, tempString2 );
      m_kwl.addPair( CUT_WIDTH_KW,      tempString3 );
      m_kwl.addPair( CUT_HEIGHT_KW,     tempString4 );
   }

   if( ap.read("--cut-center-llr", stringParam1, stringParam2, stringParam3) )
   {
      m_kwl.addPair( CUT_CENTER_LAT_KW, tempString1 );
      m_kwl.addPair( CUT_CENTER_LON_KW, tempString2 );
      m_kwl.addPair( CUT_RADIUS_KW,     tempString3 );
   }

   int num_params = ap.numberOfParams("--degrees", doubleParam1);
   if (num_params == 1)
   {
      ap.read("--degrees", doubleParam1);
      m_kwl.add( DEGREES_X_KW.c_str(), tempDouble1 );
      m_kwl.add( DEGREES_Y_KW.c_str(), tempDouble1 );
   }
   else if (num_params == 2)
   {
      ap.read("--degrees", doubleParam1, doubleParam2);
      m_kwl.add( DEGREES_X_KW.c_str(), tempDouble1 );
      m_kwl.add( DEGREES_Y_KW.c_str(), tempDouble2 );
   }   

   if ( ap.read("-e", stringParam1) || ap.read("--entry", stringParam1) )
   {
      m_kwl.addPair( std::string(ossimKeywordNames::ENTRY_KW), tempString1 );
   }

   if ( ap.read("--hemisphere", stringParam1) )
   {
      m_kwl.addPair( std::string(ossimKeywordNames::HEMISPHERE_KW), tempString1 );
   }

   if ( ap.read("--histogram-op", stringParam1) )
   {
      m_kwl.addPair( HISTO_OP_KW, tempString1 );
   }

   if ( ap.read("--image-space-scale", doubleParam1, doubleParam2) )
   {
      m_kwl.add( IMAGE_SPACE_SCALE_X_KW.c_str(), tempDouble1 );
      m_kwl.add( IMAGE_SPACE_SCALE_Y_KW.c_str(), tempDouble2 );
   }

   int idx = 0;
   while( ap.read("--input-source", stringParam1) )
   {
      key = INPUT_SOURCE_KW;
      key += ossimString::toString(idx++);
      key += ".";
      key += ossimKeywordNames::FILE_KW;
     m_kwl.addPair(key.string(), tempString1 );
   }

   idx = 0;
   while( ap.read("--src-file", stringParam1) )
   {
      key = SRC_FILE_KW;
      key += ossimString::toString(idx++);
      m_kwl.addPair(key.string(), tempString1 );
   }

   if( ap.read("--meters", stringParam1) )
   {
      m_kwl.addPair( METERS_KW, tempString1 );
   }

   if ( ap.read("-n") || ap.read("--north-up") )
   {
      m_kwl.addPair( NORTH_UP_KW, TRUE_KW);
   }

   //---
   // Deprecated: "--options-keyword-list"
   //---
   if( ap.read("--options", stringParam1) )
   {
      ossimFilename optionsKwl = tempString1;
      if ( optionsKwl.exists() )
      {
         if ( m_kwl.addFile(optionsKwl) == false )
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

   if( ap.read("--origin-latitude", stringParam1) )
   {
      m_kwl.addPair( std::string(ossimKeywordNames::ORIGIN_LATITUDE_KW), tempString1 );
   }

   if(ap.read("--output-radiometry", stringParam1))
   {
      m_kwl.addPair( OUTPUT_RADIOMETRY_KW, tempString1 );
   }

   if ( ap.read("--pad-thumbnail", stringParam1) )
   {
      m_kwl.addPair( PAD_THUMBNAIL_KW, tempString1 );
   }

   if( ap.read("--projection", stringParam1) )
   {
      m_kwl.addPair( std::string(ossimKeywordNames::PROJECTION_KW), tempString1 );
   }

   if( ap.read("--resample-filter", stringParam1) )
   {
      m_kwl.addPair( RESAMPLER_FILTER_KW, tempString1 );
   }

   if ( ap.read("-r", stringParam1) || ap.read("--rotate", stringParam1) )
   {
      m_kwl.addPair( ROTATION_KW, tempString1 );
   }

   while (ap.read("--reader-prop", stringParam1))
   {
      key = READER_PROPERTY_KW;
      key += ossimString::toString(readerPropIdx);
      m_kwl.addPair(key.string(), tempString1 );
      ++readerPropIdx;
   }

   if ( ap.read("--scale-to-8-bit") )
   {
      m_kwl.addPair( SCALE_2_8_BIT_KW, TRUE_KW);
   }

   if ( ap.read("--sharpen-mode", stringParam1) )
   {
      m_kwl.addPair( SHARPEN_MODE_KW, tempString1 );
   }

   if ( ap.read("--snap-tie-to-origin") )
   {
      m_kwl.addPair( SNAP_TIE_TO_ORIGIN_KW, TRUE_KW);
   }

   if( ap.read("--srs", stringParam1) )
   {
      ossimString os = tempString1;
      if ( os.contains("EPSG:") )
      {
         os.gsub( ossimString("EPSG:"), ossimString("") );
      }
      m_kwl.addPair( SRS_KW, os.string() );
   }

   if( ap.read("-t", stringParam1) || ap.read("--thumbnail", stringParam1) )
   {
      m_kwl.addPair( THUMBNAIL_RESOLUTION_KW, tempString1 );
   }

   if ( ap.read("--three-band-out") )
   {
      m_kwl.addPair( THREE_BAND_OUT_KW, TRUE_KW);
   }

   if( ap.read("--tile-size", stringParam1) )
   {
      m_kwl.addPair( TILE_SIZE_KW, tempString1 );
   }

   if ( ap.read("-u") || ap.read("--up-is-up") )
   {
      m_kwl.addPair( UP_IS_UP_KW, TRUE_KW);
   }

   if( ap.read("-w", stringParam1) || ap.read("--writer", stringParam1) )
   {
      m_kwl.addPair( WRITER_KW, tempString1);
   }

   while (ap.read("--writer-prop", stringParam1))
   {
      key = WRITER_PROPERTY_KW;
      key += ossimString::toString(writerPropIdx);
      m_kwl.addPair(key.string(), tempString1 );
      ++writerPropIdx;
   }

   if( ap.read("--zone", stringParam1) )
   {
      m_kwl.addPair( std::string(ossimKeywordNames::ZONE_KW), tempString1);
   }

   if(ap.read("--combiner-type", stringParam1))
   {
      m_kwl.addPair(COMBINER_TYPE_KW, tempString1);
   }

   return true;
}

bool ossimChipProcUtil::processRemainingArgs(ossimArgumentParser& ap)
{
   ossimString  key    = "";
   ossim_uint32 inputIdx = 0;

   if ( ap.argc() >= 2 )
   {
      // Output file is last arg:
      m_kwl.add( ossimKeywordNames::OUTPUT_FILE_KW, ap[ap.argc()-1]);
   }
   else
   {
      if ( !m_kwl.find(ossimKeywordNames::OUTPUT_FILE_KW) )
      {
         ap.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
         std::string errMsg = "Must supply an output file.";
         throw ossimException(errMsg);
      }
   }

   if ( ap.argc() > 2 ) // User passed inputs in front of output file.
   {
      int pos = 1; // ap.argv[0] is application name. 
      while ( pos < (ap.argc()-1) )
      {
         ossimFilename file = ap[pos];
         if ( traceDebug() )
            ossimNotify(ossimNotifyLevel_DEBUG) << "argv[" << pos << "]: " << file << "\n";

         if ( isSrcFile(file) )
         {
            if ( m_kwl.find( SRC_FILE_KW.c_str() ) )
            {
               std::string errMsg = "ossimChipProcUtil::processRemainingArgs()";
               errMsg += "ERROR Multiple src files passed in.  Please combine into one.";
               throw ossimException(errMsg);
            }
            m_kwl.addPair( SRC_FILE_KW, file.string() );
         }

         key = INPUT_SOURCE_KW;
         key += ossimString::toString(inputIdx++);
         key += ".";
         key += ossimKeywordNames::FILE_KW;
         m_kwl.addPair( key, file.string() );

         ++pos; // Go to next arg...

      } // End: while ( pos < (ap.argc()-1) )

   } // End: if ( ap.argc() > 2 )

   // End of arg parsing.
   ap.reportRemainingOptionsAsUnrecognized();
   if ( ap.errors() )
   {
      ap.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
      std::string errMsg = "Unknown option...";
      throw ossimException(errMsg);
   }

   return initialize(m_kwl);
}

bool ossimChipProcUtil::initialize( const ossimKeywordlist& kwl )
{
   clear();

   // Don't copy KWL if member KWL passed in:
   if (&kwl != &m_kwl)
   {
      // Start with clean options keyword list.
      m_kwl.clear();
      m_kwl.addList( kwl, true );
   }

   if ( traceOptions() )
   {
      ossimFilename optionsFile;
      getOutputFilename(optionsFile);
      optionsFile = optionsFile.noExtension();
      optionsFile += "-options.kwl";
      ossimString comment = " Can be use for --options argument.";
      m_kwl.write( optionsFile.c_str(), comment.c_str() );
   }

   //---
   // Populate the m_srcKwl if --src option was set.
   // Note do this before creating chains.
   //---
   initializeSrcKwl();

   // Sanity check rotation options.
   if ( upIsUp() || northUp() || hasRotation() )
   {
      std::string option;
      ossim_uint32 rotationOptionCount = 0;
      if ( upIsUp() )
      {
         option = UP_IS_UP_KW;
         ++rotationOptionCount; 
      }
      if ( northUp() )
      {
         option = NORTH_UP_KW;
         ++rotationOptionCount; 
      }
      if ( hasRotation() )
      {
         option = ROTATION_KW; 
         ++rotationOptionCount;
      }

      // Can only do ONE rotation option.
      if ( rotationOptionCount > 1 )
      {
         std::ostringstream errMsg;
         if ( upIsUp() )
         {
            errMsg << UP_IS_UP_KW << " is on.\n";
         }
         if ( northUp() )
         {
            errMsg << NORTH_UP_KW << " is on.\n";
         }
         if ( hasRotation() )
         {
            errMsg << ROTATION_KW << " is on.\n";
         }
         errMsg << "Multiple rotation options do not make sense!";
         throw ossimException( errMsg.str() );
      }

      // One input, chip operation only.
      if ( getNumberOfInputs() != 1 )
      {
         std::ostringstream errMsg;
         errMsg << option << " option takes one input.";
         throw ossimException( errMsg.str() );
      }
   }

   // Create chains for input sources.
   initSources(m_srcLayers, INPUT_SOURCE_KW);

   // Initialize projection and propagate to chains.
   initOutputProjection();

   return true;
}

void ossimChipProcUtil::appendCutRectFilter()
{
   if ( !m_procChain.valid() )
      return;

   //---
   // This is conditional.  Output radiometry may of may not be set.  This can also be set at
   // the ossimSingleImageChain level.
   //---
   if ( ( getOutputScalarType() != OSSIM_SCALAR_UNKNOWN) &&
         ( m_procChain->getOutputScalarType() != getOutputScalarType() ) )
   {
      addScalarRemapper( m_procChain, getOutputScalarType() );
   }

   assignAoiViewRect();

   //---
   // Add a cut filter. This will:
   // 1) Null out/clip any data pulled in.
   // 2) Speed up by not propagating get tile request outside the cut or "aoi"
   //    to the left hand side(input).
   //---
   m_cutRectFilter = new ossimRectangleCutFilter();

   // Set the cut rectangle:
   m_cutRectFilter->setRectangle( m_aoiViewRect );

   // Null outside.
   m_cutRectFilter->setCutType( ossimRectangleCutFilter::OSSIM_RECTANGLE_NULL_OUTSIDE );

   // Connect chipper input to source chain.
   m_cutRectFilter->connectMyInputTo( 0, m_procChain.get() );
   m_procChain = m_cutRectFilter.get();

   //---
   // Set the image size here.  Note must be set after combineLayers.  This is needed for
   // the ossimImageGeometry::worldToLocal call for a geographic projection to handle wrapping
   // accross the date line.
   //---
   m_geom->setImageSize( m_aoiViewRect.size() );

   if ( hasThumbnailResolution() )
   {
      //---
      // Adjust the projection scale and get the new rect.
      // Note this will resize the ossimImageGeometry::m_imageSize is scale changes.
      //---
      initializeThumbnailProjection();

      // Reset the source bounding rect if it changed.
      m_procChain->initialize();
   }
}

void ossimChipProcUtil::initOutputProjection()
{
   if ( isIdentity() )
   {
      createIdentityProjection();
   }
   else
   {
      // Create the output projection.
      createOutputProjection();

      // Setup the view in all the chains.
      propagateOutputProjectionToChains();
   }
}

bool ossimChipProcUtil::execute()
{
   if ( !m_procChain.valid() )
      return false;

   // This mode of operation requires a cut-rect filter to be appended with desired AOI:
   if (!m_cutRectFilter.valid())
      appendCutRectFilter();

   if (m_geom->getImageSize().hasNans())
      return 0;

   // Set up the writer.
   m_writer = createNewWriter();

   // Connect the writer to the processing chain.
   m_writer->connectMyInputTo(0, m_procChain.get());

   //---
   // Set the area of interest.
   // NOTE: This must be called after the writer->connectMyInputTo as
   // ossimImageFileWriter::initialize incorrectly resets theAreaOfInterest
   // back to the bounding rect.
   //---
   m_writer->setAreaOfInterest(m_aoiViewRect);

   if (m_writer->getErrorStatus() == ossimErrorCodes::OSSIM_OK)
   {
      // Add a listener to get percent complete.
      ossimStdOutProgress prog(0, true);
      m_writer->addListener(&prog);

      if ( traceLog() )
      {
         ossimKeywordlist logKwl;
         m_writer->saveStateOfAllInputs(logKwl);

         ossimFilename logFile;
         getOutputFilename(logFile);
         logFile.setExtension("log");

         logKwl.write( logFile.c_str() );
      }

      // Write the file:
      m_writer->execute();

      m_writer->removeListener(&prog);

      if(m_writer->isAborted())
      {
         throw ossimException( "Writer Process aborted!" );
      }
   }
   else
   {
      throw ossimException( "Unable to initialize writer for execution" );
   }

   return true;
}

void ossimChipProcUtil::abort()
{
   if(m_writer.valid())
   {
      m_writer->abort();
   }
}

ossimRefPtr<ossimImageData> ossimChipProcUtil::getChip(const ossimGrect& bounding_grect)
{
   // Set the new cut rectangle:
   assignAoiViewRect(bounding_grect);
   return getChip(m_aoiViewRect);
}

ossimRefPtr<ossimImageData> ossimChipProcUtil::getChip(const ossimIrect& bounding_irect)
{
   ossimRefPtr<ossimImageData> result = 0;
   if(!m_procChain.valid())
      return result;

   m_aoiViewRect = bounding_irect;

   // There should not be a chipper filter in the chain for getChip() calls since the chipping
   // is implied by the requested rect to getTile(), but if present, make sure it covers
   // the requested AOI to avoid masking out the tile.
   if (m_cutRectFilter.valid())
   {
      m_cutRectFilter->setRectangle( m_aoiViewRect );
      m_geom->setImageSize( m_aoiViewRect.size() );
   }

   result = m_procChain->getTile( m_aoiViewRect, 0 );
   return result;
}

void ossimChipProcUtil::initSources(std::vector< ossimRefPtr<ossimSingleImageChain> >& layers,
                                    const ossimString& keyword) const
{
   ossim_uint32 imgCount = m_kwl.numberOf( keyword.c_str() );
   ossim_uint32 maxIndex = imgCount + 100; // Allow for skippage in numbering.
   ossim_uint32 foundRecords = 0;
   ossim_uint32 i = 0;
   while ( foundRecords < imgCount )
   {
      ossimString key = keyword;
      key += ossimString::toString(i);
      key += ".";
      key += ossimKeywordNames::FILE_KW;
      ossimFilename f = m_kwl.findKey( key.string() );
      if ( f.size() )
      {
         // Look for the entry key, e.g. image0.entry: 10
         ossim_uint32 entryIndex = 0;
         key = keyword;
         key += ossimString::toString(i);
         key += ".";
         key += ossimKeywordNames::ENTRY_KW;
         std::string value = m_kwl.findKey( key.string() );
         if ( value.size() )
            entryIndex = ossimString(value).toUInt32();
         else
            entryIndex = getEntryNumber(); // Get global entry.  Set by "-e" on command line apps.

         // Add it:
         ossimRefPtr<ossimSingleImageChain> source = createChain(f, entryIndex );
         layers.push_back(source);
         ++foundRecords;
      }
      ++i;
      if ( i >= maxIndex )
         break;
   }

   if ( m_srcKwl.getSize() )
   {
      // Add stuff from src keyword list.
      imgCount = m_srcKwl.numberOf( keyword.c_str() );
      maxIndex = imgCount + 100;
      foundRecords = 0;
      i = 0;
      while ( foundRecords < imgCount )
      {
         ossimString prefix = keyword;
         prefix += ossimString::toString(i);
         prefix += ".";
         ossimSrcRecord src;
         if ( src.loadState( m_srcKwl, prefix ) )
         {
            ossimRefPtr<ossimSingleImageChain> source = createChain(src);
            layers.push_back(source);
            ++foundRecords;
         }
         ++i;
         if ( i >= maxIndex )
            break;
      }
   }
}

ossimRefPtr<ossimSingleImageChain>
ossimChipProcUtil::createChain(const ossimFilename& file, ossim_uint32 entryIndex) const
{
   ossimRefPtr<ossimSingleImageChain> ic = new ossimSingleImageChain;

   if (!file.isReadable() || !ic->open( file ))
   {
      std::string errMsg = "Could not open: ";
      errMsg += file.string();
      throw ossimException(errMsg);
   }

   // Set any reader props:
   setReaderProps( ic->getImageHandler().get() );

   // we can't guarantee the state of the image handler at this point so
   // let's make sure that the entry is always set to the requested location
   //  On Cib/Cadrg we were having problems.  Removed the compare for entry 0
   if ( setChainEntry( ic, entryIndex ) == false )
   {
      std::ostringstream errMsg;
      errMsg << " ERROR:\nEntry " << entryIndex << " out of range!" << std::endl;
      throw ossimException( errMsg.str() );
   }

   return ic;
}

ossimRefPtr<ossimSingleImageChain> ossimChipProcUtil::createChain(const ossimSrcRecord& rec) const
{
   ossimRefPtr<ossimSingleImageChain> ic = new ossimSingleImageChain;
   if ( !ic->open(rec) )
   {
      std::string errMsg = "Could not open from src record!";
      throw ossimException(errMsg);
   }

   return ic;
}


void ossimChipProcUtil::createOutputProjection()
{
   static const char* MODULE = "ossimChipProcUtil::createOutputProjection()";
   std::string op  = m_kwl.findKey( std::string(ossimKeywordNames::PROJECTION_KW) );
   std::string srs = m_kwl.findKey( SRS_KW );

   if ( op.size() && srs.size() )
   {
      ossimNotify(ossimNotifyLevel_WARN)
                           << MODULE << " WARNING:"
                           << "\nBoth " << SRS_KW << " and " << ossimKeywordNames::PROJECTION_KW
                           << " keywords are set!"
                           << "\nsrs:               " << srs
                           << "\noutput_projection: " << op
                           << "\nTaking " << srs << " over " << op << "\n";
   }

   bool usingInput = false;
   OutputProjectionType projType = getOutputProjectionType();
   ossimRefPtr<ossimMapProjection> proj = 0;

   // If an srs code use that first.
   if ( srs.size() )
   {
      proj = getNewProjectionFromSrsCode( srs );
   }
   else if ( op.size() )
   {
      switch ( projType )
      {
      case ossimChipProcUtil::GEO_PROJ:
      {
         proj = getNewGeoProjection();
         break;
      }
      case ossimChipProcUtil::GEO_SCALED_PROJ:
      {
         proj = getNewGeoScaledProjection();
         break;
      }
      case ossimChipProcUtil::INPUT_PROJ:
      {
         proj = getFirstInputProjection();
         usingInput = true;
         break;
      }
      case ossimChipProcUtil::UTM_PROJ:
      {
         proj = getNewUtmProjection();
         break;
      }
      default:
      {
         break; // Just for un-handled type warning.
      }
      }
   }

   // Check for identity projection:
   ossimRefPtr<ossimMapProjection> inputProj = getFirstInputProjection();   
   if ( proj.valid() && inputProj.valid() )
   {
      if ( *(inputProj.get()) == *(proj.get()) )
      {
         if ( projType == GEO_SCALED_PROJ )
         {
            // Get the origin used for scaling. 
            ossimGpt origin = proj->getOrigin();

            // Copy the input projection to our projection.  Has the tie and scale we need.
            proj = inputProj;

            // Set the origin for scaling.
            proj->setOrigin(origin);
         }
         else
         {
            proj = inputProj;
         }
         usingInput = true;
      }
   }

   if ( !proj.valid() )
   {
      // Try first input. If map projected use that.
      if ( inputProj.valid() )
      {
         proj = inputProj;
         usingInput = true;
         if ( traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_WARN)
                                 << "WARNING: No projection set!"
                                 << "\nDefaulting to first input's projection.\n";
         }
      }
      else
      {
         proj = getNewGeoScaledProjection();
         if ( traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_WARN)
                                 << "WARNING: No projection set!"
                                 << "\nDefaulting to scaled geographic at scene center.\n";
         }
      }
   }

   // Create our ossimImageGeometry with projection (no transform).
   m_geom  = new ossimImageGeometry( 0, proj.get() );

   //---
   // If the input is the same as output projection do not modify; else, set
   // the gsd to user selected "METERS_KW" or the best resolution of the inputs,
   // set the tie and then snap it to the projection origin.
   //---
   if ( !usingInput || hasScaleOption() )
   {
      // Set the scale.
      initializeProjectionGsd();
   }

   // Set the tie.
   intiailizeProjectionTiePoint();

   if ( snapTieToOrigin() )
   {
      // Adjust the projection tie to the origin.
      proj->snapTiePointToOrigin();
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
                                 << "using input projection: " << (usingInput?"true":"false")
                                 << "\noutput image geometry:\n";
      m_geom->print(ossimNotify(ossimNotifyLevel_DEBUG));
   }

} // End: ossimChipProcUtil::createOutputProjection()

void ossimChipProcUtil::createIdentityProjection()
{
   // Get the singe image chain.  Sould be only one.
   ossimRefPtr<ossimSingleImageChain> sic = 0;
   if ( m_srcLayers.size() )
      sic = m_srcLayers[0];

   if ( !sic.valid() )
      return;

   // Get the image handler.
   ossimRefPtr<ossimImageHandler>  ih = sic->getImageHandler();

   // Resampler:
   ossimRefPtr<ossimImageRenderer> resampler = sic->getImageRenderer();
   if ( !ih.valid() || !resampler.valid() )
      return;

   // Get the geometry from the image handler.  Since we're in "identity"
   // mode use the inputs for the outputs.
   m_geom = ih->getImageGeometry();
   if ( !m_geom.valid() )
      return;

   // Get the image projection.
   ossimRefPtr<ossimProjection> proj = m_geom->getProjection();
   if ( !proj.valid() )
      return;

   ossim_float64 rotation = 0.0;
   if ( upIsUp() )
   {
      rotation = m_geom->upIsUpAngle();
   }
   else if ( northUp() )
   {
      rotation = m_geom->northUpAngle();
   }
   else if ( hasRotation() )
   {
      rotation = getRotation();
   }

   if ( ossim::isnan( rotation ) )
   {
      rotation = 0.0;
   }

   ossimDpt imageSpaceScale;
   getImageSpaceScale( imageSpaceScale );

   ossimDrect rect;
   m_geom->getBoundingRect(rect);
   ossimDpt midPt = rect.midPoint();

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << "\nAffine transform parameters:"
                                          << "\nrotation:  " << rotation
                                          << "\nmid point: " << midPt << std::endl;
   }

   m_ivt = new ossimImageViewAffineTransform(-rotation,
                                             imageSpaceScale.x, // image space scale x
                                             imageSpaceScale.y, // image space scale y
                                             1.0,1.0,  //scale x and y
                                             0.0, 0.0, // translate x,y
                                             midPt.x, midPt.y); // pivot point

   if ( m_kwl.find( METERS_KW.c_str() ) || m_kwl.find( DEGREES_X_KW.c_str() ) )
   {
      // Set the image view transform scale.
      initializeIvtScale();
   }

   resampler->setImageViewTransform( m_ivt.get() );


} // End: createIdentityProjection()

void ossimChipProcUtil::initializeIvtScale()
{
   ossimDpt scale;
   scale.makeNan();

   if ( isIdentity() && m_ivt.valid() && m_geom.valid() )
   {
      // Check for GSD spec. Degrees/pixel takes priority over meters/pixel:
      ossimString lookup;
      lookup.string() = m_kwl.findKey( DEGREES_X_KW );
      if ( lookup.size() )
      {
         ossimDpt outputDpp;
         outputDpp.makeNan();

         outputDpp.x = lookup.toFloat64();

         lookup.string() = m_kwl.findKey( DEGREES_Y_KW );
         if ( lookup.size() )
         {
            outputDpp.y = lookup.toFloat64();
         }

         if ( !outputDpp.hasNans() )
         {
            // Input degress per pixel.  Consider this a scale of 1.0.
            ossimDpt inputDpp;
            m_geom->getDegreesPerPixel( inputDpp );

            if ( !inputDpp.hasNans() )
            {
               scale.x = inputDpp.x/outputDpp.x;
               scale.y = inputDpp.y/outputDpp.y;
            }
         }
      }
      else
      {
         lookup = m_kwl.findKey( METERS_KW );
         if ( lookup.size() )
         {
            ossimDpt outputMpp;
            outputMpp.makeNan();
            outputMpp.x = lookup.toFloat64();
            outputMpp.y = outputMpp.x;

            if ( !outputMpp.hasNans() )
            {
               // Input meters per pixel.  Consider this a scale of 1.0.
               ossimDpt inputMpp;
               m_geom->getMetersPerPixel( inputMpp );

               if ( !inputMpp.hasNans() )
               {
                  scale.x = inputMpp.x/outputMpp.x;
                  scale.y = inputMpp.y/outputMpp.y;
               }
            }
         }
      }

   } // Matches: if ( isIdentity() && ... )

   if ( !scale.hasNans() )
   {
      m_ivt->scale( scale.x, scale.y );
   }
   else
   {
      std::string errMsg = "ossimChipProcUtil::initializeIvtScale failed!";
      throw ossimException(errMsg);
   }

} // End: ossimChipProcUtil::initializeIvtScale()

void ossimChipProcUtil::intiailizeProjectionTiePoint()
{
   static const char* MODULE = "ossimChipProcUtil::intiailizeProjectionTiePoint()";

   // Get the map projection from the output geometry:
   ossimRefPtr<ossimMapProjection> mapProj = getMapProjection();

   if ( !mapProj.valid() )
   {
      std::string errMsg = "m_projection is null!";
      throw( ossimException(errMsg) );
   }

   //---
   // If the output is geographic of there are sensor model inputs, get the tie
   // using the ground point.
   //---
   if ( mapProj->isGeographic() || hasSensorModelInput() )
   {
      ossimGpt tiePoint;
      tiePoint.makeNan();
      getTiePoint(tiePoint);

      if ( !tiePoint.hasNans() )
      {
         //---
         // The tie point coordinates currently reflect the UL edge of the UL pixel.
         // We'll need to shift the tie point bac from the edge to the center base on the
         // output gsd.
         //---
         ossimDpt half_pixel_shift = m_geom->getDegreesPerPixel() * 0.5;
         tiePoint.lat -= half_pixel_shift.lat;
         tiePoint.lon += half_pixel_shift.lon;
         mapProj->setUlTiePoints(tiePoint);
      }
      else
      {
         std::string errMsg = MODULE;
         errMsg += " tie point has nans!";
         throw( ossimException(errMsg) );
      }
   }
   else
   {
      //---
      // TODO: Add test for like input projections and use above geographic tie
      // code if not.
      //---
      ossimDpt tiePoint;
      tiePoint.makeNan();
      getTiePoint(tiePoint);

      if ( !tiePoint.hasNans() )
      {
         //---
         // The tie point coordinates currently reflect the UL edge of the UL pixel.
         // We'll need to shift the tie point bac from the edge to the center base on the
         // output gsd.
         //---
         ossimDpt half_pixel_shift = m_geom->getMetersPerPixel() * 0.5;
         tiePoint.y -= half_pixel_shift.y;
         tiePoint.x += half_pixel_shift.x;
         mapProj->setUlTiePoints(tiePoint);
      }
      else
      {
         std::string errMsg = "Tie point has nans!";
         throw( ossimException(errMsg) );
      }
   }

}

void ossimChipProcUtil::initializeProjectionGsd()
{
   ossimRefPtr<ossimMapProjection> mapProj = getMapProjection();
   if ( !mapProj.valid() )
   {
      std::string errMsg = " projection is null!";
      throw( ossimException(errMsg) ); 
   }

   ossimDpt gsd;
   gsd.makeNan();

   ossimString degreesX;
   ossimString degreesY;
   ossimString meters;
   degreesX.string() = m_kwl.findKey( DEGREES_X_KW );
   degreesY.string() = m_kwl.findKey( DEGREES_Y_KW );
   meters.string()   = m_kwl.findKey( METERS_KW );

   if ( hasCutBoxWidthHeight() )
   {
      // --cut-bbox-llwh Implies a scale...
      if ( degreesX.size() || degreesY.size() || meters.size() )
      {
         std::ostringstream errMsg;
         errMsg << " ERROR: Ambiguous scale keywords!\n"
               << "Do not combine meters or degrees with cut box with a width and height.\n";
         throw( ossimException( errMsg.str() ) );
      }

      ossimString cutMinLat;
      ossimString cutMinLon;
      ossimString cutMaxLat;
      ossimString cutMaxLon;
      ossimString cutWidth;
      ossimString cutHeight;
      cutMinLat.string() = m_kwl.findKey( CUT_MIN_LAT_KW );
      cutMinLon.string() = m_kwl.findKey( CUT_MIN_LON_KW );
      cutMaxLat.string() = m_kwl.findKey( CUT_MAX_LAT_KW );
      cutMaxLon.string() = m_kwl.findKey( CUT_MAX_LON_KW );
      cutWidth.string()  = m_kwl.findKey( CUT_WIDTH_KW );
      cutHeight.string() = m_kwl.findKey( CUT_HEIGHT_KW );
      if ( cutMinLat.size() && cutMinLon.size() && cutMaxLat.size() &&
            cutMaxLon.size() && cutWidth.size() && cutHeight.size() )
      {
         ossim_float64 minLat = cutMinLat.toFloat64();
         ossim_float64 minLon = cutMinLon.toFloat64();
         ossim_float64 maxLat = cutMaxLat.toFloat64();
         ossim_float64 maxLon = cutMaxLon.toFloat64();
         ossim_float64 width  = cutWidth.toFloat64();
         ossim_float64 height = cutHeight.toFloat64();
         if ( !ossim::isnan(minLat) && !ossim::isnan(minLon) && !ossim::isnan(maxLat) &&
               !ossim::isnan(maxLon) && !ossim::isnan(width) && !ossim::isnan(height) )
         {
            gsd.x = std::fabs( maxLon - minLon ) / width;
            gsd.y = std::fabs( maxLat - minLat ) / height;

            mapProj->setDecimalDegreesPerPixel(gsd);
         }
      }
   }
   else if(hasWmsBboxCutWidthHeight())
   {
      ossimString cutWidth;
      ossimString cutHeight;
      ossimString cutWmsBbox;

      cutWidth.string()   = m_kwl.findKey( CUT_WIDTH_KW );
      cutHeight.string()  = m_kwl.findKey( CUT_HEIGHT_KW );
      cutWmsBbox.string() = m_kwl.findKey( CUT_WMS_BBOX_KW );

      cutWmsBbox = cutWmsBbox.upcase().replaceAllThatMatch("BBOX:","");
      std::vector<ossimString> cutBox = cutWmsBbox.split(",");
      if(cutBox.size()==4)
      {
         ossim_float64 minx = cutBox[0].toFloat64();
         ossim_float64 miny = cutBox[1].toFloat64();
         ossim_float64 maxx = cutBox[2].toFloat64();
         ossim_float64 maxy = cutBox[3].toFloat64();
         ossim_float64 width  = cutWidth.toFloat64();
         ossim_float64 height = cutHeight.toFloat64();
         gsd.x = std::fabs( maxx - minx ) / width;
         gsd.y = std::fabs( maxy - miny ) / height;

         // bbox is in the units of the projector
         if(mapProj->isGeographic())
         {

            mapProj->setDecimalDegreesPerPixel(gsd);
         }
         else
         {
            mapProj->setMetersPerPixel(gsd);
         }
      }
      else
      {
         std::ostringstream errMsg;
         errMsg << " ERROR: cut box does not have 4 values!\n";
         throw( ossimException( errMsg.str() ) );
      }
   } 
   else
   {
      if ( meters.size() && ( degreesX.size() || degreesY.size() ) )
      {  
         std::ostringstream errMsg;
         errMsg << " ERROR: Ambiguous scale keywords!\n"
               << "Do not combine meters with degrees.\n";
         throw( ossimException( errMsg.str() ) );
      }

      if ( degreesX.size() )
      {
         // --degrees
         gsd.x = degreesX.toFloat64();

         if ( degreesY.size() )
         {
            gsd.y = degreesY.toFloat64();
         }
         if ( !gsd.hasNans() )
         {
            mapProj->setDecimalDegreesPerPixel(gsd);
         }
      }
      else if ( meters.size() )
      {
         // --meters
         gsd.x = meters.toFloat64();
         gsd.y = gsd.x;
         if ( !gsd.hasNans() )
         {
            mapProj->setMetersPerPixel(gsd);
         }
      }
   }

   if ( gsd.hasNans() )
   {
      // Get the best resolution from the inputs.
      getMetersPerPixel(gsd);

      // See if the output projection is geo-scaled; if so, make the pixels square in meters.
      if ( getOutputProjectionType() == ossimChipProcUtil::GEO_SCALED_PROJ )
      {
         // Pick the best resolution and make them both the same.
         gsd.x = ossim::min<ossim_float64>(gsd.x, gsd.y);
         gsd.y = gsd.x;
      }

      // Set to input gsd.
      mapProj->setMetersPerPixel(gsd);
   }

   if ( traceDebug() )
      ossimNotify(ossimNotifyLevel_DEBUG) << "projection gsd: " << gsd << endl;
}

void ossimChipProcUtil::getTiePoint(ossimGpt& tie)
{
   tie.lat = ossim::nan();
   tie.lon = ossim::nan();
   tie.hgt = 0.0;

   // Loop through layers.
   ossimGpt chainTiePoint;
   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chainIdx = m_srcLayers.begin();
   while ( chainIdx != m_srcLayers.end() )
   {
      getTiePoint( (*chainIdx).get(), chainTiePoint );
      if ( tie.hasNans() )
      {
         tie = chainTiePoint;
      }
      else
      {
         if ( chainTiePoint.lat > tie.lat )
         {
            tie.lat = chainTiePoint.lat;
         }
         if ( chainTiePoint.lon < tie.lon )
         {
            tie.lon = chainTiePoint.lon;
         }
      }
      ++chainIdx;
   }
}

void ossimChipProcUtil::getTiePoint(ossimSingleImageChain* chain, ossimGpt& tie)
{
   if (chain && m_geom.valid() )
   {
      //---
      // The view is not set yet in the chain so we get the tie point from the
      // image handler geometry not from the chain which will come from the
      // ossimImageRenderer.
      //---
      ossimRefPtr<ossimImageHandler> ih = chain->getImageHandler();
      if ( ih.valid() )
      {
         ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
         if ( geom.valid() )
         {
            geom->getTiePoint( tie, true );
         }


         // Set height to 0.0 even though it's not used so hasNans test works.
         tie.hgt = 0.0;

         if ( tie.hasNans() )
         {
            std::string errMsg = "\ngeom->localToWorld returned nan for chain.";
            errMsg += "\nChain: ";
            errMsg += chain->getFilename().string();
            throw ossimException(errMsg);
         }
      }
      else
      {
         std::string errMsg = "No geometry for chain: ";
         errMsg += chain->getFilename().string();
         throw ossimException(errMsg);
      }
   }
   else
   {
      std::string errMsg = " ERROR: Null chain passed to method!";
      throw ossimException(errMsg);
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
                           << "chain name: " << chain->getFilename()
                           << "\ntie point:  " << tie << endl;
   }
}

void ossimChipProcUtil::getTiePoint(ossimDpt& tie)
{
   tie.makeNan();

   // Loop through dem layers.
   ossimDpt chainTiePoint;
   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chainIdx = m_srcLayers.begin();
   while ( chainIdx != m_srcLayers.end() )
   {
      getTiePoint( (*chainIdx).get(), chainTiePoint );
      if ( tie.hasNans() )
      {
         tie = chainTiePoint;
      }
      else
      {
         if ( chainTiePoint.y > tie.y )
         {
            tie.y = chainTiePoint.y;
         }
         if ( chainTiePoint.x < tie.x )
         {
            tie.x = chainTiePoint.x;
         }
      }
      ++chainIdx;
   }
}

void ossimChipProcUtil::getTiePoint(ossimSingleImageChain* chain, ossimDpt& tie)
{
   if (!chain || !m_geom.valid() )
   {
      std::string errMsg = "ERROR: Null chain passed to method!";
      throw ossimException(errMsg);
   }

   //---
   // The view is not set yet in the chain so we get the tie point from the
   // image handler geometry not from the chain which will come from the
   // ossimImageRenderer.
   //---
   ossimRefPtr<ossimImageHandler> ih = chain->getImageHandler();
   if ( !ih.valid() )
   {
      std::string errMsg = "No geometry for chain: ";
      errMsg += chain->getFilename().string();
      throw ossimException(errMsg);
   }

   ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
   if ( geom.valid() )
   {
      geom->getTiePoint( tie, true );
   }

   if ( tie.hasNans() )
   {
      std::string errMsg = "geom->localToWorld returned nan for chain.";
      errMsg += "\nChain: ";
      errMsg += chain->getFilename().string();
      throw ossimException(errMsg);
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << "chain name: " << chain->getFilename()
           << "\ntie point:  " << tie << endl;
   }
}

void ossimChipProcUtil::getMetersPerPixel(ossimDpt& gsd)
{
   gsd.makeNan();
   ossimDpt chainGsd;

   // Loop through dem layers.
   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chainIdx = m_srcLayers.begin();
   while ( chainIdx != m_srcLayers.end() )
   {
      getMetersPerPixel( (*chainIdx).get(), chainGsd);
      if ( gsd.hasNans() || ( chainGsd.x < gsd.x ) ) 
      {
         gsd = chainGsd;
      }
      ++chainIdx;
   }
}

void ossimChipProcUtil::getMetersPerPixel(ossimSingleImageChain* chain, ossimDpt& gsd)
{
   static const char MODULE[] = "ossimChipProcUtil::getMetersPerPixel()";
   if (!chain)
   {
      std::string errMsg = MODULE;
      errMsg += " ERROR: Null chain passed to method!";
      throw ossimException(errMsg);
   }

   ossimRefPtr<ossimImageGeometry> geom = chain->getImageGeometry();
   if ( !geom.valid() )
   {
      std::string errMsg = MODULE;
      errMsg += "\nNo geometry for chain: ";
      errMsg += chain->getFilename().string();
      throw ossimException(errMsg);
   }

   geom->getMetersPerPixel( gsd );
   if ( gsd.hasNans() )
   {
      std::string errMsg = MODULE;
      errMsg += "\ngeom->getMetersPerPixel returned nan for chain.";
      errMsg += "\nChain: ";
      errMsg += chain->getFilename().string();
      throw ossimException(errMsg);
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
                           << "chain name: " << chain->getFilename()
                           << "\nmeters per pixel: " << gsd << endl;
   }
}

ossim_float64 ossimChipProcUtil::getCentralMeridian() const
{
   ossim_float64 result = ossim::nan();
   ossimString lookup = m_kwl.findKey( std::string(ossimKeywordNames::CENTRAL_MERIDIAN_KW) );
   if ( lookup.size() )
   {
      result = lookup.toFloat64();
      if ( (result < -180.0) || (result > 180.0) )
      {
         std::string errMsg = "central meridian range error!";
         errMsg += " Valid range: -180 to 180";
         throw ossimException(errMsg);
      }
   }
   return result;
}

ossim_float64 ossimChipProcUtil::getOriginLatitude() const
{
   ossim_float64 result = ossim::nan();
   ossimString lookup = m_kwl.find(ossimKeywordNames::ORIGIN_LATITUDE_KW);
   if ( lookup.size() )
   {
      result = lookup.toFloat64();
      if ( (result < -90) || (result > 90.0) )
      {
         std::string errMsg = "origin latitude range error!";
         errMsg += " Valid range: -90 to 90";
         throw ossimException(errMsg);
      }
   }
   return result;
}

void ossimChipProcUtil::getSceneCenter(ossimGpt& gpt)
{
   std::vector<ossimGpt> centerGptArray;
   ossimGpt centerGpt;

   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chainIdx = m_srcLayers.begin();
   while ( chainIdx != m_srcLayers.end() )
   {
      getSceneCenter( (*chainIdx).get(), centerGpt);
      if ( !centerGpt.hasNans() )
      {
         centerGptArray.push_back( centerGpt );
      }
      ++chainIdx;
   }

   ossim_float64 lat = 0.0;
   ossim_float64 lon = 0.0;

   std::vector<ossimGpt>::const_iterator pointIdx = centerGptArray.begin();
   while ( pointIdx != centerGptArray.end() )
   {
      lat += (*pointIdx).lat;
      lon += (*pointIdx).lon;
      ++pointIdx;
   }

   lat /= centerGptArray.size();
   lon /= centerGptArray.size();

   if ( (lat >= -90.0) && (lat <= 90.0) && (lon >= -180.0) && (lon <= 180.0) )
   {
      gpt.lat = lat;
      gpt.lon = lon;
   }
   else
   {
      std::ostringstream errMsg;
      errMsg << "Range error!\nlatitude = "
            << ossimString::toString(lat).string()
            << "\nlongitude = "
            << ossimString::toString(lon).string();
      throw ossimException( errMsg.str() );
   }
}

void ossimChipProcUtil::getSceneCenter(ossimSingleImageChain* chain, ossimGpt& gpt)
{
   if (chain)
   {
      ossimRefPtr<ossimImageGeometry> geom = chain->getImageGeometry();
      if ( geom.valid() )
      {
         ossimIrect boundingRect = chain->getBoundingRect();
         ossimDpt midPoint = boundingRect.midPoint();
         geom->localToWorld(midPoint, gpt);
         gpt.hgt = 0.0;

         if ( gpt.hasNans() )
         {
            std::string errMsg = "geom->localToWorld returned nan for chain.";
            errMsg += "\nChain: ";
            errMsg += chain->getFilename().string();
            throw ossimException(errMsg);
         }
      }
      else
      {
         std::string errMsg = "No geometry for chain: ";
         errMsg += chain->getFilename().string();
         throw ossimException(errMsg);
      }
   }
   else
   {
      std::string errMsg = "ERROR: Null chain passed to method!";
      throw ossimException(errMsg);
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
                           << "chain name: " << chain->getFilename()
                           << "\nscene center: " << gpt << endl;
   }
}

ossimRefPtr<ossimMapProjection> ossimChipProcUtil::getFirstInputProjection()
{
   ossimRefPtr<ossimImageHandler>  ih     = 0;
   ossimRefPtr<ossimMapProjection> result = 0;

   // Get the first image handler.
   if ( m_srcLayers.size() )
   {
      ih = m_srcLayers[0]->getImageHandler();
   }

   if ( ih.valid() )
   {
      // Get the geometry from the first image handler.      
      ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
      if ( geom.valid() )
      {
         // Get the image projection.
         ossimRefPtr<ossimProjection> proj = geom->getProjection();
         if ( proj.valid() )
         {
            // Cast and assign to result.
            ossimMapProjection* mapProj = PTR_CAST( ossimMapProjection, proj.get() );
            if (mapProj)
            {
               // Must duplicate in case the output projection gets modified.
               result = (ossimMapProjection*) mapProj->dup();
            }
            if ( !result.valid() && traceDebug() )
            {
               ossimNotify(ossimNotifyLevel_WARN) << "Could not cast to map projection.\n";
            }
         }
         else if ( traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_WARN) << "No projection in first chain...\n";
         }
      }
   }
   else if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_WARN) << "No image handler in first chain...\n";
   }

   if ( traceDebug() && result.valid() )
      result->print(ossimNotify(ossimNotifyLevel_DEBUG));

   return result;
}

ossimRefPtr<ossimMapProjection> ossimChipProcUtil::getNewGeoProjection()
{
   return ossimRefPtr<ossimMapProjection>(new ossimEquDistCylProjection());
}

ossimRefPtr<ossimMapProjection> ossimChipProcUtil::getNewGeoScaledProjection()
{
   // Make projection:
   ossimRefPtr<ossimMapProjection> result = getNewGeoProjection();

   // Set the origin for scaling:

   // First check for user set "central_meridian" and "origin_latitude":
   ossimGpt origin;
   origin.lat = getOriginLatitude();
   origin.lon = getCentralMeridian();
   origin.hgt = 0.0;

   if ( origin.hasNans() )
   {
      // Use the scene center from the input.
      getSceneCenter( origin );

      //---
      // Note only latitude used for scaling, origin kept at 0.0.
      // This is a fix/hack for ossimEquDistCylProjection wrapping issues.
      //---
      origin.lon = 0.0;
   }

   if ( !origin.hasNans() )
   {
      result->setOrigin(origin);
   }
   else
   {
      std::string errMsg = "ossimChipProcUtil::getNewGeoScaledProjection ERROR";
      errMsg += "\nOrigin has nans!";
      throw ossimException(errMsg);
   }

   return result;
}

ossimRefPtr<ossimMapProjection> ossimChipProcUtil::getNewProjectionFromSrsCode(
      const std::string& code)
{
   ossimRefPtr<ossimMapProjection> result = 0;

   if (code == "4326")  // Avoid factory call for this.
   {
      result = new ossimEquDistCylProjection();
   }
   else
   {
      ossimRefPtr<ossimProjection> proj = ossimProjectionFactoryRegistry::instance()->
            createProjection(code);
      if ( proj.valid() )
      {
         result = PTR_CAST( ossimMapProjection, proj.get() );
      }
   }
   return result;
}

ossimRefPtr<ossimMapProjection> ossimChipProcUtil::getNewUtmProjection()
{
   // Make projection:
   ossimRefPtr<ossimUtmProjection> utm = new ossimUtmProjection;

   // Set the zone from keyword option:
   bool setZone = false;
   ossim_int32 zone = getZone();
   if ( (zone > 0 ) && ( zone < 61 ) )
   {
      utm->setZone( zone );
      setZone = true;
   }

   // Set the hemisphere from keyword option:
   bool setHemisphere = false;
   std::string hemisphere = getHemisphere();
   if ( hemisphere.size() )
   {
      ossimString h(hemisphere);
      h.upcase();
      if ( ( h == "N" ) || ( h == "NORTH" ) )
      {
         char c = 'N';
         utm->setHemisphere( c );
         setHemisphere = true;
      }
      if ( ( h == "S" ) || ( h == "SOUTH" ) )
      {
         char c = 'S';
         utm->setHemisphere( c );
         setHemisphere = true;
      }
   }

   if ( !setZone || !setHemisphere )
   {
      // First check for user set "central_meridian" and "origin_latitude":
      ossimGpt origin;
      origin.lat = getOriginLatitude();
      origin.lon = getCentralMeridian();
      origin.hgt = 0.0;

      if ( origin.hasNans() )
      {
         // Use the scene center from the input.
         getSceneCenter( origin );
      }

      if ( !origin.hasNans() )
      {
         if ( !setZone )
         {
            utm->setZone(origin);
         }
         if ( !setHemisphere )
         {
            utm->setHemisphere(origin);
         }
      }
      else
      {
         std::string errMsg = "ossimChipProcUtil::getNewUtmProjection ERROR";
         errMsg += "\nOrigin has nans!";
         throw ossimException(errMsg);
      }
   }

   return ossimRefPtr<ossimMapProjection>(utm.get());
}

ossimRefPtr<ossimMapProjection> ossimChipProcUtil::getMapProjection()
{
   ossimRefPtr<ossimMapProjection> mp = 0;
   if ( m_geom.valid() )
   {
      mp = dynamic_cast<ossimMapProjection*>( m_geom->getProjection() );
   }
   return mp;
}

ossimRefPtr<ossimImageFileWriter> ossimChipProcUtil::createNewWriter() const
{
   ossimFilename outputFile;
   getOutputFilename(outputFile);

   if ( outputFile == ossimFilename::NIL)
   {
      std::string errMsg = "ERROR no output file name!";
      throw ossimException(errMsg);
   }

   ossimRefPtr<ossimImageFileWriter> writer = 0;

   ossimString lookup = m_kwl.findKey( WRITER_KW );
   if ( lookup.size() )
   {
      writer = ossimImageWriterFactoryRegistry::instance()->createWriter( lookup );
      if ( !writer.valid() )
      {
         std::string errMsg = "ERROR creating writer: ";
         errMsg += lookup.string();
         throw ossimException(errMsg);
      }
   }
   else // Create from output file extension.
   {
      writer = ossimImageWriterFactoryRegistry::instance()->
            createWriterFromExtension( outputFile.ext() );

      if ( !writer.valid() )
      {
         std::string errMsg = "ERROR creating writer from extension: ";
         errMsg += outputFile.ext().string();
         throw ossimException(errMsg);
      }
   }

   // Set the output name.
   writer->setFilename( outputFile );

   // Add any writer props.
   ossim_uint32 count = m_kwl.numberOf( WRITER_PROPERTY_KW.c_str() );
   for (ossim_uint32 i = 0; i < count; ++i)
   {
      ossimString key = WRITER_PROPERTY_KW;
      key += ossimString::toString(i);
      lookup = m_kwl.findKey( key.string() );
      if ( lookup.size() )
      {
         std::vector<ossimString> splitArray;
         lookup.split(splitArray, "=");
         if(splitArray.size() == 2)
         {
            ossimRefPtr<ossimProperty> prop =
                  new ossimStringProperty(splitArray[0], splitArray[1]);

            if ( traceDebug() )
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                                    << "Setting writer prop: " << splitArray[0] << "=" << splitArray[1] << "\n";
            }

            writer->setProperty( prop );
         }
      }
   }

   // Output tile size:
   lookup = m_kwl.findKey( TILE_SIZE_KW );
   if ( lookup.size() )
   {
      ossimIpt tileSize;
      tileSize.x = lookup.toInt32();
      if ( (tileSize.x % 16) == 0 )
      {
         tileSize.y = tileSize.x;
         writer->setTileSize( tileSize );
      }
      else if ( traceDebug() )
      {
         ossimNotify(ossimNotifyLevel_NOTICE)<< " NOTICE:"
               << "\nTile width must be a multiple of 16! Using default.."
               << std::endl;
      }
   }

   return writer;
}

void ossimChipProcUtil::propagateOutputProjectionToChains()
{
   // we need to make sure the outputs are refreshed so they can reset themselves
   // Needed when we are doing interactive update to the GSD and clip window
   ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent();
   ossimEventVisitor eventVisitor(refreshEvent.get());
   ossimViewInterfaceVisitor viewVisitor(m_geom.get());

   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chainIdx = m_srcLayers.begin();
   while ( chainIdx != m_srcLayers.end() )
   {
      viewVisitor.reset();
      eventVisitor.reset();
      (*chainIdx)->accept(viewVisitor);
      (*chainIdx)->accept(eventVisitor);

      ossimRefPtr<ossimImageRenderer> resampler = (*chainIdx)->getImageRenderer();
      if ( !resampler.valid() )
      {
         std::string errMsg = "Chain has no resampler!";
         throw( ossimException(errMsg) );
      }

      //resampler->setView( m_geom.get() );
      // resampler->propagateEventToOutputs(refreshEvent);
      ++chainIdx;
   }
}

void ossimChipProcUtil::addIndexToRgbLutFilter(ossimRefPtr<ossimImageSource> &source) const
{
   if ( !source.valid() )
   {
      std::string errMsg = "ERROR: Null source passed to method!";
      throw ossimException(errMsg);
   }

   ossimRefPtr<ossimIndexToRgbLutFilter> lut = new ossimIndexToRgbLutFilter();
   ossimFilename lutFile;
   lutFile.string() = m_kwl.findKey( LUT_FILE_KW );
   if ( !lutFile.exists() )
   {
      std::string errMsg = "Color table does not exists: ";
      errMsg += lutFile.string();
      throw ossimException(errMsg);
   }

   lut->setLut(lutFile);

   // Connect to dems:
   lut->connectMyInputTo( source.get() );

   // Note sure about this.  Make option maybe? (drb)
   lut->setMode(ossimIndexToRgbLutFilter::REGULAR);

   // Shift the head of the chain to point to last node:
   source = lut.get();
}

void ossimChipProcUtil::addScalarRemapper(ossimRefPtr<ossimImageSource> &source, ossimScalarType scalar) const
{
   if ( !source.valid() )
   {
      std::string errMsg = "ERROR: Null source passed to method!";
      throw ossimException(errMsg);
   }

   if ( ( scalar != OSSIM_SCALAR_UNKNOWN ) && ( source->getOutputScalarType() != scalar ) )
   {
      ossimRefPtr<ossimScalarRemapper> remapper = new ossimScalarRemapper();
      remapper->setOutputScalarType(scalar);
      remapper->connectMyInputTo( source.get() );
      source = remapper.get();
   }
}

bool ossimChipProcUtil::setupChainHistogram( ossimRefPtr<ossimSingleImageChain>& chain) const
{
   static const char MODULE[] = "ossimChipProcUtil::setupChainHistogram(chain)";

   if ( !chain.valid() )
      return false;

   ossimRefPtr<ossimHistogramRemapper> remapper = chain->getHistogramRemapper();
   if ( !remapper.valid() )
      return false;

   if ( remapper->getHistogramFile() == ossimFilename::NIL )
   {
      ossimRefPtr<ossimImageHandler> ih = chain->getImageHandler();
      if ( ih.valid() )
      {
         ossimFilename f = ih->getFilenameWithThisExtension( ossimString("his") );

         if ( f.empty() || (f.exists() == false) )
         {
            // For backward compatibility check if single entry and _e0.his
            f = ih->getFilenameWithThisExtension( ossimString("his"), true );
         }

         if ( remapper->openHistogram( f ) == false )
            return false;
      }
   }

   bool result = false;
   if ( remapper->getHistogramFile() != ossimFilename::NIL )
   {
      ossimString op = m_kwl.findKey( HISTO_OP_KW );
      if ( op.size() )
      {
         result = true;

         // Enable.
         remapper->setEnableFlag(true);

         // Set the histo mode:
         op.downcase();
         if ( op == "auto-minmax" )
         {
            remapper->setStretchMode( ossimHistogramRemapper::LINEAR_AUTO_MIN_MAX );
         }
         else if ( (op == "std-stretch-1") || (op == "std-stretch 1") )
         {
            remapper->setStretchMode( ossimHistogramRemapper::LINEAR_1STD_FROM_MEAN );
         }
         else if ( (op == "std-stretch-2") || (op == "std-stretch 2") )
         {
            remapper->setStretchMode( ossimHistogramRemapper::LINEAR_2STD_FROM_MEAN );
         }
         else if ( (op == "std-stretch-3") || (op == "std-stretch 3") )
         {
            remapper->setStretchMode( ossimHistogramRemapper::LINEAR_3STD_FROM_MEAN );
         }
         else
         {
            result = false;
            remapper->setEnableFlag(false);
         }
      }
   }
   return result;
}

bool ossimChipProcUtil::setChainEntry(
      ossimRefPtr<ossimSingleImageChain>& chain, ossim_uint32 entryIndex ) const
{
   bool result = false;
   if ( chain.valid() )
   {
      ossimRefPtr<ossimImageHandler> ih = chain->getImageHandler();
      if ( ih.valid() )
      {
         result = ih->setCurrentEntry( entryIndex );
      }
   }
   return result;
}

void ossimChipProcUtil::getOutputFilename(ossimFilename& f) const
{
   f.string() = m_kwl.findKey( std::string(ossimKeywordNames::OUTPUT_FILE_KW) );
}

void ossimChipProcUtil::assignAoiViewRect()
{
   static const char MODULE[] = "ossimChipProcUtil::getAreaOfInterest()";

   // Nan rect for starters.
   m_aoiViewRect.makeNan();
   if (!m_procChain.valid())
   {
      // Should never happer...
      std::string errMsg = MODULE;
      errMsg += " image source null!";
      throw( ossimException(errMsg) );
   }
   if ( !m_geom.valid() )
   {
      std::string errMsg = MODULE;
      errMsg += " output projection null!";
      throw( ossimException(errMsg) );
   }

   if ( m_kwl.find( CUT_CENTER_LAT_KW.c_str() ) )
   {
      // "Cut Center" with: --cut-center-llwh or --cut-center-llr:

      ossimString latStr = m_kwl.findKey( CUT_CENTER_LAT_KW );
      ossimString lonStr = m_kwl.findKey( CUT_CENTER_LON_KW );
      if ( latStr.size() && lonStr.size() )
      {
         ossimGpt centerGpt;

         //---
         // Want the height nan going into worldToLocal call so it gets picked
         // up by the elevation manager.
         //---
         centerGpt.makeNan();

         centerGpt.lat = latStr.toFloat64();
         centerGpt.lon = lonStr.toFloat64();

         if ( !centerGpt.isLatNan() && !centerGpt.isLonNan() )
         {
            // Ground "cut center" to view:
            ossimDpt centerDpt(0.0, 0.0);
            m_geom->worldToLocal(centerGpt, centerDpt);

            if ( !centerDpt.hasNans() )
            {
               if ( isIdentity() && m_ivt.valid() ) // Chipping in image space.
               {
                  // Tranform image center point to view:
                  ossimDpt ipt = centerDpt;
                  m_ivt->imageToView( ipt, centerDpt );
               }

               // --cut-center-llwh:
               ossimString widthStr  = m_kwl.findKey( CUT_WIDTH_KW );
               ossimString heightStr = m_kwl.findKey( CUT_HEIGHT_KW );
               if ( widthStr.size() && heightStr.size() )
               {
                  ossim_int32 width  = widthStr.toInt32();
                  ossim_int32 height = heightStr.toInt32();
                  if ( width && height )
                  {
                     ossimIpt ul( ossim::round<int>(centerDpt.x - (width/2)),
                                  ossim::round<int>(centerDpt.y - (height/2)) );
                     ossimIpt lr( (ul.x + width - 1), ul.y + height - 1);
                     m_aoiViewRect = ossimIrect(ul, lr);
                  }
               }
               else // --cut-center-llr:
               {
                  ossimString radiusStr = m_kwl.findKey( CUT_RADIUS_KW );
                  if ( radiusStr.size() )
                  {
                     ossim_float64 radius = radiusStr.toFloat64();
                     if ( radius )
                     {
                        ossimDpt mpp;
                        m_geom->getMetersPerPixel( mpp );

                        if ( !mpp.hasNans() )
                        {
                           ossim_float64 rx = radius/mpp.x;
                           ossim_float64 ry = radius/mpp.y;

                           ossimIpt ul( ossim::round<int>( centerDpt.x - rx ),
                                        ossim::round<int>( centerDpt.y - ry ) );
                           ossimIpt lr( ossim::round<int>( centerDpt.x + rx ),
                                        ossim::round<int>( centerDpt.y + ry ) );
                           m_aoiViewRect = ossimIrect(ul, lr);
                        }
                     }
                  }
               }
            }
         } // Matches: if ( !centerGpt.hasNans() )
      } // Matches: if ( latStr && lonStr )
   } // Matches: if ( m_kwl.find( CUT_CENTER_LAT_KW ) )

   else if ( (m_kwl.find( CUT_MAX_LAT_KW.c_str() ) || (m_kwl.find( CUT_WMS_BBOX_LL_KW.c_str() ))))
   {
      ossimString maxLat;
      ossimString maxLon;
      ossimString minLat;
      ossimString minLon;

      // --cut-bbox-ll or --cut-bbox-llwh
      if(m_kwl.find( CUT_MAX_LAT_KW.c_str() ))
      {
         maxLat = m_kwl.findKey( CUT_MAX_LAT_KW );
         maxLon = m_kwl.findKey( CUT_MAX_LON_KW );
         minLat = m_kwl.findKey( CUT_MIN_LAT_KW );
         minLon = m_kwl.findKey( CUT_MIN_LON_KW );
      }
      else
      {
         ossimString cutBbox = m_kwl.findKey( CUT_WMS_BBOX_LL_KW );

         cutBbox = cutBbox.upcase().replaceAllThatMatch("BBOX:");
         std::vector<ossimString> cutBox = cutBbox.split(",");
         if(cutBox.size() >3)
         {
            minLon = cutBox[0];
            minLat = cutBox[1];
            maxLon = cutBox[2];
            maxLat = cutBox[3];
         }
      }

      if ( maxLat.size() && maxLon.size() && minLat.size() && minLon.size() )
      {
         ossim_float64 minLatF = minLat.toFloat64();
         ossim_float64 maxLatF = maxLat.toFloat64();
         ossim_float64 minLonF = minLon.toFloat64();
         ossim_float64 maxLonF = maxLon.toFloat64();

         //---
         // Check for swap so we don't get a negative height.
         // Note no swap check for longitude as box could cross date line.
         //---
         if ( minLatF > maxLatF )
         {
            ossim_float64 tmpF = minLatF;
            minLatF = maxLatF;
            maxLatF = tmpF;
         }

         //---
         // Assume cut box is edge to edge or "Pixel Is Area". Our
         // AOI(area of interest) uses center of pixel or "Pixel Is Point"
         // so get the degrees per pixel and shift AOI to center.
         //---
         ossimDpt halfDpp;
         m_geom->getDegreesPerPixel( halfDpp );
         halfDpp = halfDpp/2.0;

         ossimGpt gpt(0.0, 0.0, 0.0);
         ossimDpt ulPt;
         ossimDpt lrPt;

         // Upper left:
         gpt.lat = maxLatF - halfDpp.y;
         gpt.lon = minLonF + halfDpp.x;
         m_geom->worldToLocal(gpt, ulPt);

         // Lower right:
         gpt.lat = minLatF + halfDpp.y;
         gpt.lon = maxLonF - halfDpp.x;
         m_geom->worldToLocal(gpt, lrPt);

         if ( isIdentity() && m_ivt.valid() )
         {
            // Chipping in image space:

            // Tranform image ul point to view:
            ossimDpt ipt = ulPt;
            m_ivt->imageToView( ipt, ulPt );

            // Tranform image lr point to view:
            ipt = lrPt;
            m_ivt->imageToView( ipt, lrPt );
         }

         m_aoiViewRect = ossimIrect( ossimIpt(ulPt), ossimIpt(lrPt) );
      }
   }
   else if ( m_kwl.find( CUT_WMS_BBOX_KW.c_str() ) )
   {
      ossimString cutBbox = m_kwl.findKey( CUT_WMS_BBOX_KW );

      cutBbox = cutBbox.upcase().replaceAllThatMatch("BBOX:");
      std::vector<ossimString> cutBox = cutBbox.split(",");
      if(cutBox.size()==4)
      {

         ossim_float64 minx=cutBox[0].toFloat64();
         ossim_float64 miny=cutBox[1].toFloat64();
         ossim_float64 maxx=cutBox[2].toFloat64();
         ossim_float64 maxy=cutBox[3].toFloat64();

         const ossimMapProjection* mapProj = m_geom->getAsMapProjection();
         if(mapProj)
         {
            std::vector<ossimDpt> pts(4);
            ossimDpt* ptsArray = &pts.front();
            if(mapProj->isGeographic())
            {
               ossimDpt halfDpp;
               m_geom->getDegreesPerPixel( halfDpp );
               halfDpp = halfDpp/2.0;

               ossimGpt gpt(0.0, 0.0, 0.0);
               ossimDpt ulPt;
               ossimDpt lrPt;

               // Upper left:
               gpt.lat = maxy - halfDpp.y;
               gpt.lon = minx + halfDpp.x;
               m_geom->worldToLocal(gpt, ptsArray[0]);
               // Upper right:
               gpt.lat = maxy - halfDpp.y;
               gpt.lon = maxx - halfDpp.x;
               m_geom->worldToLocal(gpt, ptsArray[1]);

               // Lower right:
               gpt.lat = miny + halfDpp.y;
               gpt.lon = maxx - halfDpp.x;
               m_geom->worldToLocal(gpt, ptsArray[2]);

               //Lower left
               gpt.lat = miny + halfDpp.y;
               gpt.lon = minx + halfDpp.x;
               m_geom->worldToLocal(gpt, ptsArray[3]);
               //m_geom->worldToLocal(ossimGpt(miny,minx), ptsArray[0]);
               //m_geom->worldToLocal(ossimGpt(maxy,minx), ptsArray[1]);
               //m_geom->worldToLocal(ossimGpt(maxy,maxx), ptsArray[2]);
               //m_geom->worldToLocal(ossimGpt(miny,maxx), ptsArray[3]);

            }
            else
            {
               ossimDpt halfMpp;
               ossimDpt eastingNorthing;
               m_geom->getMetersPerPixel( halfMpp );
               halfMpp = halfMpp/2.0;

               eastingNorthing.x = minx+halfMpp.x;
               eastingNorthing.y = miny+halfMpp.y;
               mapProj->eastingNorthingToLineSample(eastingNorthing, ptsArray[0]);
               eastingNorthing.x = minx+halfMpp.x;
               eastingNorthing.y = maxy-halfMpp.y;
               mapProj->eastingNorthingToLineSample(eastingNorthing, ptsArray[1]);
               eastingNorthing.x = maxx-halfMpp.x;
               eastingNorthing.y = maxy-halfMpp.y;
               mapProj->eastingNorthingToLineSample(eastingNorthing, ptsArray[2]);
               eastingNorthing.x = maxx-halfMpp.x;
               eastingNorthing.y = miny+halfMpp.y;
               mapProj->eastingNorthingToLineSample(eastingNorthing, ptsArray[3]);
            }
            m_aoiViewRect = ossimIrect(pts);
         }
      }
   }

   // If no user defined rect set to scene bounding rect.
   if ( m_aoiViewRect.hasNans() )
      m_aoiViewRect = m_procChain->getBoundingRect(0);

} // End: ossimChipProcUtil::getAreaOfInterest

void ossimChipProcUtil::assignAoiViewRect(const ossimGrect& grect)
{
   static const char MODULE[] = "ossimChipProcUtil::getAreaOfInterest()";

   // Nan rect for starters.
   m_aoiViewRect.makeNan();
   if (!m_procChain.valid())
   {
      // Should never happer...
      std::string errMsg = MODULE;
      errMsg += " image source null!";
      throw( ossimException(errMsg) );
   }
   if ( !m_geom.valid() )
   {
      std::string errMsg = MODULE;
      errMsg += " output projection null!";
      throw( ossimException(errMsg) );
   }

   // Assume cut box is edge to edge or "Pixel Is Area". Our
   // AOI(area of interest) uses center of pixel or "Pixel Is Point"
   // so get the degrees per pixel and shift AOI to center.
   ossimDpt halfDpp;
   m_geom->getDegreesPerPixel( halfDpp );
   halfDpp = halfDpp/2.0;

   ossimGpt gpt(0.0, 0.0, 0.0);
   ossimDpt ulPt;
   ossimDpt lrPt;

   // Upper left:
   gpt.lat = grect.ul().lat - halfDpp.y;
   gpt.lon = grect.ul().lon + halfDpp.x;
   m_geom->worldToLocal(gpt, ulPt);

   // Lower right:
   gpt.lat = grect.lr().lat + halfDpp.y;
   gpt.lon = grect.lr().lon - halfDpp.x;
   m_geom->worldToLocal(gpt, lrPt);

   if ( isIdentity() && m_ivt.valid() )
   {
      // Chipping in image space. Tranform image points to view:
      ossimDpt ipt = ulPt;
      m_ivt->imageToView( ipt, ulPt );
      ipt = lrPt;
      m_ivt->imageToView( ipt, lrPt );
   }

   m_aoiViewRect = ossimIrect( ossimIpt(ulPt), ossimIpt(lrPt) );

   // If no user defined rect set to scene bounding rect.
   if ( m_aoiViewRect.hasNans() )
      m_aoiViewRect = m_procChain->getBoundingRect(0);
}

void ossimChipProcUtil::initializeThumbnailProjection()
{
   static const char MODULE[] = "ossimChipProcUtil::initializeThumbnailProjection";

   if ( m_aoiViewRect.hasNans())
   {
      std::string errMsg = MODULE;
      errMsg += " passed in rect has nans!";
      throw( ossimException(errMsg) );
   }
   if (!m_geom.valid() )
   {
      std::string errMsg = MODULE;
      errMsg += " output projection null!";
      throw( ossimException(errMsg) );
   }


   //---
   // Thumbnail setup:
   //---
   ossimString thumbRes = m_kwl.findKey( THUMBNAIL_RESOLUTION_KW );
   if ( thumbRes.size() )
   {
      ossim_float64 thumbSize = thumbRes.toFloat64();
      ossim_float64 maxRectDimension =
            ossim::max( m_aoiViewRect.width(), m_aoiViewRect.height() );

      if ( maxRectDimension > thumbSize )
      {
         // Need to adjust scale:

         // Get the corners before the scale change:
         ossimGpt ulGpt;
         ossimGpt lrGpt;

         m_geom->localToWorld(ossimDpt(m_aoiViewRect.ul()), ulGpt);
         m_geom->localToWorld(ossimDpt(m_aoiViewRect.lr()), lrGpt);

         if ( isIdentity()  && m_ivt.valid() ) // Chipping in image space.)
         {
            ossim_float64 scale = thumbSize / maxRectDimension;
            if ( m_ivt->getScale().hasNans() )
            {
               m_ivt->scale( scale, scale );
            }
            else
            {
               m_ivt->scale( m_ivt->getScale().x*scale,m_ivt->getScale().y*scale );
            }
         }
         else
         {
            ossim_float64 scale = maxRectDimension / thumbSize;

            //---
            // Adjust the projection scale.  Note the "true" is to recenter
            // the tie point so it falls relative to the projection origin.
            //
            // This call also scales: ossimImageGeometry::m_imageSize
            //---
            m_geom->applyScale(ossimDpt(scale, scale), true);
         }

         // Must call to reset the ossimImageRenderer's bounding rect for each input.
         propagateOutputProjectionToChains();

         // Get the new upper left in view space.
         ossimDpt dpt;
         m_geom->worldToLocal(ulGpt, dpt);
         ossimIpt ul(dpt);

         // Get the new lower right in view space.
         m_geom->worldToLocal(lrGpt, dpt);
         ossimIpt lr(dpt);

         //---
         // Clamp to thumbnail bounds with padding if turned on.
         // Padding is optional. If padding turned on alway make square.
         //---
         ossim_int32 ts = thumbSize;
         bool pad = padThumbnail();

         if ( ( (lr.x - ul.x + 1) > ts ) || pad )
         {
            lr.x = ul.x + ts - 1;
         }
         if ( ( (lr.y - ul.y + 1) > ts ) || pad )
         {
            lr.y = ul.y + ts - 1;
         }

         m_aoiViewRect = ossimIrect(ul, lr);
      }
   }

}

bool ossimChipProcUtil::hasBandSelection() const
{
   bool result = false;
   result = m_kwl.hasKey( std::string(ossimKeywordNames::BANDS_KW) );
   return result;
}

bool ossimChipProcUtil::hasWmsBboxCutWidthHeight()const
{
   bool result = false;

   result = (m_kwl.hasKey( CUT_HEIGHT_KW ) && m_kwl.hasKey( CUT_WIDTH_KW ) &&
         (m_kwl.hasKey( CUT_WMS_BBOX_KW )|| m_kwl.hasKey(CUT_WMS_BBOX_LL_KW)));

   return result;
}

bool ossimChipProcUtil::hasCutBoxWidthHeight() const
{
   bool result = false;
   if ( m_kwl.hasKey( CUT_HEIGHT_KW ) )
   {
      if ( m_kwl.hasKey( CUT_WIDTH_KW ) )
      {
         if ( m_kwl.hasKey( CUT_MIN_LAT_KW ) )
         {
            if ( m_kwl.hasKey( CUT_MIN_LON_KW ) )
            {
               if ( m_kwl.hasKey( CUT_MAX_LAT_KW ) )
               {
                  if ( m_kwl.hasKey( CUT_MAX_LON_KW ) )
                  {
                     result = true;
                  }
               }
            }
         } // if lat and lon WMS style bbox is specified then we will behave the same as above
         else if( m_kwl.hasKey(CUT_WMS_BBOX_LL_KW))
         {
            result = true;
         }
      }
   }
   return result;
}

bool ossimChipProcUtil::hasScaleOption() const
{
   bool result = false;
   if ( m_kwl.hasKey( METERS_KW.c_str() ) ||
         m_kwl.hasKey( DEGREES_X_KW.c_str() ) ||
         hasCutBoxWidthHeight()||
         hasWmsBboxCutWidthHeight() )
   {
      result = true;
   }
   return result;
}

bool ossimChipProcUtil::isThreeBandOut() const
{
   return keyIsTrue( THREE_BAND_OUT_KW );
}

bool ossimChipProcUtil::padThumbnail() const
{
   return  keyIsTrue( PAD_THUMBNAIL_KW );
}

void ossimChipProcUtil::setReaderProps( ossimImageHandler* ih ) const
{
   if ( ih )
   {
      ossim_uint32 count = m_kwl.numberOf( READER_PROPERTY_KW.c_str() );
      for (ossim_uint32 i = 0; i < count; ++i)
      {
         ossimString key = READER_PROPERTY_KW;
         key += ossimString::toString(i);
         ossimString value = m_kwl.findKey( key.string() );
         if ( value.size() )
         {
            std::vector<ossimString> splitArray;
            value.split(splitArray, "=");
            if(splitArray.size() == 2)
            {
               ossimRefPtr<ossimProperty> prop =
                     new ossimStringProperty(splitArray[0], splitArray[1]);

               ih->setProperty( prop );
            }
         }
      }
   }
}

void ossimChipProcUtil::getBandList( std::vector<ossim_uint32>& bandList ) const
{
   bandList.clear();
   ossimString os;
   os.string() = m_kwl.findKey( std::string( ossimKeywordNames::BANDS_KW ) );
   if ( os.size() )
   {
      std::vector<ossimString> band_list(0);
      os.split( band_list, ossimString(","), false );
      if ( band_list.size() )
      {
         std::vector<ossimString>::const_iterator i = band_list.begin();
         while ( i != band_list.end() )
         {
            ossim_uint32 band = (*i).toUInt32();
            if ( band ) // One based so we need to subtract.
            {
               bandList.push_back( band - 1 );
            }
            ++i;
         }
      }
   }

} // End: ossimChipProcUtil::getBandList

bool ossimChipProcUtil::hasLutFile() const
{
   bool result = false;
   result = ( m_kwl.find( LUT_FILE_KW.c_str() ) != 0 );
   return result;
}

bool ossimChipProcUtil::hasBrightnesContrastOperation() const
{
   bool result = false;
   std::string value = m_kwl.findKey( BRIGHTNESS_KW );
   if ( value.size() )
   {
      result = true;
   }
   else
   {
      value = m_kwl.findKey( CONTRAST_KW );
      if ( value.size() )
      {
         result = true;
      }
   }
   return result;
}

bool ossimChipProcUtil::hasGeoPolyCutterOption()const
{
   bool result = (m_kwl.find(CLIP_WMS_BBOX_LL_KW.c_str())||
         m_kwl.find(CLIP_POLY_LAT_LON_KW.c_str()));

   return result;
}

bool ossimChipProcUtil::hasThumbnailResolution() const
{
   bool result = false;
   result = ( m_kwl.find( THUMBNAIL_RESOLUTION_KW.c_str() ) != 0 );
   return result;
}

bool ossimChipProcUtil::hasHistogramOperation() const
{
   bool result = false;

   result = ( m_kwl.find( HISTO_OP_KW.c_str() ) != 0 );

   // No option for this right now.  Only through src file.
   return result;
}

ossimRefPtr<ossimImageSource>
ossimChipProcUtil::combineLayers(std::vector< ossimRefPtr<ossimSingleImageChain> >& layers) const
{
   ossimRefPtr<ossimImageSource> combiner = 0;
   ossim_uint32 layerCount = (ossim_uint32) layers.size();
   if ( layerCount  == 1 )
   {
      combiner = layers[0].get();
   }
   else if (layerCount  > 1)
   {
      combiner = createCombiner();
      std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chainIdx = layers.begin();
      while ( chainIdx !=  layers.end() )
      {
         combiner->connectMyInputTo( (*chainIdx).get() );
         ++chainIdx;
      }
   }
   return combiner;
}

ossimRefPtr<ossimImageSource> ossimChipProcUtil::createCombiner() const
{
   ossimRefPtr<ossimImageSource> result;
   ossimString combinerType = m_kwl.find(COMBINER_TYPE_KW.c_str());
   if(combinerType.empty())
      combinerType = "ossimImageMosaic";

   result = ossimImageSourceFactoryRegistry::instance()->createImageSource(combinerType);
   if(!result.valid())
      result = new ossimImageMosaic();

   return result;
}

bool ossimChipProcUtil::isDemFile(const ossimFilename& file) const
{
   bool result = false;
   ossimString ext = file.ext();
   if ( ext.size() >= 2 )
   {
      ext.downcase();
      if ( ( ext == "hgt" ) ||
            ( ext == "dem" ) ||
            ( ( (*ext.begin()) == 'd' ) && ( (*(ext.begin()+1)) == 't' ) ) )
      {
         result = true;
      }
   }
   return result;
}

bool ossimChipProcUtil::isSrcFile(const ossimFilename& file) const
{
   bool result = false;
   ossimString ext = file.ext();
   ext.downcase();
   if ( ext == "src" )
   {
      result = true;
   }
   return result;
}

ossimScalarType ossimChipProcUtil::getOutputScalarType() const
{
   ossimScalarType scalar = OSSIM_SCALAR_UNKNOWN;
   ossimString lookup = m_kwl.findKey( OUTPUT_RADIOMETRY_KW );
   if ( lookup.size() )
   {
      scalar = ossimScalarTypeLut::instance()->getScalarTypeFromString( lookup );
   }
   if ( scalar == OSSIM_SCALAR_UNKNOWN )
   {
      // deprecated keyword...
      if ( keyIsTrue( std::string(SCALE_2_8_BIT_KW) ) )
      {
         scalar = OSSIM_UINT8;
      }
   }
   return scalar;
}

bool ossimChipProcUtil::scaleToEightBit() const
{
   return ( getOutputScalarType() == OSSIM_UINT8 );
}

bool ossimChipProcUtil::snapTieToOrigin() const
{
   return keyIsTrue( SNAP_TIE_TO_ORIGIN_KW );
}

void ossimChipProcUtil::getImageSpaceScale( ossimDpt& imageSpaceScale ) const
{
   std::string value = m_kwl.findKey( IMAGE_SPACE_SCALE_X_KW );
   if ( value.size() )
   {
      imageSpaceScale.x = ossimString(value).toFloat64();
   }
   else
   {
      imageSpaceScale.x = 1.0;
   }
   value = m_kwl.findKey( IMAGE_SPACE_SCALE_Y_KW );
   if ( value.size() )
   {
      imageSpaceScale.y = ossimString(value).toFloat64();
   }
   else
   {
      imageSpaceScale.y = 1.0;
   }
}

ossim_float64 ossimChipProcUtil::getRotation() const
{
   ossim_float64 result = ossim::nan();
   std::string value = m_kwl.findKey( ROTATION_KW);
   if ( value.size() )
   {
      result = ossimString(value).toFloat64();
      if ( result < 0 )
      {
         result += 360.0;
      }

      // Range check:
      if ( ( result < 0.0 ) || ( result > 360.0 ) )
      {
         std::ostringstream errMsg;
         errMsg << "ossimChipProcUtil::getRotation range error!\n"
               << "rotation = " << result
               << "\nMust be between 0 and 360.";
         throw ossimException( errMsg.str() );
      }
   }
   return result;
}

bool ossimChipProcUtil::upIsUp() const
{
   return keyIsTrue( std::string(UP_IS_UP_KW) );
}

bool ossimChipProcUtil::hasRotation() const
{
   bool result = false;
   std::string value = m_kwl.findKey(std::string(ROTATION_KW));
   if ( value.size() )
   {
      result = true;
   }
   return result;
}

bool ossimChipProcUtil::northUp() const
{
   return keyIsTrue( std::string(NORTH_UP_KW) );
}

bool ossimChipProcUtil::isIdentity() const
{
   return (getOutputProjectionType() == IDENTITY);
}

bool ossimChipProcUtil::keyIsTrue( const std::string& key ) const
{
   bool result = false;
   std::string value = m_kwl.findKey( key );
   if ( value.size() )
   {
      result = ossimString(value).toBool();
   }
   return result;
}

ossim_uint32 ossimChipProcUtil::getEntryNumber() const
{
   ossim_uint32 result = 0;
   std::string value = m_kwl.findKey( std::string( ossimKeywordNames::ENTRY_KW ) );
   if ( value.size() )
   {
      result = ossimString(value).toUInt32();
   }
   return result;
}

ossim_int32 ossimChipProcUtil::getZone() const
{
   ossim_int32 result = 0;
   std::string value = m_kwl.findKey( std::string( ossimKeywordNames::ZONE_KW ) );
   if ( value.size() )
   {
      result = ossimString(value).toUInt32();
   }
   return result;
}


std::string ossimChipProcUtil::getHemisphere() const
{
   std::string result;
   result = m_kwl.findKey( std::string( ossimKeywordNames::HEMISPHERE_KW ) );
   return result;
}

bool ossimChipProcUtil::hasSensorModelInput()
{
   bool result = false;

   // Test image layers.
   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chainIdx = m_srcLayers.begin();
   while ( chainIdx != m_srcLayers.end() )
   {
      // Get the image handler:
      ossimRefPtr<ossimImageHandler> ih = (*chainIdx)->getImageHandler();
      if ( ih.valid() )
      {
         // Get the geometry from the first image handler.      
         ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
         if ( geom.valid() )
         {
            // Get the image projection.
            ossimRefPtr<ossimProjection> proj = geom->getProjection();
            if ( proj.valid() )
            {
               // Cast and assign to result.
               ossimMapProjection* mapProj = PTR_CAST( ossimMapProjection, proj.get() );
               if ( !mapProj )
               {
                  result = true;
                  break;
               }
            }
         }
      }   
      ++chainIdx;
   }

   return result;
}

void  ossimChipProcUtil::initializeSrcKwl()
{
   m_srcKwl.clear();
   std::string value = m_kwl.findKey(std::string(SRC_FILE_KW));
   if ( value.size() )
   {
      m_srcKwl.setExpandEnvVarsFlag(true);
      if ( m_srcKwl.addFile( value.c_str() ) == false )
         m_srcKwl.clear();
   }

}

ossim_uint32 ossimChipProcUtil::getNumberOfInputs() const
{
   ossim_uint32 result = 0;
   ossimString regularExpression = INPUT_SOURCE_KW;
   regularExpression += "[0-9]*\\.";
   regularExpression += ossimKeywordNames::FILE_KW;
   result = m_kwl.getNumberOfKeysThatMatch( regularExpression );
   result += m_srcKwl.getNumberOfKeysThatMatch( regularExpression );

   return result;
}

ossimChipProcUtil::OutputProjectionType ossimChipProcUtil::getOutputProjectionType() const
{
   OutputProjectionType result = ossimChipProcUtil::UNKNOWN_PROJ;
   const char* op  = m_kwl.find(ossimKeywordNames::PROJECTION_KW);
   if ( op )
   {
      ossimString os = op;
      os.downcase();
      if (os == "geo")
      {
         result = ossimChipProcUtil::GEO_PROJ;
      }
      else if (os == "geo-scaled")
      {
         result = ossimChipProcUtil::GEO_SCALED_PROJ;
      }
      else if ( os == "input" )
      {
         result = ossimChipProcUtil::INPUT_PROJ;
      }
      else if ( (os == "utm") || (os == "ossimutmprojection") )
      {
         result = ossimChipProcUtil::UTM_PROJ;
      }
      else if ( (os == "identity") || (os == "none") )
      {
         result = ossimChipProcUtil::IDENTITY;
      }
   }
   return result;
}

void ossimChipProcUtil::getClipPolygon(ossimGeoPolygon& polygon)const
{
   ossimString param = m_kwl.find(CLIP_WMS_BBOX_LL_KW.c_str());
   if(!param.empty())
   {
      if(!polygon.addWmsBbox(param))
      {
         polygon.clear();
      }
   }
   else
   {
      param = m_kwl.find(CLIP_POLY_LAT_LON_KW.c_str());
      if(!param.empty())
      {
         std::vector<ossimGpt> points;
         ossim::toVector(points, param);
         if(!points.empty())
         {
            polygon = points;
         }
      }
   }
}

ossim_float64 ossimChipProcUtil::getBrightness() const
{
   ossim_float64 brightness = 0.0;
   std::string value = m_kwl.findKey( BRIGHTNESS_KW );
   if ( value.size() )
   {
      brightness = ossimString(value).toFloat64();

      // Range check it:
      if ( ( brightness < -1.0 ) || ( brightness > 1.0 ) )
      {
         ossimNotify(ossimNotifyLevel_WARN)
                              << "ossimChipProcUtil::getBrightness range error!"
                              << "\nbrightness: " << brightness
                              << "\nvalid range: -1.0 to 1.0"
                              << "\nReturned brightness has been reset to: 0.0"
                              << std::endl;

         brightness = 0.0;
      }
   }
   return brightness;

}

ossim_float64 ossimChipProcUtil::getContrast() const
{
   ossim_float64 contrast = 1.0;
   std::string value = m_kwl.findKey( CONTRAST_KW );
   if ( value.size() )
   {
      contrast = ossimString(value).toFloat64();

      // Range check it:
      if ( ( contrast < 0.0 ) || ( contrast > 20.0 ) )
      {
         ossimNotify(ossimNotifyLevel_WARN)
                              << "ossimChipProcUtil::getContrast range error!"
                              << "\ncontrast: " << contrast
                              << "\nvalid range: 0 to 20.0"
                              << "\nReturned contrast has been reset to: 1.0"
                              << std::endl;

         contrast = 1.0;
      }
   }
   return contrast;

}

std::string ossimChipProcUtil::getSharpenMode() const
{
   ossimString mode = m_kwl.findKey( SHARPEN_MODE_KW );
   if ( mode.size() )
   {   
      mode.downcase();
      if ( (mode != "light") && (mode != "heavy") && (mode != "none") )
      {
         ossimNotify(ossimNotifyLevel_WARN)
                              << "ossimChipProcUtil::getSharpnessMode WARNING!"
                              << "\nInvalid sharpness mode: " << mode
                              << "\nValid modes: \"light\" and \"heavy\""
                              << std::endl;
         mode = "";
      }
   }
   return mode.string();
}

void ossimChipProcUtil::setUsage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimInit::instance()->addOptions(ap);

   // Set app name.
   std::string appName = ap.getApplicationName();
   ossimApplicationUsage* au = ap.getApplicationUsage();

   // Base class has some:
   ossimUtility::setUsage(ap);

   // Add options.
   au->addCommandLineOption( "-b or --bands <n,n...>", "Use the specified bands in given order: e.g. \"3,2,1\" will select bands 3, 2 and 1 of the input image.\nNote: it is 1 based" );
   au->addCommandLineOption( "--brightness", "<brightness>\nApply brightness to input image(s). Valid range: -1.0 to 1.0" );
   au->addCommandLineOption("--central-meridian","<central_meridian_in_decimal_degrees>\nNote if set this will be used for the central meridian of the projection.  This can be used to lock the utm zone.");
   au->addCommandLineOption("--color-table | --lut","<color-table.kwl>\nKeyword list containing color table for color-relief option.");
   au->addCommandLineOption("--contrast", "<constrast>\nApply contrast to input image(s). Valid range: -1.0 to 1.0" );
   au->addCommandLineOption("--cut-wms-bbox", "<minx>,<miny>,<maxx>,<maxy>\nSpecify a comma separated list in the format of a WMS BBOX.\nThe units are in the units of the projector defined by the --srs key");
   au->addCommandLineOption("--cut-wms-bbox-ll", "<minx>,<miny>,<maxx>,<maxy>\nSpecify a comma separated list in the format of a WMS BBOX.\nThe units are always decimal degrees");
   au->addCommandLineOption("--cut-width", "<width>\nSpecify the cut width in pixel");
   au->addCommandLineOption("--cut-height", "<height>\nSpecify the cut height in pixel");
   au->addCommandLineOption("--clip-wms-bbox-ll", "<minx>,<miny>,<maxx>,<maxy>\nSpecify a comma separated list in the format of a WMS BBOX.\nThe units are always decimal degrees");
   au->addCommandLineOption("--clip-poly-lat-lon", "Polygon in the form of a string: (lat,lon),(lat,lon),...(lat,lon)");
   au->addCommandLineOption("--cut-bbox-ll", "<min_lat> <min_lon> <max_lat> <max_lon>\nSpecify a bounding box with the minimum latitude/longitude and max latitude/longitude in decimal degrees.\nNote coordinates are edge to edge.");
   au->addCommandLineOption("--cut-bbox-llwh", "<min_lat> <min_lon> <max_lat> <max_lon> <width> <height>\nSpecify a bounding box with the minimum latitude/longitude, max latitude/longitude in decimal degrees and width/height in pixels.\nNote coordinates are edge to edge.");
   au->addCommandLineOption("--cut-center-llwh","<latitude> <longitude> <width> <height>\nSpecify the center cut in latitude longitude space with width and height in pixels.");
   au->addCommandLineOption("--cut-center-llr","<latitude> <longitude> <radius_in_meters>\nSpecify the center cut in latitude longitude space with radius in meters.");
   au->addCommandLineOption("--degrees","<dpp_xy> | <dpp_x> <dpp_y>\nSpecifies an override for degrees per pixel. Takes either a single value applied equally to x and y directions, or two values applied correspondingly to x then y. This option takes precedence over the \"--meters\" option.");
   au->addCommandLineOption("-e or --entry", "<entry> For multi image handlers which entry do you wish to extract. For list of entries use: \"ossim-info -i <your_image>\" ");
   au->addCommandLineOption("-h or --help", "Display this help and exit.");
   au->addCommandLineOption("--hemisphere", "<hemisphere>\nSpecify a projection hemisphere if supported. E.g. UTM projection. This will lock the hemisphere even if input scene center is the other hemisphere. Valid values for UTM are \"N\" and \"S\"");
   au->addCommandLineOption("--histogram-op", "<operation>\nHistogram operation to perform. Valid operations are \"auto-minmax\", \"std-stretch-1\", \"std-stretch-2\" and \"std-stretch-3\".");
   au->addCommandLineOption("--image-space-scale","<x> <y>\nSpecifies an image space scale for x and y direction. \"chip\" operation only.");
   au->addCommandLineOption("--input-source", "<file> Input raster file to process.");
   au->addCommandLineOption("--meters", "<meters>\nSpecifies an override for the meters per pixel");
   au->addCommandLineOption("-n or --north-up", "Rotates image North up. \"chip\" operation only.");
   au->addCommandLineOption("--options","<options.kwl>  This can be all or part of the application options.  To get a template you can turn on trace to the ossimChipProcUtil class by adding \"-T ossimChipProcUtil\" to your command.");
   au->addCommandLineOption("--origin-latitude","<latidude_in_decimal_degrees>\nNote if set this will be used for the origin latitude of the projection.  Setting this to something other than 0.0 with a geographic projection creates a scaled geographic projection.");
   au->addCommandLineOption("--output-radiometry", "<R>\nSpecifies the desired product's pixel radiometry type. Possible values for <R> are: U8, U11, U16, S16, F32. Note this overrides the deprecated option \"scale-to-8-bit\".");
   au->addCommandLineOption("--pad-thumbnail", "<boolean>\nIf true, output thumbnail dimensions will be padded in width or height to make square; else, it will have the aspect ratio of input,  Default=false");
   au->addCommandLineOption("--projection", "<output_projection> Valid projections: geo, geo-scaled, input or utm\ngeo = Equidistant Cylindrical, origin latitude = 0.0\ngeo-scaled = Equidistant Cylindrical, origin latitude = image center\ninput Use first images projection. Must be a map projecion.\nutm = Universal Tranverse Mercator\nIf input and multiple sources the projection of the first image will be used.\nIf utm the zone will be set from the scene center of first image.\nNOTE: --srs takes precedence over this option.");
   au->addCommandLineOption("--resample-filter","<type>\nSpecify what resampler filter to use, e.g. nearest neighbor, bilinear, cubic, sinc.\nSee ossim-info --resampler-filters");
   au->addCommandLineOption("-r or --rotate", "<degrees>\nRotate image by degrees. \"chip\" operation only.");
   au->addCommandLineOption("--reader-prop", "<string>Adds a property to send to the reader. format is name=value");
   au->addCommandLineOption("--scale-to-8-bit", "Scales the output to unsigned eight bits per band. This option has been deprecated by the newer \"--output-radiometry\" option.");
   au->addCommandLineOption("--sharpen-mode", "<mode> Applies sharpness to image chain(s). Valid modes: \"light\", \"heavy\"");
   au->addCommandLineOption("--snap-tie-to-origin", "Snaps tie point to projection origin so that (tie-origin)/gsd come out on an even integer boundary.");
   au->addCommandLineOption("--src-file","<file.src> Input source file list keyword list with list of dems or images or both to process.");
   au->addCommandLineOption("--srs","<src_code>\nSpecify a spatial reference system(srs) code for the output projection. Example: --srs EPSG:4326");
   au->addCommandLineOption("-t or --thumbnail", "<max_dimension>\nSpecify a thumbnail resolution.\nScale will be adjusted so the maximum dimension = argument given.");
   au->addCommandLineOption("--three-band-out", "Force three band output even if input is not. Attempts to map bands to RGB if possible.");
   au->addCommandLineOption("--tile-size", "<size_in_pixels>\nSets the output tile size if supported by writer.  Notes: This sets both dimensions. Must be a multiple of 16, e.g. 1024.");
   au->addCommandLineOption("-u or --up-is-up", "Rotates image to up is up. \"chip\" operation only.");
   au->addCommandLineOption("-w or --writer","<writer>\nSpecifies the output writer.  Default uses output file extension to determine writer. For valid output writer types use: \"ossim-info --writers\"\n");
   au->addCommandLineOption("--writer-prop", "<writer-property>\nPasses a name=value pair to the writer for setting it's property. Any number of these can appear on the line.");
   au->addCommandLineOption("--zone", "<zone>\nSpecify a projection zone if supported.  E.g. UTM projection. This will lock the zone even if input scene center is in another zone. Valid values for UTM are \"1\" to \"60\"");
   au->addCommandLineOption("--combiner-type", "<type>\nossimBlendMosaic, ossimFeatherMosaic, ossimImageMosaic.  Default: ossimImageMosaic.  Example --combiner-type ossimImageMosaic\n");
}


//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimChipProcUtil.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGeoPolygon.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageMosaic.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/imaging/ossimImageSourceFactoryRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimUtmProjection.h>
#include <ossim/elevation/ossimElevManager.h>
#include <cmath>
#include <sstream>
#include <string>

static const std::string AOI_GEO_RECT_KW         = "aoi_geo_rect";
static const std::string AOI_MAP_RECT_KW         = "aoi_map_rect";
static const std::string AOI_GEO_CENTER_KW       = "aoi_geo_center";
static const std::string AOI_SIZE_METERS_KW      = "aoi_size_meters";
static const std::string AOI_SIZE_PIXELS_KW      = "aoi_size_pixels";
static const std::string CLIP_POLY_LAT_LON_KW    = "clip_poly_lat_lon";
static const std::string ELEV_SOURCES_KW         = "elev_sources";
static const std::string IMAGE_SOURCE_KW         = "image_source";
static const std::string LUT_FILE_KW             = "lut_file";
static const std::string GSD_KW                  = "gsd";
static const std::string OUTPUT_RADIOMETRY_KW    = "output_radiometry";
static const std::string READER_PROPERTY_KW      = "reader_property";
static const std::string SNAP_TIE_TO_ORIGIN_KW   = "snap_tie_to_origin";
static const std::string SRS_KW                  = "srs";
static const std::string TILE_SIZE_KW            = "tile_size"; // pixels
static const std::string TRUE_KW                 = "true";
static const std::string WRITER_KW               = "writer";
static const std::string WRITER_PROPERTY_KW      = "writer_property";

ossimChipProcUtil::ossimChipProcUtil()
:  m_projIsIdentity(false),
   m_geoScaled(false),
   m_productScalarType(OSSIM_SCALAR_UNKNOWN)
{
   m_kwl.setExpandEnvVarsFlag(true);
   m_gsd.makeNan();
   m_geom = new ossimImageGeometry;
   m_procChain = new ossimImageChain;
}

ossimChipProcUtil::~ossimChipProcUtil()
{
   clear();
}

void ossimChipProcUtil::clear()
{
   m_gsd.makeNan();

   // Must disonnect chains so that they destroy.
   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator i = m_imgLayers.begin();
   while ( i != m_imgLayers.end() )
   {
      (*i)->disconnect();
      (*i) = 0;
      ++i;
   }
   m_imgLayers.clear();

   if(m_writer.valid())
   {
      m_writer->disconnect();
      m_writer = 0;
   }
}

void ossimChipProcUtil::initialize(ossimArgumentParser& ap)
{
   if( ap.read("-h") || ap.read("--help") || (ap.argc() == 1) )
   {
      setUsage(ap);
      ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      return;
   }

   std::string tempString1;
   ossimArgumentParser::ossimParameter stringParam1(tempString1);
   std::string tempString2;
   ossimArgumentParser::ossimParameter stringParam2(tempString2);
   std::string tempString3;
   ossimArgumentParser::ossimParameter stringParam3(tempString3);
   std::string tempString4;
   ossimArgumentParser::ossimParameter stringParam4(tempString4);
   double tempDouble1;
   ossimArgumentParser::ossimParameter doubleParam1(tempDouble1);
   double tempDouble2;
   ossimArgumentParser::ossimParameter doubleParam2(tempDouble2);
   vector<ossimString> paramList;

   ossim_uint32 srcIdx        = 0;
   ossim_uint32 readerPropIdx = 0;
   ossim_uint32 writerPropIdx = 0;
   ossimString  key           = "";
   ostringstream keys;

   if ( ap.read("--load-options", stringParam1))
   {
      if (!m_kwl.addFile(tempString1.c_str()))
      {
         ap.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
         throw ossimException("Must supply an output file.");
      }
   }

   if ( ap.read("--aoi-geo-bbox", stringParam1, stringParam2, stringParam3, stringParam4))
   {
      ostringstream ostr;
      ostr<<tempString1<<" "<<tempString2<<" "<<tempString3<<" "<<tempString4<<ends;
      m_kwl.addPair( AOI_GEO_RECT_KW, ostr.str() );
   }

   if ( ap.read("--aoi-geo-center", stringParam1, stringParam2))
   {
      ostringstream ostr;
      ostr<<tempString1<<" "<<tempString2<<ends;
      m_kwl.addPair( AOI_GEO_CENTER_KW, ostr.str() );
   }

   if ( ap.read("--aoi-map-bbox", stringParam1, stringParam2, stringParam3, stringParam4))
   {
      ostringstream ostr;
      ostr<<tempString1<<" "<<tempString2<<" "<<tempString3<<" "<<tempString4<<ends;
      m_kwl.addPair( AOI_MAP_RECT_KW, ostr.str() );
   }

   if ( ap.read("--aoi-size-meters", stringParam1, stringParam2))
   {
      ostringstream ostr;
      ostr<<tempString1<<" "<<tempString2<<ends;
      m_kwl.addPair( AOI_SIZE_METERS_KW, ostr.str() );
   }

   if ( ap.read("--aoi-size-pixels", stringParam1, stringParam2))
   {
      ostringstream ostr;
      ostr<<tempString1<<" "<<tempString2<<ends;
      m_kwl.addPair( AOI_SIZE_PIXELS_KW, ostr.str() );
   }

   if (ap.read("-b", stringParam1) || ap.read("--bands", stringParam1))
      m_kwl.addPair( std::string(ossimKeywordNames::BANDS_KW), tempString1 );

   if( ap.read("--central-meridian", stringParam1) )
      m_kwl.addPair( std::string(ossimKeywordNames::CENTRAL_MERIDIAN_KW), tempString1 );

   if( ap.read("--color-table", stringParam1) || ap.read("--lut", stringParam1) )
      m_kwl.addPair( LUT_FILE_KW, tempString1 );

   if ( ap.read("-e", stringParam1) || ap.read("--entry", stringParam1) )
      m_kwl.addPair( std::string(ossimKeywordNames::ENTRY_KW), tempString1 );

   if (ap.read("--dem", paramList))
   {
      ostringstream value;
      for(ossim_uint32 idx=0; idx<paramList.size(); ++idx)
         value<<paramList[idx]<<",";
      m_kwl.addPair(ELEV_SOURCES_KW, value.str() );
   }

   if( ap.read("--gsd", stringParam1) )
      m_kwl.addPair( GSD_KW, tempString1 );

   if ( ap.read("--hemisphere", stringParam1) )
      m_kwl.addPair( std::string(ossimKeywordNames::HEMISPHERE_KW), tempString1 );

   vector<ossimString> imageFnames;
   ap.read("--image", imageFnames);
   for(ossim_uint32 idx=0; idx<imageFnames.size(); ++idx)
   {
      ostringstream key;
      key<<IMAGE_SOURCE_KW<<idx<<"."<<ossimKeywordNames::FILE_KW;
      m_kwl.addPair(key.str(), imageFnames[idx] );
   }

   if( ap.read("--origin-latitude", stringParam1) )
      m_kwl.addPair( std::string(ossimKeywordNames::ORIGIN_LATITUDE_KW), tempString1 );

   if(ap.read("--output-file", stringParam1))
      m_kwl.addPair( ossimKeywordNames::OUTPUT_FILE_KW, tempString1 );

   if(ap.read("--output-radiometry", stringParam1))
      m_kwl.addPair( OUTPUT_RADIOMETRY_KW, tempString1 );

   if( ap.read("--projection", stringParam1) )
      m_kwl.addPair( std::string(ossimKeywordNames::PROJECTION_KW), tempString1 );

   while (ap.read("--reader-prop", stringParam1))
   {
      ostringstream key;
      key << READER_PROPERTY_KW << readerPropIdx;
      m_kwl.addPair(key.str(), tempString1 );
      ++readerPropIdx;
   }

   if ( ap.read("--snap-tie-to-origin") )
      m_kwl.addPair( SNAP_TIE_TO_ORIGIN_KW, TRUE_KW);

   if( ap.read("--srs", stringParam1) )
   {
      ossimString os = tempString1;
      if ( os.contains("EPSG:") )
         os.gsub( ossimString("EPSG:"), ossimString("") );
      m_kwl.addPair( SRS_KW, os.string() );
   }

   if( ap.read("--tile-size", stringParam1) )
      m_kwl.addPair( TILE_SIZE_KW, tempString1 );

   if( ap.read("-w", stringParam1) || ap.read("--writer", stringParam1) )
      m_kwl.addPair( WRITER_KW, tempString1);

   if( ap.read("-write-template", stringParam1))
   {
      ossimKeywordlist template_kwl;
      getKwlTemplate(template_kwl);
      if (!template_kwl.write(tempString1.c_str()))
      {
         ostringstream xmsg;
         xmsg<<"ossimChipProcUtil:"<<__LINE__<<" Error encountered writing template file to <"
               <<tempString1<<">";
         throw ossimException(xmsg.str());
      }
   }

   while (ap.read("--writer-prop", stringParam1))
   {
      key = WRITER_PROPERTY_KW;
      key += ossimString::toString(writerPropIdx);
      m_kwl.addPair(key.string(), tempString1 );
      ++writerPropIdx;
   }

   if( ap.read("--zone", stringParam1) )
      m_kwl.addPair( std::string(ossimKeywordNames::ZONE_KW), tempString1);
}

void ossimChipProcUtil::processRemainingArgs(ossimArgumentParser& ap)
{
   ossimString  key    = "";
   ossim_uint32 inputIdx = 0;

   bool dumpKwl = false;
   if ( ap.read("--dump-options") )
      dumpKwl = true;

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
         throw ossimException("Must supply an output file.");
      }
   }

   if ( ap.argc() > 2 ) // User passed inputs in front of output file.
   {
      int pos = 1; // ap.argv[0] is application name. 
      while ( pos < (ap.argc()-1) )
      {
         ossimFilename file = ap[pos];
         key = IMAGE_SOURCE_KW;
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
      throw ossimException("Unknown option...");
   }

   if (dumpKwl)
   {
      ossimFilename f = getClassName();
      f.setExtension("kwl");
      if (!m_kwl.write(f.chars()))
      {
         ostringstream xmsg;
         xmsg<<"ossimChipProcUtil:"<<__LINE__<<" Error encountered writing options file to <"<<f<<">";
         throw ossimException(xmsg.str());
      }
      cout<<"\nWrote options file to <"<<f<<">"<<endl;
   }

   initialize(m_kwl);
}

void ossimChipProcUtil::initialize( const ossimKeywordlist& kwl )
{
   // Don't copy KWL if member KWL passed in:
   if (&kwl != &m_kwl)
   {
      // Start with clean options keyword list.
      m_kwl.clear();
      m_kwl.addList( kwl, true );
   }

   // Assign some members from KWL:
   m_productScalarType = ossimScalarTypeLut::instance()->
         getScalarTypeFromString( m_kwl.findKey( OUTPUT_RADIOMETRY_KW ) );

   m_productFilename = m_kwl.findKey( std::string(ossimKeywordNames::OUTPUT_FILE_KW) );
   if ( m_productFilename == ossimFilename::NIL)
   {
      ostringstream errMsg;
      errMsg << "ossimChipProcUtil ["<<__LINE__<<"] No output filename provided!"<<ends;
      throw ossimException(errMsg.str());
   }

   // Create chains for input sources.
   loadImageFiles();

   // Load any specified DEMs:
   loadDemFiles();

   // Initialize projection and propagate to chains.
   createOutputProjection();

   // Pass control to derived class so it can add its specific processing to the chain(s). This
   // should return an initialized m_procChain.
   initProcessingChain();
   finalizeChain();
}

void ossimChipProcUtil::finalizeChain()
{
   ostringstream errMsg;

   // See if an LUT is requested:
   ossimFilename lutFile = m_kwl.findKey( LUT_FILE_KW );
   if (!lutFile.empty())
   {
      if ( !lutFile.exists() )
      {
         errMsg<<"ERROR ossimChipProcUtil ["<<__LINE__<<"] Color table <"<<lutFile
               <<"> does not exists: "<<ends;
         throw ossimException(errMsg.str());
      }
      ossimRefPtr<ossimIndexToRgbLutFilter> lut = new ossimIndexToRgbLutFilter();
      lut->setLut(lutFile);
      m_procChain->add(lut.get());
   }

   // Add a cut filter. This will:
   // 1) Null out/clip any data pulled in.
   // 2) Speed up by not propagating get tile request outside the cut or "aoi"
   //    to the left hand side(input).
   m_cutRectFilter = new ossimRectangleCutFilter();
   m_cutRectFilter->setRectangle( m_aoiViewRect );

   // Null outside.
   m_cutRectFilter->setCutType( ossimRectangleCutFilter::OSSIM_RECTANGLE_NULL_OUTSIDE );

   // Connect chipper input to source chain.
   m_procChain->add(m_cutRectFilter.get());

   // Set the image size here.  Note must be set after combineLayers.  This is needed for
   // the ossimImageGeometry::worldToLocal call for a geographic projection to handle wrapping
   // accross the date line.
   m_geom->setImageSize( m_aoiViewRect.size() );

   // Reset the source bounding rect if it changed.
   m_procChain->initialize();
}

bool ossimChipProcUtil::execute()
{
   if ( !m_procChain.valid() )
      throw ossimException("Null pointer encountered for m_procChain!");

   if (m_geom->getImageSize().hasNans())
      throw ossimException("Image size is NaN!");

   // Set up the writer.
   m_writer = newWriter();

   // Connect the writer to the processing chain.
   m_writer->connectMyInputTo(0, m_procChain.get());

   // Set the area of interest. NOTE: This must be called after the writer->connectMyInputTo as
   // ossimImageFileWriter::initialize incorrectly resets AOI back to the bounding rect.
   m_writer->setAreaOfInterest(m_aoiViewRect);

   if (m_writer->getErrorStatus() != ossimErrorCodes::OSSIM_OK)
      throw ossimException( "Unable to initialize writer for execution" );

   // Add a listener to get percent complete.
   ossimStdOutProgress prog(0, true);
   m_writer->addListener(&prog);

   // Write the file:
   m_writer->execute();
   m_writer->removeListener(&prog);
   if(m_writer->isAborted())
      throw ossimException( "Writer Process aborted!" );

   ossimNotify(ossimNotifyLevel_INFO)<<"Wrote product image to <"<<m_productFilename<<">"<<endl;

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
   ossimRefPtr<ossimImageData> chip = 0;
   if (!m_geom.valid())
      return chip;

   // Set the new cut rectangle:
   m_aoiGroundRect = bounding_grect;
   computeAdjustedViewFromGrect();
   return getChip(m_aoiViewRect);
}

ossimRefPtr<ossimImageData> ossimChipProcUtil::getChip(const ossimIrect& bounding_irect)
{
   ossimRefPtr<ossimImageData> chip = 0;
   if(!m_procChain.valid())
      return chip;

   m_aoiViewRect = bounding_irect;
   m_geom->setImageSize( m_aoiViewRect.size() );

   // There should not be a chipper filter in the chain for getImage() calls since the chipping
   // is implied by the requested rect to getTile(), but if present, make sure it covers
   // the requested AOI to avoid masking out the tile.
   if (!m_cutRectFilter.valid())
   {
      m_cutRectFilter = new ossimRectangleCutFilter;
      m_cutRectFilter->setCutType( ossimRectangleCutFilter::OSSIM_RECTANGLE_NULL_OUTSIDE );
      m_procChain->add(m_cutRectFilter.get());
   }
   m_cutRectFilter->setRectangle( m_aoiViewRect );
   return m_procChain->getTile( m_aoiViewRect, 0 );
}

void ossimChipProcUtil::loadImageFiles()
{
   ossim_uint32 imgCount = m_kwl.numberOf( IMAGE_SOURCE_KW.c_str() );
   ossim_uint32 maxIndex = imgCount + 100; // Allow for skippage in numbering.
   ossim_uint32 foundRecords = 0;
   ossim_uint32 i = 0;
   ossim_uint32 entryIndex = 0;

   // Returns 0 if no entry found. This is default anyway.
   ossim_uint32 globalEntryValue = ossimString(m_kwl.find(ossimKeywordNames::ENTRY_KW)).toUInt32();

   while ( foundRecords < imgCount )
   {
      ostringstream key;
      key <<  IMAGE_SOURCE_KW << i << "." << ossimKeywordNames::FILE_KW;
      ossimFilename f = m_kwl.findKey( key.str() );
      if ( f.size() )
      {
         // Look for the entry key, e.g. image_source0.entry: 10
         ostringstream entryKey;
         entryKey <<  IMAGE_SOURCE_KW << i << "." << ossimKeywordNames::ENTRY_KW;
         ossimString os = m_kwl.findKey(entryKey.str());
         if (!os.empty())
            entryIndex = os.toUInt32();
      else
         entryIndex = globalEntryValue;

      // Add it:
      ossimRefPtr<ossimSingleImageChain> ic = createInputChain(f, entryIndex);
      if (!ic.valid())
      {
         ostringstream errMsg;
         errMsg<<"ERROR: ossimChipProcUtil ["<<__LINE__<<"] Could not open <"<<f<<">"<<ends;
         throw ossimException(errMsg.str());
      }

      // Need a band selector?
      std::vector<ossim_uint32> bandList(0);
      getBandList(i, bandList );
      if ( bandList.size() )
         ic->setBandSelection( bandList );

      m_imgLayers.push_back(ic);
         ++foundRecords;
      }
      ++i;
      if ( i >= maxIndex )
         break;
   }
}

ossimRefPtr<ossimSingleImageChain>
ossimChipProcUtil::createInputChain(const ossimFilename& fname, ossim_uint32 entry_index)
{
   // Init chain with handler:
   ossimRefPtr<ossimSingleImageChain> chain = new ossimSingleImageChain;
   if (!chain->open(fname))
   {
      ostringstream errMsg;
      errMsg<<"ERROR: ossimChipProcUtil ["<<__LINE__<<"] Could not open <"<<fname<<">"<<ends;
      throw ossimException(errMsg.str());
   }

   // Set any reader props:
   ossimImageHandler* handler = chain->getImageHandler().get();
   setReaderProps( handler );
   if (!handler->setCurrentEntry( entry_index ))
   {
      std::ostringstream errMsg;
      errMsg << " ERROR: ossimChipProcUtil ["<<__LINE__<<"] Entry " << entry_index << " out of range!" << std::endl;
      throw ossimException( errMsg.str() );
   }

   // Set up the remapper:
   if ( ( m_productScalarType != OSSIM_SCALAR_UNKNOWN) &&
         ( m_procChain->getOutputScalarType() != m_productScalarType ) )
   {
      ossimRefPtr<ossimScalarRemapper> remapper = new ossimScalarRemapper();
      remapper->setOutputScalarType(m_productScalarType);
      chain->add(remapper.get());
   }

   // Set up the renderer with cache:
   chain->addResampler();
   chain->addCache();

   // Add geo polygon cutter if specifried:
   ossimString param = m_kwl.findKey(CLIP_POLY_LAT_LON_KW);
   if (!param.empty())
   {
      std::vector<ossimGpt> points;
      ossim::toVector(points, param);
      if(points.size() >= 3)
      {
         ossimGeoPolygon polygon(points);
         chain->addGeoPolyCutterPolygon(polygon);
      }
   }

   chain->initialize();
   return chain;
}

void ossimChipProcUtil::loadDemFiles()
{
   ossimString value = m_kwl.findKey(ELEV_SOURCES_KW);
   if (value.empty())
      return;

   vector<ossimString> input_files = value.split(", ");
   ossim_uint32 i = 0;
   while ( i < input_files.size() )
   {
      ossimFilename f = input_files[i].trim();
      if ( f.empty() )
      {
         ++i;
         continue;
      }

      if (!f.isReadable())
      {
         ostringstream errMsg;
         errMsg << "ossimChipProcUtil ["<<__LINE__<<"] Could not open DEM file: <" << f << ">";
         throw ossimException(errMsg.str());
      }

      m_demSources.push_back(f);
      ossimElevManager::instance()->loadElevationPath(f, true);
      ++i;
   }
}

void ossimChipProcUtil::createOutputProjection()
{
   static const char* MODULE = "ossimChipProcUtil::createOutputProjection()";

   ossimString op  = m_kwl.findKey( std::string(ossimKeywordNames::PROJECTION_KW) );
   ossimString srs = m_kwl.findKey( SRS_KW );
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
   ossimRefPtr<ossimMapProjection> proj = 0;
   m_geoScaled = false;
   op.downcase();

   if ( (op == "geo") || (srs == "4326"))
      proj = new ossimEquDistCylProjection();

   else if (srs.size())
      proj = PTR_CAST(ossimMapProjection,
                      ossimProjectionFactoryRegistry::instance()->createProjection(srs));
   else if (op == "geo-scaled")
   {
      m_geoScaled = true; // used later when reference latitude is known
      proj = new ossimEquDistCylProjection();
   }
   else if ( ( op == "input" ) || (op == "identity") || (op == "none") )
      proj = newIdentityProjection();

   else if ( (op == "utm") || (op == "ossimutmprojection") )
      proj = newUtmProjection();

   else
   {
      m_geoScaled = true; // used later when reference latitude is known
      proj = new ossimEquDistCylProjection();
   }

   // Create our ossimImageGeometry with projection (no transform).
   proj->setElevationLookupFlag(true);
   m_geom->setProjection(proj.get());

   // Set the scale and AOI.
   initializeProjectionGsd();
   initializeAOI(); // also sets the tie point to AOI UL and geoscaling to origin.

   // Setup the view in all the chains.
   propagateGeometryToChains();
}

ossimRefPtr<ossimMapProjection> ossimChipProcUtil::newIdentityProjection()
{
   ossimRefPtr<ossimSingleImageChain> sic = 0;
   ossimRefPtr<ossimMapProjection> proj = 0;
   ossimRefPtr<ossimImageRenderer> resampler = 0;

   if ( m_imgLayers.size() )
   {
      sic = m_imgLayers[0];
      if (sic.valid())
      {
         ossimRefPtr<ossimImageHandler>  ih = sic->getImageHandler();
         if (ih.valid())
         {
            ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
            if (geom.valid())
            {
               proj = dynamic_cast<ossimMapProjection*>( geom->getProjection() );
               m_projIsIdentity = true;
            }
         }
      }
   }

   // try input dem "image" if specified:
   if (!proj.valid() && (m_demSources.size()))
   {
      ossimImageGeometry geom;
      if (geom.open(m_demSources[0]))
         proj = dynamic_cast<ossimMapProjection*>(geom.getProjection());
   }

   return proj;
}

ossimRefPtr<ossimMapProjection> ossimChipProcUtil::newUtmProjection()
{
   // Make projection:
   ossimRefPtr<ossimUtmProjection> utm = new ossimUtmProjection;

   // Set the zone from keyword option:
   bool setZone = false;
   ossim_int32 zone = 0;
   std::string value = m_kwl.findKey( std::string( ossimKeywordNames::ZONE_KW ) );
   if ( value.size() )
      zone = ossimString(value).toUInt32();
   if ( (zone > 0 ) && ( zone < 61 ) )
   {
      utm->setZone( zone );
      setZone = true;
   }

   // Set the hemisphere from keyword option:
   bool setHemisphere = false;
   std::string hemisphere = m_kwl.findKey( std::string( ossimKeywordNames::HEMISPHERE_KW ) );
   if ( hemisphere.size() )
   {
      ossimString h (hemisphere);
      h.upcase();
      char hemisphere = 'U';
      if ( ( h == "N" ) || ( h == "NORTH" ) || ( h == "S" ) || ( h == "SOUTH" ) )
      {
         utm->setHemisphere( h[0] );
         setHemisphere = true;
      }
   }

   if ( !setZone || !setHemisphere )
   {
      // Check for user set "central_meridian" and "origin_latitude":
      ossimGpt origin;
      getProjectionOrigin(origin);
      if ( !setZone )
         utm->setZone(origin);
      if ( !setHemisphere )
         utm->setHemisphere(origin);
   }

   return ossimRefPtr<ossimMapProjection>(utm.get());
}

void ossimChipProcUtil::initializeProjectionGsd()
{
   // The GSD computation may be a chicken-and-egg problem. Need to try different methods depending
   // on information available in KWL including specified input images and DEMs.

   ossimRefPtr<ossimMapProjection> mapProj =
         dynamic_cast<ossimMapProjection*>(m_geom->getProjection());

   ossimString lookup = m_kwl.findKey( GSD_KW );
   if ( lookup.size() )
      m_gsd.y = m_gsd.x = lookup.toFloat64();

   if (m_gsd.hasNans())
   {
      // No GSD specified, use minimum among all input images:
      ossimGpt chainTiePoint;
      std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chainIdx = m_imgLayers.begin();
      while ( chainIdx != m_imgLayers.end() )
      {
         ossimRefPtr<ossimImageGeometry> geom =  (*chainIdx)->getImageGeometry();
         if (geom.valid())
         {
            ossimDpt gsd;
            geom->getMetersPerPixel(gsd);
            gsd.y = gsd.x = gsd.mean();
            if (m_gsd.hasNans() ||  ( m_gsd.x > gsd.x))
               m_gsd = gsd;
         }
         ++chainIdx;
      }
   }

   if (m_gsd.hasNans())
   {
      // Still no GSD established, try using minimum among all input DEMs (the operation possibly
      // involves DEMs as inputs):
      std::vector<ossimFilename>::iterator demFile = m_demSources.begin();
      while ( demFile != m_demSources.end() )
      {
         ossimImageGeometry geom;
         if (geom.open(*demFile))
         {
            ossimDpt gsd;
            geom.getMetersPerPixel(gsd);
            gsd.y = gsd.x = gsd.mean();
            if (m_gsd.hasNans() ||  ( m_gsd.x > gsd.x))
               m_gsd = gsd;
         }
         ++demFile;
      }
   }

   if (m_gsd.hasNans())
   {
      // Still no GSD specified. As a last resort, use the elevation database nominal GSD at center
      // as output GSD. This is valid since most operations without input images depend on DEM data.
      // Requires that the center point be given in input spec:
      ossimGpt centerGpt;
      findCenterGpt(centerGpt);
      if (!centerGpt.hasNans())
      {
         ossimElevManager::instance()->getHeightAboveEllipsoid(centerGpt);
         m_gsd.y = m_gsd.x = ossimElevManager::instance()->getMeanSpacingMeters();
         if (m_gsd.x == 0) // getMeanSpacing unfortunately return 0 for unknown GSD
            m_gsd.makeNan();
      }
   }

   if (!m_gsd.hasNans())
      mapProj->setMetersPerPixel(m_gsd);
}

void ossimChipProcUtil::initializeAOI()
{
   static const char MODULE[] = "ossimChipProcUtil::assignAoiViewRect()";

   // Nan rect for starters.
   m_aoiViewRect.makeNan();
   m_aoiGroundRect.makeNan();
   ossimString lookup;

   // The view rect can be specified in different ways:
   ossimDpt centerViewPt;
   centerViewPt.makeNan();
   if ( m_kwl.hasKey( AOI_GEO_CENTER_KW.c_str() ) )
   {
      // A center point was given:
      ossimGpt centerGpt;
      findCenterGpt(centerGpt);
      if ( !centerGpt.isLatNan() && !centerGpt.isLonNan() )
         m_geom->worldToLocal(centerGpt, centerViewPt);

      // A center point spec can be accompanied by a width/height (in meters or pixels):
      ossimIpt imageSize;
      if (m_kwl.hasKey(AOI_SIZE_METERS_KW))
      {
         lookup = m_kwl.findKey( AOI_SIZE_METERS_KW );
         lookup.trim();
         imageSize.x = (ossim_uint32) lookup.before(" ").toDouble()/ m_gsd.x;
         imageSize.y = (ossim_uint32) lookup.after(" ").toDouble() / m_gsd.y;
      }
      else if (m_kwl.hasKey(AOI_SIZE_PIXELS_KW))
      {
         lookup = m_kwl.findKey( AOI_SIZE_PIXELS_KW );
         lookup.trim();
         imageSize.x = lookup.before(" ").toUInt32();
         imageSize.y = lookup.after(" ").toUInt32();
      }

      // Convert pixels to GEO rect:
      ossim_uint32 area = imageSize.x * imageSize.y;
      if (area > 0)
      {
         ossimDpt ulvpt (centerViewPt.x-imageSize.x/2.0, centerViewPt.y-imageSize.y/2.0);
         ossimDpt lrvpt (centerViewPt.x+imageSize.x/2.0, centerViewPt.y+imageSize.y/2.0);
         ossimGpt ulgpt, lrgpt;
         m_geom->localToWorld(ulvpt, ulgpt);
         m_geom->localToWorld(lrvpt, lrgpt);
         m_aoiGroundRect = ossimGrect(ulgpt, lrgpt);
         computeAdjustedViewFromGrect();
      }
   }

   else if ( m_kwl.hasKey( AOI_GEO_RECT_KW ) )
   {
      ossimString lookup = m_kwl.findKey(AOI_GEO_RECT_KW);
      lookup.trim();
      vector<ossimString> substrings = lookup.split(" ");
      if (substrings.size() != 4)
      {
         ostringstream errMsg;
         errMsg << "ossimChipProcUtil ["<<__LINE__<<"] Incorrect number of values specified for "
               "aoi_geo_rect!";
         throw( ossimException(errMsg.str()) );
      }
      ossim_float64 minLatF = substrings[0].toFloat64();
      ossim_float64 minLonF = substrings[1].toFloat64();
      ossim_float64 maxLatF = substrings[2].toFloat64();
      ossim_float64 maxLonF = substrings[3].toFloat64();

      // Check for swap so we don't get a negative height.
      // Note no swap check for longitude as box could cross date line.
      if ( minLatF > maxLatF )
      {
         ossim_float64 tmpF = minLatF;
         minLatF = maxLatF;
         maxLatF = tmpF;
      }

      // Assume cut box is edge to edge or "Pixel Is Area". Our
      // AOI(area of interest) uses center of pixel or "Pixel Is Point"
      // so get the degrees per pixel and shift AOI to center.
      ossimGpt ulgpt (maxLatF, minLonF);
      ossimGpt lrgpt (minLatF , maxLonF);
      m_aoiGroundRect = ossimGrect(ulgpt, lrgpt);
      computeAdjustedViewFromGrect();
   }

   // If no user defined rect set to scene bounding rect.
   if ( m_aoiViewRect.hasNans() )
      setAoiToInputs();

   // If AOI established, we can set projection TP and origin if applicable:
   ossimRefPtr<ossimMapProjection> mapProj =
         dynamic_cast<ossimMapProjection*>(m_geom->getProjection());
   if (!m_aoiGroundRect.hasNans() && mapProj.valid())
   {
      // Geo-scaled projection needs to know the reference latitude:
      if (m_geoScaled)
      {
         // The origin may have ben explicitely specified,or just use midpoint of AOI:
         ossimGpt origin;
         if (!getProjectionOrigin(origin))
            origin = m_aoiGroundRect.midPoint();
         mapProj->setOrigin(origin);
         m_geom->getMetersPerPixel(m_gsd);
      }

      // Set the tie-point of the projection to the AOI's UL.
      // Adjust it to be on an even pixel distance from projection origin if desired:
      mapProj->setUlTiePoints(m_aoiGroundRect.ul());
      bool snapToTP = false;
      m_kwl.getBoolKeywordValue(snapToTP, SNAP_TIE_TO_ORIGIN_KW.c_str());
      if ( snapToTP )
         mapProj->snapTiePointToOrigin();

      // Re-establish the AOI view rect based on updated projection:
      computeAdjustedViewFromGrect();
   }
}

bool ossimChipProcUtil::getProjectionOrigin(ossimGpt& gpt)
{
   gpt = ossimGpt(0,0,0);
   bool found_spec = false;
   ossimString lookup = m_kwl.find(ossimKeywordNames::ORIGIN_LATITUDE_KW);
   if ( lookup.size() )
   {
      gpt.lat = lookup.toFloat64();
      if ( (gpt.lat < -90) || (gpt.lat > 90.0) )
      {
         std::string errMsg = "origin latitude range error! Valid range: -90 to 90";
         throw ossimException(errMsg);
      }
      m_geoScaled = true;
      found_spec = true;
   }

   lookup = m_kwl.find(ossimKeywordNames::CENTRAL_MERIDIAN_KW);
   if ( lookup.size() )
   {
      gpt.lon = lookup.toFloat64();
      if ( (gpt.lon < -180.0) || (gpt.lon > 180.0) )
      {
         std::string errMsg = "central meridian range error! Valid range: -180 to 180";
         throw ossimException(errMsg);
      }
      found_spec = true;
   }
   return found_spec;
}

void ossimChipProcUtil::findCenterGpt(ossimGpt& gpt)
{
   gpt.hgt = 0.0;
   ossimString lookup = m_kwl.findKey( AOI_GEO_CENTER_KW );
   if (lookup.size())
   {
      lookup.trim();
      vector<ossimString> substrings = lookup.split(", ");
      if (substrings.size() != 2)
      {
         ostringstream errMsg;
         errMsg << "ossimChipProcUtil ["<<__LINE__<<"] Incorrect number of values specified for "
               "aoi_geo_center!";
         throw( ossimException(errMsg.str()) );
      }
      gpt.lat = substrings[0].toDouble();
      gpt.lon = substrings[1].toDouble();
      return;
   }

   lookup = m_kwl.findKey(AOI_GEO_RECT_KW);
   if (lookup.size())
   {
      lookup.trim();
      vector<ossimString> substrings = lookup.split(", ");
      if (substrings.size() != 4)
      {
         ostringstream errMsg;
         errMsg << "ossimChipProcUtil ["<<__LINE__<<"] Incorrect number of values specified for "
               "aoi_geo_rect!";
         throw( ossimException(errMsg.str()) );
      }
      ossim_float64 minLatF = substrings[0].toDouble();
      ossim_float64 minLonF = substrings[1].toDouble();
      ossim_float64 maxLatF = substrings[2].toDouble();
      ossim_float64 maxLonF = substrings[3].toDouble();
      gpt.lat = 0.5*(minLatF + maxLatF);
      gpt.lon = 0.5*(minLonF + maxLonF);
      return;
   }
   gpt.makeNan();
}

void ossimChipProcUtil::setAoiToInputs()
{
   ossimRefPtr<ossimImageGeometry> geom;
   ossimGrect bbox_i;
   m_aoiGroundRect.makeNan();

   //  Assign the AOI to be the bounding rect of the union of all inputs. Start with image inputs:
   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chain = m_imgLayers.begin();
   while (chain != m_imgLayers.end())
   {
      geom = (*chain)->getImageGeometry();
      if (geom.valid())
      {
          geom->getBoundingGroundRect(bbox_i);
          if (!bbox_i.hasNans())
          {
             if (m_aoiGroundRect.hasNans())
                m_aoiGroundRect = bbox_i;
             else
                m_aoiGroundRect.expandToInclude(bbox_i);
          }
      }
      ++chain;
   }

   // Now determine DEM inputs coverage:
   ossimGrect bbox_dems;
   bbox_dems.makeNan();
   std::vector<ossimFilename>::iterator dem_file = m_demSources.begin();
   while (dem_file != m_demSources.end())
   {
      ossimRefPtr<ossimImageGeometry> geom = new ossimImageGeometry;
      if (!geom->open(*dem_file))
      {
         ossimNotify(ossimNotifyLevel_WARN)
                  << "ossimHLZUtil::initialize ERR: Cannot open DEM file <" << *dem_file << "> "
                  "as image handler. Ignoring entry for bounding box computation.\n" << std::endl;
      }
      geom->getBoundingGroundRect(bbox_i);
      if (!bbox_i.hasNans())
      {
         if (bbox_dems.hasNans())
            bbox_dems = bbox_i;
         else
            bbox_dems.expandToInclude(bbox_i);
      }
      ++dem_file;
   }

   // Finally compute the AOI as the intersection of the image and DEM inputs:
   if (m_aoiGroundRect.hasNans())
      m_aoiGroundRect = bbox_dems;
   else if (!bbox_dems.hasNans())
      m_aoiGroundRect = m_aoiGroundRect.clipToRect(bbox_dems);
}

void ossimChipProcUtil::computeAdjustedViewFromGrect()
{
   // Nan rect for starters.
   m_aoiViewRect.makeNan();
   if (m_aoiGroundRect.hasNans())
      return;

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
   gpt.lat = m_aoiGroundRect.ul().lat - halfDpp.y;
   gpt.lon = m_aoiGroundRect.ul().lon + halfDpp.x;
   m_geom->worldToLocal(gpt, ulPt);

   // Lower right:
   gpt.lat = m_aoiGroundRect.lr().lat + halfDpp.y;
   gpt.lon = m_aoiGroundRect.lr().lon - halfDpp.x;
   m_geom->worldToLocal(gpt, lrPt);

   m_aoiViewRect = ossimIrect( ossimIpt(ulPt), ossimIpt(lrPt) );

   // If no user defined rect set to scene bounding rect.
   if ( m_aoiViewRect.hasNans() )
      m_aoiViewRect = m_procChain->getBoundingRect(0);
}

void ossimChipProcUtil::propagateGeometryToChains()
{
   // we need to make sure the outputs are refreshed so they can reset themselves
   // Needed when we are doing interactive update to the GSD and clip window
   ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent();
   ossimEventVisitor eventVisitor(refreshEvent.get());
   ossimViewInterfaceVisitor viewVisitor(m_geom.get());

   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chainIdx = m_imgLayers.begin();
   if (m_projIsIdentity)
   {
      // Identity rojection applies only top the first image in the input list:
      ossimDpt scale(1,1);
      ossimRefPtr<ossimImageGeometry> geom =  (*chainIdx)->getImageGeometry();
      if (geom.valid())
      {
         ossimDpt input_gsd;
         geom->getMetersPerPixel(input_gsd);
         scale = ossimDpt(m_gsd.x/input_gsd.x, m_gsd.y/input_gsd.y);
      }
      ossimDpt midPt = m_aoiViewRect.midPoint();
      (*chainIdx)->getImageRenderer()->setImageViewTransform(
            new ossimImageViewAffineTransform(0.0, scale.x, scale.y, 1, 1, 0, 0, midPt.x, midPt.y));
      ++chainIdx;
   }

   while ( chainIdx != m_imgLayers.end() )
   {
      viewVisitor.reset();
      eventVisitor.reset();
      (*chainIdx)->accept(viewVisitor);
      (*chainIdx)->accept(eventVisitor);
      ossimRefPtr<ossimImageRenderer> resampler = (*chainIdx)->getImageRenderer();
      resampler->setView( m_geom.get() );
      resampler->propagateEventToOutputs(*refreshEvent);
      ++chainIdx;
   }
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

void ossimChipProcUtil::getBandList(ossim_uint32 image_idx,
                                    std::vector<ossim_uint32>& bandList ) const
{
   bandList.clear();
   ostringstream key;
   key <<  IMAGE_SOURCE_KW << image_idx << "." << ossimKeywordNames::BANDS_KW;
   ossimString os = m_kwl.findKey(key.str());
   if (os.empty())
   {
      // Look for possible global bands selection:
      os = m_kwl.find(ossimKeywordNames::BANDS_KW);
   }

   if ( os.size() )
   {
      std::vector<ossimString> band_list(0);
      os.split( band_list, ossimString(", "), false );
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
      combiner = new ossimImageMosaic;
      std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator chainIdx = layers.begin();
      while ( chainIdx !=  layers.end() )
      {
         combiner->connectMyInputTo( (*chainIdx).get() );
         ++chainIdx;
      }
   }
   return combiner;
}

ossimRefPtr<ossimImageFileWriter> ossimChipProcUtil::newWriter()
{
   ossimRefPtr<ossimImageFileWriter> writer = 0;

   ossimString lookup = m_kwl.findKey( WRITER_KW );
   if ( lookup.size() )
   {
      writer = ossimImageWriterFactoryRegistry::instance()->createWriter( lookup );
      if ( !writer.valid() )
      {
         ostringstream errMsg;
         errMsg << "ossimChipProcUtil ["<<__LINE__<<"] ERROR creating writer for <"<<lookup<<">"
               <<ends;
         throw ossimException(errMsg.str());
      }
   }
   else // Create from output file extension.
   {
      writer = ossimImageWriterFactoryRegistry::instance()->
            createWriterFromExtension( m_productFilename.ext() );

      if ( !writer.valid() )
      {
         ostringstream errMsg;
         errMsg << "ossimChipProcUtil ["<<__LINE__<<"] ERROR creating writer from extension <"
               <<m_productFilename.ext()<<">"<<ends;
         throw ossimException(errMsg.str());
      }
   }

   // Set the output name.
   writer->setFilename( m_productFilename );

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
   }

   return writer;
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
   au->addCommandLineOption("--aoi-geo-bbox", "<min-lat> <min-lon> <max-lat> <max-lon>\nSpecify a comma-separated list for the upper-left and lower-right corners of the scene rect in decimal degrees.");
   au->addCommandLineOption("--aoi-geo-center", "<lat> <lon>\nCenter of AOI in decimal degrees.");
   au->addCommandLineOption("--aoi-map-bbox", "<min-x> <min-y> <max-x> <max-y>\nSpecify a space-separated list for the upper-left and lower-right corners of the scene rect in decimal degrees.");
   au->addCommandLineOption("--aoi-map-center", "<x> <y>\nCenter of AOI in map coordinates.");
   au->addCommandLineOption("--aoi-size-pixels", "<dx> <dy>,\nSize of AOI in output product pixels.");
   au->addCommandLineOption("--aoi-size-meters", "<dx> <dy>,\nSize of AOI in meters.");
   au->addCommandLineOption( "-b or --bands <n,n...>", "Use the specified bands in given order: e.g. \"3,2,1\" will select bands 3, 2 and 1 of the input image.\nNote: it is 1 based" );
   au->addCommandLineOption("--central-meridian","<lon>\nNote if set this will be used for the central meridian of the projection.  This can be used to lock the utm zone.");
   au->addCommandLineOption("--color-table | --lut","<color-table.kwl>\nKeyword list containing color table for color-relief option.");
   au->addCommandLineOption("--deg-per-pixel","<dpp_xy> | <dpp_x> <dpp_y>\nSpecifies an override for degrees per pixel. Takes either a single value applied equally to x and y directions, or two values applied correspondingly to x then y. This option takes precedence over the \"--meters\" option.");
   au->addCommandLineOption("--dem", "<file1>[,<file2>...]\n Input DEM file(s) (comma-separated) to process.");
   au->addCommandLineOption("--dump-options","The current state of the utility (after the entire command line is parsed) is written to the file $PWD/<utility>.kwl, where <utility> is the name of the operation being performed. See \"--load-options\".");
   au->addCommandLineOption("-e or --entry", "<entry> For multi image handlers which entry do you wish to extract. For list of entries use: \"ossim-info -i <your_image>\" ");
   au->addCommandLineOption("--gsd", "<meters>\nSpecifies an override for the meters per pixel");
   au->addCommandLineOption("-h or --help", "Display this help and exit.");
   au->addCommandLineOption("--hemisphere", "<hemisphere>\nSpecify a projection hemisphere if supported. E.g. UTM projection. This will lock the hemisphere even if input scene center is the other hemisphere. Valid values for UTM are \"N\" and \"S\"");
   au->addCommandLineOption("--image", "<file1>[, <file2>...] Input image file(s) (comma-separated) to process.");
   au->addCommandLineOption("--load-options","[<filename>]\nThe contents of <filename> (keyword-value pairs) are loaded as command options. The command-line options take precedence.  See \"--load-options\" and \"--write-template\" options.");
   au->addCommandLineOption("--origin-latitude","<latidude_in_decimal_degrees>\nNote if set this will be used for the origin latitude of the projection.  Setting this to something other than 0.0 with a geographic projection creates a scaled geographic projection.");
   au->addCommandLineOption("--output-radiometry", "<R>\nSpecifies the desired product's pixel radiometry type. Possible values for <R> are: U8, U11, U16, S16, F32. Note this overrides the deprecated option \"scale-to-8-bit\".");
   au->addCommandLineOption("--projection", "<output_projection> Valid projections: geo, geo-scaled, input or utm\ngeo = Equidistant Cylindrical, origin latitude = 0.0\ngeo-scaled = Equidistant Cylindrical, origin latitude = image center\ninput Use first images projection. Must be a map projecion.\nutm = Universal Tranverse Mercator\nIf input and multiple sources the projection of the first image will be used.\nIf utm the zone will be set from the scene center of first image.\nNOTE: --srs takes precedence over this option.");
   au->addCommandLineOption("--reader-prop", "<string>Adds a property to send to the reader. format is name=value");
   au->addCommandLineOption("--scale-to-8-bit", "Scales the output to unsigned eight bits per band. This option has been deprecated by the newer \"--output-radiometry\" option.");
   au->addCommandLineOption("--snap-tie-to-origin", "Snaps tie point to projection origin so that (tie-origin)/gsd come out on an even integer boundary.");
   au->addCommandLineOption("--srs","<src_code>\nSpecify a spatial reference system(srs) code for the output projection. Example: --srs EPSG:4326");
   au->addCommandLineOption("-t or --thumbnail", "<max_dimension>\nSpecify a thumbnail resolution.\nScale will be adjusted so the maximum dimension = argument given.");
   au->addCommandLineOption("--tile-size", "<size_in_pixels>\nSets the output tile size if supported by writer.  Notes: This sets both dimensions. Must be a multiple of 16, e.g. 1024.");
   au->addCommandLineOption("-w or --writer","<writer>\nSpecifies the output writer.  Default uses output file extension to determine writer. For valid output writer types use: \"ossim-info --writers\"\n");
   au->addCommandLineOption("--writer-prop", "<writer-property>\nPasses a name=value pair to the writer for setting it's property. Any number of these can appear on the line.");
   au->addCommandLineOption("--write-template", "<filename>\nWrites a keywrd list template to the filename specified.");
   au->addCommandLineOption("--zone", "<zone>\nSpecify a projection zone if supported.  E.g. UTM projection. This will lock the zone even if input scene center is in another zone. Valid values for UTM are \"1\" to \"60\"");
}


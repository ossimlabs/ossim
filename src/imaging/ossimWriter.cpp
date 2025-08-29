//---
//
// License: MIT
// 
// Author:  David Burken
//
// Description: Generic image writer class.
// 
//---
// $Id$

#include <ossim/imaging/ossimWriter.h>
#include <ossim/base/ossimBooleanProperty.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimStopwatch.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimTiffConstants.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>

#define TRACE_TIME 0 /* For function level time stats. */

#include <limits>
#include <fstream>
#include <ostream>
#include <sstream>

static const std::string ADD_ALPHA_CHANNEL_KW   = "add_alpha_channel";   // bool
static const std::string ALIGN_TILES_KW         = "align_tiles";         // bool
static const std::string BLOCK_SIZE_KW          = "block_size";          // unsigned int
static const std::string FALSE_KW               = "false";
static const std::string FLUSH_TILES_KW         = "flush_tiles";         // bool
static const std::string INCLUDE_BLANK_TILES_KW = "include_blank_tiles"; // bool
static const std::string TILE_SIZE_KW           = "tile_size";           // in pixels
static const std::string TRUE_KW                = "true";

static const ossimTrace traceDebug("ossimWriter:debug");
static const ossimTrace traceTime("ossimWriter:time");

ossimWriter::ossimWriter()
   : ossimImageFileWriter(),
     m_str(0),
     m_ownsStreamFlag(false),
     m_kwl(new ossimKeywordlist()),
     m_outputTileSize(OSSIM_DEFAULT_TILE_WIDTH, OSSIM_DEFAULT_TILE_HEIGHT)
{
   // Set default options:
   ossim::defaultTileSize(m_outputTileSize);
   
   m_kwl->addPair( ALIGN_TILES_KW, FALSE_KW );
   m_kwl->addPair( BLOCK_SIZE_KW, "4096" );
   m_kwl->addPair( FLUSH_TILES_KW, TRUE_KW );
   m_kwl->addPair( INCLUDE_BLANK_TILES_KW, TRUE_KW );
   m_kwl->addPair( TILE_SIZE_KW, m_outputTileSize.toString().string() );
}

ossimWriter::~ossimWriter()
{
   close();

   // Not a leak, ref ptr.
   m_kwl = 0;
}

void ossimWriter::close()
{
   if (m_str)      
   {
      m_str->flush();

      if (m_ownsStreamFlag)
      {
         delete m_str;
         m_str = 0;
         m_ownsStreamFlag = false;
      }
   }
}

ossimString ossimWriter::getShortName() const
{
   return ossimString("ossim_writer");
}

ossimString ossimWriter::getLongName() const
{
   return ossimString("ossim writer");
}

ossimString ossimWriter::getClassName() const
{
   return ossimString("ossimWriter");
}

ossimString ossimWriter::getExtension() const
{
   ossimString result = "";
   // ttbs = tiled tiff band separate
   if ( theOutputImageType.contains("ttbs" ) )
   {
      result = "tif";
   }
   return result;
}

void ossimWriter::getImageTypeList(std::vector<ossimString>& imageTypeList) const
{
   imageTypeList.push_back(ossimString("ttbs"));
   imageTypeList.push_back(ossimString("ossim_ttbs"));
}

bool ossimWriter::isOpen() const
{
   return ( m_str ? true : false );
}

bool ossimWriter::open()
{
   bool status = false;
   
   close();

   if ( theFilename.size() && hasImageType( theOutputImageType ) )
   {
      std::ofstream* os = new std::ofstream();
      os->open( theFilename.c_str(), std::ios::out | std::ios::binary );
      if( os->is_open() )
      {
         m_str = os;
         m_ownsStreamFlag = true;
         status = true;
      }
      else
      {
         delete os;
         os = 0;
      }
   }

   return status;
}

bool ossimWriter::hasImageType(const ossimString& imageType) const
{
   bool result = false;
   if ( imageType.contains("ttbs") || imageType.contains("ztif") || (imageType == "image/tif") )
   {
      result = true;
   }
   return result;
}

bool ossimWriter::writeFile()
{
   bool status = true;
   
   if( theInputConnection.valid() &&
       (getErrorStatus() == ossimErrorCodes::OSSIM_OK) )
   {
      //---
      // Check for stream if master process.
      // Note only the master process is used for writing...
      //---
      if( theInputConnection->isMaster() )
      {
         if (!isOpen())
         {
            status = open();
         }
      }

      if ( status )
      {
         status = writeStream();

         // Flush and close the stream.
         close();
      }
   }

   return status;
}

bool ossimWriter::writeStream()
{
   //---
   // This can be called publically so we must to the same checks as the
   // writeFile method.
   //---
   bool status = false;

   // Must have a sequencer...
   if( theInputConnection.valid() )
   {
      if ( isOpen() )
      {
         //---
         // ttbs = tiled tiff band separate
         //---
         if ( theOutputImageType.contains("ttbs") )
         {
            status = true;

            if ( (theInputConnection->getTileWidth() !=
                  static_cast<ossim_uint32>(m_outputTileSize.x)) ||
                 (theInputConnection->getTileHeight() !=
                  static_cast<ossim_uint32>(m_outputTileSize.y)) )
            {
               theInputConnection->setTileSize(m_outputTileSize);
            }

            if ( status )
            {
               status = writeStreamTtbs();
            }
         }
      }
   }
   
   return status;
}

bool ossimWriter::writeStreamTtbs()
{
   static const char* const MODULE = "ossimWriter::writeStreamTtbs";
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " Entered...\n"
         << "Writer opions:\n"
         << *m_kwl << "\n";
   }
   
   // Always big tiff in native byte order.
   bool status = false;

   if ( m_str )
   {
      if ( canContiguousWrite() == true )
      {
         // Open a memory stream to write to:
         std::ostringstream* str = new std::ostringstream();

         std::streampos pos;
         getTtbsTileStartPos(pos);

         // Zero out up to start of data.
         std::vector<ossim_uint8> v((std::streamsize)pos, 0);
         str->write( (char*)v.data(), v.size() );
         str->seekp(0, std::ios_base::beg);
         
         if ( writeTiffHdr( str ) == true )
         {
            if ( writeTiffTags( str ) )
            {
               // Copy to base stream.
               m_str->write( str->str().c_str(), str->str().size() );

#if 0 /* Please leave for debug. drb */
               std::cout << "\nifd end position: " << m_str->tellp() << std::endl;
#endif

               // Write the image tiles out:
               status = writeTtbs( );
            }
         }

         if ( str )
         {
            delete str;
            str = 0;
         }
      }
      else
      {
         if ( writeTiffHdr( m_str ) == true )
         {
            std::vector<ossim_uint64>  tile_offsets;
            std::vector<ossim_uint64>  tile_byte_counts;
            
            //---
            // Min/max arrays must start off empty for
            // ossimImageData::computeMinMaxPix code.
            //---
            std::vector<ossim_float64> minBands(0);
            std::vector<ossim_float64> maxBands(0);
            
            if ( writeTtbs(
                    tile_offsets, tile_byte_counts, minBands, maxBands   ) == true )
            {
               status = writeTiffTags( tile_offsets, tile_byte_counts, minBands, maxBands );
            }
         }
      }
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " Exit status: " << (status?"true":"false") << "\n";
   }
   
   return status;
}

bool ossimWriter::writeTiffHdr( std::ostream* str )
{
#if TRACE_TIME
   ossimStopwatch sw;
   sw.start();
#endif

   bool status = false;
   
   if ( str )
   {
      //---
      // First two bytes, byte order indication.
      // "MM"(big endian) or "II"(little endian.
      //---
      std::string s;
      if ( ossim::byteOrder() == OSSIM_LITTLE_ENDIAN )
      {
         s = "II";
      }
      else
      {
         s = "MM";
      }
      str->write( s.c_str(), 2 );
      
      // Version, 42=classic tiff, 43=big tiff.
      ossim_uint16 us16 = 43;
      str->write( (const char*)&us16, 2 );
      
      // Byte size of offsets.
      us16 = 8;
      str->write( (const char*)&us16, 2 );
      
      // Always 0:
      us16 = 0;
      str->write( (const char*)&us16, 2 );
      
      // Offset to the IFD(image file directory).
      ossim_uint64 ul64 = 16;
      str->write( (const char*)&ul64, 8 );

      status = str->good();
   }
      
#if TRACE_TIME
   sw.stop();
   ossimNotify(ossimNotifyLevel_NOTICE)
      << "ossimWriter::writeTiffHdr time in seconds: "
      << std::fixed << std::setprecision(8) << sw.count() << "\n";
#endif
   
   return status;
}

bool ossimWriter::writeTiffTags( const std::vector<ossim_uint64>& tile_offsets,
                                 const std::vector<ossim_uint64>& tile_byte_counts,
                                 const std::vector<ossim_float64>& minBands,
                                 const std::vector<ossim_float64>& maxBands )
{
#if TRACE_TIME
   ossimStopwatch sw;
   sw.start();
#endif
   
   bool status = false;

   if ( m_str && theInputConnection.valid() )
   {
      ossimRefPtr<ossimMapProjection> mapProj = 0;
      ossimRefPtr<ossimImageGeometry> geom = theInputConnection->getImageGeometry();
      if ( geom.valid() )
      {
         ossimRefPtr<ossimProjection> proj = geom->getProjection();
         mapProj = dynamic_cast<ossimMapProjection*>( proj.get() );
      }
      
      // Seek to the IFD.
      m_str->seekp( 16, std::ios_base::beg );
      
      // tag count, this will be rewritten at the end:
      ossim_uint64 tagCount = 0;
      m_str->write( (const char*)&tagCount, 8 );
      
      //---
      // This is where the tile offsets, tile byte counts and arrays bytes are
      // written. Starting at byte position 512 which gives from
      // 16 -> 512(496 bytes) to write tags.
      //---
      std::streamoff arrayWritePos = 512;
      
      // Used throughout:
      ossim_uint16 tag;
      ossim_uint16 type;
      ossim_uint64 count;
      ossim_uint16 value_ui16;
      ossim_uint32 value_ui32;
      bool computeAlpha = addAlpha();
      
      // image width tag 256:
      tag   = ossim::TIFFTAG_IMAGEWIDTH;
      count = 1;
      if ( theAreaOfInterest.width() <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
      {
         type = ossim::TIFF_SHORT;
         value_ui16 = (ossim_uint16)theAreaOfInterest.width();
         writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
      }
      else
      {
         type = ossim::TIFF_LONG;
         value_ui32 = theAreaOfInterest.width();
         writeTiffTag<ossim_uint32>( tag, type, count, &value_ui32, arrayWritePos );
      }
      ++tagCount;
      
      // image length tag 257:
      tag   = ossim::TIFFTAG_IMAGELENGTH;
      count = 1;
      if ( theAreaOfInterest.height() <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
      {
         type = ossim::TIFF_SHORT;
         value_ui16 = (ossim_uint16)theAreaOfInterest.height();
         writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
      }
      else
      {
         type = ossim::TIFF_LONG;
         value_ui32 = theAreaOfInterest.height();
         writeTiffTag<ossim_uint32>( tag, type, count, &value_ui32, arrayWritePos );
      }
      ++tagCount;
      
      // bits per sample tag 258:
      tag   = ossim::TIFFTAG_BITSPERSAMPLE;
      count = theInputConnection->getNumberOfOutputBands();
      if ( computeAlpha ) ++count;
      type  = ossim::TIFF_SHORT;
      value_ui16 = (ossim_uint16)ossim::getBitsPerPixel( theInputConnection->getOutputScalarType() );
      if ( count == 1 )
      {
         writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
      }
      else
      {
         std::vector<ossim_uint16> v(count, value_ui16);
         if ( computeAlpha ) v[count-1] = 8; // Alpha always 8 bit.
         writeTiffTag<ossim_uint16>( tag, type, count, &v.front(), arrayWritePos );
      }
      ++tagCount;
      
      // compression tag 259:
      tag   = ossim::TIFFTAG_COMPRESSION;
      type  = ossim::TIFF_SHORT;
      count = 1;
      value_ui16 = ossim::COMPRESSION_NONE; // tmp only uncompressed supported.
      writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
      ++tagCount;
      
      // photo interpretation tag 262:
      tag   = ossim::TIFFTAG_PHOTOMETRIC;
      type  = ossim::TIFF_SHORT;
      count = 1;
      if ( theInputConnection->getNumberOfOutputBands() == 3 )
      {
         value_ui16 = ossim::PHOTO_RGB;
      }
      else
      {
         value_ui16 = ossim::PHOTO_MINISBLACK;
      }
      writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
      ++tagCount;
      
      // samples per pixel tag 277:
      tag   = ossim::TIFFTAG_SAMPLESPERPIXEL;
      type  = ossim::TIFF_SHORT;
      count = 1;
      value_ui16 = theInputConnection->getNumberOfOutputBands();
      if ( computeAlpha ) ++value_ui16;
      writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
      ++tagCount;
      
      // Writes two tags 280 and 281:
      if ( writeMinMaxTiffTags( arrayWritePos ) == true )
      {
         tagCount += 2;  
      }
      
      // planar conf tag 284:
      tag   = ossim::TIFFTAG_PLANARCONFIG;
      type  = ossim::TIFF_SHORT;
      count = 1;
      value_ui16 = ossim::PLANARCONFIG_SEPARATE;
      writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
      ++tagCount;
      
      if ( isTiled() )
      {
         // tile width tag 322:
         tag   = ossim::TIFFTAG_TILEWIDTH;
         count = 1;
         if (  m_outputTileSize.x <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
         {
            type = ossim::TIFF_SHORT;
            value_ui16 = (ossim_uint16)m_outputTileSize.x;
            writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
         }
         else
         {
            type = ossim::TIFF_LONG;
            value_ui32 = (ossim_uint32)m_outputTileSize.x;
            writeTiffTag<ossim_uint32>( tag, type, count, &value_ui32, arrayWritePos );
         }
         ++tagCount;
         
         // tile length tag 323:
         tag   = ossim::TIFFTAG_TILELENGTH;
         count = 1;
         if (  m_outputTileSize.y <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
         {
            type = ossim::TIFF_SHORT;
            value_ui16 = (ossim_uint16)m_outputTileSize.y;
            writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
         }
         else
         {
            type = ossim::TIFF_LONG;
            value_ui32 = (ossim_uint32)m_outputTileSize.y;
            writeTiffTag<ossim_uint32>( tag, type, count, &value_ui32, arrayWritePos );
         }
         ++tagCount;

         // tile offsets tag 324:
         tag   = ossim::TIFFTAG_TILEOFFSETS;
         count = tile_offsets.size();
         type  = ossim::TIFF_LONG8;
         writeTiffTag<ossim_uint64>( tag, type, count, &tile_offsets.front(), arrayWritePos );
         ++tagCount;

         // tile byte counts tag 325:
         tag   = ossim::TIFFTAG_TILEBYTECOUNTS;
         count = tile_byte_counts.size();
         type  = ossim::TIFF_LONG8;
         writeTiffTag<ossim_uint64>( tag, type, count, &tile_byte_counts.front(), arrayWritePos );
         ++tagCount;

      } // Matches: if ( isTiled() )

      // extra samples tag 338:
      if ( computeAlpha )
      {
         tag = ossim::TIFFTAG_EXTRASAMPLES;
         count = 1;
         type  = ossim::TIFF_SHORT;
         value_ui16 = 2; // "unassociated_alpha_data" and "transparency masks"
         writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
         ++tagCount;
      }
      
      // sample format tag 339:
      tag   = ossim::TIFFTAG_SAMPLEFORMAT;
      count = theInputConnection->getNumberOfOutputBands();
      if ( computeAlpha ) ++count;
      type  = ossim::TIFF_SHORT;
      value_ui16 = getTiffSampleFormat();
      if ( count == 1 )
      {
         writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
      }
      else
      {
         std::vector<ossim_uint16> v(count, value_ui16);
         if ( computeAlpha ) v[count-1] = 1; // Alpha always 8 bit.
         writeTiffTag<ossim_uint16>( tag, type, count, &v.front(), arrayWritePos );
      }
      ++tagCount;
      
      // Writes two tags 340 and 341 (conditional on scalar type):
      if ( writeSMinSMaxTiffTags( minBands, maxBands, arrayWritePos ) == true )
      {
         tagCount += 2;
      }
      
      // Write geo keys if valid map projection:
      if ( mapProj.valid() )
      {
         std::vector<ossim_float64> vf;
         ossimDpt scale;
         ossimDpt tie;
         
         if ( mapProj->isGeographic() )
         {
            ossimGpt gpt;
            mapProj->lineSampleToWorld( theAreaOfInterest.ul(), gpt );
            tie.x = gpt.lon;
            tie.y = gpt.lat;
            scale = mapProj->getDecimalDegreesPerPixel();
         }
         else
         {
            mapProj->lineSampleToEastingNorthing( theAreaOfInterest.ul(), tie );
            scale = mapProj->getMetersPerPixel();
         }
         
         // model pixel scale tag 33550:
         tag   = ossim::MODEL_PIXEL_SCALE_TAG;
         count = 3; // x, y, z
         type  = ossim::TIFF_DOUBLE;
         vf.resize( count );
         vf[0] = scale.x;
         vf[1] = scale.y;
         vf[2] = 0.0;
         writeTiffTag<ossim_float64>( tag, type, count, &vf.front(), arrayWritePos );
         ++tagCount;

         // model tie point tag 33992:
         tag   = ossim::MODEL_TIE_POINT_TAG;
         count = 6; // x, y, z
         type  = ossim::TIFF_DOUBLE;
         vf.resize( count );
         vf[0] = 0.0;   // x image point
         vf[1] = 0.0;   // y image point
         vf[2] = 0.0;   // z image point
         vf[3] = tie.x; // longitude or easting
         vf[4] = tie.y; // latitude of northing
         vf[5] = 0.0;
         writeTiffTag<ossim_float64>( tag, type, count, &vf.front(), arrayWritePos );
         ++tagCount;

         // geo key directory tag 34735:
         tag   = ossim::GEO_KEY_DIRECTORY_TAG;
         count = 0; // set later.
         type  = ossim::TIFF_SHORT;
         std::vector<ossim_uint16> vs(0);

         ossim_int32 geoKeyDoubleParamIndex = 0;
         vf.clear();

         // No
         vs.push_back(1);
         vs.push_back(1);
         vs.push_back(0);
         vs.push_back(10); // Updated later.

         vs.push_back(ossim::GT_MODEL_TYPE_GEO_KEY); // 1024
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back(mapProj->isGeographic() ? ossim::MODEL_TYPE_GEOGRAPHIC :
                      ossim::MODEL_TYPE_PROJECTED);

         vs.push_back(ossim::GT_RASTER_TYPE_GEO_KEY); // 1025
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back(ossim::PIXEL_IS_POINT);

         if ( mapProj->isGeographic() )
         {
            vs.push_back(ossim::GEOGRAPHIC_TYPE_GEO_KEY); // 2048
            vs.push_back(0);
            vs.push_back(1);
            vs.push_back((ossim_uint16)(mapProj->getPcsCode()));
         }

         vs.push_back(ossim::GEOG_GEODETIC_DATUM_GEO_KEY); // 2050
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back((ossim_uint16)(mapProj->getDatum()->epsgCode()));

         if ( mapProj->isGeographic() )
         {
            vs.push_back(ossim::GEOG_ANGULAR_UNITS_GEO_KEY); // 2054
            vs.push_back(0);
            vs.push_back(1);
            vs.push_back(ossim::ANGULAR_DEGREE);
         }
         
         vs.push_back(ossim::GEOG_ELLIPSOID_GEO_KEY); // 2056
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back((ossim_uint16)(mapProj->getDatum()->ellipsoid()->getEpsgCode()));

         // Stored in external OOGEO_DOUBLE_PARAMS_TAG
         vs.push_back(ossim::GEOG_SEMI_MAJOR_AXIS); // 2057
         vs.push_back(ossim::GEO_DOUBLE_PARAMS_TAG); 
         vs.push_back(1);
         vs.push_back(geoKeyDoubleParamIndex++);
         vf.push_back(mapProj->getDatum()->ellipsoid()->a());
         
         vs.push_back(ossim::GEOG_SEMI_MINOR_AXIS); // 2058
         vs.push_back(ossim::GEO_DOUBLE_PARAMS_TAG);
         vs.push_back(1);
         vs.push_back(geoKeyDoubleParamIndex++);
         vf.push_back(mapProj->getDatum()->ellipsoid()->b());
         
         vs.push_back(ossim::PROJECTED_CS_TYPE_GEO_KEY); // 3072
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back((ossim_uint16)(mapProj->getPcsCode()));

         vs.push_back(ossim::PROJECTION_GEO_KEY); // 3074
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back((ossim_uint16)(mapProj->getPcsCode()));

         if ( mapProj->isGeographic() == false )
         {
            vs.push_back(ossim::PROJ_LINEAR_UNITS_GEO_KEY); // 3076
            vs.push_back(0);
            vs.push_back(1);
            vs.push_back(ossim::LINEAR_METER);
         }

         if ( mapProj->isGeographic() == true )
         {
            vs.push_back(ossim::PROJ_NAT_ORIGIN_LONG_GEO_KEY); // 3080
            vs.push_back(ossim::GEO_DOUBLE_PARAMS_TAG);
            vs.push_back(1);
            vs.push_back(geoKeyDoubleParamIndex++);
            vf.push_back(mapProj->getOrigin().lon);
            
            vs.push_back(ossim::PROJ_NAT_ORIGIN_LAT_GEO_KEY); // 3081
            vs.push_back(ossim::GEO_DOUBLE_PARAMS_TAG);
            vs.push_back(1);
            vs.push_back(geoKeyDoubleParamIndex++);
            vf.push_back(mapProj->getOrigin().lat);
         }
      
         count = vs.size();
         vs[3] = (count / 4) - 1;
         writeTiffTag<ossim_uint16>( tag, type, count, &vs.front(), arrayWritePos );
         ++tagCount;
       
         // geo double params tag 33550:
         tag   = ossim::GEO_DOUBLE_PARAMS_TAG;
         count = vf.size(); // ellipsoid major, minor axis
         type  = ossim::TIFF_DOUBLE;
         writeTiffTag<ossim_float64>( tag, type, count, &vf.front(), arrayWritePos );
         ++tagCount;
      }

      // Write trailing zero indicading no more IFDs.
      ossim_uint64 offsetToNextIfd = 0;
      m_str->write( (const char*)&offsetToNextIfd, 8 );

#if 0 /* Please leave for debug. drb */
      std::cout << "tag count: " << tagCount
                << "\nifd end position: " << m_str->tellp() << std::endl;
#endif

      // Seek back and re-write the tag count.
      m_str->seekp( 16, std::ios_base::beg );
      m_str->write( (const char*)&tagCount, 8 );
   
      status =  m_str->good();
      
   } // Matches: if ( m_str )
   
#if TRACE_TIME
   sw.stop();
   ossimNotify(ossimNotifyLevel_NOTICE)
      << "ossimWriter::writeTiffTags\n"
      << "time in seconds: "
      << std::fixed << std::setprecision(8) << sw.count() << "\n";
#endif
   
   return status;
}


bool ossimWriter::writeTiffTags( std::ostream* str )
{
#if TRACE_TIME
   ossimStopwatch sw;
   sw.start();
#endif
   
   bool status = false;

   if ( str && theInputConnection.valid() )
   {
      ossimRefPtr<ossimMapProjection> mapProj = 0;
      ossimRefPtr<ossimImageGeometry> geom = theInputConnection->getImageGeometry();
      if ( geom.valid() )
      {
         ossimRefPtr<ossimProjection> proj = geom->getProjection();
         mapProj = dynamic_cast<ossimMapProjection*>( proj.get() );
      }
   
      // tag count, this will be rewritten at the end:
      ossim_uint64 tagCount = 0;
      str->write( (const char*)&tagCount, 8 );

      //---
      // This is where the tile offsets, tile byte counts and arrays bytes are
      // written. Starting at byte position 512 which gives from
      // 16 -> 512(496 bytes) to write tags.
      //---
      std::streamoff arrayWritePos = 512;

      // Used throughout:
      ossim_uint16 tag;
      ossim_uint16 type;
      ossim_uint64 count;
      ossim_uint16 value_ui16;
      ossim_uint32 value_ui32;
      bool computeAlpha = addAlpha();
   
      // image width tag 256:
      tag   = ossim::TIFFTAG_IMAGEWIDTH;
      count = 1;
      if ( theAreaOfInterest.width() <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
      {
         type = ossim::TIFF_SHORT;
         value_ui16 = (ossim_uint16)theAreaOfInterest.width();
         writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
      }
      else
      {
         type = ossim::TIFF_LONG;
         value_ui32 = theAreaOfInterest.width();
         writeTiffTag<ossim_uint32>( str, tag, type, count, &value_ui32, arrayWritePos );
      }
      ++tagCount;
   
      // image length tag 257:
      tag   = ossim::TIFFTAG_IMAGELENGTH;
      count = 1;
      if ( theAreaOfInterest.height() <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
      {
         type = ossim::TIFF_SHORT;
         value_ui16 = (ossim_uint16)theAreaOfInterest.height();
         writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
      }
      else
      {
         type = ossim::TIFF_LONG;
         value_ui32 = theAreaOfInterest.height();
         writeTiffTag<ossim_uint32>( str, tag, type, count, &value_ui32, arrayWritePos );
      }
      ++tagCount;

      // bits per sample tag 258:
      tag   = ossim::TIFFTAG_BITSPERSAMPLE;
      count = theInputConnection->getNumberOfOutputBands();
      if ( computeAlpha ) ++count;
      type  = ossim::TIFF_SHORT;
      value_ui16 = (ossim_uint16)ossim::getBitsPerPixel( theInputConnection->getOutputScalarType() );
      if ( count == 1 )
      {
         writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
      }
      else
      {
         std::vector<ossim_uint16> v(count, value_ui16);
         if ( computeAlpha ) v[count-1] = 8; // Alpha always 8 bit.
         writeTiffTag<ossim_uint16>( str, tag, type, count, &v.front(), arrayWritePos );
      }
      ++tagCount;

      // compression tag 259:
      tag   = ossim::TIFFTAG_COMPRESSION;
      type  = ossim::TIFF_SHORT;
      count = 1;
      value_ui16 = ossim::COMPRESSION_NONE; // tmp only uncompressed supported.
      writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
      ++tagCount;
   
      // photo interpretation tag 262:
      tag   = ossim::TIFFTAG_PHOTOMETRIC;
      type  = ossim::TIFF_SHORT;
      count = 1;
      if ( theInputConnection->getNumberOfOutputBands() == 3 )
      {
         value_ui16 = ossim::PHOTO_RGB;
      }
      else
      {
         value_ui16 = ossim::PHOTO_MINISBLACK;
      }
      writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
      ++tagCount;

      // samples per pixel tag 277:
      tag   = ossim::TIFFTAG_SAMPLESPERPIXEL;
      type  = ossim::TIFF_SHORT;
      count = 1;
      value_ui16 = theInputConnection->getNumberOfOutputBands();
      if ( computeAlpha ) ++value_ui16;
      writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
      ++tagCount;

      // Writes two tags 280 and 281:
      if ( writeMinMaxTiffTags( str, arrayWritePos ) == true )
      {
         tagCount += 2;  
      }
   
      // planar conf tag 284:
      tag   = ossim::TIFFTAG_PLANARCONFIG;
      type  = ossim::TIFF_SHORT;
      count = 1;
      value_ui16 = ossim::PLANARCONFIG_SEPARATE;
      writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
      ++tagCount;

      if ( isTiled() )
      {
         // tile width tag 322:
         tag   = ossim::TIFFTAG_TILEWIDTH;
         count = 1;
         if (  m_outputTileSize.x <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
         {
            type = ossim::TIFF_SHORT;
            value_ui16 = (ossim_uint16)m_outputTileSize.x;
            writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
         }
         else
         {
            type = ossim::TIFF_LONG;
            value_ui32 = (ossim_uint32)m_outputTileSize.x;
            writeTiffTag<ossim_uint32>( str, tag, type, count, &value_ui32, arrayWritePos );
         }
         ++tagCount;
      
         // tile length tag 323:
         tag   = ossim::TIFFTAG_TILELENGTH;
         count = 1;
         if (  m_outputTileSize.y <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
         {
            type = ossim::TIFF_SHORT;
            value_ui16 = (ossim_uint16)m_outputTileSize.y;
            writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
         }
         else
         {
            type = ossim::TIFF_LONG;
            value_ui32 = (ossim_uint32)m_outputTileSize.y;
            writeTiffTag<ossim_uint32>( str, tag, type, count, &value_ui32, arrayWritePos );
         }
         ++tagCount;
         
         std::vector<ossim_uint64> tile_offsets;
         std::vector<ossim_uint64> tile_byte_counts;
         if ( getTileInfo( tile_offsets, tile_byte_counts ) )
         {
            // tile offsets tag 324:
            tag   = ossim::TIFFTAG_TILEOFFSETS;
            count = tile_offsets.size();
            type  = ossim::TIFF_LONG8;
            writeTiffTag<ossim_uint64>( str, tag, type, count, &tile_offsets.front(), arrayWritePos );
            ++tagCount;

            // tile byte counts tag 325:
            tag   = ossim::TIFFTAG_TILEBYTECOUNTS;
            count = tile_byte_counts.size();
            type  = ossim::TIFF_LONG8;
            writeTiffTag<ossim_uint64>( str, tag, type, count, &tile_byte_counts.front(),
                                        arrayWritePos );
            ++tagCount;
         }

      } // Matches: if ( isTiled() )

      // extra samples tag 338:
      if ( computeAlpha )
      {
         tag = ossim::TIFFTAG_EXTRASAMPLES;
         count = 1;
         type  = ossim::TIFF_SHORT;
         value_ui16 = 2; // "unassociated_alpha_data" and "transparency masks"
         writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
         ++tagCount;
      }

      // sample format tag 339:
      tag   = ossim::TIFFTAG_SAMPLEFORMAT;
      count = theInputConnection->getNumberOfOutputBands();
      if ( computeAlpha ) ++count;
      type  = ossim::TIFF_SHORT;
      value_ui16 = getTiffSampleFormat();
      if ( count == 1 )
      {
         writeTiffTag<ossim_uint16>( str, tag, type, count, &value_ui16, arrayWritePos );
      }
      else
      {
         std::vector<ossim_uint16> v(count, value_ui16);
         if ( computeAlpha ) v[count-1] = 1; // Alpha always 8 bit.
         writeTiffTag<ossim_uint16>( str, tag, type, count, &v.front(), arrayWritePos );
      }
      ++tagCount;

      // Writes two tags 340 and 341 (conditional on scalar type):
      if ( writeSMinSMaxTiffTags( str, arrayWritePos ) == true )
      {
         tagCount += 2;
      }

      // Write geo keys if valid map projection:
      if ( mapProj.valid() )
      {
         std::vector<ossim_float64> vf;
         ossimDpt scale;
         ossimDpt tie;
      
         if ( mapProj->isGeographic() )
         {
            ossimGpt gpt;
            mapProj->lineSampleToWorld( theAreaOfInterest.ul(), gpt );
            tie.x = gpt.lon;
            tie.y = gpt.lat;
            scale = mapProj->getDecimalDegreesPerPixel();
         }
         else
         {
            mapProj->lineSampleToEastingNorthing( theAreaOfInterest.ul(), tie );
            scale = mapProj->getMetersPerPixel();
         }
      
         // model pixel scale tag 33550:
         tag   = ossim::MODEL_PIXEL_SCALE_TAG;
         count = 3; // x, y, z
         type  = ossim::TIFF_DOUBLE;
         vf.resize( count );
         vf[0] = scale.x;
         vf[1] = scale.y;
         vf[2] = 0.0;
         writeTiffTag<ossim_float64>( str, tag, type, count, &vf.front(), arrayWritePos );
         ++tagCount;

         // model tie point tag 33992:
         tag   = ossim::MODEL_TIE_POINT_TAG;
         count = 6; // x, y, z
         type  = ossim::TIFF_DOUBLE;
         vf.resize( count );
         vf[0] = 0.0;   // x image point
         vf[1] = 0.0;   // y image point
         vf[2] = 0.0;   // z image point
         vf[3] = tie.x; // longitude or easting
         vf[4] = tie.y; // latitude of northing
         vf[5] = 0.0;
         writeTiffTag<ossim_float64>( str, tag, type, count, &vf.front(), arrayWritePos );
         ++tagCount;

         // geo key directory tag 34735:
         tag   = ossim::GEO_KEY_DIRECTORY_TAG;
         count = 0; // set later.
         type  = ossim::TIFF_SHORT;
         std::vector<ossim_uint16> vs(0);

         ossim_int32 geoKeyDoubleParamIndex = 0;
         vf.clear();

         // No
         vs.push_back(1);
         vs.push_back(1);
         vs.push_back(0);
         vs.push_back(10); // Updated later.

         vs.push_back(ossim::GT_MODEL_TYPE_GEO_KEY); // 1024
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back(mapProj->isGeographic() ? ossim::MODEL_TYPE_GEOGRAPHIC :
                      ossim::MODEL_TYPE_PROJECTED);

         vs.push_back(ossim::GT_RASTER_TYPE_GEO_KEY); // 1025
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back(ossim::PIXEL_IS_POINT);

         if ( mapProj->isGeographic() )
         {
            vs.push_back(ossim::GEOGRAPHIC_TYPE_GEO_KEY); // 2048
            vs.push_back(0);
            vs.push_back(1);
            vs.push_back((ossim_uint16)(mapProj->getPcsCode()));
         }

         vs.push_back(ossim::GEOG_GEODETIC_DATUM_GEO_KEY); // 2050
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back((ossim_uint16)(mapProj->getDatum()->epsgCode()));

         if ( mapProj->isGeographic() )
         {
            vs.push_back(ossim::GEOG_ANGULAR_UNITS_GEO_KEY); // 2054
            vs.push_back(0);
            vs.push_back(1);
            vs.push_back(ossim::ANGULAR_DEGREE);
         }
         
         vs.push_back(ossim::GEOG_ELLIPSOID_GEO_KEY); // 2056
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back((ossim_uint16)(mapProj->getDatum()->ellipsoid()->getEpsgCode()));

         // Stored in external OOGEO_DOUBLE_PARAMS_TAG
         vs.push_back(ossim::GEOG_SEMI_MAJOR_AXIS); // 2057
         vs.push_back(ossim::GEO_DOUBLE_PARAMS_TAG); 
         vs.push_back(1);
         vs.push_back(geoKeyDoubleParamIndex++);
         vf.push_back(mapProj->getDatum()->ellipsoid()->a());
         
         vs.push_back(ossim::GEOG_SEMI_MINOR_AXIS); // 2058
         vs.push_back(ossim::GEO_DOUBLE_PARAMS_TAG);
         vs.push_back(1);
         vs.push_back(geoKeyDoubleParamIndex++);
         vf.push_back(mapProj->getDatum()->ellipsoid()->b());

         vs.push_back(ossim::PROJECTED_CS_TYPE_GEO_KEY); // 3072
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back((ossim_uint16)(mapProj->getPcsCode()));

         vs.push_back(ossim::PROJECTION_GEO_KEY); // 3074
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back((ossim_uint16)(mapProj->getPcsCode()));

         if ( mapProj->isGeographic() == false )
         {
            vs.push_back(ossim::PROJ_LINEAR_UNITS_GEO_KEY); // 3076
            vs.push_back(0);
            vs.push_back(1);
            vs.push_back(ossim::LINEAR_METER);
         }

         if ( mapProj->isGeographic() == true )
         {
            vs.push_back(ossim::PROJ_NAT_ORIGIN_LONG_GEO_KEY); // 3080
            vs.push_back(ossim::GEO_DOUBLE_PARAMS_TAG);
            vs.push_back(1);
            vs.push_back(geoKeyDoubleParamIndex++);
            vf.push_back(mapProj->getOrigin().lon);

            vs.push_back(ossim::PROJ_NAT_ORIGIN_LAT_GEO_KEY); // 3081
            vs.push_back(ossim::GEO_DOUBLE_PARAMS_TAG);
            vs.push_back(1);
            vs.push_back(geoKeyDoubleParamIndex++);
            vf.push_back(mapProj->getOrigin().lat);
         }
      
         count = vs.size();
         vs[3] = (count / 4) - 1;
         writeTiffTag<ossim_uint16>( str, tag, type, count, &vs.front(), arrayWritePos );
         ++tagCount;
       
         // geo double params tag 33550:
         tag   = ossim::GEO_DOUBLE_PARAMS_TAG;
         count = vf.size(); // ellipsoid major, minor axis
         type  = ossim::TIFF_DOUBLE;
         writeTiffTag<ossim_float64>( str, tag, type, count, &vf.front(), arrayWritePos );
         ++tagCount;
      }

      // Write trailing zero indicading no more IFDs.
      ossim_uint64 offsetToNextIfd = 0;
      str->write( (const char*)&offsetToNextIfd, 8 );

      // Write a trailing null:
      char c = '\0';
      str->write( &c, 1 );

#if 0 /* Please leave for debug. drb */
      std::cout << "tag count: " << tagCount
                << "\nifd end position: " << str->tellp() << std::endl;
#endif

      // Seek back and re-write the tag count.
      str->seekp( 16, std::ios_base::beg );
      str->write( (const char*)&tagCount, 8 );
      
      str->seekp( 0, std::ios_base::end );
   
      status = str->good();
   }
   
#if TRACE_TIME
   sw.stop();
   ossimNotify(ossimNotifyLevel_NOTICE)
      << "ossimWriter::writeTiffTags time in seconds: "
      << std::fixed << std::setprecision(8) << sw.count() << "\n";
#endif
   
   return status;
}

bool ossimWriter::writeMinMaxTiffTags( std::streamoff& arrayWritePos )
{
   return writeMinMaxTiffTags( m_str, arrayWritePos );
}

bool ossimWriter::writeMinMaxTiffTags( std::ostream* str, std::streamoff& arrayWritePos )
{
   bool status = false;

   if ( str )
   {
      status = true;
      
      // DEFAULT for OSSIM_UINT32.
      ossim_uint16 minValue = 1;
      ossim_uint16 maxValue = 255;
      
      switch( theInputConnection->getOutputScalarType() )
      {
         case OSSIM_UINT8:
         {
            break; // defaulted above
         }
         case OSSIM_USHORT11:
         {
            maxValue = 2047;
            break;
         }
         case OSSIM_USHORT12:
         {
            maxValue = 4095;
            break;
         }
         case OSSIM_USHORT13:
         {
            maxValue = 8191;
            break;
         }
         case OSSIM_USHORT14:
         {
            maxValue = 16383;
            break;
         }
         case OSSIM_USHORT15:
         {
            maxValue = 32767;
            break;
         }
         case OSSIM_UINT16:
         {
            maxValue = 65535;
            break;
         }
         default:
            status = false;
      }
      
      if ( status )
      {
         writeTiffTag<ossim_uint16>( str, ossim::TIFFTAG_MINSAMPLEVALUE,
                                     ossim::TIFF_SHORT,
                                     1, &minValue, arrayWritePos );
         writeTiffTag<ossim_uint16>( str, ossim::TIFFTAG_MAXSAMPLEVALUE,
                                     ossim::TIFF_SHORT,
                                     1, &maxValue, arrayWritePos );
      }
   }
   
   return status;
}

bool ossimWriter::writeSMinSMaxTiffTags( const std::vector<ossim_float64>& minBands,
                                         const std::vector<ossim_float64>& maxBands,
                                         std::streamoff& arrayWritePos )
{
   return writeSMinSMaxTiffTags( m_str, minBands, maxBands, arrayWritePos );
}

bool ossimWriter::writeSMinSMaxTiffTags( std::ostream* str,
                                         const std::vector<ossim_float64>& minBands,
                                         const std::vector<ossim_float64>& maxBands,
                                         std::streamoff& arrayWritePos )
{
   bool status = false;
   
   if( str && minBands.size() && maxBands.size() )
   {
      ossim_float64 minValue = *std::min_element(minBands.begin(), minBands.end());
      ossim_float64 maxValue = *std::max_element(maxBands.begin(), maxBands.end());
      
      switch( theInputConnection->getOutputScalarType() )
      {
         case OSSIM_SINT16: 
         case OSSIM_UINT32:
         case OSSIM_FLOAT32:
         case OSSIM_FLOAT64:
         case OSSIM_NORMALIZED_FLOAT:
         case OSSIM_NORMALIZED_DOUBLE:
         {
            ossim_float32 v = static_cast<ossim_float32>(minValue);
            writeTiffTag<ossim_float32>( str, ossim::TIFFTAG_SMINSAMPLEVALUE,
                                         ossim::TIFF_FLOAT, 1, &v, arrayWritePos );
            v = static_cast<ossim_float32>(maxValue);
            writeTiffTag<ossim_float32>( str, ossim::TIFFTAG_SMAXSAMPLEVALUE,
                                         ossim::TIFF_FLOAT, 1, &v, arrayWritePos );
            status = true;
            break;
         }
         default:
         {
            break;
         }
      }
   }
   return status;
}

bool ossimWriter::writeSMinSMaxTiffTags( std::ostream* str, std::streamoff& arrayWritePos )
{
   bool status = false;
   if ( str && theInputConnection )
   {
      //---
      // SMin / SMax only written for specific scalars.
      // 8 bit, and specialized 16 bit(11, 12, 13, 14 and 15) not written.
      //---
      switch( theInputConnection->getOutputScalarType() )
      {
         case OSSIM_SINT16: 
         case OSSIM_UINT32:
         case OSSIM_FLOAT32:
         case OSSIM_FLOAT64:
         case OSSIM_NORMALIZED_FLOAT:
         case OSSIM_NORMALIZED_DOUBLE:
         {
            const ossim_uint32 BANDS = theInputConnection->getNumberOfOutputBands();
            std::vector<ossim_float64> minBands(BANDS);
            std::vector<ossim_float64> maxBands(BANDS);
            for ( ossim_uint32 band = 0; band < BANDS; ++band )
            {
               minBands[band] = theInputConnection->getMinPixelValue(band);
               maxBands[band] = theInputConnection->getMaxPixelValue(band);
            }

            ossim_float64 minValue = *std::min_element(minBands.begin(), minBands.end());
            ossim_float64 maxValue = *std::max_element(maxBands.begin(), maxBands.end());
            
            ossim_float32 v = static_cast<ossim_float32>(minValue);
            writeTiffTag<ossim_float32>( str, ossim::TIFFTAG_SMINSAMPLEVALUE,
                                         ossim::TIFF_FLOAT, 1, &v, arrayWritePos );
            v = static_cast<ossim_float32>(maxValue);
            writeTiffTag<ossim_float32>( str, ossim::TIFFTAG_SMAXSAMPLEVALUE,
                                         ossim::TIFF_FLOAT, 1, &v, arrayWritePos );
            status = true;
            break;
         }
         default:
         {
            break;
         }
      }
   }
   return status;
}

template <class T>
void ossimWriter::writeTiffTag(
   ossim_uint16 tag, ossim_uint16 type, ossim_uint64 count,
   const T* value, std::streamoff& arrayWritePos )
{
   if ( m_str )
   {
      writeTiffTag( m_str, tag, type, count, value, arrayWritePos );
   }
}

template <class T>
void ossimWriter::writeTiffTag(
   std::ostream* str, ossim_uint16 tag, ossim_uint16 type,
   ossim_uint64 count, const T* value, std::streamoff& arrayWritePos )
{
   if ( str )
   {
      str->write( (const char*)&tag, 2 );
      str->write( (const char*)&type, 2 );
      str->write( (const char*)&count, 8 );
      
      ossim_uint64 bytes = sizeof( T ) * count;
      
      if ( bytes <= 8 )
      {
         str->write( (const char*)value, bytes );
         if ( bytes < 8 )
         {
            // Fill remaining bytes with 0.
            char c = '\0';
            str->write( (const char*)&c, (8-bytes) );
         }
      }
      else // Greater than 8 bytes, must write at end of file.
      {
         // Store the offset to array:
         str->write( (const char*)&arrayWritePos, 8 );
         
         // Capture posistion:
         std::streampos currentPos = str->tellp();
         
         // Seek to end:
         str->seekp( arrayWritePos, std::ios_base::beg );
         
         // Write:
         str->write( (const char*)value, bytes );
         
         // Capture new offset for next array write.
         arrayWritePos = str->tellp();
         
         // Seek back:
         str->seekp( currentPos );
      }
   }
}

//---
// This write method if for non-streaming where tile offsets and byte counts
// are captured on the fly. Supports sparce tiles.
//---
bool ossimWriter::writeTtbs( std::vector<ossim_uint64>& tile_offsets,
                             std::vector<ossim_uint64>& tile_byte_counts,
                             std::vector<ossim_float64>& minBands,
                             std::vector<ossim_float64>& maxBands )
{
   ossimStopwatch* sw1 = 0; // total
   ossimStopwatch* sw2 = 0; // input getNextTile only
   ossimStopwatch* sw3 = 0; // I/O write only

   if ( traceTime() )
   {
      sw1 = new ossimStopwatch();
      sw2 = new ossimStopwatch();
      sw3 = new ossimStopwatch();      
      sw1->start();
   }

   static const char* const MODULE = "ossimWriter::writeTtbs(...)";
   if ( traceDebug() ) CLOG << " Entered...\n";

   // Start the sequence at the first tile.
   theInputConnection->setToStartOfSequence();

   // Control flags:
   bool alignTiles    = getAlignTilesFlag();
   bool flushTiles    = getFlushTilesFlag();
   bool writeBlanks   = getWriteBlanksFlag();
   bool computeMinMax = needsMinMax();
   bool computeAlpha  = addAlpha();

   // Block size for write:
   const std::streamsize BLOCK_SIZE = getBlockSize();
   
   const ossim_int32 BANDS       = (ossim_int32)theInputConnection->getNumberOfOutputBands();
   const ossim_int32 TILES_WIDE  = (ossim_int32)theInputConnection->getNumberOfTilesHorizontal();
   const ossim_int32 TILES_TOTAL = (ossim_int32)theInputConnection->getNumberOfTiles();

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "align tiles flag:     " << alignTiles
         << "\nflush tiles flag:     " << flushTiles
         << "\nwrite blanks flag:    " << writeBlanks
         << "\nadd alpha flag:       " << computeAlpha
         << "\ncompute min max flag: " << computeMinMax
         << "\nwrite block size:     " << BLOCK_SIZE
         << "\nBANDS:                " << BANDS
         << "\nTILES_WIDE:           " << TILES_WIDE
         << "\nTILES_TOTAL:          " << TILES_TOTAL << "\n";
   }

   tile_offsets.resize( TILES_TOTAL * (computeAlpha?BANDS+1:BANDS) );
   tile_byte_counts.resize( TILES_TOTAL * (computeAlpha?BANDS+1:BANDS) );

   ossimDataObjectStatus tileStatus   = OSSIM_STATUS_UNKNOWN;
   ossim_int64 ossimTileIndex         = 0;
   ossim_int64 tiffTileIndex          = 0; // per tile
   ossim_int64 tiffTileBandIndex      = 0; // per band
   ossim_int64 tileSizeInBytesPerBand = 0;
   ossim_int64 tileSizeInBytes        = 0;
   ossim_int64 alphaTileSizeInBytes   = 0;
   // ossim_int64 bandOffsetInBytes      = 0;

   std::streampos pos;
   getTtbsTileStartPos( pos );
   
   std::streamsize overflow = 0;
   if ( alignTiles )
   {
      // Snap to block boundary.
      overflow = pos % BLOCK_SIZE;
      if ( overflow > 0 )
      {
         pos += BLOCK_SIZE - overflow;
      }
   }
   m_str->seekp( pos );

#if 0 /* Please leave for debug. drb */
   std::cout << "\nimage data start position: " << m_str->tellp() << std::endl;
#endif

   while ( ossimTileIndex < TILES_TOTAL )
   {
      if ( traceTime() ) sw2->start();
      
      ossimRefPtr<ossimImageData> id = theInputConnection->getNextTile();

      if ( traceTime() ) sw2->stop();
      
      if(!id)
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " ERROR:"
            << "Error returned writing tiff tile:  " << ossimTileIndex
            << "\nNULL Tile from input encountered"
            << std::endl;
         return false;
      }

      tiffTileIndex = ossimTileIndex;

      tileStatus = id->getDataObjectStatus();
      
      if ( ossimTileIndex == 0 )
      {
         // Uncompressed constant tile size.
         tileSizeInBytesPerBand = (ossim_int64)id->getSizePerBandInBytes();
         tileSizeInBytes        = (ossim_int64)id->getSizeInBytes();
         if ( computeAlpha )
         {
            // Alpha tile is always 8 bit.
            alphaTileSizeInBytes = (ossim_int64)id->getSizePerBand();
         }
      }

      if ( computeMinMax )
      {
         if ( (tileStatus == OSSIM_FULL) || (tileStatus == OSSIM_PARTIAL) )
         {
            // Compute running min, max.
            id->computeMinMaxPix(minBands, maxBands);
         }
      }

      if ((writeBlanks == true) || (tileStatus == OSSIM_FULL) || (tileStatus == OSSIM_PARTIAL))
      {
         if ( computeAlpha && (id->hasAlpha() == false) )
         {
            id->computeAlphaChannel();
         }

         // Grab a pointer to the tile for all bands.
         const char* data = (const char*)id->getBuf();
         if ( data )
         {
            if ( traceTime() ) sw3->start();

            // Get stream position of first tile band.
            pos = m_str->tellp();
            if ( alignTiles )
            {
               // Snap to block boundary.
               overflow = pos % BLOCK_SIZE;
               if ( overflow > 0 )
               {
                  pos += BLOCK_SIZE - overflow;
               }
               m_str->seekp( pos );
            }
            
            if ( m_str->good() )
            {
               // Write the tile to stream. All bands will be written contiguously.
               m_str->write( data, (std::streamsize)tileSizeInBytes);

               if ( computeAlpha )
               {
                  const char* alpha = (const char*)id->getAlphaBuf();
                  if ( alpha )
                  {
                     m_str->write( alpha, (std::streamsize)alphaTileSizeInBytes );
                  }
               }

               if ( flushTiles )
               {
                  m_str->flush();
               }
               
               // Check stream:
               if ( m_str->fail() == true )
               {
                  ossimNotify(ossimNotifyLevel_DEBUG)
                     << MODULE << " ERROR:\nWrite error on tiff tile:  " << ossimTileIndex
                     << std::endl;
                  return false;
               }

               // Capture the tile byte position and size in bytes.
               for (ossim_int32 band=0; band < BANDS; ++band)
               {
                  tiffTileBandIndex = tiffTileIndex + band * TILES_TOTAL;
                  tile_offsets[ tiffTileBandIndex ] = (ossim_uint64)pos + band * tileSizeInBytesPerBand;
                  tile_byte_counts[ tiffTileBandIndex ] = (ossim_uint64)tileSizeInBytesPerBand;
               }
               if ( computeAlpha )
               {
                  tiffTileBandIndex = tiffTileIndex + BANDS * TILES_TOTAL;
                  tile_offsets[ tiffTileBandIndex ] = (ossim_uint64)pos + BANDS * tileSizeInBytesPerBand;
                  tile_byte_counts[ tiffTileBandIndex ] = (ossim_uint64)alphaTileSizeInBytes;
               }
            }
            else
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                  << MODULE << " ERROR:\nStream has gone bad!" << std::endl;
               return false;
            }

            if ( traceTime() ) sw3->stop();
            
         }
         else
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << MODULE << " ERROR:\nNull input tile:  " << ossimTileIndex
               << std::endl;
            return false;
         }
      }
      else 
      {
         //---
         // Sparse tile mode:
         // Set the offset and byte count to zero to indicate blank/empty tile.
         //---
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "sparse blank tile[" << tiffTileIndex << "]: " << tiffTileIndex << "\n";
         }

         for (ossim_int32 band=0; band < BANDS; ++band)
         {
            tiffTileBandIndex = tiffTileIndex + band * TILES_TOTAL;
            tile_offsets[ tiffTileBandIndex ] = 0;
            tile_byte_counts[ tiffTileBandIndex ] = 0;
         }
         if ( computeAlpha )
         {
            tiffTileBandIndex = tiffTileIndex + BANDS * TILES_TOTAL;
            tile_offsets[ tiffTileBandIndex ] = 0;
            tile_byte_counts[ tiffTileBandIndex ] = 0;
         }
      }
      
      ++ossimTileIndex;

      if( needsAborting() )
      {
         setPercentComplete(100);
         break; // Get out...
      }
      else if ( ossimTileIndex % TILES_WIDE )
      {
         // Output percent complete every row of tiles.
         double tileNum = ossimTileIndex;
         double numTiles = TILES_TOTAL;
         setPercentComplete(tileNum / numTiles * 100.0);
      }

   } // End: while ( ossimTileIndex < TILES_TOTAL )

   if ( traceTime() )
   {
      sw1->stop();
      ossimNotify(ossimNotifyLevel_NOTICE)
         << std::setiosflags(std::ios::fixed) << std::setprecision(3)
         << MODULE << " timing results:"
         << "\ninput time in seconds: " << sw2->count()
         << "\nwrite time in seconds: " << sw3->count()
         << "\ntotal time in seconds: " << sw1->count() << std::endl;

      delete sw3;
      delete sw2;
      delete sw1;
      sw1 = 0;
      sw2 = 0;
      sw3 = 0;
   }

   if ( traceDebug() ) CLOG << " Exited...\n";
   
   return m_str->good();
}

//---
// This method is for streaming, i.e. contiguous write. No sparse tiles.
//---
bool ossimWriter::writeTtbs()
{
   ossimStopwatch* sw1 = 0; // total
   ossimStopwatch* sw2 = 0; // input getNextTile only
   ossimStopwatch* sw3 = 0; // I/O write only
   
   if ( traceTime() )
   {
      sw1 = new ossimStopwatch();
      sw2 = new ossimStopwatch();
      sw3 = new ossimStopwatch();      
      sw1->start();
   }

   static const char* const MODULE = "ossimWriter::writeTtbs(void)";
   if ( traceDebug() ) CLOG << " Entered...\n";

   if ( !theInputConnection.valid() )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE << " ERROR: Null input connection!" << std::endl;
      return false;
   }

   // Start the sequence at the first tile.
   theInputConnection->setToStartOfSequence();

   // Control flags:
   bool alignTiles    = getAlignTilesFlag();
   bool flushTiles    = getFlushTilesFlag();
   bool writeBlanks   = getWriteBlanksFlag();
   bool computeMinMax = needsMinMax();
   bool computeAlpha  = addAlpha();

   // Block size for write:
   const std::streamsize BLOCK_SIZE = getBlockSize();
   
   const ossim_int32 BANDS       = (ossim_int32)theInputConnection->getNumberOfOutputBands();
   const ossim_int32 TILES_WIDE  = (ossim_int32)theInputConnection->getNumberOfTilesHorizontal();
   const ossim_int32 TILES_TOTAL = (ossim_int32)theInputConnection->getNumberOfTiles();

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "align tiles flag:     " << alignTiles
         << "\nflush tiles flag:     " << flushTiles
         << "\nwrite blanks flag:    " << writeBlanks
         << "\nadd alpha flag:       " << computeAlpha
         << "\ncompute min max flag: " << computeMinMax
         << "\nwrite block size:     " << BLOCK_SIZE
         << "\nBANDS:                " << BANDS
         << "\nTILES_WIDE:           " << TILES_WIDE
         << "\nTILES_TOTAL:          " << TILES_TOTAL
         << "\n";
   }

   ossimDataObjectStatus tileStatus   = OSSIM_STATUS_UNKNOWN;
   ossim_int64 ossimTileIndex         = 0;
   // ossim_int64 tiffTileIndex          = 0;
   // ossim_int64 tileSizeInBytesPerBand = 0;
   ossim_int64 tileSizeInBytes        = 0;
   ossim_int64 alphaTileSizeInBytes   = 0;

   while ( ossimTileIndex < TILES_TOTAL )
   {
      if ( traceTime() ) sw2->start();

      ossimRefPtr<ossimImageData> id = theInputConnection->getNextTile();

      if ( traceTime() ) sw2->stop();

      if(!id)
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " ERROR: writing tiff tile:  " << ossimTileIndex
            << "\nNULL Tile from input encountered"
            << std::endl;
         return false;
      }

      if ( ossimTileIndex == 0 )
      {
         // Uncompressed constant tile size.
         tileSizeInBytes = (ossim_int64)id->getSizeInBytes();
         if ( computeAlpha )
         {
            // Alpha tile is always 8 bit.
            alphaTileSizeInBytes = (ossim_int64)id->getSizePerBand();
         }
      }

      if ((writeBlanks == true) || (tileStatus == OSSIM_FULL) || (tileStatus == OSSIM_PARTIAL))
      {
         if ( computeAlpha && (id->hasAlpha() == false) )
         {
            id->computeAlphaChannel();
         }

         // Grab a pointer to the tile for all bands.
         const char* data = (const char*)id->getBuf();
         if ( data )
         {
            if ( traceTime() ) sw3->start();
            
            if ( m_str->good() )
            {
               // Write the tile to stream. All bands will be written contiguously.
               m_str->write( data, (std::streamsize)tileSizeInBytes);

               if ( computeAlpha )
               {
                  const char* alpha = (const char*)id->getAlphaBuf();
                  if ( alpha )
                  {
                     m_str->write( alpha, (std::streamsize)alphaTileSizeInBytes );
                  }
               }

               if ( flushTiles )
               {
                  m_str->flush();
               }
               
               // Check stream:
               if ( m_str->fail() == true )
               {
                  ossimNotify(ossimNotifyLevel_DEBUG)
                     << MODULE << " ERROR:\nWrite error on tiff tile:  " << ossimTileIndex
                     << std::endl;
                  return false;
               }
            }
            else
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                  << MODULE << " ERROR:\nStream has gone bad!" << std::endl;
               return false;
            }

            if ( traceTime() ) sw3->stop();

         }
         else
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << MODULE << " ERROR:\nNull input tile:  " << ossimTileIndex
               << std::endl;
            return false;
         }
      }
      
      ++ossimTileIndex;

      if( needsAborting() )
      {
         setPercentComplete(100);
         break; // Get out...
      }
      else if ( ossimTileIndex % TILES_WIDE )
      {
         // Output percent complete every row of tiles.
         double tileNum = ossimTileIndex;
         double numTiles = TILES_TOTAL;
         setPercentComplete(tileNum / numTiles * 100.0);
      }
      
   } // End: while ( ossimTileIndex < TILES_TOTAL )

   if ( traceTime() )
   {
      sw1->stop();
      ossimNotify(ossimNotifyLevel_NOTICE)
         << std::setiosflags(std::ios::fixed) << std::setprecision(3)
         << MODULE << " timing results:"
         << "\ninput time in seconds: " << sw2->count()
         << "\nwrite time in seconds: " << sw3->count()
         << "\ntotal time in seconds: " << sw1->count() << std::endl;

      delete sw3;
      delete sw2;
      delete sw1;
      sw1 = 0;
      sw2 = 0;
      sw3 = 0;
   }
   
   if ( traceDebug() ) CLOG << " Exited...\n";
   
   return m_str->good();
}

bool ossimWriter::setOutputStream(std::ostream& stream)
{
   if (m_ownsStreamFlag && m_str)
   {
      delete m_str;
   }
   m_str = &stream;
   m_ownsStreamFlag = false;
   return true;
}

void ossimWriter::setTileSize(const ossimIpt& tileSize)
{
   if ( (tileSize.x % 16) || (tileSize.y % 16) )
   {
      if(traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
                  << "ossimWriter::setTileSize ERROR:"
                  << "\nTile size must be a multiple of 16!"
                  << "\nSize remains:  " << m_outputTileSize
                  << std::endl;
      }
   }
   else
   {
      m_outputTileSize = tileSize;

      // For save state:
      m_kwl->addPair( TILE_SIZE_KW, m_outputTileSize.toString().string() );
   }
}

const ossimIpt& ossimWriter::getOutputTileSize() const
{
   return m_outputTileSize;
}

bool ossimWriter::saveState( ossimKeywordlist& kwl, const char* prefix) const
{
   // Lazy man save state...
   kwl.add( prefix, *(m_kwl.get()), true );
   return ossimImageFileWriter::saveState(kwl, prefix);
}

bool ossimWriter::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   bool result = false;
   if ( ossimImageFileWriter::loadState(kwl, prefix) )
   {
      if ( theOutputImageType.contains("ttbs") || theOutputImageType.contains("ztif") )
      {
         result = true;
         
         std::string pfx = prefix?prefix:"";  
         std::string value;

         value = kwl.findKey( pfx, ADD_ALPHA_CHANNEL_KW);
         if ( value.size() )
         {
            m_kwl->addPair( ADD_ALPHA_CHANNEL_KW, value, true );
         }

         value = kwl.findKey( pfx, ALIGN_TILES_KW );
         if ( value.size() )
         {
            m_kwl->addPair( ALIGN_TILES_KW, value, true );
         }

         value = kwl.findKey( pfx, BLOCK_SIZE_KW );
         if ( value.size() )
         {
            m_kwl->addPair( BLOCK_SIZE_KW, value, true );
         }

         value = kwl.findKey( pfx, FLUSH_TILES_KW );
         if ( value.size() )
         {
            m_kwl->addPair( FLUSH_TILES_KW, value, true );
         }

         value = kwl.findKey( pfx, INCLUDE_BLANK_TILES_KW );
         if ( value.size() )
         {
            m_kwl->addPair( INCLUDE_BLANK_TILES_KW, value, true );
         }

         value = kwl.findKey( pfx, TILE_SIZE_KW );
         if ( value.size() )
         {
            m_outputTileSize.toPoint(value);
            m_kwl->addPair( TILE_SIZE_KW, m_outputTileSize.toString().string(), true );
         }
      }
   }

   return result;
}

void ossimWriter::setProperty(ossimRefPtr<ossimProperty> property)
{
   if( property.valid() )
   {
      // See if it's one of our properties:
      std::string key = property->getName().string();

      if ( traceDebug() )
      {
         ossimString value;
         property->valueToString(value);

         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimWriter::setProperty DEBUG:"
            << "\nkey:   " << key
            << "\nvalue: " << value << "\n";
      }
      
      if ( ( key == ADD_ALPHA_CHANNEL_KW ) ||
           ( key == ALIGN_TILES_KW ) ||
           ( key == BLOCK_SIZE_KW )  ||
           ( key == FLUSH_TILES_KW ) ||
           ( key == INCLUDE_BLANK_TILES_KW ) )
      {
         ossimString value;
         property->valueToString(value);
         m_kwl->addPair( key, value.string(), true );
      }
      else if ( key == TILE_SIZE_KW )
      {
         // Comes in as a single int, e.g.: 256
         ossimString value;
         property->valueToString(value);
         m_outputTileSize.x = value.toInt32();
         m_outputTileSize.y = m_outputTileSize.x;

         // Store in keywordlist / save state as a point, e.g.: ( 256, 256 )
         m_kwl->addPair( key,  m_outputTileSize.toString().string(), true );
      }
      else
      {
         ossimImageFileWriter::setProperty(property);
      }
   }
}

ossimRefPtr<ossimProperty> ossimWriter::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> prop = 0;

   if ( name.string() == ADD_ALPHA_CHANNEL_KW )
   {
      std::string value = m_kwl->findKey( ADD_ALPHA_CHANNEL_KW );
      ossimRefPtr<ossimBooleanProperty> boolProp =
         new ossimBooleanProperty(name, ossimString(value).toBool());
      prop = boolProp.get();
   }
   else if ( name.string() == ALIGN_TILES_KW )
   {
      std::string value = m_kwl->findKey( ALIGN_TILES_KW );
      ossimRefPtr<ossimBooleanProperty> boolProp =
         new ossimBooleanProperty(name, ossimString(value).toBool());
      prop = boolProp.get();
   }
   else if( name == BLOCK_SIZE_KW )
   {
      // Property a single int, e.g.: 4096
      ossim_int64 blockSize = getBlockSize();
      ossimRefPtr<ossimStringProperty> stringProp =
         new ossimStringProperty(name, ossimString::toString(blockSize), false); // editable flag
      prop = stringProp.get();
   }
   else if ( name.string() == FLUSH_TILES_KW )
   {
      std::string value = m_kwl->findKey( FLUSH_TILES_KW );
      ossimRefPtr<ossimBooleanProperty> boolProp =
         new ossimBooleanProperty(name, ossimString(value).toBool());
      prop = boolProp.get();
   }
   else if ( name.string() == INCLUDE_BLANK_TILES_KW )
   {
      std::string value = m_kwl->findKey( INCLUDE_BLANK_TILES_KW );
      ossimRefPtr<ossimBooleanProperty> boolProp =
         new ossimBooleanProperty(name, ossimString(value).toBool());
      prop = boolProp.get();
   }
   else if( name.string() == TILE_SIZE_KW )
   {
      // Property a single int, e.g.: 256
      ossimRefPtr<ossimStringProperty> stringProp =
         new ossimStringProperty(name, ossimString::toString(m_outputTileSize.x), false); // editable flag
      stringProp->setReadOnlyFlag(false);
      stringProp->setChangeType(ossimProperty::ossimPropertyChangeType_AFFECTS_OTHERS);
      stringProp->addConstraint(ossimString("16"));
      stringProp->addConstraint(ossimString("32"));
      stringProp->addConstraint(ossimString("64"));
      stringProp->addConstraint(ossimString("128"));
      stringProp->addConstraint(ossimString("256"));      
      stringProp->addConstraint(ossimString("512"));      
      stringProp->addConstraint(ossimString("1024"));      
      stringProp->addConstraint(ossimString("2048"));
      prop = stringProp.get();
   }
   else
   {
      prop = ossimImageFileWriter::getProperty(name);
   }
   return prop;
}

void ossimWriter::getPropertyNames(std::vector<ossimString>& propertyNames) const
{
   propertyNames.push_back(ossimString(ADD_ALPHA_CHANNEL_KW));
   propertyNames.push_back(ossimString(ALIGN_TILES_KW));
   propertyNames.push_back(ossimString(BLOCK_SIZE_KW));
   propertyNames.push_back(ossimString(FLUSH_TILES_KW));   
   propertyNames.push_back(ossimString(INCLUDE_BLANK_TILES_KW));
   propertyNames.push_back(ossimString(TILE_SIZE_KW));
   ossimImageFileWriter::getPropertyNames(propertyNames);
}

ossim_uint16 ossimWriter::getTiffSampleFormat() const
{
   ossim_uint16 result = 0;
   switch( theInputConnection->getOutputScalarType() )
   {
      case OSSIM_UINT8:
      case OSSIM_USHORT11:
      case OSSIM_USHORT12:
      case OSSIM_USHORT13:
      case OSSIM_USHORT14:
      case OSSIM_USHORT15:
      case OSSIM_UINT16:
      case OSSIM_UINT32:
         result = ossim::SAMPLEFORMAT_UINT;
         break;

      case OSSIM_SINT16:
         result = ossim::SAMPLEFORMAT_INT;
         break;

      case OSSIM_FLOAT32:
      case OSSIM_FLOAT64:
      case OSSIM_NORMALIZED_FLOAT:
      case OSSIM_NORMALIZED_DOUBLE:
         result = ossim::SAMPLEFORMAT_IEEEFP;
         break;

      default:
         break;
   }

   return result;
}

bool ossimWriter::isTiled() const
{
   return ( theOutputImageType.contains("ttbs" ) || theOutputImageType.contains("ztif") );
}

bool ossimWriter::getAlignTilesFlag() const
{
   bool result = true; // default
   std::string value = m_kwl->findKey( ALIGN_TILES_KW );
   if ( value.size() )
   {
      result = ossimString(value).toBool();
   }
   return result;
}

ossim_int64 ossimWriter::getBlockSize() const
{
   ossim_int64 result = 4096; // default
   std::string value = m_kwl->findKey( BLOCK_SIZE_KW );
   if ( value.size() )
   {
      result = ossimString(value).toInt64();

      // Disallow anything not on 1024 boundary.
      if ( result % 1024 )
      {
         result = 4096;
         ossimNotify(ossimNotifyLevel_DEBUG)
                  << "ossimWriter::getBlockSize ERROR:"
                  << "\nBlock size MUST be a multiple of 1024!"
                  << "\nBlock size remains:  " << result
                  << std::endl;
      }
   }
   return result;
}

bool ossimWriter::getFlushTilesFlag() const
{
   bool result = true; // default
   std::string value = m_kwl->findKey( FLUSH_TILES_KW );
   if ( value.size() )
   {
      result = ossimString(value).toBool();
   }
   return result;
}

bool ossimWriter::getWriteBlanksFlag() const
{
   bool result = true; // default
   std::string value = m_kwl->findKey( INCLUDE_BLANK_TILES_KW );
   if ( value.size() )
   {
      result = ossimString(value).toBool();
   }
   return result;
}

bool ossimWriter::needsMinMax() const
{
   bool result = false;
   if ( theInputConnection.valid() )
   {
      switch( theInputConnection->getOutputScalarType() )
      {
         case OSSIM_SINT16: 
         case OSSIM_UINT32:
         case OSSIM_FLOAT32:
      case OSSIM_FLOAT64:
         case OSSIM_NORMALIZED_FLOAT:
         case OSSIM_NORMALIZED_DOUBLE:
         {
            result = true;
            break;
         }
         default:
         {
            break;
         }
      }
   }
   return result;
}

bool ossimWriter::canContiguousWrite() const
{
   bool result = true;
   if ( ( getAlignTilesFlag() == true ) ||
        ( getWriteBlanksFlag() == false ) )
   {
      result = false;
   }
   return result;
}

bool ossimWriter::getTileInfo( std::vector<ossim_uint64>& tile_offsets,
                               std::vector<ossim_uint64>& tile_byte_counts ) const
{
   bool result = false;

   if ( theInputConnection.valid() )
   {
      const ossim_int64 TILES = (ossim_int32)theInputConnection->getNumberOfTiles();
      const ossim_int64 BANDS = (ossim_int32)theInputConnection->getNumberOfOutputBands();
      const ossimIpt TILE_SIZE = theInputConnection->getTileSize();
      ossimScalarType scalar = theInputConnection->getOutputScalarType();
      const ossim_int64 BYTES_PER_PIXEL = (ossim_int32)ossim::scalarSizeInBytes(scalar);
      const ossim_int64 TILE_SIZE_PER_BAND = TILE_SIZE.x * TILE_SIZE.y * BYTES_PER_PIXEL;
      bool computeAlpha = addAlpha();
      const ossim_int64 ALPHA_TILE_SIZE = TILE_SIZE.x * TILE_SIZE.y; // Always 8 bit.

      std::streampos pos;
      getTtbsTileStartPos( pos );
      if ( pos > 0 )
      {
         //---
         // Zero fill the offsets and byte counts. A zero offset and byte code
         // is indicative of a not blank tile on the reader side.
         //---
         tile_offsets.clear();
         tile_byte_counts.clear();
         tile_offsets.resize( TILES * (computeAlpha?BANDS+1:BANDS), 0 );
         tile_byte_counts.resize( TILES * (computeAlpha?BANDS+1:BANDS), 0 );

         result = true;
         ossim_int64 tiffTileIndex = 0;

         for ( ossim_int64 tile = 0; tile < TILES; ++tile )
         {
            tiffTileIndex = theInputConnection->getTileIndex(tile);

#if 0 /* Please leave for debug. drb 20190122 */
            cout << "tile index: " << tile
                 << " sequence tiff tile index: " << tiffTileIndex
                 << "\n";
#endif
            if ( (tiffTileIndex > -1) && (tiffTileIndex < TILES) )
            {
               for ( ossim_int64 band = 0; band < BANDS; ++band )
               {
                  tile_byte_counts[(band*TILES) + tiffTileIndex] = (ossim_uint64)TILE_SIZE_PER_BAND;
                  tile_offsets[(band*TILES) + tiffTileIndex] = (ossim_uint64)pos;
                  pos += TILE_SIZE_PER_BAND;
               }
               if ( computeAlpha )
               {
                  tile_byte_counts[(BANDS*TILES) + tiffTileIndex] = (ossim_uint64)ALPHA_TILE_SIZE;
                  tile_offsets[(BANDS*TILES) + tiffTileIndex] = (ossim_uint64)pos;
                  pos += ALPHA_TILE_SIZE;
               }
            }
            else
            {
               ossimNotify(ossimNotifyLevel_WARN)
                  << "ossimWriter::getTileInfo(...) ERROR:"
                  << "\nSequence index for tile[" << tile << "]: " << tiffTileIndex
                  << "\nTotal tiles for AOI: " << TILES << std::endl;
               result = false;
               break;
            }
         }
      }
   }
   return result;
}

void ossimWriter::getTtbsTileStartPos( std::streampos& pos ) const
{
   //---
   // Adjust the starting position for tiles to make room for IFD tags, tile
   // offset and tile byte counts and arrays.
   //
   // Assuming:
   // IFD start = 16, end 512, gives 496 bytes for tags.
   // Array section start = 512, end is start + (16 * tile_count * bands) + 256 bytes
   // for geotiff array bytes.
   //---
   pos = -1;
   if ( theInputConnection )
   {
      bool computeAlpha  = addAlpha();
      const ossim_int32 TILES = (ossim_int32)theInputConnection->getNumberOfTiles();
      const ossim_int32 BANDS = (ossim_int32)theInputConnection->getNumberOfOutputBands();
      pos = 512 + 16 * TILES * (computeAlpha?BANDS+1:BANDS) + 256;
   }
}

bool ossimWriter::addAlpha() const
{
   bool result = false;
   std::string value = m_kwl->findKey( ADD_ALPHA_CHANNEL_KW );
   if ( value.size() )
   {
      result = ossimString(value).toBool();
   }
   return result;
}

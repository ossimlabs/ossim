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
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimTiffConstants.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>

#include <limits>
#include <ostream>

static const ossimTrace traceDebug("ossimWriter:debug");

using namespace std; // tmp drb

ossimWriter::ossimWriter()
   : ossimImageFileWriter(),
     m_str(0),
     m_ownsStreamFlag(false)
{
}

ossimWriter::~ossimWriter()
{
   close();
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
   if ( theOutputImageType == "ttbs" ) // tiled tiff band separate
   {
      result = "tif";
   }
   return result;
}

void ossimWriter::getImageTypeList(std::vector<ossimString>& imageTypeList) const
{
   // imageTypeList.push_back(ossimString("tiff_tiled_band_separate"));
   imageTypeList.push_back(ossimString("ossim_ttbs")); // tmp drb
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
      os->open( theFilename.c_str(), ios::out | ios::binary );
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
   if ( (imageType == "ossim_ttbs") || (imageType == "image/tif") )
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
         if ( theOutputImageType == "ossim_ttbs" )
         {
            status = writeStreamTtbs();
         }
      }
   }
   
   return status;
}

bool ossimWriter::writeStreamTtbs()
{
   // Alway big tiff in native byte order.
   
   bool status = false;

   if ( writeTiffHdr() == true )
   {
      std::vector<ossim_uint64>  tile_offsets;
      std::vector<ossim_uint64>  tile_byte_counts;
      std::vector<ossim_float64> minBands;
      std::vector<ossim_float64> maxBands;
      
      if ( writeTiffTilesBandSeparate(
              tile_offsets, tile_byte_counts, minBands, maxBands   ) == true )
      {
         status = writeTiffTags( tile_offsets, tile_byte_counts, minBands, maxBands );
      }
   }
   
   return status;
}

bool ossimWriter::writeTiffHdr()
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
   m_str->write( s.c_str(), 2 );

   // Version, 42=classic tiff, 43=big tiff.
   ossim_uint16 us16 = 43;
   m_str->write( (const char*)&us16, 2 );

   // Byte size of offsets.
   us16 = 8;
   m_str->write( (const char*)&us16, 2 );

   // Always 0:
   us16 = 0;
   m_str->write( (const char*)&us16, 2 );

   //---
   // Offset to first IFD(image file directory).
   // This will be updated by writeTiffTags method.
   //---
   ossim_uint64 ul64 = 16;
   m_str->write( (const char*)&ul64, 8 );
   
   return m_str->good();
}

bool ossimWriter::writeTiffTags( const std::vector<ossim_uint64>& tile_offsets,
                                 const std::vector<ossim_uint64>& tile_byte_counts,
                                 const std::vector<ossim_float64>& minBands,
                                 const std::vector<ossim_float64>& maxBands )
{
   bool status = false;

   ossimRefPtr<ossimMapProjection> mapProj = 0;
   ossimRefPtr<ossimImageGeometry> geom = theInputConnection->getImageGeometry();
   if ( geom.valid() )
   {
      ossimRefPtr<ossimProjection> proj = geom->getProjection();
      mapProj = dynamic_cast<ossimMapProjection*>( proj.get() );
   }
   
   // Seek back and set the image file dectory offset(ifd).
   std::streamoff currentPos = m_str->tellp();

   m_str->seekp( 8, std::ios_base::beg );
   
   ossim_uint64 ifdOffset = (ossim_uint64)currentPos;
   m_str->write( (const char*)&ifdOffset, 8 );

   // Seek back to ifd:
   m_str->seekp( currentPos, std::ios_base::beg );
   
   // tag count:
   ossim_uint64 ul64 = mapProj.valid() ? 18 : 14; // tmp drb
   m_str->write( (const char*)&ul64, 8 );

   // Set the sart position for array writing.
   std::streamoff arrayWritePos = m_str->tellp();
   arrayWritePos += 24 * 20; // tmp drb

   // Used throughout:
   ossim_uint16 tag;
   ossim_uint16 type;
   ossim_uint64 count;
   ossim_uint16 value_ui16;
   ossim_uint32 value_ui32;
   
   // image width tag 256:
   tag   = ossim::OTIFFTAG_IMAGEWIDTH;
   count = 1;
   if ( theAreaOfInterest.width() <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
   {
      type = ossim::OTIFF_SHORT;
      value_ui16 = (ossim_uint16)theAreaOfInterest.width();
      writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
   }
   else
   {
      type = ossim::OTIFF_LONG;
      value_ui32 = theAreaOfInterest.width();
      writeTiffTag<ossim_uint32>( tag, type, count, &value_ui32, arrayWritePos );
   }
   
   // image length tag 257:
   tag   = ossim::OTIFFTAG_IMAGELENGTH;
   count = 1;
   if ( theAreaOfInterest.height() <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
   {
      type = ossim::OTIFF_SHORT;
      value_ui16 = (ossim_uint16)theAreaOfInterest.height();
      writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
   }
   else
   {
      type = ossim::OTIFF_LONG;
      value_ui32 = theAreaOfInterest.width();
      writeTiffTag<ossim_uint32>( tag, type, count, &value_ui32, arrayWritePos );
   }   

   // bits per sample tag 258:
   tag   = ossim::OTIFFTAG_BITSPERSAMPLE;
   count = theInputConnection->getNumberOfOutputBands();
   type  = ossim::OTIFF_SHORT;
   value_ui16 = (ossim_uint16)ossim::getBitsPerPixel( theInputConnection->getOutputScalarType() );
   if ( count == 1 )
   {
      writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
   }
   else
   {
      std::vector<ossim_uint16> v(count, value_ui16);
      writeTiffTag<ossim_uint16>( tag, type, count, &v.front(), arrayWritePos );
   }

   // compression tag 259:
   tag   = ossim::OTIFFTAG_COMPRESSION;
   type  = ossim::OTIFF_SHORT;
   count = 1;
   value_ui16 = ossim::COMPRESSION_NONE; // tmp only uncompressed supported.
   writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
   
   // photo interpretation tag 262:
   tag   = ossim::OTIFFTAG_PHOTOMETRIC;
   type  = ossim::OTIFF_SHORT;
   count = 1;
   value_ui16 = ossim::OPHOTO_MINISBLACK;
   writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );

   // samples per pixel tag 277:
   tag   = ossim::OTIFFTAG_SAMPLESPERPIXEL;
   type  = ossim::OTIFF_SHORT;
   count = 1;
   value_ui16 = theInputConnection->getNumberOfOutputBands();
   writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );

   // Writes two tags 280 and 281:
   writeMinMaxTiffTags( minBands, maxBands, arrayWritePos );

   // planar conf tag 284:
   tag   = ossim::OTIFFTAG_PLANARCONFIG;
   type  = ossim::OTIFF_SHORT;
   count = 1;
   value_ui16 = ossim::OTIFFTAG_PLANARCONFIG_SEPARATE;
   writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
   
   // tile width tag 322:
   tag   = ossim::OTIFFTAG_TILEWIDTH;
   count = 1;
   if ( theInputConnection->getTileWidth() <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
   {
      type = ossim::OTIFF_SHORT;
      value_ui16 = (ossim_uint16)theInputConnection->getTileWidth();
      writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
   }
   else
   {
      type = ossim::OTIFF_LONG;
      value_ui32 = theAreaOfInterest.width();
      writeTiffTag<ossim_uint32>( tag, type, count, &value_ui32, arrayWritePos );
   }

   // tile length tag 323:
   tag   = ossim::OTIFFTAG_TILELENGTH;
   count = 1;
   if ( theInputConnection->getTileHeight() <= OSSIM_DEFAULT_MAX_PIX_UINT16 )
   {
      type = ossim::OTIFF_SHORT;
      value_ui16 = (ossim_uint16)theInputConnection->getTileWidth();
      writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
   }
   else
   {
      type = ossim::OTIFF_LONG;
      value_ui32 = theAreaOfInterest.width();
      writeTiffTag<ossim_uint32>( tag, type, count, &value_ui32, arrayWritePos );
   }

   // tile offsets tag 324:
   tag   = ossim::OTIFFTAG_TILEOFFSETS;
   count = tile_offsets.size();
   type  = ossim::OTIFF_LONG8;
   writeTiffTag<ossim_uint64>( tag, type, count, &tile_offsets.front(), arrayWritePos );

   // tile byte counts tag 325:
   tag   = ossim::OTIFFTAG_TILEBYTECOUNTS;
   count = tile_byte_counts.size();
   type  = ossim::OTIFF_LONG8;
   writeTiffTag<ossim_uint64>( tag, type, count, &tile_byte_counts.front(), arrayWritePos );

   // sample format tag 339:
   tag   = ossim::OTIFFTAG_SAMPLEFORMAT;
   count = theInputConnection->getNumberOfOutputBands();
   type  = ossim::OTIFF_SHORT;
   value_ui16 = getTiffSampleFormat();
   if ( count == 1 )
   {
      writeTiffTag<ossim_uint16>( tag, type, count, &value_ui16, arrayWritePos );
   }
   else
   {
      std::vector<ossim_uint16> v(count, value_ui16);
      writeTiffTag<ossim_uint16>( tag, type, count, &v.front(), arrayWritePos );
   }

   // Writes two tags 340 and 341 (conditional on scalar type):
   writeSMinSMaxTiffTags( minBands, maxBands, arrayWritePos );

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
      tag   = ossim::OMODEL_PIXEL_SCALE_TAG;
      count = 3; // x, y, z
      type  = ossim::OTIFF_DOUBLE;
      vf.resize( count );
      vf[0] = scale.x;
      vf[1] = scale.y;
      vf[2] = 0.0;
      writeTiffTag<ossim_float64>( tag, type, count, &vf.front(), arrayWritePos );

      // model tie point tag 33992:
      tag   = ossim::OMODEL_TIE_POINT_TAG;
      count = 6; // x, y, z
      type  = ossim::OTIFF_DOUBLE;
      vf.resize( count );
      vf[0] = 0.0;   // x image point
      vf[1] = 0.0;   // y image point
      vf[2] = 0.0;   // z image point
      vf[3] = tie.x; // longitude or easting
      vf[4] = tie.y; // latitude of northing
      vf[5] = 0.0;
      writeTiffTag<ossim_float64>( tag, type, count, &vf.front(), arrayWritePos );

      // geo key directory tag 34735:
      tag   = ossim::OGEO_KEY_DIRECTORY_TAG;
      count = 0; // set later.
      type  = ossim::OTIFF_SHORT;
      std::vector<ossim_uint16> vs(0);

      // No
      vs.push_back(1);
      vs.push_back(1);
      vs.push_back(0);
      vs.push_back(10); // Updated later.

      vs.push_back(ossim::OGT_MODEL_TYPE_GEO_KEY); // 1024
      vs.push_back(0);
      vs.push_back(1);
      vs.push_back(mapProj->isGeographic() ? ossim::OMODEL_TYPE_GEOGRAPHIC :
                   ossim::OMODEL_TYPE_PROJECTED);

      vs.push_back(ossim::OGT_RASTER_TYPE_GEO_KEY); // 1025
      vs.push_back(0);
      vs.push_back(1);
      vs.push_back(ossim::OPIXEL_IS_POINT);

      if ( mapProj->isGeographic() )
      {
         vs.push_back(ossim::OGEOGRAPHIC_TYPE_GEO_KEY); // 2048
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back((ossim_uint16)(mapProj->getPcsCode()));
      }

      vs.push_back(ossim::OGEOG_GEODETIC_DATUM_GEO_KEY); // 2050
      vs.push_back(0);
      vs.push_back(1);
      vs.push_back((ossim_uint16)(mapProj->getDatum()->epsgCode()));

      if ( mapProj->isGeographic() )
      {
         vs.push_back(ossim::OGEOG_ANGULAR_UNITS_GEO_KEY); // 2054
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back(ossim::OANGULAR_DEGREE);
      }
         
      vs.push_back(ossim::OGEOG_ELLIPSOID_GEO_KEY); // 2056
      vs.push_back(0);
      vs.push_back(1);
      vs.push_back((ossim_uint16)(mapProj->getDatum()->ellipsoid()->getEpsgCode()));

      // Stored in external OOGEO_DOUBLE_PARAMS_TAG
      vs.push_back(ossim::OGEOG_SEMI_MAJOR_AXIS); // 2057
      vs.push_back(ossim::OGEO_DOUBLE_PARAMS_TAG); 
      vs.push_back(1);
      vs.push_back(0);
         
      vs.push_back(ossim::OGEOG_SEMI_MINOR_AXIS); // 2058
      vs.push_back(ossim::OGEO_DOUBLE_PARAMS_TAG);
      vs.push_back(1);
      vs.push_back(1);

      vs.push_back(ossim::OPROJECTED_CS_TYPE_GEO_KEY); // 3072
      vs.push_back(0);
      vs.push_back(1);
      vs.push_back((ossim_uint16)(mapProj->getPcsCode()));

      if ( mapProj->isGeographic() == false )
      {
         vs.push_back(ossim::OPROJ_LINEAR_UNITS_GEO_KEY); // 3076
         vs.push_back(0);
         vs.push_back(1);
         vs.push_back(ossim::OLINEAR_METER);
      }
      
      count = vs.size();
      vs[3] = (count / 4) - 1;
      writeTiffTag<ossim_uint16>( tag, type, count, &vs.front(), arrayWritePos );
       
      // geo double params tag 33550:
      tag   = ossim::OGEO_DOUBLE_PARAMS_TAG;
      count = 2; // ellipsoid major, minor axis
      type  = ossim::OTIFF_DOUBLE;
      vf.resize( count );
      vf[0] = mapProj->getDatum()->ellipsoid()->a();
      vf[1] = mapProj->getDatum()->ellipsoid()->b();
      writeTiffTag<ossim_float64>( tag, type, count, &vf.front(), arrayWritePos );
   }

   // Last write is zero indicading no more ifds.
   ossim_uint64 offsetToNextIfd = 0;
   m_str->write( (const char*)&offsetToNextIfd, 8 );

   status =  m_str->good();
   
   return status;
}

bool ossimWriter::writeMinMaxTiffTags( const vector<ossim_float64>& minBands,
                                       const vector<ossim_float64>& maxBands,
                                       std::streamoff& arrayWritePos )
{
   bool status = true;
   
   if(minBands.size() && maxBands.size())
   {
      ossim_float64 minValue = *std::min_element(minBands.begin(), minBands.end());
      ossim_float64 maxValue = *std::max_element(maxBands.begin(), maxBands.end());
      
      switch( theInputConnection->getOutputScalarType() )
      {
         case OSSIM_USHORT11:
         {
            ossim_uint16 v = 0;
            writeTiffTag<ossim_uint16>( ossim::OTIFFTAG_MINSAMPLEVALUE,
                                        ossim::OTIFF_SHORT,
                                        1, &v, arrayWritePos );
            v = 2047;
            writeTiffTag<ossim_uint16>( ossim::OTIFFTAG_MAXSAMPLEVALUE,
                                       ossim::OTIFF_SHORT,
                                       1, &v, arrayWritePos );
            break;
         }
         case OSSIM_UINT8:
         case OSSIM_UINT16:
         {
            ossim_uint16 v = static_cast<ossim_uint16>(minValue);
            writeTiffTag<ossim_uint16>( ossim::OTIFFTAG_MINSAMPLEVALUE,
                                        ossim::OTIFF_SHORT,
                                        1, &v, arrayWritePos );
            v = static_cast<ossim_uint16>(maxValue);
            writeTiffTag<ossim_uint16>( ossim::OTIFFTAG_MAXSAMPLEVALUE,
                                        ossim::OTIFF_SHORT,
                                        1, &v, arrayWritePos );
            break;
         }
         default:
         {
            status = false;
            break;
         }
      }
   }
   return status;
}

bool ossimWriter::writeSMinSMaxTiffTags( const vector<ossim_float64>& minBands,
                                         const vector<ossim_float64>& maxBands,
                                         std::streamoff& arrayWritePos )
{
   bool status = true;
   
   if(minBands.size() && maxBands.size())
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
            writeTiffTag<ossim_float32>( ossim::OTIFFTAG_SMINSAMPLEVALUE,
                                         ossim::OTIFF_SHORT,
                                         1, &v, arrayWritePos );
            v = static_cast<ossim_float32>(maxValue);
            writeTiffTag<ossim_float32>( ossim::OTIFFTAG_SMAXSAMPLEVALUE,
                                         ossim::OTIFF_SHORT,
                                         1, &v, arrayWritePos );
            break;
         }
         default:
         {
            status = false;
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
   m_str->write( (const char*)&tag, 2 );
   m_str->write( (const char*)&type, 2 );
   m_str->write( (const char*)&count, 8 );

   ossim_uint64 bytes = sizeof( T ) * count;
   
   if ( bytes <= 8 )
   {
      m_str->write( (const char*)value, bytes );
      if ( bytes < 8 )
      {
         // Fill remaining bytes with 0.
         char c = '\0';
         m_str->write( (const char*)&c, (8-bytes) );
      }
   }
   else // Greater than 8 bytes, must write at end of file.
   {
      // Store the offset to array:
      m_str->write( (const char*)&arrayWritePos, 8 );

      // Capture posistion:
      std::streamoff currentPos = m_str->tellp();

      // Seek to end:
      m_str->seekp( arrayWritePos, std::ios_base::beg );
      
      // Write:
      m_str->write( (const char*)value, bytes );

      // Capture new offset for next array write.
      arrayWritePos = m_str->tellp();

      // Seek back:
      m_str->seekp( currentPos, std::ios_base::beg );
   }
}

bool ossimWriter::writeTiffTilesBandSeparate( std::vector<ossim_uint64>& tile_offsets,
                                              std::vector<ossim_uint64>& tile_byte_counts,
                                              std::vector<ossim_float64>& minBands,
                                              std::vector<ossim_float64>& maxBands )
{
   static const char* const MODULE = "ossimTiffWriter::writeToTilesBandSeparate";
   if (traceDebug()) CLOG << " Entered." << std::endl;

   // Start the sequence at the first tile.
   theInputConnection->setToStartOfSequence();

   const ossim_int32 BANDS       = (ossim_int32)theInputConnection->getNumberOfOutputBands();
   const ossim_int32 TILES_WIDE  = (ossim_int32)theInputConnection->getNumberOfTilesHorizontal();
   const ossim_int32 TILES_TOTAL = (ossim_int32)theInputConnection->getNumberOfTiles();

   tile_offsets.resize( TILES_TOTAL*BANDS );
   tile_byte_counts.resize( TILES_TOTAL*BANDS );
   minBands.resize( BANDS );
   maxBands.resize( BANDS );

   ossim_int64 ossimTileIndex    = 0;
   ossim_int64 tiffTileIndex     = 0;
   ossim_int64 tileSizeInBytes   = 0;
   ossim_int64 bandOffsetInBytes = 0;
   std::streamoff startPos = m_str->tellp();

   while ( ossimTileIndex < TILES_TOTAL )
   {
      ossimRefPtr<ossimImageData> id = theInputConnection->getNextTile();
      if(!id)
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " ERROR:"
            << "Error returned writing tiff tile:  " << ossimTileIndex
            << "\nNULL Tile from input encountered"
            << std::endl;
         return false;
      }

      if ( ossimTileIndex == 0 )
      {
         tileSizeInBytes = (ossim_int64)id->getSizePerBandInBytes();
         bandOffsetInBytes = tileSizeInBytes * TILES_TOTAL;
      }

      // Compute running min, max.
      id->computeMinMaxPix(minBands, maxBands);

      // Band loop.
      for (ossim_int32 band=0; band < BANDS; ++band)
      {
         // Grab a pointer to the tile for the band.
         const char* data = (const char*)id->getBuf(band);
         
         // Compress data here(future maybe, i.e. jpeg, j2k...

         //---
         // Write the tile.
         // Note: tiles laid out, all the red tiles, all the green tiles all the
         // blue tiles.
         //---
         if(data)
         {
            // Compute the stream position:
            std::streamoff pos = startPos + ossimTileIndex * tileSizeInBytes +
               band * bandOffsetInBytes;
            m_str->seekp( pos );

            if ( m_str->good() )
            {
               tiffTileIndex = ossimTileIndex + band * TILES_TOTAL;
               tile_offsets[ tiffTileIndex ] = (ossim_uint64)pos;
               tile_byte_counts[ tiffTileIndex ] = (ossim_uint64)tileSizeInBytes;
               
               // Write the tile to stream:
               m_str->write( data, (std::streamsize)tileSizeInBytes);
               
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

         }
         else
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << MODULE << " ERROR:\nNull input tile:  " << ossimTileIndex
               << std::endl;
            return false;
         }
         
      } // End of band loop.

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

   if (traceDebug()) CLOG << " Exited." << std::endl;
   
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

ossim_uint16 ossimWriter::getTiffSampleFormat() const
{
   ossim_uint16 result = 0;
   switch( theInputConnection->getOutputScalarType() )
   {
      case OSSIM_UINT8:
      case OSSIM_USHORT11:
      case OSSIM_UINT16:
      case OSSIM_UINT32:
         result = ossim::OSAMPLEFORMAT_UINT;
         break;

      case OSSIM_SINT16:
         result = ossim::OSAMPLEFORMAT_INT;
         break;

      case OSSIM_FLOAT32:
      case OSSIM_FLOAT64:
      case OSSIM_NORMALIZED_FLOAT:
      case OSSIM_NORMALIZED_DOUBLE:
         result = ossim::OSAMPLEFORMAT_IEEEFP;
         break;

      default:
         break;
   }

   return result;
}

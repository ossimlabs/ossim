//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//
// Motion Imagery File TRE(MTIMFA) class  definition.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 15 for more info.
//
//---
// $Id

#include <ossim/support_data/ossimNitfMtimfaTag.h>
#include <ossim/base/ossimString.h>
#include <cstring>
#include <iomanip>
#include <iostream>

void ossimNitfMtimfaCameraBlock::parseStream(std::istream& in)
{
   clearFields();

   in.read(m_startTimestamp, START_TIMESTAMP_SIZE);
   in.read(m_endTimestamp, END_TIMESTAMP_SIZE);
   in.read(m_imageSegIndex, IMAGE_SEG_INDEX_SIZE);   
}

void ossimNitfMtimfaCameraBlock::writeStream(std::ostream& out) const
{
   out.write(m_startTimestamp, START_TIMESTAMP_SIZE);
   out.write(m_endTimestamp, END_TIMESTAMP_SIZE);
   out.write(m_imageSegIndex, IMAGE_SEG_INDEX_SIZE);
}

std::ostream& ossimNitfMtimfaCameraBlock::print( std::ostream& out,
                                                 const std::string& prefix,
                                                 ossim_uint32 index) const
{
   std::string pfx = prefix;
   pfx += "TEMPORAL_BLOCK";
   pfx += ossimString::toString(index).string();
   pfx += ".";
   
   const ossim_int32 W = 17;
   
   out << setiosflags(std::ios::left)
       << pfx << std::setw(W) << "START_TIMESTAMP:" << ossimString(m_startTimestamp).trim() << "\n"
       << pfx << std::setw(W) << "END_TIMESTAMP:" << ossimString(m_endTimestamp).trim() << "\n"
       << pfx << std::setw(W) << "IMAGE_SEG_INDEX:" << m_imageSegIndex << "\n";

   return out;
}

void ossimNitfMtimfaCameraBlock::clearFields()
{
   std::memset( m_startTimestamp, ' ', START_TIMESTAMP_SIZE );
   std::memset( m_endTimestamp, ' ', END_TIMESTAMP_SIZE );
   std::memset( m_imageSegIndex, '0',  IMAGE_SEG_INDEX_SIZE);

   m_startTimestamp[START_TIMESTAMP_SIZE] = '\0';
   m_endTimestamp[START_TIMESTAMP_SIZE] = '\0';
   m_imageSegIndex[IMAGE_SEG_INDEX_SIZE] = '\0';   
}

void ossimNitfMtimfaCamera::parseStream(std::istream& in)
{
   clearFields();

   in.read(m_cameraId, CAMERAS_ID_SIZE);
   in.read(m_numTempBlocks, NUM_TEMP_BLOCS_SIZE);

   ossim_uint32 count = ossimString(m_numTempBlocks).toUInt32();
   if ( count )
   {
      m_cameraBlocks.resize( count );
      for ( auto&& i : m_cameraBlocks )
      {
         i.parseStream( in );
      }
   }
}

void ossimNitfMtimfaCamera::writeStream(std::ostream& out) const
{
   out.write(m_cameraId, CAMERAS_ID_SIZE);
   out.write(m_numTempBlocks, NUM_TEMP_BLOCS_SIZE);
   
   const auto& cv = m_cameraBlocks;
   for ( auto&& i : cv )
   {
      i.writeStream( out );
   }
}

std::ostream& ossimNitfMtimfaCamera::print( std::ostream& out,
                                            const std::string& prefix,
                                            ossim_uint32 index) const
{
   std::string pfx = prefix;
   pfx += "CAMERA";
   pfx += ossimString::toString(index).string();
   pfx += ".";

   const ossim_int32 W = 33;

   out << setiosflags(std::ios::left)
       << pfx << std::setw(W) << "CAMERAS_ID:" << ossimString(m_cameraId).trim() << "\n"
       << pfx << std::setw(W) << "NUM_TEMP_BLOCKS:" << m_numTempBlocks << "\n";

   for (ossim_uint32 i = 0; i < (ossim_uint32)m_cameraBlocks.size(); ++i )
   {
      m_cameraBlocks[i].print(out, pfx, i);
   } 

   return out;
}

void ossimNitfMtimfaCamera::clearFields()
{
   std::memset( m_cameraId, ' ', CAMERAS_ID_SIZE );
   std::memset( m_numTempBlocks, '0', NUM_TEMP_BLOCS_SIZE);
   
   m_cameraId[CAMERAS_ID_SIZE] = '\0';
   m_numTempBlocks[NUM_TEMP_BLOCS_SIZE] = '\0';

   m_cameraBlocks.clear();
}

// For ossimRtti stuff...
RTTI_DEF1(ossimNitfMtimfaTag, "ossimNitfMtimfaTag", ossimNitfRegisteredTag);

ossimNitfMtimfaTag::ossimNitfMtimfaTag()
   : ossimNitfRegisteredTag(std::string("MTIMFA"), 0)
{
   clearFields();
}

void ossimNitfMtimfaTag::parseStream(std::istream& in)
{
   clearFields();

   in.read(m_layerId, LAYER_ID_SIZE);
   in.read(m_cameraSetIndex, CAMERA_SET_INDEX_SIZE);
   in.read(m_timeIntervalIndex, TIME_INTERVAL_INDEX_SIZE);   
   in.read(m_numCamerasDefined, NUM_CAMERAS_DEFINED_SIZE);

   ossim_uint32 count = ossimString(m_numCamerasDefined).toUInt32();
   if ( count )
   {
      m_camera.resize( count );
      for ( auto&& i : m_camera )
      {
         i.parseStream( in );
      }
   }
}

void ossimNitfMtimfaTag::writeStream(std::ostream& out)
{
   out.write(m_layerId, LAYER_ID_SIZE);
   out.write(m_cameraSetIndex, CAMERA_SET_INDEX_SIZE);
   out.write(m_timeIntervalIndex, TIME_INTERVAL_INDEX_SIZE);
   out.write(m_numCamerasDefined, NUM_CAMERAS_DEFINED_SIZE);   

   const auto& cv = m_camera;
   for (auto&& i : cv)
   {
      i.writeStream( out );
   }
}

void ossimNitfMtimfaTag::clearFields()
{
   std::memset(m_layerId, ' ', LAYER_ID_SIZE);
   std::memset(m_cameraSetIndex, '0', CAMERA_SET_INDEX_SIZE);
   std::memset(m_timeIntervalIndex, '0', TIME_INTERVAL_INDEX_SIZE);
   std::memset(m_numCamerasDefined, '0', NUM_CAMERAS_DEFINED_SIZE);

   m_layerId[LAYER_ID_SIZE] = '\0';
   m_cameraSetIndex[CAMERA_SET_INDEX_SIZE] = '\0';
   m_timeIntervalIndex[TIME_INTERVAL_INDEX_SIZE] = '\0';
   m_numCamerasDefined[NUM_CAMERAS_DEFINED_SIZE] = '\0';   

   m_camera.clear();
}

std::ostream& ossimNitfMtimfaTag::print( std::ostream& out,
                                         const std::string& prefix ) const
{
   std::string pfx = prefix;
   pfx += "MTIMFA.";

   const ossim_int32 W = 41;

   out << setiosflags(std::ios::left)
       << pfx << std::setw(W) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(W) << "CEL:" << getTagLength() << "\n"
       << pfx << std::setw(W) << "LAYER_ID: " << m_layerId << "\n"
       << pfx << std::setw(W) << "CAMERA_SET_INDEX: " << m_cameraSetIndex << "\n"
       << pfx << std::setw(W) << "TIME_INTERVAL_INDEX: " << m_timeIntervalIndex << "\n"
       << pfx << std::setw(W) << "NUM_CAMERAS_DEFINED: " << m_numCamerasDefined << "\n";

   for (ossim_uint32 i = 0; i < (ossim_uint32)m_camera.size(); ++i )
   {
      m_camera[i].print(out, pfx, i);
   } 
   
   return out;
}

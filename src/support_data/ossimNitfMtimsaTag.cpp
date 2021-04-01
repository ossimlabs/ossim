//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//
// Motion Imagery Segment TRE(MTIMSA) class definition.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 18 for more info.
// Contains binary data, see document NGA.STND.0044_1.3_MIE4NITF,
// table 8 for more info.
//---
// $Id

#include <ossim/support_data/ossimNitfMtimsaTag.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimString.h>
#include <cstring>
#include <iomanip>
#include <iostream>

// For ossimRtti stuff...
RTTI_DEF1(ossimNitfMtimsaTag, "ossimNitfMtimsaTag", ossimNitfRegisteredTag);

ossimNitfMtimsaTag::ossimNitfMtimsaTag()
   : ossimNitfRegisteredTag(std::string("MTIMSA"), 0)
{
   clearFields();
}

void ossimNitfMtimsaTag::parseStream(std::istream& in)
{
   clearFields();

   // Binary data stored in big endian in file.
   ossimEndian* endian = 0;
   if ( ossim::byteOrder() != OSSIM_BIG_ENDIAN )
   {
      endian = new ossimEndian();
   }
   

   in.read(m_imageSegIndex, IMAGE_SEG_INDEX_SIZE);
   in.read(m_geocoordsStatic, GEOCOORDS_STATIC_SIZE);
   in.read(m_layerId, LAYER_ID_SIZE);
   in.read(m_cameraSetIndex, CAMERA_SET_INDEX_SIZE);   
   in.read(m_cameraId, CAMERA_ID_SIZE);
   in.read(m_timeIntervalIndex, TIME_INTERVAL_INDEX_SIZE);
   in.read(m_tempBlockIndex, TEMP_BLOCK_INDEX_SIZE);
   in.read(m_nominalFrameRate, NOMINAL_FRAME_RATE_SIZE);   
   in.read(m_referenceFrameNum, REFERENCE_FRAME_NUM_SIZE);
   in.read(m_baseTimestamp, BASE_TIMESTAMP_SIZE);

   // Binary data:
   in.read((char*)&m_dtMultiplier, DT_MULTIPLIER_SIZE);
   in.read((char*)&m_dtSize, DT_SIZE);   
   in.read((char*)&m_numberFrames, NUMBER_OF_FRAMES_SIZE);
   in.read((char*)&m_numberDt, NUMBER_DT_SIZE);
   if ( endian )
   {
      endian->swap( m_dtMultiplier );
      endian->swap( m_numberFrames );
      endian->swap( m_numberDt );

      delete endian;
      endian = 0;
   }

   ossim_uint32 bytes = m_numberDt * m_dtSize;
   m_dt.resize(bytes);
   in.read((char*)m_dt.data(), bytes); // DTs
}

void ossimNitfMtimsaTag::writeStream(std::ostream& out)
{
   // Binary data stored in big endian in file.
   ossimEndian* endian = 0;
   if ( ossim::byteOrder() != OSSIM_BIG_ENDIAN )
   {
      endian = new ossimEndian();
   }
    
   out.write(m_imageSegIndex, IMAGE_SEG_INDEX_SIZE);
   out.write(m_geocoordsStatic, GEOCOORDS_STATIC_SIZE);
   out.write(m_layerId, LAYER_ID_SIZE);
   out.write(m_cameraSetIndex, CAMERA_SET_INDEX_SIZE);   
   out.write(m_cameraId, CAMERA_ID_SIZE);
   out.write(m_timeIntervalIndex, TIME_INTERVAL_INDEX_SIZE);
   out.write(m_tempBlockIndex, TEMP_BLOCK_INDEX_SIZE);
   out.write(m_nominalFrameRate, NOMINAL_FRAME_RATE_SIZE);   
   out.write(m_referenceFrameNum, REFERENCE_FRAME_NUM_SIZE);
   out.write(m_baseTimestamp, BASE_TIMESTAMP_SIZE);

   // Binary data:
   ossim_uint64 dtMultiplier = m_dtMultiplier;
   ossim_uint32 numberFrames = m_numberFrames;
   ossim_uint32 numberDt = m_numberDt;
   if (endian)
   {
      endian->swap(dtMultiplier);
      endian->swap(numberFrames);
      endian->swap(numberDt);

      delete endian;
      endian = 0;
   }
   out.write((char*)&m_dtMultiplier, DT_MULTIPLIER_SIZE);
   out.write((char*)&m_dtSize, DT_SIZE);
   out.write((char*)&m_numberFrames, NUMBER_OF_FRAMES_SIZE);
   out.write((char*)&m_numberDt, NUMBER_DT_SIZE);

   ossim_uint32 bytes = m_numberDt * m_dtSize;
   out.write((char*)m_dt.data(), bytes); // DTs
}

void ossimNitfMtimsaTag::clearFields()
{
   std::memset(m_imageSegIndex, '0', IMAGE_SEG_INDEX_SIZE);
   std::memset(m_geocoordsStatic, '0', GEOCOORDS_STATIC_SIZE);
   std::memset(m_layerId, ' ', LAYER_ID_SIZE);
   std::memset(m_cameraSetIndex, '0', CAMERA_SET_INDEX_SIZE);   
   std::memset(m_cameraId, ' ', CAMERA_ID_SIZE);
   std::memset(m_timeIntervalIndex, '0', TIME_INTERVAL_INDEX_SIZE);
   std::memset(m_tempBlockIndex, '0', TEMP_BLOCK_INDEX_SIZE);
   std::memcpy(m_nominalFrameRate, "Nan          ", NOMINAL_FRAME_RATE_SIZE);   
   std::memset(m_referenceFrameNum, ' ', REFERENCE_FRAME_NUM_SIZE);
   std::memset(m_baseTimestamp, ' ', BASE_TIMESTAMP_SIZE);
   
   m_imageSegIndex[IMAGE_SEG_INDEX_SIZE] = '\0';
   m_geocoordsStatic[GEOCOORDS_STATIC_SIZE] = '\0';
   m_layerId[LAYER_ID_SIZE] = '\0';
   m_cameraSetIndex[CAMERA_SET_INDEX_SIZE] = '\0';   
   m_cameraId[CAMERA_ID_SIZE] = '\0';
   m_timeIntervalIndex[TIME_INTERVAL_INDEX_SIZE] = '\0';
   m_tempBlockIndex[TEMP_BLOCK_INDEX_SIZE] = '\0';
   m_nominalFrameRate[NOMINAL_FRAME_RATE_SIZE] = '\0';   
   m_referenceFrameNum[REFERENCE_FRAME_NUM_SIZE] = '\0';
   m_baseTimestamp[BASE_TIMESTAMP_SIZE] = '\0';

   // Binary data:
   m_dtMultiplier = 0;
   m_dtSize       = 1;
   m_numberFrames = 0;
   m_numberDt     = 0;
   m_dt.clear();
}

std::ostream& ossimNitfMtimsaTag::print( std::ostream& out,
                                         const std::string& prefix ) const
{
   std::string pfx = prefix;
   pfx += "MTIMSA.";

   const ossim_int32 W = 24;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(W) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(W) << "CEL:" << getTagLength() << "\n"
       << pfx << std::setw(W) << "IMAGE_SEG_INDEX: " << m_imageSegIndex << "\n"
       << pfx << std::setw(W) << "GEOCOORDS_STATIC: " << m_geocoordsStatic << "\n"
       << pfx << std::setw(W) << "LAYER_ID: " << m_layerId << "\n"
       << pfx << std::setw(W) << "CAMERA_SET_INDEX: " << m_cameraSetIndex << "\n"
       << pfx << std::setw(W) << "CAMERA_ID: " << m_cameraId << "\n"
       << pfx << std::setw(W) << "TIME_INTERVAL_INDEX: " << m_timeIntervalIndex << "\n"
       << pfx << std::setw(W) << "TEMP_BLOCK_INDEX: " << m_tempBlockIndex << "\n"
       << pfx << std::setw(W) << "NOMINAL_FRAME_RATE: " << m_nominalFrameRate << "\n"
       << pfx << std::setw(W) << "REFERENCE_FRAME_NUM: " << m_referenceFrameNum << "\n"
       << pfx << std::setw(W) << "BASE_TIMESTAMP: " << m_baseTimestamp << "\n"
       << pfx << std::setw(W) << "DT_MULTIPLIER: " << m_dtMultiplier << "\n"
       << pfx << std::setw(W) << "DT_SIZE: " << (int)m_dtSize << "\n"
       << pfx << std::setw(W) << "NUMBER_FRAMES: " << m_numberFrames << "\n"
       << pfx << std::setw(W) << "NUMBER_DT: " << m_numberDt << "\n"            
       << pfx << std::setw(W) << "DTn: " << "COMING SOON!" << "\n";
   
   return out;
}

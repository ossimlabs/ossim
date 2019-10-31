//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//
// Motion Imagery Segment TRE(MTIMSA) class declaration.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 18 for more info.
// Contains binary data, see document NGA.STND.0044_1.3_MIE4NITF,
// table 8 for more info.
//
//---
// $Id

#ifndef ossimNitfMtimsaTag_HEADER
#define ossimNitfMtimsaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <string>
#include <vector>

class OSSIM_DLL ossimNitfMtimsaTag : public ossimNitfRegisteredTag
{
public:

   enum
   {
      IMAGE_SEG_INDEX_SIZE = 3,
      GEOCOORDS_STATIC_SIZE = 2,
      LAYER_ID_SIZE = 36,
      CAMERA_SET_INDEX_SIZE = 3,
      CAMERA_ID_SIZE = 36,
      TIME_INTERVAL_INDEX_SIZE = 6,
      TEMP_BLOCK_INDEX_SIZE = 3,
      NOMINAL_FRAME_RATE_SIZE = 13,
      REFERENCE_FRAME_NUM_SIZE = 9,
      BASE_TIMESTAMP_SIZE = 24,
      DT_MULTIPLIER_SIZE = 8,
      DT_SIZE = 1,
      NUMBER_OF_FRAMES_SIZE = 4,
      NUMBER_DT_SIZE = 4
   };

   /** @brief default constructor */
   ossimNitfMtimsaTag();
   
   /** @brief Method to parse data from stream. */
   virtual void parseStream(std::istream& in);

   /** @brief Method to write data to stream. */
   virtual void writeStream(std::ostream& out);

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const;
   
protected:

   /** @brief Method to clear all fields including null terminating. */
   virtual void clearFields();

   /**
    * FIELD: IMAGE_SEG_INDEX
    *
    * The image segment index of this NITF Image Segment in the NITF file
    * containing this camera andtemporal block.
    * 
    * 3 bytes
    *
    * BCS-N, positive integer 001 – 999
    */
   char m_imageSegIndex[IMAGE_SEG_INDEX_SIZE+1];

   /**
    * FIELD: GEOCOORDS_STATIC
    *
    * A flag that indicated whether or not the geo-coordinates associated with
    * this NITF Image Segment apply to all frames in the Image Segment.
    * 
    * 2 bytes
    *
    * BCS-N, positive integer 00 or 99.  00 – Geocoordinates are not static,
    * 99 – Geocoordinates are static.
    */
   char m_geocoordsStatic[GEOCOORDS_STATIC_SIZE+1];
   
   /**
    * FIELD: LAYER_ID
    *
    * The token of the phenomenological layer of the camera whose MI data is
    * contained in this NITF Image Segment. A 36 character string identifying
    * the layer. Field is large enough to hold a UUID.
    * 
    * 36 bytes
    *
    * BCS-A May only be spaces filled for Quick-Look image.
    */
   char m_layerId[LAYER_ID_SIZE+1];

   /**
    * FIELD: CAMERA_SET_INDEX
    *
    * The index of the camera set containing the camera whose MI data is
    * contained in this NITF Image Segment.
    * 
    * 3 bytes
    *
    * BCS-N, positive integer 001 – 999, May be 000 for Quick-Look image.
    */
   char m_cameraSetIndex[CAMERA_SET_INDEX_SIZE+1];

   /**
    * FIELD: CAMERA_ID
    *
    * The UUID of the camera whose MI data is contained in this NITF Image
    * Segment.
    * 
    * 36 bytes
    *
    * BCS-A, A valid UUID, May only be spaces filled for a Quick-Look image.
    */
   char m_cameraId[CAMERA_ID_SIZE+1];

   /**
    * FIELD: TIME_INTERVAL_INDEX
    *
    * The index of the time interval corresponding to the NITF file containing
    * this NITF Image Segment.
    * 
    * 6 bytes
    *
    * BCS-N, 000001 – 999999, May be 000000 for a QuickLook image.
    */
   char m_timeIntervalIndex[TIME_INTERVAL_INDEX_SIZE+1];

   /**
    * FIELD: TEMP_BLOCK_INDEX
    *
    * The index of the temporal block corresponding this NITF Image Segment.
    * 
    * 3 bytes
    *
    * BCS-N, positive integer 001 – 999, May be 000 for a Quick-Look image.
    */
   char m_tempBlockIndex[TEMP_BLOCK_INDEX_SIZE+1];

   /**
    * FIELD: NOMINAL_FRAME_RATE
    *
    * The nominal frame rate of MI data in frames/second.
    * 
    * 13 bytes
    *
    * UE/13 0.0000000E+00 to 3.4028234E+38, or NaN
    */
   char m_nominalFrameRate[NOMINAL_FRAME_RATE_SIZE+1];

   /**
    * FIELD: REFERENCE_FRAME_NUM
    *
    * The absolute frame number of the first frame in this temporal block for
    * this camera as determined by the collection system.
    * 
    * 9 bytes
    *
    * BCS-A spaces or BCS-N, positive integer 000000001 – 999999999
    */
   char m_referenceFrameNum[REFERENCE_FRAME_NUM_SIZE+1];

   /**
    * FIELD: BASE_TIMESTAMP
    *
    * The base timestamp from which the timestamps for the frames in this
    * temporal block is derived.
    * 
    * 24 bytes
    *
    * BCS-A, UTC format
    */
   char m_baseTimestamp[BASE_TIMESTAMP_SIZE+1];
   
   /**
    * FIELD: DT_MULTIPLIER 
    *
    * The number of nanoseconds equal to one time unit, or minimum “delta time”
    * that can be expressed between frames.
    * 
    * 8 bytes
    *
    * UINT64, 1 – 2^64-1, stored big endian byte order in file.
    */
   ossim_uint64 m_dtMultiplier;

   /**
    * FIELD: DT_SIZE
    *
    * The size in bytes of the DTn values.
    * 
    * 1 byte
    *
    * UINT8, 0x01 – 0x08 (1 – 8)
    */
   ossim_uint8 m_dtSize;

   /**
    * FIELD: NUMBER_FRAMES
    *
    * The number of frames in this Image Segment for this camera and temporal
    * block.
    * 
    * 4 bytes
    *
    * UINT32, 0 – 2^32-1, stored big endian byte order in file.
    */
   ossim_uint32 m_numberFrames;
   
   /**
    * FIELD: NUMBER_DT
    *
    * The number of delta time values.
    * 
    * 4 bytes
    *
    * UINT32, 0 – 2^32-1, stored big endian byte order in file.
    */
   ossim_uint32 m_numberDt;

   /**
    * FIELD: DT
    *
    * The number of delta time units. Number of bytes per DTn value is
    * determined by DT_SIZE. One to eight bytes allowed.
    * 
    * bytes variable
    *
    * UINTn, where n = 8xDT_SIZE 0 – 2^(8 x DT_SIZE) -1
    */
   std::vector<ossim_uint8> m_dt;
   
TYPE_DATA
   
}; // End: class ossimNitfMtimsaTag

#endif /* matches #ifndef ossimNitfMtimsaTag_HEADER */

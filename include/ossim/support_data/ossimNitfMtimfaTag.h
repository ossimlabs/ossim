//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//
// Motion Imagery File TRE(MTIMFA) class declaration.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 15 for more info.
//
//---
// $Id

#ifndef ossimNitfMtimfaTag_HEADER
#define ossimNitfMtimfaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <string>
#include <vector>

/**
 * @class ossimNitfMtimfaCameraBlock
 *
 * Camera temporal block only portion of MTIMFA tag.
 */
class ossimNitfMtimfaCameraBlock
{
public:
   enum
   {
      START_TIMESTAMP_SIZE = 24,
      END_TIMESTAMP_SIZE   = 24,
      IMAGE_SEG_INDEX_SIZE = 3
   };

   void parseStream(std::istream& in);
   
   void writeStream(std::ostream& out) const;
   
   std::ostream& print( std::ostream& out,
                        const std::string& prefix,
                        ossim_uint32 index ) const;

private:

   void clearFields();

   /**
    * FIELD: START_TIMESTAMP
    *
    * Start time of m'th temporal block for n'th camera.
    * 
    * 24 bytes
    * 
    * BCS-A, Spaces or UTC format
    *
    * Year, month, day, hour, minute, seconds, nanosecs
    */
   char m_startTimestamp[START_TIMESTAMP_SIZE+1];

   /**
    * FIELD: END_TIMESTAMP
    *
    * End time of m'th temporal block for n'th camera.
    * 
    * 24 bytes
    * 
    * BCS-A, Spaces or UTC format
    *
    * Year, month, day, hour, minute, seconds, nanosecs
    */
   char m_endTimestamp[END_TIMESTAMP_SIZE+1];

   /**
    * FIELD: IMAGE_SEG_INDEX
    *
    * Index of the NITF Image Segment in this NITF file that contains the m'th
    * temporal block of MI data for the n'th camera.
    * 
    * 3 bytes
    *
    * BCS-A spaces or BCS-N, positive integer 001 – 999
    */
   char m_imageSegIndex[IMAGE_SEG_INDEX_SIZE+1];
   
}; // End: class ossimNitfMtimfaCameraBlock

/**
 * @class ossimNitfMtimfaCamera
 *
 * Camera ID only portion of MTIMFA tag.
 */
class ossimNitfMtimfaCamera
{
public:
   enum
   {
      CAMERAS_ID_SIZE     = 36,
      NUM_TEMP_BLOCS_SIZE = 3
   };

   void parseStream(std::istream& in);
   
   void writeStream(std::ostream& out) const;
   
   std::ostream& print( std::ostream& out,
                        const std::string& prefix,
                        ossim_uint32 index ) const;

private:

   void clearFields();

   /**
    * FIELD: CAMERAS_ID
    *
    * The UUID of the n'th camera in this phenomenological layer in this camera
    * set. See NGA.RP.0001_1.0.0 for UUID details.
    * 
    * 36 bytes
    *
    * BCS-A, A valid UUID in the “8-4-4-4-12” hexadecimal format
    * (see ITU-T X.667)
    */
   char m_cameraId[CAMERAS_ID_SIZE+1];

   /**
    * FIELD: NUM_TEMP_BLOCKS
    *
    * The number of temporal blocks defined for the n'th camera in this time
    * interval.
    * 
    * 3 bytes
    * 
    * BCS-N, positive integer 001 – 999
    */
   char m_numTempBlocks[NUM_TEMP_BLOCS_SIZE+1];

   /**
    * Holds an array of camera temporal blocks.
    */
   std::vector<ossimNitfMtimfaCameraBlock> m_cameraBlocks;
   
}; // End: class ossimNitfMtimfaCamera

class OSSIM_DLL ossimNitfMtimfaTag : public ossimNitfRegisteredTag
{
public:

   enum
   {
      LAYER_ID_SIZE = 36,
      CAMERA_SET_INDEX_SIZE = 3,
      TIME_INTERVAL_INDEX_SIZE = 6,
      NUM_CAMERAS_DEFINED_SIZE = 3
   };

   /** @brief default constructor */
   ossimNitfMtimfaTag();
   
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
    * FIELD: LAYER_ID
    *
    * A 36 character string identifying the layer. Field is large enough to
    * hold a UUID.
    * 
    * 36 bytes
    *
    * BCS-A
    */
   char m_layerId[LAYER_ID_SIZE+1];

   /**
    * FIELD: CAMERA_SET_INDEX
    *
    * The index of the camera set containing the cameras in this NITF file.
    * 
    * 3 bytes
    *
    * BCS-N, 001 – 999
    */
   char m_cameraSetIndex[CAMERA_SET_INDEX_SIZE+1];

   /**
    * FIELD: TIME_INTERVAL_INDEX
    *
    * The index of the time interval corresponding to this NITF file.
    * 
    * 6 bytes
    *
    * BCS-N, 000001 – 999999
    */
   char m_timeIntervalIndex[TIME_INTERVAL_INDEX_SIZE+1];

   /**
    * FIELD: NUM_CAMERAS_DEFINED
    *
    * The number of cameras in this camera set for this phenomenological layer
    * that collected MI data in this time interval.
    * 
    * 3 bytes
    *
    * BCS-N, positive integer 001 – 999
    */
   char m_numCamerasDefined[NUM_CAMERAS_DEFINED_SIZE+1];
   
   /**
    * Holds an array of cameras.
    */
   std::vector<ossimNitfMtimfaCamera> m_camera;
   
TYPE_DATA
   
}; // End: class ossimNitfMtimfaTag

#endif /* matches #ifndef ossimNitfMtimfaTag_HEADER */

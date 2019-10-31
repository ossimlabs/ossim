//---
//
// License:  MIT
//
// Author:  David Burken
//
// Description:
//
// Motion Imagery Collection Summary TRE(MIMCSA) class declaration.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 11 for more info.
//
//---
// $Id

#ifndef ossimNitfMimcsaTag_HEADER
#define ossimNitfMimcsaTag_HEADER 1
#include <ossim/support_data/ossimNitfRegisteredTag.h>

class OSSIM_DLL ossimNitfMimcsaTag : public ossimNitfRegisteredTag
{
public:

   enum
   {
      LAYER_ID_SIZE           = 36,
      NOMINAL_FRAME_RATE_SIZE = 13,
      MIN_FRAME_RATE_SIZE     = 13,
      MAX_FRAME_RATE_SIZE     = 13,
      T_RSET_SIZE             = 2,
      MI_REQ_DECODER_SIZE     = 2,
      MI_REQ_PROFILE_SIZE     = 36,      
      MI_REQ_LEVEL_SIZE       = 6
      //                       -----
      //                        121 bytes
   };
   
   /** @brief default constructor */
   ossimNitfMimcsaTag();

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
   
   /**
    * @brief Get the LAYER_ID field.
    * @return The LAYER_ID field as a string.
    */
   ossimString getLayerId() const;
   
   /**
    * @brief Get the NOMINAL_FRAME_RATE field.
    * @return The NOMINAL_FRAME_RATE field as a string.
    */
   ossimString getNominalFrameRate() const;

   /**
    * @brief Get the MIN_FRAME_RATE field.
    * @return The MIN_FRAME_RATE field as a string.
    */
   ossimString getMinFrameRate() const;

   /**
    * @brief Get the MAX_FRAME_RATE field.
    * @return The MAX_FRAME_RATE field as a string.
    */
   ossimString getMaxFrameRate() const;

   /**
    * @brief Get the T_RSET field.
    * @return The T_RSET field as a string.
    */
   ossimString getTRset() const;

   /**
    * @brief Get the MI_REQ_DECODER field.
    * @return The MI_REQ_DECODER field as a string.
    */
   ossimString getMiReqDecoder() const;

   /**
    * @brief Get the MI_REQ_PROFILE field.
    * @return The MI_REQ_PROFILE field as a string.
    */
   ossimString getMiReqProfile() const;

   /**
    * @brief Get the MI_REQ_LEVEL field.
    * @return The MI_REQ_LEVEL field as a string.
    */
   ossimString getMiReqLevel() const;

protected:

   /** @brief Method to clear all fields including null terminating. */
   virtual void clearFields();

   /**
    * FIELD: LAYER_ID
    *
    * A 36 character string identifying the layer. Field is large enough to
    * hold a UUID.
    * 
    * 36 byte field BCS-A
    */
   char m_layerId[LAYER_ID_SIZE+1];
   
   /**
    * FIELD: NOMINAL_FRAME_RATE
    *
    * The nominal frame rate of MI data in frames/second.
    * 
    * 13 byte field UE/13
    *
    * 0.0000000E+00 to 3.4028234E+38 or “NaN” followed by 10 BCS-A spaces
    */
   char m_nominalFrameRate[MIN_FRAME_RATE_SIZE+1];

   /**
    * FIELD: MIN_FRAME_RATE
    *
    * The minimum frame rate of MI data in frames/second.
    * 
    * 13 byte field UE/13
    *
    * 0.0000000E+00 to 3.4028234E+38 or “NaN” followed by 10 BCS-A spaces
    */
   char m_minFrameRate[MIN_FRAME_RATE_SIZE+1];

   /**
    * FIELD: MAX_FRAME_RATE
    *
    * The maximum frame rate of MI data in frames/second.
    *
    * 13 byte field UE/13
    *
    * 0.0000000E+00 to 3.4028234E+38 or “NaN” followed by 10 BCS-A spaces
    */
   char m_maxFrameRate[MAX_FRAME_RATE_SIZE+1];

   /**
    * FIELD: T_RSET
    *
    * Temporal rset of the MI data. The value 00 indicates the original
    * temporal rate. 
    *
    * 2 byte field BCS-N positive integer 00 – 99
    */
   char m_tRset[T_RSET_SIZE+1];

   /**
    * FIELD: MI_REQ_DECODER
    *
    * The IC field value from the NITF Image Subheader.
    *
    * 2 byte field BCS-A Any legal IC field value
    */
   char m_miReqDecoder[MI_REQ_DECODER_SIZE+1];

   /**
    * FIELD: MI_REQ_PROFILE
    *
    * The name of the profile of the compression standard being used.
    *
    * 36 byte field BCS-A Any legal IC field value
    */
   char m_miReqProfile[MI_REQ_PROFILE_SIZE+1];

   /**
    * FIELD: MI_REQ_LEVEL
    *
    * The name/value of the level of the profile of the compression standard
    * being used.
    *
    * 6 byte field BCS-A Any legal IC field value
    */
   char m_miReqLevel[MI_REQ_LEVEL_SIZE+1];
   
TYPE_DATA   
};

#endif /* matches #ifndef ossimNitfMimcsaTag_HEADER */

//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: class declaration for base codec(encoder/decoder).
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimJpegCodec_HEADER
#define ossimJpegCodec_HEADER 1
#include <ossim/imaging/ossimCodecBase.h>

class OSSIM_DLL ossimJpegCodec : public ossimCodecBase
{
public:
   
   ossimJpegCodec();
   virtual ~ossimJpegCodec();

   /**
    * @return Codec type.  "jpeg"
    */  
   virtual ossimString getCodecType()const;
   
   /**
    * @brief Encode jpeg method.
    *
    * Current options handled by this factory:
    *
    * type: jpeg
    * quality: 75
    *
    * @param in Input data to encode.
    * 
    * @param out Encoded output data.
    *
    * @return true on success, false on failure.
    */   
   virtual bool encode( const ossimRefPtr<ossimImageData>& in,
                        std::vector<ossim_uint8>& out ) const;

   /**
    * @brief Decode jpeg method.
    *
    * @param in Input data to decode.
    * 
    * @param out Output tile.  If the pointer to ossimImageData is null
    * internally it will be created.  For code loops it is better to pre
    * initialized to correct size.
    *
    * @note Caller should set "out's" image rectangle upon successful
    * decode.
    *
    * @return true on success, false on failure.
    */
   virtual bool decode( const std::vector<ossim_uint8>& in,
                        ossimRefPtr<ossimImageData>& out ) const;

   /**
    * Ineterface to allow for specific properties to be set.
    *
    */ 
   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   
   /**
    * Interface to get the value of a specific property
    *
    * @param in name.  Property name to retrieve
    *
    * @return property value
    */
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const; 
   
   /**
    * Get a list of all supported property names.  Currently only "quality" property 
    * is exposed
    *
    * @param out proeprtyNames.  push the list of proeprty names to the list
    */
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;
   
   /**
    * Allocate the state of the object thorugh a factory load/keywordlist
    *
    * @param in kwl. Input keywordlist that holds the values.
    *
    * @param in prefix. prefix to use for all keywords.
    */
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);
   
   /**
    * Save the state of the codec to the keywordlist
    *
    * @param out kwl. Input keywordlist that holds the values.
    *
    * @param in prefix. prefix to use for all keywords.
    */
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;
   
protected:

   /**
    * @brief For decoding standard jpeg block.
    * Curently only handles CMYK.
    * @param in Input jpeg buffer.
    * @param out Initialized by this.
    * @return true on success, false on error.
    */
   bool decodeJpeg(const std::vector<ossim_uint8>& in,
                   ossimRefPtr<ossimImageData>& out ) const;
   
   /**
    * @brief For decoding color spaces other that mono and rgb.
    * Curently only handles CMYK.
    * @param in Input jpeg buffer.
    * @param out Initialized by this.
    * @return true on success, false on error.
    */
   bool decodeJpegToRgb(const std::vector<ossim_uint8>& in,
                        ossimRefPtr<ossimImageData>& out ) const;

   /**
    * @return The enumerated libjpeg out_color_space.
    */
   ossim_int32 getColorSpace( const std::vector<ossim_uint8>& in ) const;
   
   ossim_uint32 m_quality;
   TYPE_DATA;
};

#endif

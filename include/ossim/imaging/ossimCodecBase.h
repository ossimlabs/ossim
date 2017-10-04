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
#ifndef ossimCodecBase_HEADER
#define ossimCodecBase_HEADER 1

#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimPropertyInterface.h>
#include <ossim/imaging/ossimImageData.h>
class OSSIM_DLL ossimCodecBase
   : public ossimObject, public ossimPropertyInterface
{
public:

    /**
    * Will return the identifier used to identify the codec type.  For example the Jpeg codec
    * will have "jpeg" as the identifier
    *
    * @return Codec identifier
    */
	virtual ossimString getCodecType()const=0;

   /**
    * @brief Encode method.
    *
    * Pure virtual method that encodes the passed in buffer to this codec.
    *
    * @param in Input data to encode.
    * 
    * @param out Encoded output data.
    *
    * @return true on success, false on failure.
    */

   virtual bool encode( const ossimRefPtr<ossimImageData>& in,
                        std::vector<ossim_uint8>& out ) const=0;

  /**
    * @brief Decode method.
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
                        ossimRefPtr<ossimImageData>& out ) const=0;

   virtual const std::string& getExtension() const=0;
};

#endif

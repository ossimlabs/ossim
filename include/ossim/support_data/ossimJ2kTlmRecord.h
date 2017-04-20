//---
// 
// License: MIT
//
// Author: David Burken
// 
// Description: Container class declaration for J2K Tile-part lengths record.
// See document BPJ2K01.10 Table 7-21 for detailed description.
//
// $Id$
//---

#ifndef ossimJ2kTlmRecord_HEADER
#define ossimJ2kTlmRecord_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIosFwd.h>
#include <string>

class OSSIM_DLL ossimJ2kTlmRecord
{
public:
   
   /** default constructor */
   ossimJ2kTlmRecord();

   /** destructor */
   ~ossimJ2kTlmRecord();

   /**
    * Parse method.  Performs byte swapping as needed.
    *
    * @param in Stream to parse.
    *
    * @note SIZ Marker (0xff51) is not read.
    */
   void parseStream(ossim::istream& in);

   /**
    * Write method.
    *
    * Note: Write include two marker bytes.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);

   /**
    * @return Index of this marker segment relative to other marker
    * segments in the header.
    */
   ossim_uint8 getZtlm() const;
   
   /**
    * @brief Get the ST portion of STLM field( bits 5 and 6).
    * @return ST bits as an unsigned char.
    * */
   ossim_uint8 getSt() const;

   /**
    * @brief Set the ST bits of STLM field.
    *
    * @param bits Either 0, 1, or 2.
    *
    * 0 = Ttlm parameter is not present.  Tiles are in order.
    * 1 = Ttlm parameter 8 bits.
    * 2 = Ttlm parameter 16 bits.
    *
    * @return true if in range; false, if not.
    */
   bool setSt(ossim_uint8 bits );

   /** @return The higher order 4 bits of m_Stlm as an unsigned char. */
   ossim_uint8 getSp() const;

   /**
    * @brief Set the SP bit of Stlm field.
    *
    * @param bit Either 0 or 1.
    *
    * 0 = Ptlm parameter 16 bits
    * 1 = Ptlm parameter 32 bits
    *
    * @return true if in range; false, if not.
    */
   bool setSp(ossim_uint8 bit );

   /** @return The tile count derived from m_Ltml, sd and sp. */
   ossim_uint16 getTileCount() const;

   /**
    * @brief Get the tile length for tile at index from ptlm array.
    * @param index
    * @param length Initialized by this to of tile legnth or 0 out of array
    * bounds.
    * @return true on success, false if out of bounds.
    */
   bool getTileLength( ossim_int32 index,
                       ossim_uint32& length ) const;

   /**
    * @brief Sets the tile length for tile at x,y in ptlm array.
    * @param index
    * @param length of tile
    * @return true on success, false if out of bounds.
    */
   bool setTileLength( ossim_int32 index,
                       ossim_uint32 length );

   /**
    * @brief Adds Ptlm array from first to last.
    *
    * The range used is [first,last), which contains all the elements between
    * first and last, including the element pointed by first but not the
    * element pointed by last.
    * 
    * @param first array index
    * @param last array index
    * @param init Initialized by this. This is added to so whatever it starts
    * with you get that plus the accumlated indexes.
    * @return true on success; false, on errors.
    */
   bool accumulate( ossim_int32 first, ossim_int32 last, std::streampos& init ) const;

   /**
    * @brief Sets SP bit, initializes ptlm array, and ltlm(size).
    *
    * Note: Ptlm array is cleared on this call.
    *
    * @param spBit 0 = 16 bit Ptlm parameter, 1 = 32 bit Ptlm parameter.
    * @param count of ptlm array or tiles.
    */
   bool initPtlmArray( ossim_uint8 spBit, ossim_uint16 count );


   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out String to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string()) const;

   /** operator<< */
   friend OSSIM_DLL std::ostream& operator<<(
      std::ostream& out, const ossimJ2kTlmRecord& obj);

private:
   
   /** @brief Deletes m_Ttlm and m_Ptlm arrays. */
   void clear();

   /** @brief Deletes m_Ttlm array. */
   void clearTtlm();

   /** @brief Deletes m_Ptlm array. */
   void clearPtlm();
   
   /**
    * @brief Computes length of this segment minus marker itself.
    * @return Length of this segment.
    */
   ossim_uint16 computeLength( ossim_uint16 tileCount ) const;
   
   /** NOTE: tml segmet marker 0xff55 not stored. */

   /** Length of segment minus marker. */
   ossim_uint16 m_Ltlm;

   /**
    * Index of marker segment relative to all other TLM marker segments
    * present in the current header. 0 - 255
    */
   ossim_uint8 m_Ztlm;

   /**
    *  Indicator for Ttlm and Ptlm field sizes. Two variable in one byte.
    *
    *  ST Ttlm is lower order bytes.
    *  
    *  0 = Ttlm size = 0  bits, tiles in order. Ttlm size = 0 bits
    *  1 = Ttlm size = 8  bits, range 0 - 254
    *  2 = Ttlm size = 16 bits, range 0 - 65535
    *
    *  SP Ptlm is upper order bytes.
    *  
    *  16 bits if SP = 0, range 14 - 65535
    *  32 bits if SP = 1, range 14 - (2^31-1)
    */
   ossim_uint8 m_Stlm;

   /**
    * Tile index for tile-parts.  Either none or one value for every tile-part.
    * 0  bits if ST = 0
    * 8  bits if ST = 1, range 0 - 254
    * 16 bits if ST = 2, range 0 - 65534
    */
   void* m_Ttlm;

   /**
    * The length, in bytes, from the beginning of the SOT marker of the
    * tile-part to the end of the codestream data for that tile-part.
    * There should be one Ptlm for every tile-part.
    *
    * 16 bits if SP = 0, range 14 - 65535
    * 32 bits if SP = 1, range 14 - (2^32-1)
    *
    */
   void* m_Ptlm;



};

#endif /* End of "#ifndef ossimJ2kTlmRecord_HEADER" */

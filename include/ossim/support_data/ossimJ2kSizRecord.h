//---
// License: MIT
// Description: Container class for J2K Image and tile size (SIZ) record.
// See document BPJ2K01.00 Table 7-6 Image and tile size (15444-1 Annex A5.1)
// $Id$
//---

#ifndef ossimJ2kSizRecord_HEADER
#define ossimJ2kSizRecord_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIosFwd.h>
#include <string>

class OSSIM_DLL ossimJ2kSizRecord
{
public:
   
   /** default constructor */
   ossimJ2kSizRecord();

   /** destructor */
   ~ossimJ2kSizRecord();

   /**
    * Parse method.  Performs byte swapping as needed.
    *
    * @param in Stream to parse.
    *
    * @note SIZ Marker (0xff51) is not read.
    */
   void parseStream(ossim::istream& in);

   /** @return scalar type based on bit depth and signed bit from theSsiz. */
   ossimScalarType getScalarType() const;

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
      std::ostream& out, const ossimJ2kSizRecord& obj);

   /** segmet marker 0xff51 (big endian) */
   ossim_uint16 m_marker;

   /** length of segment minus marker */
   ossim_uint16 m_Lsiz;

   /** profile */
   ossim_uint16 m_Rsiz;

   /** width of reference grid */
   ossim_uint32 m_Xsiz;

   /** height of reference grid */
   ossim_uint32 m_Ysiz;

   /**
    * Horizontal offset from the orgin of reference grid to the left side
    * of image.
    */
   ossim_uint32 m_XOsiz;

   /**
    * Vertical offset from the orgin of reference grid to the top
    * of image.
    */
   ossim_uint32 m_YOsiz;

   /** width of one reference tile */
   ossim_uint32 m_XTsiz;
   
   /** height of one reference tile */   
   ossim_uint32 m_YTsiz;

   /**
    * Horizontal offset from the orgin of reference grid to the left edge
    * of first tile.
    */
   ossim_uint32 m_XTOsiz;

   /**
    * Vertical offset from the orgin of reference grid to the top
    * edge of first tile.
    */
   ossim_uint32 m_YTOsiz;

   /** number of component in the image */
   ossim_uint16 m_Csiz;

   /**
    * sign bit and bit depth of data
    * unsigned = 0xxx xxxx (msb == 0)
    * signed   = 1xxx xxxx (msb == 1)
    * bit depth = x000 0000 + 1 (first seven bits plus one)
    */
   ossim_uint8  m_Ssiz;

   /**
    * Horizontal separation of a sample of the component with respect to the
    * reference grid.
    */
   ossim_uint8  m_XRsiz;

   /**
    * Vertical separation of a sample of the component with respect to the
    * reference grid.
    */
   ossim_uint8  m_YRsiz;
};

#endif /* End of "#ifndef ossimJ2kSizRecord_HEADER" */

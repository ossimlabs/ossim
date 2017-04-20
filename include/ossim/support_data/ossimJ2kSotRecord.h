//---
// License: MIT
// Description: Container class for J2K "Start Of Tile" (SOT) record.
// marker: FF90
// See document BPJ2K01.00 Table 7-3 Image and tile size (15444-1 Annex A.4.2)
// $Id$
//---

#ifndef ossimJ2kSotRecord_HEADER
#define ossimJ2kSotRecord_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIosFwd.h>
#include <string>

class OSSIM_DLL ossimJ2kSotRecord
{
public:
   
   /** default constructor */
   ossimJ2kSotRecord();

   /** destructor */
   ~ossimJ2kSotRecord();

   /**
    * Parse method.  Performs byte swapping as needed.
    *
    * @param in Stream to parse.
    *
    * @note Marker is not read.
    */
   void parseStream(ossim::istream& in);

   /**
    * Write method.
    *
    * Note: Write includes two marker bytes.
    *
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);

   /**
    * @brief Sets the tile index.
    * @param isot
    */
   void setIsot( ossim_uint16 isot );
   
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
   
   /**
    * operator<<.
    */
   friend OSSIM_DLL std::ostream& operator<<(
      std::ostream& out, const ossimJ2kSotRecord& obj);

   /** Length in bytes of the marker segment. */
   ossim_uint16 theLsot;

   /** Tile index. Tiles are in raster order starting at 0. */
   ossim_uint16 theIsot;

   /** The length in bytes of this record including the SOT marker. */
   ossim_uint32 thePsot;

   /** Tile-Part index. */
   ossim_uint8  theTpsot;

   /**
    * 0 = Number of tile-parts of this tile in the codestream is not defined
    * in this header.
    *
    * 1-255 number of tile-parts of this tile in the codestream.
    */
   ossim_uint8  theTnsot;
};

#endif /* End of "#ifndef ossimJ2kSotRecord_HEADER" */

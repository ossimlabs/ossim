//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Container class for J2K Coding style default (COD) record.
//
// See document BPJ2K01.00 Table 7-7 Image and tile size (15444-1 Annex A5.1)
// 
//----------------------------------------------------------------------------
// $Id: ossimJ2kCodRecord.h,v 1.5 2005/10/13 21:24:47 dburken Exp $
#ifndef ossimJ2kCodRecord_HEADER
#define ossimJ2kCodRecord_HEADER 1

#include <ossim/base/ossimConstants.h>

#include <iosfwd>
#include <string>

class OSSIM_DLL ossimJ2kCodRecord
{
public:
   
   /** default constructor */
   ossimJ2kCodRecord();

   /** destructor */
   ~ossimJ2kCodRecord();

   /**
    * Parse method.  Performs byte swapping as needed.
    *
    * @param in Stream to parse.
    *
    * @note COD Marker (0xff52) is not read.
    */
   void parseStream(std::istream& in);

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out String to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "j2k." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string()) const;

   /** operator<< */
   friend OSSIM_DLL std::ostream& operator<<(
      std::ostream& out, const ossimJ2kCodRecord& obj);

   /** segmet marker 0xff52 (big endian) */
   ossim_uint16 m_marker;

   /** length of segment minus marker */
   ossim_uint16 m_lcod;

   /** Coding style */
   ossim_uint8 m_scod;

   /** SGcod - Progression order */
   ossim_uint8 m_progressionOrder;

   /** SGcod - Number of layers */
   ossim_uint16 m_numberOfLayers;

   /** SGcod - Multiple component transform */
   ossim_uint8 m_multipleComponentTransform;

   /** SPcod - Number of decomposition levels */
   ossim_uint8 m_numberOfDecompositionLevels;

   /** SPcod - Code-block width */
   ossim_uint8 m_codeBlockWidth;
   
   /** SPcod - Code-block height */
   ossim_uint8 m_codeBlockHeight;

   /** SPcod - Code-block height */
   ossim_uint8 m_codeBlockStyle;

   /** SPcod - transformation (wavelet filter) */
   ossim_uint8 m_transformation;

    /** SPcod - precinct size (only is defined, Scod = xxxx xxx1) */
   ossim_uint8 m_precinctSize;
};

#endif /* End of "#ifndef ossimJ2kCodRecord_HEADER" */

//----------------------------------------------------------------------------
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: JP2 Info object.
// 
//----------------------------------------------------------------------------
// $Id: ossimJp2Info.h 23220 2015-04-04 15:39:49Z dburken $
#ifndef ossimJp2Info_HEADER
#define ossimJp2Info_HEADER 1

#include <ossim/support_data/ossimJ2kInfo.h>

/**
 * @brief JP2 info class.
 */
class OSSIM_DLL ossimJp2Info : public ossimJ2kInfo
{
public:

   /** Anonymous enumerations: */
   enum
   {
      SIGNATURE_BOX_SIZE = 12,
      GEOTIFF_UUID_SIZE  = 16
   };
   
   /** default constructor */
   ossimJp2Info();

   /** virtual destructor */
   virtual ~ossimJp2Info();

   /**
    * @brief open method.
    *
    * @param file File name to open.
    *
    * @return true on success false on error.
    */
   virtual bool open(const ossimFilename& file);

   /**
    * Print method.
    *
    * @param out Stream to print to.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;

   /**
    * @brief This gets a box of type and stuffs in box.
    *
    * Code for copying a box to a buffer. Currently used in ossimOpjWriter
    * to overcome short falls of trying to write in between the uuid box in
    * between the jp2h and start of jp2c written by opj_start_compress.
    *
    * @param type Defines box type. E.g. 0x6A703268 (jp2h)
    * @param includeAll if true the length fields are copied. If false
    * just the DBox is copied.
    * @param box Initialized by this. LBox and XLBox fields
    * are NOT copied to the box.
    * @return is the start position of the box. At the LBox.
    */
   std::streamoff getBox( const ossim_uint32 type,
                          bool includeAll,
                          std::vector<ossim_uint8>& box ) const;

   /**
    * @brief This gets a box of type and stuffs in box.
    *
    * Code for copying a box to a buffer. Note that stream is not rewound or
    * cleared if it hits EOF.
    *
    * @param type Defines box type. E.g. 0x6A703268 (jp2h)
    * @param includeAll if true the length fields are copied. If false
    * just the DBox is copied.
    * @param str Stream to read from.
    * @param box Initialized by this. LBox and XLBox fields
    * are NOT copied to the box.
    * @return The start position of the box (LBox) or zero if not found.
    */
   std::streamoff getBox( const ossim_uint32& type,
                          bool includeAll,
                          std::ifstream& str,
                          std::vector<ossim_uint8>& box ) const;
   /**
    * @brief Method to get the embedded JP2 GeoTIFF box. This includes the 16
    * GEOTIFF_UUID bytes.
    * @param str Stream to read from.
    * @return  The start position of the box (LBox) or zero if not found.
    */
   std::streamoff getGeotiffBox( std::ifstream& str,
                                 std::vector<ossim_uint8>& box ) const;

   /**
    * @brief Method to get the embedded JP2 GML Box.
    * @param str Stream to read from.
    * @return  The start position of the box (LBox) or zero if not found.
    */
   std::streamoff getGmlBox( std::ifstream& str,
                             std::vector<ossim_uint8>& box ) const;
   
   /**
    * @brief This finds a box of type and returns the location of its data.
    *
    * @param type Defines box type. E.g. 0x6A703268 (jp2h)
    * @param str Stream to read from.
    * @param length Number of bytes in the data.
    * @return The start position of the box (LBox) or zero if not found.
    */
   std::streamoff findBoxData( const ossim_uint32& type,
                               std::ifstream& str,
                               ossim_uint32& length ) const;

protected:

   /** @brief Print tbox type as string if known. */
   void printTboxType( ossim_uint32 tbox, std::ostream& out) const;

   /** Initializes i reference.  Does byte swapping as needed. */
   void readUInt32(ossim_uint32& i, std::ifstream& str) const;

   /** Initializes i reference.  Does byte swapping as needed. */
   void readUInt64(ossim_uint64& i, std::ifstream& str) const;

   std::ostream &printPleiades(std::ostream &out,
                               const ossimString &prefix = ossimString()) const;
};

#endif /* End of "#ifndef ossimJp2Info_HEADER" */

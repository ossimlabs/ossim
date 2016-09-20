//---
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Generic image writer class.
// 
//---
// $Id$
#ifndef ossimWriter_HEADER
#define ossimWriter_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <iosfwd>
#include <vector>

/**
 * @brief ossimWriter - Generic image writer.
 *
 * Please do NOT add any external library dependencies. drb - 26 May 2016
 */
class OSSIM_DLL ossimWriter : public ossimImageFileWriter
{
public:
   
   /** default constructor */
   ossimWriter();

   /** virtual destructor */
   virtual ~ossimWriter();

   /** @return "ossim_writer" */
   virtual ossimString getShortName() const;

   /** @return "ossim writer" */
   virtual ossimString getLongName() const;

   /** @return "ossimWriter" */
   virtual ossimString getClassName() const;

   /**
    * Returns a 3-letter extension from the image type descriptor 
    * (theOutputImageType) that can be used for image file extensions.
    *
    * @param imageType string representing image type.
    *
    * @return the 3-letter string extension.
    */
   virtual ossimString getExtension() const;
   
   /**
    * @brief getImageTypeList method.
    *
    * Satisfies ossimImageFileWriter::getImageTypeList pure virtual.
    * 
    * Appends the writers image types to the "imageTypeList".
    * 
    * This is the actual image type name.  So for
    * example, ossimTiffWriter has several image types.  Some of these
    * include TIFF_TILED, TIFF_TILED_BAND_SEPARATE ... etc.
    * The ossimGdalWriter
    * may include GDAL_IMAGINE_HFA, GDAL_RGB_NITF, GDAL_JPEG20000, ... etc.
    *
    * @param imageTypeList list to append to.
    */
   virtual void getImageTypeList(std::vector<ossimString>& imageTypeList) const;

   /**
    * @brief isOpen
    * @return true if open; else, false.
    */
   virtual bool isOpen()const; 

   /**
    * @brief open
    * @return true on success, false on error.
    */
   virtual bool open();

   /** @brief close Flushes and deletes stream if we own it. */
   virtual void close();
   
   bool hasImageType(const ossimString& imageType) const;

   /**
    * @brief Method to write the image to a stream.
    *
    * @return true on success, false on error.
    */
   virtual bool writeStream();

   /**
    * @brief Sets the output stream.
    * @param stream The stream to write to.
    * @return true if object can write to stream, false if not.
    */
   virtual bool setOutputStream(std::ostream& str);

protected:
   
   /**
    * @brief Write out the file.
    * @return true on success, false on error.
    */
   virtual bool writeFile();

private:

   bool writeStreamTtbs();

   bool writeTiffHdr();
   bool writeTiffTags( const std::vector<ossim_uint64>& tile_offsets,
                       const std::vector<ossim_uint64>& tile_byte_counts,
                       const std::vector<ossim_float64>& minBands,
                       const std::vector<ossim_float64>& maxBands );

   bool writeMinMaxTiffTags( const std::vector<ossim_float64>& minBands,
                             const std::vector<ossim_float64>& maxBands,
                             std::streamoff& arrayWritePos  );
   
   bool writeSMinSMaxTiffTags( const std::vector<ossim_float64>& minBands,
                             const std::vector<ossim_float64>& maxBands,
                             std::streamoff& arrayWritePos  );

   // 
   template <class T> void writeTiffTag(ossim_uint16 tag, ossim_uint16 type,
                                        ossim_uint64 count,
                                        const T* value,
                                        std::streamoff& arrayWritePos );
   
   bool writeTiffTilesBandSeparate( std::vector<ossim_uint64>& tile_offsets,
                                    std::vector<ossim_uint64>& tile_byte_counts,
                                    std::vector<ossim_float64>& minBands,
                                    std::vector<ossim_float64>& maxBands );

   ossim_uint16 getTiffSampleFormat() const;
  

   std::ostream* m_str;
   bool          m_ownsStreamFlag; 
   
}; // End: class ossimWriter

#endif /* End of "#ifndef ossimWriter_HEADER" */

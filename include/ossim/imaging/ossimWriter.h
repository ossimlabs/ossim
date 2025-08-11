//---
//
// License: MIT
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
#include <ossim/base/ossimIpt.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <iosfwd>
#include <vector>

class ossimKeywordlist;
class ossimProperty;

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
    * Current write type:
    * ossim_ttbs - tiled tiff band separate, big tiff format.
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
    * @brief Sets the output stream to write to.
    *
    * This sets the stream and sets m_ownsStreamFlag to false. So destructor
    * will not delete the stream if this method is used.
    * 
    * @param stream The stream to write to.
    * @return true if object can write to stream, false if not.
    */
   virtual bool setOutputStream(std::ostream& str);

   /**
    * @brief Sets the output tile size for tiled formats.
    * @param tileSize Must be a multiple of 16.
    */
   virtual void setTileSize(const ossimIpt& tileSize);

   /**
    * @brief Gets the tile size.
    * @return Reference to tile size.
    */
   virtual const ossimIpt& getOutputTileSize() const;

   /**
    * @brief Saves the state of the object.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Will set the property whose name matches the argument
    * "property->getName()".
    *
    * @param property Object containing property to set.
    */
   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   
   /**
    * @param name Name of property to return.
    * 
    * @returns A pointer to a property object which matches "name".  Returns
    * NULL if no match is found.
    */
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name) const;

   /**
    * Pushes this's names onto the list of property names.
    *
    * @param propertyNames array to add this's property names to.
    */
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames) const;

 
   
protected:
   
   /**
    * @brief Write out the file.
    * @return true on success, false on error.
    */
   virtual bool writeFile();

private:
   
   /**
    * @brief Writes a tiled tiff band separate to stream.
    * @return true on success, false on error.
    */
   bool writeStreamTtbs();

   /**
    * @brief Writes tiff header to stream.
    * @param str Stream to write to.
    * @return true on success, false on error.
    */
   bool writeTiffHdr( std::ostream* str );

   /**
    * @brief Writes tags to image file directory(IFD).
    * @param tile_offsets
    * @param tile_byte_counts
    * @param minBands
    * @param maxBands
    * @return true on success, false on error.
    */
   bool writeTiffTags( const std::vector<ossim_uint64>& tile_offsets,
                       const std::vector<ossim_uint64>& tile_byte_counts,
                       const std::vector<ossim_float64>& minBands,
                       const std::vector<ossim_float64>& maxBands );

   /**
    * @brief Writes tags to image file directory(IFD).
    * @param str Stream to write to.
    * @return true on success, false on error.
    */
   bool writeTiffTags( std::ostream* str );

   /**
    * @brief Writes tags TIFFTAG_MINSAMPLEVALUE(280) and
    * TIFFTAG_MAXSAMPLEVALUE(281).  Only written if scalar type is an unsigned
    * byte or short.
    * @param arrayWritePos Position to write array to.  This will be updated
    * if array is written with new offset.
    * @return true on success, false on error.
    */
   bool writeMinMaxTiffTags( std::streamoff& arrayWritePos  );
   
   /**
    * @brief Writes tags TIFFTAG_MINSAMPLEVALUE(280) and
    * TIFFTAG_MAXSAMPLEVALUE(281).  Only written if scalar type is an unsigned
    * byte or short.
    * @param str Stream to write to.
    * @param arrayWritePos Position to write array to.  This will be updated
    * if array is written with new offset.
    * @return true on success, false on error.
    */
   bool writeMinMaxTiffTags( std::ostream* str,
                             std::streamoff& arrayWritePos );
   

   /**
    * @brief Writes tags TIFFTAG_SMINSAMPLEVALUE(340) and
    * TIFFTAG_SMAXSAMPLEVALUE(341).  Only written if scalar type is not an
    * unsigned byte or short.
    * @param minBands Array of min values from image write.
    * @param maxBands Array of max values from image write.
    * @return true if tags are written, false if not.
    * A false return is not necessarily an error, just means the
    * tags were not written due to the scalar type.
    */
   bool writeSMinSMaxTiffTags( const std::vector<ossim_float64>& minBands,
                               const std::vector<ossim_float64>& maxBands,
                               std::streamoff& arrayWritePos  );

   /**
    * @brief Writes tags TIFFTAG_SMINSAMPLEVALUE(340) and
    * TIFFTAG_SMAXSAMPLEVALUE(341).  Only written if scalar type is not an
    * unsigned byte or short.
    * @param str Stream to write to.
    * @param minBands Array of min values from image write.
    * @param maxBands Array of max values from image write.
    * @return true if tags are written, false if not.
    * A false return is not necessarily an error, just means the
    * tags were not written due to the scalar type.
    */
   bool writeSMinSMaxTiffTags( std::ostream* str,
                               const std::vector<ossim_float64>& minBands,
                               const std::vector<ossim_float64>& maxBands,
                               std::streamoff& arrayWritePos  );

   /**
    * @brief Writes tags TIFFTAG_SMINSAMPLEVALUE(340) and
    * TIFFTAG_SMAXSAMPLEVALUE(341).  Only written if scalar type is not an
    * unsigned byte or short.
    *
    * This method pulls min and max values from the input connection as
    * oposed to scanned from tile values.
    * 
    * @param str Stream to write to.
    * @return true if tags are written, false if not.
    * A false return is not necessarily an error, just means the
    * tags were not written due to the scalar type.
    */ 
   bool writeSMinSMaxTiffTags( std::ostream* str,
                               std::streamoff& arrayWritePos );

   /**
    * @brief Writes tiff tag to image file directory(IFD).
    * @param tag
    * @param type
    * @param count
    * @param value(s) or offset to array.
    * @param arrayWritePos Position to write array to.  This will be updated
    * if array is written with new offset.
    */   
   template <class T> void writeTiffTag(ossim_uint16 tag,
                                        ossim_uint16 type,
                                        ossim_uint64 count,
                                        const T* value,
                                        std::streamoff& arrayWritePos );

   /**
    * @brief Writes tiff tag to image file directory(IFD).
    * @param str Stream to write to.
    * @param tag
    * @param type
    * @param count
    * @param value(s) or offset to array.
    * @param arrayWritePos Position to write array to.  This will be updated
    * if array is written with new offset.
    */   
   template <class T> void writeTiffTag( std::ostream* str,
                                         ossim_uint16 tag,
                                         ossim_uint16 type,
                                         ossim_uint64 count,
                                         const T* value,
                                         std::streamoff& arrayWritePos );

   
   /**
    * @brief Writes image data to stream. (TTBS = Tiled Tiff Band Separate)
    *
    * Data is in a band separate tile layout(PLANARCONFIG_SEPARATE), i.e. red
    * tile, green tile, blue tile, ..., red tile, green tile, blue tile.
    * 
    * @param tile_offsets Initialized by this with offset for each tile.
    * @param tile_byte_counts Initialized by this with the byte count of each
    * tile.
    * @param minBands Initialized by this with the min values for each band.
    * @param maxBands Initialized by this with the max values for each band.
    * @return true on success, false on error.
    */
   bool writeTtbs( std::vector<ossim_uint64>& tile_offsets,
                   std::vector<ossim_uint64>& tile_byte_counts,
                   std::vector<ossim_float64>& minBands,
                   std::vector<ossim_float64>& maxBands );

   /**
    * @brief Writes image data to stream. (TTBS = Tiled Tiff Band Separate)
    * Data is in a band separate tile layout(PLANARCONFIG_SEPARATE), i.e. red
    * tile, green tile, blue tile, ..., red tile, green tile, blue tile.
    *
    * @note This method does not capture tile offsets and byte sizes or
    * scan for min/max.
    *
    * @return true on success, false on error.
    */
   bool writeTtbs();
   
   /**
    * @brief Gets the tiff sample format based on scalar type.
    * E.g SAMPLEFORMAT_UINT, SAMPLEFORMAT_INT or SAMPLEFORMAT_IEEEFP.
    * @return TIFF sample format or 0 if not mapped to a scalar type.
    */
   ossim_uint16 getTiffSampleFormat() const;

   /**
    *  @return true if the output type is tiled, false if not.
    */
   bool isTiled() const;

   /**
    * @return Value of options key: "align_tiles".
    * If true, aligns tile addresses to 4096 boundary.
    * default=true
    */
   bool getAlignTilesFlag() const;

   /**
    * @return Value of options key: "block_size".
    * If true, aligns tile addresses to block boundary.
    * default=true
    */
   ossim_int64 getBlockSize() const;

   /**
    * @brief Value of options key: "sequencer_box_size".
    * @param size Initialized by this. No range check. The sequencer range
    * checks.
    * @return true on success, false on error.
    */
   bool getSequencerBoxSize( ossimIpt& size ) const;

   /**
    * @brief Value of options key: "sequencer_mode".
    * @param mode Initialized by this. No range check. The sequencer range
    * checks.
    * @return true on success, false on error.
    */
   bool getSequencerMode( std::string& mode ) const;

   /**
    * @return Value of options key: "flush_tiles".
    * If true, ostream::flush() is called after each tile write.
    * default=true
    */
   bool getFlushTilesFlag() const;
  
   /**
    * @return Value of options key: "include_blank_tiles".
    * If true, empty/blank tiles will be written; if false, the tile will not
    * be written, the tile offset and byte count will be set to zero.
    * default=true (write blanks).
    */
   bool getWriteBlanksFlag() const;

   /**
    * @return true if min / max should be scanned for when writing tiles;
    * false, if not.
    */
   bool needsMinMax() const;
   
   /**
    * @return true if format is such that it can be written contiguously with
    * no seeking. Pertinent for streaming code.
    */
   bool canContiguousWrite() const;

   /**
    * @brief Initializes tile offsets and byte counts assuming sequential
    * write from a fixed position.
    * @param tile_offsets Initialized by this with offset for each tile.
    * @param tile_byte_counts Initialized by this with the byte count of each
    * tile.
    * @return true on success, false on error.
    */
   bool getTileInfo( std::vector<ossim_uint64>& tile_offsets,
                     std::vector<ossim_uint64>& tile_byte_counts ) const;

   void getTtbsTileStartPos( std::streampos& pos ) const;

   /**
    * @return true if "add_alpha_channel" key is present and set to true; else,
    * false.
    */
   bool addAlpha() const;
  
   std::ostream* m_str;
   bool          m_ownsStreamFlag;

   /** Hold all options. */
   ossimRefPtr<ossimKeywordlist> m_kwl;

   ossimIpt      m_outputTileSize;
   
}; // End: class ossimWriter

#endif /* End of "#ifndef ossimWriter_HEADER" */

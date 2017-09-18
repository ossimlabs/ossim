//----------------------------------------------------------------------------
//
// File: ossimLasReader.h
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimLasReader_HEADER
#define ossimLasReader_HEADER 1

#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimUnitConversionTool.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/projection/ossimProjection.h>

class ossimLasHdr;
class ossimLasPointRecordInterface;

/**
 * @class ossimLasReader
 *
 * OSSIM LAS LIDAR reader.
 */
class ossimLasReader : public ossimImageHandler
{
public:

   /** default constructor */
   ossimLasReader();

   /** virtual destructor */
   virtual ~ossimLasReader();

   /**
    *  @brief open method.
    *
    *  Satisfies ossimImageHandler::open pure virtual.
    *  
    *  @return Returns true on success, false on error.
    *
    *  @note This method relies on the data member ossimImageData::theImageFile
    *  being set.  Callers should do a "setFilename" prior to calling this
    *  method or use the ossimImageHandler::open that takes a file name and an
    *  entry index.
    */
   virtual bool open();

   /**
    *  @brief is open method.
    *
    *  Satisfies ossimImageHandler::isOpen pure virtual.
    *
    *  @return true if open, false if not.
    */
   virtual bool isOpen()const;

   /**
    * @brief Close method.
    */
   virtual void close();

   /**
    *  Returns a pointer to a tile given an origin representing the upper left
    *  corner of the tile to grab from the image.
    *  Satisfies pure virtual from TileSource class.
    */
   virtual ossimRefPtr<ossimImageData> getTile(const  ossimIrect& rect,
                                               ossim_uint32 resLevel=0);
   
   /**
    * Method to get a tile.   
    *
    * @param result The tile to stuff.  Note The requested rectangle in full
    * image space and bands should be set in the result tile prior to
    * passing.  It will be an error if:
    * result.getNumberOfBands() != this->getNumberOfOutputBands()
    *
    * @return true on success false on error.  If return is false, result
    *  is undefined so caller should handle appropriately with makeBlank or
    * whatever.
    */
   virtual bool getTile(ossimImageData* result, ossim_uint32 resLevel=0);
   
   /**
    * @brief Gets bands.
    *
    * Satisfies ossimImageSource::getNumberOfInputBands pure virtual.
    * 
    * @retrun Number of bands.
    */
   virtual ossim_uint32 getNumberOfInputBands() const;

   /**
    * @brief Gets lines.
    *
    * Satisfies ossimImageHandler::getNumberOfLines pure virtual.
    *
    * @param resLevel Reduced resolution level to return lines of.
    * Default = 0
    *
    * @return The number of lines for specified reduced resolution level.
    */
   virtual ossim_uint32 getNumberOfLines(ossim_uint32 resLevel = 0) const;

   /**
    * @brief Gets samples.
    *
    * Satisfies ossimImageHandler::getNumberOfSamples
    *
    * @param resLevel Reduced resolution level to return samples of.
    * Default = 0
    *
    * @return The number of samples for specified reduced resolution level.
    */
   virtual ossim_uint32 getNumberOfSamples(ossim_uint32 resLevel = 0) const;
    
   /**
    * @brief Gets tile width.
    *
    * Satisfies ossimImageHandler::getImageTileWidth pure virtual.
    * 
    * @return The tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the ossimImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual ossim_uint32 getImageTileWidth() const;

   /**
    * @brief Gets tile height.
    *
    * Satisfies ossimImageHandler::getImageTileHeight pure virtual.
    * 
    * @return The tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the ossimImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual ossim_uint32 getImageTileHeight() const;

   /** @return The width of the output tile. */
   virtual ossim_uint32 getTileWidth() const;
   
   /** @returns The height of the output tile. */
   virtual ossim_uint32 getTileHeight() const;

   /** @return The output pixel type of the tile source. */
   ossimScalarType getOutputScalarType() const;

   /**
    * @brief Gets entry list.
    * @param entryList This is the list to initialize with entry indexes.
    */
   virtual void getEntryList(std::vector<ossim_uint32>& entryList) const;

   /** @return The current entry number. */
   virtual ossim_uint32 getCurrentEntry() const;

   /**
    * @param entryIdx Entry number to select.
    * @return true if it was able to set the current entry and false otherwise.
    */
   virtual bool setCurrentEntry(ossim_uint32 entryIdx);

   /** @return "las" */
   virtual ossimString getShortName() const;
   
   /** @return "ossim las (liblas) reader" */
   virtual ossimString getLongName()  const;

   /**
    * Returns the image geometry object associated with this tile source or
    * NULL if non defined.  The geometry contains full-to-local image
    * transform as well as projection (image-to-world).
    */
   virtual ossimRefPtr<ossimImageGeometry> getImageGeometry();

   /** @return Min pixel value. */
   virtual double getMinPixelValue(ossim_uint32 band=0) const;

   /** @return Min pixel value. */
   virtual double getMaxPixelValue(ossim_uint32 band=0) const;

   /** @return Min pixel value. */
   virtual double getNullPixelValue(ossim_uint32 band=0) const;

   /**
    * @brief Method to save the state of an object to a keyword list.
    * @return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;
  
   /**
    * @brief Method to the load (recreate) the state of an object from a keyword list.
    * @return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

   /**
    * @brief Set propterty method. Overrides ossimImageHandler::setProperty.
    *
    * Current property name handled:
    * "scale" One double value representing the scale in meters per pixel. It is
    * assumed the scale is same for x and y direction.
    * 
    * @param property to set.
    */
   virtual void setProperty(ossimRefPtr<ossimProperty> property);

   /**
     * @brief Get propterty method. Overrides ossimImageHandler::getProperty.
    * @param name Property name to get.
    */
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name) const;

   /**
    * @brief Get propterty names. Overrides ossimImageHandler::getPropertyNames.
    * @param propertyNames Array to initialize.
    */
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames) const;

   /** @return The total number of decimation levels. */
   virtual ossim_uint32 getNumberOfDecimationLevels() const;

   virtual ossim_uint32 getNumberOfOutputBands() const;

protected:
   /**
    * @brief Will complete the opening process.
    *
    * Overrides ossimImageHandler::completeOpen() as we do not ever have overviews.
    */
   virtual void completeOpen();
   
private:

   /**
    * @brief Container class to hold accumulated point data.  Currently single
    * band.
    */
   class Bucket
   {
   public:
      Bucket(): a(0.0), c(0), red(0), green(0), blue(0) {}
         
      void add(const ossim_float64& point) { a += point; ++c; }
      ossim_float64 getValue() const { return ( c ? a/c : -99999.0 ); }
      void setRed(const ossim_uint16& value) { red = value; }
      void setGreen(const ossim_uint16& value) { green = value; }
      void setBlue(const ossim_uint16& value) { blue = value; }
      void setIntensity(const ossim_uint16& value) { intensity = value; }
      ossim_uint16 getRed() const { return red; }
      ossim_uint16 getGreen() const { return green; }
      ossim_uint16 getBlue() const { return blue; }
      ossim_uint16 getIntensity() const { return intensity; }

      ossim_float64 a; // accumulation
      ossim_uint32  c; // count
      ossim_uint16 red;
      ossim_uint16 green;
      ossim_uint16 blue;
      ossim_uint16 intensity;
   };

   bool init();
   
   void initValues(); // m_ul, m_lr, m_minZ
   bool initProjection();
   void initTile();
   void initUnits(const ossimKeywordlist& geomKwl);
   bool parseVarRecords();

   /**
    * @brief Looks for external FGDC text file to initialize projection from.
    */
   bool initFromExternalMetadata();
   
   /** @brief Get the scale for resLevel. */
   void getScale(ossimDpt& scale, ossim_uint32 resLevel) const;

   /** @brief Sets m_gsd data member and projection if projection is set. */
   void setGsd( const ossim_float64& gsd );

   void convertToMeters(ossim_float64& value) const;

   /**
    * Returns a point of type.
    */
   ossimLasPointRecordInterface* getNewPointRecord() const;

   std::ifstream                m_str;
   ossimLasHdr*                 m_hdr;
   ossimRefPtr<ossimProjection> m_proj;
   ossimDpt                     m_ul;
   ossimDpt                     m_lr;
   ossim_float64                m_maxZ;
   ossim_float64                m_minZ;
   ossimDpt                     m_gsd;
   ossimRefPtr<ossimImageData>  m_tile;
   ossim_uint8                  m_entry;
   std::mutex                   m_mutex;
   bool                         m_scan;  // Scan for bounds at open.
   ossimUnitType                m_units;
   ossimUnitConversionTool*     m_unitConverter;
TYPE_DATA
};

inline bool ossimLasReader::isOpen()const
{
   return ( m_str.is_open() && m_hdr );
}

inline void ossimLasReader::convertToMeters(ossim_float64& value) const
{
   if ( value )
   {
      m_unitConverter->setValue(value, m_units);
      value = m_unitConverter->getMeters();
   }
}

#endif /* #ifndef ossimLasReader_HEADER */

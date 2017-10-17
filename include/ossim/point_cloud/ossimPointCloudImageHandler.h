//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  MIT -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id$

#ifndef ossimPointCloudImageSource_HEADER
#define ossimPointCloudImageSource_HEADER

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/point_cloud/ossimPointCloudHandler.h>
#include <vector>
#include <mutex>

class ossimImageData;
class ossimTiffOverviewTileSource;
class ossimPoinCloudHandler;

/**
 * Class used for rendering point cloud data into a raster tile. It is derived from
 * ossimImageHandler so that it can be included in the available file formats for reading as
 * maintained by the ossimImageHandlerRegistry. This base class implementation exposes the data
 * elements represented by the base-class ossimPointCloudSource and the individual point base-class
 * ossimPointCloudRecord, namely, intensity, RGB color, elevation, and number of returns.
 *
 * For sensor systems providing additional renderable data items, a derived class will be required
 * to expose those components.
 */
class OSSIMDLLEXPORT ossimPointCloudImageHandler : public ossimImageHandler
{
public:
   enum Components { INTENSITY=0, HIGHEST, LOWEST, RETURNS, RGB, NUM_COMPONENTS /*not a component*/ };

   ossimPointCloudImageHandler();
   virtual ~ossimPointCloudImageHandler();

   /**
    *  @brief open method.
    *  Satisfies ossimImageHandler::open pure virtual.
    *  @return Returns true on success, false on error.
    *  @note This method relies on the data member ossimImageData::theImageFile
    *  being set.  Callers should do a "setFilename" prior to calling this
    *  method or use the ossimImageHandler::open that takes a file name and an
    *  entry index.
    */
   virtual bool open();

   /**
    *  @brief is open method.
    *  Satisfies ossimImageHandler::isOpen pure virtual.
    *  @return true if open, false if not.
    */
   virtual bool isOpen() const { return m_pch.valid(); }


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
    * @param result The tile to stuff.  Note The requested rectangle in full
    * image space and bands should be set in the result tile prior to
    * passing.  It will be an error if:
    * result.getNumberOfBands() != this->getNumberOfOutputBands()
    * @return true on success false on error.  If return is false, result
    *  is undefined so caller should handle appropriately with makeBlank or
    * whatever.
    */
   virtual bool getTile(ossimImageData* result, ossim_uint32 resLevel=0);
   
   /**
    * @brief Gets bands.
    * Satisfies ossimImageSource::getNumberOfInputBands pure virtual.
    * @retrun Number of bands.
    */
   virtual ossim_uint32 getNumberOfInputBands() const;

   /**
    * @brief Gets lines.
    * Satisfies ossimImageHandler::getNumberOfLines pure virtual.
    * @param resLevel Reduced resolution level to return lines of.
    * Default = 0
    * @return The number of lines for specified reduced resolution level.
    */
   virtual ossim_uint32 getNumberOfLines(ossim_uint32 resLevel = 0) const;

   /**
    * @brief Gets samples.
    * Satisfies ossimImageHandler::getNumberOfSamples
    * @param resLevel Reduced resolution level to return samples of.
    * Default = 0
    * @return The number of samples for specified reduced resolution level.
    */
   virtual ossim_uint32 getNumberOfSamples(ossim_uint32 resLevel = 0) const;

   /**
    * @brief Gets tile width.
    * Satisfies ossimImageHandler::getImageTileWidth pure virtual.
    * @return The tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the ossimImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual ossim_uint32 getImageTileWidth() const;

   /**
    * @brief Gets tile height.
    * Satisfies ossimImageHandler::getImageTileHeight pure virtual.
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
    * This list reflects the data component entry list provided by the derived point-cloud handler.
    * This base class understands the basic (LIDAR-biased) entries: "Intensity", "RGB",
    * "Elevation", and "Return". If a sensor provides other components, then a derived image handler
    * will be needed to rasterize that data channel as well.
    * @param entryList This is the list to initialize with entry indexes.
    */
   virtual void getEntryList(std::vector<ossim_uint32>& entryList) const;

   virtual void getEntryNames(std::vector<ossimString>& entryNames) const;

   /** @return The current entry number. */
   virtual ossim_uint32 getCurrentEntry() const;

   /**
    * @param entryIdx Entry number to select.
    * @return true if it was able to set the current entry and false otherwise.
    */
   virtual bool setCurrentEntry(ossim_uint32 entryIdx);

   /** @return "point-cloud" */
   virtual ossimString getShortName() const;
   
   /** @return "ossim point cloud to image renderer" */
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

   /** @return The total number of decimation levels. */
   virtual ossim_uint32 getNumberOfDecimationLevels() const;

   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0) const;

   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

   virtual void getValidImageVertices(std::vector<ossimIpt>& validVertices,
                                      ossimVertexOrdering ordering=OSSIM_CLOCKWISE_ORDER,
                                      ossim_uint32 resLevel=0) const;

   /**
    * The reader properties are:
    * -- the GSD ("meters_per_pixel") which overrides computed nominal GSD
    * -- the GSD factor ("gsd_factor") scales the computed nominal GSD, defaults to 1.0.
    * -- the active component as entry number ("entry") with possible values [0 - 4] corresponding
    *    to "component" property (listed below)
    * -- the active component ("component") as string with possible values
    *    "intensity", "highest", "lowest", "returns", or "rgb", respectively (case insensitive)
    */
   void setProperty(ossimRefPtr<ossimProperty> property);
   ossimRefPtr<ossimProperty> getProperty(const ossimString& name) const;

   /**
    * Permits backdoor for setting the input point cloud handler object. Useful for debug
    */
   bool setPointCloudHandler(ossimPointCloudHandler* pch);

   /** @brief Get the GSD for resLevel. */
   void getGSD(ossimDpt& gsd, ossim_uint32 resLevel) const;

   /** @brief Sets m_gsd data member and projection if projection is set. */
   void setGSD( const ossim_float64& gsd );


protected:
   class PcrBucket
   {
   public:
      PcrBucket() : m_bucket(0), m_numSamples(0) {}
      PcrBucket(const ossim_float32* init_value, ossim_uint32 numBands);
      PcrBucket(const ossim_float32& R, const ossim_float32& G, const ossim_float32& B);
      PcrBucket(const ossim_float32& init_value);
      ~PcrBucket();
      ossim_float32* m_bucket;
      int m_numSamples;
   };

   void initTile();

   void addSample(std::map<ossim_int32, PcrBucket*>& accumulator,
                  ossim_int32 index,
                  const ossimPointRecord* sample);

   void normalize(std::map<ossim_int32, PcrBucket*>& accumulator);

   ossim_uint32 componentToFieldCode() const;

   ossimRefPtr<ossimPointCloudHandler> m_pch;
   ossim_float32                m_maxPixel;
   ossim_float32                m_minPixel;
   ossimDpt                     m_gsd;
   ossim_float64                m_gsdFactor;
   ossimRefPtr<ossimImageData>  m_tile;
   std::mutex                   m_mutex;
   Components                   m_activeComponent;
   std::vector<ossimString>     m_componentNames;

   TYPE_DATA
};

#endif /* ossimPointCloudRenderer_HEADER */

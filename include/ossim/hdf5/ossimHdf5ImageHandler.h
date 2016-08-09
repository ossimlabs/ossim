//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: OSSIM HDF5 Reader.
//
//----------------------------------------------------------------------------
// $Id

#ifndef ossimHdf5ImageHandler_HEADER
#define ossimHdf5ImageHandler_HEADER 1

#include <ossim/plugin/ossimPluginConstants.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/hdf5/ossimHdf5.h>
#include <ossim/hdf5/ossimHdf5ImageDataset.h>
#include <OpenThreads/Mutex>
#include <vector>
#include <string>

class OSSIM_PLUGINS_DLL ossimHdf5ImageHandler : public ossimImageHandler
{
   friend class ossimHdf5ImageDataset;

public:

   /** default constructor */
   ossimHdf5ImageHandler();

   /** virtual destructor */
   virtual ~ossimHdf5ImageHandler();

   /** @return "hdf5" */
   virtual ossimString getShortName() const;

   /** @return "ossim hdf5" */
   virtual ossimString getLongName()  const;

   /** @return "ossimH5Reader" */
   virtual ossimString getClassName()    const;

   /**
    *  Returns a pointer to a tile given an origin representing the upper
    *  left corner of the tile to grab from the image.
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
     *  Returns the number of bands in the image.
     *  Satisfies pure virtual from ImageHandler class.
     */
   virtual ossim_uint32 getNumberOfInputBands() const;

   /**
    * Returns the number of bands in a tile returned from this TileSource.
    * Note: we are supporting sources that can have multiple data objects.
    * If you want to know the scalar type of an object you can pass in the
    */
   virtual ossim_uint32 getNumberOfOutputBands()const;

   /**
     *  Returns the number of lines in the image.
     *  Satisfies pure virtual from ImageHandler class.
     */
   virtual ossim_uint32 getNumberOfLines(ossim_uint32 reduced_res_level = 0) const;

   /**
    *  Returns the number of samples in the image.
    *  Satisfies pure virtual from ImageHandler class.
    */
   virtual ossim_uint32 getNumberOfSamples(ossim_uint32 reduced_res_level = 0) const;

   /**
    * Returns the zero based image rectangle for the reduced resolution data
    * set (rrds) passed in.  Note that rrds 0 is the highest resolution rrds.
    */
   virtual ossimIrect getImageRectangle(ossim_uint32 reduced_res_level = 0) const;

   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
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
    * Returns the output pixel type of the tile source.
    */
   virtual ossimScalarType getOutputScalarType() const;

   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the ossimImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual ossim_uint32 getImageTileWidth() const;

   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the ossimImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual ossim_uint32 getImageTileHeight() const;

   virtual bool isOpen() const;

   /** Close method. */
   virtual void close();

   /**
    * @return The number of entries (images) in the image file.
    */
   virtual ossim_uint32 getNumberOfEntries()const;

   /**
    * @brief Get the name of entry as a string.
    *
    * Example given from HDF5 file:
    * 
    * entry_name: /All_Data/VIIRS-IMG-GTM-EDR-GEO_All/QF1_VIIRSGTMGEO
    *
    * @param entryIdx Zero based entry index. If out of range name will
    * be cleared.
    * 
    * @param name Initialized by this.
    */
   virtual void getEntryNames(std::vector<ossimString>& entryNames) const;

   /**
    * @param entryList This is the list to initialize with entry indexes.
    *
    * @note This implementation returns puts one entry "0" in the list.
    */
   virtual void getEntryList(std::vector<ossim_uint32>& entryList) const;

   virtual bool setCurrentEntry(ossim_uint32 entryIdx);

   /**
    * @return The current entry number.
    */
   virtual ossim_uint32 getCurrentEntry() const;

   /** @return Null pixel value. */
   virtual double getNullPixelValue(ossim_uint32 band=0)const;

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

   /** For this class only, add the specified name (can be full HDF path or pathless name only)
    * for all renderable datasets to consider. If none are specified, then all multi-dimensional
    * datasets (excluding geo sets) are considered renderable.
    */
   void addRenderableSetName(const ossimString& name);

protected:

   /** @brief Method to get geometry from hdf file. */
   virtual ossimRefPtr<ossimImageGeometry> getInternalImageGeometry();

   /**
    * @brief Gets projection from Latitude, Longitude, Height datasets if
    * present.
    *
    * @param latDataSet H5::DataSet& to layer,
    *    e.g. /All_Data/VIIRS-DNB-GEO_All/Latitude
    * @param lonDataSet H5::DataSet& to layer,
    *    e.g. /All_Data/VIIRS-DNB-GEO_All/Longitude
    */
   ossimRefPtr<ossimProjection> processCoarseGridProjection(
      H5::DataSet& latDataSet,
      H5::DataSet& lonDataSet,
      const ossimIrect& validRect ) const;

   /**
    * @brief Get dataset names for Latiitude and Longitude datasets.
    * @param h5File Pointer to file.
    * @param latName Initializes by this.
    * @param lonName Initializes by this.
    * @return true on sucess, false on error.
    */
   bool getLatLonDatasetNames(  H5::H5File* h5File,
                                std::string& latName,
                                std::string& lonName ) const;
   /**
    * @param Initializes lat and lon data sets.
    * @param h5File Pointer to file.
    * @param latDataSet Initialized by this.
    * @param lonDataSet Initialized by this.
    */
   bool getLatLonDatasets( H5::H5File* h5File,
                           H5::DataSet& latDataSet,
                           H5::DataSet& lonDataSet ) const; 
   
   bool getDataSetRect( std::string& name, std::string& lonName ) const;
   
  /**
    *  @brief open method.
    *  @return true on success, false on error.
    */
   virtual bool open();

   /** @brief Allocates the tile. */ 
   void allocate();

   std::vector<ossimString>         m_renderableNames;
   ossimRefPtr<ossimHdf5>           m_hdf5;
   std::vector<ossimHdf5ImageDataset> m_entries;
   ossim_uint32                     m_currentEntry;
   ossimRefPtr<ossimImageData>      m_tile;
   OpenThreads::Mutex               m_mutex;
   
TYPE_DATA
};

#endif /* #ifndef ossimH5Reader_HEADER */

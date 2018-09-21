//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
//*****************************************************************************
#ifndef ossimTiffHandlerState_HEADER
#define ossimTiffHandlerState_HEADER 1
#include <ossim/support_data/ImageHandlerState.h>
#include <ossim/projection/ossimProjection.h>

typedef struct tiff TIFF;  
class ossimTieGptSet;
namespace ossim
{
  /**
  * This is a Tiff handler state object.  This handles caching
  * state of a ossimTiffTileSource.  The example code provided will 
  * work for any Image handler that supports state caching.  Note,  
  * if anything changes on the input image then the state will need 
  * to be recalculated by opening the image without the state and 
  * grabbing it's state again.
  *
  * Example code to get a state of an image source:
  *
  * @code
  * #include <ossim/base/ossimKeywordlist.h>
  * #include <ossim/imaging/ossimImageHandlerRegistry.h>
  * #include <ossim/imaging/ossimImageHandler.h>
  * #include <ossim/support_data/ImageHandlerState.h>
  *
  * ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(filename);
  *
  * if(h)
  * {
  *   std::shared_ptr<ossim::ImageHandlerState> state = h->getState();
  *   if(state)
  *   {
  *     ossimKeywordlist kwl;
  *     state->save(kwl);
  *     std::cout << kwl << "\n";
  *   }
  * }
  * @endCode
  *
  * Example code using a state
  *
  * @code
  * #include <ossim/base/ossimKeywordlist.h>
  * #include <ossim/imaging/ossimImageHandlerRegistry.h>
  * #include <ossim/imaging/ossimImageHandler.h>
  * #include <ossim/support_data/ImageHandlerState.h>
  *
  * ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(filename);
  *
  * if(h)
  * {
  *   std::shared_ptr<ossim::ImageHandlerState> state = h->getState();
  *   if(state)
  *   {
  *      h = ossimImageHandlerRegistry::instance()->open(state);
  *      if(h)
  *      {
  *         std::cout << "Successfully opened with a state\n";  
  *      } 
  *   }
  * }
  * @endCode
  *
  * Small Keywordklist example from just the tiff.  Typically this is grabbed
  * from the image handler but here we will just show the tiff tags
  *
  * @code
  * connection_string:  /data/sanfran_utm.tif
  * current_entry:  0
  * tiff.image0.is_geotiff:  1
  * tiff.image0.is_tiled:  true
  * tiff.image0.tifftag.angular_units:  9108
  * tiff.image0.tifftag.bits_per_sample:  8
  * tiff.image0.tifftag.compression:  1
  * tiff.image0.tifftag.datum_code:  6326
  * tiff.image0.tifftag.false_easting:  500000
  * tiff.image0.tifftag.false_northing:  0
  * tiff.image0.tifftag.gcs_code:  4326
  * tiff.image0.tifftag.geo_double_params:  (6378137,6356752.3141999999061)
  * tiff.image0.tifftag.geo_pixel_scale:  (5,5,0)
  * tiff.image0.tifftag.geo_tie_points:  (0,0,0,538185,4208785,0)
  * tiff.image0.tifftag.image_length:  8000
  * tiff.image0.tifftag.image_width:  8000
  * tiff.image0.tifftag.linear_units:  9001
  * tiff.image0.tifftag.max_sample_value:  255
  * tiff.image0.tifftag.min_sample_value:  1
  * tiff.image0.tifftag.model_type:  1
  * tiff.image0.tifftag.origin_lat:  0
  * tiff.image0.tifftag.origin_lon:  -123
  * tiff.image0.tifftag.pcs_code:  32610
  * tiff.image0.tifftag.photometric:  2
  * tiff.image0.tifftag.planar_config:  1
  * tiff.image0.tifftag.raster_type:  2
  * tiff.image0.tifftag.sample_format:  1
  * tiff.image0.tifftag.samples_per_pixel:  3
  * tiff.image0.tifftag.scale_factor:  0.9996
  * tiff.image0.tifftag.tile_length:  64
  * tiff.image0.tifftag.tile_width:  64
  * image_handler_type:
  * number_of_directories:  1
  * type:  ossim::TiffHandlerState
  * @endCode
  */
  class OSSIM_DLL TiffHandlerState : public ossim::ImageHandlerState
  {
  public:
    TiffHandlerState();
    virtual ~TiffHandlerState();
    virtual const ossimString& getTypeName()const override;
    static const ossimString& getStaticTypeName();

    /**
    * General access to get the tags of an image
    *
    * @return ossimKeywordlist that contains the tags
    */
    ossimKeywordlist& getTags(){return m_tags;}

    /**
    * General access to the the tags of an image
    *
    * @return ossimKeywordlist that contains the tags
    */
    const ossimKeywordlist& getTags()const{return m_tags;}

    /**
    * Is used by this state to add a valud to the tags
    *
    * @param key is the identifier for the value
    * @param value is the value to add to the tags
    */
    void addValue(const ossimString& key, const ossimString& value);

    /**
    * Given a key return the value
    * 
    * @param value returns the value
    * @param key is the key holding the value
    *
    * @return true if the value was found and false otherwise
    */
    bool getValue(ossimString& value, const ossimString& key)const;


    /**
    * Convenience method to return the value of a key given the 
    * directory.  It will construct a new key from the directory.  If 
    * the directory is 0 and the key is sub_file_type then the key that
    * is actually queried internally is "tiff.image0.sub_file_type"
    * 
    * @param value returns the value
    * @param directory is the directory to return the key
    * @param key is the key holding the value
    *
    * @return true if the value was found and false otherwise
    */
    bool getValue(ossimString& value,
                  const ossim_uint32 directory, 
                  const ossimString& key)const;

    /**
    * Will return true or false if the directory and key is in the
    * tags.
    *
    * @return 
    */
    bool exists(ossim_uint32 directory, const ossimString& key)const;

    /**
    *   Will check if a key exists
    */
    bool exists(const ossimString& key)const;
    bool checkBool(const ossimString& key)const;
    bool checkBool(ossim_uint32 directory, const ossimString& key)const;
    
    /**
    * This is overriden from the base and will load the main entry.  For Tiff we 
    * do not support multiple pages yet so this value will
    * be ignored.  In the future it will be used to load the state
    * of individual pages.  For now,  what is loaded is based on the sub_file_type and
    * this entry is treated as the main entry point
    *
    * @param file file to to open
    *
    * @return true if defaults were loaded and false 
    *         otheriwse
    */
    virtual bool loadDefaults(const ossimFilename& file, ossim_uint32 entry=0)override;

    /**
    * Will load default values
    *
    * @param tiffPtr tiff pointer
    */
    void loadDefaults(TIFF* tiffPtr);

    void loadDefaults(std::shared_ptr<ossim::istream> &str,
                      const std::string &connectionString);
        /**
    * Will load only value for the current directory
    *
    * @param tiffPtr tiff pointer
    */
    void loadCurrentDirectory(TIFF *tiffPtr);

    /**
    * Will change and load the directory provided
    *
    * @param tiffPtr tiff pointer
    * @param directory the directory to load
    */
    void loadDirectory(TIFF* tiffPtr, ossim_uint32 directory);

    /**
    * Convenience method to check if the directory 
    * is a reduced image specification
    *
    * @param directory the directory to test
    */
    bool isReduced(ossim_uint32 directory)const;

    /**
    * Convenience method to check if the directory 
    * is a image mask specification
    *
    * @param directory the directory to test
    */
    bool isMask(ossim_uint32 directory)const;

    /**
    * Convenience method to check if the directory 
    * is a paged image specification
    *
    * @param directory the directory to test
    */
    bool isPage(ossim_uint32 directory)const;


    /**
    * Convenience method to check if a directory 
    * is tiled
    *
    * @param directory the directory to test
    * @return true if tiled and false otherwise
    */
    bool isTiled(ossim_uint32 directory)const;

    /**
    * Convenience method to get an Int32 value.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int32 getInt32Value(const ossimString& key, ossim_int32 directory=0)const;

    /**
    * Convenience method to get an Double value.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_float64 getDoubleValue(const ossimString& key, ossim_int32 directory=0)const;

    /**
    * Convenience method to get a pcs code.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int32 getPcsCode(ossim_int32 directory=0)const;

    /**
    * Convenience method to get a datum code.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int32 getDatumCode(ossim_int32 directory=0)const;

    /**
    * Convenience method to get a gcs code.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int32 getGcsCode(ossim_int32 directory=0)const;

    /**
    * Convenience method to get a model type.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int32 getModelType(ossim_int32 directory=0)const;

    /**
    * Convenience method to get angular units.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int32 getAngularUnits(ossim_int32 directory=0)const;

    /**
    * Convenience method to get linear units.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int32 getLinearUnits(ossim_int32 directory=0)const;

    /**
    * Convenience method to get tile height.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int64 getTileLength(ossim_int32 directory = 0) const;

    /**
    * Convenience method to get image length.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int64 getImageLength(ossim_int32 directory = 0) const;

    /**
    * Convenience method to get tile width.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int64 getTileWidth(ossim_int32 directory = 0) const;

    /**
    * Convenience method to get image width.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int64 getImageWidth(ossim_int32 directory=0)const;

    /**
    * Convenience method to get origin lat.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_float64 getOriginLat(ossim_int32 directory=0)const;

    /**
    * Convenience method to get origin lon.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_float64 getOriginLon(ossim_int32 directory=0)const;

    /**
    * Convenience method to get standard parallel 1.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_float64 getStandardParallel1(ossim_int32 directory=0)const;

    /**
    * Convenience method to get standard parallel 2.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_float64 getStandardParallel2(ossim_int32 directory=0)const;

    /**
    * Convenience method to get false easting.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_float64 getFalseEasting(ossim_int32 directory=0)const;

    /**
    * Convenience method to get false northing.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_float64 getFalseNorthing(ossim_int32 directory=0)const;

    /**
    * Convenience method to get scale factor.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_float64 getScaleFactor(ossim_int32 directory=0)const;

    /**
    * Convenience method to get a raster type.  
    *
    * This does not supply a way to check if a value existed or not.
    */
    ossim_int32 getRasterType(ossim_int32 directory = 0) const;
    ossim_int32 getPlanarConfig(ossim_int32 directory = 0) const;
    ossim_int32 getPhotoInterpretation(ossim_int32 directory = 0) const;
    ossim_int32 getCompressionType(ossim_int32 directory = 0) const;
    ossim_uint16 getBitsPerSample(ossim_int32 directory = 0) const;
    ossim_uint16 getSamplesPerPixel(ossim_int32 directory = 0) const;
    ossim_uint16 getSampleFormat(ossim_int32 directory = 0) const;
  
    ossim_uint32 getRowsPerStrip(ossim_int32 directory = 0) const;
    
    bool getMinSampleValue(ossim_float64 &minSampleValue, ossim_int32 directory = 0) const;
    bool getMaxSampleValue(ossim_float64 &maxSampleValue, ossim_int32 directory = 0) const;
    bool hasColorMap(ossim_int32 dir)const;
    bool isReduced(ossim_int32 directory = 0) const;
    bool isPage(ossim_int32 directory = 0) const;
    bool isMask(ossim_int32 directory = 0) const;
    ossim_int32 getSubFileType(ossim_int32 directory = 0) const;
    ossim_uint32 getNumberOfDirectories()const;
    bool getColorMap(std::vector<ossim_uint16> &red,
                     std::vector<ossim_uint16> &green,
                     std::vector<ossim_uint16> &blue,
                     ossim_int32 directory = 0) const;

    /**
    * Convenience method to get a double array.  
    *
    * @param result resulting double array
    * @param directory to use
    * @param key to get a double array.      
    */
    bool getDoubleArray(std::vector<ossim_float64>& result, 
                        ossim_int32 directory,
                        const ossimString& key)const;

    /**
    * Convenience method to get a Geo double params array.  
    *
    * @param result resulting double array
    * @param directory to use
    */
    bool getGeoDoubleParams(std::vector<ossim_float64>& result,
                            ossim_int32 directory)const;

    /**
    * Convenience method to get a Geo double params array.  
    *
    * @param result resulting double array
    * @param directory to use
    */
    bool getGeoPixelScale(std::vector<ossim_float64>& result,
                          ossim_int32 directory)const;

    /**
    * Convenience method to get a Geo Tie points array.  
    *
    * @param result resulting double array
    * @param directory to use
    */
    bool getGeoTiePoints(std::vector<ossim_float64>& result,
                         ossim_int32 directory)const;

    bool getCitation(ossimString& citation, ossim_int32 directory)const;
    /**
    * Convenience method to get a Geo trans matrix array.  
    *
    * @param result resulting double array
    * @param directory to use
    */
    bool getGeoTransMatrix(std::vector<ossim_float64>& result,
                           ossim_int32 directory)const;

    /**
    * Loads the the state object from keywordlist.
    *
    * @param kwl keywordlist that olds the state of the object
    * @param prefix optional prefix value that is used as a prefix 
    *        for all keywords.
    */
    virtual bool load(const ossimKeywordlist& kwl,
                           const ossimString& prefix="") override;

    /**
    * Saves the state of the object to a keyword list.
    *
    * @param kwl keywordlist that the state will be saved to
    * @param prefix optional prefix value that is used as a prefix 
    *        for all keywords.
    */
    virtual bool save(ossimKeywordlist& kwl,
                           const ossimString& prefix="")const override;
  private:
    static const ossimString m_typeName;

    /**
    * Used to save a color map.  Will save the map into the
    * tag list as arrays for colormap.red, colormap.blue, colormap.green
    * Each list is a string of the form "(valu1,valu2, .... valueN)" with 
    * the parenthesis included
    */
    void saveColorMap(const ossimString& dirPrefix,
                      const ossim_uint16* red, 
                      const ossim_uint16* green,
                      const ossim_uint16* blue,
                      ossim_uint32 numberOfEntries);

    /**
    * loads tags into the keywordlist for a given directory
    */
    void loadGeotiffTags(TIFF* tiffPtr, const ossimString& dirPrefix);

    /**
    * converts an array into a string list of the form
    *
    * (vale1,value2,...,valueN)
    */
    void convertArrayToStringList(ossimString& result, double* doubleArray, ossim_int32 doubleArraSize)const;

    ossimKeywordlist  m_tags; 
  };

}
#endif

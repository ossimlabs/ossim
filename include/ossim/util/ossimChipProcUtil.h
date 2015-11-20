//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimChipProcUtil_HEADER
#define ossimChipProcUtil_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimRectangleCutFilter.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/util/ossimUtility.h>
#include <map>
#include <vector>

/***************************************************************************************************
 * @brief Base class for all utilities that process chips of image (or DEM) pixels.
 *
 * Many utilities share parameters in common, especially ROI (bounding box), input imagery,
 * color look-up tables, DEM file specification, etc. This class does the following:
 *   * Consolidates the parsing of common parameters,
 *   * Provides for creating input chains with needed handlers and resamplers. The derived classes
 *     would finish populating the processing chain(s) according to their requirements.
 *   * Inserts the chipper filter at the end of the processing chain
 *   * Appends the appropriate writer
 *   * Provides a functional, default implementation of execute() and getChip()
 *
 * @note Almost all methods use throw for stack unwinding.  This is not in
 * method declarations to alleviate build errors on windows.  Sorry...
 **************************************************************************************************/
class OSSIM_DLL ossimChipProcUtil : public ossimUtility
{
public:
   enum OutputProjectionType
   {
      UNKNOWN_PROJ    = 0,
      GEO_PROJ        = 1,
      GEO_SCALED_PROJ = 2,
      INPUT_PROJ      = 3,
      UTM_PROJ        = 4,
      IDENTITY        = 5
   };

   /** default constructor */
   ossimChipProcUtil();

   /** virtual destructor */
   virtual ~ossimChipProcUtil();

   virtual void setUsage(ossimArgumentParser& ap);

   /**
    * @brief Disconnects and clears the dem and image layers.
    */
   virtual void clear();
   
   /**
    * Initial method to be ran prior to execute. Intended for command-line app usage.
    * @param ap Arg parser to initialize from.
    * @note Throws ossimException on error.
    * @note A throw with an error message of "usage" is used to get out when
    * a usage is printed.
    */
   virtual void initialize(ossimArgumentParser& ap);

   /**
    * This method is responsible for completely setting up the full processing chain according to
    * the specifications given in the kwl passed in. If the utility is run from a command line,
    * the initialize(ossimArgumentParser) will assign the member master KWL and pass it to this
    * method. Web service calls will fill a KWL and pass it.
    *
    * This base class implementation should be called by the derived class implementation (assuming
    * the derived class needs to pull some parameters out of the KWL before the chains are set up.
    *
    * This method will instantiate the output projection and define the output bounding rect and
    * product size in pixels. The processing chain (stored in m_procChain) will be completely
    * initialized and ready for calls to getTile(). Then either getChip() or execute() can be called
    * depending on usage to fetch product.
    * @note Throws ossimException on error.
    */
   virtual void initialize(const ossimKeywordlist& kwl);

   /**
    * @brief execute method.  Performs the actual product write.
    * @note Throws ossimException on error.
    */
   virtual bool execute();

   virtual void abort();

   /**
    * Overrides base class implementation to indicate this class supports getChip() calls.
    * Can be done with dunamic cast and pointer test, but not sure how that is supported in SWIG
    * (OLK 11/2015).
    */
   virtual bool isChipProcessor() const { return true; }

   /**
   * The meat and potatos of this class. Performs an execute on specified rect.
   */
   ossimRefPtr<ossimImageData> getChip(const ossimGrect& gnd_rect);
   ossimRefPtr<ossimImageData> getChip(const ossimIrect& img_rect);

   /**
    * @brief Gets the output file name.
    * @param f Initialized by this with the filename.
    */
   void getOutputFilename(ossimFilename& f) const;

protected:
   /**
    * Intended to be called after derived class has picked off its own options from the parser, and
    * arguments remain (such as input and output filenames).
    * @note Throws ossimException on error.
    */
   void processRemainingArgs(ossimArgumentParser& ap);

   /**
    * Derived classes initialize their custom chains here.
    */
   virtual void initProcessingChain() = 0;

   /**
    * Sets up the AOI box cutter filter and related stuff and initializes area of interest(aoi).
    * The filter is appended to the current m_procChain.
    */
   virtual void appendCutRectFilter();

   /** @brief Creates chains for image entries associated with specified keyword. This is usually
    * the input image sources but could also be used for reading list of color sources
    * @note Throws ossimException on error.
    */
   void loadImageFiles();
   
   /** Loads all DEM files specified in master KWL into the elev manager's database */
   void loadDemFiles();

   /**
    * @brief Creates the ossimSingleImageChain from file and populates the chain with the reader
    * handler ONLY. Derived classes must finish constructing the chain according to their
    * processing needs.
    *
    * @param file File to open.
    * @param entryIndex Entry to open.
    * @return Ref pointer to ossimSingleImageChain.
    * @note Throws ossimException on error.
    */
   ossimRefPtr<ossimSingleImageChain> createChain(const ossimFilename& file,
                                                  ossim_uint32 entryIndex=0) const;

   /**
    * @brief Creates the output or view projection.
    * @note All chains must be constructed prior to calling this.
    */
   void createOutputProjection();
   
   /**
    * @brief Sets the single image chain for identity operations view to
    * a ossimImageViewAffineTransform.  This will have a rotation if
    * up is up is selected.  Also set m_outputProjection to the input's
    * for area of interest.
    */
   void createIdentityProjection();

   /**
    * @brief Gets the first input projection.
    *
    * This gets the output projection of the first dem layer if present;
    * if not, the first image layer.
    * 
    * @return ref ptr to projection, could be null.
    */
   ossimRefPtr<ossimMapProjection> getFirstInputProjection();

   /**
    * @brief Convenience method to get geographic projection.
    * @return new ossimEquDistCylProjection.
    */
   ossimRefPtr<ossimMapProjection> getNewGeoProjection();

   /**
    * @brief Convenience method to get geographic projection.
    *
    * This method sets the origin to the center of the scene bounding rect
    * of all layers.
    * @return new ossimEquDistCylProjection.
    */
   ossimRefPtr<ossimMapProjection> getNewGeoScaledProjection();

    /**
    * @brief Convenience method to get a projection from an srs code.
    * @return new ossimMapProjection.
    */  
   ossimRefPtr<ossimMapProjection> getNewProjectionFromSrsCode( const std::string& code );

   /**
    * @brief Convenience method to get a utm projection.
    * @return new ossimUtmProjection.
    */     
   ossimRefPtr<ossimMapProjection> getNewUtmProjection();

   /**
    * @brief Convenience method to get a pointer to the  output map
    * projection.
    *
    * Callers should check for valid() as the pointer could be
    * 0 if not initialized.
    * 
    * @returns The ossimMapProjection* from the m_outputGeometry as a ref
    * pointer.
    */
   ossimRefPtr<ossimMapProjection> getMapProjection();

   /**
    * @brief Sets the projection tie point to the scene bounding rect corner.
    * @note Throws ossimException on error.
    */
   void intiailizeProjectionTiePoint();

   /**
    * @brief Initializes the projection gsd.
    *
    * This loops through all chains to find the best resolution gsd.
    *
    * @note Throws ossimException on error.
    */
   void initializeProjectionGsd();   

   /**
    * @brief Initializes the image view transform(IVT) scale.
    *
    * Chip mode only. Sets IVT scale to output / input.
    * 
    * @note Throws ossimException on error.
    */
   void initializeIvtScale();   

   /**
    * @brief Loops through all layers to get the upper left tie point.
    * @param tie Point to initialize.
    */
   void getTiePoint(ossimGpt& tie);

   /**
    * @brief Gets the upper left tie point from a chain.
    * @param chain The chain to get tie point from.
    * @param tie Point to initialize.
    * @note Throws ossimException on error.
    */
   void getTiePoint(ossimSingleImageChain* chain, ossimGpt& tie);

   /**
    * @brief Loops through all layers to get the upper left tie point.
    * @param tie Point to initialize.
    */
   void getTiePoint(ossimDpt& tie);

   /**
    * @brief Gets the upper left tie point from a chain.
    * @param chain The chain to get tie point from.
    * @param tie Point to initialize.
    * @note Throws ossimException on error.
    */
   void getTiePoint(ossimSingleImageChain* chain, ossimDpt& tie);

   /**
    * @brief Loops through all layers to get the best gsd.
    * @param gsd Point to initialize.
    */
   void getMetersPerPixel(ossimDpt& gsd);

   /**
    * @brief Gets the gsd from a chain.
    * @param chain The chain to get gsd from.
    * @param gsd Point to initialize.
    * @note Throws ossimException on error.
    */   
   void getMetersPerPixel(ossimSingleImageChain* chain, ossimDpt& gsd);

   /**
    * @brief Gets value of key "central_meridan" if set, nan if not.
    *
    * @return Value as a double or nan if keyord is not set.
    * 
    * @note Throws ossimException on range error.
    */
   ossim_float64 getCentralMeridian() const;

   /**
    * @brief Gets value of key "origin_latitude" if set, nan if not.
    *
    * @return Value as a double or nan if keyord is not set.
    * 
    * @note Throws ossimException on range error.
    */
   ossim_float64 getOriginLatitude() const;

   /**
    * @brief Loops through all layers to get the scene center ground point.
    * @param gpt Point to initialize.
    * @note Throws ossimException on error.
    */
   void getSceneCenter(ossimGpt& gpt);

   /**
    * @brief Gets the scene center from a chain.
    * @param chain The chain to get scene center from.
    * @param gpt Point to initialize.
    * @note Throws ossimException on error.
    */   
   void getSceneCenter(ossimSingleImageChain* chain, ossimGpt& gpt);

   /**
    * @brief Creates a new writer.
    *
    * This will use the writer option (-w or --writer), if present; else,
    * it will be derived from the output file extention.
    *
    * This will also set any writer properties passed in.
    *
    * @return new ossimImageFileWriter.
    * @note Throws ossimException on error.
    */
   ossimRefPtr<ossimImageFileWriter> createNewWriter() const;

   /**
    * @brief loops through all chains and sets the output projection.
    * @note Throws ossimException on error.
    */
   void propagateOutputProjectionToChains();

   /**
    * @brief Creates ossimIndexToRgbLutFilter and appends it to source. The source is modified
    * to point to the new chain head.
    * @note Throws ossimException on error.
    */
   void addIndexToRgbLutFilter(ossimRefPtr<ossimImageSource> &source) const;

   /**
    * When multiple input sources are present, this method instantiates a combiner and adds inputs
    * @return Reference to the combiner.
    */
   ossimRefPtr<ossimImageSource>
   combineLayers(std::vector< ossimRefPtr<ossimSingleImageChain> >& layers) const;

   /**
    * @brief Creates ossimScalarRemapper and connects to source.  The source is modified
    * to point to the new chain head.
    * @param Source to connect to.
    * @param scalar Scalar type.
    * @note Throws ossimException on error.
    */
   void addScalarRemapper(ossimRefPtr<ossimImageSource> &source,
                          ossimScalarType scalar) const;

   /**
    * @brief Set up ossimHistogramRemapper for a chain.
    * @param chain Chain to set up.
    * @return true on success, false on error.
    */
   bool setupChainHistogram( ossimRefPtr<ossimSingleImageChain>& chain) const;

   /**
    * @brief Sets entry for a chain.
    * @param chain Chain to set up.
    * @param entryIndex Zero based index.
    * @return true on success, false on error.
    */
   bool setChainEntry( ossimRefPtr<ossimSingleImageChain>& chain,
                       ossim_uint32 entryIndex ) const;

   /**
    * @brief Initializes m_aoiViewRect with the output area of interest as specified in master KWL.
    *
    * Initialization will either come from user defined cut options or the
    * source bounding rect with user options taking precidence.
    *
    * @note Throws ossimException on error.
    */
   void assignAoiViewRect();

   /**
    * @brief Initializes m_aoiViewRect from specified ground rect.
    */
   void assignAoiViewRect(const ossimGrect& bbox);

   /**
    * Assigns the AOI to be the bounding rect of the union of all inputs
    */
   void setAoiToInputs();

   /**
    * @brief Method to calculate and initialize scale and area of interest
    * for making a thumbnail.
    *
    * Sets the scale of the output projection so that the adjusted rectangle
    * meets the cut rect and demension requirements set in options.
    *
    * @note Throws ossimException on error.
    */
   void initializeThumbnailProjection();

   /**
    * @brief Gets the band list if BANDS keyword is set.
    *
    * NOTE: BANDS keyword values are ONE based.  bandList values are
    * ZERO based.
    *
    * @param bandList List initialized by this.
    */
   void getBandList( std::vector<ossim_uint32>& bandList ) const;

   /** @return true if brightness or contrast option is set; false, if not. */
   bool hasBrightnesContrastOperation() const;

   /** @return true if any Geo Poly cutter option is set */
   bool hasGeoPolyCutterOption()const;
   
   /** @return true if thumbnail option is set; false, if not. */
   bool hasThumbnailResolution() const;

   /** @return true if histogram option is set; false, if not. */
   bool hasHistogramOperation() const;

   /**
    * @brief Gets the emumerated output projection type.
    *
    * This looks in m_kwl for ossimKeywordNames::PROJECTION_KW.
    * @return The enumerated output projection type.
    * @note This does not cover SRS keyword which could be any type of projection.
    */
   OutputProjectionType getOutputProjectionType() const;

   /**
    * @brief Returns the scalar type from OUTPUT_RADIOMETRY_KW keyword if
    * present. Deprecated SCALE_2_8_BIT_KW is also checked.
    *
    * @return ossimScalarType Note this can be OSSIM_SCALAR_UNKNOWN if the
    * keywords are not present.
    */
   ossimScalarType getOutputScalarType() const;

   /** @return true if scale to eight bit option is set; false, if not. */
   bool scaleToEightBit() const;

   /**
    * @brief Gets the image space scale.
    *
    * This is a "chip" operation only.
    *
    * Keys: 
    * IMAGE_SPACE_SCALE_X_KW
    * IMAGE_SPACE_SCALE_Y_KW
    *
    * Scale will be 1.0, 1.0 if keys not found. 
    */
   void getImageSpaceScale( ossimDpt& imageSpaceScale ) const;
   
   /**
    * @brief Gets rotation.
    *
    * @return Rotation in decimal degrees if ROTATION_KW option is set;
    * ossim::nan, if not.
    *
    * @note Throws ossimException on range error.
    */
   ossim_float64 getRotation() const;

   /** @return true if ROTATION_KW option is set; false, if not. */
   bool hasRotation() const;

   /** @return true if UP_IS_UP_KW option is set; false, if not. */
   bool upIsUp() const;

   /** @return true if NORTH_UP_KW option is set; false, if not. */
   bool northUp() const;

   /** @return true if key is set to true; false, if not. */
   bool keyIsTrue( const std::string& key ) const;

   /**
    * @return The entry number if set.  Zero if ossimKeywordNames::ENTRY_KW not
    * found.
    */
   ossim_uint32 getEntryNumber() const;

   /**
    * @return The zone if set.  Zero if ossimKeywordNames::ZONE_KW not
    * found.
    */
   ossim_int32 getZone() const;

   /**
    * @return The hemisphere if set. Empty string if
    * ossimKeywordNames::HEMISPHERE_KW not found.
    */
   std::string getHemisphere() const;

   /**
    * @return True if any input has a sensor model input, false if all input
    * projections are map projections.
    */
   bool hasSensorModelInput();

   /**
    * @return true if all size cut box width height keywords are true.
    */
   bool hasCutBoxWidthHeight() const;

   /**
   *  @return true if the WMS style cut and the width and height keywords are set
   */
   bool hasWmsBboxCutWidthHeight() const;

   /**
    * @return true if meters, degrees or cut box with width and height option.
    */  
   bool hasScaleOption() const;
   
   /**
    * @brief Passes reader properties to single image handler if any.
    * @param ih Image handler to set properties on.
    */
   void setReaderProps( ossimImageHandler* ih ) const;
   
   void getClipPolygon(ossimGeoPolygon& polygon)const;
   /**
    * @brief Gets the brightness level.
    * 
    * This will return 0.0 if the keyword is not found or if the range check
    * is not between -1.0 and 1.0.
    *
    * @return brightness
    */
   ossim_float64 getBrightness() const;

   /**
    * @brief Gets the contrast level.
    * 
    * This will return 1.0 if the keyword is not found or if the range check
    * is not between 0.0 and 20.0.
    *
    * @return brightness
    */   
   ossim_float64 getContrast() const;

   /**
    * @brief Gets the sharpen mode.
    *
    * Valid modes: light, heavy
    * 
    * @return sharpness mode
    */   
   std::string getSharpenMode() const;

   /** @brief Hidden from use copy constructor. */
   ossimChipProcUtil( const ossimChipProcUtil& /* obj */) : m_projIsIdentity(false) {}

   /** @brief Hidden from use assignment operator. */
   const ossimChipProcUtil& operator=( const ossimChipProcUtil& /*rhs*/ ) { return *this; }

   ossimRefPtr<ossimImageSource> createCombiner()const;

   ossimKeywordlist m_kwl;
   ossimRefPtr<ossimImageGeometry> m_geom; //> Product chip/image geometry
   ossimIrect m_aoiViewRect;
   ossimRefPtr<ossimImageViewAffineTransform> m_ivt;
   std::vector< ossimRefPtr<ossimSingleImageChain> > m_imgLayers;
   std::vector< ossimFilename > m_demSources; //> Stores list of DEMs provided to the utility (versus pulled from the elevation database)
   mutable ossimRefPtr<ossimImageFileWriter> m_writer;
   ossimRefPtr<ossimImageSource> m_procChain;
   ossimRefPtr<ossimRectangleCutFilter> m_cutRectFilter;
   bool m_projIsIdentity;
   ossimFilename m_lutFile;
};

#endif /* #ifndef ossimChipProcUtil_HEADER */

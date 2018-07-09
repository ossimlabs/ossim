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
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimRectangleCutFilter.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimListenerManager.h>
#include <ossim/util/ossimTool.h>
#include <map>
#include <vector>

/**************************************************************************************************
 * Base class for all utilities that process chips of image (or DEM) pixels.
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
class OSSIM_DLL ossimChipProcTool : public ossimTool,
                                    public ossimProcessInterface,
                                    public ossimListenerManager

{
public:
   /** default constructor */
   ossimChipProcTool();

   /** virtual destructor */
   virtual ~ossimChipProcTool();

   virtual void setUsage(ossimArgumentParser& ap);

   /** Disconnects and clears the dem and image layers. */
   virtual void clear();
   
   /** Initial method to be ran prior to execute. Intended for command-line app usage.
    * @param ap Arg parser to initialize from.
    * @return FALSE if --help option requested or no params provided, so that derived classes can
    * @note Throws ossimException on error.
    * @note A throw with an error message of "usage" is used to get out when a usage is printed. */
   virtual bool initialize(ossimArgumentParser& ap);

   /** This method is responsible for completely setting up the full processing chain according to
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
    * @note Throws ossimException on error. */
   virtual void initialize(const ossimKeywordlist& kwl);

   /** Performs the actual product write.
    * @note Throws ossimException on error. */
   virtual bool execute();

   virtual void abort();

   /** Overrides base class implementation to indicate this class supports getChip() calls.
    * Can be done with dunamic cast and pointer test, but not sure how that is supported in SWIG
    * (OLK 11/2015). */
   virtual bool isChipProcessor() const { return true; }

   virtual ossimListenerManager* getManager();
   virtual ossimObject* getObject();
   virtual const ossimObject* getObject() const;

   /** The meat and potatos of this class. Performs an execute on specified rect. */
   virtual ossimRefPtr<ossimImageData> getChip(const ossimIrect& img_rect);
   virtual ossimRefPtr<ossimImageData> getChip(const ossimGrect& gnd_rect);
   ossimRefPtr<ossimImageData> getChip(const ossimDrect& map_bounding_rect, const ossimDpt& gsd);

   /**
    * @brief Get chip method that assumes pre-initialized state.
    * @return ossimImageData object. This can be null/not-valid if not
    * initialized.
    */
   ossimRefPtr<ossimImageData> getChip();

   const ossimFilename& getProductFilename() const { return m_productFilename; }

protected:
   /** Intended to be called after derived class has picked off its own options from the parser, and
    * arguments remain (such as input and output filenames).
    * @note Throws ossimException on error. */
   void processRemainingArgs(ossimArgumentParser& ap);

   /** Derived classes initialize their custom chains here. */
   virtual void initProcessingChain() = 0;

   /** Called after initProcessingChain() to append common items to the processing chain.
    * Sets up the AOI box cutter filter and related stuff and initializes area of interest(aoi).
    * The filter is appended to the current m_procChain. Derived class should override if not
    * applicable to its processing chain. */
   virtual void finalizeChain();

   /** Creates chains for image entries associated with specified keyword. This is usually
    * the input image sources but could also be used for reading list of color sources.
    * @note Throws ossimException on error. */
   virtual void loadImageFiles();
   
   /** Loads all DEM files specified in master KWL into the elev manager's database */
   virtual void loadDemFiles();

   /** Creates the ossimSingleImageChain from image filename and populates the chain with resampler
    * and product output projection on view-side of resampler's IVT.
    * @param file File to open.
    * @param entryIndex Entry to open.
    * @return Ref pointer to ossimSingleImageChain.
    * @note Throws ossimException on error. */
   ossimRefPtr<ossimSingleImageChain> createInputChain(const ossimFilename& image_file,
                                                       ossim_uint32 entry_index=0);

   /** Creates the output or view projection.
    * @note All chains must be constructed prior to calling this. */
   void createOutputProjection();
   
   /** Sets the single image chain for identity operations view to a ossimImageViewAffineTransform.
    *  This will have a rotation if up is up is selected.  Also set m_outputProjection to the
    *  input's for area of interest. */
   ossimRefPtr<ossimMapProjection>  newIdentityProjection();

   /** Convenience method to get a utm projection.
    * @return new ossimUtmProjection. */
   ossimRefPtr<ossimMapProjection> newUtmProjection();

   /** Initializes the projection gsd. This loops through all chains to find the minimum gsd.
    * @note Throws ossimException on error. */
   virtual void initializeProjectionGsd();

   /** Initializes m_aoiViewRect with the output area of interest as specified in master KWL.
    *  Initialization will either come from user defined cut options or the
    *  source bounding rect with user options taking precidence.
    * @note Throws ossimException on error. */
   virtual void initializeAOI();

   /** Reads the KWL for origin latitude and central meridian.
    * @param gpt Point to initialize. Set to 0 unless lat or lon specified in KWL
    * @return false if no items found in KWL.
    * @note Throws ossimException on error. */
   bool getProjectionOrigin(ossimGpt& gpt);

   /** Creates a new writer. This will use the writer option (-w or --writer), if present; else,
    * it will be derived from the output file extention. This will also set any writer properties
    * passed in.
    * @return new ossimImageFileWriter.
    * @note Throws ossimException on error. */
   ossimRefPtr<ossimImageFileWriter> newWriter();

   /** Loops through all chains and sets the output projection.
    * @note Throws ossimException on error. */
   void propagateGeometryToChains();

   /** When multiple input sources are present, this method instantiates a combiner and adds inputs
    * @return Reference to the combiner. */
   ossimRefPtr<ossimImageSource>
   combineLayers(std::vector< ossimRefPtr<ossimSingleImageChain> >& layers) const;

   /** Initializes m_aoiViewRect given m_aoiGroundRect. */
   void computeAdjustedViewFromGrect();

   /** Assigns the AOI to be the bounding rect of the union of all inputs. */
   void setAoiToInputs();

   /** Gets the band list if BANDS keyword is set.
    * NOTE: BANDS keyword values are ONE based.  bandList values are ZERO based.
    * @param input image index for which the band selection applies
    * @param bandList List initialized by this. */
   void getBandList(ossim_uint32 image_idx, std::vector<ossim_uint32>& bandList ) const;

   /** Passes reader properties to single image handler if any.
    * @param ih Image handler to set properties on. */
   void setReaderProps( ossimImageHandler* ih ) const;
   
   /** Hidden from use copy constructor. */
   ossimChipProcTool( const ossimChipProcTool& obj );

   /** Hidden from use assignment operator. */
   const ossimChipProcTool& operator=( const ossimChipProcTool& /*rhs*/ ) { return *this; }

   ossimRefPtr<ossimGeoPolygon>  createClipPolygon()const;

   /** Tries to determine the AOI center point based on KWL entries, else returns NaNs in gpt.
    * Needed for bootstrapping the GSD computation when information in KWL is sparse */
   void findCenterGpt(ossimGpt& gpt);

   /**
    * Some utilities need to work on DEMs as images. This method determines which DEMs cover the AOI
    * and creates an image mosaic with the DEMS serving as pixel sources. Throws exceptions.
    */
   ossimRefPtr<ossimImageSource>  mosaicDemSources();

   ossimRefPtr<ossimImageGeometry> m_geom; //> Product chip/image geometry
   ossimIrect m_aoiViewRect;
   ossimGrect m_aoiGroundRect;
   std::vector< ossimRefPtr<ossimSingleImageChain> > m_imgLayers;
   std::vector< ossimFilename > m_demSources; //> Stores list of DEMs provided to the utility (versus pulled from the elevation database)
   mutable ossimRefPtr<ossimImageFileWriter> m_writer;
   ossimRefPtr<ossimImageChain> m_procChain;
   ossimRefPtr<ossimRectangleCutFilter> m_cutRectFilter;
   bool m_projIsIdentity;
   ossimDpt m_gsd; // meters
   bool m_geoScaled;
   ossimFilename m_productFilename;
   ossimScalarType m_productScalarType;
   bool m_needCutRect; // True when a specific AOI, different from the input, was requested
};

#endif /* #ifndef ossimChipProcUtil_HEADER */

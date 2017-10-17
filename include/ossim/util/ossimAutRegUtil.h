//----------------------------------------------------------------------------
// File: ossimAutRegUtil.h
// 
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Hicks
//
// Description: Utility class for autonomous registration.
//----------------------------------------------------------------------------
#ifndef ossimAutRegUtil_HEADER
#define ossimAutRegUtil_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimTieMeasurementGeneratorInterface.h>

#include <ctime>
#include <vector>
#include <iostream>


// Forward class declarations:
class ossimArgumentParser;
class ossimImageHandler;
class ossimKeywordlist;
class ossimImageViewAffineTransform;
class ossimAdjustmentExecutive;
class ossimObservationSet;

/**
 * @brief ossimAutRegUtil class.
 *
 * This is a utility class to register two images.
 *
 * @note Almost all methods use throw for stack unwinding.  This is not in
 * method declarations to alleviate build errors on windows.
 */
class OSSIM_DLL ossimAutRegUtil : public ossimReferenced
{
public:
   /** emumerated operations */
   enum ossimAutRegOperation
   {
      OSSIM_AUTREG_OP_UNKNOWN = 0,
      OSSIM_AUTREG_OP_COREG   = 1,
      OSSIM_AUTREG_OP_MASREG  = 2
   };

   /**
    * @brief default constructor
    */
   ossimAutRegUtil();
   
   /**
    * @brief virtual destructor
    */
   ~ossimAutRegUtil();

   /**
    * @brief Disconnects and clears the dem and image layers?????????????.
    */
   void clear();
   
   /**
    * @brief Initialize method, using arguments, to be run prior to execute.
    * @param ap Arg parser to initialize from.
    * @note Throws ossimException on error.
    * @note A throw with an error message of "usage" is used to get out when
    * a usage is printed.
    */
   bool initialize(ossimArgumentParser& ap);


   /**
    * @brief Initialize method, using predefined kwl, to be run prior to execute.
    * @note Throws ossimException on error.
    */
   void initialize(const ossimKeywordlist& kwl);


   /**
    * @brief Execute method.  Performs the registration process.
    * @note Throws ossimException on error.
    */
   void execute();


   // /**
   //  * @brief Summarize method
   //  */
   void summarizeSolution() const;


   // /**
   //  * @brief saveAdjustment method
   //  */
   void saveAdjustment();

   /**
    * @brief Gets the output file name.
    * @param f Initialized by this with the filename.
    */
   void getOutputFilename(ossimFilename& f) const;

private:

   /**
    * @brief Internal initialize method.
    * @note Throws ossimException on error.
    */
   void initialize();

   /**
    * @brief Internal load image pair method.
    * @return true if succsessful.
    */
   bool loadImages();

   /**
    * @brief Internal correlation region determination.
    * @return true if succsessful.
    */
   bool delineateROIs();

   /**
    * @brief Internal observation set poplulate method.
    * @return true if succsessful.
    */
   bool populateObsSet();


   /** @return true if key is set to true; false, if not. */
   bool keyIsTrue(ossimRefPtr<ossimKeywordlist> kwl, const std::string& key ) const;
                                  

   /** @return true if file extension is "src" */
   bool isSrcFile(const ossimFilename& file) const;

   /** @brief Initializes m_srcKwl if option was set. */
   void initializeSrcKwl();

   /** @brief Initializes m_ocvKwl if option was set. */
   void initializeOcvKwl();

   /** @brief Initializes m_oaxKwl if option was set. */
   void initializeOaxKwl();

   /**
    * @brief Adds application arguments to the argument parser.
    * @param ap Parser to add to.
    */
   void addArguments(ossimArgumentParser& ap);

   /**
    * @brief Configures tie measurement generator
    */
   void configureTieMeasGenerator();

   /**
    * @brief Configures adjustment executive
    */
   void configureAdjustmentExecutive();


   /** @brief Initializes arg parser and outputs usage. */
   void usage(ossimArgumentParser& ap);


   /**
    * @brief Passes reader properties to single image handler if any.
    * @param ih Image handler to set properties on.
    */
   void setReaderProps( ossimImageHandler* ih ) const;

   /**
    * @brief Sets entry for a chain.
    * @param chain Chain to set up.
    * @param entryIndex Zero based index.
    * @return true on success, false on error.
    */
   bool setChainEntry( ossimRefPtr<ossimSingleImageChain>& chain,
                       ossim_uint32 entryIndex ) const;
   
   /**
    * @brief Method to create a chain and add to img layers from file.
    * @param file Image to open.
    * @parm entryIndex Entry to open.
    */
   bool addImgSource(const ossimFilename& file,
                     ossim_uint32 entryIndex);


   /**
    * @brief Creates a ossimSingleImageChain from file.
    * @param file File to open.
    * @param entryIndex Entry to open.
    * options like histogram stretches.
    * @return Ref pointer to ossimSingleImageChain.
    * @note Throws ossimException on error.
    */
   ossimRefPtr<ossimSingleImageChain> createChain(const ossimFilename& file,
                                                  ossim_uint32 entryIndex) const;
   
   /**
    * @brief Sets the single image chain for identity operations view to
    * an ossimImageViewAffineTransform.
    */
   void createIdentityProjection(int idx);

   /** @brief Hidden from use copy constructor. */
   ossimAutRegUtil( const ossimAutRegUtil& obj );

   /** @brief Hidden from use assignment operator. */
   const ossimAutRegUtil& operator=( const ossimAutRegUtil& rhs );

   /**
    * @brief Tie point generator interface
    */
   ossimTieMeasurementGeneratorInterface* m_tGen;
   ossimRefPtr<ossimObject> m_tGenObj;
   
   /** @brief Correlation ROIs */
   std::vector<ossimIrect> m_roiRects;
   
   /** @brief OpenCV match window display flag */
   bool m_showMatchWindow;
   
   /**
    * @brief Adjustment executive interface
    */
   /** @brief Executive object */
   ossimRefPtr<ossimAdjustmentExecutive> m_adjExec;

   /** @brief Observation set */
   ossimRefPtr<ossimObservationSet> m_obsSet;

   /** @brief A priori tie point sigmas */
   ossimColumnVector3d m_LatLonHgtSigmas;
   ossimColumnVector3d m_LatLonHgtControlSigmas;


   /** @brief Report stream pointer */
   std::ostream* m_rep;


   /**  @brief Array of image source chains. */
   std::vector< ossimRefPtr<ossimSingleImageChain> > m_imgLayer;

   /** Hold all options passed into intialize. */
   ossimRefPtr<ossimKeywordlist> m_kwl;

   /** Hold contents of OpenCV config file if --ocv_config_file is used. */
   ossimRefPtr<ossimKeywordlist> m_ocvKwl;

   /** Hold contents of oax config file if --oax_config_file is used. */
   ossimRefPtr<ossimKeywordlist> m_oaxKwl;

   /** Hold contents of src file if --src is used. */
   ossimRefPtr<ossimKeywordlist> m_srcKwl;
   
   /** Enumerated operation to perform. */
   ossimAutRegOperation m_operation;
   
   /**
    * Image view transform(IVT). Only set/used in "chip"(identity) operation as
    * the IVT for the resampler(ossimImageRenderer).
    */
   ossimRefPtr<ossimImageViewAffineTransform> m_ivt[2];

   /**
    * The image geometry.  In chip mode this will be from the input image. So
    * this may or may not have a map projection. In any other mode it
    * will the view or output geometry which will be a map projection.
    */
   ossimRefPtr<ossimImageGeometry> m_geom[2];
   
   /** Control image flags. */
   bool m_controlImage[2];
};

#endif // #ifndef ossimAutRegUtil_HEADER

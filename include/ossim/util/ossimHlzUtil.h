//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimHLZUtil_HEADER
#define ossimHLZUtil_HEADER

#include <ossim/util/ossimChipProcUtil.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimLeastSquaresPlane.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/parallel/ossimJob.h>
#include <ossim/parallel/ossimJobMultiThreadQueue.h>
#include <ossim/point_cloud/ossimPointCloudHandler.h>
#include <OpenThreads/ReadWriteMutex>
#include <vector>

/*!
 *  Class for finding helicopter landing zones (HLZ) on a DEM given the final destination and max
 *  range from destination.
 */
class OSSIMDLLEXPORT ossimHlzUtil : public ossimChipProcUtil
{
public:
   ossimHlzUtil();
   ~ossimHlzUtil();

   virtual void setUsage(ossimArgumentParser& ap);
   virtual bool initialize(ossimArgumentParser& ap);
   virtual bool initialize(const ossimKeywordlist& kwl);
   virtual bool execute();
   virtual void abort() {}
   virtual ossimString getClassName() const { return "ossimHLZUtil"; }

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

protected:
   class MaskSource
   {
   public:
      MaskSource(ossimHlzUtil* hlzUtil, const ossimFilename& mask_image, bool exclude);
      ossimRefPtr<ossimImageSource> image;
      bool exclude;
      bool valid;
   };

   virtual void initializeChain();

   /** @brief Hidden from use copy constructor. */
   ossimHlzUtil( const ossimHlzUtil& obj );

   /** @brief Hidden from use assignment operator. */
   const ossimHlzUtil& operator=( const ossimHlzUtil& rhs );

   /** @brief Initializes arg parser and outputs usage. */
   bool initialize();
   void usage(ossimArgumentParser& ap);
   void addArguments(ossimArgumentParser& ap);
   bool loadDemFile();
   bool loadPcFile();
   bool loadMaskFiles();
   bool initHlzFilter();
   bool writeFile();
   void dumpProductSummary() const;
   void paintReticle();
   void createInputChain(ossimRefPtr<ossimImageHandler>& handler,
                         ossimRefPtr<ossimImageSource>& chain);
   void writeSlopeImage();
   void setProductGSD(const double& meters_per_pixel);

   ossimGpt m_destinationGpt;
   double m_slopeThreshold; // (degrees)
   double m_roughnessThreshold; // peak deviation from plane (meters)
   double m_hlzMinRadius; // meters
   ossimFilename m_demFile;
   ossimFilename m_pcFile; // optional level-2 point-cloud file
   ossimFilename m_maskFile; // optional mask specification (may be KWL file with multiple masks)
   ossimFilename m_slopeFile; // optional byproduct output
   ossimIrect m_demRect; // rect (in raster coordinates) of the input dem for AOI
   ossimIrect m_viewRect; // rect (in raster coordinates) of output image.
   ossimGrect m_gndRect;
   ossimIpt m_demFilterSize;
   ossimRefPtr<ossimImageData> m_demBuffer;
   ossimRefPtr<ossimImageData> m_outBuffer;
   double m_gsd;
   ossimDpt m_demGsd;
   ossimFilename m_productFile;
   ossim_int32 m_reticleSize;
   ossimFilename m_lutFile;
   ossimRefPtr<ossimJobMultiThreadQueue> m_jobMtQueue;
   bool m_outputSummary;
   ossim_uint32 m_jobCount;
   bool m_isInitialized;

   ossim_uint8 m_badLzValue;
   ossim_uint8 m_marginalLzValue;
   ossim_uint8 m_goodLzValue;
   ossim_uint8 m_reticleValue;
   bool m_useLsFitMethod;
   ossimRefPtr<ossimImageSource> m_combinedElevSource;
   std::vector< ossimRefPtr<ossimPointCloudHandler> > m_pcSources;
   OpenThreads::Mutex m_chainMutex;
   std::vector<MaskSource> m_maskSources;

   // For debugging:
   ossim_uint32 m_numThreads;
   double d_accumT;

   class ChipProcessorJob : public ossimJob
   {
   public:
      ChipProcessorJob(ossimHlzUtil* hlzUtil, const ossimIpt& origin, ossim_uint32 chip_id=0);

      virtual void start();
      virtual bool level1Test() = 0;
      bool level2Test();
      bool maskTest();

      ossimHlzUtil* m_hlzUtil;
      ossimIpt m_demChipUL;
      ossimIpt m_demChipLR;
      ossim_uint8 m_status;
      float m_nullValue;

      // ossim_uint32 m_chipId; // for debug
      static OpenThreads::ReadWriteMutex m_bufMutex;
   };

   class LsFitChipProcessorJob : public ChipProcessorJob
   {
   public:
      LsFitChipProcessorJob(ossimHlzUtil* hlzUtil, const ossimIpt& origin, ossim_uint32 chip_id=0)
         : ChipProcessorJob(hlzUtil, origin, chip_id),
           m_plane (new ossimLeastSquaresPlane) {}

      ~LsFitChipProcessorJob() { delete m_plane; }
      virtual bool level1Test();
      ossimLeastSquaresPlane* m_plane;
   };

   class NormChipProcessorJob : public ChipProcessorJob
   {
   public:
      NormChipProcessorJob(ossimHlzUtil* hlzUtil, const ossimIpt& origin, ossim_uint32 chip_id=0)
         : ChipProcessorJob(hlzUtil, origin, chip_id) {}

      virtual bool level1Test();
   };

};

#endif

//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimHLZUtil_HEADER
#define ossimHLZUtil_HEADER

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
#include <ossim/imaging/ossimMemoryImageSource.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/parallel/ossimJob.h>
#include <ossim/parallel/ossimJobMultiThreadQueue.h>
#include <ossim/point_cloud/ossimPointCloudHandler.h>
#include <ossim/util/ossimChipProcTool.h>
#include <vector>

#include <mutex>
/*!
 *  Class for finding helicopter landing zones (HLZ) on a DEM given the final destination and max
 *  range from destination.
 */
class OSSIMDLLEXPORT ossimHlzTool : public ossimChipProcTool
{
public:
   ossimHlzTool();
   ~ossimHlzTool();

   virtual void setUsage(ossimArgumentParser& ap);
   virtual bool initialize(ossimArgumentParser& ap);
   virtual void initialize(const ossimKeywordlist& kwl);
   virtual bool execute();
   virtual ossimRefPtr<ossimImageData> getChip(const ossimIrect& img_rect);

   virtual ossimString getClassName() const { return "ossimHlzUtil"; }

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

protected:
   class MaskSource
   {
   public:
      MaskSource(ossimHlzTool* hlzUtil, const ossimFilename& mask_image, bool exclude);
      ossimRefPtr<ossimSingleImageChain> image;
      bool exclude;
   };

   virtual void initProcessingChain();

   /** @brief Hidden from use copy constructor. */
   ossimHlzTool( const ossimHlzTool& obj );

   /** @brief Hidden from use assignment operator. */
   const ossimHlzTool& operator=( const ossimHlzTool& rhs );

   /** @brief Initializes arg parser and outputs usage. */
   void usage(ossimArgumentParser& ap);
   void addArguments(ossimArgumentParser& ap);
   void loadPcFiles(); // throws exception
   void loadMaskFiles(); // throws exception
   void writeSlopeImage();
   void setProductGSD(const double& meters_per_pixel);
   bool computeHLZ();

   double m_slopeThreshold; // (degrees)
   double m_roughnessThreshold; // peak deviation from plane (meters)
   double m_hlzMinRadius; // meters
   ossimFilename m_slopeFile; // optional byproduct output
   ossimIpt m_demFilterSize;
   ossimRefPtr<ossimImageData> m_demBuffer;
   ossimRefPtr<ossimImageData> m_outBuffer;
   ossimRefPtr<ossimMemoryImageSource> m_memSource;
   ossim_uint8 m_badLzValue;
   ossim_uint8 m_marginalLzValue;
   ossim_uint8 m_goodLzValue;
   bool m_useLsFitMethod;
   ossimRefPtr<ossimImageSource> m_combinedElevSource;
   std::vector< ossimRefPtr<ossimPointCloudHandler> > m_pcSources;
   std::vector<MaskSource> m_maskSources;

   // For debugging:
   ossim_uint32 m_numThreads;
   double d_accumT;

   class PatchProcessorJob : public ossimJob
   {
   public:
      PatchProcessorJob(ossimHlzTool* hlzUtil, const ossimIpt& origin, ossim_uint32 chip_id=0);

      virtual bool level1Test() = 0;
      bool level2Test();
      bool maskTest();

      ossimHlzTool* m_hlzUtil;
      ossimIpt m_demPatchUL;
      ossimIpt m_demPatchLR;
      ossim_uint8 m_status;
      float m_nullValue;

      // ossim_uint32 m_chipId; // for debug
      static std::mutex m_bufMutex;

   protected:
      virtual void run();

   };

   class LsFitPatchProcessorJob : public PatchProcessorJob
   {
   public:
      LsFitPatchProcessorJob(ossimHlzTool* hlzUtil, const ossimIpt& origin, ossim_uint32 chip_id=0)
         : PatchProcessorJob(hlzUtil, origin, chip_id),
           m_plane (new ossimLeastSquaresPlane) {}

      ~LsFitPatchProcessorJob() { delete m_plane; }
      virtual bool level1Test();
      ossimLeastSquaresPlane* m_plane;
   };

   class NormPatchProcessorJob : public PatchProcessorJob
   {
   public:
      NormPatchProcessorJob(ossimHlzTool* hlzUtil, const ossimIpt& origin, ossim_uint32 chip_id=0)
         : PatchProcessorJob(hlzUtil, origin, chip_id) {}

      virtual bool level1Test();
   };

};

#endif

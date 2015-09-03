//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Oscar Kramer
//
//*************************************************************************
// $Id: ossimHLZUtil.h 23465 2015-08-13 13:36:26Z okramer $

#ifndef ossimHLZUtil_HEADER
#define ossimHLZUtil_HEADER

#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimArgumentParser.h>
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
class OSSIMDLLEXPORT ossimHLZUtil : public ossimObject,
                                    public ossimProcessInterface,
                                    public ossimListenerManager
{
public:
   ossimHLZUtil();
   ~ossimHLZUtil();

   /**
    * Initializes from command line arguments.
    */
   bool parseCommand(ossimArgumentParser& ap);

   /*
    * Initializes after parameter set-methods have been called (in lieu of argument parser init)
    */
   bool initialize();

   void setAoiRadius(const double& meters) { m_aoiRadius = meters; }
   void setDestinationGpt(const ossimGpt& dest) { m_destinationGpt = dest; }
   void setRoughnessThreshold(const double& meters) { m_roughnessThreshold = meters; }
   void setSlopeThreshold(const double& degrees) { m_slopeThreshold = degrees; }
   void setLZRadiusThreshold(const double& meters) { m_hlzMinRadius = meters; }
   void setProductGSD(const double& meters_per_pixel);
   void setOutputFileName(const ossimFilename& fname) { m_productFile = fname; }

   /**
    * Returns true if successful
    */
   virtual bool execute();

   virtual ossimObject* getObject() { return this; }
   virtual const ossimObject* getObject() const  { return this; }
   virtual ossimListenerManager* getManager()  { return this; };

   void printApiJson(ostream& out) const;

protected:
   class MaskSource
   {
   public:
      MaskSource(ossimHLZUtil* hlzUtil, ossimKeywordlist& kwl, ossim_uint32 index);
      MaskSource(ossimHLZUtil* hlzUtil, const ossimFilename& mask_image);
      ossimRefPtr<ossimImageSource> image;
      bool exclude;
      bool valid;
   };

   /** @brief Initializes arg parser and outputs usage. */
   void usage(ossimArgumentParser& ap);
   void addArguments(ossimArgumentParser& ap);
   bool loadDemFile();
   bool loadPcFile();
   bool loadMaskFiles();
   bool initProcessingChain();
   bool initHlzFilter();
   bool writeFile();
   void dumpProductSummary() const;
   void paintReticle();
   void createInputChain(ossimRefPtr<ossimImageHandler>& handler,
                         ossimRefPtr<ossimImageSource>& chain);
   void writeSlopeImage();

   ossimGpt m_destinationGpt;
   double m_slopeThreshold; // (degrees)
   double m_roughnessThreshold; // peak deviation from plane (meters)
   double m_hlzMinRadius; // meters
   ossimFilename m_demFile;
   ossimFilename m_pcFile; // optional level-2 point-cloud file
   ossimFilename m_maskFile; // optional mask specification (may be KWL file with multiple masks)
   ossimFilename m_slopeFile; // optional byproduct output
   double m_aoiRadius; // meters
   ossimRefPtr<ossimImageGeometry> m_productGeom;
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
      ChipProcessorJob(ossimHLZUtil* hlzUtil, const ossimIpt& origin, ossim_uint32 chip_id=0);

      virtual void start();
      virtual bool level1Test() = 0;
      bool level2Test();
      bool maskTest();

      ossimHLZUtil* m_hlzUtil;
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
      LsFitChipProcessorJob(ossimHLZUtil* hlzUtil, const ossimIpt& origin, ossim_uint32 chip_id=0)
         : ChipProcessorJob(hlzUtil, origin, chip_id),
           m_plane (new ossimLeastSquaresPlane) {}

      ~LsFitChipProcessorJob() { delete m_plane; }
      virtual bool level1Test();
      ossimLeastSquaresPlane* m_plane;
   };

   class NormChipProcessorJob : public ChipProcessorJob
   {
   public:
      NormChipProcessorJob(ossimHLZUtil* hlzUtil, const ossimIpt& origin, ossim_uint32 chip_id=0)
         : ChipProcessorJob(hlzUtil, origin, chip_id) {}

      virtual bool level1Test();
   };

};

#endif

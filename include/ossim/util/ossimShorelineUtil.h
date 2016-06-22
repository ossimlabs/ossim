//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimShorelineUtil_HEADER
#define ossimShorelineUtil_HEADER

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
#include <ossim/imaging/ossimMemoryImageSource.h>
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
class OSSIMDLLEXPORT ossimShorelineUtil : public ossimChipProcUtil
{
public:
   enum AlgorithmId { UNKNOWN, NDWI, AWEI, PAN_THRESHOLD };
   enum ThresholdMode { NONE=0, MEAN=1, SIGMA=2, VARIANCE=3, VALUE=4 };

   ossimShorelineUtil();
   ~ossimShorelineUtil();

   virtual void setUsage(ossimArgumentParser& ap);
   virtual bool initialize(ossimArgumentParser& ap);
   virtual void initialize(const ossimKeywordlist& kwl);
   virtual ossimRefPtr<ossimImageData> getChip(const ossimIrect& img_rect);
   virtual bool execute();

   virtual ossimString getClassName() const { return "ossimShorelineUtil"; }

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;


protected:
   virtual void initProcessingChain();
   void initLandsat8();

   /** @brief Hidden from use copy constructor. */
   ossimShorelineUtil( const ossimShorelineUtil& obj );

   /** @brief Hidden from use assignment operator. */
   const ossimShorelineUtil& operator=( const ossimShorelineUtil& rhs );

   /** @brief Initializes arg parser and outputs usage. */
   void usage(ossimArgumentParser& ap);
   void addArguments(ossimArgumentParser& ap);
   bool addPropsToJSON();
   bool autoComputeThreshold();

   ossim_uint8 m_waterValue;
   ossim_uint8 m_marginalValue;
   ossim_uint8 m_landValue;
   ossimString m_sensor;
   double m_threshold;
   double m_tolerance;
   AlgorithmId m_algorithm;
   ThresholdMode m_thresholdMode;
   double m_smoothing;
   ossimFilename m_vectorFilename;
   bool m_doRaster;
   std::map<ossimString, ossimString> m_geoJsonProps;
   bool m_noVector;
};

#endif

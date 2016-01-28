//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimViewshedUtil_HEADER
#define ossimViewshedUtil_HEADER

#include <ossim/util/ossimChipProcUtil.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/parallel/ossimJob.h>
#include <ossim/parallel/ossimJobMultiThreadQueue.h>
#include <OpenThreads/ReadWriteMutex>

/*!
 *  Class for computing the viewshed on a DEM given the viewer location and max range of visibility
 */

class OSSIMDLLEXPORT ossimViewshedUtil : public ossimChipProcUtil
{
   friend class SectorProcessorJob;
   friend class RadialProcessorJob;
   friend class RadialProcessor;

public:
   ossimViewshedUtil();
   ~ossimViewshedUtil();

   /**
    * Initializes the aurgument parser with expected parameters and options. It does not output
    * anything. To see the usage, the caller will need to do something like:
    *
    *   ap.getApplicationUsage()->write(<ostream>);
    */
   virtual void setUsage(ossimArgumentParser& ap);

   /**
    * Initializes from command line arguments.
    * @note Throws ossimException on error.
    */
   virtual void initialize(ossimArgumentParser& ap);

   /**
    * Reads processing params from KWL and prepares for execute. Returns TRUE if successful.
    * @note Throws ossimException on error.
    */
   virtual void initialize(const ossimKeywordlist& kwl);

   /**
    * Writes product to output file. Returns true if successful.
    * @note Throws ossimException on error.
    */
   virtual bool execute();

   /**
    * Disconnects and clears the DEM and image layers. Leaves OSSIM initialized.
    */
   virtual void clear();

   virtual ossimString getClassName() const { return "ossimViewshedUtil"; }

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

   /** For engineering/debug */
   void test();

protected:
   class Radial
   {
   public:
      Radial() : azimuth (0), elevation (-99999999.0), insideAoi(false) {}

      // Angles are stored as arctangents: azimuth = dy/dx,  elevation = dz/dx
      double azimuth;
      double elevation;
      bool insideAoi;
   };

   virtual void initProcessingChain();
   void paintReticle();
   void initRadials();
   bool writeHorizonProfile();
   void computeRadius();
   bool optimizeFOV();

   ossimGpt  m_observerGpt;
   ossimDpt  m_observerVpt;
   double m_obsHgtAbvTer; // meters above the terrain
   double m_visRadius; // meters
   Radial** m_radials;
   bool m_initialized;
   bool m_obsInsideAoi;
   bool m_displayAsRadar; // True when explicit visRadius is supplied
   ossim_uint32 m_halfWindow; // visRadius adjusted by GSD (in pixels)
   ossimRefPtr<ossimImageData> m_outBuffer;
   ossim_uint8 m_visibleValue;
   ossim_uint8 m_hiddenValue;
   ossim_uint8 m_overlayValue;
   ossim_int32 m_reticleSize;
   bool m_simulation;
   ossimRefPtr<ossimJobMultiThreadQueue> m_jobMtQueue;
   ossim_uint32 m_numThreads;
   double m_startFov;
   double m_stopFov;
   bool m_threadBySector;
   ossimFilename m_horizonFile;
   std::map<double, double> m_horizonMap;

   // For debugging:
   double d_accumT;
   OpenThreads::Mutex d_mutex;
};

/**
 * For support of multithreading. The --tbs option directs ossimViewshedUtil to create a thread
 * for each sector (using the SectorProcessorJob). Otherwise, the threads are mapped to a single
 * radial. There are 8 sectors total (45 deg each) so a max of 8 threads are launched. Thus it may
 * be faster to use the RadialProcessorJob scheme (default) when the number of available cores > 8.
 */
class SectorProcessorJob : public ossimJob
{
   friend class ossimViewshedUtil;

private:
   SectorProcessorJob(ossimViewshedUtil* vs_util, ossim_uint32 sector, ossim_uint32 numRadials)
   : m_vsUtil (vs_util), m_sector (sector), m_numRadials (numRadials)  {}

   virtual void start();

   ossimViewshedUtil* m_vsUtil;
   ossim_uint32 m_sector;
   ossim_uint32 m_numRadials;
};


class RadialProcessorJob : public ossimJob
{
   friend class ossimViewshedUtil;

private:
   RadialProcessorJob(ossimViewshedUtil* vs_util,
                      ossim_uint32 sector,
                      ossim_uint32 radial,
                      ossim_uint32 numRadials)
   : m_vsUtil (vs_util), m_sector (sector), m_radial (radial), m_numRadials (numRadials) {}

   virtual void start();

   ossimViewshedUtil* m_vsUtil;
   ossim_uint32 m_sector;
   ossim_uint32 m_radial;
   ossim_uint32 m_numRadials;
};

/**
 * This class provides a common entry point for both SectorProcessorJob and RadialProcessorJob for
 * processing a single radial. Eventually, SectorProcessorJob can likely go away (invoked with the
 * "--tbs" command-line option, and doRadial() method can be moved into RadialProcessorJob class.
 * In the meantime, both Sector/thread and Radial/thread schemes are supported to continue
 * evaluating performance.
 */
class RadialProcessor
{
public:
   static void doRadial(ossimViewshedUtil* vs, ossim_uint32 s, ossim_uint32 r);

private:
   static OpenThreads::ReadWriteMutex m_bufMutex;
   static OpenThreads::ReadWriteMutex m_radMutex;
   RadialProcessor() {};
};

#endif

//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Oscar Kramer
//
//*************************************************************************
// $Id: ossimViewshedUtil.h 23409 2015-07-08 16:24:41Z okramer $

#ifndef ossimViewshedUtil_HEADER
#define ossimViewshedUtil_HEADER

#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/parallel/ossimJob.h>
#include <ossim/parallel/ossimJobMultiThreadQueue.h>
#include <OpenThreads/ReadWriteMutex>

/*!
 *  Class for computing the viewshed on a DEM given the viewer location and max range of visibility
 */

class OSSIMDLLEXPORT ossimViewshedUtil : public ossimObject,
                                         public ossimProcessInterface,
                                         public ossimListenerManager
{
   friend class SectorProcessorJob;
   friend class RadialProcessorJob;
   friend class RadialProcessor;

public:
   ossimViewshedUtil();
   ~ossimViewshedUtil();

   /**
    * Initializes from command line arguments.
    */
   bool initialize(ossimArgumentParser& ap);

   /*
    * Initializes after parameter set-methods have been called (in lieu of argument parser init)
    */
   bool initialize();

   /**
    * Directly assigns the maximum visibility radius property (in meters).
    */
   void setVisibilityRadius(const double& radius) { m_visRadius = radius; }

   /**
    * Directly assigns the observer's ground point property.
    */
   void setObserverGpt(const ossimGpt& vpt) { m_observerGpt = vpt; }

   void setOutputFileName(const ossimFilename& fname) { m_filename = fname; }

   /**
    * Sets the nominal output resolution in meters
    */
   void setGSD(const double& meters_per_pixel);

   /**
    * Sets the boundary azimuths in degrees for the field-of-view (clockwise from start to stop)
    */
   void setFOV(const double& start_deg, const double& stop_deg);

   /**
    * Returns true if successful
    */
   virtual bool execute();

   virtual ossimObject* getObject() { return this; }
   virtual const ossimObject* getObject() const  { return this; }
   virtual ossimListenerManager* getManager()  { return this; };

   void printApiJson(ostream& out) const;

protected:
   class Radial
   {
   public:
      Radial() : azimuth (0), elevation (-99999999.0) {}
      Radial (const double& az) : azimuth (az), elevation (-99999999.0) {}

      // Angles are stored as arctangents: azimuth = dy/dx,  elevation = dz/dx
      double azimuth;
      double elevation;
   };

   /** @brief Initializes arg parser and outputs usage. */
   void usage(ossimArgumentParser& ap);
   void addArguments(ossimArgumentParser& ap);
   bool writeFile();
   void dumpProductSummary() const;
   void paintReticle();
   void initRadials();
   bool writeHorizonProfile();
   bool writeJsonApi(const ossimFilename& outfile);

   ossimGpt  m_observerGpt;
   ossimFilename m_demFile;
   double m_obsHgtAbvTer; // meters above the terrain
   double m_visRadius; // meters
   Radial** m_radials;
   ossimRefPtr<ossimImageGeometry> m_geometry;
   bool m_initialized;
   ossimIrect m_viewRect;
   ossim_uint32 m_halfWindow; // visRadius adjusted by GSD (in pixels)
   ossimRefPtr<ossimImageData> m_outBuffer;
   double m_gsd;
   ossimFilename m_filename;
   ossim_uint8 m_visibleValue;
   ossim_uint8 m_hiddenValue;
   ossim_uint8 m_observerValue;
   ossim_int32 m_reticleSize;
   ossimFilename m_lutFile;
   bool m_simulation;
   ossimRefPtr<ossimJobMultiThreadQueue> m_jobMtQueue;
   ossim_uint32 m_numThreads;
   bool m_outputSummary;
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

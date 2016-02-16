//*****************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//*****************************************************************************

#include <ossim/util/ossimViewshedUtil.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimPolygon.h>
#include <ossim/base/ossimRtti.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossim2dTo2dShiftTransform.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/init/ossimInit.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/elevation/ossimImageElevationDatabase.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimMemoryImageSource.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>

using namespace std;

const char* ossimViewshedUtil::DESCRIPTION =
      "Computes bitmap image representing the viewshed from specified location using only "
      "DEM information.";
const string FOV_KW             = "fov";
const string HEIGHT_OF_EYE_KW   = "height_of_eye";
const string HORIZON_FILE_KW    = "horizon_file";
const string OBSERVER_KW        = "observer";
const string RADIUS_KW          = "radius";
const string RETICLE_KW         = "reticle";
const string VIEWSHED_CODING_KW = "viewshed_coding";

ossimViewshedUtil::ossimViewshedUtil()
:   m_obsHgtAbvTer (1.5),
    m_visRadius (0.0),
    m_radials (0),
    m_obsInsideAoi (true),
    m_displayAsRadar (false),
    m_halfWindow (0),
    m_outBuffer (NULL),
    m_visibleValue (1),
    m_hiddenValue (128),
    m_overlayValue (255),
    m_reticleSize(25),
    m_simulation (false),
    m_numThreads(1),
    m_startFov(0),
    m_stopFov(0),
    m_threadBySector(false),
    d_accumT(0)
{
   m_observerGpt.makeNan();
}

ossimViewshedUtil::~ossimViewshedUtil()
{
   if (m_radials)
   {
      for (int i=0; i<4; ++i)
         delete [] m_radials[i];
      delete [] m_radials;
   }
}

void ossimViewshedUtil::setUsage(ossimArgumentParser& ap)
{
   // Set the general usage:
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString appName = ap.getApplicationName();
   ossimString usageString = appName;
   usageString += " [options] <obs_lat> <obs_lon> <output-image>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->addCommandLineOption(
         "--fov <start> <end>", "Optional arguments specifying the field-of"
         "-view boundary azimuths (in degrees). By default, a 360 deg FOV is"
         " computed. The arc is taken clockwise from start to end, so for a"
         " FOV of 225 deg from W, through N to SE, start=270 and end=135");
   au->addCommandLineOption(
         "--height-of-eye <meters>", "Specifies the observers height-of-eye above the "
         "terrain in meters. Defaults to 1.5 m.");
   au->addCommandLineOption(
         "--horizon <filename>", "Experimental. Outputs the max elevation angles "
         "for all azimuths to <filename>, for horizon profiling.");
   au->addCommandLineOption(
         "--radius <meters>", "Specifies max visibility in meters. Required "
         "unless --size is specified. This option constrains output to a circle, "
         "similar to a radar display");
   au->addCommandLineOption(
         "--reticle <int>", "Specifies the size of the reticle at the observer"
         "location in pixels from the center (i.e., the radius of the reticle). "
         "Defaults to 2. A value of 0 hides the reticle. See --values option for "
         "setting reticle color.");
   au->addCommandLineOption(
         "--simulation", "For engineering/debug purposes ");
   au->addCommandLineOption(
         "--tbs", "\"Thread By Sector\". For engineering/debug purposes ");
   au->addCommandLineOption(
         "--threads <n>", "Number of threads. Defaults to use all available cores. "
         "For engineering/debug purposes ");
   au->addCommandLineOption(
         "--viewshed-coding <int int int>", "Specifies the pixel values (0-255) for the visible,"
         " hidden and overlay pixels, respectively. Defaults to visible=null (0), "
         "hidden=128, and overlay (observer position, reticle, and circumference) is "
         "highlighted with 255.");

   // Base class has its own:
   ossimChipProcUtil::setUsage(ap);

   ostringstream description;
   description << DESCRIPTION << "\n\nExamples:\n\n"
         "    "<<appName<<" --radius 50  28.0 -80.5 output-hlz.tif\n"
         "An alternate command line provides switch for observer lat and lon:\n\n"
         "    "<<appName<<" --rlz 25 --observer 28.0 -80.5  output-hlz.tif \n";
   au->setDescription(description.str());
}

void ossimViewshedUtil::initialize(ossimArgumentParser& ap)
{
   ostringstream xmsg;
   xmsg<<"ossimViewshedUtil::initialize(ossimArgumentParser) -- ";

   int numArgsExpected = 4;

   // Base class first:
   ossimChipProcUtil::initialize(ap);

   string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);
   string ts2;
   ossimArgumentParser::ossimParameter sp2(ts2);
   string ts3;
   ossimArgumentParser::ossimParameter sp3(ts3);

   if ( ap.read("--fov", sp1, sp2) )
   {
      double startFov = ossimString(ts1).toDouble();
      if (startFov < 0)
         startFov += 360.0;
      ostringstream value;
      value<<startFov<<" "<<ts2;
      m_kwl.addPair( FOV_KW, value.str() );
   }

   if ( ap.read("--hgt-of-eye", sp1) || ap.read("--height-of-eye", sp1) )
      m_kwl.addPair( HEIGHT_OF_EYE_KW, ts1 );

   if ( ap.read("--horizon", sp1) || ap.read("--horizon-file", sp1))
      m_kwl.addPair( HORIZON_FILE_KW, ts1 );

   if ( ap.read("--observer", sp1, sp2) )
   {
      ostringstream value;
      value<<ts1<<" "<<ts2;
      m_kwl.addPair( OBSERVER_KW, value.str() );
      numArgsExpected -= 2;
   }

   if ( ap.read("--radius", sp1) )
      m_kwl.addPair( RADIUS_KW, ts1 );

   if ( ap.read("--reticle", sp1) )
      m_kwl.addPair( RETICLE_KW, ts1 );

   if ( ap.read("--values", sp1, sp2, sp3) || ap.read("--viewshed-coding", sp1, sp2, sp3))
   {
      ostringstream value;
      value<<ts1<<" "<<ts2<<" "<<ts3;
      m_kwl.addPair( VIEWSHED_CODING_KW, value.str() );
   }

   // The remaining options are available only via command line (i.e., no KWL entries defined)
   if ( ap.read("--tbs") )
      m_threadBySector = true;

   if ( ap.read("--simulation") )
      m_simulation = true;

   if ( ap.read("--threads", sp1) )
      m_numThreads = ossimString(ts1).toUInt32();

   if ( ap.argc() < numArgsExpected )
   {
      xmsg<<"Expecting more arguments.";
      ap.reportError(xmsg.str());
      throw(xmsg.str());
   }
   else
   {
      ossimString latstr = ap[1];
      ossimString lonstr = ap[2];
      ostringstream value;
      value<<latstr<<" "<<lonstr;
      m_kwl.addPair( OBSERVER_KW, value.str() );
      ap.remove(1,2);
      processRemainingArgs(ap);
   }
}

void ossimViewshedUtil::initialize(const ossimKeywordlist& kwl)
{
   clear();
   ossimString value;

   value = kwl.findKey(FOV_KW);
   if (!value.empty())
   {
      vector <ossimString> coordstr;
      value.split(coordstr, ossimString(" ,"), false);
      if (coordstr.size() == 2)
      {
         m_startFov = coordstr[0].toDouble();
         m_stopFov = coordstr[1].toDouble();
         if (m_startFov < 0)
            m_startFov += 360.0;
      }
   }

   value = kwl.findKey(HEIGHT_OF_EYE_KW);
   if (!value.empty())
      m_obsHgtAbvTer = value.toDouble();

   m_horizonFile = kwl.findKey(HORIZON_FILE_KW);

   value = kwl.findKey(OBSERVER_KW);
   if (!value.empty())
   {
      vector <ossimString> coordstr;
      value.split(coordstr, ossimString(" ,"), false);
      if (coordstr.size() == 2)
      {
         m_observerGpt.lat = coordstr[0].toDouble();
         m_observerGpt.lon = coordstr[1].toDouble();
         m_observerGpt.hgt = 0.0;
      }
   }

   value = kwl.findKey(RADIUS_KW);
   if (!value.empty())
   {
      m_visRadius = value.toDouble();
      m_displayAsRadar = true;
   }

   value = kwl.findKey(RETICLE_KW);
   if (!value.empty())
      m_reticleSize = value.toInt32();

   value = kwl.findKey(VIEWSHED_CODING_KW);
   if (!value.empty())
   {
      vector <ossimString> coordstr;
      value.split(coordstr, ossimString(" ,"), false);
      if (coordstr.size() == 3)
      {
         m_visibleValue = coordstr[0].toUInt8();
         m_hiddenValue = coordstr[1].toUInt8();
         m_overlayValue = coordstr[2].toUInt8();
      }
   }

   // If running simulation, clear out all pre-loaded elevation databases:
   if (m_simulation)
   {
      ossimElevManager::instance()->clear();
      ossimElevManager::instance()->setUseGeoidIfNullFlag(false);
   }

   // Base class does most work:
   ossimChipProcUtil::initialize(kwl);
}

void ossimViewshedUtil::clear()
{
   m_observerGpt.makeNan();
   m_visRadius = 0;
   m_outBuffer = 0;
   m_horizonMap.clear();
   m_jobMtQueue = 0;
   ossimChipProcUtil::clear();
}

void ossimViewshedUtil::initProcessingChain()
{
   ostringstream xmsg;

   if (m_observerGpt.hasNans())
   {
      xmsg<<"ossimViewshedUtil:"<<__LINE__<<" Observer ground position has not been set."<<ends;
      throw ossimException(xmsg.str());
   }

   ossimElevManager* elevMgr = ossimElevManager::instance();

   // Initialize the height of eye component of observer position:
   m_observerGpt.hgt = elevMgr->getHeightAboveEllipsoid(m_observerGpt);
   m_observerGpt.hgt += m_obsHgtAbvTer;
   m_geom->worldToLocal(m_observerGpt, m_observerVpt);

   ossimRefPtr<ossimMapProjection> mapProj =
         dynamic_cast<ossimMapProjection*>(m_geom->getProjection());

   // Determine if default GSD needs to be computed.
   if (m_gsd.hasNans())
   {
      m_gsd.x = m_gsd.y = elevMgr->getMeanSpacingMeters();
      if (mapProj.valid()) // already validated but just in case
         mapProj->setMetersPerPixel(m_gsd);
   }

   // If no radius specified, need to compute R large enough to cover the requested AOI:
   if (m_visRadius == 0)
      computeRadius();
   if (m_halfWindow == 0)
      m_halfWindow = ossim::round<ossim_int32, double>(m_visRadius/m_gsd.x);
   ossim_uint32 size = 2*m_halfWindow + 1;

   // If no AOI defined, just use the visibility rectangle:
   ossimIrect visRect (ossimIpt(m_observerVpt), size, size);
   if (m_aoiViewRect.hasNans())
   {
      m_aoiViewRect = visRect;
      m_geom->localToWorld(ossimDrect(m_aoiViewRect), m_aoiGroundRect);
      m_geom->setImageSize(m_aoiViewRect.size());
   }

   // Allocate the output image buffer:
   ossimIrect bufViewRect = visRect.clipToRect(m_aoiViewRect);
   if (bufViewRect.area() == 0)
   {
      xmsg<<"ossimViewshedUtil:"<<__LINE__<<" The requested AOI rect is outside the visibility range." << ends;
      throw ossimException(xmsg.str());
   }
   m_outBuffer = ossimImageDataFactory::instance()->
         create(0, OSSIM_UINT8, 1, m_aoiViewRect.width(), m_aoiViewRect.height());
   if(!m_outBuffer.valid())
   {
      xmsg<<"ossimViewshedUtil:"<<__LINE__<<" Output buffer allocation failed." << ends;
      throw ossimException(xmsg.str());
   }
   m_outBuffer->setImageRectangle(m_aoiViewRect);

   // The processing chain for this class is simply a memory source containing the output buffer:
   m_memSource = new ossimMemoryImageSource;
   m_memSource->setImage(m_outBuffer);
   m_memSource->setImageGeometry(m_geom.get());

   // If input image(s) provided, need to combine them with the product:
   if (m_imgLayers.empty())
   {
      m_procChain->add(m_memSource.get());
   }
   else
   {
      ossimRefPtr<ossimImageSource> combiner = combineLayers(m_imgLayers);
      combiner->connectMyInputTo(m_memSource.get());
      m_procChain->add(combiner.get());
   }
}

ossimRefPtr<ossimImageData> ossimViewshedUtil::getChip(const ossimIrect& bounding_irect)
{
   ostringstream xmsg;
   if (!m_geom.valid())
      return 0;

   m_aoiViewRect = bounding_irect;
   m_geom->setImageSize( m_aoiViewRect.size() );
   m_geom->localToWorld(m_aoiViewRect, m_aoiGroundRect);

   // Allocate the output image buffer:
   m_outBuffer = ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, m_aoiViewRect.width(),
                                                           m_aoiViewRect.height());
   if (!m_outBuffer.valid() || !m_memSource.valid())
   {
      xmsg<<"ossimViewshedUtil:"<<__LINE__<<"  Error encountered allocating output image buffer.";
      throw(xmsg.str());
   }

   // Initialize the image with all points hidden:
   m_outBuffer->initialize();
   m_outBuffer->setImageRectangle(m_aoiViewRect);
   m_outBuffer->fill(m_procChain->getNullPixelValue());
   m_memSource->setImage(m_outBuffer);

   // Initialize the radials after intersecting the requested FOV with the FOV required to see the
   // full AOI (not applicable if observer inside AOI). Skip radial init if no intersection found:
   if (!optimizeFOV())
      return 0;
   initRadials();

   // The viewshed process necessarily first fills the output buffer with the complete result before
   // the writer requests a tile. Control is passed later to the base class execute() for writing.
   d_accumT = 0;

   if (m_numThreads == 0)
      m_numThreads = ossim::getNumberOfThreads();

   if (m_numThreads > 1)
   {
      ossimRefPtr<ossimJobQueue> jobQueue = new ossimJobQueue();
      for (int sector=0; sector<8; ++sector)
      {
         if (m_radials[sector] == 0)
            continue;

         if (m_threadBySector)
         {
            SectorProcessorJob* job = new SectorProcessorJob(this, sector, m_halfWindow);
            jobQueue->add(job, false);
         }
         else
         {
            for (ossim_uint32 r=0; r<=m_halfWindow; ++r)
            {
               RadialProcessorJob* job = new RadialProcessorJob(this, sector, r, m_halfWindow);
               jobQueue->add(job, false);
            }
         }
         if (needsAborting())
            return 0;
      }

      cout << "\nSubmitting "<<jobQueue->size()<<" jobs..."<<endl;
      m_jobMtQueue = new ossimJobMultiThreadQueue(jobQueue.get(), m_numThreads);

      // Wait until all radials have been processed before proceeding:
      cout << "Waiting for job threads to finish..."<<endl;
      while (m_jobMtQueue->hasJobsToProcess() || m_jobMtQueue->numberOfBusyThreads())
         OpenThreads::Thread::microSleep(250);
   }
   else
   {
      // Unthreaded processing:
      cout << "\nProcessing radials (non-threaded)..."<<endl;

      // Loop over pixels in layer for each sector:
      for (int sector=0; sector<8; ++sector)
      {
         if (m_radials[sector] == 0)
            continue;

         SectorProcessorJob spj (this, sector, m_halfWindow);
         spj.start();

         if (needsAborting())
            return 0;

      } // end loop over sectors
   }

   cout << "Finished processing radials."<<endl;
   paintReticle();

   return m_outBuffer;
}

bool ossimViewshedUtil::execute()
{
   getChip(m_aoiViewRect);

   if (!m_horizonFile.empty() && writeHorizonProfile())
      cout << "Wrote horizon profile to <"<<m_horizonFile<<">" <<endl;

   return ossimChipProcUtil::execute();
}

bool ossimViewshedUtil::optimizeFOV()
{
   bool intersects = false;

   // If the observer position lies outside of the requested AOI, we can reduce the search arc:
   if (m_aoiGroundRect.pointWithin(m_observerGpt))
      return true;

   // Determine cardinal region (N, NE, E, ...) of observer relative to AOI:
   enum CardinalDirections { N=1, S=2, E=4, W=8, NE=5, NW=9, SE=6, SW=10 };
   int direction = 0;
   if (m_observerGpt.lat > m_aoiGroundRect.ul().lat)
      direction = (int) N;
   else if (m_observerGpt.lat < m_aoiGroundRect.ll().lat)
      direction = (int) S;
   if (m_observerGpt.lon < m_aoiGroundRect.ul().lon)
      direction += (int) W;
   else if (m_observerGpt.lon > m_aoiGroundRect.ur().lon)
      direction += (int) E;

   // Calculate start and stop FOV depending on region:
   double start, stop;
   switch ((CardinalDirections) direction)
   {
   case N:
      start = m_observerGpt.azimuthTo(m_aoiGroundRect.ur());
      stop  = m_observerGpt.azimuthTo(m_aoiGroundRect.ul());
      break;
   case NE:
      start = m_observerGpt.azimuthTo(m_aoiGroundRect.lr());
      stop  = m_observerGpt.azimuthTo(m_aoiGroundRect.ul());
      break;
   case E:
      start = m_observerGpt.azimuthTo(m_aoiGroundRect.lr());
      stop  = m_observerGpt.azimuthTo(m_aoiGroundRect.ur());
      break;
   case SE:
      start = m_observerGpt.azimuthTo(m_aoiGroundRect.ll());
      stop  = m_observerGpt.azimuthTo(m_aoiGroundRect.ur());
      break;
   case S:
      start = m_observerGpt.azimuthTo(m_aoiGroundRect.ll());
      stop  = m_observerGpt.azimuthTo(m_aoiGroundRect.lr());
      break;
   case SW:
      start = m_observerGpt.azimuthTo(m_aoiGroundRect.ul());
      stop  = m_observerGpt.azimuthTo(m_aoiGroundRect.lr());
      break;
   case W:
      start = m_observerGpt.azimuthTo(m_aoiGroundRect.ul());
      stop  = m_observerGpt.azimuthTo(m_aoiGroundRect.ll());
      break;
   case NW:
      start = m_observerGpt.azimuthTo(m_aoiGroundRect.ur());
      stop  = m_observerGpt.azimuthTo(m_aoiGroundRect.ll());
      break;
   }

   // Now need to intersect this arc with the requested FOV:
   if (m_startFov == m_stopFov)
   {
      // There was no requested FOV (i.e, FOV = 360). So use the optimized FOV straight away:
      m_startFov = start;
      m_stopFov  = stop;
      intersects = true;
   }
   else
   {
      // Pick m_startFov as reference, and make sure all others are greater:
      double a1 = m_stopFov;
      double a2 = start;
      double a3 = stop;
      if (m_startFov > m_stopFov)
         a1 += 360;
      if (m_startFov > start)
         a2 += 360;
      if (m_startFov > stop)
         a3 += 360;

      // Map to sort remaining azimuths by increasing angle clockwise:
      map<double, int> angle_map;
      angle_map.insert(pair<double, int>(a1, 1));
      angle_map.insert(pair<double, int>(a2, 2));
      angle_map.insert(pair<double, int>(a3, 3));

      map<double, int>::iterator iter = angle_map.begin();
      if (iter->second == 1)
      {
         ++iter;
         if (iter->second == 3)
            intersects = true;
      }
      else if (iter->second == 2)
      {
         m_startFov = start;
         intersects = true;
         ++iter;
         if (iter->second == 3)
            m_stopFov = stop;
      }
      else
      {
         intersects = true;
         m_stopFov = stop;
      }
   }

   if (!intersects)
   {
      ossimNotify(ossimNotifyLevel_INFO)<<
            "ossimViewshedUtil::optimizeFOV() -- No FOV intersection found. Nothing to do."<<endl;
   }
   else
   {
      ossimNotify(ossimNotifyLevel_INFO)<<"ossimViewshedUtil::optimizeFOV() -- "
            "The start and stop FOV azimuths have been optimized to "<<m_startFov<<" -> "
            <<m_stopFov<<" deg."<<endl;
   }

   return intersects;
}

void ossimViewshedUtil::computeRadius()
{
   ostringstream xmsg ("ossimViewshedUtil::computeRadius() -- ");

   // AOI is required for computing R
   if (m_aoiViewRect.hasNans())
   {
      xmsg<<"AOI undefined. Cannot compute visibility radius." << ends;
      throw ossimException(xmsg.str());
   }

   // Compute distance from observer to farthest corner of AOI. This is the radius
   m_visRadius= m_observerGpt.distanceTo(m_aoiGroundRect.ul());
   double d = m_observerGpt.distanceTo(m_aoiGroundRect.ur());
   if (d > m_visRadius)
      m_visRadius = d;
   d = m_observerGpt.distanceTo(m_aoiGroundRect.lr());
   if (d > m_visRadius)
      m_visRadius = d;
   d = m_observerGpt.distanceTo(m_aoiGroundRect.ll());
   if (d > m_visRadius)
      m_visRadius = d;
}

void ossimViewshedUtil::initRadials()
{
   // All eaight sectors' radials have the same azimuths except that the abscissa and ordinate are
   // reversed between the N-S and E-W sectors, i.e., the N and S sectors use the y-axis as the
   // abscissa (u) and the x-axis is the ordinate (v). The azimuth (dv/du) is therefore DX/DY for
   // the north and south, while the azimuth is DY/DX for the east and west sectors. Nevertheless,
   // each sectors radials must be maintained separately as they contain the max elevation angle.

   // First determine which sectors are involved given the desired FOV:
   bool* sectorInFov = new bool[8];
   memset(sectorInFov, false, 8);
   bool crossed_north = true;
   if (m_stopFov <= m_startFov) // Crosses 0 azimuth
      crossed_north = false;
   double azimuth = m_startFov;
   for (int i=0; (i < 8) && ((azimuth < m_stopFov) || !crossed_north); ++i)
   {
      if ((azimuth >= 0) && (azimuth < 45.0))
         sectorInFov[0] = true;
      else if (azimuth < 90.0)
         sectorInFov[1] = true;
      else if (azimuth < 135.0)
         sectorInFov[2] = true;
      else if (azimuth < 180.0)
         sectorInFov[3] = true;
      else if (azimuth < 225.0)
         sectorInFov[4] = true;
      else if (azimuth < 270.0)
         sectorInFov[5] = true;
      else if (azimuth < 315.0)
         sectorInFov[6] = true;
      else if (azimuth < 360.0)
         sectorInFov[7] = true;

      azimuth += 45.0;
      if (azimuth >= 360.0)
      {
         azimuth -= 360.0;
         crossed_north = true;
      }
   }

   // Compute the azimuth slopes for each radial in the sector.
   m_radials = new Radial* [8];
   double du = m_halfWindow;
   for (int sector=0; sector<8; ++sector)
   {
      if (!sectorInFov[sector])
      {
         m_radials[sector] = 0;
         continue;
      }

      ossim_uint32 ridx = 0;
      m_radials[sector] = new Radial [m_halfWindow+1];
      for (ossim_int32 dv = 0; dv <= (ossim_int32) m_halfWindow; ++dv)
      {
         if (sector & 1) // odd-numbered sector, azimuths computed in reverse order
            m_radials[sector][m_halfWindow-ridx].azimuth = ((double)dv)/du;
         else
            m_radials[sector][ridx].azimuth = ((double)dv)/du;
         ++ridx;
      }
   }

   // Cleanup:
   delete [] sectorInFov;
   sectorInFov = 0;
}

void ossimViewshedUtil::paintReticle()
{
   if (m_reticleSize == 0)
      return;

   // Highlight the observer position with X reticle:
   ossimDpt obsViewPt;
   m_geom->worldToLocal(m_observerGpt, obsViewPt);
   if (m_aoiViewRect.pointWithin(ossimIpt(obsViewPt)))
   {
      for (int i=-m_reticleSize; i<=m_reticleSize; ++i)
      {
         if (m_aoiViewRect.pointWithin(ossimIpt(m_observerVpt.x + i, m_observerVpt.y)))
            m_outBuffer->setValue(m_observerVpt.x + i, m_observerVpt.y    , m_overlayValue);
         if (m_aoiViewRect.pointWithin(ossimIpt(m_observerVpt.x, m_observerVpt.y + i)))
            m_outBuffer->setValue(m_observerVpt.x    , m_observerVpt.y + i, m_overlayValue);
      }
   }

   // Paint boundary rectangle if no visibility radius painted:
   if (!m_displayAsRadar)
   {
      for (int y=m_aoiViewRect.ul().y; y<=m_aoiViewRect.lr().y; y++)
      {
         m_outBuffer->setValue(m_aoiViewRect.ul().x, y, m_overlayValue);
         m_outBuffer->setValue(m_aoiViewRect.lr().x, y, m_overlayValue);
      }
      for (int x=m_aoiViewRect.ul().x; x<=m_aoiViewRect.lr().x; x++)
      {
         m_outBuffer->setValue(x, m_aoiViewRect.ul().y, m_overlayValue);
         m_outBuffer->setValue(x, m_aoiViewRect.lr().y, m_overlayValue);
      }
   }
}

bool ossimViewshedUtil::writeHorizonProfile()
{
   // Store the max elevation angles for horizon profiling:
   double az_deg, arctan;
   for (ossim_uint32 sector=0; sector<8; ++sector)
   {
      if (m_radials[sector] == 0)
         continue;

      for (ossim_uint32 radial = 0; radial <= m_halfWindow; ++radial)
      {
         arctan = ossim::atand(m_radials[sector][radial].azimuth);
         switch (sector)
         {
         case 0: // 0 - 45
            az_deg = arctan;
            break;
         case 1: // 45 - 90
            az_deg = 90 - arctan;
            break;
         case 2: // 90 - 135
            az_deg = 90 + arctan;
            break;
         case 3: // 135 - 180
            az_deg = 180 - arctan;
            break;
         case 4: // 180 - 225
            az_deg = 180 + arctan;
            break;
         case 5: // 225 - 270
            az_deg = 270 - arctan;
            break;
         case 6: // 270 - 315
            az_deg = 270 + arctan;
            break;
         case 7: // 315 - 360
            az_deg = 360 - arctan;
            break;
         default:
            break;
         }

         m_horizonMap.insert(pair<double, double>(az_deg, m_radials[sector][radial].elevation));
      }
   }

   // Open output file and write the map:
   ofstream fstr (m_horizonFile.chars());
   if (!fstr.is_open())
      return false;
   map<double, double>::iterator iter = m_horizonMap.begin();
   while (iter != m_horizonMap.end())
   {
      fstr << iter->first << ", " << iter->second << endl;
      ++iter;
   }

   fstr.close();
   return true;
}

void SectorProcessorJob::start()
{
   // Loop over all the sector's radials and walk over each one.
   for (ossim_uint32 r=0; r<=m_numRadials; ++r)
   {
      RadialProcessor::doRadial(m_vsUtil, m_sector, r);
   }
}

void RadialProcessorJob::start()
{
   RadialProcessor::doRadial(m_vsUtil, m_sector, m_radial);
}

OpenThreads::ReadWriteMutex RadialProcessor::m_bufMutex;

void RadialProcessor::doRadial(ossimViewshedUtil* vsUtil,
                               ossim_uint32 sector_idx,
                               ossim_uint32 radial_idx)
{
   double u, v;
   ossimDpt pt_i, vpt_i;
   ossimGpt gpt_i;
   double elev_i, elev;
   double r2_max = vsUtil->m_halfWindow*vsUtil->m_halfWindow;

   // Establish shorthand access to radial:
   ossimViewshedUtil::Radial& radial = vsUtil->m_radials[sector_idx][radial_idx];

   // Walk along the radial using the appropriate coordinate abscissa for that sector and
   // compute ordinate using the radials azimuth:
   for (u=1.0; u <= (double) vsUtil->m_halfWindow; u += 1.0)
   {
      // Compute ordinate from abscissa and slope of this radial:
      v = radial.azimuth*(u);
      switch (sector_idx)
      {
      case 0: // N-NE, (u, v) = (-y, x)
         pt_i.y = -u;
         pt_i.x = v;
         break;
      case 1: // NE-E, (u, v) = (x, -y)
         pt_i.x = u;
         pt_i.y = -v;
         break;
      case 2: // E-SE, (u, v) = (x, y)
         pt_i.x = u;
         pt_i.y = v;
         break;
      case 3: // SE-S, (u, v) = (y, x)
         pt_i.y = u;
         pt_i.x = v;
         break;
      case 4: // S-SW, (u, v) = (y, -x)
         pt_i.y = u;
         pt_i.x = -v;
         break;
      case 5: // SW-W, (u, v) = (-x, y)
         pt_i.x = -u;
         pt_i.y = v;
         break;
      case 6: // W-NW, (u, v) = (-x, -y)
         pt_i.x = -u;
         pt_i.y = -v;
         break;
      case 7: // NW-N, (u, v) = (-y, -x)
         pt_i.y = -u;
         pt_i.x = -v;
         break;
      default:
         break;
      }

      // Shift to actual view coordinates:
      vpt_i = pt_i + vsUtil->m_observerVpt;
      ossimIpt ipt (vpt_i);

      // Check if alread accounted for at this location:
      //if (!vsUtil->m_outBuffer->isNull(vpt_i))
      //   continue;

      // Check if we are exiting the AOI (no more processing required for this radial):
      bool pointInsideAoi = vsUtil->m_aoiViewRect.pointWithin(ipt);
      if (radial.insideAoi && !pointInsideAoi)
         break;

      // Alternatively, check if we were OUTSIDE and now moving INSIDE:
      if (!radial.insideAoi && pointInsideAoi)
         radial.insideAoi = true;

      // Check if we passed beyong the visibilty radius, and exit loop if so:
      if (vsUtil->m_displayAsRadar && ((u*u + v*v) >= r2_max))
      {
         //OpenThreads::ScopedWriteLock lock (m_bufMutex);
         vsUtil->m_outBuffer->setValue(ipt.x, ipt.y, vsUtil->m_overlayValue);
         break;
      }

      // Fetch the pixel value as the elevation value and compute elevation angle from
      // the observer pt as dz/dx
      vsUtil->m_geom->localToWorld(vpt_i, gpt_i);
      if (vsUtil->m_simulation && ossim::isnan(gpt_i.hgt))
         gpt_i.hgt = vsUtil->m_observerGpt.hgt-vsUtil->m_obsHgtAbvTer; // ground level

      if (!gpt_i.hasNans())
      {
         // Compare elev angle to max angle latched so far along this radial:
         elev_i = (gpt_i.hgt - vsUtil->m_observerGpt.hgt) / u;
         elev = radial.elevation;
         if (elev_i > elev)
         {
            // point is visible, latch this line-of-sight as the new max elevation angle for this
            // radial, and mark the output pixel as visible:
            radial.elevation = elev_i;
            //OpenThreads::ScopedWriteLock lock (m_bufMutex);
            vsUtil->m_outBuffer->setValue(ipt.x, ipt.y, vsUtil->m_visibleValue);
         }
         else
         {
            //OpenThreads::ScopedWriteLock lock (m_bufMutex);
            vsUtil->m_outBuffer->setValue(ipt.x, ipt.y, vsUtil->m_hiddenValue);
         }
      }
   } // end loop over radial's abscissas
}

void ossimViewshedUtil::test()
{
   m_aoiGroundRect = ossimGrect(1.0, 0.0, 0.0, 1.0);

   m_observerGpt = ossimGpt(1.5, 0.5);
   m_startFov = 180;
   m_stopFov = 270;
   cout<<"Before: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<endl;
   optimizeFOV();
   cout<<"After: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<"\n"<<endl;

   m_observerGpt = ossimGpt(1.5, 0.5);
   m_startFov = 335;
   m_stopFov = 180;
   cout<<"Before: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<endl;
   optimizeFOV();
   cout<<"After: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<"\n"<<endl;

   m_observerGpt = ossimGpt(1.5, 0.5);
   m_startFov = 270;
   m_stopFov = 0;
   cout<<"Before: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<endl;
   optimizeFOV();
   cout<<"After: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<"\n"<<endl;

   m_observerGpt = ossimGpt(-0.5, 0.5);
   m_startFov = 270;
   m_stopFov = 10;
   cout<<"Before: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<endl;
   optimizeFOV();
   cout<<"After: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<"\n"<<endl;

   m_observerGpt = ossimGpt(-0.5, 0.5);
   m_startFov = 350;
   m_stopFov = 90;
   cout<<"Before: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<endl;
   optimizeFOV();
   cout<<"After: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<"\n"<<endl;

   m_observerGpt = ossimGpt(-0.5, 0.5);
   m_startFov = 10;
   m_stopFov = 20;
   cout<<"Before: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<endl;
   optimizeFOV();
   cout<<"After: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<"\n"<<endl;

   m_observerGpt = ossimGpt(-0.5, 0.5);
   m_startFov = 270;
   m_stopFov = 90;
   cout<<"Before: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<endl;
   optimizeFOV();
   cout<<"After: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<"\n"<<endl;

   m_observerGpt = ossimGpt(-0.5, 0.5);
   m_startFov = 90;
   m_stopFov = 270;
   cout<<"Before: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<endl;
   optimizeFOV();
   cout<<"After: m_startFov="<<m_startFov<<"  m_stopFov="<<m_stopFov<<"\n"<<endl;
}

//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

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

const char* ossimViewshedUtil::DESCRIPTION =
      "Computes bitmap image representing the viewshed from specified location using only "
      "DEM information.";


ossimViewshedUtil::ossimViewshedUtil()
:   m_obsHgtAbvTer (1.5),
    m_visRadius (0.0),
    m_radials (0),
    m_initialized (false),
    m_halfWindow (0),
    m_outBuffer (NULL),
    m_gsd (0),
    m_visibleValue (0),
    m_hiddenValue (128),
    m_observerValue (255),
    m_reticleSize(2),
    m_simulation (false),
    m_numThreads(0),
    m_outputSummary(false),
    m_startFov(0),
    m_stopFov(0),
    m_threadBySector(false),
    d_accumT(0)
{
   m_observerGpt.makeNan();
}

ossimViewshedUtil::~ossimViewshedUtil()
{
   for (int i=0; i<4; ++i)
      delete [] m_radials[i];
   delete [] m_radials;
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
         "--fov <start> <end>",
         "Optional arguments specifying the field-of"
         "-view boundary azimuths (in degrees). By default, a 360 deg FOV is"
         " computed. The arc is taken clockwise from start to end, so for a"
         " FOV of 225 deg from W, through N to SE, start=270 and end=135");
   au->addCommandLineOption(
         "--hgt-of-eye <meters>",
         "Specifies the observers height-of-eye above the "
         "terrain in meters. Defaults to 1.5 m.");
   au->addCommandLineOption(
         "--horizon <filename>",
         "Experimental. Outputs the max elevation angles "
         "for all azimuths to <filename>, for horizon profiling.");
   au->addCommandLineOption(
         "--radius <meters>",
         "Specifies max visibility in meters. Required "
         "unless --size is specified. This option constrains output to a circle, "
         "similar to a radar display");
   au->addCommandLineOption(
         "--reticle <int>",
         "Specifies the size of the reticle at the observer"
         "location in pixels from the center (i.e., the radius of the reticle). "
         "Defaults to 2. A value of 0 hides the reticle. See --values option for "
         "setting reticle color.");
   au->addCommandLineOption(
         "--simulation",
         "For engineering/debug purposes ");
   au->addCommandLineOption(
         "--size <int>",
         "Instead of a visibility radius, directly specifies "
         "the dimensions of the output product in pixels (output is "
         "square). Required unless --radius is specified.");
   au->addCommandLineOption(
         "--tbs",
         "\"Thread By Sector\". For engineering/debug purposes ");
   au->addCommandLineOption(
         "--threads <n>",
         "Number of threads. Defaults to use all available cores. "
         "For engineering/debug purposes ");
   au->addCommandLineOption(
         "--values <int int int>",
         "Specifies the pixel values (0-255) for the visible,"
         " hidden and reticle pixels, respectively. Defaults to visible=null (0), "
         "hidden=128, and observer position reticle is highlighted with 255.");

   // Base class has its own:
   ossimChipProcUtil::setUsage(ap);

   ostringstream description;
   description << DESCRIPTION << "\n\nExamples:\n\n"
         "    "<<appName<<" --radius 50  28.0 -80.5 output-hlz.tif\n"
         "    "<<appName<<" --size 1024  28.0 -80.5 output-hlz.tif\n\n"
         "An alternate command line provides switch for observer lat and lon:\n\n"
         "    "<<appName<<" --rlz 25 --observer 28.0 -80.5  output-hlz.tif \n";
   au->setDescription(description.str());
}

bool ossimViewshedUtil::initialize(ossimArgumentParser& ap)
{
   ostringstream xmsg ("ossimViewshedUtil::initialize(ossimArgumentParser) -- ");

   // Base class first:
   if (!ossimUtility::initialize(ap))
      return false;

   std::string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);
   std::string ts2;
   ossimArgumentParser::ossimParameter sp2(ts2);
   std::string ts3;
   ossimArgumentParser::ossimParameter sp3(ts3);


   if (ap.read("--dem", sp1) || ap.read("--dem-file", sp1))
      m_demFile = ts1;

   if ( ap.read("--fov", sp1, sp2) )
   {
      m_startFov = ossimString(ts1).toDouble();
      m_stopFov = ossimString(ts2).toDouble();
      if (m_startFov < 0)
         m_startFov += 360.0;
   }

   if ( ap.read("--gsd", sp1) )
      m_gsd = ossimString(ts1).toDouble();

   if ( ap.read("--hgt-of-eye", sp1) || ap.read("--height-of-eye", sp1) )
      m_obsHgtAbvTer = ossimString(ts1).toDouble();

   if ( ap.read("--horizon", sp1) || ap.read("--horizon-file", sp1))
      m_horizonFile = ossimString(ts1);

   if ( ap.read("--lut", sp1) || ap.read("--lut-file", sp1))
      m_lutFile = ts1;

   if ( ap.read("--observer", sp1, sp2) )
   {
      m_observerGpt.lat = ossimString(ts1).toDouble();
      m_observerGpt.lon = ossimString(ts2).toDouble();
      m_observerGpt.hgt = 0.0;
   }

   if ( ap.read("--radius", sp1) )
      m_visRadius = ossimString(ts1).toDouble();

   if ( ap.read("--reticle", sp1) )
      m_reticleSize = ossimString(ts1).toInt32();

   if ( ap.read("--tbs") )
      m_threadBySector = true;

   if ( ap.read("--simulation") )
      m_simulation = true;

   if ( ap.read("--summary") )
      m_outputSummary = true;

   if ( ap.read("--size", sp1) )
      m_halfWindow = ossimString(ts1).toUInt32() / 2;

   if ( ap.read("--threads", sp1) )
      m_numThreads = ossimString(ts1).toUInt32();

   if ( ap.read("--values", sp1, sp2, sp3) )
   {
      m_visibleValue = ossimString(ts1).toUInt8();
      m_hiddenValue = ossimString(ts2).toUInt8();
      m_observerValue = ossimString(ts3).toUInt8();
   }

   // There should only be the required command line args left:
   if ( (m_observerGpt.hasNans() && (ap.argc() != 4)) ||
        (!m_observerGpt.hasNans() && (ap.argc() != 2)) )
   {
      setUsage(ap);
      return false;
   }

   // Verify minimum required args were specified:
   if (m_demFile.empty() && (m_visRadius == 0) && (m_halfWindow == 0))
   {
      xmsg<<"Command line is underspecified."<<ends;
      throw ossimException(xmsg.str());
   }

   // Parse the required command line params:
   int ap_idx = 1;
   if (m_observerGpt.hasNans())
   {
      m_observerGpt.lat =  ossimString(ap[1]).toDouble();
      m_observerGpt.lon =  ossimString(ap[2]).toDouble();
      m_observerGpt.hgt =  0;
      ap_idx = 3;
   }
   m_filename = ap[ap_idx];

   initializeChain();
   return true;
}

bool ossimViewshedUtil::initialize(const ossimKeywordlist& kwl)
{
   ostringstream xmsg ("ossimViewshedUtil::initialize(kwl) -- ");

   // Base class first:
   if (!ossimChipProcUtil::initialize(kwl))
      return false;

   ossimString value;

   m_demFile = kwl.find("dem_file");
   if (m_demFile.empty())
      m_demFile = kwl.find(ossimKeywordNames::ELEVATION_CELL_KW);


   value = kwl.find("fov");
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

   value = kwl.find("gsd");
   if (value.empty())
      value = kwl.find(ossimKeywordNames::METERS_PER_PIXEL_KW);
   if (!value.empty())
      m_gsd = value.toDouble();

   value = kwl.find("height_of_eye");
   if (!value.empty())
      m_obsHgtAbvTer = value.toDouble();

   m_horizonFile = kwl.find("horizon_file");

   m_lutFile = kwl.find("lut_file");

   value = kwl.find("observer");
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

   value = kwl.find("radius");
   if (!value.empty())
      m_visRadius = value.toDouble();

   value = kwl.find("reticle");
   if (!value.empty())
      m_reticleSize = value.toInt32();

   kwl.getBoolKeywordValue(m_threadBySector, "thread_by_sector");
   kwl.getBoolKeywordValue(m_simulation, "simulation");
   kwl.getBoolKeywordValue(m_outputSummary, "summary");

   value = kwl.find("size");
   if (!value.empty())
      m_halfWindow = value.toInt32();

   value = kwl.find(ossimKeywordNames::THREADS_KW);
   if (!value.empty())
      m_numThreads = value.toInt32();

   value = kwl.find("values");
   if (!value.empty())
   {
      vector <ossimString> coordstr;
      value.split(coordstr, ossimString(" ,"), false);
      if (coordstr.size() == 3)
      {
         m_visibleValue = coordstr[0].toUInt8();
         m_hiddenValue = coordstr[1].toUInt8();
         m_observerValue = coordstr[2].toUInt8();
      }
   }

   m_filename = kwl.find(ossimKeywordNames::OUTPUT_FILE_KW);
   if (value.empty())
   {
      xmsg <<"No output file name provided."<<ends;
      throw ossimException(xmsg.str());
   }

  // Verify minimum required args were specified:
   if (m_demFile.empty() && (m_visRadius == 0) && (m_halfWindow == 0))
   {
      xmsg << "Keywordlist is underspecified." << ends;
      throw ossimException(xmsg.str());
   }

   initializeChain();
   return true;
}

void ossimViewshedUtil::clear()
{
   m_observerGpt.makeNan();
   m_demFile.clear();
   m_lutFile.clear();
   m_visRadius = 0;
   m_outBuffer = 0;
   m_filename.clear();
   m_horizonMap.clear();
   m_jobMtQueue = 0;
   m_geometry = 0;
}

void ossimViewshedUtil::initializeChain()
{
   ostringstream xmsg ("ossimViewshedUtil::initializeChain() -- ");

   // TODO: Need to implement throws
   // TODO: Accept srcLayers as imagery to blend with viewshed
   // TODO: Define additional --dem input files? NO -- ALWAYS DATA BASE

   if (m_observerGpt.hasNans())
   {
      xmsg<<"Observer ground position has not been set."<<ends;
      throw ossimException(xmsg.str());
   }

   ossimElevManager* elevMgr = ossimElevManager::instance();

   // If DEMs provided as files on command line, reset the elev manager to use only these:
   if (!m_demFile.empty())
   {
      elevMgr->clear();
      ossimRefPtr<ossimImageElevationDatabase> ied = new ossimImageElevationDatabase;
      if(!ied->open(m_demFile))
      {
         xmsg << "Cannot open DEM file at <"<<m_demFile<<">\n" << ends;
         throw ossimException(xmsg.str());
      }

      if (m_simulation)
         ied->setGeoid(new ossimIdentityGeoid);

      elevMgr->addDatabase(ied.get());

      // Possibly the image size has not been specified, in which case we use the same dimensions
      // as the input dem:
      if (((m_halfWindow == 0) && (m_visRadius == 0)) || (m_gsd == 0))
      {

         ossimRefPtr<ossimImageHandler> dem = ossimImageHandlerRegistry::instance()->open(m_demFile);
         if (!dem.valid())
         {
            xmsg << "Cannot open DEM file at <"<<m_demFile<<">" << ends;
            throw ossimException(xmsg.str());
         }
         ossimRefPtr<ossimImageGeometry> geom = dem->getImageGeometry();
         if (!geom.valid())
         {
            xmsg<<"Could not establish geometry of DEM file at <"<<m_demFile<<">" << ends;
            throw ossimException(xmsg.str());
         }

         // Hack workaround for ossimElevManager::getMeanSpacingMeters() returning 0 when DEM file
         // specified:
         if (m_gsd == 0)
         {
            ossimDpt gsd = geom->getMetersPerPixel();
            m_gsd = (gsd.x + gsd.y)/2.0;
         }
         if ((m_halfWindow == 0) && (m_visRadius == 0))
         {
            ossimIpt size = geom->getImageSize();
            m_halfWindow = (size.x + size.y) / 4;
         }
      }

      // When DEM file specified, need to turn off all defaulting to ellipsoid/geoid to make sure
      // only the DEM file data is processed:
      elevMgr->setDefaultHeightAboveEllipsoid(ossim::nan());
      elevMgr->setUseGeoidIfNullFlag(false);
   }

   if (m_simulation)
      elevMgr->setEnableFlag(false);

   // Initialize the height of eye component of observer position:
   m_observerGpt.hgt = ossimElevManager::instance()->getHeightAboveEllipsoid(m_observerGpt);
   m_observerGpt.hgt += m_obsHgtAbvTer;

   // Determine if default GSD needs to be computed.
   if (m_gsd == 0)
   {
      // This is incorrectly returning 0 when DEM is provided on command line:
      m_gsd = ossimElevManager::instance()->getMeanSpacingMeters();
      if (ossim::isnan(m_gsd))
         m_gsd = 0;
   }

   // Compute the bounding rect in pixel space given the visibility range and the GSD:
   if ((m_gsd == 0) || ((m_visRadius == 0) && (m_halfWindow == 0)))
   {
      xmsg<<"GSD, visibility radius or image size have not been set." << ends;
      throw ossimException(xmsg.str());
   }
   if (m_halfWindow == 0)
      m_halfWindow = ossim::round<ossim_int32, double>(m_visRadius/m_gsd);

   m_viewRect.set_ulx(-m_halfWindow);
   m_viewRect.set_uly(-m_halfWindow);
   m_viewRect.set_lrx(m_halfWindow);
   m_viewRect.set_lry(m_halfWindow);
   ossimIpt image_size (m_viewRect.width(), m_viewRect.height());

   // Establish the image geometry's map projection:
   ossimRefPtr<ossimEquDistCylProjection> mapProj = new ossimEquDistCylProjection();
   mapProj->setOrigin(m_observerGpt);
   mapProj->setMetersPerPixel(ossimDpt(m_gsd, m_gsd));
   ossimDpt degPerPixel (mapProj->getDecimalDegreesPerPixel());
   mapProj->setElevationLookupFlag(true);
   ossimGpt ulTiePt (m_observerGpt);
   ulTiePt.lat += degPerPixel.lat * m_halfWindow;
   ulTiePt.lon -= degPerPixel.lon * m_halfWindow;
   mapProj->setUlTiePoints(ulTiePt);

   // Need a transform so that we can use the observer point as the output image origin (0,0):
   ossimRefPtr<ossim2dTo2dTransform> transform =  new ossim2dTo2dShiftTransform(m_viewRect.lr());
   m_geometry = new ossimImageGeometry(transform.get(), mapProj.get());
   m_geometry->setImageSize(image_size);

   // Allocate the output image buffer:
   m_outBuffer = ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1,
                                                           m_viewRect.width(), m_viewRect.height());
   if(!m_outBuffer.valid())
   {
      xmsg<<"Output buffer allocation failed." << ends;
      throw ossimException(xmsg.str());
   }

   // Initialize the image with all points hidden:
   m_outBuffer->initialize();
   m_outBuffer->setImageRectangle(m_viewRect);
   m_outBuffer->fill(m_visibleValue);

#if 0
   //### TODO: REMOVE DEBUG BLOCK
   {
      ossimDpt viewPt;
      m_geometry->worldToLocal(m_observerGpt, viewPt);
      cout<<"ossimViewshedUtil::initialize() should get (0,0)... viewPt="<<viewPt<<endl;
      ossimGpt testPt(m_observerGpt);
      testPt.lat -= 100*degPerPixel.y;
      testPt.lon += 100*degPerPixel.x;
      m_geometry->worldToLocal(testPt, viewPt);
      cout<<"ossimViewshedUtil::initialize() should get ~(100,100)... viewPt="<<viewPt<<endl;
   }
#endif

   // Initialize the radials:
   initRadials();

   if (m_outputSummary)
      dumpProductSummary();

   m_initialized = true;
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
   std::memset(sectorInFov, false, 8);
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

bool ossimViewshedUtil::execute()
{
   if (!m_initialized)
      return false;

   d_accumT = 0;
   bool success =  false;

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
            return false;
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
            return false;

      } // end loop over sectors
   }

   cout << "Finished processing radials."<<endl;
   paintReticle();

   cout << "Writing output file..." <<endl;
   success = writeFile();

   if (!m_horizonFile.empty())
   {
      cout << "Writing horizon profile output file..." <<endl;
      success = writeHorizonProfile();
   }

   cout << "Returning..."<<endl;
   return success;
}

void ossimViewshedUtil::paintReticle()
{
   // Highlight the observer position with X reticle:
   if (m_reticleSize > 0)
   {
      m_outBuffer->setValue(0, 0, m_observerValue);
      for (int i=-m_reticleSize; i<=m_reticleSize; ++i)
      {
         m_outBuffer->setValue(i, 0, m_observerValue);
         m_outBuffer->setValue(0,  i, m_observerValue);
      }
   }

   if (m_visRadius == 0)
   {
      // Also outline the square area of interest:
      ossim_int32 hw = (ossim_int32) m_halfWindow;
      for (ossim_int32 u=-hw; u<=hw; ++u)
      {
         m_outBuffer->setValue(  u,-hw, m_observerValue);
         m_outBuffer->setValue(  u, hw, m_observerValue);
         m_outBuffer->setValue(-hw,  u, m_observerValue);
         m_outBuffer->setValue( hw,  u, m_observerValue);
      }
   }
}

bool ossimViewshedUtil::writeFile()
{
   ossimIrect rect (0, 0, m_viewRect.width()-1, m_viewRect.height()-1);
   m_outBuffer->setImageRectangle(rect);

   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(m_outBuffer);
   memSource->setImageGeometry(m_geometry.get());
   ossimImageSource* last_source = memSource.get();

   // See if an LUT is requested:
   ossimRefPtr<ossimIndexToRgbLutFilter> lutSource = 0;
   if (!m_lutFile.empty())
   {
      ossimKeywordlist lut_kwl;
      lut_kwl.addFile(m_lutFile);
      lutSource = new ossimIndexToRgbLutFilter;
      if (!lutSource->loadState(lut_kwl))
      {
         ossimNotify(ossimNotifyLevel_WARN) << "ossimViewshedUtil::writeFile() ERROR: The LUT "
               "file <"<<m_lutFile<<"> could not be read. Ignoring remap request.\n"<< std::endl;
         lutSource = 0;
      }
      else
      {
         lutSource->connectMyInputTo(last_source);
         lutSource->initialize();
         last_source = lutSource.get();
      }
   }

   // Set up the writer:
   ossimRefPtr<ossimImageFileWriter> writer = 0;
   if (m_filename.ext().contains("tif"))
   {
      ossimTiffWriter* tif_writer = new ossimTiffWriter();
      tif_writer->setGeotiffFlag(true);
      tif_writer->setFilename(m_filename);
      writer = tif_writer;
   }
   else
   {
      writer = ossimImageWriterFactoryRegistry::instance()->createWriter(m_filename);
   }
   bool success = false;
   if (writer.valid())
   {
      writer->connectMyInputTo(0, last_source);
      success = writer->execute();
   }

   return success;
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
   std::map<double, double>::iterator iter = m_horizonMap.begin();
   while (iter != m_horizonMap.end())
   {
      fstr << iter->first << ", " << iter->second << endl;
      ++iter;
   }

   fstr.close();
   return true;
}

void ossimViewshedUtil::dumpProductSummary() const
{
   ossimIpt isize (m_geometry->getImageSize());
   cout  << "\nSummary of Viewshed product image:"
         << "\n   Output file name: " << m_filename
         << "\n   Image size: " << isize
         << "\n   product GSD: " << m_gsd << " m"
         << "\n   View radius: " << (int) (m_gsd * isize.x/2.0) << " m"
         << "\n   Scalar type: " << m_outBuffer->getScalarTypeAsString()
         << endl;
}

void SectorProcessorJob::start()
{
   // Loop over all the sector's radials and walk over each one.
   for (ossim_uint32 r=0; r<=m_numRadials; ++r)
      RadialProcessor::doRadial(m_vsUtil, m_sector, r);
}


void RadialProcessorJob::start()
{
   RadialProcessor::doRadial(m_vsUtil, m_sector, m_radial);
}

OpenThreads::ReadWriteMutex RadialProcessor::m_bufMutex;
OpenThreads::ReadWriteMutex RadialProcessor::m_radMutex;

void RadialProcessor::doRadial(ossimViewshedUtil* vsUtil,
                               ossim_uint32 sector,
                               ossim_uint32 radial)
{
   double v;
   ossimDpt pt_i;
   double elev_i, elev;
   double r2_max = vsUtil->m_halfWindow*vsUtil->m_halfWindow;

   // Walk along the radial using the appropriate coordinate abscissa for that sector and
   // compute ordinate using the radials azimuth:
   for (double u=1.0; u <= (double) vsUtil->m_halfWindow; u += 1.0)
   {
      // Compute ordinate from abscissa and slope of this radial:
      v = vsUtil->m_radials[sector][radial].azimuth*(u);
      switch (sector)
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

      ossimIpt ipt (ossim::round<ossim_int32,double>(pt_i.x),
                    ossim::round<ossim_int32,double>(pt_i.y));

      // Check if we passed beyong the visibilty radius, and exit loop if so:
      if ((vsUtil->m_visRadius > 0) && ((u*u + v*v) >= r2_max))
      {
         OpenThreads::ScopedWriteLock lock (m_bufMutex);
         vsUtil->m_outBuffer->setValue(ipt.x, ipt.y, vsUtil->m_observerValue);
         break;
      }

      // Fetch the pixel value as the elevation value and compute elevation angle from
      // the observer pt as dz/dx
      ossimGpt gpt_i;
      vsUtil->m_geometry->localToWorld(pt_i, gpt_i);

      if (vsUtil->m_simulation && ossim::isnan(gpt_i.hgt))
         gpt_i.hgt = vsUtil->m_observerGpt.hgt-vsUtil->m_obsHgtAbvTer; // ground level

      else if (!gpt_i.hasNans())
      {
         // Compare elev angle to max angle latched so far along this radial:
         elev_i = (gpt_i.hgt - vsUtil->m_observerGpt.hgt) / u;
         elev = vsUtil->m_radials[sector][radial].elevation;
         if (elev_i > elev)
         {
            // point is visible, latch this line-of-sight as the new max elevation angle for this
            // radial, and mark the output pixel as visible:
            //   m_outBuffer->setValue(ossim::round<ossim_int32,double>(pt_i.x),
            //                           ossim::round<ossim_int32,double>(pt_i.y), m_visibleValue);
            //OpenThreads::ScopedWriteLock lock (m_radMutex);
            vsUtil->m_radials[sector][radial].elevation = elev_i;
         }
         else
         {
            OpenThreads::ScopedWriteLock lock (m_bufMutex);
            vsUtil->m_outBuffer->setValue(ipt.x, ipt.y, vsUtil->m_hiddenValue);
         }
      }
   } // end loop over radial's abscissas
}


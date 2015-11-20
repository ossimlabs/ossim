//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimRtti.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimException.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/elevation/ossimImageElevationDatabase.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimImageMosaic.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimSlopeFilter.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimMemoryImageSource.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/point_cloud/ossimPointCloudHandlerRegistry.h>
#include <ossim/util/ossimHlzUtil.h>
#include <fstream>

static const char* MASK_EXCLUDE_KW = "excludes";
static const char* MASK_INCLUDE_KW = "includes";
static const char* SLOPE_OUTPUT_FILE_KW = "slope_output_file";
static const char* POINT_CLOUD_FILE_KW = "point_cloud_file";
static const char* RETICLE_SIZE_KW = "reticle_size";
static const char* LZ_MIN_RADIUS_KW = "lz_min_radius";
static const char* ROUGHNESS_THRESHOLD_KW = "roughness_threshold";
static const char* SLOPE_THRESHOLD_KW = "slope_threshold";
static const char* TARGET_GEOLOCATION_KW = "target_geolocation";

const char* ossimHlzUtil::DESCRIPTION =
      "Computes bitmap of helicopter landing zones given ROI and DEM.";

ossimHlzUtil::ossimHlzUtil()
: m_slopeThreshold(7.0),
  m_roughnessThreshold(0.5),
  m_hlzMinRadius(25.0),
  m_outBuffer(NULL),
  m_gsd(0),
  m_reticleSize(10),
  m_outputSummary(false),
  m_jobCount(0),
  m_badLzValue(0),
  m_marginalLzValue(1),
  m_goodLzValue(2),
  m_reticleValue(3),
  m_useLsFitMethod(false),
  m_isInitialized(false),
  m_numThreads(1),
  d_accumT(0)
{
   m_destinationGpt.makeNan();
}

ossimHlzUtil::~ossimHlzUtil()
{
}

void ossimHlzUtil::setUsage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimChipProcUtil::setUsage(ap);

   // Set the general usage:
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " [options] <output-image>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->addCommandLineOption(
         "--excludes <file1>[, <file2>...]",
         "List of raster image(s) representing mask files that defines regions to be excluded from."
         "HLZ solutions. Any non-zero pixel is excluded Multiple filenames must be comma-separated.");
   au->addCommandLineOption(
         "--includes <file1>[, <file2>...]",
         "List of raster image(s) representing mask files that defines regions where the HLZs ."
         "identified must overlap. Any non-zero pixel represents an inclusion zone. Multiple "
         "filenames must be comma-separated.");
   au->addCommandLineOption(
         "--ls-fit",
         "Slope is computed via an LS fit to a plane instead of the default slope computation using "
         "differences to compute normal vector.");
   au->addCommandLineOption(
         "--output-slope <filename.tif>",
         "Generates a slope byproduct image (floating point degrees) to the specified filename. "
         "Only valid if normal-vector method used (i.e., --ls-fit option NOT specified)");
   au->addCommandLineOption(
         "--pc | --point-cloud <file1>[, <file2>...]",
         "Specifies ancillary point-cloud data file(s) for level-2 search for obstructions. "
         "Must be comma-separated file names.");
   au->addCommandLineOption(
         "--reticle <int>",
         "Specifies the size of the reticle at the destination point location in pixels from the "
         "center (i.e., the radius of the reticle). Defaults to 10. A value of 0 hides the reticle. "
         "See --values option for setting reticle color.");
   au->addCommandLineOption(
         "--rlz <meters>",
         "Specifies minimum radius of landing zone. Defaults to 25 m. ");
   au->addCommandLineOption(
         "--roughness <meters>",
         "Specifies the terrain roughness threshold (meters). This is the maximum deviation from a "
         "flat plane permitted. Defaults to 0.5 m. Valid only with --ls-fit specified.");
   au->addCommandLineOption("--simulation", "For engineering/debug purposes ");
   au->addCommandLineOption(
         "--slope <degrees>",
         "Threshold for acceptable landing zone terrain slope. Defaults to 7 deg.");
   au->addCommandLineOption(
         "--summary",
         "Causes a product summary to be output to the console.");
   au->addCommandLineOption(
         "--threads <n>",
         "Number of threads. Defaults to use single core. For engineering/debug purposes.");
   au->addCommandLineOption(
         "--values <bad marg good ret>",
         "Specifies the pixel values (0-255) for the output product corresponding to bad, marginal, "
         "and good landing zones, respectively, with the fourth value representing the reticle "
         "value. Defaults to bad=0 (null), marg=1, , good=2, and reticle is highlighted with 3.");

}

void ossimHlzUtil::initialize(ossimArgumentParser& ap)
{
   ossimChipProcUtil::initialize(ap);

   std::string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);
   std::string ts2;
   ossimArgumentParser::ossimParameter sp2(ts2);
   std::string ts3;
   ossimArgumentParser::ossimParameter sp3(ts3);
   std::string ts4;
   ossimArgumentParser::ossimParameter sp4(ts4);
   ossimString  key ;

   if (ap.read("--ls-fit"))
   {
      // Command line mode only
      m_useLsFitMethod = true;
   }

   vector<ossimString> maskFnames;
   ap.read("--excludes", maskFnames);
   for(ossim_uint32 idx=0; idx<maskFnames.size(); ++idx)
   {
      key = MASK_EXCLUDE_KW;
      key += ossimString::toString(idx++);
      m_kwl.addPair(key.string(), maskFnames[idx] );
   }

   maskFnames.clear();
   ap.read("--includes", maskFnames);
   for(ossim_uint32 idx=0; idx<maskFnames.size(); ++idx)
   {
      key = MASK_INCLUDE_KW;
      key += ossimString::toString(idx++);
      m_kwl.addPair(key.string(), maskFnames[idx] );
   }

   if ( ap.read("--output-slope", sp1))
      m_kwl.add(SLOPE_OUTPUT_FILE_KW, ts1.c_str());

   vector<ossimString> pcFnames;
   ap.read("--point-clouds", pcFnames);
   for(ossim_uint32 idx=0; idx<pcFnames.size(); ++idx)
   {
      key = POINT_CLOUD_FILE_KW;
      key += ossimString::toString(idx++);
      m_kwl.addPair(key.string(), pcFnames[idx] );
   }

   if (ap.read("--reticle", sp1))
      m_kwl.add(RETICLE_SIZE_KW, ts1.c_str());

   if (ap.read("--rlz", sp1))
      m_kwl.add(LZ_MIN_RADIUS_KW, ts1.c_str());

   if (ap.read("--roughness", sp1))
      m_kwl.add(ROUGHNESS_THRESHOLD_KW, ts1.c_str());

   if (ap.read("--slope", sp1))
      m_kwl.add(SLOPE_THRESHOLD_KW, ts1.c_str());

   if (ap.read("--target", sp1, sp2))
   {
      ossimString geolocstr = ts1 + " " + ts2;
      m_kwl.add(TARGET_GEOLOCATION_KW, geolocstr.chars());
   }

   if (ap.read("--threads", sp1))
   {
      // Command line mode only
      m_numThreads = ossimString(ts1).toUInt32();
   }

   if (ap.read("--values", sp1, sp2, sp3, sp4))
   {
      // Command line mode only
      m_badLzValue = ossimString(ts1).toUInt8();
      m_marginalLzValue = ossimString(ts2).toUInt8();
      m_goodLzValue = ossimString(ts3).toUInt8();
      m_reticleValue = ossimString(ts4).toUInt8();
   }

   processRemainingArgs(ap);
}

void ossimHlzUtil::initialize(const ossimKeywordlist& kwl)
{
   ossimString value;

   m_slopeFile = m_kwl.find(SLOPE_OUTPUT_FILE_KW);
   m_pcFile = m_kwl.find(POINT_CLOUD_FILE_KW);

   value = m_kwl.find(RETICLE_SIZE_KW);
   if (!value.empty())
      m_reticleSize = value.toInt32();

   value = m_kwl.find(LZ_MIN_RADIUS_KW);
   if (!value.empty())
      m_hlzMinRadius = value.toDouble();

   value = m_kwl.find(ROUGHNESS_THRESHOLD_KW);
   if (!value.empty())
      m_roughnessThreshold = value.toDouble();


   value = m_kwl.find(SLOPE_THRESHOLD_KW);
   if (!value.empty())
      m_slopeThreshold = value.toDouble();

   value = m_kwl.find(TARGET_GEOLOCATION_KW);
   if (!value.empty())
   {
      vector<ossimString> latlon = value.split(", ", true);
      if (latlon.size() == 2)
      {
         m_destinationGpt.lat = latlon[0].toDouble();
         m_destinationGpt.lon = latlon[1].toDouble();
      }
   }

   ossimChipProcUtil::initialize(kwl);
}

void ossimHlzUtil::initProcessingChain()
{
   // Establish the ground rect:
   ossimGpt ul, ur, lr, ll;
   m_geom->getBoundingGroundRect(m_gndRect);

   // If PC provided as file on command line, Load it. This uses the output ground rect so needs to
   // be after the initialization of m_geometry:
   if (!m_pcFile.empty() && !loadPcFile())
      throw ossimException("Error loading point cloud file.");

   // If threat-domes spec (or any mask) provided as file on command line, Load it:
   if (!loadMaskFiles())
      throw ossimException("Error loading mask file.");

   // Allocate the output image buffer:
   m_outBuffer = ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, m_viewRect.width(),
                                                           m_viewRect.height());
   if (!m_outBuffer.valid())
      throw ossimException("Error encountered allocating output image buffer.");

   // Initialize the image with all points hidden:
   m_outBuffer->initialize();
   m_outBuffer->setImageRectangle(m_viewRect);
   m_outBuffer->fill(m_badLzValue);

   // Establish connection to DEM posts directly as raster "images" versus using the OSSIM elev
   // manager that performs interpolation of DEM posts for arbitrary locations. These elev images
   // feed into a combiner in order to have a common tap for elev pixels:
   // If a DEM file was not provided as an argument, the elev sources array needs be initialized:
   m_isInitialized = false;
   if (!m_combinedElevSource.valid())
   {
      // HLZ requires access to individual elevation posts for computing statistics. We use the
      // elevation manager to identify cells that provide coverage over the AOI, then we open those
      // cells as images with associated geometries.
      ossimElevManager* elevMgr = ossimElevManager::instance();

      // Query elevation manager for cells providing needed coverage:
      std::vector<std::string> cells;
      elevMgr->getCellsForBounds(m_gndRect, cells);

      // Open a raster image for each elevation source being considered:
      ossimConnectableObject::ConnectableObjectList elevChains;
      std::vector<std::string>::iterator fname_iter = cells.begin();
      while (fname_iter != cells.end())
      {
         ossimRefPtr<ossimImageHandler> dem =
               ossimImageHandlerRegistry::instance()->open(*fname_iter);
         if (!dem.valid())
         {
            ossimNotify(ossimNotifyLevel_WARN)
                    << "ossimHLZUtil::initElevSources() ERR: Cannot open DEM file at <"
                    <<*fname_iter<<">\n"<< std::endl;
            return;
         }

         ossimRefPtr<ossimImageSource> chain;
         createInputChain(dem, chain);
         ossimRefPtr<ossimConnectableObject> connectable = chain.get();
         elevChains.push_back(connectable);
         ++fname_iter;
      }

      if (elevChains.size() == 1)
         m_combinedElevSource = (ossimImageSource*) elevChains[0].get();
      else
         m_combinedElevSource = new ossimImageMosaic(elevChains);
   }

   if (!m_useLsFitMethod)
   {
      // Add the slope computation engine on the elevation source.
      // Set up processing chain with plane to normal filter, equation combiner, and band selector.
      ossimRefPtr<ossimSlopeFilter> slope_filter = new ossimSlopeFilter;
      slope_filter->connectMyInputTo(m_combinedElevSource.get());
      slope_filter->setSlopeType(ossimSlopeFilter::DEGREES);
      m_combinedElevSource = slope_filter.get();
      m_combinedElevSource->initialize();

      m_slopeFile = m_kwl.find(SLOPE_OUTPUT_FILE_KW);
      if (!m_slopeFile.empty())
         writeSlopeImage();
   }
   m_isInitialized = true;

   if (!initHlzFilter())
      return;

   if (m_outputSummary)
      dumpProductSummary();
}

void ossimHlzUtil::setProductGSD(const double& meters_per_pixel)
{
   m_gsd = meters_per_pixel;

   if (m_geom.valid())
   {
      ossimMapProjection* map_proj =
            dynamic_cast<ossimMapProjection*>(m_geom->getProjection());
      if (map_proj)
         map_proj->setMetersPerPixel(ossimDpt(m_gsd, m_gsd));
   }
}

void ossimHlzUtil::createInputChain(ossimRefPtr<ossimImageHandler>& handler,
                                    ossimRefPtr<ossimImageSource>& chain)
{
   ossimRefPtr<ossimImageViewProjectionTransform> ivt = new ossimImageViewProjectionTransform;
   ivt->setImageGeometry(handler->getImageGeometry().get());
   ivt->setViewGeometry(m_geom.get());

   chain = new ossimImageRenderer(handler.get(),ivt.get());
   chain->initialize();
}


bool ossimHlzUtil::loadPcFile()
{
   // When a PC file is provided, certain parameters can be implied versus explicitely provided
   // in the command-line arguments:
   ossimRefPtr<ossimPointCloudHandler> pc_handler =
         ossimPointCloudHandlerRegistry::instance()->open(m_pcFile);
   if (!pc_handler.valid())
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "ossimHLZUtil::loadPointCloudFile ERR: Cannot open point-cloud file <" << m_pcFile << "> "
            << std::endl;
      return false;
   }

   // Verify that PC bounding rect overlaps the output bounding rect:
   ossimGrect pc_bbox;
   pc_handler->getBounds(pc_bbox);
   if (!m_gndRect.intersects(pc_bbox))
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "ossimHLZUtil::loadPointCloudFile ERR: point-cloud file <" << m_pcFile << "> "
            << "does not overlap the output ROI." << std::endl;
      return false;
   }

   m_pcSources.push_back(pc_handler);
   return true;
}

bool ossimHlzUtil::loadMaskFiles()
{
   ossimString key;
   ossimFilename mask_file;

  // Exclusion masks:
   for(ossim_uint32 idx=0; true; ++idx)
   {
      key = MASK_EXCLUDE_KW;
      key += ossimString::toString(idx++);
      mask_file = m_kwl.find(key.chars());
      if (mask_file.empty())
         break;

      MaskSource mask_image (this, m_maskFile, true);
      if (mask_image.valid)
         m_maskSources.push_back(mask_image);
   }

   // Inclusion masks:
   for(ossim_uint32 idx=0; true; ++idx)
   {
      key = MASK_INCLUDE_KW;
      key += ossimString::toString(idx++);
      mask_file = m_kwl.find(key.chars());
      if (mask_file.empty())
         break;

      // First check if the filename specified is an image file:
      MaskSource mask_image (this, m_maskFile, false);
      if (mask_image.valid)
         m_maskSources.push_back(mask_image);
   }

   return true;
}

bool ossimHlzUtil::initHlzFilter()
{
   if ((m_hlzMinRadius == 0) || !m_combinedElevSource.valid())
      return false;

   // Determine number of posts (in one dimension) needed to cover the specified LZ radius:
   m_demGsd = m_geom->getMetersPerPixel();
   m_demFilterSize.x = (int) ceil(m_hlzMinRadius/m_demGsd.x);
   m_demFilterSize.y = (int) ceil(m_hlzMinRadius/m_demGsd.y);
   if ((m_demFilterSize.x < 2) || (m_demFilterSize.y < 2))
   {
      ossimNotify(ossimNotifyLevel_WARN)
                  << "ossimHLZUtil::initHlzFilter() ERR: The DEM provided does not have sufficient"
                  " resolution to determine HLZs. Aborting..." << std::endl;
      return false;
   }

   // clip the requested output geo rect by the rect available from the dem:
   ossimGrect demGndRect;
   m_geom->getBoundingGroundRect(demGndRect);
   if (m_gndRect.hasNans())
      m_gndRect = demGndRect;
   else
      m_gndRect = m_gndRect.clipToRect(demGndRect);

   // Convert (clipped) requested rect to raster coordinate space in the DEM file:
   ossimDpt ulp, lrp;
   m_geom->worldToLocal(m_gndRect.ul(), ulp);
   m_geom->worldToLocal(m_gndRect.lr(), lrp);
   m_demRect.set_ul(ulp);
   m_demRect.set_lr(lrp);

   // To help with multithreading, just load entire AOI of DEM into memory:
   m_demBuffer = m_combinedElevSource->getTile(m_demRect);
   if (!m_demBuffer.valid())
      return false;

   return true;
}

bool ossimHlzUtil::execute()
{
   if (!m_geom.valid())
      return false;

   d_accumT = 0;
   bool success = false;

   // Establish loop limits in input DEM raster coordinate space:
   ossim_int32 min_x = m_demRect.ul().x;
   ossim_int32 min_y = m_demRect.ul().y;
   ossim_int32 max_x = m_demRect.lr().x - m_demFilterSize.x;
   ossim_int32 max_y = m_demRect.lr().y - m_demFilterSize.y;
   ossimIpt chip_origin;
   ossim_uint32 numChips = (max_x-min_x)*(max_y-min_y);

   // Determine the DEM step size as a fraction of the LZ radius:
   const double CHIP_STEP_FACTOR = 0.25; // chip position increment as fraction of chip width
   ossim_int32 dem_step =
         (ossim_int32) floor(4*CHIP_STEP_FACTOR*m_hlzMinRadius/(m_demGsd.x+m_demGsd.y));

   // Hack: degrading to single thread when slope-image scheme is used. Runs extremely slow in
   // multithread mode, but much faster as single thread than multithreaded ls-fit
   if ((m_numThreads == 1) || !m_useLsFitMethod)
   {
      // Not threaded (or slope-image scheme):
      setPercentComplete(0);
      ossim_uint32 chipId = 0;
      for (chip_origin.y = min_y; chip_origin.y <= max_y; chip_origin.y += dem_step)
      {
         for (chip_origin.x = min_x; chip_origin.x <= max_x; chip_origin.x += dem_step)
         {
            ossimHlzUtil::ChipProcessorJob* job = 0;
            if (m_useLsFitMethod)
               job = new ossimHlzUtil::LsFitChipProcessorJob(this, chip_origin, chipId++);
            else
               job = new ossimHlzUtil::NormChipProcessorJob(this, chip_origin, chipId++);
            job->start();
         }
         setPercentComplete(100*chipId/numChips);
      }
   }
   else
   {
      if (m_numThreads == 0)
         m_numThreads = ossim::getNumberOfThreads();

      // Loop over input DEM, creating a thread job for each filter window:
      ossimRefPtr<ossimJobQueue> jobQueue = new ossimJobQueue();
      m_jobMtQueue = new ossimJobMultiThreadQueue(jobQueue.get(), m_numThreads);

      cout << "\nPreparing " << numChips << " jobs..." << endl; // TODO: DEBUG
      setPercentComplete(0);
      ossim_int32 qsize = 0;
      ossimIpt chip_origin;
      ossim_uint32 chipId = 0;
      for (chip_origin.y = min_y; chip_origin.y <= max_y; ++chip_origin.y)
      {
         for (chip_origin.x = min_x; chip_origin.x <= max_x; ++chip_origin.x)
         {
            //cout << "Submitting " << chipId << endl;
            ossimHlzUtil::ChipProcessorJob* job = 0;
            if (m_useLsFitMethod)
               job = new ossimHlzUtil::LsFitChipProcessorJob(this, chip_origin, chipId++);
            else
               job = new ossimHlzUtil::NormChipProcessorJob(this, chip_origin, chipId++);
            jobQueue->add(job, false);
         }
         qsize = jobQueue->size();
         setPercentComplete(100*(chipId-qsize)/numChips);
      }

      // Wait until all chips have been processed before proceeding:
      cout << "All jobs queued. Waiting for job threads to finish..." << endl;
      while (m_jobMtQueue->hasJobsToProcess() || m_jobMtQueue->numberOfBusyThreads())
      {
         qsize = m_jobMtQueue->getJobQueue()->size();
         setPercentComplete(100*(numChips-qsize)/numChips);
         OpenThreads::Thread::microSleep(10000);
      }
   }

   cout << "Finished processing chips." << endl;
   paintReticle();

   cout << "Writing output file..." << endl;
   success = writeFile();

   cout << "Returning..." << endl;
   return success;
}

void ossimHlzUtil::paintReticle()
{
   // Highlight the observer position with X reticle:
   ossimDpt center;
   m_viewRect.getCenter(center);

   if (m_reticleSize > 0)
   {
      m_outBuffer->setValue((int) center.x, (int) center.y, m_reticleValue);
      for (int i = -m_reticleSize; i <= m_reticleSize; ++i)
      {
         m_outBuffer->setValue(i + (int) center.x,       (int) center.y, m_reticleValue);
         m_outBuffer->setValue(    (int) center.x  , i + (int) center.y, m_reticleValue);
      }
   }

   // Also outline the square area of interest:
   for (ossim_int32 x = m_viewRect.ul().x; x <= m_viewRect.lr().x; ++x)
   {
      m_outBuffer->setValue(x, m_viewRect.ul().y, m_reticleValue);
      m_outBuffer->setValue(x, m_viewRect.lr().y, m_reticleValue);
   }
   for (ossim_int32 y = m_viewRect.ul().y; y <= m_viewRect.lr().y; ++y)
   {
      m_outBuffer->setValue(m_viewRect.ul().x, y, m_reticleValue);
      m_outBuffer->setValue(m_viewRect.lr().x, y, m_reticleValue);
   }
}

bool ossimHlzUtil::writeFile()
{
   ossimIrect rect(0, 0, m_viewRect.width() - 1, m_viewRect.height() - 1);
   m_outBuffer->setImageRectangle(rect);

   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(m_outBuffer);
   memSource->setImageGeometry(m_geom.get());
   ossimRefPtr<ossimImageSource>last_source = memSource.get();

   // See if an LUT is requested:
   if (m_lutFile.isReadable())
      addIndexToRgbLutFilter(last_source);

   // Set up the writer:
   bool success = false;
   ossimRefPtr<ossimImageFileWriter> writer = createNewWriter();
   if (writer.valid())
   {
      writer->connectMyInputTo(0, last_source.get());
      success = writer->execute();
   }

   return success;
}

void ossimHlzUtil::writeSlopeImage()
{
   // Set up the writer:
   ossimRefPtr<ossimImageFileWriter> writer = 0;
   ossimTiffWriter* tif_writer = new ossimTiffWriter();
   tif_writer->setGeotiffFlag(true);
   tif_writer->setFilename(m_slopeFile);
   writer = tif_writer;
   writer->connectMyInputTo(0, m_combinedElevSource.get());
   writer->setAreaOfInterest(m_viewRect);
   if (writer->execute())
      cout<<"Wrote slope image to <"<<m_slopeFile<<">."<<endl;
   else
   {
      cout<<"ossimHLZUtil::writeSlopeImage() Error encountered writing slope image to <"
            <<m_slopeFile<<">."<<endl;
   }
}

void ossimHlzUtil::dumpProductSummary() const
{
   ossimIpt isize(m_geom->getImageSize());
   cout << "\nSummary of HLZ product image:" << "\n   Output file name: " << m_productFile
         << "\n   Image size: " << isize
         << "\n   Slope threshold: " << m_slopeThreshold << " deg"
         << "\n   Roughness threshold: " << m_roughnessThreshold << " m"
         << "\n   Min LZ radius: " << m_hlzMinRadius << " m"
         << "\n   product GSD: " << m_gsd << " m"
         << "\n   radius-of-interest: " << (int) (m_gsd * isize.x / 2.0) << " m"
         << "\n   Scalar type: " << m_outBuffer->getScalarTypeAsString() << endl;
}

OpenThreads::ReadWriteMutex ossimHlzUtil::ChipProcessorJob::m_bufMutex;

ossimHlzUtil::ChipProcessorJob::ChipProcessorJob(ossimHlzUtil* hlzUtil, const ossimIpt& origin,
                                   ossim_uint32 /*chip_id*/)
: m_hlzUtil (hlzUtil),
  m_demChipUL (origin),
  m_status (0),
  m_nullValue (hlzUtil->m_demBuffer->getNullPix(0))
{
   m_demChipLR.x = m_demChipUL.x + m_hlzUtil->m_demFilterSize.x;
   m_demChipLR.y = m_demChipUL.y + m_hlzUtil->m_demFilterSize.y;
}

void ossimHlzUtil::ChipProcessorJob::start()
{
   if (level1Test() && level2Test() && maskTest())
   {
      // Passed all tests (though m_status may indicate obstruction), so mark this chip
      // appropriately as marginal or good:
      ossimIpt p;

      for (p.y = m_demChipUL.y; p.y < m_demChipLR.y; ++p.y)
      {
         for (p.x = m_demChipUL.x; p.x < m_demChipLR.x; ++p.x)
         {
            OpenThreads::ScopedWriteLock lock (m_bufMutex);
            if (m_status == 2)
               m_hlzUtil->m_outBuffer->setValue(p.x, p.y, m_hlzUtil->m_goodLzValue);
            else
               m_hlzUtil->m_outBuffer->setValue(p.x, p.y, m_hlzUtil->m_marginalLzValue);
         }
      }
   }
}

bool ossimHlzUtil::ChipProcessorJob::level2Test()
{
   // Level 2 only valid if a point cloud dataset is available:
   if (m_hlzUtil->m_pcSources.empty())
   {
      ++m_status; // assumes level2 passes
      return true;
   }

   // Need to convert DEM file coordinate bounds to geographic.
   ossimGpt chipUlGpt, chipLrGpt;
   m_hlzUtil->m_geom->localToWorld(ossimDpt(m_demChipUL), chipUlGpt);
   m_hlzUtil->m_geom->localToWorld(ossimDpt(m_demChipLR), chipLrGpt);
   chipUlGpt.hgt = ossim::nan();
   chipLrGpt.hgt = ossim::nan();
   ossimGrect grect (chipUlGpt, chipLrGpt);

   // TODO: LIMITATION: Only a single point cloud source is considered. Need to expand to handle
   // a list:
   const ossimPointCloudHandler* pc_src = m_hlzUtil->m_pcSources[0].get();

   // First check if there is even any coverage:
   m_status = 0; // reset assumes no coverage
   ossimGrect bb;
   pc_src->getBounds(bb);
   if (!bb.intersects(grect))
      return false;

   ossimPointBlock pc_block(0, ossimPointRecord::ReturnNumber|ossimPointRecord::NumberOfReturns);
   pc_src->getBlock(grect, pc_block);
   if (pc_block.empty())
      return false;

   bool found_obstruction = false;
   m_status = 1; // bump to indicate passed level 1

   // Scan the block for obstructions:
   ossimGpt point_plh;
   ossimDpt point_xy;
   ossim_uint32 numPoints = pc_block.size();
   for (ossim_uint32 i=0; (i<numPoints) && !found_obstruction; ++i)
   {
      //If this is not the only return, implies clutter along the ray:
      int num_returns = (int) pc_block[i]->getField(ossimPointRecord::NumberOfReturns);
      if (num_returns > 1)
      {
         found_obstruction = true;
         break;
      }
   }

   if (!found_obstruction)
      m_status = 2;

   return true;
}


bool ossimHlzUtil::ChipProcessorJob::maskTest()
{
   // Threat dome only valid if a point cloud dataset is available:
   if (m_hlzUtil->m_maskSources.empty())
      return true;

   ossimIrect chipRect (m_demChipUL, m_demChipLR);
   vector<MaskSource>::iterator mask_source = m_hlzUtil->m_maskSources.begin();
   bool test_passed = true;
   ossimIpt p;
   ossim_uint8 mask_value;

   while (mask_source != m_hlzUtil->m_maskSources.end())
   {
      ossimRefPtr<ossimImageData> mask_data = mask_source->image->getTile(chipRect);
      for (p.y = m_demChipUL.y; (p.y < m_demChipLR.y) && test_passed; ++p.y)
      {
         for (p.x = m_demChipUL.x; (p.x < m_demChipLR.x) && test_passed; ++p.x)
         {
            mask_value = mask_data->getPix(p);
            if (  ( mask_value &&  mask_source->exclude) ||
                  (!mask_value && !mask_source->exclude) )
               test_passed = false;
         }
      }
      if (!test_passed)
         break;

      ++mask_source;
   }

   return test_passed;
}


bool ossimHlzUtil::LsFitChipProcessorJob::level1Test()
{
   // Start with computing best-fit plane:
   ossimIpt p;
   double z;
   double y_meters;
   for (p.y = m_demChipUL.y; p.y < m_demChipLR.y; ++p.y)
   {
      y_meters = p.y*m_hlzUtil->m_demGsd.y;
      for (p.x = m_demChipUL.x; p.x < m_demChipLR.x; ++p.x)
      {
         z = m_hlzUtil->m_demBuffer->getPix(p, 0);
         if ((z == m_nullValue) || ossim::isnan(z))
            return false;
         m_plane->addSample(p.x*m_hlzUtil->m_demGsd.x, y_meters, z);
      }
   }
   if (!m_plane->solveLS())
      return false;

   // The slope is derived from the normal unit vector. Extract that from the solution and test
   // against threshold:
   double a, b, c;
   m_plane->getLSParms(a, b, c);
   double z_proj = 1.0 / sqrt(a*a + b*b + 1.0);
   double theta = fabs(ossim::acosd(z_proj));
   if (theta > m_hlzUtil->m_slopeThreshold)
      return false;

   // Passed the slope test. Now measure the roughness as peak deviation from the plane:
   double distance;
   for (p.y = m_demChipUL.y; (p.y < m_demChipLR.y); ++p.y)
   {
      for (p.x = m_demChipUL.x; (p.x < m_demChipLR.x); ++p.x)
      {
         z = m_hlzUtil->m_demBuffer->getPix(p, 0);
         distance = fabs(z_proj * (a*p.x + b*p.y + c - z));
         if (distance > m_hlzUtil->m_roughnessThreshold)
            return false;
      }
   }

   m_status = 1; // indicates passed level 1
   return true;
}

bool ossimHlzUtil::NormChipProcessorJob::level1Test()
{
   // The processing chain is outputing slope values in degrees from vertical.
   // Scan the data tile for slopes outside the threshold:
   ossimIpt p;
   float theta;
   for (p.y = m_demChipUL.y; p.y < m_demChipLR.y; ++p.y)
   {
      for (p.x = m_demChipUL.x; p.x < m_demChipLR.x; ++p.x)
      {
         theta = m_hlzUtil->m_demBuffer->getPix(p, 0);
         if ((theta == m_nullValue) || ossim::isnan(theta) || (theta > m_hlzUtil->m_slopeThreshold))
            return false;
      }
   }

   m_status = 1; // indicates passed level 1
   return true;
}

ossimHlzUtil::MaskSource::MaskSource(ossimHlzUtil* hlzUtil, const ossimFilename& mask_image,
                                     bool exclusion)
:  exclude (exclusion),
   valid (false)
{
   ossimRefPtr<ossimImageHandler> handler = ossimImageHandlerRegistry::instance()->open(mask_image);
   if (handler.valid())
   {
      hlzUtil->createInputChain(handler, image);
      valid = true;
   }
}

















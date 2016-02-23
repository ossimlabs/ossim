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
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/point_cloud/ossimPointCloudHandlerRegistry.h>
#include <ossim/util/ossimHlzUtil.h>
#include <fstream>

static const string MASK_EXCLUDE_KW = "excludes";
static const string MASK_INCLUDE_KW = "includes";
static const string SLOPE_OUTPUT_FILE_KW = "slope_output_file";
static const string POINT_CLOUD_FILE_KW = "point_cloud_file";
static const string HLZ_CODING_KW = "hlz_coding";
static const string LZ_MIN_RADIUS_KW = "lz_min_radius";
static const string ROUGHNESS_THRESHOLD_KW = "roughness_threshold";
static const string SLOPE_THRESHOLD_KW = "slope_threshold";

const char* ossimHlzUtil::DESCRIPTION =
      "Computes bitmap of helicopter landing zones given ROI and DEM.";

using namespace std;

ossimHlzUtil::ossimHlzUtil()
: m_slopeThreshold(7.0),
  m_roughnessThreshold(0.5),
  m_hlzMinRadius(25.0),
  m_outBuffer(NULL),
  m_badLzValue(255),
  m_marginalLzValue(128),
  m_goodLzValue(64),
  m_useLsFitMethod(true),
  m_numThreads(1),
  d_accumT(0)
{
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
   au->addCommandLineOption("--excludes <file1>[, <file2>...]",
         "List of raster image(s) representing mask files that defines regions to be excluded from."
         "HLZ solutions. Any non-zero pixel is excluded Multiple filenames must be comma-separated.");
   au->addCommandLineOption("--hlz-coding <bad> <marginal> <good>",
         "Specifies the pixel values (0-255) for the output product corresponding to bad, marginal, "
         "and good landing zones, respectively. Defaults to bad=255 (null), marginal=128, and "
         "good=64.");
   au->addCommandLineOption("--includes <file1>[, <file2>...]",
         "List of raster image(s) representing mask files that defines regions where the HLZs ."
         "identified must overlap. Any non-zero pixel represents an inclusion zone. Multiple "
         "filenames must be comma-separated.");
   au->addCommandLineOption("--output-slope <filename.tif>",
         "Generates a slope byproduct image (floating point degrees) to the specified filename. "
         "Only valid if normal-vector method used (i.e., --ls-fit option NOT specified)");
   au->addCommandLineOption("--pc | --point-cloud <file1>[, <file2>...]",
         "Specifies ancillary point-cloud data file(s) for level-2 search for obstructions. "
         "Must be comma-separated file names.");
   au->addCommandLineOption("--min-lz-radius <meters>",
         "Specifies minimum radius of landing zone. Defaults to 25 m. ");
   au->addCommandLineOption("--max-roughness <meters>",
         "Specifies the terrain roughness threshold (meters). This is the maximum deviation from a "
         "flat plane permitted. Defaults to 0.5 m. Valid only with --ls-fit specified.");
   au->addCommandLineOption("--max-slope <degrees>",
         "Threshold for acceptable landing zone terrain slope. Defaults to 7 deg.");
   au->addCommandLineOption("--threads <n>",
         "Number of threads. Defaults to use single core. For engineering/debug purposes.");
   au->addCommandLineOption("--use-slope",
         "Slope is computed from the normal vector using neighboring posts instead of "
         "least-squares fit to a plane (prefered). For engineering/debug purposes.");
}

bool ossimHlzUtil::initialize(ossimArgumentParser& ap)
{
   if (!ossimChipProcUtil::initialize(ap))
      return false;

   string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);
   string ts2;
   ossimArgumentParser::ossimParameter sp2(ts2);
   string ts3;
   ossimArgumentParser::ossimParameter sp3(ts3);
   ossimString  key ;

   vector<ossimString> maskFnames;
   ap.read("--excludes", maskFnames);
   for(ossim_uint32 idx=0; idx<maskFnames.size(); ++idx)
   {
      key = MASK_EXCLUDE_KW;
      key += ossimString::toString(idx++);
      m_kwl.addPair(key.string(), maskFnames[idx] );
   }

   if (ap.read("--hlz-coding", sp1, sp2, sp3) || ap.read("--values", sp1, sp2, sp3))
   {
      ostringstream value;
      value<<ts1<<" "<<ts2<<" "<<ts3;
      m_kwl.addPair( HLZ_CODING_KW, value.str() );
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
      m_kwl.addPair(SLOPE_OUTPUT_FILE_KW, ts1);

   vector<ossimString> pcFnames;
   ap.read("--point-clouds", pcFnames);
   for(ossim_uint32 idx=0; idx<pcFnames.size(); ++idx)
   {
      key = POINT_CLOUD_FILE_KW;
      key += ossimString::toString(idx++);
      m_kwl.addPair(key.string(), pcFnames[idx] );
   }

   if (ap.read("--min-lz-radius", sp1) || ap.read("--rlz", sp1))
      m_kwl.addPair(LZ_MIN_RADIUS_KW, ts1);

   if (ap.read("--max-roughness", sp1) || ap.read("--roughness", sp1))
      m_kwl.addPair(ROUGHNESS_THRESHOLD_KW, ts1);

   if (ap.read("--max-slope", sp1) || ap.read("--slope", sp1))
      m_kwl.addPair(SLOPE_THRESHOLD_KW, ts1);

   if (ap.read("--threads", sp1))
   {
      // Command line mode only
      m_numThreads = ossimString(ts1).toUInt32();
   }

   if (ap.read("--use_slope"))
   {
      // Command line mode only
      m_useLsFitMethod = false;
   }

   processRemainingArgs(ap);
   return true;
}

void ossimHlzUtil::initialize(const ossimKeywordlist& kwl)
{
   ossimString value;
   ostringstream xmsg;

   value = m_kwl.findKey(LZ_MIN_RADIUS_KW);
   if (!value.empty())
      m_hlzMinRadius = value.toDouble();
   if (m_hlzMinRadius < 1.0)
   {
      xmsg<<"ossimHlzUtil:"<<__LINE__<<"  The HLZ minimum radius is too small.";
      throw(xmsg.str());
   }

   value = m_kwl.findKey(ROUGHNESS_THRESHOLD_KW);
   if (!value.empty())
      m_roughnessThreshold = value.toDouble();

   value = m_kwl.findKey(SLOPE_THRESHOLD_KW);
   if (!value.empty())
      m_slopeThreshold = value.toDouble();

   value = m_kwl.findKey(HLZ_CODING_KW);
   if (!value.empty())
   {
      vector<ossimString> values = value.split(" ");
      if (values.size() == 3)
      {
         m_badLzValue = values[0].toUInt8();
         m_marginalLzValue = values[1].toUInt8();
         m_goodLzValue = values[2].toUInt8();
      }
      else
      {
         xmsg<<"ossimHlzUtil:"<<__LINE__<<"  Unexpected number of values encountered for keyword <"
               <<HLZ_CODING_KW<<">.";
         throw(xmsg.str());
      }
   }

   ossimChipProcUtil::initialize(kwl);
}

void ossimHlzUtil::initProcessingChain()
{
   ostringstream xmsg;

   if (m_aoiGroundRect.hasNans() || m_aoiViewRect.hasNans())
   {
      xmsg<<"ossimHlzUtil:"<<__LINE__<<"  Encountered NaNs in AOI."<<ends;
      throw ossimException(xmsg.str());
   }


   // If PC provided as file on command line, Load it. This uses the output ground rect so needs to
   // be after the initialization of m_geom:
   loadPcFiles();

   // If threat-domes spec (or any mask) provided as file on command line, Load it:
   loadMaskFiles();

   // In order to use the slope filter to establish terrain quality, the elevation data needs to
   // be loaded as images, not elevation cells. Need to transfer relevant cells to image chains:
   mosaicDemSources();

   // The "chain" for this utility is just the memory source containing the output buffer:
   m_outBuffer = ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, m_aoiViewRect.width(),
                                                           m_aoiViewRect.height());
   if (!m_outBuffer.valid())
   {
      xmsg<<"ossimHlzUtil:"<<__LINE__<<"  Error encountered allocating output image buffer.";
      throw(xmsg.str());
   }
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

   if (!m_useLsFitMethod)
   {
      // Add the slope computation engine on the elevation source.
      // Set up processing chain with plane to normal filter, equation combiner, and band selector.
      ossimRefPtr<ossimSlopeFilter> slope_filter = new ossimSlopeFilter;
      slope_filter->connectMyInputTo(m_combinedElevSource.get());
      slope_filter->setSlopeType(ossimSlopeFilter::DEGREES);
      m_combinedElevSource = slope_filter.get();
      m_combinedElevSource->initialize();

      m_slopeFile = m_kwl.findKey(SLOPE_OUTPUT_FILE_KW);
      if (!m_slopeFile.empty())
         writeSlopeImage();
   }
   // Determine number of posts (in one dimension) needed to cover the specified LZ radius:
   if ((m_gsd.x == 0) || (m_gsd.y == 0) || m_gsd.hasNans())
   {
      ostringstream xmsg;
      xmsg<<"ossimHlzUtil:"<<__LINE__<<"  Invalid GSD: "<<m_gsd;
      throw(xmsg.str());
   }

   m_demFilterSize.x = (int) ceil(m_hlzMinRadius/m_gsd.x);
   m_demFilterSize.y = (int) ceil(m_hlzMinRadius/m_gsd.y);
   if ((m_demFilterSize.x < 2) || (m_demFilterSize.y < 2))
   {
      xmsg<<"ossimHlzUtil:"<<__LINE__<<"  The DEM provided does not have sufficient"
                  " resolution to determine HLZs.";
      throw ossimException(xmsg.str());
   }
}

void ossimHlzUtil::loadPcFiles()
{
   ostringstream xmsg;

   ossimString key;
   ossimFilename pc_file;
   ossimPointCloudHandlerRegistry* registry = ossimPointCloudHandlerRegistry::instance();
   ossimGrect pc_bbox;

   for(ossim_uint32 idx=0; true; ++idx)
   {
      key = POINT_CLOUD_FILE_KW;
      key += ossimString::toString(idx++);
      pc_file = m_kwl.find(key.chars());
      if (pc_file.empty())
         break;

      ossimRefPtr<ossimPointCloudHandler> pc_handler =  registry->open(pc_file);
      if (!pc_handler.valid())
      {
         xmsg<<"ossimHlzUtil:"<<__LINE__<<"  Error loading point cloud file <"<<pc_file<<">.";
         throw(xmsg.str());
      }

      // Verify that PC bounding rect overlaps the output bounding rect:
      pc_handler->getBounds(pc_bbox);
      if (m_aoiGroundRect.intersects(pc_bbox))
      {
         m_pcSources.push_back(pc_handler);
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
               << "ossimHLZUtil::loadPcFile() WARNING: point-cloud file <" << pc_file << "> "
               << "does not overlap the output ROI." << endl;
      }
   }
}

void ossimHlzUtil::loadMaskFiles()
{
   ostringstream xmsg;

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

      MaskSource mask_image (this, mask_file, true);
      if (mask_image.image.valid())
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
      MaskSource mask_image (this, mask_file, false);
      if (mask_image.image.valid())
         m_maskSources.push_back(mask_image);
   }
}

void ossimHlzUtil::mosaicDemSources()
{
   ostringstream xmsg;

   if (m_demSources.empty())
   {
      // Establish connection to DEM posts directly as raster "images" versus using the OSSIM elev
      // manager that performs interpolation of DEM posts for arbitrary locations. These elev images
      // feed into a combiner in order to have a common tap for elev pixels:
      ossimElevManager* elevMgr = ossimElevManager::instance();
      elevMgr->getCellsForBounds(m_aoiGroundRect, m_demSources);
   }

   // Open a raster image for each elevation source being considered:
   ossimConnectableObject::ConnectableObjectList elevChains;
   vector<ossimFilename>::iterator fname_iter = m_demSources.begin();
   while (fname_iter != m_demSources.end())
   {
      ossimRefPtr<ossimSingleImageChain> chain = createInputChain(*fname_iter).get();
      if (!chain.valid() || !chain->getImageRenderer().valid() )
      {
         xmsg<<"ossimHlzUtil:"<<__LINE__<<"  Cannot open DEM file at <"<<*fname_iter<<">";
         throw(xmsg.str());
      }

      // Set up the input chain with proper renderer IVT:
      ossimRefPtr<ossimImageViewProjectionTransform> ivt = new ossimImageViewProjectionTransform
            (chain->getImageHandler()->getImageGeometry().get(), m_geom.get());
      chain->getImageRenderer()->setImageViewTransform(ivt.get());
      ossimRefPtr<ossimConnectableObject> connectable = chain.get();
      elevChains.push_back(connectable);
      ++fname_iter;
   }

   if (elevChains.size() == 1)
      m_combinedElevSource = (ossimImageSource*) elevChains[0].get();
   else
      m_combinedElevSource = new ossimImageMosaic(elevChains);
}

ossimRefPtr<ossimImageData> ossimHlzUtil::getChip(const ossimIrect& bounding_irect)
{
   ostringstream xmsg;
   if (!m_geom.valid())
      return 0;

   m_aoiViewRect = bounding_irect;
   m_geom->setImageSize( m_aoiViewRect.size() );

   // To help with multithreading, just load entire AOI of DEM into memory:
   m_demBuffer = m_combinedElevSource->getTile(m_aoiViewRect);
   if (!m_demBuffer.valid())
      return 0;

   // Allocate the output image buffer:
   m_outBuffer = ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, m_aoiViewRect.width(),
                                                           m_aoiViewRect.height());
   if (!m_outBuffer.valid() || !m_memSource.valid())
   {
      xmsg<<"ossimHlzUtil:"<<__LINE__<<"  Error encountered allocating output image buffer.";
      throw(xmsg.str());
   }

   // Initialize the image with all points hidden:
   m_outBuffer->initialize();
   m_outBuffer->setImageRectangle(m_aoiViewRect);
   m_outBuffer->fill(0);
   m_memSource->setImage(m_outBuffer);

   d_accumT = 0;
   bool success = false;

   // Establish loop limits in input DEM raster coordinate space:
   ossim_int32 min_x = m_aoiViewRect.ul().x;
   ossim_int32 min_y = m_aoiViewRect.ul().y;
   ossim_int32 max_x = m_aoiViewRect.lr().x - m_demFilterSize.x;
   ossim_int32 max_y = m_aoiViewRect.lr().y - m_demFilterSize.y;
   ossimIpt chip_origin;
   ossim_uint32 numPatches = (max_x-min_x)*(max_y-min_y);

   // Determine the DEM step size as a fraction of the LZ radius:
   const double CHIP_STEP_FACTOR = 0.25; // chip position increment as fraction of chip width
   ossim_int32 dem_step =
         (ossim_int32) floor(4*CHIP_STEP_FACTOR*m_hlzMinRadius/(m_gsd.x+m_gsd.y));
   if (dem_step <= 0)
      dem_step = 1;

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
            ossimHlzUtil::PatchProcessorJob* job = 0;
            if (m_useLsFitMethod)
               job = new ossimHlzUtil::LsFitPatchProcessorJob(this, chip_origin, chipId++);
            else
               job = new ossimHlzUtil::NormPatchProcessorJob(this, chip_origin, chipId++);
            job->start();
         }
         setPercentComplete(100*chipId/numPatches);
      }
   }
   else
   {
      if (m_numThreads == 0)
         m_numThreads = ossim::getNumberOfThreads();

      // Loop over input DEM, creating a thread job for each filter window:
      ossimRefPtr<ossimJobMultiThreadQueue> jobMtQueue =
            new ossimJobMultiThreadQueue(0, m_numThreads);
      ossimJobQueue* jobQueue = jobMtQueue->getJobQueue();

      cout << "\nPreparing " << numPatches << " jobs..." << endl; // TODO: DEBUG
      setPercentComplete(0);
      ossim_int32 qsize = 0;
      ossimIpt chip_origin;
      ossim_uint32 chipId = 0;
      for (chip_origin.y = min_y; chip_origin.y <= max_y; ++chip_origin.y)
      {
         for (chip_origin.x = min_x; chip_origin.x <= max_x; ++chip_origin.x)
         {
            //cout << "Submitting " << chipId << endl;
            ossimHlzUtil::PatchProcessorJob* job = 0;
            if (m_useLsFitMethod)
               job = new ossimHlzUtil::LsFitPatchProcessorJob(this, chip_origin, chipId++);
            else
               job = new ossimHlzUtil::NormPatchProcessorJob(this, chip_origin, chipId++);
            jobQueue->add(job, false);
         }
         qsize = jobQueue->size();
         setPercentComplete(100*(chipId-qsize)/numPatches);
      }

      // Wait until all chips have been processed before proceeding:
      cout << "All jobs queued. Waiting for job threads to finish..." << endl;
      while (jobMtQueue->hasJobsToProcess() || jobMtQueue->numberOfBusyThreads())
      {
         qsize = jobMtQueue->getJobQueue()->size();
         setPercentComplete(100*(numPatches-qsize)/numPatches);
         OpenThreads::Thread::microSleep(10000);
      }
      jobMtQueue = 0;
   }

   cout << "Finished processing chips." << endl;

   return m_outBuffer;
}

bool ossimHlzUtil::execute()
{
   getChip(m_aoiViewRect);
   return ossimChipProcUtil::execute();
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
   writer->setAreaOfInterest(m_aoiViewRect);
   if (writer->execute())
      cout<<"Wrote slope image to <"<<m_slopeFile<<">."<<endl;
   else
   {
      cout<<"ossimHLZUtil::writeSlopeImage() Error encountered writing slope image to <"
            <<m_slopeFile<<">."<<endl;
   }
}

OpenThreads::ReadWriteMutex ossimHlzUtil::PatchProcessorJob::m_bufMutex;

ossimHlzUtil::PatchProcessorJob::PatchProcessorJob(ossimHlzUtil* hlzUtil, const ossimIpt& origin,
                                   ossim_uint32 /*chip_id*/)
: m_hlzUtil (hlzUtil),
  m_demPatchUL (origin),
  m_status (0),
  m_nullValue (hlzUtil->m_demBuffer->getNullPix(0))
{
   m_demPatchLR.x = m_demPatchUL.x + m_hlzUtil->m_demFilterSize.x;
   m_demPatchLR.y = m_demPatchUL.y + m_hlzUtil->m_demFilterSize.y;
}

void ossimHlzUtil::PatchProcessorJob::start()
{
   bool passed = level1Test() && level2Test() && maskTest();
   ossimIpt p;

   OpenThreads::ScopedWriteLock lock (m_bufMutex);
   for (p.y = m_demPatchUL.y; p.y < m_demPatchLR.y; ++p.y)
   {
      for (p.x = m_demPatchUL.x; p.x < m_demPatchLR.x; ++p.x)
      {
         if (passed && (m_status == 2))
            m_hlzUtil->m_outBuffer->setValue(p.x, p.y, m_hlzUtil->m_goodLzValue);
         else if (passed && (m_status == 1))
            m_hlzUtil->m_outBuffer->setValue(p.x, p.y, m_hlzUtil->m_marginalLzValue);
         else
            m_hlzUtil->m_outBuffer->setValue(p.x, p.y, m_hlzUtil->m_badLzValue);
      }
   }
}

bool ossimHlzUtil::LsFitPatchProcessorJob::level1Test()
{
   // Start with computing best-fit plane:
   ossimIpt p;
   double z;
   double y_meters;
   for (p.y = m_demPatchUL.y; p.y < m_demPatchLR.y; ++p.y)
   {
      y_meters = p.y*m_hlzUtil->m_gsd.y;
      for (p.x = m_demPatchUL.x; p.x < m_demPatchLR.x; ++p.x)
      {
         z = m_hlzUtil->m_demBuffer->getPix(p, 0);
         if ((z == m_nullValue) || ossim::isnan(z))
            return false;
         m_plane->addSample(p.x*m_hlzUtil->m_gsd.x, y_meters, z);
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
   for (p.y = m_demPatchUL.y; (p.y < m_demPatchLR.y); ++p.y)
   {
      for (p.x = m_demPatchUL.x; (p.x < m_demPatchLR.x); ++p.x)
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

bool ossimHlzUtil::NormPatchProcessorJob::level1Test()
{
   // The processing chain is outputing slope values in degrees from vertical.
   // Scan the data tile for slopes outside the threshold:
   ossimIpt p;
   float theta;
   for (p.y = m_demPatchUL.y; p.y < m_demPatchLR.y; ++p.y)
   {
      for (p.x = m_demPatchUL.x; p.x < m_demPatchLR.x; ++p.x)
      {
         theta = m_hlzUtil->m_demBuffer->getPix(p, 0);
         if ((theta == m_nullValue) || ossim::isnan(theta) || (theta > m_hlzUtil->m_slopeThreshold))
            return false;
      }
   }

   m_status = 1; // indicates passed level 1
   return true;
}

bool ossimHlzUtil::PatchProcessorJob::level2Test()
{
   // Level 2 only valid if a point cloud dataset is available:
   if (m_hlzUtil->m_pcSources.empty())
   {
      ++m_status; // assumes level2 passes
      return true;
   }

   // Need to convert DEM file coordinate bounds to geographic.
   ossimGpt chipUlGpt, chipLrGpt;
   m_hlzUtil->m_geom->localToWorld(ossimDpt(m_demPatchUL), chipUlGpt);
   m_hlzUtil->m_geom->localToWorld(ossimDpt(m_demPatchLR), chipLrGpt);
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

bool ossimHlzUtil::PatchProcessorJob::maskTest()
{
   // Threat dome only valid if a mask source is available:
   if (m_hlzUtil->m_maskSources.empty())
      return true;

   ossimIrect chipRect (m_demPatchUL, m_demPatchLR);
   vector<MaskSource>::iterator mask_source = m_hlzUtil->m_maskSources.begin();
   bool test_passed = true;
   ossimIpt p;
   ossim_uint8 mask_value;

   while ((mask_source != m_hlzUtil->m_maskSources.end()) && test_passed)
   {
      ossimRefPtr<ossimImageData> mask_data = mask_source->image->getTile(chipRect);
      for (p.y = m_demPatchUL.y; (p.y < m_demPatchLR.y) && test_passed; ++p.y)
      {
         for (p.x = m_demPatchUL.x; (p.x < m_demPatchLR.x) && test_passed; ++p.x)
         {
            mask_value = mask_data->getPix(p);
            if (( mask_value &&  mask_source->exclude) || (!mask_value && !mask_source->exclude))
               test_passed = false;
         }
      }
      ++mask_source;
   }

   return test_passed;
}

ossimHlzUtil::MaskSource::MaskSource(ossimHlzUtil* hlzUtil,
                                     const ossimFilename& mask_image,
                                     bool exclusion)
:  exclude (exclusion)
{
   image = hlzUtil->createInputChain(mask_image);
}

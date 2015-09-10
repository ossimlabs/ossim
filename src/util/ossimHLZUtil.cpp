//*******************************************************************
// License:  See top level LICENSE.txt file.
// Author:  Oscar Kramer
//*******************************************************************
//  $Id: ossimHLZUtil.cpp 23465 2015-08-13 13:36:26Z okramer $

#include <ossim/util/ossimHLZUtil.h>

#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimRtti.h>
#include <ossim/base/ossimGrect.h>
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
#include <fstream>

const char* MASK_PREFIX = "mask";
const char* MASK_EXCLUDE_KW = "exclude";

ossimHLZUtil::ossimHLZUtil()
: m_slopeThreshold(7.0),
  m_roughnessThreshold(0.5),
  m_hlzMinRadius(25.0),
  m_aoiRadius(0),
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
  m_numThreads(1),
  d_accumT(0)
{
   m_destinationGpt.makeNan();
}

ossimHLZUtil::~ossimHLZUtil()
{
}

void ossimHLZUtil::usage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimInit::instance()->addOptions(ap);

   // Add options.
   addArguments(ap);

   // Write usage.
   ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));

   ossimNotify(ossimNotifyLevel_INFO)
   << "\nFinds acceptable helicopter landing zones given terrain data and LZ criteria. The "
   << "output is an 8-bit, single band, raster image. Options exist for customizing the output"
   << " pixel values including an option to generate a color raster with the use of a look-up"
   << " table.\n\n"
   << "Examples:\n\n"
   << "    "<<ap.getApplicationName()<<" [options] --roi 5000 --rlz 25 --target 25.5000 -80.000 output-hlz.tif\n"
   << "    "<<ap.getApplicationName()<<" [options] --dem mydata.hgt --rlz 25 output-hlz.tif \n\n"
   << std::endl;
}

void ossimHLZUtil::addArguments(ossimArgumentParser& ap)
{
   // Set the general usage:
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " [options] <output-image>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->addCommandLineOption(
         "--gsd <meters>",
         "Specifies output GSD in meters. Defaults to the same resolution as best input DEM. "
         "Alternatively, if a DEM file is specified, the product GSD defaults to the input DEM's GSD.");
   au->addCommandLineOption(
         "--dem <filename>",
         "Specifies the input DEM filename to use for level-1 processing. If none provided, the "
         "elevation database is referenced as specified in prefs file for the ROI specified.");
   au->addCommandLineOption(
         "--ls-fit",
         "Slope is computed via an LS fit to a plane instead of the default slope computation using "
         "differences to compute normal vector.");
   au->addCommandLineOption(
         "--lut <filename>",
         "Specifies the optional lookup table filename for mapping the single-band output image to "
         "an RGB. The LUT provided must be in the ossimIndexToRgbLutFilter format and must handle "
         "the discrete output values (see --values option).");
   au->addCommandLineOption(
         "--mask <filename>",
         "Either a Keyword-list file or raster image. The KWL file can contain multiple mask files "
         "and how to interpret them, i.e., whether non-null pixels define exclusion zones (cannot "
         "land inside) or inclusion (must land inside). A single mask raster image is assumed to "
         "represent exclusion zones.");
   au->addCommandLineOption(
         "--output-slope <filename.tif>",
         "Generates a slope byproduct image (floating point degrees) to the specified filename. "
         "Only valid if normal-vector method used (i.e., --ls-fit option NOT specified)");
   au->addCommandLineOption(
         "--pc | --point-cloud <filename>",
         "Specifies ancillary point-cloud data file for level-2 search for obstructions.");
   au->addCommandLineOption(
         "--request-api",
         "Causes applications API to be output as JSON to stdout. Accepts optional filename to "
         "store JSON output.");
   au->addCommandLineOption(
         "--reticle <int>",
         "Specifies the size of the reticle at the destination point location in pixels from the "
         "center (i.e., the radius of the reticle). Defaults to 10. A value of 0 hides the reticle. "
         "See --values option for setting reticle color.");
   au->addCommandLineOption(
         "--rlz <meters>",
         "Specifies minimum radius of landing zone. Defaults to 25 m. ");
   au->addCommandLineOption(
         "--roi <meters>",
         "radius of interest surrounding the destination point. If absent, the product defaults to "
         "1024 x 1024 pixels, with a radius of 512 * GSD. Alternatively, if a DEM file is "
         "specified, the product ROI defaults to the full DEM coverage.");
   au->addCommandLineOption(
         "--roughness <meters>",
         "Specifies the terrain roughness threshold (meters). This is the maximum deviation from a "
         "flat plane permitted. Defaults to 0.5 m. Valid only with --ls-fit specified.");
   au->addCommandLineOption("--simulation", "For engineering/debug purposes ");
   au->addCommandLineOption(
         "--size <int>",
         "Instead of a radius of interest, directly specifies the dimensions of the "
         " output product in pixels (output is square). Required unless --roi is specified and/or "
         "an inout DEM is specified.");
   au->addCommandLineOption(
         "--slope <degrees>",
         "Threshold for acceptable landing zone terrain slope. Defaults to 7 deg.");
   au->addCommandLineOption(
         "--summary",
         "Causes a product summary to be output to the console.");
   au->addCommandLineOption(
         "--target <lat> <lon>",
         "The center target destination around which suitable HLZs are identified. This can be "
         "omitted if an input DEM file is provided, in which case the center of the DEM is the target.");
   au->addCommandLineOption(
         "--threads <n>",
         "Number of threads. Defaults to use single core. For engineering/debug purposes.");
   au->addCommandLineOption(
         "--values <bad marg good ret>",
         "Specifies the pixel values (0-255) for the output product corresponding to bad, marginal, "
         "and good landing zones, respectively, with the fourth value representing the reticle "
         "value. Defaults to bad=0 (null), marg=1, , good=2, and reticle is highlighted with 3.");
}

bool ossimHLZUtil::parseCommand(ossimArgumentParser& ap)
{
   if ((ap.argc() == 1) || ap.read("-h") || ap.read("--help"))
   {
      usage(ap);
      return false;
   }

   std::string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);
   std::string ts2;
   ossimArgumentParser::ossimParameter sp2(ts2);
   std::string ts3;
   ossimArgumentParser::ossimParameter sp3(ts3);
   std::string ts4;
   ossimArgumentParser::ossimParameter sp4(ts4);

   if (ap.read("--gsd", sp1))
      m_gsd = ossimString(ts1).toDouble();

   if (ap.read("--dem", sp1))
      m_demFile = ts1;

   if (ap.read("--ls-fit"))
      m_useLsFitMethod = true;

   if (ap.read("--lut", sp1))
      m_lutFile = ts1;

   if (ap.read("--mask", sp1))
      m_maskFile = ts1;

   if ( ap.read("--output-slope", sp1))
   {
      m_slopeFile = ts1;
   }
   if (ap.read("--pc", sp1) || ap.read("--point-cloud", sp1))
      m_pcFile = ts1;

   if ( ap.read("--request-api", sp1))
   {
      ofstream ofs ( ts1.c_str() );
      printApiJson(ofs);
      ofs.close();
      return false;
   }
   if ( ap.read("--request-api"))
   {
      printApiJson(cout);
      return false;
   }
   if (ap.read("--reticle", sp1))
      m_reticleSize = ossimString(ts1).toInt32();

   if (ap.read("--rlz", sp1))
      m_hlzMinRadius = ossimString(ts1).toDouble();

   if (ap.read("--roi", sp1))
      m_aoiRadius = ossimString(ts1).toDouble();

   if (ap.read("--roughness", sp1))
      m_roughnessThreshold = ossimString(ts1).toDouble();

   if (ap.read("--summary"))
      m_outputSummary = true;

   if (ap.read("--size", sp1))
   {
      m_viewRect.set_lrx(ossimString(ts1).toUInt32());
      m_viewRect.set_lry(m_viewRect.lr().x);
   }

   if (ap.read("--slope", sp1))
      m_slopeThreshold = ossimString(ts1).toDouble();

   if (ap.read("--target", sp1, sp2))
   {
      m_destinationGpt.lat = ossimString(ts1).toDouble();
      m_destinationGpt.lon = ossimString(ts2).toDouble();
   }

   if (ap.read("--threads", sp1))
   {
      m_numThreads = ossimString(ts1).toUInt32();
   }

   if (ap.read("--values", sp1, sp2, sp3, sp4))
   {
      m_badLzValue = ossimString(ts1).toUInt8();
      m_marginalLzValue = ossimString(ts2).toUInt8();
      m_goodLzValue = ossimString(ts3).toUInt8();
      m_reticleValue = ossimString(ts4).toUInt8();
   }

   // There should only be the required command line args left:
   if (ap.argc() != 2)
   {
      usage(ap);
      return false;
   }

   // Parse the required command line params:
   m_productFile = ap[1];

   // Verify minimum required args were specified:
   if (m_demFile.empty() && m_destinationGpt.isLonLatNan())
   {
      ossimNotify(ossimNotifyLevel_WARN)
                  << "ossimHLZUtil::initialize ERR: Command line is underspecified." << std::endl;
      usage(ap);
      return false;
   }
   return initialize();
}

bool ossimHLZUtil::initialize()
{
   m_productGeom = new ossimImageGeometry;

   // If DEM provided as file on command line, reset the elev manager to use only this:
   if (!m_demFile.empty() && !loadDemFile())
      return false;

   // Determine if default GSD needs to be computed.
   if (m_gsd == 0)
   {
      // Query for target H so as to autoload cell(s):
      ossimElevManager::instance()->getHeightAboveEllipsoid(m_destinationGpt);
      m_gsd = ossimElevManager::instance()->getMeanSpacingMeters();
      if (ossim::isnan(m_gsd))
         m_gsd = 0;
   }

   // Make sure that AOI and GSD are initialized:
   if ((m_aoiRadius == 0) && (m_gsd != 0))
   {
      // The radius of interest can default given a GSD to achieve a specific output image size.
      if (m_viewRect.area() <= 1)
            m_viewRect = ossimIrect(0, 0, 1023, 1023);
      m_aoiRadius = (m_viewRect.size().x + m_viewRect.size().y - 2) * m_gsd / 4.0;
   }
   else if ((m_gsd == 0) && (m_aoiRadius != 0))
   {
      // Likewise, the GSD can default given an AOI and image size:
      if (m_viewRect.area() <= 1)
            m_viewRect = ossimIrect(0,0,1023,1023);
      m_gsd = 4.0 * m_aoiRadius / (m_viewRect.size().x + m_viewRect.size().y - 2);
   }
   if ((m_gsd == 0) || (m_aoiRadius == 0))
   {
      ossimNotify(ossimNotifyLevel_WARN) << "ossimHLZUtil::initialize() ERROR: GSD and/or AOI "
            "radius have not been set." << std::endl;
      return false;
   }

   // If needed, compute the bounding rect in pixel space given the visibility range and the GSD:
   if (m_viewRect.area() <= 1)
   {
      m_viewRect.set_lrx(2.0 * ossim::round<ossim_int32, double>(m_aoiRadius / m_gsd) - 1);
      m_viewRect.set_lry(m_viewRect.lr().x);
   }

   // Establish the output image geometry's map projection if not already done:
   ossimRefPtr<ossimMapProjection> mapProj = m_productGeom->getAsMapProjection();
   if (!mapProj.valid())
   {
      mapProj = new ossimEquDistCylProjection();
      m_productGeom->setProjection(mapProj.get());
   }

   mapProj->setOrigin(m_destinationGpt);
   mapProj->setMetersPerPixel(ossimDpt(m_gsd, m_gsd));
   ossimDpt degPerPixel(mapProj->getDecimalDegreesPerPixel());
   mapProj->setElevationLookupFlag(false);
   ossimGpt ulTiePt(m_destinationGpt);
   ossimIpt image_size(m_viewRect.width(), m_viewRect.height());
   ossimDpt offset (-image_size.x / 2.0, -image_size.y / 2.0);
   ulTiePt.lat -= degPerPixel.lat * offset.y;
   ulTiePt.lon += degPerPixel.lon * offset.x;
   mapProj->setUlTiePoints(ulTiePt);

   // Need a transform so that we can use the observer point as the output image origin (0,0):
   m_productGeom->setImageSize(image_size);

   // Establish the ground rect:
   ossimGpt ul, ur, lr, ll;
   m_productGeom->getBoundingGroundRect(m_gndRect);

   // If PC provided as file on command line, Load it. This uses the output ground rect so needs to
   // be after the initialization of m_geometry:
   if (!m_pcFile.empty() && !loadPcFile())
      return false;

   // If threat-domes spec (or any mask) provided as file on command line, Load it:
   if (!m_maskFile.empty() && !loadMaskFiles())
      return false;

   // Allocate the output image buffer:
   m_outBuffer = ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, m_viewRect.width(),
                                                           m_viewRect.height());
   if (!m_outBuffer.valid())
      return false;

   // Initialize the image with all points hidden:
   m_outBuffer->initialize();
   m_outBuffer->setImageRectangle(m_viewRect);
   m_outBuffer->fill(m_badLzValue);

   // Establish connection to DEM posts directly as raster "images" versus using the OSSIM elev
   // manager that performs interpolation of DEM posts for arbitrary locations. These elev images
   // feed into a combiner in order to have a common tap for elev pixels:
   if (!initProcessingChain())
      return false;

   if (!initHlzFilter())
      return false;

   if (m_outputSummary)
      dumpProductSummary();

   return true;
}

void ossimHLZUtil::setProductGSD(const double& meters_per_pixel)
{
   m_gsd = meters_per_pixel;

   if (m_productGeom.valid())
   {
      ossimMapProjection* map_proj =
            dynamic_cast<ossimMapProjection*>(m_productGeom->getProjection());
      if (map_proj)
         map_proj->setMetersPerPixel(ossimDpt(m_gsd, m_gsd));
   }
}

bool ossimHLZUtil::loadDemFile()
{
   ossimElevManager* elevMgr = ossimElevManager::instance();
   elevMgr->clear();

   ossimRefPtr<ossimImageElevationDatabase> ied = new ossimImageElevationDatabase;
   if (!ied->open(m_demFile))
   {
      ossimNotify(ossimNotifyLevel_WARN)
               << "ossimHLZUtil::initialize ERR: Cannot open DEM file at <" << m_demFile << ">\n"
               << std::endl;
      return false;
   }
   elevMgr->addDatabase(ied.get());

   // When a dem file is provided, certain parameters can be implied versus explicitely provided
   // in the command-line arguments:
   ossimRefPtr<ossimImageHandler> dem_handler =
         ossimImageHandlerRegistry::instance()->open(m_demFile, true, false);
   if (!dem_handler.valid())
   {
      ossimNotify(ossimNotifyLevel_WARN)
               << "ossimHLZUtil::initialize ERR: Cannot open DEM file <" << m_demFile << "> "
               "as image handler.\n" << std::endl;
      return false;
   }

   // Create the processing chain used for this elevation single image source:
   createInputChain(dem_handler, m_combinedElevSource);
   ossimRefPtr<ossimImageGeometry> dem_geom = dem_handler->getImageGeometry();
   if (dem_geom == 0)
   {
      ossimNotify(ossimNotifyLevel_WARN)
               << "ossimHLZUtil::loadDemFile() ERR: DEM file <" << m_demFile << "> "
               "does not have valid geometry.\n" << std::endl;
      return false;
   }

   // Match the DEM's projection:
   //m_productGeom = new ossimImageGeometry(*dem_geom);
   dem_geom->getBoundingGroundRect(m_gndRect);
   if (m_destinationGpt.isLatLonNan())
   {
      m_destinationGpt = m_gndRect.midPoint();
   }
   if (m_gsd == 0)
   {
      ossimDpt gsdPt (dem_geom->getMetersPerPixel());
      m_gsd = 0.5 * (gsdPt.x + gsdPt.y);
   }
   if (m_aoiRadius == 0)
   {
      ossimDpt mtrsPerDeg (m_destinationGpt.metersPerDegree());
      m_aoiRadius = 0.25 * (m_gndRect.width()*mtrsPerDeg.x + m_gndRect.height()*mtrsPerDeg.y);
   }

   return true;
}

void ossimHLZUtil::createInputChain(ossimRefPtr<ossimImageHandler>& handler,
                                    ossimRefPtr<ossimImageSource>& chain)
{
   ossimRefPtr<ossimImageViewProjectionTransform> ivt = new ossimImageViewProjectionTransform;
   ivt->setImageGeometry(handler->getImageGeometry().get());
   ivt->setViewGeometry(m_productGeom.get());

   chain = new ossimImageRenderer(handler.get(),ivt.get());
   chain->initialize();
}


bool ossimHLZUtil::initProcessingChain()
{
   // If a DEM file was not provided as an argument, the elev sources array needs be initialized:
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
            return false;
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

      if (!m_slopeFile.empty())
         writeSlopeImage();
   }

   return true;
}

bool ossimHLZUtil::loadPcFile()
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

bool ossimHLZUtil::loadMaskFiles()
{
   // First check if the filename specified is an image file:
   MaskSource mask_image (this, m_maskFile);
   if (mask_image.valid)
   {
      m_maskSources.push_back(mask_image);
      return true;
   }

   ossimKeywordlist mask_kwl (m_maskFile);
   ossim_uint32 index = 0;
   while (true)
   {
      MaskSource mask_source (this, mask_kwl, index);
      if (mask_source.valid)
         m_maskSources.push_back(mask_source);
      else if (index > 0)
         break;
      ++index;
   }

   return true;
}

bool ossimHLZUtil::initHlzFilter()
{
   if ((m_hlzMinRadius == 0) || !m_combinedElevSource.valid())
      return false;

   // Determine number of posts (in one dimension) needed to cover the specified LZ radius:
   m_demGsd = m_productGeom->getMetersPerPixel();
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
   m_productGeom->getBoundingGroundRect(demGndRect);
   if (m_gndRect.hasNans())
      m_gndRect = demGndRect;
   else
      m_gndRect = m_gndRect.clipToRect(demGndRect);

   // Convert (clipped) requested rect to raster coordinate space in the DEM file:
   ossimDpt ulp, lrp;
   m_productGeom->worldToLocal(m_gndRect.ul(), ulp);
   m_productGeom->worldToLocal(m_gndRect.lr(), lrp);
   m_demRect.set_ul(ulp);
   m_demRect.set_lr(lrp);

   // To help with multithreading, just load entire AOI of DEM into memory:
   m_demBuffer = m_combinedElevSource->getTile(m_demRect);
   if (!m_demBuffer.valid())
      return false;

   return true;
}

bool ossimHLZUtil::execute()
{
   if (!m_productGeom.valid() && !initialize())
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
            ossimHLZUtil::ChipProcessorJob* job = 0;
            if (m_useLsFitMethod)
               job = new ossimHLZUtil::LsFitChipProcessorJob(this, chip_origin, chipId++);
            else
               job = new ossimHLZUtil::NormChipProcessorJob(this, chip_origin, chipId++);
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
            ossimHLZUtil::ChipProcessorJob* job = 0;
            if (m_useLsFitMethod)
               job = new ossimHLZUtil::LsFitChipProcessorJob(this, chip_origin, chipId++);
            else
               job = new ossimHLZUtil::NormChipProcessorJob(this, chip_origin, chipId++);
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

void ossimHLZUtil::paintReticle()
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

bool ossimHLZUtil::writeFile()
{
   ossimIrect rect(0, 0, m_viewRect.width() - 1, m_viewRect.height() - 1);
   m_outBuffer->setImageRectangle(rect);

   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(m_outBuffer);
   memSource->setImageGeometry(m_productGeom.get());
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
         ossimNotify(ossimNotifyLevel_WARN) << "ossimHLZUtil::writeFile() ERROR: The LUT "
               "file <"
               << m_lutFile
               << "> could not be read. Ignoring remap request.\n"
               << std::endl;
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
   if (m_productFile.ext().contains("tif"))
   {
      ossimTiffWriter* tif_writer = new ossimTiffWriter();
      tif_writer->setGeotiffFlag(true);
      tif_writer->setFilename(m_productFile);
      writer = tif_writer;
   }
   else
   {
      writer = ossimImageWriterFactoryRegistry::instance()->createWriter(m_productFile);
   }
   bool success = false;
   if (writer.valid())
   {
      writer->connectMyInputTo(0, last_source);
      success = writer->execute();
   }

   return success;
}

void ossimHLZUtil::writeSlopeImage()
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

void ossimHLZUtil::dumpProductSummary() const
{
   ossimIpt isize(m_productGeom->getImageSize());
   cout << "\nSummary of HLZ product image:" << "\n   Output file name: " << m_productFile
         << "\n   Image size: " << isize
         << "\n   Slope threshold: " << m_slopeThreshold << " deg"
         << "\n   Roughness threshold: " << m_roughnessThreshold << " m"
         << "\n   Min LZ radius: " << m_hlzMinRadius << " m"
         << "\n   product GSD: " << m_gsd << " m"
         << "\n   radius-of-interest: " << (int) (m_gsd * isize.x / 2.0) << " m"
         << "\n   Scalar type: " << m_outBuffer->getScalarTypeAsString() << endl;
}

OpenThreads::ReadWriteMutex ossimHLZUtil::ChipProcessorJob::m_bufMutex;

ossimHLZUtil::ChipProcessorJob::ChipProcessorJob(ossimHLZUtil* hlzUtil, const ossimIpt& origin,
                                   ossim_uint32 /*chip_id*/)
: m_hlzUtil (hlzUtil),
  m_demChipUL (origin),
  m_status (0),
  m_nullValue (hlzUtil->m_demBuffer->getNullPix(0))
{
   m_demChipLR.x = m_demChipUL.x + m_hlzUtil->m_demFilterSize.x;
   m_demChipLR.y = m_demChipUL.y + m_hlzUtil->m_demFilterSize.y;
}

void ossimHLZUtil::ChipProcessorJob::start()
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

bool ossimHLZUtil::ChipProcessorJob::level2Test()
{
   // Level 2 only valid if a point cloud dataset is available:
   if (m_hlzUtil->m_pcSources.empty())
   {
      ++m_status; // assumes level2 passes
      return true;
   }

   // Need to convert DEM file coordinate bounds to geographic.
   ossimGpt chipUlGpt, chipLrGpt;
   m_hlzUtil->m_productGeom->localToWorld(ossimDpt(m_demChipUL), chipUlGpt);
   m_hlzUtil->m_productGeom->localToWorld(ossimDpt(m_demChipLR), chipLrGpt);
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


bool ossimHLZUtil::ChipProcessorJob::maskTest()
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


bool ossimHLZUtil::LsFitChipProcessorJob::level1Test()
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

bool ossimHLZUtil::NormChipProcessorJob::level1Test()
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

void ossimHLZUtil::printApiJson(ostream& out) const
{
   ossimFilename json_path (ossimPreferences::instance()->findPreference("ossim_share_directory"));
   json_path += "/ossim/util/ossimHlzApi.json";
   if (json_path.isReadable())
   {
      char line[256];
      ifstream ifs (json_path.chars());
      ifs.getline(line, 256);

       while (ifs.good())
       {
         out << line << endl;
         ifs.getline(line, 256);
       }

       ifs.close();
   }
}

ossimHLZUtil::MaskSource::MaskSource(ossimHLZUtil* hlzUtil, ossimKeywordlist& kwl,
                                     ossim_uint32 index)
:  exclude (true),
   valid (false)
{
   ossimString prefix (MASK_PREFIX);
   prefix += ossimString::toString(index) + ".";

   ossimFilename maskFile = kwl.find(prefix.chars(), ossimKeywordNames::FILENAME_KW);
   if (maskFile.empty())
      return;

   ossimRefPtr<ossimImageHandler> handler = ossimImageHandlerRegistry::instance()->open(maskFile);
   if (!handler.valid())
   {
      ossimNotify(ossimNotifyLevel_WARN) <<
            "ossimHLZUtil::MaskSource -- Error encountered instantiating mask image <"
            <<maskFile<<">. Ignoring mask source." << endl;
      return;
   }

   kwl.getBoolKeywordValue(exclude, MASK_EXCLUDE_KW, prefix.chars());

   hlzUtil->createInputChain(handler, image);
   valid = true;
};



ossimHLZUtil::MaskSource::MaskSource(ossimHLZUtil* hlzUtil, const ossimFilename& mask_image)
:  exclude (true),
   valid (false)
{
   ossimRefPtr<ossimImageHandler> handler = ossimImageHandlerRegistry::instance()->open(mask_image);
   if (handler.valid())
   {
      hlzUtil->createInputChain(handler, image);
      valid = true;
   }
};

















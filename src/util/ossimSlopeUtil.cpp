//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimSlopeUtil.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/imaging/ossimSlopeFilter.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimImageMosaic.h>
#include <iostream>

using namespace std;

const char* CENTER_KW = "center_lat_lon";
const char* DEM_KW    = "dem_file"; // Also accepting ossimKeywordNames::ELEVATION_CELL_KW
const char* REMAP_KW  = "remap_to_8bit";
const char* LUT_KW    = "lut_file";
const char* ROI_KW    = "roi_radius";

const char* ossimSlopeUtil::DESCRIPTION =
      "Computes raster image with pixels representing the angle between the local surface"
      " normal and the local vertical given a DEM.";

ossimSlopeUtil::ossimSlopeUtil()
:  m_aoiRadius(0),
   m_remapToByte(false)
{
   m_centerGpt.makeNan();
}

ossimSlopeUtil::~ossimSlopeUtil()
{
}

void ossimSlopeUtil::setUsage(ossimArgumentParser& ap)
{
   // Add options.
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " [options] <output-image>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->addCommandLineOption(
         "--center <lat> <lon>",
         "The center position of the output product. Required if no input DEM is specified.");
   au->addCommandLineOption(
         "--dem <filename>",
         "Specifies the input DEM filename. If none provided, the elevation database is referenced "
         "as specified in prefs file for the center and ROI specified.");
   au->addCommandLineOption(
         "--remap",
         "The range of slope angle (0.0 to 90.0) is remapped to 0-255 (one byte/pixel)");
   au->addCommandLineOption(
         "--lut <filename>",
         "Specifies the optional lookup table filename for mapping the single-band output "
         "image to an RGB. The LUT provided must be in the ossimIndexToRgbLutFilter format "
         "and should accomodate the output pixel range. This option forces remap to 8-bit, "
         "0-255 where 255 = 90 deg slope");
   au->addCommandLineOption(
         "--roi <meters>",
         "radius of interest surrounding the center point. If absent, the product defaults to "
         "1024 x 1024 pixels, with a radius of 512 * GSD. Alternatively, if a DEM file is "
         "specified, the product ROI defaults to the full DEM coverage.");

   ossimString description =
         "Utility for computing the slope at each elevation post and generating "
         "a corresponding slope image. The output scalar type is a normalized float with 1.0 = 90 "
         "degree angle from the local vertical. Optional 8-bit scalar type is available."
         "Examples:\n\n"
         "    ossim-slope [options] --dem <input-dem> <output-slope-image-file>\n"
         "    ossim-slope [options] --center <lat> <lon> --roi <meters> <output-slope-image-file>\n";
   au->setDescription(description);

   // Base class has its own:
   ossimUtility::setUsage(ap);
}

bool ossimSlopeUtil::initialize(ossimArgumentParser& ap)
{
   // Base class first:
   if (!ossimUtility::initialize(ap))
      return false;

   std::string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);
   std::string ts2;
   ossimArgumentParser::ossimParameter sp2(ts2);

   if (ap.read("--center", sp1, sp2) || ap.read("--center-lat-lon", sp1, sp2))
   {
      m_centerGpt.lat = ossimString(ts1).toDouble();
      m_centerGpt.lon = ossimString(ts2).toDouble();
      m_centerGpt.hgt = 0.0;
   }

   if (ap.read("--dem", sp1) || ap.read("--dem-file", sp1))
      m_demFile = ts1;

   if ( ap.read("--remap") || ap.read("--remap-to-8bit"))
   {
      m_remapToByte = true;
   }

   if ( ap.read("--lut", sp1) || ap.read("--lut-file", sp1))
      m_lutFile = ts1;

   if (ap.read("--roi", sp1) || ap.read("--roi-radius", sp1))
      m_aoiRadius = ossimString(ts1).toDouble();

   if (m_demFile.empty() && m_centerGpt.hasNans())
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"No DEM file nor center point provided. Cannot "
            <<"compute slope image."<<endl;
      setUsage(ap);
      return false;
   }

   // There should only be the required command line args left:
   if (ap.argc() != 2)
   {
      setUsage(ap);
      return false;
   }

   m_slopeFile = ap[1];

   return initializeChain();
}

bool ossimSlopeUtil::initialize(const ossimKeywordlist& kwl)
{
   clear();

   // Base class first:
   if (!ossimUtility::initialize(kwl))
      return false;

   ossimString value;
   value = kwl.find(CENTER_KW);
   if (!value.empty())
   {
      vector <ossimString> coordstr;
      value.split(coordstr, ossimString(" ,"), false);
      if (coordstr.size() == 2)
      {
         m_centerGpt.lat = coordstr[0].toDouble();
         m_centerGpt.lon = coordstr[1].toDouble();
         m_centerGpt.hgt = 0.0;
      }
   }

   m_demFile = kwl.find(DEM_KW);
   if (m_demFile.empty())
      m_demFile = kwl.find(ossimKeywordNames::ELEVATION_CELL_KW);

   kwl.getBoolKeywordValue(m_remapToByte, REMAP_KW);

   m_lutFile = kwl.find(LUT_KW);

   value = kwl.find(ROI_KW);
   if (!value.empty())
      m_aoiRadius = value.toDouble();

   m_slopeFile = kwl.find(ossimKeywordNames::OUTPUT_FILE_KW);
   if (value.empty())
   {
      ostringstream msg;
      msg <<"No output slope file provided."<<ends;
      ossimException e (msg.str());
      throw e;
   }

   if (m_demFile.empty() && m_centerGpt.hasNans())
   {
      ostringstream msg;
      msg <<"No center point provided and no DEM file provided. Cannot compute slope image."<<ends;
      ossimException e (msg.str());
      throw e;
   }

   return initializeChain();
}

void ossimSlopeUtil::clear()
{
   m_centerGpt.makeNan();
   m_demFile.clear();
   m_lutFile.clear();
   m_aoiRadius = 0;
   m_procChain = 0;
   m_slopeFile.clear();
   m_remapToByte = false;
}

bool ossimSlopeUtil::initializeChain()
{
   // Establish connection to elevation data. Image handler (or combiner) returned in m_procChain:
   if (!m_demFile.empty())
   {
      if (!loadDemFile())
      return false;
   }
   else if (!loadElevDb())
      return false;

   ossimRefPtr<ossimSlopeFilter> slope_filter = new ossimSlopeFilter(m_procChain.get());
   slope_filter->setSlopeType(ossimSlopeFilter::NORMALIZED);
   m_procChain = slope_filter.get();

   // If remap to one byte per pixel selected, insert remapper here:
   if (m_remapToByte || !m_lutFile.empty())
   {
      ossimRefPtr<ossimScalarRemapper> sr = new ossimScalarRemapper;
      sr->connectMyInputTo(0, m_procChain.get());
      m_procChain = sr.get();
      sr->setOutputScalarType(OSSIM_UINT8);
   }

   // If LUT remap requested, insert here in the chain:
   if (!m_lutFile.empty())
   {
      if (m_lutFile.isReadable())
      {
         ossimRefPtr<ossimIndexToRgbLutFilter> lut = new ossimIndexToRgbLutFilter;
         lut->connectMyInputTo(0, m_procChain.get());
         m_procChain = lut.get();
         lut->setLut(m_lutFile);
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)<<"The LUT file specified, <"<<m_lutFile<<"> is "
               "not readbale. The LUT remap will be ignored."<<endl;
      }
   }

   m_procChain->initialize();
   return true;
}

bool ossimSlopeUtil::loadDemFile()
{
   m_procChain = ossimImageHandlerRegistry::instance()->open(m_demFile, true, false);
   if (!m_procChain.valid())
   {
      ossimNotify(ossimNotifyLevel_FATAL) << "Could not open DEM file at <"<<m_demFile
            <<">. Aborting..."<<endl;
      return false;
   }

   return true;
}

bool ossimSlopeUtil::loadElevDb()
{
   // Determine if default GSD needs to be computed. Query for target H so as to autoload cell:
   ossimElevManager* elevMgr = ossimElevManager::instance();
   elevMgr->getHeightAboveEllipsoid(m_centerGpt);
   double gsd = elevMgr->getMeanSpacingMeters();
   if (ossim::isnan(gsd))
   {
      ossimNotify(ossimNotifyLevel_FATAL) << "Could not establish DEM GSD at center point "
            <<m_centerGpt<<". Verify that the elevation database provides coverage at this "
            <<"location."<<endl;
      return false;
   }

   // Establish output radius if not provided:
   ossimIrect viewRect(0, 0, 1023, 1023);
   if (m_aoiRadius == 0)
   {
      // The radius of interest can default given a GSD to achieve a specific output image size.
      m_aoiRadius = (viewRect.size().x + viewRect.size().y -2) * gsd / 4.0;
   }

   // Establish ground-space AOI rectangle:
   ossimDpt metersPerDeg (m_centerGpt.metersPerDegree());
   double dlat = m_aoiRadius/metersPerDeg.y;
   double dlon = m_aoiRadius/metersPerDeg.x;
   ossimGrect gndRect (m_centerGpt.lat + dlat, m_centerGpt.lon - dlon,
                       m_centerGpt.lat - dlat, m_centerGpt.lon + dlon);

   // Query elevation manager for cells providing needed coverage:
   std::vector<std::string> cells;
   elevMgr->getCellsForBounds(gndRect, cells);

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

      elevChains.push_back(dem.get());
      ++fname_iter;
   }

   // Establish the output image's projection geometry:
   ossimRefPtr<ossimEquDistCylProjection> mapProj = new ossimEquDistCylProjection();
   mapProj->setOrigin(m_centerGpt);
   mapProj->setMetersPerPixel(ossimDpt(gsd, gsd));
   ossimDpt degPerPixel(mapProj->getDecimalDegreesPerPixel());
   mapProj->setElevationLookupFlag(false);
   mapProj->setUlTiePoints(gndRect.ul());
   ossimRefPtr<ossimImageGeometry> productGeom = new ossimImageGeometry(0, mapProj.get());
   ossimDpt viewPt;
   productGeom->worldToLocal(gndRect.ul(), viewPt);
   viewRect.set_ulx(ossim::round<ossim_int32, double>(viewPt.x));
   viewRect.set_uly(ossim::round<ossim_int32, double>(viewPt.y));
   productGeom->worldToLocal(gndRect.lr(), viewPt);
   viewRect.set_lrx(ossim::round<ossim_int32, double>(viewPt.x));
   viewRect.set_lry(ossim::round<ossim_int32, double>(viewPt.y));
   ossimIpt image_size(viewRect.width(), viewRect.height());
   productGeom->setImageSize(image_size);

   // Now loop to add a renderer to each input cell to insure common output projection:
   ossimConnectableObject::ConnectableObjectList::iterator cell_iter = elevChains.begin();
   while (cell_iter != elevChains.end())
   {
      ossimImageSource* chain = (ossimImageSource*) cell_iter->get();
      ossimRefPtr<ossimImageViewProjectionTransform> ivt = new ossimImageViewProjectionTransform(
            chain->getImageGeometry().get(), productGeom.get());

      chain = new ossimImageRenderer(chain, ivt.get());
      chain->initialize();
      *cell_iter = chain;
      ++cell_iter;
   }

   // Finally create the combiner:
   m_procChain = new ossimImageMosaic(elevChains);
   return true;
}

bool ossimSlopeUtil::execute()
{
   if (!m_procChain.valid())
   {
      if (!initializeChain())
         return false;
   }

   // Set up the writer:
   bool all_good = false;

   ossimRefPtr<ossimTiffWriter> tif_writer =  new ossimTiffWriter();
   tif_writer->setGeotiffFlag(true);

   tif_writer->setFilename(m_slopeFile);
   if (tif_writer.valid())
   {
      tif_writer->connectMyInputTo(0, m_procChain.get());
      ossimIrect viewRect;
      m_procChain->getImageGeometry()->getBoundingRect(viewRect);
      tif_writer->setAreaOfInterest(viewRect);

      all_good = tif_writer->execute();
      if (all_good)
         ossimNotify(ossimNotifyLevel_INFO)<<"Output written to <"<<m_slopeFile<<">"<<endl;
      else
      {
         ossimNotify(ossimNotifyLevel_FATAL)<<"Error encountered writing out slope image to <"
               <<m_slopeFile<<">."<<endl;
      }
   }

   return all_good;
}



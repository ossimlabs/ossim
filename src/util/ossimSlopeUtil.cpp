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
:  m_remapToByte(false)
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
         "--remap",
         "The range of slope angle (0.0 to 90.0) is remapped to 0-255 (one byte/pixel)");
   ossimString description =
         "Utility for computing the slope at each elevation post and generating "
         "a corresponding slope image. The output scalar type is a normalized float with 1.0 = 90 "
         "degree angle from the local vertical. Optional 8-bit scalar type is available."
         "Examples:\n\n"
         "    ossim-slope [options] --dem <input-dem> <output-slope-image-file>\n"
         "    ossim-slope [options] --center <lat> <lon> --roi <meters> <output-slope-image-file>\n";
   au->setDescription(description);

   // Base class has its own:
   ossimChipProcUtil::setUsage(ap);
}

void ossimSlopeUtil::initialize(ossimArgumentParser& ap)
{
   // Base class first:
   ossimChipProcUtil::initialize(ap);

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

   if ( ap.read("--remap") || ap.read("--remap-to-8bit"))
   {
      m_remapToByte = true;
   }

   processRemainingArgs(ap);
}

void ossimSlopeUtil::initialize(const ossimKeywordlist& kwl)
{
   // This method
   clear();

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

   kwl.getBoolKeywordValue(m_remapToByte, REMAP_KW);

   // Base class does the heavy work:
   ossimChipProcUtil::initialize(kwl);
}

void ossimSlopeUtil::initProcessingChain()
{
   std::ostringstream errMsg;

   if (!loadElevDb())
   {
      errMsg << "Must supply an output file."<<std::endl;
      throw ossimException(errMsg.str());
   }

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
         errMsg << "The LUT file specified, <"<<m_lutFile<<"> is "
               "not readable. The LUT remap will be ignored."<<std::endl;
         throw ossimException(errMsg.str());
      }
   }

   m_procChain->initialize();
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
   assignAoiViewRect();
   if (m_aoiViewRect.hasNans())
   {
      m_aoiViewRect = ossimIrect(0, 0, 1023, 1023);
   }
   double aoiRadius = (m_aoiViewRect.size().x + m_aoiViewRect.size().y -2) * gsd / 4.0;

   // Establish ground-space AOI rectangle:
   ossimDpt metersPerDeg (m_centerGpt.metersPerDegree());
   double dlat = aoiRadius/metersPerDeg.y;
   double dlon = aoiRadius/metersPerDeg.x;
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
   m_aoiViewRect.set_ulx(ossim::round<ossim_int32, double>(viewPt.x));
   m_aoiViewRect.set_uly(ossim::round<ossim_int32, double>(viewPt.y));
   productGeom->worldToLocal(gndRect.lr(), viewPt);
   m_aoiViewRect.set_lrx(ossim::round<ossim_int32, double>(viewPt.x));
   m_aoiViewRect.set_lry(ossim::round<ossim_int32, double>(viewPt.y));
   ossimIpt image_size(m_aoiViewRect.width(), m_aoiViewRect.height());
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



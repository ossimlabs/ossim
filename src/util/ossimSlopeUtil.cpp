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

static const std::string IMAGE_SOURCE_KW = "image_source";
static const std::string ELEV_SOURCE_KW  = "elev_source";
const char* ossimSlopeUtil::DESCRIPTION  =
      "Utility for computing the slope at each elevation post and generating "
      "a corresponding slope image.";

ossimSlopeUtil::ossimSlopeUtil()
: m_recursiveCall (false)
{
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
<<<<<<< HEAD
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
         "and should accommodate the output pixel range. This option forces remap to 8-bit, "
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
=======
   au->setDescription(DESCRIPTION);
>>>>>>> dev

   // Base class has its own:
   ossimChipProcUtil::setUsage(ap);
}

bool ossimSlopeUtil::initialize(ossimArgumentParser& ap)
{
   if (!ossimChipProcUtil::initialize(ap))
      return false;

   processRemainingArgs(ap);
   return true;
}

void ossimSlopeUtil::initialize(const ossimKeywordlist& kwl)
{
   // This method
   clear();

   // Base class does the heavy work:
   ossimChipProcUtil::initialize(kwl);
}

void ossimSlopeUtil::initProcessingChain()
{
   ostringstream key;

   // Reinitialization needs to skip this method if called recursively:
   if (m_recursiveCall)
      return;

   // Need to establish list of DEM cells given either the bounding rect or explicit DEM cells:
   if (m_demSources.empty())
   {
      // Query elevation manager for cells providing needed coverage:
      std::vector<ossimFilename> cells;
      ossimElevManager::instance()->getCellsForBounds(m_aoiGroundRect, cells);

      // Insert the list of DEM cells into the KWL as input images:
      ossimConnectableObject::ConnectableObjectList elevChains;
      for(ossim_uint32 idx=0; idx<cells.size(); ++idx)
      {
         // Add the DEM as an image source to the KWL:
         ostringstream key;
         key<<IMAGE_SOURCE_KW<<ossimString::toString(idx)<<"."<<ossimKeywordNames::FILE_KW;
         m_kwl.addPair(key.str(), cells[idx].string() );
      }
   }
   else
   {
      // DEMs explicitly listed in keywordlist, use these:
      for(ossim_uint32 idx=0; idx<m_demSources.size(); ++idx)
      {
         // Add the DEM as an image source to the KWL:
         ostringstream key;
         key<<IMAGE_SOURCE_KW<<ossimString::toString(idx)<<"."<<ossimKeywordNames::FILE_KW;
         m_kwl.addPair(key.str(), m_demSources[idx].string() );
      }
   }

   // Remove any occurence of elev_source in the KWL:
   ossimString regex = ELEV_SOURCE_KW + ".*";
   m_kwl.removeKeysThatMatch(regex);

   // Reinitialize the object with the DEMs listed as input images and create a mosaic of them:
   m_recursiveCall = true;
   initialize(m_kwl);
   m_recursiveCall = false;
   ossimRefPtr<ossimImageSource> combiner = combineLayers(m_imgLayers);
   m_procChain->add(combiner.get());

   // Finally add the slope filter:
   ossimRefPtr<ossimSlopeFilter> slope_filter = new ossimSlopeFilter;
   slope_filter->setSlopeType(ossimSlopeFilter::NORMALIZED);
   m_procChain->add(slope_filter.get());

   finalizeChain();
}


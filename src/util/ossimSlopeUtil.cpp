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
   au->setDescription(DESCRIPTION);

   // Base class has its own:
   ossimChipProcUtil::setUsage(ap);
}

void ossimSlopeUtil::initialize(ossimArgumentParser& ap)
{
   ossimChipProcUtil::initialize(ap);
   processRemainingArgs(ap);
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


//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

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
#include <ossim/util/ossimOrthoTool.h>
#include <iostream>

using namespace std;

const char* ossimOrthoTool::DESCRIPTION  = "Utility for orthorectifying and reprojecting image data.";

ossimOrthoTool::ossimOrthoTool()
{
}

ossimOrthoTool::~ossimOrthoTool()
{
}

void ossimOrthoTool::setUsage(ossimArgumentParser& ap)
{
   // Add options.
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " ortho [options] [<input-image>] <output-image>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->setDescription(DESCRIPTION);

   // Base class has its own:
   ossimChipProcTool::setUsage(ap);
}

bool ossimOrthoTool::initialize(ossimArgumentParser& ap)
{
   if (!ossimChipProcTool::initialize(ap))
      return false;
   if (m_helpRequested)
      return true;

   processRemainingArgs(ap);
   return true;
}

void ossimOrthoTool::initialize(const ossimKeywordlist& kwl)
{
   // Base class does the heavy work:
   ossimChipProcTool::initialize(kwl);
}

void ossimOrthoTool::initProcessingChain()
{
   ossimRefPtr<ossimImageSource> input_mosaic = combineLayers(m_imgLayers);
   m_procChain->add(input_mosaic.get());

   finalizeChain();
}


//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
//*******************************************************************
//  $Id$

#include <ossim/util/ossimPointCloudTool.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/elevation/ossimImageElevationDatabase.h>
#include <ossim/point_cloud/ossimPointCloudHandlerRegistry.h>

using namespace std;

const char* ossimPointCloudTool::DESCRIPTION =
   "Utility class for generating point-cloud-derived image products.";

ossimPointCloudTool::ossimPointCloudTool()
:  m_operation (LOWEST_DEM),
   m_gsd (0)
{
}

ossimPointCloudTool::~ossimPointCloudTool()
{
   m_pcHandler = 0;
   m_prodGeom = 0;
   m_pcuFilter = 0;
}

void ossimPointCloudTool::addArguments(ossimArgumentParser& ap)
{
   // Set the general usage:
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " [options] <point-cloud-file> <output-image>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->addCommandLineOption(
         "--gsd <meters>",
         "Specifies output GSD in meters. Defaults to the same "
         "resolution as input DEM.");
   au->addCommandLineOption(
         "--dem <filename>",
         "Specifies the input DEM filename. If none provided, the elevation database is "
         "referenced as specified in prefs file");
   au->addCommandLineOption(
         "--lut <filename>",
         "Specifies the optional lookup table filename for "
         "mapping the single-band output image to an RGB. The LUT provided must be "
         "in the ossimIndexToRgbLutFilter format and must handle the three output "
         "viewshed values (see --values option).");
   au->addCommandLineOption(
         "--method",
         "Specify the desired operation. Possible values are:\n"
         "  \"highest-dem\", \"lowest-dem\" (default), or \"highest-lowest\". \n"
         "Alternatively can be specified in shorthand as \"h-d\", \"l-d\", or \"h-l\", "
         "respectively.");
   au->addCommandLineOption(
         "--request-api",
         "Causes applications API to be output as JSON to stdout."
         " Accepts optional filename to store JSON output.");
   au->addCommandLineOption(
         "--threads <n>",
         "Number of threads. Defaults to use all available cores. "
         "For engineering/debug purposes ");
}

void ossimPointCloudTool::usage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimInit::instance()->addOptions(ap);

   // Add options.
   addArguments(ap);

   // Write usage.
   ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));

   ossimNotify(ossimNotifyLevel_INFO)
   <<"\nPerforms specified processing on point-cloud.\n\n"
   << std::endl;
}

bool ossimPointCloudTool::initialize(ossimArgumentParser& ap)
{
   if ( (ap.argc() == 1) || ap.read("-h") || ap.read("--help") )
   {
      usage(ap);
      return false;
   }

   ossimString ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);

   if ( ap.read("--dem", sp1) )
      m_demFile = ts1;

   if ( ap.read("--gsd", sp1) )
      m_gsd = ossimString(ts1).toDouble();

   if ( ap.read("--lut", sp1) )
      m_lutFile = ts1;

   if ( ap.read("--method", sp1) )
   {
      if (ts1.contains("highest-dem") || ts1.contains("h-d"))
         m_operation = HIGHEST_DEM;
      else if (ts1.contains("highest-lowest") || ts1.contains("h-l"))
         m_operation = HIGHEST_LOWEST;
   }

   if ( ap.read("--pc", sp1) )
      m_demFile = ts1;

/*
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
*/
   // There should only be the required command line args left:
   if (ap.argc() < 3)
   {
      usage(ap);
      return false;
   }

   // Parse the required command line params:
   m_pcFile = ap[1];
   m_prodFile = ap[2];

   return initialize();
}

void ossimPointCloudTool::loadJSON(const Json::Value &json_request)
{

}

bool ossimPointCloudTool::initialize()
{
   if (loadPC())
   {
      ossimNotify(ossimNotifyLevel_WARN)
              << "ossimPointCloudTool::initialize ERR: Cannot open PC file at <"<<m_pcFile
              <<">\n"<< endl;
      return false;
   }

   if (!m_demFile.empty() && !loadDem())
      return false;

   m_pcuFilter = new ossimPointCloudUtilityFilter(this);
   return true;
}

bool ossimPointCloudTool::loadPC()
{
   // DEM provided as file on command line, reset the elev manager to use only this:
   m_pcHandler = ossimPointCloudHandlerRegistry::instance()->open(m_pcFile);
   if(!m_pcHandler.valid())
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "ossimPointCloudTool::initialize ERR: Cannot open PC file at <"<<m_pcFile
            <<">\n" << std::endl;
      return false;
   }

   // Use "rasterized" PC to establish best output image geometry:
   m_pciHandler = new ossimPointCloudImageHandler;
   m_pciHandler->setPointCloudHandler(m_pcHandler.get());
   m_prodGeom = m_pciHandler->getImageGeometry();
   if (!m_prodGeom.valid() || !m_prodGeom->getAsMapProjection())
      return false;

   if (m_gsd != 0)
      m_prodGeom->getAsMapProjection()->setMetersPerPixel(ossimDpt(m_gsd, m_gsd));

   return true;
}

bool ossimPointCloudTool::loadDem()
{
   // DEM provided as file on command line, reset the elev manager to use only this:
   ossimElevManager* elevMgr = ossimElevManager::instance();
   elevMgr->clear();
   ossimRefPtr<ossimImageElevationDatabase> ied = new ossimImageElevationDatabase;
   if(!ied->open(m_demFile))
   {
      ossimNotify(ossimNotifyLevel_WARN)
                           << "ossimViewshedUtil::initialize ERR: Cannot open DEM file at <"<<m_demFile<<">\n"
                           << std::endl;
      return false;
   }

   elevMgr->addDatabase(ied.get());

   // When DEM file specified, need to turn off all defaulting to ellipsoid/geoid to make sure
   // only the DEM file data is processed:
   elevMgr->setDefaultHeightAboveEllipsoid(ossim::nan());
   elevMgr->setUseGeoidIfNullFlag(false);

   return true;
}

void ossimPointCloudTool::setGSD(const double& meters_per_pixel)
{
   if (m_prodGeom->getAsMapProjection() && (meters_per_pixel > 0))
   {
      m_gsd = meters_per_pixel;
      m_prodGeom->getAsMapProjection()->setMetersPerPixel(ossimDpt(m_gsd, m_gsd));
   }
}

bool ossimPointCloudTool::execute()
{
   // See if an LUT is requested:
   ossimImageSource* last_source = m_pcuFilter.get();
   ossimRefPtr<ossimIndexToRgbLutFilter> lutSource = 0;
   if (!m_lutFile.empty())
   {
      ossimKeywordlist lut_kwl;
      lut_kwl.addFile(m_lutFile);
      lutSource = new ossimIndexToRgbLutFilter;
      if (!lutSource->loadState(lut_kwl))
      {
         ossimNotify(ossimNotifyLevel_WARN) << "ossimPointCloudTool::writeFile() ERROR: The LUT "
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
   ossimRefPtr<ossimImageFileWriter> writer =
         ossimImageWriterFactoryRegistry::instance()->createWriter(m_prodFile);
   bool success = false;
   if (writer.valid())
   {
      writer->connectMyInputTo(0, last_source);
      success = writer->execute();
   }

   return success;
}


void ossimPointCloudTool::saveJSON(Json::Value &json_request) const
{

}


//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//     Author: oscar.kramer@maxar.com
//
//**************************************************************************************************

#include <ossim/util/ossimRemapTool.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimException.h>
#include <ossim/imaging/ossimBandMergeSource.h>
#include <ossim/imaging/ossimHistogramRemapper.h>
#include <ossim/base/ossimMultiResLevelHistogram.h>
#include <ossim/base/ossimMultiBandHistogram.h>
#include <ossim/imaging/ossimImageHistogramSource.h>
#include <iostream>
#include <ossim/imaging/ossimHistogramWriter.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/base/ossimAffineTransform.h>

using namespace std;

const char* ossimRemapTool::DESCRIPTION  =
   "Performs remap to 8-bit including optional histogram stretch and saves the corresponding external geometry file.";

ossimRemapTool::ossimRemapTool()
   :  m_doHistoStretch(true),
      m_entry(0)
{
   theStdOutProgress.setFlushStreamFlag(true);
}

ossimRemapTool::ossimRemapTool(const ossimFilename& inputFile,
                               int entryIndex,
                               bool doHistoStretch,
                               ossimFilename outputFile)
   :  m_inputFilename (inputFile),
      m_entry (entryIndex),
      m_doHistoStretch(doHistoStretch),
      m_gsd(-1.0)
{
   m_productFilename = outputFile;
   theStdOutProgress.setFlushStreamFlag(true);

   initProcessingChain();

   if (!execute())
      throw ossimException("Error encountered writing remap image.");
}

ossimRemapTool::~ossimRemapTool()
{
}

void ossimRemapTool::setUsage(ossimArgumentParser& ap)
{
   // Add options.
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " remap [options] <input-image> [<remap-out-image>]";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->setDescription(DESCRIPTION);

   // Base class has its own:
   ossimTool::setUsage(ap);

   au->addCommandLineOption("-e, --entry", "<entry> For multi image handlers which entry do you wish to extract. For list of entries use: \"ossim-info -i <your_image>\" ");
   au->addCommandLineOption("-n, --no-histo", "Optionally bypass histogram-stretch. ");
   au->addCommandLineOption("-g, --gsd", "Set the output resolution, in meters. Default is same as input resolution (no resampling).");
}

bool ossimRemapTool::initialize(ossimArgumentParser& ap)
{
   if (!ossimTool::initialize(ap))
      return false;
   if (m_helpRequested)
      return true;

   ossimString ts1;
   ossimArgumentParser::ossimParameter sp1 (ts1);

   if ( ap.read("-e", sp1) || ap.read("--entry", sp1) )
      m_entry = ts1.toUInt32();

   if ( ap.read("--no-histo") || ap.read("-n"))
      m_doHistoStretch = false;

   if ( ap.read("--gsd", sp1) || ap.read("-g", sp1))
      m_gsd = ts1.toDouble();

   // Determine input filename:
   if ( ap.argc() > 1 )
      m_inputFilename = ap[1];

   if (!m_inputFilename.isReadable())
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<"ossimRemapTool::initialize() Input filename <"
                                         <<m_inputFilename<<"> was not specified or is not "
                                         <<"readable. Try again.\n"<<endl;
      return false;
   }

   // Establish output filename:
   if ( ap.argc() > 2 )
   {
      m_productFilename = ap[2];
      cout<<m_inputFilename<<endl;
   }
   try
   {
      initProcessingChain();
   }
   catch (ossimException& xe)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<xe.what()<<endl;
      return false;
   }

   return true;
}

void ossimRemapTool::initProcessingChain()
{
   ostringstream errMsg ("ossimRemapTool::initProcessingChain() ERROR: ");

   ossimRefPtr<ossimImageHandler> handler =
      ossimImageHandlerRegistry::instance()->open(m_inputFilename);
   if (!handler)
   {
      errMsg<<"Could not open input image file at <"<<m_inputFilename<<">.";
      throw ossimException(errMsg.str());
   }
   if (m_entry)
      handler->setCurrentEntry(m_entry);
   m_procChain->add(handler.get());
   m_geom = handler->getImageGeometry();

   // Check for scaling:
   if (m_gsd > 0)
   {
      ossimDpt inGsd = m_geom->getMetersPerPixel();
      if (inGsd.x == 0 || inGsd.y == 0 || inGsd.hasNans())
      {
         errMsg<<"Input GSD = "<<inGsd<<" is not allowed! Cannot continue.";
         throw ossimException(errMsg.str());
      }
      ossimDpt scale (m_gsd / inGsd.x, m_gsd / inGsd.y);
      ossimRefPtr<ossimAffineTransform> scaler = new ossimAffineTransform();
      scaler->setScale(scale);
      m_geom->setTransform(scaler.get());
   }

   // Add histogram remapper if requested:
   if (m_doHistoStretch)
   {
      ossimRefPtr<ossimMultiResLevelHistogram> histogram = handler->getImageHistogram();
      if (!histogram)
      {
         // Need to create a histogram:
         ossimRefPtr<ossimImageHistogramSource> histoSource = new ossimImageHistogramSource;
         ossimRefPtr<ossimHistogramWriter> writer = new ossimHistogramWriter;
         histoSource->connectMyInputTo(0, handler.get());
         histoSource->enableSource();
         writer->connectMyInputTo(0, histoSource.get());
         ossimFilename histoFile;
         histoFile = handler->getFilenameWithThisExtension(ossimString("his"));
         writer->setFilename(histoFile);
         writer->addListener(&theStdOutProgress);
         writer->execute();
         histogram = handler->getImageHistogram();
         if (!histogram)
         {
            errMsg<<"Could not create histogram from <"<<histoFile<<">.";
            throw ossimException(errMsg.str());
         }
      }

      // Ready the histogram object in the processing chain:
      ossimRefPtr<ossimHistogramRemapper> histogramRemapper = new ossimHistogramRemapper();
      histogramRemapper->setEnableFlag(true);
      histogramRemapper->setStretchMode( ossimHistogramRemapper::LINEAR_AUTO_MIN_MAX );
      histogramRemapper->setHistogram(histogram);
      m_procChain->add(histogramRemapper.get());
   }

   // Add scalar remapper:
   ossimRefPtr<ossimScalarRemapper> scalarRemapper = new ossimScalarRemapper();
   scalarRemapper->setOutputScalarType(OSSIM_UINT8);
   m_procChain->add(scalarRemapper.get());

   m_procChain->initialize();
}

bool ossimRemapTool::execute()
{
   if (m_productFilename.empty())
   {
      m_productFilename = m_inputFilename.fileNoExtension() + "-remap";
      m_productFilename.setExtension(m_inputFilename.ext());
   }

   m_geom->getBoundingRect(m_aoiViewRect);

   // Parent class has service to create writer:
   m_writer = newWriter();
   m_writer->connectMyInputTo(0, m_procChain.get());

   // Add a listener to get percent complete.
   m_writer->addListener(&theStdOutProgress);

   // Write the file and external geometry:
   if(!m_writer->execute())
      return false;

   if (!m_writer->writeExternalGeometryFile())
      return false;

   ossimNotify(ossimNotifyLevel_INFO)<<"Wrote product image to <"<<m_productFilename<<">"<<endl;

   return true;
}


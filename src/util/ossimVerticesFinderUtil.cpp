//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimVerticesFinderUtil.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimException.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimVertexExtractor.h>
#include <iostream>

using namespace std;

const char* ossimVerticesFinderUtil::DESCRIPTION  =
      "Utility for determining the active image corner vertices inside larger null-filled image "
      "rectangle.";

ossimVerticesFinderUtil::ossimVerticesFinderUtil()
:  m_entryIndex (0)
{
}

ossimVerticesFinderUtil::~ossimVerticesFinderUtil()
{
}

void ossimVerticesFinderUtil::setUsage(ossimArgumentParser& ap)
{
   // Add options.
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " [options] <input-image> [<output-vertices.kwl>]";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->setDescription(DESCRIPTION);

   // Base class has its own:
   ossimUtility::setUsage(ap);

   au->addCommandLineOption("-e or --entry",
                            "<entry> For multi image handlers which entry do you wish to extract. "
                            "For list of entries use: \"ossim-info -i <your_image>\" ");
}

bool ossimVerticesFinderUtil::initialize(ossimArgumentParser& ap)
{
   if (!ossimUtility::initialize(ap))
      return false;

   std::string tempString1;
   ossimArgumentParser::ossimParameter stringParam1(tempString1);

   if ( ap.read("-e", stringParam1) || ap.read("--entry", stringParam1) )
      m_kwl.addPair( std::string(ossimKeywordNames::ENTRY_KW), tempString1 );

   cout<<ap.argc()<<endl;//TODO:REMOVE

   if ( ap.argc() > 1 )
   {
      m_kwl.add( ossimKeywordNames::IMAGE_FILE_KW, ap[1]);
      ap.remove(1);
   }
   if ( ap.argc() > 1 )
   {
      m_kwl.add( ossimKeywordNames::OUTPUT_FILE_KW, ap[1]);
      ap.remove(1);
   }
   if ( ap.argc() > 1 )
   {
      ossimNotify(ossimNotifyLevel_NOTICE)<<"ossimVerticesFinderUtil:"<<__LINE__<<" Too many "
            "arguments provided on command line. Ignoring superfluous entries.";
   }

   initialize(m_kwl);
   return true;
}

void ossimVerticesFinderUtil::initialize(const ossimKeywordlist& kwl)
{
   ostringstream errMsg;

   // Don't copy KWL if member KWL passed in:
   if (&kwl != &m_kwl)
   {
      // Start with clean options keyword list.
      m_kwl.clear();
      m_kwl.addList( kwl, true );
   }

   m_inputFile  = m_kwl.find( ossimKeywordNames::IMAGE_FILE_KW);

   m_outputFile = m_kwl.find( ossimKeywordNames::OUTPUT_FILE_KW);

   ossimString value = m_kwl.find( ossimKeywordNames::ENTRY_KW);
   if (!value.empty())
      m_entryIndex = value.toUInt32();

   // See if an LUT is requested:
   if (m_inputFile.empty() || !m_inputFile.isReadable())
   {
      errMsg<<"ERROR ossimVerticesFinderUtil:"<<__LINE__<<"  Input filename was not provided or "
            " file <"<<m_inputFile <<"> is not readable. "<<ends;
      throw ossimException(errMsg.str());
   }

   if (m_outputFile.empty())
   {
      m_outputFile = m_inputFile.path();
      m_outputFile = m_outputFile.dirCat(m_inputFile.fileNoExtension());
      m_outputFile += "_vertices.kwl";
   }
}

bool ossimVerticesFinderUtil::execute()
{
   ostringstream errMsg;

   // Get an image handler for the input file.
   ossimRefPtr<ossimImageHandler> ih = ossimImageHandlerRegistry::instance()->open(m_inputFile);
   if (ih.valid())
   {
      ih->setCurrentEntry(m_entryIndex);
   }
   else
   {
      errMsg<<"ERROR ossimVerticesFinderUtil:"<<__LINE__<<"  Unsupported image file format for <"
            << m_inputFile<<">. Cannot open file.";
      throw ossimException(errMsg.str());
   }

   // Give the image handler to the vertex extractor as an input source.
   ossimRefPtr<ossimVertexExtractor> ve = new ossimVertexExtractor(ih.get());

   // Set the file name.
   ve->setOutputName(m_outputFile);

   // Set the area of interest to the full bounding rect of the source.
   ve->setAreaOfInterest(ih->getBoundingRect(0));

   // Add a listener for the percent complete to standard output.
   ossimStdOutProgress prog(0, true);
   ve->addListener(&prog);

   // Start the extraction...
   bool success = ve->execute();
   return success;
}


void ossimVerticesFinderUtil::getKwlTemplate(ossimKeywordlist& kwl)
{
   kwl.add(ossimKeywordNames::ENTRY_KW, "<sub-image entry index>");
   kwl.add(ossimKeywordNames::IMAGE_FILE_KW, "<input-raster-file>");
   kwl.add(ossimKeywordNames::OUTPUT_FILE_KW, "<output-vertices-file>");
}


//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimSubImageTool.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/projection/ossimRpcSolver.h>
#include <iostream>

using namespace std;

const char* ossimSubImageTool::DESCRIPTION  = "Tool for extracting a sub-image from a full image.";
const char* BBOX_KW = "bbox";

ossimSubImageTool::ossimSubImageTool()
{
}

ossimSubImageTool::~ossimSubImageTool()
{
}

void ossimSubImageTool::setUsage(ossimArgumentParser& ap)
{
   // Add options.
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " subimage [options] <input-image> <output-image>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->setDescription(DESCRIPTION);

   // Base class has its own. Skip the ossimChipProcTool usage as that adds a lot of view-space
   // specific stuff not used in this tool:
   ossimTool::setUsage(ap);

   au->addCommandLineOption("--bbox <ulx> <uly> <lrx> <lry>",
                            "Specify upper-left and lower-right bounds the image rect (in pixels).");
   au->addCommandLineOption("-e | --entry <N> ",
                            "For multi image entries which entry do you wish to extract. For list of entries use: \"ossim-info -i <your_image>\" ");
}

bool ossimSubImageTool::initialize(ossimArgumentParser& ap)
{
   if (!ossimTool::initialize(ap))
      return false;
   if (m_helpRequested)
      return true;

   string tempString1;
   ossimArgumentParser::ossimParameter stringParam1(tempString1);
   string tempString2;
   ossimArgumentParser::ossimParameter stringParam2(tempString2);
   string tempString3;
   ossimArgumentParser::ossimParameter stringParam3(tempString3);
   string tempString4;
   ossimArgumentParser::ossimParameter stringParam4(tempString4);
   double tempDouble1;
   ossimArgumentParser::ossimParameter doubleParam1(tempDouble1);
   double tempDouble2;
   ossimArgumentParser::ossimParameter doubleParam2(tempDouble2);
   vector<ossimString> paramList;

   ossim_uint32 readerPropIdx = 0;
   ossim_uint32 writerPropIdx = 0;
   ostringstream keys;

   if ( ap.read("--bbox", stringParam1, stringParam2, stringParam3, stringParam4))
   {
      ostringstream ostr;
      ostr<<tempString1<<" "<<tempString2<<" "<<tempString3<<" "<<tempString4<<ends;
      m_kwl.addPair( string(BBOX_KW), ostr.str() );
   }
   if ( ap.read("-e", stringParam1) || ap.read("--entry", stringParam1) )
      m_kwl.addPair( string(ossimKeywordNames::ENTRY_KW), tempString1 );

   processRemainingArgs(ap);

   return true;
}

void ossimSubImageTool::initialize(const ossimKeywordlist& kwl)
{
   m_productFilename = m_kwl.find(ossimKeywordNames::OUTPUT_FILE_KW);

   // Init chain with handler:
   ostringstream key;
   key<<ossimKeywordNames::IMAGE_FILE_KW<<"0";
   ossimFilename fname = m_kwl.findKey(key.str());
   ossimRefPtr<ossimImageHandler> handler =
         ossimImageHandlerRegistry::instance()->open(fname, true, false);
   if (!handler)
   {
      ostringstream errMsg;
      errMsg<<"ERROR: ossimSubImageTool ["<<__LINE__<<"] Could not open <"<<fname<<">"<<ends;
      throw ossimException(errMsg.str());
   }

   // Look for the entry keyand set it:
   ossim_uint32 entryIndex = 0;
   ossimString os = m_kwl.find(ossimKeywordNames::ENTRY_KW);
   if (os)
      entryIndex = os.toUInt32();
   if (!handler->setCurrentEntry( entryIndex ))
   {
      ostringstream errMsg;
      errMsg << " ERROR: ossimSubImageTool ["<<__LINE__<<"] Entry " << entryIndex << " out of range!" << endl;
      throw ossimException( errMsg.str() );
   }
   m_procChain->add(handler.get());

   // And finally the bounding rect:
   ossimString lookup = m_kwl.find(BBOX_KW);
   lookup.trim();
   vector<ossimString> substrings = lookup.split(", ", true);
   if (substrings.size() != 4)
   {
      ostringstream errMsg;
      errMsg << "ossimSubImageTool ["<<__LINE__<<"] Incorrect number of values specified for bbox!";
      throw( ossimException(errMsg.str()) );
   }

   int ulx = substrings[0].toInt32();
   int uly = substrings[1].toInt32();
   int lrx = substrings[2].toInt32();
   int lry = substrings[3].toInt32();

   // Check for swap:
   if ( ulx > lrx )
   {
      int tmpF = ulx;
      ulx = lrx;
      lrx = tmpF;
   }
   if ( uly > lry )
   {
      int tmpF = uly;
      uly = lry;
      lry = tmpF;
   }

   // Use of view rect here is same as image space since there is no renderer:
   m_aoiViewRect.set_ul(ossimIpt(ulx, uly));
   m_aoiViewRect.set_lr(ossimIpt(lrx, lry));
   m_needCutRect = true;

   finalizeChain();
}

bool ossimSubImageTool::execute()
{
   // Compute RPC representation of subimage:
   ossimRefPtr<ossimImageGeometry> inputGeom = m_procChain->getImageGeometry();
   if (!inputGeom || !inputGeom->getProjection())
   {
      ostringstream errMsg;
      errMsg << " ERROR: ossimSubImageTool ["<<__LINE__<<"] Null projection returned for input "
            "image!" << endl;
      throw ossimException( errMsg.str() );
   }
   ossimRefPtr<ossimRpcSolver> solver = new ossimRpcSolver(true);
   bool converged = solver->solveCoefficients(inputGeom.get());
   ossimRefPtr<ossimRpcModel> rpc = solver->createRpcModel();
   m_geom = new ossimImageGeometry(nullptr, rpc.get());
   m_geom->setImageSize(m_aoiViewRect.size());

   return ossimChipProcTool::execute();
}


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
#include <ossim/base/ossimXmlDocument.h>
#include <iostream>

using namespace std;

const char* ossimSubImageTool::DESCRIPTION  = "Tool for extracting a sub-image from a full image.";
const char* BBOX_KW = "bbox";

ossimSubImageTool::ossimSubImageTool()
:  m_geomFormat (OGEOM)
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

   ostringstream descr;
   descr << DESCRIPTION << "\n\n"
         <<  " No reprojection is done. Presently, the subimage geometry is represented by an RPC "
         << "replacement model until generic models can support subimage chipping.";
   au->setDescription(descr.str());

   // Set the command line options:
   // Base class has its own. Skip the ossimChipProcTool usage as that adds a lot of view-space
   // specific stuff not used in this tool:
   ossimTool::setUsage(ap);

   au->addCommandLineOption(
         "--bbox <ulx> <uly> <lrx> <lry>",
         "Specify upper-left and lower-right bounds the image rect (in pixels).");
   au->addCommandLineOption(
         "-e | --entry <N> ",
         "For multi image entries which entry do you wish to extract. For list "
         "of entries use: \"ossim-info -i <your_image>\" ");
   au->addCommandLineOption(
         "--geom <format>", "Specifies format of the subimage RPC geometry file."
         " Possible values are: \"OGEOM\" (OSSIM geometry, default), \"DG\" (DigitalGlobe WV/QB "
         ".RPB format), \"JSON\" (MSP-style JSON), or \"XML\". Case insensitive.");
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

   if ( ap.read("--geom", stringParam1))
   {
      ossimString formatStr (tempString1);
      formatStr.upcase();
      if (formatStr == "OGEOM")
         m_geomFormat = OGEOM;
      else if (formatStr == "DG")
         m_geomFormat = DG;
      else if (formatStr == "JSON")
         m_geomFormat = JSON;
      else if (formatStr == "XML")
         m_geomFormat = XML;
      else
      {
         ostringstream errMsg;
         errMsg << " ERROR: ossimSubImageTool ["<<__LINE__<<"] Unknown geometry format <"
               <<formatStr<<"> specified. Aborting." << endl;
         throw ossimException( errMsg.str() );
      }
   }
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

   ossimRefPtr<ossimProjection> inputProj = inputGeom->getProjection();
   ossimRefPtr<ossimRpcModel> rpc;
   ossimRefPtr<ossimRpcSolver> solver = new ossimRpcSolver(false);
   bool converged = false;

   // If the input projection itself is an RPC, just copy it. No solving required:
   ossimRpcModel* inputRpc = dynamic_cast<ossimRpcModel*>(inputGeom->getProjection());
   if (inputRpc)
      rpc = inputRpc;
   else
   {
      converged = solver->solve(m_aoiViewRect, inputGeom.get());
      rpc = solver->getRpcModel();
   }

   // The RPC needs to be shifted in image space so that it will work in the subimage coordinates:
   rpc->setImageOffset(m_aoiViewRect.ul());
   m_geom = new ossimImageGeometry(nullptr, rpc.get());
   m_geom->setImageSize(m_aoiViewRect.size());
   ossimKeywordlist kwl;
   m_geom->saveState(kwl);

   bool success = ossimChipProcTool::execute();
   if (!success)
      return false;

   // Need to save the subimage RPC data:
   bool write_ok = false;
   ossimFilename geomFile (m_productFilename);
   if (m_geomFormat == OGEOM) // Default case if none specified
   {
      geomFile.setExtension("geom");
      write_ok = kwl.write(geomFile);
   }
   else if (m_geomFormat == JSON)
   {
#if OSSIM_HAS_JSONCPP
      geomFile.setExtension("json");
      ofstream jsonStream (geomFile.string());
      if (!jsonStream.fail())
      {
         // Note that only the model/projection is saved here, not the full ossimImageGeometry that
         // contains the subimage shift transform. So need to cheat and add the shift to the RPC
         // line and sample offsets:
         write_ok = rpc->toJSON(jsonStream);
         jsonStream.close();
      }
#else
      ostringstream errMsg;
      errMsg << " ERROR: ossimSubImageTool ["<<__LINE__<<"] JSON geometry output requested but JSON is not "
            "available in this build! <" << endl;
      throw ossimException( errMsg.str() );
#endif
   }
   else if (m_geomFormat == DG)
   {
      geomFile.setExtension("RPB");
      ofstream rpbStream (geomFile.string());
      if (!rpbStream.fail())
      {
         write_ok = rpc->toRPB(rpbStream);
         rpbStream.close();
      }
   }
   else if (m_geomFormat == XML)
   {
      geomFile.setExtension("xml");
      ossimXmlDocument xmlDocument;
      xmlDocument.fromKwl(kwl);
      write_ok = xmlDocument.write(geomFile);
   }

   if (write_ok)
      ossimNotify(ossimNotifyLevel_INFO) << "Wrote geometry file to <"<<geomFile<<">.\n" << endl;
   else
   {
      ossimNotify(ossimNotifyLevel_FATAL) << "Error encountered writing output RPC geometry file."
            << std::endl;
      return false;
   }

   return true;
}


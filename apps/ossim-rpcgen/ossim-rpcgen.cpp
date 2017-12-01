#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimRpcSolver.h>
#include <sstream>
#include <ossim/base/ossimXmlDocument.h>

using namespace std;

int main(int argc, char* argv[])
{
   ossimString tempString1;
   ossimString tempString2;
   ossimString tempString3;
   ossimString tempString4;
   ossimArgumentParser::ossimParameter tempParam1(tempString1);
   ossimArgumentParser::ossimParameter tempParam2(tempString2);
   ossimArgumentParser::ossimParameter tempParam3(tempString3);
   ossimArgumentParser::ossimParameter tempParam4(tempString4);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   bool rpcFlag       = false;
   bool cgFlag       = false;
   ossimDrect imageRect;
   double error = 0.1;

   imageRect.makeNan();
   ossimApplicationUsage* au = argumentParser.getApplicationUsage();

   au->setApplicationName(argumentParser.getApplicationName());
   au->setDescription(
         argumentParser.getApplicationName() + " takes an input image and generates a "
         "corresponding RPC geometry in a variety of formats. If a bounding box is specified, the "
         "default output filename (based on the input filename) will be appended with the bbox spec"
         ", unless an output filename is explicitely provided.");
   au->setCommandLineUsage(
         argumentParser.getApplicationName() + " [options] <input-file> [<output-file>]");
   au->addCommandLineOption(
         "-h | --help","Display this information");
   au->addCommandLineOption(
         "--bbox <ulx> <uly> <lrx> <lry>","Subimage rectangle in image space for constraining "
         "RPC computation over the AOI only. Note that the RPC image space UL corner will "
         "correspond to (0,0), i.e., the model will be shifted from the original full-image model.");
   au->addCommandLineOption(
         "--tolerance <double>","Used as an RMS error tolerance in meters between original model "
         "and RPC.");
   au->addCommandLineOption(
         "--geom <format>", "Specifies format of the subimage RPC geometry file."
         " Possible values are: \"OGEOM\" (OSSIM geometry, default), \"DG\" (DigitalGlobe WV/QB "
         ".RPB format), \"JSON\" (MSP-style JSON), or \"XML\". Case insensitive.");
   
   int numArgs = argumentParser.argc();
   if (argumentParser.read("-h") || argumentParser.read("--help") || (numArgs == 1))
   {
      au->write(ossimNotify(ossimNotifyLevel_INFO));
      ossimInit::instance()->finalize();
      exit(0);
   }

   enum RpcGeomFormat { OGEOM, DG, JSON, XML } rpcGeomFormat=OGEOM;

   ossimFilename geomFile, jsonFile, xmlFile, rpbFile;
   ossimString suffix;
   ossimIpt rpcGridSize(10,10);

   if(argumentParser.read("--tolerance", tempParam1))
      error = tempString1.toDouble();

   if(argumentParser.read("--bbox", tempParam1,tempParam2,tempParam3,tempParam4 ))
   {
      double ulx,uly,lrx,lry,flip;
      ulx = tempString1.toDouble();
      uly = tempString2.toDouble();
      lrx = tempString3.toDouble();
      lry = tempString4.toDouble();

      if (lrx < ulx)
      {
         flip = ulx; ulx = lrx; lrx = flip;
      }
      if (lry < uly)
      {
         flip = uly; uly = lry; lry = flip;
      }

      imageRect = ossimDrect(ulx,uly,lrx,lry);
      ostringstream s;
      s<<"_bbox-"<<ulx<<"-"<<uly<<"-"<<lrx<<"-"<<lry;
      suffix = s.str();
   }

   if ( argumentParser.read("--geom", tempParam1))
   {
      ossimString formatStr (tempString1);
      formatStr.upcase();
      if (formatStr == "OGEOM")
         rpcGeomFormat = OGEOM;
      else if (formatStr == "DG")
         rpcGeomFormat = DG;
      else if (formatStr == "JSON")
         rpcGeomFormat = JSON;
      else if (formatStr == "XML")
         rpcGeomFormat = XML;
      else
      {
         ostringstream errMsg;
         errMsg << " ERROR: ossimSubImageTool ["<<__LINE__<<"] Unknown geometry format <"
               <<formatStr<<"> specified. Aborting." << endl;
         throw ossimException( errMsg.str() );
      }
   }
   argumentParser.reportRemainingOptionsAsUnrecognized();
   if (argumentParser.errors())
   {
      argumentParser.writeErrorMessages(std::cout);
      exit(1);
   }

   // Read remaining args as input and optional output filenames:
   if (argumentParser.argc() < 2)
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"ERROR: Need an input filename."<<endl;
      au->write(ossimNotify(ossimNotifyLevel_INFO));
      ossimInit::instance()->finalize();
      exit(0);
   }
   ossimFilename inputFile (argumentParser[1]);
   ossimFilename outputFile;
   if (argumentParser.argc() > 2)
      outputFile = argumentParser[2];

   // Establish input geometry:
   ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(inputFile);
   ossimRefPtr<ossimProjection> inputProj = 0;
   ossim_int32 minSpacing = 100;
   ossimRefPtr<ossimImageGeometry> geom;
   if(h.valid())
   {
      geom = h->getImageGeometry();
      if(imageRect.hasNans())
         imageRect = h->getBoundingRect();
   }
   else
   {
      ossimNotify(ossimNotifyLevel_FATAL) << "ERROR: Unable to open the image file <"<<inputFile<<">" << std::endl;
      au->write(ossimNotify(ossimNotifyLevel_INFO));
      exit(1);
   }
   if (!geom || !geom->getProjection())
   {
      ossimNotify(ossimNotifyLevel_FATAL) << "ERROR: Unable to obtain an input projection. " << std::endl;
      exit(1);
   }
   if(imageRect.hasNans())
   {
      ossimNotify(ossimNotifyLevel_FATAL) << "ERROR: Unable to establish the image bounding rect." << std::endl;
      exit(1);
   }

   // First consider if the input is already an RPC (type B), and if so, simply copy it:
   ossimRefPtr<ossimRpcModel> rpc;
   ossimRpcModel* inputRpc = dynamic_cast<ossimRpcModel*>(geom->getProjection());
   if (inputRpc)
   {
      ossimNotify(ossimNotifyLevel_INFO) << "\nThe input image is already using RPC. Simply copying "
            "the coefficients to the output with offset (if any) applied..." << std::endl;
      rpc = inputRpc;
   }
   else
   {
      // Solve for replacement RPC:
      ossimNotify(ossimNotifyLevel_INFO) << "\nSolving for RPC coefficients..." << std::endl;
      ossimRefPtr<ossimRpcSolver> solver = new ossimRpcSolver(true, false);
      bool converged = solver->solve(imageRect, geom.get(), error);
      double meanResidual = solver->getRmsError();
      double maxResidual = solver->getMaxError();
      rpc = solver->getRpcModel();
   }

   // Apply the offset to the bbox so that the RPC image-space coordinates will start at 0,0:
   rpc->setImageOffset(imageRect.ul());
   ossimRefPtr<ossimImageGeometry> rpcgeom = new ossimImageGeometry(nullptr, rpc.get());
   rpcgeom->setImageSize(imageRect.size());
   ossimKeywordlist kwl;
   rpcgeom->saveState(kwl);

   // Write output file(s):
   bool write_ok = false;
   switch (rpcGeomFormat)
   {
   case JSON:
   {
#if OSSIM_HAS_JSONCPP
      if (outputFile)
         jsonFile = outputFile;
      else
         jsonFile = inputFile.fileNoExtension()+suffix;
      jsonFile.setExtension("json");
      ossimNotify(ossimNotifyLevel_INFO) << "\nWriting RPC JSON file to <"<<jsonFile<<">." << endl;
      ofstream jsonStream (jsonFile.string());
      if (!jsonStream.fail())
      {
         write_ok = rpc->toJSON(jsonStream);
         jsonStream.close();
      }
#else
      ossimNotify(ossimNotifyLevel_FATAL) << "ERROR: JSON output requested but JSON is not "
            "available in this build! <"<< std::endl;
      exit(1);
#endif
      break;
   }

   case DG:
   {
      if (outputFile)
         rpbFile = outputFile;
      else
         rpbFile = inputFile.fileNoExtension()+suffix;
      rpbFile.setExtension("RPB");
      ossimNotify(ossimNotifyLevel_INFO) << "\nWriting RPB file to <"<<rpbFile<<">." << endl;
      ofstream rpbStream (rpbFile.string());
      if (!rpbStream.fail())
      {
         write_ok = rpc->toRPB(rpbStream);
         rpbStream.close();
      }
      break;
   }

   case XML:
   {
      if (outputFile)
         xmlFile = outputFile;
      else
         xmlFile = inputFile.fileNoExtension()+suffix;
      xmlFile.setExtension("xml");
      ossimNotify(ossimNotifyLevel_INFO) << "\nWriting RPC XML file to <"<<xmlFile<<">." << endl;
      ossimXmlDocument xmlDocument;
      xmlDocument.fromKwl(kwl);
      write_ok = xmlDocument.write(xmlFile);
      break;
   }

   case OGEOM:
   default:
   {
      if (outputFile)
         geomFile = outputFile;
      else
         geomFile = inputFile.fileNoExtension()+suffix;
      geomFile.setExtension("geom");
      ossimNotify(ossimNotifyLevel_INFO) << "\nWriting RPC geometry file to <"<<geomFile<<">." << endl;
      write_ok = kwl.write(geomFile);
      break;
   }
   }

   if (write_ok)
      ossimNotify(ossimNotifyLevel_INFO)<<"Finished.\n"<<endl;
   else
   {
      ossimNotify(ossimNotifyLevel_FATAL) << "Error encountered writing output RPC file." << std::endl;
      exit(1);
   }

   return 0;
}

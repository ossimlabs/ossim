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
   bool enableElevFlag = true;
   bool enableAdjustmentFlag = true;
   ossimDrect imageRect;
   double error = 0.1;

   imageRect.makeNan();
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(
         argumentParser.getApplicationName() + " takes an input image and generates a "
         "corresponding RPC geometry in a variety of formats. If a bounding box is specified, the "
         "default output filename (based on the input filename) will be appended with the bbox spec"
         ", unless an output filename is explicitely provided.");
   argumentParser.getApplicationUsage()->setCommandLineUsage(
         argumentParser.getApplicationName() + " [options] <input-file> [<output-file>]");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "-h | --help","Display this information");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--geom","[default] Outputs the RPC to an OSSIM geometry file.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--json","Outputs the RPC in JSON format.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--rpb","Output WorldView-style RPB format.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--xml","Output the RPC in XML format.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--bbox <ulx> <uly> <width> <height>","Subimage rectangle in image space for constraining "
         "RPC computation over the AOI only.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--tolerance <double>","Used as an RMS error tolerance in meters between original model "
         "and RPC.");
   
   int numArgs = argumentParser.argc();
   if (argumentParser.read("-h") || argumentParser.read("--help") || (numArgs == 1))
   {
      argumentParser.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      ossimInit::instance()->finalize();
      exit(0);
   }

   bool doGeom=false, doJson=false, doRpb=false, doXml=false;
   ossimFilename geomFile, jsonFile, xmlFile, rpbFile;
   ossimString suffix;
   ossimIpt rpcGridSize(10,10);

   if(argumentParser.read("--tolerance", tempParam1))
      error = tempString1.toDouble();

   if(argumentParser.read("--bbox", tempParam1,tempParam2,tempParam3,tempParam4 ))
   {
      double x,y,w,h;
      x = tempString1.toDouble();
      y = tempString2.toDouble();
      w = tempString3.toDouble();
      h = tempString4.toDouble();

      if(w < 1) w = 1;
      if(h < 1) h = 1;
      imageRect = ossimDrect(x,y,x+(w-1), y+(h-1));
      ostringstream s;
      s<<"_bbox-"<<x<<"-"<<y<<"-"<<w<<"-"<<h;
      suffix = s.str();
   }

   if (argumentParser.read("--geom"))
      doGeom = true;
   if(argumentParser.read("--json"))
      doJson = true;
   if (argumentParser.read("--rpb"))
      doRpb = true;
   if (argumentParser.read("--xml"))
      doXml = true;

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
      argumentParser.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      ossimInit::instance()->finalize();
      exit(0);
   }
   ossimFilename inputFile (argumentParser[1]);
   ossimFilename outputFile;
   if (argumentParser.argc() > 2)
      outputFile = argumentParser[2];

   // Establish input geometry::
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
      argumentParser.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
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

   // Solve for replacement RPC:
   ossimRefPtr<ossimRpcSolver> solver = new ossimRpcSolver(enableElevFlag);

   bool converged = solver->solve(imageRect, geom.get(), error);
   double meanResidual = solver->getRmsError();
   double maxResidual = solver->getMaxError();

   ossimRefPtr<ossimRpcModel> rpc = solver->getRpcModel();
   ossimRefPtr<ossimImageGeometry> rpcgeom = new ossimImageGeometry(nullptr, rpc.get());
   rpcgeom->setImageSize(imageRect.size());
   ossimKeywordlist kwl;
   rpcgeom->saveState(kwl);

   // Write output file(s):
   bool write_ok = false;
   if (doGeom || !(doJson || doRpb || doXml)) // Default case if none specified
   {
      if (outputFile)
         geomFile = outputFile;
      else
         geomFile = inputFile.fileNoExtension()+suffix;
      geomFile.setExtension("geom");
      ossimNotify(ossimNotifyLevel_INFO) << "\nWriting RPC geometry file to <"<<geomFile<<">." << endl;
      write_ok = kwl.write(geomFile);
   }

   if (doJson)
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
   }

   if (doRpb)
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
   }

   if (doXml)
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

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
         "corresponding RPC geometry.");
   argumentParser.getApplicationUsage()->setCommandLineUsage(
         argumentParser.getApplicationName() + " [options] <input file>");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "-h or --help","Display this information");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--geom","Default -- Outputs the RPC to an OSSIM geometry file. The input file's base name"
         " is used.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--json","Outputs the RPC to a JSON file. The input file's base name is used.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--rpb","Output WorldView-style RPB format. The input file's base name is used.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--rect <ulx> <uly> <width> <height>","Subimage rectangle in image space for constraining "
         "RPC computation over the AOI only.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--tolerance <double>","Used as an RMS error tolerance in meters between original model "
         "and RPC.");
   argumentParser.getApplicationUsage()->addCommandLineOption(
         "--xml","Output XML-formatted RPC. The input file's base name is used.");
   
   int numArgs = argumentParser.argc();
   if (argumentParser.read("-h") || argumentParser.read("--help") || (numArgs == 1))
   {
      argumentParser.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      ossimInit::instance()->finalize();
      exit(0);
   }
   ossimFilename inputFile (argumentParser[--numArgs]);
   argumentParser.remove(numArgs);

   ossimFilename geomFile, jsonFile, xmlFile, rpbFile;
   ossimIpt rpcGridSize(10,10);
   if(argumentParser.read("--tolerance", tempParam1))
      error = tempString1.toDouble();

   if (argumentParser.read("--geom"))
   {
      geomFile = inputFile;
      geomFile.setExtension("geom");
   }

   if(argumentParser.read("--json"))
#if OSSIM_HAS_JSONCPP
   {
      jsonFile = inputFile;
      jsonFile.setExtension("json");
   }
#else
   {
      ossimNotify(ossimNotifyLevel_FATAL) << "ERROR: JSON output requested but JSON is not "
            "available in this build! <"<< std::endl;
      exit(1);
   }
#endif

   if (argumentParser.read("--rpb"))
   {
      rpbFile = inputFile;
      rpbFile.setExtension("RPB");
   }

   if (argumentParser.read("--xml"))
   {
      xmlFile = inputFile;
      xmlFile.setExtension("xml");
   }

   if(argumentParser.read("--rect", tempParam1,tempParam2,tempParam3,tempParam4 ))
   {
      double x,y,w,h;
      x = tempString1.toDouble();
      y = tempString2.toDouble();
      w = tempString3.toDouble();
      h = tempString4.toDouble();

      if(w < 1) w = 1;
      if(h < 1) h = 1;
      imageRect = ossimDrect(x,y,x+(w-1), y+(h-1));
   }
   argumentParser.reportRemainingOptionsAsUnrecognized();
   if (argumentParser.errors())
   {
      argumentParser.writeErrorMessages(std::cout);
      exit(1);
   }

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

   if (!(geomFile || xmlFile || jsonFile || rpbFile))
      geomFile = inputFile.setExtension("geom");

   ossimRefPtr<ossimRpcSolver> solver = new ossimRpcSolver(enableElevFlag);

   bool converged = solver->solve(imageRect, geom.get(), error);
   double meanResidual = solver->getRmsError();
   double maxResidual = solver->getMaxError();

   ossimRefPtr<ossimRpcModel> rpc = solver->getRpcModel();
   ossimRefPtr<ossimImageGeometry> rpcgeom = new ossimImageGeometry(nullptr, rpc.get());
   ossimKeywordlist kwl;
   rpcgeom->saveState(kwl);

   if (geomFile)
   {
      kwl.write(geomFile);
   }

   if (xmlFile)
   {
      ossimXmlDocument xmlDocument;
      xmlDocument.fromKwl(kwl);
      bool write_ok = xmlDocument.write(xmlFile);
      if (!write_ok)
      {
         ossimNotify(ossimNotifyLevel_FATAL) << "ERROR: Unable to write XML output file <"<<xmlFile<<">." << std::endl;
         exit(1);
      }
   }

   if (rpbFile)
   {
      ofstream rpbStream (rpbFile.string());
      if (rpbStream.fail())
      {
         ossimNotify(ossimNotifyLevel_FATAL) << "ERROR: Unable to open RPB output file <"<<rpbFile<<">." << std::endl;
         exit(1);
      }
      rpc->toRPB(rpbStream);
      rpbStream.close();
   }

   if (jsonFile)
   {
      ofstream jsonStream (jsonFile.string());
      if (jsonStream.fail())
      {
         ossimNotify(ossimNotifyLevel_FATAL) << "ERROR: Unable to open JSON output file <"<<jsonFile<<">." << std::endl;
         exit(1);
      }
      rpc->toJSON(jsonStream);
      jsonStream.close();
   }
   return 0;
}

#include <ossim/projection/ossimCoarseGridModel.h>
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
   double error = .1;

   imageRect.makeNan();
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName() + " takes an input geometry (or image) and creates a converted output geometry");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName() + " [options] <input file>");
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   argumentParser.getApplicationUsage()->addCommandLineOption("--rpc","Create an RPC projection");
   argumentParser.getApplicationUsage()->addCommandLineOption("--rpc-gridsize","defines the grid size for the rpc estimate default is --rpc-gridsize=\"10 10\"");
   argumentParser.getApplicationUsage()->addCommandLineOption("--noelev","the projection but 0 out the elevation");
   argumentParser.getApplicationUsage()->addCommandLineOption("--disable-adjustments","Current applies to coarse grid.  It will try to make the grid adjustable if the input projection is adjustable");
   argumentParser.getApplicationUsage()->addCommandLineOption("--cg","Create a coarse grid projection");
   argumentParser.getApplicationUsage()->addCommandLineOption("--rect"," 4 values ulx uly width height");
   argumentParser.getApplicationUsage()->addCommandLineOption("--tolerance","Used as an error tolerance.  Currently on coarse grid uses it and is the pixel error for the estimate");
   argumentParser.getApplicationUsage()->addCommandLineOption("--output","Override the default output name");
   
   if (argumentParser.read("-h") || argumentParser.read("--help") || (argumentParser.argc() == 1))
   {
      argumentParser.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      ossimInit::instance()->finalize();
      exit(0);
   }
   ossimFilename outputFile;
   ossimIpt rpcGridSize(10,10);
   if(argumentParser.read("--tolerance", tempParam1))
   {
      error = tempString1.toDouble();
   }
   
   if (argumentParser.read("--rpc"))
   {
      rpcFlag = true;
   }
   if (argumentParser.read("--rpc-gridsize",tempParam1, tempParam2))
   {
      rpcGridSize.x = tempString1.toInt32();
      rpcGridSize.y = tempString2.toInt32();
      if(rpcGridSize.x < 1)
      {
         rpcGridSize.x = 8;
      }
      if(rpcGridSize.y < 1)
      {
         rpcGridSize.y = rpcGridSize.x;
      }
   }
   if (argumentParser.read("--cg"))
   {
      cgFlag = true;
   }
   if (argumentParser.read("--noelev"))
   {
      enableElevFlag = false;
   }
   if(argumentParser.read("--disable-adjustments"))
   {
      enableAdjustmentFlag = false;
   }
   if(argumentParser.read("--output", tempParam1))
   {
      outputFile = ossimFilename(tempString1);
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
      exit(0);
   }
   ossimFilename file(argv[1]);
   ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(file);
   ossimRefPtr<ossimProjection> inputProj = 0;
   ossimKeywordlist kwl;
   ossim_int32 minSpacing = 100;
   ossimRefPtr<ossimImageGeometry> geom;
   if(h.valid())
   {
      geom      = h->getImageGeometry();
      imageRect = h->getBoundingRect();
   }
   else if(!imageRect.hasNans())
   {
      kwl.add(ossimKeywordNames::GEOM_FILE_KW,
              file.c_str());
      inputProj = ossimProjectionFactoryRegistry::instance()->createProjection(kwl);
   }
   if(!geom.valid()||!geom->getProjection())
   {
      ossimNotify(ossimNotifyLevel_WARN) << "Unable to obtain an input projection. Returning " << std::endl;
      argumentParser.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
   }
   else if(!imageRect.hasNans())
   {
      if(outputFile.empty())
      {
         outputFile = file.setExtension("geom");
      }
      if(rpcFlag)
      {
         ossimRefPtr<ossimRpcSolver> solver = new ossimRpcSolver(enableElevFlag);
         
         solver->solveCoefficients(imageRect,
                                  geom.get(),
                                  rpcGridSize.x,
                                  rpcGridSize.y);
         
         ossimRefPtr<ossimImageGeometry> outputProj = solver->createRpcProjection();
         kwl.clear();
         outputProj->saveState(kwl);
         kwl.write(outputFile);
      }
      else if(cgFlag)
      {
         ossimCoarseGridModel::setInterpolationError(error);
         ossimCoarseGridModel::setMinGridSpacing(minSpacing);
         ossimCoarseGridModel cg;
         
         cg.buildGrid(imageRect,
                      inputProj.get(),
                      500.0,
                      enableElevFlag,
                      enableAdjustmentFlag);
         kwl.clear();
         cg.saveState(kwl);
         kwl.write(outputFile);
         cg.saveCoarseGrid(outputFile.setExtension("dat"));
      }
   }
   else
   {
      ossimNotify(ossimNotifyLevel_WARN) << "Unable to find an image rect" << std::endl;
   }
   return 0;
}

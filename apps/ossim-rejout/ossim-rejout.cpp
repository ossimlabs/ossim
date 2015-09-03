//*******************************************************************
//
// Author:  Frederic Claudel, CSIR
//
// rejects outliers and optimize model (RANSAC)
// to use after correl, with a tie points file
//
// front-end for class ossimOutlierRejection
//
//*******************************************************************

#include <cstdlib>
#include <iostream>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimPropertyInterface.h>
#include <ossim/base/ossimProcessInterface.h>

//error header
static const char* PROGERR = "ERROR rejout";

//globals
ossimFilename outGeomName="rejout.geom";
ossimFilename outItptName="inliers.xml";
ossimString modelDefinition="ossimPolynomProjection{1 x y x2 xy y2 x3 y3 xy2 x2y z xz yz}";

int
main(int argc, char *argv[])
{
   //error status : initially ok
   int result=0;

   //init OSSIM (prefs, elevation, plugins, etc)
   std::string  tempString;
   double       tempDouble;
   ossimArgumentParser::ossimParameter argString(tempString);
   ossimArgumentParser::ossimParameter argDouble(tempDouble);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   //init with default parms
//   ossimOutlierRejection mo;

   ossimRefPtr<ossimObject> moObject = ossimObjectFactoryRegistry::instance()->createObject(ossimString("ossimOutlierRejection"));
   ossimPropertyInterface* moPropertyInterface = PTR_CAST(ossimPropertyInterface, moObject.get());
   ossimProcessInterface* moProcessInterface = PTR_CAST(ossimProcessInterface, moObject.get());
   if(!moObject.valid())
   {
      ossimNotify(ossimNotifyLevel_WARN) << "Unable to find object ossimOutlierRejection in registration plugin" << std::endl;
      exit(1);
   }
   ossimRefPtr<ossimProperty> inlierRatio = moPropertyInterface->getProperty("inlier_ratio");
   ossimRefPtr<ossimProperty> inlierImageAccuracy = moPropertyInterface->getProperty("inlier_image_accuracy");
   
   //read args : options
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" removes outliers and optimizes model using tie points");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" [options] <GML_tiepoints_file>");
   argumentParser.getApplicationUsage()->addCommandLineOption("-h",
      ossimString("display Usage/Help"));
   argumentParser.getApplicationUsage()->addCommandLineOption("-d",
      ossimString("<model_definition> : default=") + modelDefinition );
   argumentParser.getApplicationUsage()->addCommandLineOption("-i",
                                                              ossimString("<inlier_ratio> : default=") + (inlierRatio.valid()?inlierRatio->valueToString():ossimString("")));
   argumentParser.getApplicationUsage()->addCommandLineOption("-a",
                                                              ossimString("<inlier_accuracy> : pixels, default=") + (inlierImageAccuracy.valid()?inlierImageAccuracy->valueToString():ossimString("")) );
   argumentParser.getApplicationUsage()->addCommandLineOption("-o",
      ossimString("<tiept_file> : output inliers tie points path, default=") + outItptName );
   argumentParser.getApplicationUsage()->addCommandLineOption("-g",
      ossimString("<geom_file> : output geometry file, default=") + outGeomName );

   if (argumentParser.read("-h"))
   {
      //help/usage
      argumentParser.getApplicationUsage()->write(std::cout);
      result = 0;
   }
   else
   {
      if (argumentParser.read("-d", argString))
      {
         
         modelDefinition = tempString;
      }
      if (argumentParser.read("-i", argDouble))
      {
         moPropertyInterface->setProperty("inlier_ratio", ossimString::toString(tempDouble));
//         mo.setInlierRatio(tempDouble);
      }
      if (argumentParser.read("-a", argDouble))
      {
         moPropertyInterface->setProperty("inlier_image_accuracy", ossimString::toString(tempDouble));
//         mo.setInlierImageAccuracy(tempDouble);
      }
      if (argumentParser.read("-g", argString))
      {
         moPropertyInterface->setProperty("geom_output_filename", tempString);
//         outGeomName = ossimString(tempString);
      }
      if (argumentParser.read("-o", argString))
      {
         moPropertyInterface->setProperty("inlier_output_filename", tempString);
//       outItptName = ossimString(tempString);
      }

      //read mandatory args : master and slave images
      //should only remain 2 args
      int nbargs = argumentParser.argc() - 1;
      if (nbargs == 1)
      {            
         ossimString tiePtPath(argumentParser.argv()[1]);

         moPropertyInterface->setProperty("gml_tieset_filename", tiePtPath);
         moPropertyInterface->setProperty("model_definition", modelDefinition);
         moProcessInterface->execute();
//          if (result==0) result = mo.loadGMLTieSet(tiePtPath)    ? 0 : 4 ;
//          if (result==0) result = mo.setupModel(modelDefinition) ? 0 : 5 ;
//          if (result==0) result = mo.removeOutliers()            ? 0 : 6 ;
//          if ((result==0) && (outGeomName != "")) mo.exportModel(outGeomName);
//          if ((result==0) && (outItptName != "")) mo.saveGMLTieSet(outItptName);

      }
      else
      {
         cerr<<PROGERR<<" bad number of non-optional args "<< nbargs << ",needs 1"<< endl;
         result = 3;
      }
   }
   
   return result;
}

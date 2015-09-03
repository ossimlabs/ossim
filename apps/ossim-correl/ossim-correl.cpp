//*******************************************************************
//
// Author:  Frederic Claudel
//    from ../mosaic/mosaic.cpp
//
//*******************************************************************
//

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimOutputSource.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <iostream>
#include <cstdlib>
using namespace std;

//error header
static const char* PROGERR = "ERROR correl";

int
main(int argc, char *argv[])
{
   //error status : initially ok
   int  result=0;

   //init OSSIM (prefs, elevation, plugins, etc)
   std::string  tempString;
   unsigned int tempUint;
   double       tempDouble;
   ossimArgumentParser::ossimParameter argString(tempString);
   ossimArgumentParser::ossimParameter argUint(tempUint);
   ossimArgumentParser::ossimParameter argDouble(tempDouble);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   //default parms
   ossimRefPtr<ossimObject> icObject = ossimObjectFactoryRegistry::instance()->createObject(ossimString("ossimImageCorrelator"));
   ossimOutputSource* icSource = PTR_CAST(ossimOutputSource, icObject.get());
   ossimProcessInterface* icProcessInterface = PTR_CAST(ossimProcessInterface, icObject.get());
   ossimPropertyInterface* icPropertyInterface = PTR_CAST(ossimPropertyInterface, icObject.get());

   if(!icObject.valid()||!icSource||!icProcessInterface||!icPropertyInterface)
   {
      ossimNotify(ossimNotifyLevel_WARN) << "Unable to find object ossimImageCorrelator in registration plugin" << std::endl;
      exit(1);
   }
   
   ossimRefPtr<ossimProperty> masterBand        = icSource->getProperty("master_band");
   ossimRefPtr<ossimProperty> slaveBand         = icSource->getProperty("slave_band");
   ossimRefPtr<ossimProperty> scaleRatio        = icSource->getProperty("scale_ratio");
   ossimRefPtr<ossimProperty> cornerDensity     = icSource->getProperty("corner_density");
   ossimRefPtr<ossimProperty> minCorrel         = icSource->getProperty("min_correl");
   ossimRefPtr<ossimProperty> templateRadius    = icSource->getProperty("template_radius");
   ossimRefPtr<ossimProperty> slaveAccuracy     = icSource->getProperty("slave_accuracy");
   ossimRefPtr<ossimProperty> projectionType    = icSource->getProperty("projection_type");
   ossimRefPtr<ossimProperty> outputFilename    = icSource->getProperty("output_filename");
   
   //read args : options   
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" coregisters a slave image to a master image");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" [options] <master_im> <slave_im>");
   argumentParser.getApplicationUsage()->addCommandLineOption("-h",
      ossimString("display Usage/Help"));
   argumentParser.getApplicationUsage()->addCommandLineOption("-m", ossimString("band index to use for master (default ") + masterBand->valueToString() + ")");
   argumentParser.getApplicationUsage()->addCommandLineOption("-s",
                                                              ossimString("band index to use for slave (default ") + slaveBand->valueToString() + ")");
   argumentParser.getApplicationUsage()->addCommandLineOption("-r",
      ossimString("scale ratio, >0 and <=1 (default ") + scaleRatio->valueToString() + ")");
   argumentParser.getApplicationUsage()->addCommandLineOption("-d",
                                                              ossimString("corner density, >0 and <=1 (default ") + cornerDensity->valueToString() + ")");
   argumentParser.getApplicationUsage()->addCommandLineOption("-c",
                                                              ossimString("minimum correlation, >=-1 and <=1 (default ") + minCorrel->valueToString() +")");
   argumentParser.getApplicationUsage()->addCommandLineOption("-t",
                                                              ossimString("template radius, pixel >=1 (default ") + templateRadius->valueToString() +")");
   argumentParser.getApplicationUsage()->addCommandLineOption("-e",
                                                              ossimString("absolute accuracy error, in pixels (default ") + slaveAccuracy->valueToString() +")");
   argumentParser.getApplicationUsage()->addCommandLineOption("-p",
                                                              ossimString("projection type: M (master) or S (slave) (default ") + projectionType->valueToString() + ")");
   argumentParser.getApplicationUsage()->addCommandLineOption("-o",
                                                              ossimString("output file name for tie points (default ") + outputFilename->valueToString() +")");

   if (argumentParser.read("-h"))
   {
      //help/usage
      argumentParser.getApplicationUsage()->write(std::cout);
      result = 0;
   } else {
      if (argumentParser.read("-m", argUint))
      {
         icPropertyInterface->setProperty("master_band", ossimString::toString(tempUint));
//         ic.setMasterBand(tempUint);
      }
      if (argumentParser.read("-s",argUint))
      {
         icPropertyInterface->setProperty("slave_band", ossimString::toString(tempUint));
      }
      if (argumentParser.read("-r",argDouble))
      {
         //TBD check boundaries : 0 to 1
         icPropertyInterface->setProperty("scale_ratio", ossimString::toString(tempDouble));
//         ic.setScaleRatio(tempDouble);
      }
       if (argumentParser.read("-d",argDouble))
      {
         //TBD check boundaries
//         ic.setCornerDensity(tempDouble);
         icPropertyInterface->setProperty("corner_density", ossimString::toString(tempDouble));
      }
      if (argumentParser.read("-c",argDouble))
      {
         //TBD check boundaries
//         ic.setMinCorrel(tempDouble);
         icPropertyInterface->setProperty("min_correl", ossimString::toString(tempDouble));
      }
      if (argumentParser.read("-t",argUint))
      {
         icPropertyInterface->setProperty("template_radius", ossimString::toString(tempUint));
//         ic.setTemplateRadius(tempUint);
      }
      if (argumentParser.read("-e",argDouble))
      {
         //TBD check boundaries
//         ic.setSlaveAccuracy(tempDouble);
         icPropertyInterface->setProperty("slave_accuracy", ossimString::toString(tempDouble));
      }
      if (argumentParser.read("-p",argString))
      {
         ossimString projArg(tempString);
         projArg = projArg.upcase();
         projArg = projArg.trim();
         if( (projArg != "S")&&
             (projArg != "M")&&
             (projArg != "G")  )
         {
            cerr<<PROGERR<<"::main() bad arg for option -p: "<<projArg<<" ,need M or S or G"<<endl;
            result = 1;
         }
         else
         {
//            ic.setProjectionType(projArg);
            icPropertyInterface->setProperty("projection_type", projArg);
         }
      }
      if (argumentParser.read("-o",argString))
      {
         icPropertyInterface->setProperty("output_filename", tempString);         
//         ic.setOutputName(tempString);
         
      }
      //read mandatory args : master and slave images
      //should only remain 2 args
      int nbargs = argumentParser.argc() - 1;
      if (nbargs == 2)
      {
         icPropertyInterface->setProperty("master_filename", ossimString(argumentParser.argv()[1]));
         icPropertyInterface->setProperty("slave_filename", ossimString(argumentParser.argv()[2]));
//          ic.setMaster(argumentParser.argv()[1]);
//          ic.setSlave( argumentParser.argv()[2]);

         theStdOutProgress.setFlushStreamFlag(true);
         icSource->addListener(&theStdOutProgress);
//         ic.addListener(&theStdOutProgress);

         result = (icProcessInterface->execute()) ? 0 : 4 ;

      }
      else
      {
         cerr<<PROGERR<<" bad number of non-optional args "<< nbargs << ",needs 2"<< endl;
         result = 3;
      }
   }

   return result;
}

//*******************************************************************
//
// Author:  Frederic Claudel
// to use after correl, with a tie points file
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
static const char* PROGERR = "ERROR modopt";

int
main(int argc, char *argv[])
{
   //error status : initially ok
   int result=0;

   //init OSSIM (prefs, elevation, plugins, etc)
   std::string  tempString;
   ossimArgumentParser::ossimParameter argString(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   //init with default parms
   ossimRefPtr<ossimObject> moObject = ossimObjectFactoryRegistry::instance()->createObject(ossimString("ossimModelOptimizer"));
   ossimPropertyInterface* moPropertyInterface = PTR_CAST(ossimPropertyInterface, moObject.get());
   ossimProcessInterface* moProcessInterface = PTR_CAST(ossimProcessInterface, moObject.get());
   if(!moObject.valid())
   {
      ossimNotify(ossimNotifyLevel_WARN) << "Unable to find object ossimModelOptimizer in registration plugin" << std::endl;
      exit(1);
   }

   ossimRefPtr<ossimProperty> modelDefinition = moPropertyInterface->getProperty("model_definition");
   ossimRefPtr<ossimProperty> outGeomFilename = moPropertyInterface->getProperty("geom_output_filename");

   //read args : options
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" optimizes a projection model using tie points");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" [options] <GML_tiepoints_file>");
   argumentParser.getApplicationUsage()->addCommandLineOption("-h",
      ossimString("display Usage/Help"));
   argumentParser.getApplicationUsage()->addCommandLineOption("-d",
      ossimString("<model_definition> : default=") + (modelDefinition.valid()?modelDefinition->valueToString():ossimString("")) );
   argumentParser.getApplicationUsage()->addCommandLineOption("-g",
      ossimString("<geom_file> : output geometry file, default=") + (outGeomFilename.valid()?outGeomFilename->valueToString():ossimString("")) );

   if (argumentParser.read("-h"))
   {
      //help/usage
      argumentParser.getApplicationUsage()->write(std::cout);
      result = 0;
   } else {
      if (argumentParser.read("-d", argString))
      {
         moPropertyInterface->setProperty("model_definition", tempString);
      }
      if (argumentParser.read("-g", argString))
      {
         moPropertyInterface->setProperty("geom_output_filename", tempString);
      }

      //read mandatory args : master and slave images
      //should only remain 2 args
      int nbargs = argumentParser.argc() - 1;
      if (nbargs == 1)
      {            
         ossimString tiePtPath(argumentParser.argv()[1]);

         moPropertyInterface->setProperty("gml_tieset_filename", tiePtPath);
         moProcessInterface->execute();
         //if (result==0) result = mo.loadGMLTieSet(tiePtPath) ? 0 : 5 ;
         //if (result==0) result = mo.execute()                ? 0 : 4 ;

      } else {
         cerr<<PROGERR<<" bad number of non-optional args "<< nbargs << ",needs 1"<< endl;
         result = 3;
      }
   }
   
   return result;
}

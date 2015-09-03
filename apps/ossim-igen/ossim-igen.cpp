//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: igen.cpp 13025 2008-06-13 17:06:30Z sbortman $



#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/init/ossimInit.h>
#include <ossim/parallel/ossimIgen.h>
#include <ossim/parallel/ossimMpi.h>
#include <iostream>
#include <string>
using namespace std;

static ossimTrace traceDebug("igen:debug");

int main(int argc, char* argv[])
{
   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << "entered main" << std::endl;
   }
   
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" takes a spec file as input and produces a product");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" [options] <spec_file>");
   argumentParser.getApplicationUsage()->addCommandLineOption("-t or --thumbnail", "thumbnail resolution");
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
 

   if(argumentParser.read("-h") ||
      argumentParser.read("--help")||
      argumentParser.argc() <2)
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      exit(0);
   }

   ossimRefPtr<ossimIgen> igen = new ossimIgen;
   double start=0, stop=0;
   
   ossimMpi::instance()->initialize(&argc, &argv);
   start = ossimMpi::instance()->getTime();

   ossimKeywordlist kwl;
   kwl.setExpandEnvVarsFlag(true);
   
   while(argumentParser.read("-t", stringParam)   ||
         argumentParser.read("--thumbnail", stringParam));
   
   if(ossimMpi::instance()->getRank() > 0)
   {
      // since this is not the master process
      // then it will set the keyword list form the master
      // so set this to empty
      //
      igen->initialize(ossimKeywordlist());
   }
   else if(argumentParser.argc() > 1)
   {
      if(kwl.addFile(argumentParser.argv()[1]))
      {
         if(tempString != "")
         {
            kwl.add("igen.thumbnail",
                    "true",
                    true);
            kwl.add("igen.thumbnail_res",
                    tempString.c_str(),
                    true);
         }
         else
         {
            kwl.add("igen.thumbnail",
                    "false",
                    true);
         }
         kwl.add("igen.thumbnail_res",
                 tempString.c_str(),
                 true);

         igen->initialize(kwl);
      }
   }

   try // Can throw exception:
   {
      igen->outputProduct();
   }
   catch(const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_FATAL)
         << "ossim-igen caught exception:\n"
         << e.what()
         << std::endl; 
   }
   
   if(ossimMpi::instance()->getRank() == 0)
   {
      stop = ossimMpi::instance()->getTime();
      ossimNotify(ossimNotifyLevel_NOTICE)
         << "Time elapsed: " << (stop-start)
         << std::endl;
   }

   ossimMpi::instance()->finalize();
   
   return 0;
}

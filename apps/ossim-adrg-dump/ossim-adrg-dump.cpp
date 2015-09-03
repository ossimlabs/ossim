//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Ken Melero
//
//*******************************************************************
//  $Id: ossim-adrg-dump.cpp 15122 2009-08-22 15:41:57Z dburken $

#include <cstdlib>
#include <iostream>
#include <iomanip>
using namespace std;

// Common includes.
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>

// Application specfic includes.
// #include "imaging/formats/adrg/ossimAdrgZdr.h"
#include <ossim/imaging/ossimAdrgHeader.h>

//***
// Static trace for debugging
//***
static ossimTrace traceDebug("adrg_header_dump:debug");

int main(int argc, char* argv[])
{

   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   argumentParser.getApplicationUsage()->
      setApplicationName(argumentParser.getApplicationName());

   argumentParser.getApplicationUsage()->
      setDescription(argumentParser.getApplicationName()+" This application dumps to screen the contents of an ADRG Header (.gen) file.");

   argumentParser.getApplicationUsage()->
      setCommandLineUsage(argumentParser.getApplicationName()+" [options] <.imgfile> ");

   // Parse command line arguments.
   // ossimInit::instance()->initialize(argc, argv);

   if (argumentParser.read("-h") ||
       argumentParser.read("--help")||(argumentParser.argc() != 2))
   {
      argumentParser.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_NOTICE));
      exit(0);
   }

   // ADRG header file.
   ossimFilename f = ossimFilename(argv[1]);

   if(traceDebug())
      cout << "Trying to open header:  " << f << endl;
        
   // Instantiate support data class to parse all needed header files.
   ossimAdrgHeader* theAdrgHeader = new ossimAdrgHeader(f);
//   ossimAdrgZdr* theAdrgHeader = new ossimAdrgZdr(f);

   // Check for errors.
   if(theAdrgHeader->errorStatus())
   {
      if (traceDebug())
      {
         cerr << "Error in ossimAdrg header detected. " << endl;
      }

      exit(1);
   }

   // Dump header to stdout.
   cout << *theAdrgHeader << endl;

   delete theAdrgHeader;
   exit(0);
}

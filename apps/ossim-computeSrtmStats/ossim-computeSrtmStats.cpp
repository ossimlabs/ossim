//**************************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Written by:   David Burken
// 
// Description:  This program will open an srtm file.  Compute statistics,
// then write a ossim meta data (omd) file.
//
//**************************************************************************
// $Id: computeSrtmStats.cpp 13025 2008-06-13 17:06:30Z sbortman $

#include <vector>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>

using namespace std;

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/init/ossimInit.h>
#include <ossim/support_data/ossimSrtmSupportData.h>

static const ossimTrace traceDebug(ossimString("computeStrmStats:debug"));

static void usage()
{
   ossimNotify(ossimNotifyLevel_INFO)
      << "\nNOTES:"
      << "\n- Output is to a \".omd\" file so if the image was"
      << " \"N27W081.hgt\" ouput will be a \"N27W081.omd\" file."
      << "\n- Example:  computeSrtmStats *.hgt"
      << endl;
}

int main(int argc, char* argv[])
{
   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   // Set the app name.
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());

   // Set usage option.
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");

   // Usage...
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" <srtm_file>\n");
   
   if (argumentParser.read("-h") ||
       argumentParser.read("--help")||(argumentParser.argc() < 2))
   {
      argumentParser.getApplicationUsage()->
         write(ossimNotify(ossimNotifyLevel_WARN));
      usage();
      exit(0);
   }
   
   argumentParser.reportRemainingOptionsAsUnrecognized();

   if(argc == 1)
   {
      argumentParser.getApplicationUsage()->
         write(ossimNotify(ossimNotifyLevel_WARN));
      usage();
      exit(1);
   }

   // Default variables

   ossim_uint32 fileCount = argc - 1;
   
   ossim_uint32 idx = 1;
   for(idx = 1; idx <= fileCount; ++idx)
   {
      // Get the srtm file name.
      ossimFilename srtmFile(argv[idx]);

      ossimNotify(ossimNotifyLevel_INFO)
         << "Processing srtm file:  " << srtmFile << endl;

      ossimSrtmSupportData ss;
      if (ss.setFilename(srtmFile, true) == true)
      {
         ossimFilename omdFile = srtmFile;
         omdFile.setExtension(ossimString("omd"));
         ossimNotify(ossimNotifyLevel_NOTICE)
            << "Wrote file:            " << omdFile.c_str() << std::endl;
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "Error processing:      " << srtmFile.c_str() << std::endl;
      }
      
   } // End of loop through file...

   return 0;
}

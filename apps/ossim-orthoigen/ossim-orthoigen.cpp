//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: ossim-orthoigen.cpp 3023 2011-11-02 15:02:27Z david.burken $

#include <iostream>
#include <cstdlib>
#include <list>
#include <fstream>
#include <iterator>
#include <iomanip>
using namespace std;

#include <ossim/parallel/ossimOrthoIgen.h>
#include <ossim/parallel/ossimMpi.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <sstream>

static ossimTrace traceDebug("orthoigen:debug");

//*************************************************************************************************
// USAGE
//*************************************************************************************************
static void usage()
{
   ossimNotify(ossimNotifyLevel_NOTICE) <<
      "Valid output writer types for \"-w\" or \"--writer\" option:\n\n" << ends;
   ossimImageWriterFactoryRegistry::instance()->
      printImageTypeList(ossimNotify(ossimNotifyLevel_NOTICE));
}

//*************************************************************************************************
// FINALIZE -- Convenient location for placing debug breakpoint for catching program exit.
//*************************************************************************************************
void finalize(int code)
{
   exit (code);
}

//*************************************************************************************************
// MAIN
//*************************************************************************************************
int main(int argc, char* argv[])
{
#if OSSIM_HAS_MPI
   ossimMpi::instance()->initialize(&argc, &argv);
   if (ossimMpi::instance()->getRank() == 0)
   {
      ossimNotify(ossimNotifyLevel_INFO)
         << "MPI running with "
         << ossimMpi::instance()->getNumberOfProcessors()
         << " processors..." << std::endl;
   }
#endif

   //---
   // Start the timer.  Note ossimMpi::instance()->getRank() works with or without
   // mpi being enabled.
   //---
   if (ossimMpi::instance()->getRank() == 0) ossimTimer::instance()->setStartTick();

   enum
   {
      OK    = 0,
      ERROR = 1
   };

   // Turn off the initial load of elevation.  This will be loaded if needed later.
   ossimInit::instance()->setElevEnabledFlag(false);
   
   ossimRefPtr<ossimOrthoIgen> orthoIgen = new ossimOrthoIgen;
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   ossimApplicationUsage* u = argumentParser.getApplicationUsage();
   u->setApplicationName(argumentParser.getApplicationName());
   u->setCommandLineUsage(     
      "\n\n"
      "    ossim-orthoigen [options] \"<input_image1>[|switches]\" \"[<input_image2>[|switches]\" ...] <output_image>\n"
      "\n"
      "    ossim-orthoigen [options] <input_spec>.src <output_image>");

   u->setDescription(
      "Creates a product image given an input specification as described below. There are two ways\n"
      "of specifying the input source list. Switches on explicit form are \n"
      "\n"
      "    <N>|<B1,B2,...>|<histo-op>|<ovr_path>. \n"
      "\n"
      "The <histo-op> presently can be either \"auto-minmax\" or \"std-stretch-N\" where N=1, 2 or 3.\n"
      "The <input_spec>.src keyword list contains keyword entries in the form of \n"
      "\n"
      "    imageN.<keyword>: value\n"
      "\n"
      "Supported keywords are \"file\", \"entry\", \"rgb\", \"hist\", \"ovr\", with values \n"
      "identical to those specified in the explicit switch form.\n");

   orthoIgen->addArguments(argumentParser);
   u->addCommandLineOption("-h or --help","Display this information");
   u->addCommandLineOption(
      "--enable-entry-decoding","A filename can be separated by a | and a number (NO space).  \n"
      "Example: a.toc|0 will do entry 0 of a.toc file and on unix you might want to use a \\ since\n"
      "the separator is a pipe sign. Example: ./a.toc\\|0 will do entry 0 of a.toc.");

   if (argumentParser.read("-h") || argumentParser.read("--help") || (argumentParser.argc() == 1))
   {
      u->write(ossimNotify(ossimNotifyLevel_INFO));
      usage();
      ossimMpi::instance()->finalize();
      ossimInit::instance()->finalize();
      finalize(OK);
   }
   bool enableEntryDecoding = false;
   if(argumentParser.read("--enable-entry-decoding"))
   {
      enableEntryDecoding = true;
   }
   
   orthoIgen->initialize(argumentParser);
   argumentParser.reportRemainingOptionsAsUnrecognized();
   if (argumentParser.errors())
   {
      argumentParser.writeErrorMessages(ossimNotify(ossimNotifyLevel_WARN));
      ossimMpi::instance()->finalize();
      finalize(OK);
   }
   
   orthoIgen->clearFilenameList();
   orthoIgen->addFiles(argumentParser, enableEntryDecoding);

   int status = OK;

   try
   {
      orthoIgen->execute();
   }
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      status = ERROR;
   }
   
   if(ossimMpi::instance()->getRank() == 0)
   {
      ossimNotify(ossimNotifyLevel_NOTICE)
         << std::setiosflags(ios::fixed)
         << std::setprecision(3)
         << "Time elapsed : " << ossimTimer::instance()->time_s() 
         << std::endl;
   }
   
   ossimMpi::instance()->finalize();
   finalize(status); 
}

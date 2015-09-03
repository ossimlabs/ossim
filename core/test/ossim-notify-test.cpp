//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// ossimNotify functions test.
// 
//----------------------------------------------------------------------------
// $Id: ossim-notify-test.cpp 23285 2015-04-28 19:45:07Z dburken $

#include <iostream>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimNotify.h>

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   ossimFilename logFile;
   ossimGetLogFilename(logFile);

   cout << "log file after ossimInit::instance()->initialize: "
        << logFile << endl;

   if (logFile.size() == 0)
   {
      logFile = "/tmp/ossim-log.txt";
      ossimSetLogFilename(logFile);
   }

   ossimGetLogFilename(logFile);

   cout << "set logFile end of test: " << logFile << endl;

   // ossimSetError was hanging:
   cout << "Calling ossimSetError(...) ..." << endl;
   ossimSetError( ossimString("ossimTiffOverviewBuilder"),
                 ossimErrorCodes::OSSIM_WRITE_FILE_ERROR,
                 "%s file %s line %d\nError creating reduced res set!",
                 "MODULE",
                 __FILE__,
                 __LINE__);
   cout << "ossimSetError(...) returned..." << endl;
   
   return 0;
}

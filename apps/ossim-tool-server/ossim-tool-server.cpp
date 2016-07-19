//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/util/ossimToolServer.h>

int main(int argc, char *argv[])
{

   // Initialize ossim stuff, factories, plugin, etc.
   ossimInit::instance()->initialize(argc, argv);

   const char* DEFAULT_PORT = "ossimd";
   const char* portid = DEFAULT_PORT;
   if (argc > 1)
      portid = argv[1];

   ossimToolServer ots;
   ots.startListening(portid);

   return 0;
}

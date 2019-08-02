//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//     Author: oscar.kramer@maxar.com
//
//**************************************************************************************************

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimException.h>
#include <ossim/util/ossimRemapTool.h>
#include <ossim/init/ossimInit.h>
#include <getopt.h>
#include <iostream>

using namespace std;

void usage(char* argv0, int exitCode, std::string errMsg="")
{
   if (!errMsg.empty())
      std::cout<<"\n"<<errMsg<<std::endl;
   std::cout<<
      "\nPerforms remap to 8-bit including optional histogram stretch and saves the corresponding"
      "\nexternal geometry file. If output filename is omitted, the output will be <input-image>-remap.ext"
      "\n"
      "\nUsage: "<<argv0<<" [options] <input-image.ext> [<output-image>] "
      "\n"
      "\nOptions:"
      "\n  -e <N>    Entry index for multi-entry input files."
      "\n  -h        Dump this help."
      "\n  -n        Skip histogram stretch."
      "\n"<<std::endl;
   exit(exitCode);
}


int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->initialize(ap);
   ostringstream msg;
   int entryIndex=0, c=0;
   bool doHistoStretch=true;

   // Parse command line:
   while ((c = getopt(argc, argv, "e:hn")) != -1)
   {
      switch (c)
      {
      case 'e':
         entryIndex = atoi(optarg);
         break;
      case 'h':
         usage(argv[0], 0);
         break;
      case 'n':
         doHistoStretch = false;
         break;
      case '?':
         msg << "Unknown option '" << (char) optopt << "' specified.";
         usage(argv[0], 1, msg.str());
         break;
      default:
         abort();
      }
   }

   int numArgs = argc - optind;
   if (numArgs < 1)
      usage(argv[0], 1, "Input filename required.");

   ossimFilename inputFilename (argv[optind++]);
   ossimFilename outputFilename;
   if (numArgs > 1)
      outputFilename = argv[optind];

   try
   {
      // Constructor does everything or throws exception:
      ossimRemapTool ort (inputFilename, entryIndex, doHistoStretch, outputFilename);
   }
   catch (const ossimException& e)
   {
      std::cerr << e.what() << std::endl;
      return 1;
   }

   return 0;
}

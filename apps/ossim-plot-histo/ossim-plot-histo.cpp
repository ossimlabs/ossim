//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
//*******************************************************************
//  $Id: create_histo.cpp 11743 2007-09-19 19:57:31Z gpotts $


#include <ossim/ossimConfig.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimMultiResLevelHistogram.h>
#include <ossim/base/ossimMultiBandHistogram.h>
#include <ossim/base/ossimHistogram.h>
#include <ossim/imaging/ossimHistogramWriter.h>
#include <ossim/imaging/ossimImageHistogramSource.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/imaging/ossimImageSourceSequencer.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#ifdef WIN32
#define MY_POPEN(arg1, arg2) _popen(arg1, arg2)
#else
#define MY_POPEN(arg1, arg2) popen(arg1, arg2)
#endif

void usage()
{
   cout << "examples:\n"
	     << " ossim-plot-histo foo.tif\n"
	     << "            Will create a histogram for the input image and call\n"
	     << "            it foo.his, then will plot the histogram.\n\n"
        << " ossim-plot-histo foo.his\n"
        << "            Simply plots existing histogram.\n\n"
        << " ossim-plot-histo --with dots foo.his\n"
        << "            Uses specified symbol for data point. See \"gnuplot plot with\" "
        << "            documentation.\n\n"
        << "Note: gnuplot must be installed before using this utility."
        << endl;
}

void plotHistogram(const ossimFilename& histoFile, const ossimString& plotWith,
                   const ossimString& xticstr)
{
   // Open the histogram file:
   ossimRefPtr<ossimMultiResLevelHistogram> h = new ossimMultiResLevelHistogram();
   if (!h->importHistogram(histoFile))
   {
      cout << "Could not read the histogram file <"<<histoFile<<">" << endl;
      exit(1);
   }

   // Create temporary data file stream:
   FILE * gnuplotPipe = MY_POPEN ("gnuplot -persistent", "w");
   //FILE * gnuplotPipe = fopen("gnuplot.dat", "w");
   if (!gnuplotPipe)
   {
      cout << "Could not create temporary gnuplot pipe. "<< endl;
      exit(1);
   }

   if (!xticstr.empty())
   {
      ostringstream xticsCmd;
      xticsCmd<<"set xtics ("<<xticstr<<")";
      cout << "xticsCmd = "<<xticsCmd.str()<< endl;
      fprintf(gnuplotPipe, "%s \n",xticsCmd.str().c_str());
   }

   // Output histogram data to temporary x, y file:
   ossimString command ("plot '-' with ");
   command.append(plotWith);
   fprintf(gnuplotPipe, "%s \n", command.chars());
   ossim_uint32 band = 0;
   ossimRefPtr<ossimHistogram> histogram = h->getHistogram(band);
   int numBins = histogram->GetRes();
   const float* x = histogram->GetVals();
   const ossim_int64* y = histogram->GetCounts();
   for (int i=0; i<numBins; ++i)
   {
      fprintf(gnuplotPipe, "%f %i \n", x[i], y[i]);
   }
   fprintf(gnuplotPipe, "e");
   fclose(gnuplotPipe);
}

int main(int argc, char *argv[])
{
   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" Plots image histogram.");
   
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" <input_file>");
   
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help", "Display this information");
   argumentParser.getApplicationUsage()->addCommandLineOption("--with <symbol>", "See \"gnuplot plot with\" documentation");
   argumentParser.getApplicationUsage()->addCommandLineOption("--xtics (<string>)", "See \"gnuplot plot xtics\" documentation");
   
   ossim_uint32 maxLevels = 1;
   ossim_int32 entry = -1;
   ossimFilename plotFile;
   
   ossim_float64 minValueOverride = ossim::nan();
   ossim_float64 maxValueOverride = ossim::nan();
   ossim_int32   numberOfBinsOverride = -1;

   bool fastMode = false;
   ossimString plotWith = "dots";
   ossimString xtics;
   
   if ( (argumentParser.read("-h")) || (argumentParser.read("--help")) )
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      usage();
      exit(0);
   }
   
   if(argumentParser.read("--with", stringParam))
   {
      plotWith = tempString;
   }
   if(argumentParser.read("--xtics", stringParam))
   {
      xtics = tempString;
      cout << "xtics = "<<xtics<< endl;
   }
   if (argumentParser.argc() == 2)
   {
      ossimFilename inputFile = argv[1];
      if(!inputFile.exists())
      {
         cerr << "Input file does not exist " << inputFile << endl;
         exit(1);
      }
      maxLevels = 1;
      if (inputFile.ext() != "his")
      {
         ostringstream cmd;
         cmd<<"ossim-create-histo "<<inputFile<<ends;
         system(cmd.str().c_str());
         inputFile.setExtension("his");
      }
      plotHistogram(inputFile, plotWith, xtics);
   }
   else
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      usage();
   }

   return 0;
}

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


void usage()
{
   cout << "examples:\n"
	<< " create_histo foo.tif\n"
	<< "            Will create a histogram for the input image and call\n"
	<< "            it foo.his\n\n"
        << " create_histo -i foo.tif -o foo.his\n"
        << "            this will compute a histogram from the input foo.tif\n"
        << "            and write it to foo.his\n\n"
        << " create_histo -p foo2.his -o foo.his\n"
        << "            will import the proprietary histogram file and output\n"
        << "            an ossim histogram file to foo.his\n\n"
        << "NOTE:  If no output is given it will default to output.his\n"
        << "       unless the special case of a single input image is given\n"
        << endl;
}

void importHistogram(const ossimString& importFile,
                     const ossimString&   outputFile)
{
   ossimRefPtr<ossimMultiResLevelHistogram> histo      = new ossimMultiResLevelHistogram;

   histo->importHistogram(importFile);
   ossimKeywordlist kwl;
   histo->saveState(kwl);
   kwl.write(outputFile.c_str());
}

void computeHistogram(const ossimString& imageOption,
                      const ossimString& outputOption,
                      ossim_float64 minValueOverride,
                      ossim_float64 maxValueOverride,
                      ossim_int32 numberOfBinsOverride,
                      ossim_int32 entryNumberOverride,
                      bool fastMode,
                      int maxResLevels=1
		      )
{
   theStdOutProgress.setFlushStreamFlag(true);
   ossimRefPtr<ossimImageHandler> handler = ossimImageHandlerRegistry::instance()->open(ossimFilename(imageOption));
   if(handler.valid())
   {
      ossimFilename histogramFile;
      std::vector<ossim_uint32> entryList;
      handler->getEntryList(entryList);

      if(entryNumberOverride >= 0)
      {
         std::vector<ossim_uint32>::iterator iter= std::find(entryList.begin(),
                                                             entryList.end(),
                                                             (ossim_uint32)entryNumberOverride);

         if(iter != entryList.end())
         {
            entryList.clear();
            entryList.push_back(entryNumberOverride);
         }
         else
         {
            ossimNotify(ossimNotifyLevel_WARN) << "Invalid entry number " << entryNumberOverride << " passed in.  Returning...." << std::endl;
            return;
         }
      }
      ossim_uint32 idx = 0;
      for(idx = 0; idx < entryList.size(); ++idx)
      {
         ossimNotify(ossimNotifyLevel_NOTICE)<<"entry " << entryList[idx] << std::endl;
         handler->setCurrentEntry(entryList[idx]);

         // Check handler to see if it's filtering bands.
         if ( handler->isBandSelector() )
         { 
            // Set output list to input.
            handler->setOutputToInputBandList();
         }
         
         ossimRefPtr<ossimProperty> histogramFilename = handler->getProperty("histogram_filename");
         if(histogramFilename.valid())
         {
            if (outputOption == "")
            {
              histogramFile = ossimFilename(histogramFilename->valueToString());
            }
            else
            {
              histogramFile = outputOption;
            }
            
            ossimRefPtr<ossimImageHistogramSource> histoSource = new ossimImageHistogramSource;
            ossimRefPtr<ossimHistogramWriter> writer = new ossimHistogramWriter;
            if(maxResLevels)
            {
               histoSource->setMaxNumberOfRLevels(maxResLevels);
            }
            else
            {
               histoSource->setMaxNumberOfRLevels(handler->getNumberOfDecimationLevels());
            }
            if(!ossim::isnan(minValueOverride))
            {
               histoSource->setMinValueOverride(minValueOverride);
            }
            if(!ossim::isnan(maxValueOverride))
            {
               histoSource->setMaxValueOverride(maxValueOverride);
            }
            if(numberOfBinsOverride > 0)
            {
               histoSource->setNumberOfBinsOverride(numberOfBinsOverride);
            }
            if (fastMode)
            {
               histoSource->setComputationMode(OSSIM_HISTO_MODE_FAST);
            }
            histoSource->connectMyInputTo(0, handler.get());
            histoSource->enableSource();
            writer->connectMyInputTo(0, histoSource.get());
            
            writer->setFilename(histogramFile);
            writer->addListener(&theStdOutProgress);
            writer->execute();
            writer=0;
         }
      }
   }
}

int main(int argc, char *argv[])
{
   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" creates a histogram from image.");
   
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" <args>");
   
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help", "Display this information");

   argumentParser.getApplicationUsage()->addCommandLineOption("-p", "<histogram_file> Imports imagelinks proprietary histogram_file.");
   
   argumentParser.getApplicationUsage()->addCommandLineOption("-i", "<image_file> Creates a histogram from the image_file.");
   
   argumentParser.getApplicationUsage()->addCommandLineOption("-l", "<res_level> Output the specified number of res levels default is, set to 0 if you want all(not supported for import histogram -i option");
   argumentParser.getApplicationUsage()->addCommandLineOption("-t", "<tiles for fast histogram>");
   argumentParser.getApplicationUsage()->addCommandLineOption("--create-histogram-fast", "Computes a histogram in fast mode which samples partial tiles.");
   
   argumentParser.getApplicationUsage()->addCommandLineOption("-o", "<output_file> Output the histogram to the indicated file.");

   argumentParser.getApplicationUsage()->addCommandLineOption("--min",  "Minimum value to use");
   argumentParser.getApplicationUsage()->addCommandLineOption("--max",  "Maximum value to use");
   argumentParser.getApplicationUsage()->addCommandLineOption("--bins", "Number of bins to use");
   argumentParser.getApplicationUsage()->addCommandLineOption("--entry", "entry number to use");

   argumentParser.getApplicationUsage()->addCommandLineOption("-f", "fast mode");
   
   ossimString importOption;
   ossimString imageOption;
   ossimString outputOption;
   ossim_uint32 maxLevels = 1;
   ossim_int32 entry = -1;
   
   ossim_float64 minValueOverride = ossim::nan();
   ossim_float64 maxValueOverride = ossim::nan();
   ossim_int32   numberOfBinsOverride = -1;

   bool fastMode = false;
   
   if ( (argumentParser.read("-h")) || (argumentParser.read("--help")) )
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      usage();
      exit(0);
    }
   
   if (argumentParser.argc() == 2)
   {
      ossimFilename inputImage = argv[1];
      imageOption = inputImage;
      if(!inputImage.exists())
      {
         cerr << "Image does not exist " << inputImage << endl;
         exit(1);
      }
      maxLevels = 1;
   }
   else
   {
      if(argumentParser.read("--min", stringParam))
      {
         minValueOverride = tempString.toDouble();
      }
      if(argumentParser.read("--max", stringParam))
      {
         maxValueOverride = tempString.toDouble();
      }
      if(argumentParser.read("--bins", stringParam))
      {
         numberOfBinsOverride = tempString.toInt32();
      }
      if(argumentParser.read("--bins", stringParam))
      {
         numberOfBinsOverride = tempString.toInt32();
      }
      if(argumentParser.read("--entry", stringParam))
      {
         entry = tempString.toInt32();
      }

      if(argumentParser.read("-o", stringParam))
      {
         outputOption = tempString;
      }
      
      if ( argumentParser.read("-f") ||
           argumentParser.read("--create-histogram-fast"))
      {
         fastMode = true;
      }

      if(argumentParser.read("-p", stringParam))
      {
         importOption = tempString;
      }
      if(argumentParser.read("-l", stringParam))
      {
         ossimString s = tempString;
         maxLevels = s.toUInt32();
      }
   }

   if((importOption != "") && (imageOption  != ""))
   {
      cout << "you can either import or create a histogram from an image but\n"
           << "we can't do both" << endl;
      exit(1);
   }

   if(importOption != "")
   {
      importHistogram(importOption, "");
   }
   else if(argc > 1)
   {
      ossim_uint32 idx = 1;
      while(argv[idx])
      {
         ossimNotify(ossimNotifyLevel_NOTICE)
            <<"file " << argv[idx] << std::endl;

         computeHistogram(ossimString(argv[idx]), outputOption, minValueOverride, maxValueOverride, numberOfBinsOverride, entry, fastMode, maxLevels);
         ++idx;
      }
   }
   else
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      usage();
   }

   return 0;
}

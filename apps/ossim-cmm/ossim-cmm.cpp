//**************************************************************************
//
// License:  See top level LICENSE.txt filel.
//
// Written by:   Garrett Potts
// 
// Description:  This program will force a recompute of Min/Max Pixel Values.
//
//**************************************************************************
// $Id: cmm.cpp 14515 2009-05-15 15:12:28Z dburken $

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>

using namespace std;

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageSourceFactory.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageSourceSequencer.h>
#include <algorithm>
#include <map>

static const ossimTrace traceDebug(ossimString("cmm:debug"));

//**************************************************************************
// usage()
//**************************************************************************
void usage()
{
   ossimNotify(ossimNotifyLevel_INFO)
      << "\nNOTES:"
      << "\n - Default output is to a \".omd\" file so if the image was"
      << " \"foo.tif\" you\n   will get a \"foo.omd\"."
      << "\n - If the .omd file exists already this will open it and add"
      << "\n   or, overwrite the min, max keywords only."
      << "\n - If the -p option is used no .omd file will be written."
      << endl;
   
   return;
}

bool isInImageEntryList(const std::vector<ossim_uint32>& imageEntryList,
                        ossim_uint32 entry)
{
   for(ossim_uint32 i=0; i<imageEntryList.size(); ++i)
   {
      if (imageEntryList[i] == entry)
      {
         return true;
      }
   }
   return false;
}

//**************************************************************************
// Main
//**************************************************************************
int main(int argc, char* argv[])
{
   //---
   // Get the arg count so we can tell if an arg was consumed by
   // ossimInit::instance()->initialize
   //---
   int originalArgCount = argc;  

   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   if ( (ap.argc() == 1) && (originalArgCount > 1) )
   {
      exit(0); // ossimInit consumed all options.
   }
   
   ap.getApplicationUsage()->setApplicationName(ap.getApplicationName());
   
   ap.getApplicationUsage()->setDescription(ap.getApplicationName()+" will force a recompute of Min/Max Pixel Values.");
   
   ap.getApplicationUsage()->setCommandLineUsage(ap.getApplicationName()+" <image_file>\nThis program will force a recompute of Min/Max Pixel Values.\n");
   
   ap.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   
   ap.getApplicationUsage()->addCommandLineOption("-p","print values to standard output");
   
   ap.getApplicationUsage()->addCommandLineOption("-e or --entry","Give the entry(zero based) to compute min / max for.\nNOTE: Option only valid with a single file.");

   ap.getApplicationUsage()->addCommandLineOption("-l or --list-entries","Lists the entries within the image and returns without doing anything else.");
   ap.getApplicationUsage()->addCommandLineOption("-o", "Write omd to file specified. If used on a multi-entry file, given \"foo.omd\" you will get: \"foo_e0.omd\"");
   ap.getApplicationUsage()->addCommandLineOption("--min","Hard coded min value for all bands");
   ap.getApplicationUsage()->addCommandLineOption("--max","Hard coded max value for all bands");
   ap.getApplicationUsage()->addCommandLineOption("--null","Hard coded null value for all bands");
   
   ap.getApplicationUsage()->addCommandLineOption("--reader-prop <string>", 
      "Adds a property to send to the reader. format is name=value");
   
   bool out_data = false;
   bool listEntriesFlag = false;
   double nullValue = ossim::nan();
   double minValue  = ossim::nan();
   double maxValue  = ossim::nan();
   bool hasNull = false;
   ossimFilename outputFile;
   
   std::vector<ossim_uint32> entryList;

   if (ap.read("-h") ||
       ap.read("--help")||(ap.argc() < 2))
   {
      ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_WARN));
      usage();
      exit(0);
   }
   if(ap.read("--min", stringParam))
   {
      minValue = tempString.toDouble();
   }
   if(ap.read("--max", stringParam))
   {
      maxValue = tempString.toDouble();
   }
   if(ap.read("--null", stringParam))
   {
      nullValue = tempString.toDouble();
      if ( !ossim::isnan(nullValue) )
      {
         hasNull = true;
      }
   }
   if(ap.read("-p"))
   {
      out_data = true;
   }
   if(ap.read("-l") || ap.read("--list-entries"))
   {
      listEntriesFlag = true; 
   }
   if( ap.read("-o", stringParam) )
   {
      outputFile = tempString.trim();
   }
   while(ap.read("-e", stringParam) ||
         ap.read("--entry", stringParam))
   {
      ossim_uint32 entryNumber = ossimString(tempString).toUInt32();
      
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "DEBUG:"
            << "\nentryNumber:  " << entryNumber
            << endl;
      }

      entryList.push_back(entryNumber);
   }

   std::map<ossimString, ossimString, ossimStringLtstr> readerPropertyMap;
   while(ap.read("--reader-prop", stringParam))
   {
      std::vector<ossimString> nameValue;
      ossimString(tempString).split(nameValue, "=");
      if(nameValue.size() == 2)
      {
         readerPropertyMap.insert( std::make_pair(nameValue[0], nameValue[1]) );
      }
   }  
   
   ap.reportRemainingOptionsAsUnrecognized();
   if(argc == 1)
   {
      usage();
      exit(1);
   }
   
   // Default variables

   ossim_uint32 fileCount = argc - 1;

   // Check for -e option with multiple images.
   if ( (fileCount > 1) && entryList.size() )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "Entry option only valid with a single file!"
         << endl;
      exit(1);
   }
   
   ossim_uint32 idx = 1;
   for(idx = 1; idx <= fileCount; ++idx)
   {
      // Get the image file name.
      ossimFilename img_file(argv[idx]);

      ossimNotify(ossimNotifyLevel_INFO)
         << "Processing image file:  " << img_file << endl;
      
      // Instantiate image to see if it is supported.
      ossimRefPtr<ossimImageHandler> ih;
      ih = ossimImageHandlerRegistry::instance()->open(img_file);      
      
      // Error checking.
      if (!ih.valid())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "Unsupported image file:  " << img_file << endl;
         continue; // Go to next file.
      }
      
      // Error checking.
      if (ih->getErrorStatus() == ossimErrorCodes::OSSIM_ERROR)
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "Error reading image:  " << img_file << endl;
         continue; // Go to next file.
      }

      // Set the reader properties if any.
      if ( readerPropertyMap.size() )
      {
         ossimPropertyInterface* pi = (ossimPropertyInterface*)ih.get();
         std::map<ossimString, ossimString, ossimStringLtstr>::iterator iter =
            readerPropertyMap.begin();
         while(iter != readerPropertyMap.end())
         {
            pi->setProperty(iter->first, iter->second);
            ++iter;
         }
      }

      // Get the list of entries that this image has.
      std::vector<ossim_uint32> imageEntryList;
      ih->getEntryList(imageEntryList);

      if (listEntriesFlag)
      {
         std::cout << "File " << img_file << " entries:\n";
         std::copy(imageEntryList.begin(),
                   imageEntryList.end(),
                   std::ostream_iterator<ossim_uint32>(std::cout, " "));
         std::cout << std::endl;
         continue; // Go to next file.
      }

      //---
      // If no entry (-e option) was given assume to do all entries.
      //---
      if(entryList.size() == 0)
      {
         for(ossim_uint32 imageEntryIdx = 0;
             imageEntryIdx < imageEntryList.size();
             ++imageEntryIdx)
         {
            entryList.push_back(imageEntryList[imageEntryIdx]);
         }
      }

      for(ossim_uint32 entryIdx = 0; entryIdx < entryList.size(); ++entryIdx)
      {
         if(isInImageEntryList(imageEntryList, entryList[entryIdx]) == false)
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << "Entry not found:  " << entryList[entryIdx] << endl;
            
            continue; // Go to next entry.
         }

         ossimNotify(ossimNotifyLevel_INFO)
         << "Computing min/max for entry number:  " << entryList[entryIdx]
         << endl;

         // Select the entry.
         ih->setCurrentEntry(entryList[entryIdx]);

         // Build the .omd file name.
         ossimFilename omd_file;
         if (outputFile.size())
         {
            omd_file = outputFile;            
         }
         else
         {
            omd_file = img_file;
         }

         if(imageEntryList.size() > 1)
         {
            ossim_uint32 currentEntry = ih->getCurrentEntry();
            omd_file.setExtension("");
            omd_file += "_e";
            omd_file += ossimString::toString(currentEntry);

            //---
            // Note:  Set extension was not used here deliberately to avoid
            // messing
            // up a MODIS file in hdf format with multiple '.'s in the file
            // name.
            //---
            omd_file += ".omd";
         }
         else
         {
            omd_file.setExtension("omd");
         }
      
         ossimRefPtr<ossimImageSourceSequencer> is =
            new ossimImageSourceSequencer(ih.get());

         //---
         // Note: getImageTileWidth/Height will return zero if the image is not
         // intenally tiles.
         //---
         ossimIpt tileWidthHeight(ih->getImageTileWidth(),
                                  ih->getImageTileHeight());
         
         if (!tileWidthHeight.x)
         {
            // make the sequencer read entire strips from the image handler
            // at a time.  This will speed up access time for strip images
            //
            tileWidthHeight.x = ih->getBoundingRect().width();
         }
         if(!tileWidthHeight.y)
         {
            tileWidthHeight.y = ih->getTileHeight();
         }

         if ( traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "Sequencer tile size: " << tileWidthHeight << endl;
         }
         
         is->setTileSize(tileWidthHeight);

         //---
         // Make arrays of doubles to hold the min/max values.
         // Initialize mins to default maxes and maxes to default mins to be
         // safe.
         //---
         ossim_uint32 i; // for numerous iterations...

         const ossimScalarType ST = ih->getOutputScalarType();
         
         const double DEFAULT_MIN = ossim::defaultMin(ST);
         const double DEFAULT_MAX = ossim::defaultMax(ST);
         
         const ossim_uint32 BANDS = ih->getNumberOfInputBands();

         vector<double> tmin(BANDS);
         vector<double> tmax(BANDS);
         vector<double> tnull(BANDS);
         for (i = 0; i < BANDS; ++i)
         {
            tmin[i] = DEFAULT_MAX;
            tmax[i] = DEFAULT_MIN;

            if ( hasNull )
            {
               // User passed in with --null option so set here.
               tnull[i] = nullValue;
            }
            else
            {
               tnull[i] = ih->getNullPixelValue(i);
            }
         }
         
         // Scan the image and compute the min and max.
         const double TOTAL_TILES = is->getNumberOfTiles();
         double tile_count = 0.0;
         ossimNotify(ossimNotifyLevel_INFO)
            << setiosflags(ios::fixed) << setprecision(0);

         if((ossim::isnan(minValue))||
            (ossim::isnan(maxValue)))
         {
            ossimRefPtr<ossimImageData> id = is->getNextTile();
            while(id.valid())
            {
               if ( hasNull )
               {
                  // Pass null to image data object so it doesn't get picked up as "min".
                  id->setNullPix( nullValue );
               }
               id->computeMinMaxPix(tmin, tmax);
               id = is->getNextTile();
               ++tile_count;
               ossimNotify(ossimNotifyLevel_INFO)
                  << "\r"  << setw(3)
                  << (tile_count / TOTAL_TILES * 100.0) << "%"
                  << flush;
            }
         }
         if(!ossim::isnan(minValue))
         {
            std::fill(tmin.begin(), tmin.end(), minValue);
         }
         if(!ossim::isnan(maxValue))
         {
            std::fill(tmax.begin(), tmax.end(), maxValue);
         }
         ossimNotify(ossimNotifyLevel_WARN)
            << "\r100%\nFinished..." << endl;
         
         ossimKeywordlist okwl(omd_file);
         
         for(i = 0; i < BANDS; ++i)
         {
            //---
            // Check to see if values got assigned.  If not flip to default and
            // issue a warning.
            //---
            if (tmin[i] == DEFAULT_MAX)
            {
               tmin[i] = DEFAULT_MIN;
               ossimNotify(ossimNotifyLevel_WARN)
                  << "cmm scan for min failed! Using default min."
                  << std::endl;
            }
            if (tmax[i] == DEFAULT_MIN)
            {
               tmax[i] = DEFAULT_MAX;
               ossimNotify(ossimNotifyLevel_WARN)
                  << "cmm scan for max failed! Using default max."
                  << std::endl;
            }
            
            ossimString band =
               ossimString("band")+ossimString::toString(i+1)+".";

            if (tmin[i] == DEFAULT_MAX)
            {
               tmin[i] = DEFAULT_MIN;
            }
            if (tmax[i] == DEFAULT_MIN)
            {
            }
            
            okwl.add(band + "min_value", tmin[i], true);
            okwl.add(band + "max_value", tmax[i], true);
            okwl.add(band + "null_value", tnull[i], true); 
            
            if(out_data)
            {
               ossimNotify(ossimNotifyLevel_INFO)
                  << setiosflags(ios::fixed) << setprecision(16)
                  << "band" << ossimString::toString(i+1) << ".min_value: "
                  << tmin[i]
                  << "\nband" << ossimString::toString(i+1) << ".max_value: "
                  << tmax[i]
                  << "\nband" << ossimString::toString(i+1) << ".null_value: "
                  << tnull[i] << endl;
            }
         }  // End of band loop.

         //----
         // ESH 05/2009 -- Fix for ticket #591:
         // Ingest command errors - when using the ew_ingest -r option
         //
         // Add missing info (number of bands, scalar type) to the .omd file
         //---
         okwl.add("number_bands",
                  BANDS,
                  true);
         okwl.add("scalar_type",
                  (ossimScalarTypeLut::instance()->getEntryString(ST)),
                  true);

         if(!out_data)
         {
            okwl.write(omd_file);
            ossimNotify(ossimNotifyLevel_INFO)
               << "wrote file:  " << omd_file << endl;
         }
         
      } // End of loop through entries of a file.
      
   } // End of loop through file...

   return 0;
}

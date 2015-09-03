//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Mingjie Su
//
//*******************************************************************
//  $Id: ossim-src2src.cpp 2644 2011-05-26 15:20:11Z oscar.kramer $

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimContainerProperty.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimTilingPoly.h>
#include <ossim/init/ossimInit.h>
#include <ossim/plugin/ossimSharedPluginRegistry.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimMapProjectionInfo.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/support_data/ossimInfoBase.h>
#include <ossim/support_data/ossimInfoFactoryRegistry.h>
#include <ossim/support_data/ossimSrcRecord.h>

#include <iostream>
#include <vector>
using namespace std;


//**************************************************************************
// usage()
//**************************************************************************
void usage()
{
   ossimNotify(ossimNotifyLevel_INFO)
      << " examples:\n\n"
      << "    ossim-create-src --tiling-template ./mytemplate.template ./myfile.tif ./outdir\n"
      << std::endl;;
}

//*****************************************************************************************
// Centralized point to capture all program exits (aids in debugging -- please leave)
//*****************************************************************************************
void finalize(int exit_code)
{
   exit(exit_code);
}

void parseSrcFile(const ossimFilename& srcFile, vector<ossimSrcRecord>& srcRecordFiles, ossimString& prefix)
{
   if (!srcFile.isReadable())
      return;

   ossimKeywordlist src_kwl (srcFile);
   unsigned int image_idx = 0;
   ossimString lookup = src_kwl.find("dem0.file");
   if (!lookup.empty())
   {
      prefix = "dem";
   }

   // Loop to read all image file entries:
   while (true)
   {
      ossimSrcRecord srcRecord(src_kwl, image_idx++, prefix);
      if (!srcRecord.valid()) 
         break;

      srcRecordFiles.push_back(srcRecord);
   }
}

ossimRefPtr<ossimTilingPoly> getTiling(const ossimFilename& templateFile)
{
   ossimKeywordlist templateKwl;
   if (!templateKwl.addFile(templateFile))
   {
      return 0;
   }

   ossimString prefix ("igen.tiling.");
   while (1)
   {
      if (templateKwl.find(prefix.chars(), "tile_source"))
      {
         break;
      }
      else
      {
         return 0;
      }

      // If we got here, then no matches were found in the template. Try again but without a prefix:
      if (prefix.empty())
         break;
      prefix.clear();
   }

   // Initialize the tiling object if enabled:
   ossimRefPtr<ossimTilingPoly> tiling = new ossimTilingPoly;
  
   if (!tiling->loadState(templateKwl, prefix))
   {
      return 0;
   }

   return tiling;
}

ossimDrect getRect(ossimRefPtr<ossimMapProjection> mapProj, 
   vector<ossimGpt> points)
{
   std::vector<ossimDpt> rectTmp;
   rectTmp.resize(points.size());

   for(std::vector<ossimGpt>::size_type index=0; index < points.size(); ++index)
   {
      mapProj->worldToLineSample(points[(int)index], rectTmp[(int)index]);
   }

   if (rectTmp.size() > 3)
   {
      ossimDrect rect2 = ossimDrect(rectTmp[0],
         rectTmp[1],
         rectTmp[2],
         rectTmp[3]);

      return rect2;
   }
   return ossimDrect();
}

//**************************************************************************
// Main Application
//**************************************************************************
int main(int argc, char *argv[])
{
   std::string tempString1;
   ossimArgumentParser::ossimParameter stringParam1(tempString1);
   std::string tempString2;
   ossimArgumentParser::ossimParameter stringParam2(tempString2);

   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   ossimApplicationUsage* appuse = argumentParser.getApplicationUsage();
   
   appuse->setApplicationName(argumentParser.getApplicationName());

   appuse->setDescription(argumentParser.getApplicationName()+
      " generate src files for polygons.");

   appuse->setCommandLineUsage(argumentParser.getApplicationName()+
      "ossim-create-src [--tiling-template] <full path to template file>");
                          
   appuse->addCommandLineOption("--tiling-template", "Specify an external file that contains tiling information.");

   ossimFilename templateFile = "";
  
   if(argumentParser.read("--tiling-template", stringParam1))
   {
      templateFile = ossimFilename(tempString1);
   }

   // End of arg parsing.
   argumentParser.reportRemainingOptionsAsUnrecognized();
   if ( argumentParser.errors() )
   {
      argumentParser.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
      finalize(1);
   }

   if (argc == 1)
   {
      ossimNotify(ossimNotifyLevel_WARN)<< "Must supply an input file or info option." << std::endl;
      finalize(0);
   }

   ossim_uint32 idx = 0;
   ossim_uint32 last_idx = argumentParser.argc()-1;
   ossimFilename inputSrcFile = "";
   ossimString prefixStr = "image";
   vector<ossimSrcRecord> inputFiles;
   while(argumentParser.argv()[idx] && (idx < last_idx))
   {
      ossimString file = argumentParser.argv()[idx];
      if (file.contains(".src"))
      {
         // input file spec provided via src file. Need to parse it:
         inputSrcFile = ossimFilename(file);
         if (!inputSrcFile.exists() )
         {
            ossimNotify(ossimNotifyLevel_WARN) << "ossim-src2src: file does not exist:  " << inputSrcFile
               << std::endl;
            finalize(1);
         }
         parseSrcFile(inputSrcFile, inputFiles, prefixStr);
      }
      ++idx;
   }

   // The last filename left on the command line should be the product filename:
   ossimFilename outputFileDir = argumentParser.argv()[last_idx];
   if (!outputFileDir.isDir())
   {
      outputFileDir = outputFileDir.path();
      if (!outputFileDir.exists() )
      {
         ossimNotify(ossimNotifyLevel_WARN) << "ossim-src2src: output directory does not exist:  " << outputFileDir
            << std::endl;
         finalize(1);
      }
   }

   if (!templateFile.exists())
   {
      ossimNotify(ossimNotifyLevel_WARN) << "ossim-src2src: template file does not exist::  " << templateFile
         << std::endl;
      finalize(1);
   }

   ossimRefPtr<ossimTilingPoly> polyTiling = getTiling(templateFile);
   if (!polyTiling.valid())
   {
      ossimNotify(ossimNotifyLevel_WARN) << "ossim-src2src: error generating tiling:  " << templateFile
         << std::endl;
      finalize(1);
   }

   // Open up src file to get coordinate info from each file listed in src file.
   ossim_float64 minLat = 90.0;
   ossim_float64 maxLat = -90.0;
   ossim_float64 minLon = 180.0;
   ossim_float64 maxLon = -180.0;

   ossimString tileName;
   ossimIrect clipRect;
   map<ossimString, ossimSrcRecord> infos;
   ossimRefPtr<ossimMapProjection> mapProj = 0;

   for (ossim_uint32 i = 0; i < inputFiles.size(); i++)
   {
      ossimFilename inputFile = inputFiles[i].getFilename();
      vector<ossim_uint32> entryList;
      ossimRefPtr<ossimImageHandler> ih = ossimImageHandlerRegistry::instance()->open(inputFile);
      if (ih.valid() )
      {
         if(inputFiles[i].getEntryIndex() > -1 )
         {
            entryList.push_back(inputFiles[i].getEntryIndex());
         }
         else
         {
            ih->getEntryList(entryList);
         }

         for(ossim_uint32 entryIdx = 0; entryIdx < entryList.size(); ++entryIdx)
         {
            // Establish the image handler for this particular frame. This may be just
            // the handler already opened in the case of single image per file:
            ossimImageHandler* img_handler = 0;
            if (entryList.size() == 1)
            {
               img_handler = ih.get();
            }
            else
            {
               img_handler = (ossimImageHandler*)ih->dup();
            }

            img_handler->setCurrentEntry(entryList[entryIdx]);

            ossimDrect outputRect = img_handler->getBoundingRect();

            ossimRefPtr<ossimImageGeometry> geom = img_handler->getImageGeometry();
            if(geom.valid())
            {
               ossimGpt ulg;
               ossimGpt llg;
               ossimGpt lrg;
               ossimGpt urg;

               geom->localToWorld(outputRect.ul(), ulg);
               geom->localToWorld(outputRect.ll(), llg);
               geom->localToWorld(outputRect.lr(), lrg);
               geom->localToWorld(outputRect.ur(), urg);

               //find min and max lat, lon for whole src file
               if (minLat > llg.lat)
               {
                  minLat = llg.lat;
               }

               if (minLon > llg.lon)
               {
                  minLon = llg.lon;
               }

               if (maxLat < urg.lat)
               {
                  maxLat = urg.lat;
               }

               if (maxLon < urg.lon)
               {
                  maxLon = urg.lon;
               }

               ossimString fileInfo = inputFile + "|" + ossimString::toString(entryIdx) + "|" + 
                  ossimString::toString(ulg.lon) + "," + ossimString::toString(ulg.lat) + "," + 
                  ossimString::toString(urg.lon) + "," + ossimString::toString(urg.lat) + "," + 
                  ossimString::toString(lrg.lon) + "," + ossimString::toString(lrg.lat) + "," + 
                  ossimString::toString(llg.lon) + "," + ossimString::toString(llg.lat);

               infos[fileInfo] = inputFiles[i];

               if (!mapProj.valid())
               {
                  ossimRefPtr<ossimProjection> proj = geom->getProjection();
                  if (proj.valid())
                  {
                     mapProj = PTR_CAST(ossimMapProjection, proj->dup());
                     if (!mapProj.valid()) //default it to ossimEquDistCylProjection
                     {
                        mapProj = new ossimEquDistCylProjection;
                        mapProj->setMetersPerPixel(geom->getMetersPerPixel());
                     }
                  }
               }
            }
            img_handler = 0;
         }
         ih = 0;
      }
      else
      {
         continue;
      }
   }
   inputFiles.clear();

   //get bounding rect for entire image
   vector<ossimGpt> points;
   if (points.size() == 0)
   {
      points.push_back(ossimGpt(maxLat, minLon));
      points.push_back(ossimGpt(maxLat, maxLon));
      points.push_back(ossimGpt(minLat, maxLon));
      points.push_back(ossimGpt(minLat, minLon));
   }

   if(mapProj.valid())
   {
      ossimGpt tie(maxLat, minLon);
      mapProj->setUlTiePoints(tie);
      polyTiling->initialize(*(mapProj.get()), getRect(mapProj, points));
   }

   while(polyTiling->next(mapProj, clipRect, tileName))
   {
      ossimFilename outSrc = outputFileDir + "/" + ossimFilename(tileName).noExtension() + ".src";
     
      map<ossimString, ossimSrcRecord>::iterator it = infos.begin();
      ossim_int32 index = 0;
      map<ossim_int32, ossimSrcRecord> srcList;
      while (it != infos.end())
      {
         ossimString fileInfo = it->first;
         vector<ossimString> fileInfos = fileInfo.split("|");
         ossim_int32 entryId = fileInfos[1].toInt();
         vector<ossimString> latlonInfo = fileInfos[2].split(",");

         double ulgLon = latlonInfo[0].toDouble();
         double ulgLat = latlonInfo[1].toDouble();
         double urgLon = latlonInfo[2].toDouble();
         double urgLat = latlonInfo[3].toDouble();
         double lrgLon = latlonInfo[4].toDouble();
         double lrgLat = latlonInfo[5].toDouble();
         double llgLon = latlonInfo[6].toDouble();
         double llgLat = latlonInfo[7].toDouble();

         points.clear();
         points.push_back(ossimGpt(ulgLat, ulgLon));
         points.push_back(ossimGpt(urgLat, urgLon));
         points.push_back(ossimGpt(lrgLat, lrgLon));
         points.push_back(ossimGpt(llgLat, llgLon));

         //get bounding for each image listed in src file
         ossimDrect rect = getRect(mapProj, points);

         //check if the rect of image intersect with the rect of tile
         if (rect.intersects(clipRect))
         {
            ossimSrcRecord srcRecord = it->second;
            srcRecord.setEntryIndex(entryId);
            srcList[index] = srcRecord;
            index++;
         }
         it++;
      }

      //write image files which intersect with the tile to a new src file
      if (srcList.size() > 0)
      {
         ofstream outputSrcFile(outSrc.c_str());
         map<ossim_int32, ossimSrcRecord>::iterator srcIt = srcList.begin();
         while (srcIt != srcList.end())
         {
            ossimString prefix = prefixStr + ossimString::toString(srcIt->first) + ".";

            ossimFilename newSrcFile = srcIt->second.getFilename();
            ossim_int32 entryId = srcIt->second.getEntryIndex();
            ossimFilename supportDir = srcIt->second.getSupportDir();
            ossimString hisOp = srcIt->second.getHistogramOp();
            vector<ossim_uint32> bands = srcIt->second.getBands();
            double weight = srcIt->second.getWeight();
            ossimFilename overviewPath = srcIt->second.getOverviewPath();
            ossimFilename hisPath = srcIt->second.getHistogramPath();
            ossimFilename maskPath = srcIt->second.getMaskPath();

            if (!newSrcFile.empty())
            {
               ossimString str = prefix + "file: " + newSrcFile;
               outputSrcFile << str << "\n";
            }

            if (entryId > -1)
            {
               ossimString str = prefix + "entry: " + ossimString::toString(entryId);
               outputSrcFile << str << "\n";
            }

            if (!supportDir.empty())
            {
               ossimString str = prefix + "support: " + supportDir;
               outputSrcFile << str << "\n";
            }

            if (!hisOp.empty())
            {
               ossimString str = prefix + "hist-op: " + hisOp;
               outputSrcFile << str << "\n";
            }

            if (bands.size())
            {
               ossimString str = "";
               for (ossim_uint32 bandIndex = 0; bandIndex < bands.size(); bandIndex++)
               {
                  if (bandIndex == (bands.size()-1))
                  {
                     str = str + ossimString::toString(bands[bandIndex]+1);
                  }
                  else
                  {
                     str = str + ossimString::toString(bands[bandIndex]+1) + ",";
                  }
               }
               str = prefix + "rgb: " + str;
               outputSrcFile << str << "\n";
            }

            if (weight > 0)
            {
               ossimString str = prefix + "opacity: " + ossimString::toString(weight);
               outputSrcFile << str << "\n";
            }

            if (!overviewPath.empty())
            {
               ossimString str = prefix + "ovr: " + overviewPath;
               outputSrcFile << str << "\n";
            }

            if (!hisPath.empty())
            {
               ossimString str = prefix + "hist: " + hisPath;
               outputSrcFile << str << "\n";
            }

            if (!maskPath.empty())
            {
               ossimString str = prefix + "mask: " + maskPath;
               outputSrcFile << str << "\n";
            }
            outputSrcFile << "\n";
            srcIt++;
         }
         outputSrcFile.close();
         srcList.clear();
      }
   }

   if (polyTiling.valid())
   {
      polyTiling = 0;
   }
   if (mapProj.valid())
   {
      mapProj = 0;
   }
   points.clear();
   infos.clear();
   
   finalize(0);
   return 0; // Never reached, but satisfies return type
}

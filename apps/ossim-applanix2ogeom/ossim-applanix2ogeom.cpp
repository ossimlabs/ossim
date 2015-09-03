
//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Applinix to ossim geometry.
// 
// Application to create an ossim geometry file from Applanix
// support data files.
//
//----------------------------------------------------------------------------
// $Id: applanix2ogeom.cpp 13025 2008-06-13 17:06:30Z sbortman $

#include <iostream>
#include <cstdlib>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/support_data/ossimApplanixEOFile.h>
#include <ossim/projection/ossimApplanixEcefModel.h>
#include <ossim/projection/ossimApplanixUtmModel.h>


static const ossimTrace traceDebug(ossimString("applanix2ogeom:debug"));

bool processImage(ossimApplanixEOFile& eo,
                  const ossimKeywordlist& kwl,
                  const ossimString& id,
                  const ossimFilename& outputDir);

int main(int argc, char* argv[])
{
   static const char MODULE[] = "applanix2ogeom";

   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   bool outputDirOverrideFlag = false;
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   argumentParser.getApplicationUsage()->setApplicationName(
      argumentParser.getApplicationName());
   
   argumentParser.getApplicationUsage()->setDescription(
      argumentParser.getApplicationName()+
      " Creates ossim geometry file from Applannix support data.");
   
   argumentParser.getApplicationUsage()->setCommandLineUsage(
      argumentParser.getApplicationName()+
      " <camera_file> <exterior_orientation_file> <imageToProcess> <optional_output_directory>");

   argumentParser.getApplicationUsage()->addUsageExplanation(
      ossimApplicationUsage::OSSIM_COMMAND_LINE_OPTION,
      std::string("camera_file"),
      std::string("Serial number specific keyword list with camera parameters."));
   
   argumentParser.getApplicationUsage()->addUsageExplanation(
      ossimApplicationUsage::OSSIM_COMMAND_LINE_OPTION,
      std::string("exterior_orientation_file"),
      std::string("Applanix exterior orientation file."));
   
   argumentParser.getApplicationUsage()->addUsageExplanation(
      ossimApplicationUsage::OSSIM_COMMAND_LINE_OPTION,
      std::string("imageToProcess"),
      std::string("Image to create geometry for. If not present then all geometries are created"));
   
   argumentParser.getApplicationUsage()->addUsageExplanation(
      ossimApplicationUsage::OSSIM_COMMAND_LINE_OPTION,
      std::string("option_output_directory"),
      std::string("Option directory to output geometry file(s) to.  If not set the imageToProcess directory will be used."));

   argumentParser.getApplicationUsage()->addCommandLineOption(
      "-h or --help", "Shows help");
   
   //---
   // Extract optional arguments.
   //---
   if(argumentParser.read("-h") || argumentParser.read("--help"))
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      exit(0);
   }
   
   argumentParser.reportRemainingOptionsAsUnrecognized();
   if (argumentParser.errors())
   {
      argumentParser.writeErrorMessages(std::cout);
      exit(1);
   }

   // cout << "argc:  " << argumentParser.argc() << endl;
   
   if(argumentParser.argc() < 4)
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      exit(1);
   }

//    if(argumentParser.argc() == 4)
//    {
//       if(!ossimFilename(argumentParser.argv()[3]).isDir())
//       {
//          ossimNotify(ossimNotifyLevel_WARN) << "for the 3 argument option the last argument must be a directory" << std::endl;
//          exit(0);
//       }
//    }
   ossimFilename cameraFile     = argumentParser.argv()[1];
   ossimFilename eoFile         = argumentParser.argv()[2];
   ossimFilename imageToProcess;
   ossimFilename outputDir;
   ossim_uint32 maxIdx = argumentParser.argc();
   if (argumentParser.argc() > 4)
   {
      outputDir = argumentParser.argv()[argumentParser.argc()-1];

      if (outputDir.isDir() == false) // doesn't exist
      {
         --maxIdx;
         if (outputDir.createDirectory() == false)
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << MODULE << " ERROR:"
               << "\nCould not create directory:  " << outputDir
               << endl;
            exit(1);
         }
      }
      outputDirOverrideFlag = outputDir.exists();
   }

   
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " DEBUG:"
         << "\ncamera file:       " << cameraFile
         << "\neo file:           " << eoFile
         << endl;
   }

   
   // Keyword list to pass to model loadState.
   ossimKeywordlist kwl;
   // Open up the exterior orientation file.
   ossimApplanixEOFile eo;
   if (eo.parseFile(eoFile) == false)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "Could not open:  " << eoFile << endl;
      exit(1);
   }
   eo.indexRecordIds();

   if(argc >4)
   {
      ossim_uint32 idx = 0;
      for(idx = 3; idx < maxIdx; ++idx)
      {
         kwl.clear();
         // Add the eo_file keyword to the keyword list.
         kwl.add("eo_file", eoFile.c_str(), true);
         
         // Add the eo_file keyword to the keyword list.
         kwl.add("camera_file", cameraFile.c_str(), true);
         kwl.add("compute_gsd_flag", true, true);
         
         // See if the ID is in the eo file.
         imageToProcess = argumentParser.argv()[idx];
         ossimString id = imageToProcess.fileNoExtension();
         ossimRefPtr<ossimApplanixEORecord> rec = eo.getRecordGivenId(id);
         if (!rec)
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << "applanix2ogeom ERROR:"
               << "\nMatching id for imageToProcess not found in eo file!"
               << "\nfile:  " << imageToProcess
               << "\nid:    " << id
               << "\nReturning..." << endl;
            exit(1);
         }
         kwl.add("image_id", id, true);
         if (!outputDirOverrideFlag)
         {
            outputDir = imageToProcess.path();
         }
         processImage(eo, kwl, id, outputDir);
      }
   }
   else if(argc <=4)
   {
      outputDir = argumentParser.argv()[argumentParser.argc()-1];

      if(!outputDir .isDir())
      {
         ossimFilename idFile = outputDir.file();
         outputDir = outputDir.path();
         idFile = idFile.setExtension("");
         const ossimRefPtr<ossimApplanixEORecord> rec = eo.getRecordGivenId(idFile);
         
         if(rec.valid())
         {
            // Add the eo_file keyword to the keyword list.
            kwl.add("eo_file", eoFile.c_str(), true);
            kwl.add("eo_id", idFile.c_str(), true);
            
            // Add the eo_file keyword to the keyword list.
            kwl.add("camera_file", cameraFile.c_str(), true);
            kwl.add("compute_gsd_flag", true, true);
            
            processImage(eo, kwl, idFile, outputDir);
         }
      }
      else
      {
         ossim_uint32 idx = 0;
         ossim_uint32 nRecords = eo.getNumberOfRecords();
         ossim_int32 idIdx = eo.getFieldIdx("ID");
         if(idIdx >= 0)
         {
            for(idx = 0; idx < nRecords; ++idx)
            {
               const ossimRefPtr<ossimApplanixEORecord> rec = eo.getRecord(idx);
               
               if(rec.valid())
               {
                  ossimString id = (*rec)[idIdx];
                  // Add the eo_file keyword to the keyword list.
                  kwl.add("eo_file", eoFile.c_str(), true);
                  kwl.add("eo_id", id.c_str(), true);
                  
                  // Add the eo_file keyword to the keyword list.
                  kwl.add("camera_file", cameraFile.c_str(), true);
                  kwl.add("compute_gsd_flag", true, true);
                  
                  processImage(eo, kwl, id, outputDir);
               }
            }
         }
         else
         {
            ossimNotify(ossimNotifyLevel_WARN) << "NO ID RECORD FOUND!!" << std::endl;
            exit(0);
         }
      }
   }
   
   
   return 0;
}

bool processImage(ossimApplanixEOFile& eo,
                  const ossimKeywordlist& kwl,
                  const ossimString& id,
                  const ossimFilename& outputDir)
{
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "applanix2ogeom:processImage DEBUG:"
         << "\nkwl:\n"
         << kwl
         << endl;
   }

//   ossimRefPtr<ossimApplanixEcefModel> model = new ossimApplanixEcefModel;
   ossimRefPtr<ossimProjection> model;

   if(eo.isUtmFrame())
   {
      model = new ossimApplanixUtmModel;
   }
   else
   {
     model = new ossimApplanixEcefModel;
   }
   if (model->loadState(kwl) == false)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "applanix2ogeom:processImage ERROR:"
         << "\nossimApplanixEcefModel::loadState failded for id:  "
         << id
         << endl;
      return false;
   }
      
   ossimFilename geomFile = id;
   geomFile.setExtension(ossimString(".geom"));
   if (outputDir != ossimFilename::NIL)
   {
      geomFile = outputDir.dirCat(geomFile);
   }

   ossimKeywordlist geomKwl;
   model->saveState(geomKwl);

   if (geomKwl.write(geomFile.c_str()) == false)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "applanix2ogeom:processImage ERROR:"
         << "\nCould not write file:  " << geomFile << endl;
      return false;
   }
   else
   {
      ossimNotify(ossimNotifyLevel_NOTICE)
         << "Wrote file:  " << geomFile << endl;
   }
   return true;
}

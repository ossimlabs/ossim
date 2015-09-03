//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author:  David Burken
//
// Description:
//
// Contains application definition "create_cg" app.
//
// It takes an image filename as input and creates a course grid model.
//
//*******************************************************************
//  $Id$

#include <ossim/ossimConfig.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimCoarseGridModel.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>

int main(int argc, char *argv[])
{
   //---
   // Get the arg count so we can tell if an arg was consumed by
   // ossimInit::instance()->initialize
   //---
   int originalArgCount = argc;
   
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   
   if ( (argumentParser.argc() == 1) && (originalArgCount > 1) )
   {
      exit(0); // ossimInit consumed all options.
   }

   ossimString appname = argumentParser.getApplicationName();
   ossimApplicationUsage* appuse = argumentParser.getApplicationUsage();
   appuse->setApplicationName(appname);
   appuse->setDescription(appname+" Creates OSSIM coarse grid geometry from rigorous sensor model.");
   appuse->setCommandLineUsage(appname+" [options] <image_file>");
   
   appuse->addCommandLineOption("-d or --support", 
      "<geom_path> -- Specifies a supplementary directory path to contain the geom and associated "
      "grid files. The default is to use the image directory.");

   appuse->addCommandLineOption("-e", 
      "<error> -- default is 0.1.  This is the maximum difference tolerated in pixels between the "
      "rigorous model and the computed coarse grid. The smaller this value, the bigger the "
      "resulting grid.");

   appuse->addCommandLineOption("-s",
      "<min_line_sample_spacing> -- Specifies the min line sample spacing for the coarse grid. "
      "Helps in converging. Basically will stop splitting if number of pixels per grid cell gets "
      "below this number.");

   appuse->addCommandLineOption("-o", 
      "<base_out_name> -- Specifies a new base output name for the geom and associated grid files. "
      "The default is to use the base name of the image file with .geom and .ocg extensions. "
      "If specified then the output is <base_out_name>.geom and <base_out_name>.ocg where the "
      ".ocg contains the coarse grid data. THIS IS NOT COMMON AND MAY PREVENT OSSIM FROM "
      "SUBSEQUENTLY LOCATING THE GEOMETRY FILE.");

   appuse->addCommandLineOption("-h or --help", "Shows help");

   if(argumentParser.read("-h") || argumentParser.read("--help") || (argc < 2))
   {
      appuse->write(std::cout);
      exit(0);
   }
   
   // The last argument is always the image filename:
   ossimFilename image_file = argv[argc-1];
   
   double error = 0.1;
   if(argumentParser.read("-e", error))
   {
      if (error < 0.0)
         error = 0.1;
   }

   ossim_int32 minSpacing = 100;
   if(argumentParser.read("-s", minSpacing))
   {
      if (minSpacing < 1)
         minSpacing = 100;
   }
   
   ossimFilename outputBaseName;
   argumentParser.read("-o", outputBaseName.string());

   ossimFilename outputPath;
   argumentParser.read("-d", outputPath.string());
   if (outputPath.empty())
   {
      argumentParser.read("--support", outputPath);
      if (outputPath.empty())
         outputPath = image_file.path();
   }

   // Establish image handler for access to rigorous model:
   ossimRefPtr<ossimImageHandler> ih = ossimImageHandlerRegistry::instance()->open(image_file);
   if (ih.valid())
   {
      // Instantiate the rigorous model:
      ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
      if(geom.valid() && geom->getProjection())
      {
         // Compute the coarse grid:
         ossimRefPtr<ossimCoarseGridModel> cg = new ossimCoarseGridModel;
         cg->setInterpolationError(error);
         cg->setMinGridSpacing(minSpacing);
         ossimNotify(ossimNotifyLevel_NOTICE)<<"\nComputing coarse grid..."<<endl;
         cg->buildGrid(ih->getBoundingRect(), geom.get());

         // If no basename was provided, use the defaults for geom file. PREFERRED:
         if (outputBaseName.empty())
            outputBaseName = ih->createDefaultGeometryFilename().file();

         // Save the coarse grid to appropriate location:
         ossimFilename filename (outputPath.dirCat(outputBaseName));
         filename.expand();
         ossimNotify(ossimNotifyLevel_NOTICE)<<"\nSaving OCG grid and geometry to <"
            <<filename.path()<<">..."<<endl;
         cg->saveCoarseGrid(filename); // this saves geom file as well
      }
   }

   return 0;
}

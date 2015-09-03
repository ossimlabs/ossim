//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ken Melero
//
// Merges input images together into output image.  Images must be of
// same scalar type and same number of bands.  If image width is different
// output image will be expanded to the largest input image width.
//
//*******************************************************************
//  $Id: band_merge.cpp 10695 2007-04-12 13:56:50Z gpotts $

#include <ossim/imaging/ossimJpegWriter.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimImageTypeLut.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimImageWriterFactory.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimBandMergeSource.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <iostream>
#include <exception>
static ossimTrace traceDebug("band_merge:main");

static void usage();
static void outputWriterTypes();

int main(int argc, char* argv[])
{
   static const char MODULE[] = "band_merge:main";

   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
 
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" merges band separate images to one image");
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   argumentParser.getApplicationUsage()->addCommandLineOption("-o or --create-overiew", "Creates and overview for the output image");
   argumentParser.getApplicationUsage()->addCommandLineOption("-w or --tile-width", "Defines the tile width for the handlers that support tiled output");
   
   if (traceDebug()) CLOG << " Entered..." << std::endl;

   if (argumentParser.read("-h") ||
       argumentParser.read("--help")||(argumentParser.argc() < 2))
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      usage(); // for writer output types
      exit(0);
   }

   // Keyword list to initialize image writers with.
   ossimKeywordlist kwl;
   const char* PREFIX = "imagewriter.";

   ossim_uint32 tile_width = 32;
   bool create_overview = false;

   if (argumentParser.read("-o") || argumentParser.read("--crate-overview"))
   {
      create_overview = true;
      std::cout << "\nOutput overview building enabled." << std::endl;
   }

   if (argumentParser.read("-w", stringParam) ||
       argumentParser.read("-tile-width", stringParam))
   {
      tile_width = ossimString(tempString).toInt();
      if ((tile_width % 16) != 0)
      {
         cerr << MODULE << " NOTICE:"
              << "\nTile width must be a multiple of 16!"
              << "\nDefaulting to 128"
              << std::endl;
         tile_width = 0;
      }
      std::cout << "Tile width set to:  " << tile_width << std::endl;
   }
   
   argumentParser.reportRemainingOptionsAsUnrecognized();
   
   // Three required args:  output_type, input file, and output file.
   if (argumentParser.errors())
   {
      argumentParser.writeErrorMessages(std::cout);
      exit(0);
   }
   if (argumentParser.argc() == 1)
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      usage(); // for writer output types
      exit(0);
   }
   
   ossim_uint32 number_of_source_images = argumentParser.argc() - 3;

   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\nargc:  " << argumentParser.argc()
           << "\nnumber_of_source_images:  " << number_of_source_images
           << "\ntile_width:  " << tile_width
           << "\ncreate_overview:  " << (create_overview?"true":"false")
           << std::endl;
   }

   ossimString output_type = argv[1];
   output_type.downcase();
   std::cout << "Output type:        " << output_type << std::endl;

   // Create the vector of image handlers.
   ossimConnectableObject::ConnectableObjectList ihs;
   for(ossim_uint32 h = 0; h < number_of_source_images; ++h)
   {
      ossimFilename f = argv[h + 2];
      std::cout << "Input_image[" << h << "]:     " << f << std::endl;
      ihs.push_back(ossimImageHandlerRegistry::instance()->open(f));
   }

   // Get the output file.
   ossimFilename output_file = argv[argumentParser.argc() - 1];
   std::cout << "Output file:        " << output_file << std::endl;   

   //---
   // Set the output writer type and the image type.
   //---
   kwl.add(PREFIX, ossimKeywordNames::TYPE_KW, output_type.c_str());

   ossimRefPtr<ossimBandMergeSource> bm = new ossimBandMergeSource(ihs);
   ossimRefPtr<ossimImageFileWriter> fileWriter =
      ossimImageWriterFactoryRegistry::instance()->createWriter(kwl, PREFIX);
   if(!fileWriter)
   {
      bm->disconnect();
      bm = 0;
      ossimConnectableObject::ConnectableObjectList::iterator i = ihs.begin();
      while (i != ihs.end())
      {
         (*i)->disconnect();
         (*i) = 0;
         ++i;
      }
      
      cerr << "Error making an image writer..."
           << "\nExiting application..." << std::endl;
      exit(1);
   }
   
   // Write out a geometry file for new image.
   ossimKeywordlist bm_geom;
   ossimRefPtr<ossimImageGeometry> geom = bm->getImageGeometry();
   geom->saveState(bm_geom);
   
   ossimFilename geom_file = output_file;
   geom_file.setExtension("geom");
   bm_geom.write(geom_file);
   
   fileWriter->connectMyInputTo(0, bm.get());
   
   if (tile_width)
   {
      // Set the tile size...
      fileWriter->setTileSize(ossimIpt(tile_width, tile_width));
   }
   
   fileWriter->open(output_file);
   
   ossimStdOutProgress prog(2);
   fileWriter->addListener(&prog);
   
   fileWriter->setAreaOfInterest(bm->getBoundingRect());

   if( fileWriter->canCastTo("ossimTiffWriter") )
   {
//      ossimTiffWriter* twriter = PTR_CAST(ossimTiffWriter, fileWriter);
      
      if(fileWriter.valid())
      {
         try
         {
            fileWriter->execute();
         }
         catch(std::exception& e)
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "band_merge: ERROR exception caught:\n"
               << e.what()
               << std::endl;
         }
         catch (...)
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "band_merge: ERROR - Unknown exception caught!"
               << std::endl;
         }
      }
   }
   else if(fileWriter->canCastTo("ossimJpegWriter"))
   {
      ossimRefPtr<ossimScalarRemapper> remapper = NULL;
      
      if(PTR_CAST(ossimJpegWriter, fileWriter.get()) &&
         (bm->getOutputScalarType() != OSSIM_UCHAR))
      {
         remapper = new ossimScalarRemapper(bm.get(), OSSIM_UCHAR);
         remapper->initialize();
         fileWriter->connectMyInputTo(0, remapper.get());
      }
      fileWriter->initialize();
      try
      {
         fileWriter->execute();
      }
      catch(std::exception& e)
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "band_merge: ERROR exception caught:\n"
            << e.what()
            << std::endl;
      }
      catch (...)
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "band_merge: ERROR - Unknown exception caught!"
            << std::endl;
      }
   }
   else
   {
      try
      {
         fileWriter->execute();
      }
      catch(std::exception& e)
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "band_merge: ERROR exception caught:\n"
               << e.what()
               << std::endl;
         }
      }
      catch (...)
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "band_merge: ERROR - Unknown exception caught!"
               << std::endl;
         }
      }
   }
   
   if (create_overview == true)
   {
      fileWriter->writeOverviewFile();
   }
   fileWriter->disconnect();
   fileWriter = 0;
   bm = 0;
   ossimConnectableObject::ConnectableObjectList::iterator it = ihs.begin();
   while (it != ihs.end())
   {
      (*it)->disconnect();
      (*it) = 0;
      ++it;
   }

   exit(0);
}

void usage()
{
   std::cout
      << "band_merge: Band Merge\n"
      << "Usage:\n"
      << "band_merge [-h][-o][-w tile_width] <output_type> <input_file1> "
      << "<input_file2> ... <output_file>"
      << "\n\nOptional argument list:"
      << "\n -h  -- display Usage/Help"
      << "\n -o  -- create an ossim overview file with output file"
      << "\n -w  -- tile width (default = 32)"
      << "\n"
      << "\nNOTES:"
      << "\n\"-w\" option only valid with tiff type."
      << "\n\"Tile width must be a multiple of 32."
      << "\nValid output writer types:"
      << "\n";
   
   outputWriterTypes();
}

void outputWriterTypes()
{
   ossimImageTypeLut lut;

   ossim_uint32 index = 0;
   ossimString type = lut.getTableIndexString(index);
   
   while (type.size())
   {
      std::cout << "\t" << type << std::endl;
      ++index;
      type = lut.getTableIndexString(index);
   }
   std::cout << std::endl;
}

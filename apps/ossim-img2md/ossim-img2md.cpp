//----------------------------------------------------------------------------
// Copyright (c) 2005, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:  Application to output metadata given meta data type,
// source image, output file.  Additional keywords can be passed to writer
// via the "-t" option which takes a keyword list.
//
//----------------------------------------------------------------------------
// $Id: img2md.cpp 9111 2006-06-14 12:34:08Z gpotts $

#include <iostream>
#include <iterator>
using namespace std;

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageMetaDataWriterRegistry.h>
#include <ossim/imaging/ossimMetadataFileWriter.h>

static void outputWriterTypes();
static void usage();

int main(int argc, char* argv[])
{
   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   
   argumentParser.getApplicationUsage()->setApplicationName(
      argumentParser.getApplicationName());
   
   argumentParser.getApplicationUsage()->setDescription(
      argumentParser.getApplicationName()+" outputs metadata for an image.");
   
   argumentParser.getApplicationUsage()->setCommandLineUsage(
      argumentParser.getApplicationName()+" [options] <metadata_writer> <input_file> <output_file>");

   argumentParser.getApplicationUsage()->addCommandLineOption(
      "-h or --help", "Shows help");

   argumentParser.getApplicationUsage()->addCommandLineOption(
      "-t or --template", "Template to pass to meta data writer.");
   
   //---
   // Extract optional arguments.
   //---
   ossimFilename templateFile = ossimFilename::NIL;
        
   if(argumentParser.read("-h") || argumentParser.read("--help"))
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      usage(); // For metadata writer types.
      exit(0);
   }
   if( argumentParser.read("-t", stringParam) ||
       argumentParser.read("--template", stringParam) )
   {
      templateFile = tempString.c_str();
   }
   
   argumentParser.reportRemainingOptionsAsUnrecognized();
   if (argumentParser.errors())
   {
      argumentParser.writeErrorMessages(std::cout);
      exit(1);
   }

   if(argumentParser.argc() < 4)
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      usage(); // For metadata writer types.
      exit(1);
   }
   
   ossimString   metaDataWriterType = argumentParser.argv()[1];
   ossimFilename imageFile          = argumentParser.argv()[2];
   ossimFilename outputFile         = argumentParser.argv()[3];
   
   ossimRefPtr<ossimImageHandler> ih =
      ossimImageHandlerRegistry::instance()->open(imageFile);
   if (!ih)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "Could not open:  " << imageFile << endl; 
   }

   metaDataWriterType.downcase();
   
   ossimRefPtr<ossimMetadataFileWriter> mw =
      ossimImageMetaDataWriterRegistry::instance()->createWriter(
         metaDataWriterType);
   if (!mw)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "Could not create meta data writer of type:  "
         << metaDataWriterType << endl;
      exit(1);
   }

   ossimKeywordlist kwl;
   kwl.add(ossimKeywordNames::FILENAME_KW, outputFile.c_str());

   if (templateFile != ossimFilename::NIL)
   {
      if (kwl.addFile(templateFile) == false)
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "Could not load template file:  "
            << templateFile << endl;
         exit(1);
      }
   }

   mw->loadState(kwl);

   mw->connectMyInputTo(ih.get());

   if (mw->execute() == true)
   {
      ossimNotify(ossimNotifyLevel_NOTICE)
         << "Wrote file:  " << outputFile.c_str() << endl;
   }
   
   return 0;
}

void usage()
{
   cout << "\nNOTES:"
        << "\nValid metadata writer types:"
        << "\n";
   outputWriterTypes();
}

void outputWriterTypes()
{
   std::vector<ossimString> metadatatypeList;
   
   ossimImageMetaDataWriterRegistry::instance()->getMetadatatypeList(
      metadatatypeList);
   
   std::copy(metadatatypeList.begin(),
             metadatatypeList.end(),
             std::ostream_iterator<ossimString>(std::cout, "\t\n"));
}

//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Application to scan image and attemp to detect the valid image vertices and
// write them to a keyword list file for later use.  Note that if input
// file is "foo.tif" this will create "foo_vertices.kwl".
//
//*******************************************************************
//  $Id: extract_vertices.cpp 10074 2006-12-13 13:27:53Z dburken $

#include <iostream>
using namespace std;

#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimVertexExtractor.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimTrace.h>

static ossimTrace traceDebug("extract_vertices:main");

void usage()
{
   cout << "\nextract_vertices <image_file> <optional_output_file>"
        << "\nNOTE:\n"
        << "   Scans the image, extracts vertices and writes results to"
        << " a keyword list.\n"
        << "   The optional_output_file parameter specifies the path for the"
        << " extracted\n"
        << "   vertices.  If not specified, the name of the image_file with "
        << "\"_vertices.kwl\"\n"
        << "   appended.  So if image = \"foo.tif\" then"
        << "  results file = \"foo_vertices.kwl\".\n" << endl;
}

int main(int argc, char *argv[])
{
   static const char MODULE[] = "extract_vertices:main";

   if (traceDebug()) CLOG << " Entered..." << endl;
   
   ossimInit::instance()->initialize(argc, argv);

   if (argc != 2)
   {
      usage();
      exit(0);
   }

   ossimFilename input_file = argv[1];

   // Get an image handler for the input file.
   ossimRefPtr<ossimImageHandler> ih
      = ossimImageHandlerRegistry::instance()->open(input_file);
   
   if (!ih)
   {
      cout << "ERROR: Unsupported image file: " << input_file
           << "\nExiting application." << endl;
      exit(0);
   }

   // Check for errors.
   if (ih->getErrorStatus() == ossimErrorCodes::OSSIM_ERROR)
   {
      cerr << "ERROR: Unable to read image file: " << input_file
           << "\nExiting application." << endl;
      ih = 0;
      exit(1);
   }

   // Initialize the image handler.
   ih->initialize();

   ossimFilename output_file;
   // Get output path from command line.
   if(argc>2)
   {
     output_file = argv[2];
   }
   // Create the output file name from the input file name.
   else
   {
     output_file = input_file.path();
     output_file = output_file.dirCat(input_file.fileNoExtension());
     output_file += "_vertices.kwl";
   }

   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\nInput file:   " << input_file
           << "\nOutput_file:  " << output_file
           << endl;
   }

   // Give the image handler to the vertex extractor as an input source.
   ossimRefPtr<ossimVertexExtractor> ve = new ossimVertexExtractor(ih.get());

   // Set the file name.
   ve->setOutputName(output_file);

   // Set the area of interest to the full bounding rect of the source.
   ve->setAreaOfInterest(ih->getBoundingRect(0));

   // Add a listener for the percent complete to standard output.
   ossimStdOutProgress prog(0, true);
   ve->addListener(&prog);
   
   // Start the extraction...
   ve->execute();
   ih = 0;
   ve->disconnect();
   ve = 0;
   if (traceDebug()) CLOG << " Exited..." << endl;
   
   exit(0);
}

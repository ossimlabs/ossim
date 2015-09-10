/*!
 *
 * OVERVIEW:
 *
 * This app extends image_copy and adds an additional filter
 * to copy the first band from the input.
 *
 * PURPOSE:
 *
 * Learn how to use your first image filter ossimBandSelector.
 *
 */

// iostream is used for general output
//
#include <iostream>
#include <iterator>
#include "base/data_types/ossimFilename.h"
#include "base/data_types/ossimString.h"

// This is the majic number factory.  All loaders are registered to this factory.
// it will lookp through all factories to try to open the passed in image file
//
#include "imaging/factory/ossimImageHandlerRegistry.h"

// Base pointer for the passed back object type
//
#include "imaging/formats/ossimImageHandler.h"

// Base pointer our file writer
//
#include "imaging/formats/ossimImageFileWriter.h"

#include "imaging/factory/ossimImageWriterFactoryRegistry.h"

#include "imaging/tile_sources/ossimBandSelector.h"

// this is the most important class and is called as the first line of all applications.
// without this alll the important factories are not created.
//
#include "init/ossimInit.h"

using namespace std;

void usage();
void printOutputTypes();

int main(int argc, char* argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   if(argc!=4)
   {
      usage();
   }
   else
   {
      // try to open up the passed in image
      //
      ossimImageHandler *handler   = ossimImageHandlerRegistry::instance()->open(ossimFilename(argv[2]));

      // try to create a writer for the output image type.
      //
      ossimImageFileWriter* writer = ossimImageWriterFactoryRegistry::instance()->createWriter(ossimString(argv[1]));

      
      if(!handler)
      {
         cout << "Unable to open input image: "<< argv[2] << endl;
         return 1;
      }
      if(!writer)
      {
         cout << "Unable to create writer of type: " << argv[1] << endl;
         return 1;
      }
      ossimBandSelector* selector = new ossimBandSelector;

      // specify a lookup table that maps
      // output band list to each input band
      // the first output band is mapped to
      // the first input band 0.
      //
      vector<ossim_uint32> outputBandList(1);
      
      outputBandList[0] = 0;
      selector->setOutputBandList(outputBandList);

      selector->connectMyInputTo(0, handler);
      
      // specify the output file name
      writer->setFilename(ossimFilename(argv[3]));
      writer->connectMyInputTo(0, selector);
      writer->execute();
      
      delete writer;
      delete handler;
   }

   return 0;
}


void usage()
{
   cout << "image_copy <output_type> <input filename> <output filename>" << endl
        << "where output types are: " << endl;
   printOutputTypes();
}

void printOutputTypes()
{
    std::vector<ossimString> outputType;
   
    ossimImageWriterFactoryRegistry::instance()->getImageTypeList(outputType);
    std::copy(outputType.begin(),
              outputType.end(),
              ostream_iterator<ossimString>(cout, "\n"));
}

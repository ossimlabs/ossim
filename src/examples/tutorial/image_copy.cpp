/*!
 *
 * OVERVIEW:
 *
 * this app is a simple image copy. It will use
 * the image writer factory to instantiate a registered
 * writer and connect to the input and execute the writer.
 *
 * The writers can be instantiatd by class name or by output image
 * type.  
 *
 * PURPOSE:
 *
 * Learn how to construct a writer from a factory. Connect the
 * writer to the image loadeer/reader and output the image.
 *
 */

// iostream is used for general output
//
#include <iostream>
#include <iterator>

#include "base/data_types/ossimFilename.h"
#include "base/data_types/ossimString.h"

// this is for implementing progress output.
//
#include "base/common/ossimStdOutProgress.h"

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
         delete handler;
         return 1;
      }

      // specify the output file name
      writer->setFilename(ossimFilename(argv[3]));

      // within OSSIM we have a concept of inputs that can be connected together
      // writer have only 1 input and we index them starting from 0. If we only
      // supplied the second argument it would find the first available input and connect
      // it to that slot.  Here, we explicitly say connect the handler to slot 0 of the
      // writer.
      //
      writer->connectMyInputTo(0, handler);

      // Optionally we can add listeners to listen for certain events.
      //
      // all writers should execute event processing and generate a progress event.
      // we have a default event listener that listens for this event and can be used
      // to output the progress of the exected process.
      //
      // the first argument is to specify the precision of the percent complete
      // output, here we default to 0 for whole number outputs. The second argument
      // specifies to flush the stream on each print If you don't want progress
      // output then don't add this listener
      //
      // the defalut standard out listener is found in base/common/ossimStdOutProgress.h"
      // 
      ossimStdOutProgress progress(0, true);
      writer->addListener(&progress);

      // execute the writer.  Writer will start sequencing through
      // all the tiles from the input and output it to disk.
      //
      writer->execute();

      // now to be on the safe side we will remove any added listeners.
      writer->removeListener(&progress);
      
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

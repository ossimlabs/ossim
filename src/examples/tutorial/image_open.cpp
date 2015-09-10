/*!
 *
 * OVERVIEW:
 *
 * this app is a simple image open.
 * It should open any image supported by the ossim library.
 * Once the image is open it prints out some general information
 * about the image.
 *
 * PURPOSE:
 *
 * Learn how to open an image and query attributes.  Also learn which
 * headers are required to perform these tasks.
 *
 */

// iostream is used for general output
//
#include <iostream>

// within this program ossimCommon is used for ossimGetScalarSizeInBytes.
// This header will have some common globabl inline and non-inline functions
//
#include "base/common/ossimCommon.h"


#include "base/data_types/ossimFilename.h"

// used to get the string name of the scalar type for the handler.  The scalar
// type specifies if its unsigned char, float, double, ...etc
//
#include "base/misc/lookup_tables/ossimScalarTypeLut.h"

// This is the majic number factory.  All loader are registered to this factory.
// it will lookp through all factories to try to open the passed in image file
//
#include "imaging/factory/ossimImageHandlerRegistry.h"

// Base pointer for the passed back object type
//
#include "imaging/formats/ossimImageHandler.h"

// this is the most important class and is called as the first line of all applications.
// without this alll the important factories are not created.
//
#include "init/ossimInit.h"

using namespace std;

int main(int argc, char* argv[])
{
   ossimInit::instance()->initialize(argc, argv);
   
   if(argc == 2)
   {
      // call the registries open and let it traverse through all registered
      // factories to open up an image.
      //
      ossimImageHandler *handler = ossimImageHandlerRegistry::instance()->open(ossimFilename(argv[1]));

      if(handler)
      {
         // includes full resolution so if there are no overviews this value should be
         // 1. Note: resolution levels or overviews or indexed from 0 being full
         // resolution to (number of decimations or overviews -1..
         //
         // Overviews are very import for resampling images to different resolutions efficiently.
         // The overviews are generally power of 2 decimations.  Assume we have an  image that has 
         // full resolution of 1024x1024 in size. When we are talking about resolution 0 then
         // we are talking about the full res 1024x1024, but when we are talking
         // about resolution level 1 then we are looking at the start of the overview and referes to
         // a decimation of 2^1 which is 512x512 image.  So when we ask for resolution level N then
         // we are talking about a decimation of 2^N
         //
         // So again, when we query the number of decimation levels this includes resolution layer 0.
         //
         int overviews     = handler->getNumberOfDecimationLevels();

         // the image bounds can take an argument to specify which resolution I would like to get
         // the bounding rect of.  By default if no argument is given it assumes full resolution or
         // resolution 0.
         //
         ossimIrect bounds = handler->getBoundingRect(0);


         // scalar type cooresponds to the pixel radiometry type.  The pixel can be of precision
         // float, double, unsigned char, unsigned short, signed short data values. Look at
         // ossimScalarType enumeration found in base/common/ossimConstants.h for supporting types
         // 
         ossimScalarType scalarType = handler->getOutputScalarType();
         
         // This is a utility class that maps enumeration values to strings.  So we will use this
         // when we query the scalar type from the image and wish to print the text version of it.
         //
         //
         ossimScalarTypeLut* lut = ossimScalarTypeLut::instance();
         
         cout <<"filename        = " << handler->getFilename() << endl
              << "width          = " << bounds.width() << endl
              << "height         = " << bounds.height() << endl
              << "overview count = " << (overviews-1) << endl
              << "scalar type    = " << lut->getEntryString(handler->getOutputScalarType()) << endl

            // ossimGetScalarSizeInBytes is a utility functions found in base/common/ossimCommon.h
            // it will return the byte size of the scalar type.
            //
              << "pixel size     = " << ossimGetScalarSizeInBytes(scalarType) << " byte(s)" <<endl
              << "Handler used   = " << handler->getClassName() << endl;

         delete handler;
      }
      else
      {
         cout << "Unable to open image = " << argv[1] << endl;
      }
   }
   else
   {
      cout << "usage: open_image <file name>" << endl;
   }

   // call the finalize so the ossim can cleanup if needed.
   ossimInit::instance()->finalize();
   
   return 0;
}

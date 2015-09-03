/*!
 *
 * OVERVIEW:
 *
 * We will retrieve information from the input source and manipulate the data.
 * All filters will have a data access point called getTile that can take a
 * rectangle/region of interest and a resolution level.  Once we have
 * retrieved the data we will show documented examples on how to manipulate
 * the data.
 *
 *
 * PURPOSE:
 *
 * 1. Learn how to request data from the connected input and output some basic
 *    information about the requested area of interest
 * 2. Learn how to use ossimImageData object to manipulate pixel data.
 * 3. Learn how to query subregions and copy subregions.
 * 4. Understand what is a NULL, EMPTY, FULL, and PARTIAL data object.
 *    this is very import for mosaicking and other pixel
 *    manipulation filters.
 *
 */

// this is the most important class and is called as the first line of all applications.
// without this all the important factories are not created.
//
#include "init/ossimInit.h"

#include "base/data_types/ossimFilename.h"
#include "base/data_types/ossimString.h"

#include "imaging/ossimImageData.h"

#include "imaging/formats/ossimImageHandler.h"
#include "imaging/factory/ossimImageHandlerRegistry.h"
#include "imaging/factory/ossimImageDataFactory.h"

// Base pointer for the passed back object type
//
#include "imaging/formats/ossimImageHandler.h"

// used to get the string name of the scalar type for the handler.  The scalar
// type specifies if its unsigned char, float, double, ...etc
//
#include "base/misc/lookup_tables/ossimScalarTypeLut.h"

#include <iterator>
#include <iostream>

void usage();
void printDataStatus(ossimDataObjectStatus status);
void demo1(const ossimFilename& filename);
void demo2(const ossimFilename& filename);
void demo3(const ossimFilename& filename);

int main(int argc, char* argv[])
{
    ossimInit::instance()->initialize(argc, argv);

    if(argc != 2)
    {
       usage();
    }
    else
    {
       // demo 1 we will access some data from the input and print
       // some basic information about the data dn then access the data
       // buffers for each band.
       //
       demo1(ossimFilename(argv[1]));


       // Demo 2 just shows ways to allocate the ossimImageData
       //
       demo2(ossimFilename(argv[1]));

       // Demo 3 just show how to load region of a tile
       //
       demo3(ossimFilename(argv[1]));

    }
    
    ossimInit::instance()->finalize();
}

void usage()
{
   cout <<"image_data <image file>"<< endl;
}

//
// The data object status is very useful and is used to determine
// if the data is NULL( not initialized), empty (initialized but blank or empty,
// partial(contains some null/invalid values), or full (all valid data).
//
// partial data means you have part of the information with valid data and the
// rest of it is set to the null or invalid pixel value for that band.
//
// full data means that every pixel for each band has valid information.
//
// empty means that no data is present
//
// null means no data and the buffer is null.
//
// For this status to be set on the ossimImageData there is a method
// validate that is called. Typically this is called when you implement
// a filter that changes the stored data.
//
void printDataStatus(ossimDataObjectStatus status)
{
   // now lets output some of the information about the tile.  The tile
   //
   switch(status)
   {
   case OSSIM_NULL:
   {
      cout << "data status = " << "null" << endl;
      break;
   }
   case OSSIM_EMPTY:
   {
      cout << "data status = " << "empty" << endl;
      break;
   }
   case OSSIM_PARTIAL:
   {
      cout << "data status = " << "partial" << endl;
      break;
   }
   case OSSIM_FULL:
   {
      cout << "data status = " << "full" << endl;
      break;
   }
   default:
   {
      cout << "data status = " << "unknown" << endl;
   }
   }
}

// Demo 1 will retrieve a region of interest using the 
// ossimIrect which is an integer rectangle.
//
void demo1(const ossimFilename& filename)
{
   cout << "___________________________________DEMO 1_____________________________________\n";
   ossimImageHandler* handler = ossimImageHandlerRegistry::instance()->open(filename);

   if(handler)
   {
   }
   else
   {
      cerr <<"Unable to open image " << filename << endl;
      return;
   }

   // we will query the first 100 by 100 pixel of data from the input
   // and compute the average value of each band.  We will only allow
   // this to happen with unsigned char data.
   //
   // Rectangles in ossim take absolute coordinates for upper
   // left and lower right and does not take an upper left
   // point and then a width height.  So if I want the data
   // starting at point location 0,0 and ending at 100 pixels along
   // the x and y direction then we have 0 to 99 along x = 100 and
   // 0 to 99 along y = 100.
   //
   // the second argument to getTile is optional and corresponds
   // to the resolution level or overview.
   //
   ossimIrect regionsOfInterest(0,0, 99, 99);

   // the data returned from the call getTile is not owned by you and
   // you should not delete the pointer.
   //
   ossimRefPtr<ossimImageData> data = handler->getTile(regionsOfInterest);

   if(data.valid())
   {
      // output the rectangle of the tile.
      cout << "tile rect = " << data->getImageRectangle() << endl;
      
      // output the status of the tile.  See printDataStatus above for
      // documentation of status.
      printDataStatus(data->getDataObjectStatus());

      // how many bands are there.
      //
      cout << "Number of bands = " << data->getNumberOfBands() << endl;

      // as in image_open lets use te scalr lut to pint the scalar type as a
      // string.
      cout << "Pixel scalar type = "
           << ossimScalarTypeLut::instance()->getEntryString(data->getScalarType())
           << endl;
      
      // for each band let's print the min, max and null pixel values
      // note:  the min max values are not for the tile but for the entire
      //        input.  Typically these are used for tile normalization
      //        and clamping to data bounds.
      //
      cout << "min pix: ";
      std::copy(data->getMinPix(),
                data->getMinPix()+data->getNumberOfBands(),
                ostream_iterator<double>(cout, ", "));
      cout << "\nmax pix: ";
      std::copy(data->getMaxPix(),
                data->getMaxPix()+data->getNumberOfBands(),
                ostream_iterator<double>(cout, ", "));
      cout << "\nnull pix: ";
      std::copy(data->getNullPix(),
                data->getNullPix()+data->getNumberOfBands(),
                ostream_iterator<double>(cout, " "));
      cout << endl;
      
      if(data->getScalarType()!=OSSIM_UCHAR)
      {
         delete handler;
         cerr << "Demo 1 only works for uchar data images" << endl;
         return;
      }
      // Now lets compute the average pixel for each band.  The data buffer is
      // internally stored in osismImageData object as a void* buffer.  We must cast
      // to the scalar type or work in normalized space.  For this example we will
      // work the tile in its native type and will not normalize and we will also
      // only work with unsigned char  or uchar data.
      //
      // Note:  ossimImageData already has a compute mean and sigma.  We will re-implement
      //        some code here
      //
      // I will implement a more efficient algorithm by only checking for invalid data
      // if the status is not full.  If its full we don't have to check for
      // null value and all we need to do is compute the sum.  Although we can just check for null
      // all the time and not worry about 2 different loops.
      //
      int upperBound             = data->getWidth()*data->getHeight();
      if(data->getDataObjectStatus() == OSSIM_FULL)
      {
         // since te data is full all pixls are used in the avverage
         double totalNumberOfPixels = upperBound;
         for(ossim_uint32 bandIndex = 0; bandIndex < data->getNumberOfBands(); ++bandIndex)
         {
            double sumOfThePixels      = 0;
            // get access to the raw band data.
            ossim_uint8* buf = (ossim_uint8*)data->getBuf(bandIndex);
            for(int offset = 0; offset < upperBound; ++offset)
            {
               sumOfThePixels += *buf;
               ++buf;
            }
            if(totalNumberOfPixels > 0)
            {
               cout << "band " << bandIndex << " average = " << sumOfThePixels/totalNumberOfPixels << endl;
            }
            else
            {
               cout << "band " << bandIndex << " average = " << 0.0 << endl;
            }
         }
      }
      else if(data->getDataObjectStatus() == OSSIM_PARTIAL)
      {
         for(ossim_uint32 bandIndex = 0; bandIndex < data->getNumberOfBands(); ++bandIndex)
         {
            double totalNumberOfPixels = 0;
            double sumOfThePixels      = 0;
            ossim_uint8* buf = (ossim_uint8*)data->getBuf(bandIndex);
            ossim_uint8 np   = (ossim_uint8)data->getNullPix(bandIndex);
            for(int offset = 0; offset < upperBound; ++offset)
            {
               if(np != *buf)
               {
                  sumOfThePixels += *buf;
                  ++totalNumberOfPixels;
               }
               ++buf;
            }
            if(totalNumberOfPixels > 0)
            {
               cout << "band " << bandIndex << " average = " << sumOfThePixels/totalNumberOfPixels << endl;
            }
            else
            {
               cout << "band " << bandIndex << " average = " << 0.0 << endl;
            }
            
         }
      }
   }
   
   cout << "___________________________________END DEMO 1_____________________________________\n";
   delete handler;
}

// Demo 2 will show how to create your own data objects
// there are several ways to do this.
void demo2(const ossimFilename& filename)
{
   cout << "___________________________________DEMO 2_____________________________________\n";
   ossimImageHandler* handler = ossimImageHandlerRegistry::instance()->open(filename);

   if(!handler)
     {
       cerr << "Unable to open file " << filename << " for demo 2\n";
       return;
     }
   // Create image data using the factory technique.
   // This might be the best way to do it.  We might have optimized
   // implementations for certain input types.  For instance you might
   // want to create an ossimImageData that is optimized for 3 band
   // ossim_uint8 data.
   //
   // there other create methods but this takes as its 
   // first argument a source that owns this data object
   // and the second argument is the input source to use
   // to help instantiate the object.  Please refer to 
   // ossim_core/imaging/factory/ossimImageDataFactory
   // for further implementation
   //
   ossimRefPtr<ossimImageData> data = ossimImageDataFactory::instance()->create(NULL,
                                                                                handler);
   
   // note: the data is not initialized/allocated.  We have 
   // a concept of a NULL or un initialized tile so all the
   // meta data is carried with it but just don't take up space
   // by allocating the buffer.  The number of bands, the rectangle
   // of interest, min, max, null are all set.
   //
   // We will now initialize the data.  The initialize will
   // allocate the buffer based on the number of bands, width,
   // height and scalar type and then will set the buffer to 
   // the null pixel value and set the status OSSIM_EMPTY.
   //
   data->initialize();
   
   
   
   // if you ever want to see if the initialization has been
   // done then you must call isInitialize.
   //
   cout << "data initialized: " << data->isInitialize() << endl;
   
   
   // To make a duplicate copy of the ossimImageData the easiest
   // way to do this is to call the dup method.
   //
   ossimRefPtr<ossimImageData> dupData = (ossimImageData*)data->dup();
   
   ossimIrect newRect(10,20,200,200);
   
   // this particualr method will reallocate the tile
   // if already initialized and set it to the new
   // rectangle of interest.
   //
   dupData->setImageRectangle(newRect);


   // allocating without the factory
   //
   ossimRefPtr<ossimImageData> data2 = new ossimImageData(NULL, // owner of the data object
                                                          OSSIM_UCHAR, // what scalar type
                                                          3, // number of bands
                                                          128, // width
                                                          128); // height
   
   data2->initialize();

   // delete the allocated data objects.
   cout << "___________________________________END DEMO 2_____________________________________\n";
}

// Demo 3 will show how to create your own data objects
// there are several ways to do this.
void demo3(const ossimFilename& filename)
{
   cout << "___________________________________DEMO 3_____________________________________\n";
   ossimImageHandler* handler = ossimImageHandlerRegistry::instance()->open(filename);

   if(!handler)
     {
       cerr << "Unable to open file " << filename << " for demo 2\n";
       return;
     }

   // lets just get some data. we will get a 128 by 
   // 128 tile that has upper left origin at 10, 10.
   //
   ossimRefPtr<ossimImageData> data = handler->getTile(ossimIrect(10,
                                                                  10,
                                                                  10 + 127,
                                                                  10 + 127));
   
   if(data.valid())
   {
      cout << "Data rectangle = " << data->getImageRectangle() << endl;
      
      ossimRefPtr<ossimImageData> data2 = ossimImageDataFactory::instance()->create(0,
                                                                                    handler);
      
      data2->setImageRectangle(ossimIrect(0,0,127,127));
      data2->initialize();
      cout << "Data 2 rectangle = " << data2->getImageRectangle() << endl;
      
      cout << "loading data2 rectangle\n";
      // now load the data 2 region with data.  Notice the
      // tile overlaps the data tile and only the 
      // overlapping region is copied.
      //
      data2->loadTile(data.get());
      
      // now we can use the raw load tile where you supply the
      // rectangle.  Now if you do this your buffer is assumed
      // to have the same number of bands.
      //
      // Now you must pass in the interleave type of your buffer
      // The ossimImageData has interleave band sequential, OSSIM_BSQ
      // which means the bands are sequential in memory where
      // all of band 1 data followed by all of band 2 ... etc.  
      // The other interleave type are OSSIM_BIP or band
      // interleaved by pixel.  For instance if you had an RGB
      // data object then it would be RGB, RGB, RGB ... etc.
      // The final interleave type is by line OSSIM_BIL.  This
      // just says band 1 line1 followed by band2 line2 ... etc until all
      // lines are stored.
      //
      data2->loadTile(data->getBuf(),
		       data->getImageRectangle(),
                      OSSIM_BSQ);
      
      // I manipulated the buffer so lets validate it for future 
      // use.  I would only validate after you get done doing your
      // data manipulation.
      //
      data2->validate();
      
      cout << "status after load is should be partial \n";
      printDataStatus(data2->getDataObjectStatus());
      
      // loadBand has simalar arguments but allows
      // you to do band loads.
      //
      for(ossim_uint32 band = 0; band < data->getNumberOfBands(); ++band)
      {
         data2->loadBand(data->getBuf(band),
                         data->getImageRectangle(),
                         band);
      }
      data2->validate();
      
      
      // please refer to ossim_core/imaging/ossimImageData.h for 
      // other load methods
      //
   }
   cout << "___________________________________END DEMO 3_____________________________________\n";
}

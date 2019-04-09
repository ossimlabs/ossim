//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Description: Utility to generate custom synthesized image for testing code..
//
// $Id: ossim-image-synth.cpp 23163 2015-02-23 16:04:05Z okramer $
//----------------------------------------------------------------------------

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/imaging/ossimMemoryImageSource.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimMapProjection.h>
#include <cmath>
#include <sstream>
#include <iostream>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>

using namespace std;

#define USE_UINT8 true

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   // Establish the image geometry's map projection:
   ossimIpt image_size (416, 416);
   ossimGpt observerGpt (0, 0, 0);
   ossimDpt gsd (1.0, 1.0); // must be same value in both directions
   ossimRefPtr<ossimEquDistCylProjection> mapProj = new ossimEquDistCylProjection();
   mapProj->setOrigin(observerGpt);
   mapProj->setMetersPerPixel(gsd);
   ossimDpt degPerPixel (mapProj->getDecimalDegreesPerPixel());
   mapProj->setElevationLookupFlag(false);
   ossimGpt ulTiePt (observerGpt);
   ulTiePt.lat += degPerPixel.lat * (image_size.y-1)/2.0;
   ulTiePt.lon -= degPerPixel.lon * (image_size.x-1)/2.0;
   mapProj->setUlTiePoints(ulTiePt);
   ossimRefPtr<ossimImageGeometry> geometry = new ossimImageGeometry(0, mapProj.get());
   geometry->setImageSize(image_size);

   // Set the destination image size:
   ossimRefPtr<ossimImageData> outImage =
         ossimImageDataFactory::instance()->create(0, OSSIM_UINT16, 3, image_size.x, image_size.y);
   typedef ossim_uint16 PIXEL_TYPE;
   ossim_uint16 min = 0.0;
   ossim_uint16 max = OSSIM_DEFAULT_MAX_PIX_UINT16;

   if(outImage.valid())
      outImage->initialize();
   else
      return -1;

   outImage->fill(min);

   PIXEL_TYPE value = 0;
   PIXEL_TYPE* bufR = ( PIXEL_TYPE*) outImage->getBuf(0);
   PIXEL_TYPE* bufG = ( PIXEL_TYPE*) outImage->getBuf(1);
   PIXEL_TYPE* bufB = ( PIXEL_TYPE*) outImage->getBuf(2);

   ossim_uint32 i = 0;
   for (int y=0; y<image_size.y; y++)
   {
      for (int x=0; x<image_size.x; x++)
      {
         bufR[i] = value;
         bufG[i] = value + 0x1000;
         bufB[i++] = value + 0x2000;
         value++;
      }
   }

   ossimFilename filename("testpattern.png");

   // Create output image chain:
   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(outImage);
   memSource->setImageGeometry(geometry.get());

   ossimRefPtr<ossimImageFileWriter> writer =
      ossimImageWriterFactoryRegistry::instance()->createWriterFromExtension(filename.ext());
   if (!writer)
      throw runtime_error( "Unable to create writer given filename extension." );
   writer->connectMyInputTo(0, memSource.get());
   writer->setFilename(filename);
   bool success = writer->execute();
   //writer->writeExternalGeometryFile();

   if (success)
      cout<<"Wrote "<<filename<<"\n"<<endl;
   else
      cout<<"Error encountered writing "<<filename<<"\n"<<endl;

   return (int)success;
}

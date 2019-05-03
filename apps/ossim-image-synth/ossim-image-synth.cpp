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

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);
   
   if ( (ap.argc() < 2) || ap.read("-h") || ap.read("--help") )
   {
      cout << "\nUsage: "<<ap[0]<<" <filename>\n"<<endl;
      return 0;
   }

   ossimFilename filename = ap[1];

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
   typedef uint16_t PIXEL_TYPE;
   double min = 0.0;
   double max = 255;

   if(outImage.valid())
      outImage->initialize();
   else
      return -1;

   outImage->fill(min);

   PIXEL_TYPE step = 8;
   PIXEL_TYPE value = 0;
   PIXEL_TYPE* bufferR = ( PIXEL_TYPE*) outImage->getBuf(0);
   PIXEL_TYPE* bufferG = ( PIXEL_TYPE*) outImage->getBuf(1);
   PIXEL_TYPE* bufferB = ( PIXEL_TYPE*) outImage->getBuf(2);

   ossim_uint32 i = 0;
   for (uint16_t y=1; y<=image_size.y; y++)
   {
      for (uint16_t x=0; x<image_size.x; x++)
      {
         bufferR[i] = y;
         bufferG[i] = y;
         bufferB[i++] = y;
      }
   }

   // Create output image chain:
   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(outImage);
   memSource->setImageGeometry(geometry.get());

   // Create TIFF writer:
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

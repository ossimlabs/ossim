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

using namespace std;

#define USE_UINT8 false

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);
   
   if ( (ap.argc() < 4) || ap.read("-h") || ap.read("--help") )
   {
      cout << "\nUsage: "<<ap[0]<<" <dx> <dy> <filename>\n"<<endl;
      return 0;
   }

   int dx = atoi(ap[1]);
   int dy = atoi(ap[2]);
   ossimFilename filename = ap[3];
   filename.setExtension(".tif");

   // Establish the image geometry's map projection:
   ossimIpt image_size (256, 256);
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
#if USE_UINT8
   ossimRefPtr<ossimImageData> outImage =
         ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, image_size.x, image_size.y);
   typedef unsigned char PIXEL_TYPE;
   double min = 0.0;
   double max = 255;
#else
   ossimRefPtr<ossimImageData> outImage =
         ossimImageDataFactory::instance()->create(0, OSSIM_NORMALIZED_DOUBLE, 1, image_size.x, image_size.y);
   typedef double PIXEL_TYPE;
   double min = 0.0;
   double max = 1.0;
#endif

   if(outImage.valid())
      outImage->initialize();
   else
      return -1;
   double A = (max - min)/2.0;
   outImage->fill(min);
   PIXEL_TYPE* buffer = ( PIXEL_TYPE*) outImage->getBuf(0);

   if ((dx != 0) || (dy != 0))
   {
      // Allocate image buffer:
      ossim_uint32 i = 0;
      double phase = 0;

      double dpx = 0;
      if (dx != 0)
         dpx = 2*M_PI/dx; // phase rate (radians per pixel)
      double dpy = 0;
      if (dy != 0)
         dpy = 2*M_PI/dy; // phase rate (radians per pixel)

      for (int y=0; y<image_size.y; y++)
      {
         phase = y*dpy;
         for (int x=0; x<image_size.x; x++)
         {
            // Loops to fill one n x n chip with a single freq (1/lambda) component:
            buffer[i++] = (PIXEL_TYPE) (A * (cos(phase) + 1.0));
            phase += dpx;
         }
      }
   }

   // Create output image chain:
   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(outImage);
   memSource->setImageGeometry(geometry.get());

   ossimRefPtr<ossimTiffWriter> writer = new ossimTiffWriter();
   writer->connectMyInputTo(0, memSource.get());
   writer->setFilename(filename);
   writer->setGeotiffFlag(true);
   bool success = writer->execute();
   //writer->writeExternalGeometryFile();

   return 0;
}

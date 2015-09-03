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
   filename.setExtension(".tif");

   // Set the destination image size:
   ossimIpt image_size (256 , 256);
   ossimRefPtr<ossimImageData> outImage =
         ossimImageDataFactory::instance()->create(0, OSSIM_FLOAT32, 1, image_size.x, image_size.y);
   if(outImage.valid())
      outImage->initialize();
   else
      return -1;
   
   // Fill the buffer with test image pattern. Start with fill:
   outImage->fill(1);

   // Establish the image geometry's map projection:
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

   // Define different tile textures: noisy, inclined, flat, combo.
   ossim_uint32 tile_size = 32;

   // Start with noisy tiles below and above amplitude threshold:
   ossim_uint32 buf_size = tile_size*tile_size;
   ossimRefPtr<ossimImageData> noisy_tile1 =
         ossimImageDataFactory::instance()->create(0, OSSIM_FLOAT32, 1, tile_size, tile_size);
   noisy_tile1->initialize();
   ossim_float32* noisy_buf1 = noisy_tile1->getFloatBuf();

   ossimRefPtr<ossimImageData> noisy_tile2 =
         ossimImageDataFactory::instance()->create(0, OSSIM_FLOAT32, 1, tile_size, tile_size);
   noisy_tile2->initialize();
   ossim_float32* noisy_buf2 = noisy_tile2->getFloatBuf();

   float noiseAmplitude1 = 0.5; // meters
   float noiseAmplitude2 = 5.0; // meters
   srand (time(NULL));
   for (ossim_uint32 i=0; i<buf_size; ++i)
   {
      noisy_buf1[i] = 1.0 + noiseAmplitude1 * ((float)rand()/(float)RAND_MAX - 0.5);
      noisy_buf2[i] = 1.0 + noiseAmplitude2 * ((float)rand()/(float)RAND_MAX - 0.5);
   }

   // Inclined planes below and above inclination threshold.
   // Eq. of plane in terms of azimuth and inclination is: z = -tan(in) [ x sin(az) + y cos(az) ]
   ossimRefPtr<ossimImageData> incline_tile1 =
         ossimImageDataFactory::instance()->create(0, OSSIM_FLOAT32, 1, tile_size, tile_size);
   incline_tile1->initialize();
   ossim_float32* incline_buf1 = incline_tile1->getFloatBuf();

   ossimRefPtr<ossimImageData> incline_tile2 =
         ossimImageDataFactory::instance()->create(0, OSSIM_FLOAT32, 1, tile_size, tile_size);
   incline_tile2->initialize();
   ossim_float32* incline_buf2 = incline_tile2->getFloatBuf();

   double azimuth = 45.0; // degrees azimuth, horizontal component of surface normal
   double inclination1 = 2.0; // degrees inclination of surface normal from local vertical
   double inclination2 = 8.0; // (above threshold)
   double sinAz = ossim::sind(azimuth);
   double cosAz = ossim::cosd(azimuth);
   double x0 = tile_size/2.0;
   double y0 = tile_size/2.0;
   double gain1 = -gsd.x * ossim::tand(inclination1);
   double gain2 = -gsd.x * ossim::tand(inclination2);
   for (ossim_uint32 y=0; y<tile_size; ++y)
   {
      for (ossim_uint32 x=0; x<tile_size; ++x)
      {
         incline_tile1->setValue(x, y, 1.0 + gain1*((x-x0)*sinAz + (y-y0)*cosAz));
         incline_tile2->setValue(x, y, 1.0 + gain2*((x-x0)*sinAz + (y-y0)*cosAz));
      }
   }

   // Create hybrid incline and noisy, below and above threshold:
   ossimRefPtr<ossimImageData> hybrid_tile1 =
         ossimImageDataFactory::instance()->create(0, OSSIM_FLOAT32, 1, tile_size, tile_size);
   hybrid_tile1->initialize();
   ossim_float32* hybrid_buf1 = hybrid_tile1->getFloatBuf();

   ossimRefPtr<ossimImageData> hybrid_tile2 =
         ossimImageDataFactory::instance()->create(0, OSSIM_FLOAT32, 1, tile_size, tile_size);
   hybrid_tile2->initialize();
   ossim_float32* hybrid_buf2 = hybrid_tile2->getFloatBuf();

   for (ossim_uint32 i=0; i<buf_size; ++i)
   {
      hybrid_buf1[i] = noisy_buf1[i] + incline_buf1[i] - 1.0;
      hybrid_buf2[i] = noisy_buf2[i] + incline_buf2[i] - 1.0;
   }

   // Create plateau:
   ossimRefPtr<ossimImageData> plateau_tile =
         ossimImageDataFactory::instance()->create(0, OSSIM_FLOAT32, 1, tile_size, tile_size);
   plateau_tile->initialize();
   plateau_tile->fill(5.0);

   // Now populate the output buffer with tiles: XXXOXOXXOXXOXOO
   std::vector<ossimImageData*> tiles;
   tiles.push_back(noisy_tile1.get());
   tiles.push_back(noisy_tile2.get());
   tiles.push_back(incline_tile1.get());
   tiles.push_back(incline_tile2.get());
   tiles.push_back(hybrid_tile1.get());
   tiles.push_back(hybrid_tile2.get());
   tiles.push_back(plateau_tile.get());
   ossimImageData* tile;
   ossim_uint32 tiles_idx = 0;
   ossimIpt tile_origin(0,0);
   for (tile_origin.y=0; tile_origin.y<image_size.y; tile_origin.y+=tile_size)
   {
      for (tile_origin.x=0; tile_origin.x<image_size.x; tile_origin.x+=tile_size)
      {
         tile = tiles[tiles_idx++];
         if (tiles_idx == tiles.size())
            tiles_idx = 0;
         tile->setOrigin(tile_origin);
         outImage->loadTile(tile);
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

   return 0;
}

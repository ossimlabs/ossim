#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/imaging/ossimMemoryImageSource.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimKMeansFilter.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
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
   
   if ( ap.read("-h") || ap.read("--help") )
   {
      cout << "\nUsage: "<<ap[0]<<" <filename>\n"<<endl;
      return 0;
   }

   ossimFilename filename = "kmeans.tif";
   if (argc > 1)
   {
      filename = ap[1];
      filename.setExtension(".tif");
   }

   // Set the destination image size:
   ossimIpt image_size (512 , 512);
   ossimRefPtr<ossimImageData> outImage =
         ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, image_size.x, image_size.y);
   if(outImage.valid())
      outImage->initialize();
   else
      return -1;
   
   // Fill the buffer with test image pattern. Start with fill:
   outImage->fill(0);

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

   ossim_uint32 tile_size = 256;
   ossim_uint8 mean1 = 20;
   ossim_uint8 mean2 = 60;
   ossim_uint8 mean3 = 100;

   ossimRefPtr<ossimImageData> outTile =
         ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, tile_size, tile_size);
   outTile->initialize();
   ossim_uint8* buf = outTile->getUcharBuf();
   srand (time(NULL));
   ossim_uint32 buf_size = tile_size*tile_size;
   ossim_uint32 buf_offset = buf_size / 4;
   double noiseAmplitude1 = 15;
   ossim_uint32 i=0;
   for (; i<buf_offset; ++i)
      buf[i] = mean1 + round(2.0* noiseAmplitude1 * ((float)rand()/(float)RAND_MAX - 0.5));
   buf_offset += buf_size / 4;
   for (;i<buf_offset; ++i)
      buf[i] = mean2 + round(2.0* noiseAmplitude1 * ((float)rand()/(float)RAND_MAX - 0.5));
   buf_offset += buf_size / 4;
   for (;i<buf_offset; ++i)
      buf[i] = mean3 + round(2.0* noiseAmplitude1 * ((float)rand()/(float)RAND_MAX - 0.5));

   ossimIpt tile_origin(0,0);
   for (tile_origin.y=0; tile_origin.y<image_size.y; tile_origin.y+=tile_size)
   {
      for (tile_origin.x=0; tile_origin.x<image_size.x; tile_origin.x+=tile_size)
      {
         outTile->setOrigin(tile_origin);
         outImage->loadTile(outTile.get());
      }
   }

   // Create output image chain:
   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(outImage);
   memSource->setImageGeometry(geometry.get());
   //ossimRefPtr<ossimTiffWriter> writer = new ossimTiffWriter();
   //writer->connectMyInputTo(0, memSource.get());
   //ossimFilename test_raster ("gen-input.tif");
   //writer->setFilename(test_raster);
   //writer->setGeotiffFlag(true);
   //bool success = writer->execute();

   ossimRefPtr<ossimKMeansFilter> kmeans = new ossimKMeansFilter;
   kmeans->connectMyInputTo(memSource.get());
   ossim_uint32 lut [3] = { 1, 128, 255 };
   kmeans->setGroupPixelValues(lut, 3);
   kmeans->setVerbose();
//   ossimRefPtr<ossimTiffWriter> writer = new ossimTiffWriter();
//   writer->connectMyInputTo(0, kmeans.get());
//   test_raster = "kmeans-out.tif";
//   writer->setFilename(test_raster);
//   writer->setGeotiffFlag(true);
//   success = writer->execute();

   ossimKeywordlist lutkwl;
   lutkwl.add("type", "ossimIndexToRgbLutFilter");
   lutkwl.add("mode", "literal");
   lutkwl.add("entry0.index", "0");
   lutkwl.add("entry0.color", "0 0 0");
   lutkwl.add("entry1.index", "1");
   lutkwl.add("entry1.color", "255 255 1");
   lutkwl.add("entry2.index", "128");
   lutkwl.add("entry2.color", "255 1 1");
   lutkwl.add("entry3.index", "255");
   lutkwl.add("entry3.color", "1 255 1");
   ossimRefPtr<ossimIndexToRgbLutFilter> lutfilter = new ossimIndexToRgbLutFilter;
   lutfilter->connectMyInputTo(kmeans.get());
   lutfilter->loadState(lutkwl);

   ossimRefPtr<ossimTiffWriter> writer = writer = new ossimTiffWriter();
   writer->connectMyInputTo(0, lutfilter.get());
   writer->setFilename(filename);
   writer->setGeotiffFlag(true);
   bool success = writer->execute();

   return 0;
}

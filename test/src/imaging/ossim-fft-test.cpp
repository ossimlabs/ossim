#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageSourceFactoryRegistry.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/imaging/ossimMemoryImageSource.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimFftFilter.h>
#include <ossim/imaging/ossimCastTileSourceFilter.h>
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimMapProjection.h>
#include <cmath>
#include <sstream>
#include <iostream>

using namespace std;

void usage(char* appName)
{
   cout << "\nUsage: "<<appName<<" [-i <input-image-name>] [-x <int> -y <int>]\n"<<endl;
}

ossimRefPtr<ossimImageSource> synthesizeInput(int dx, int dy)
{
   // Create geometry:
   ossimGpt observerGpt (0, 0, 0);
   ossimDpt gsd (1.0, 1.0); // must be same value in both directions
   ossimRefPtr<ossimEquDistCylProjection> mapProj = new ossimEquDistCylProjection();
   mapProj->setOrigin(observerGpt);
   mapProj->setMetersPerPixel(gsd);
   ossimDpt degPerPixel (mapProj->getDecimalDegreesPerPixel());
   mapProj->setElevationLookupFlag(false);
   ossimGpt ulTiePt (observerGpt);
   ossimIpt image_size (256 , 256);
   ulTiePt.lat += degPerPixel.lat * (image_size.y-1)/2.0;
   ulTiePt.lon -= degPerPixel.lon * (image_size.x-1)/2.0;
   mapProj->setUlTiePoints(ulTiePt);
   ossimRefPtr<ossimImageGeometry> geometry = new ossimImageGeometry(0, mapProj.get());
   geometry->setImageSize(image_size);

   // Allocate image buffer:
   ossimRefPtr<ossimImageData> outImage =
         ossimImageDataFactory::instance()->create(0, OSSIM_DOUBLE, 3, image_size.x, image_size.y);
   outImage->initialize();
   outImage->fill(0);
   double* buffer = (double*) outImage->getBuf(0);
   buffer[0] = OSSIM_DEFAULT_MAX_PIX_DOUBLE;
#if 0
   ossim_uint8 A = 255; // amplitude
   ossimIpt chip_origin;
   ossim_uint32 i = 0;

   // Generate data chips (n x n) at varying frequencies:

   if ((dx != 0) || (dy != 0))
   {
      for (chip_origin.y=0; chip_origin.y<image_size.y; chip_origin.y+=dy)
      {
         double phase = 0; // Chip always starts at 0 phase angle
         double dpy = 0;
         if (dy != 0)
            dpy = 2*M_PI/dy; // phase rate (radians per pixel)

         for (chip_origin.x=0; chip_origin.x<image_size.x; chip_origin.x+=dx)
         {
            double phaseX = 0; // Chip always starts at 0 phase angle
            double dpx = 0;
            if (dx != 0)
               dpx = 2*M_PI/dx; // phase rate (radians per pixel)

            // Loops to fill one n x n chip with a single freq (1/lambda) component:
            for (int y=0; y<dy; y++)
            {
               phase = y*dpy;
               phase = 0;
               i = (y+chip_origin.y)*image_size.x + chip_origin.x;
               for (int x=0; x<dx; x++)
               {
                  buffer[i++] = (ossim_uint8) A*cos(phase) + A;
                  phase += dpx;
               }

            }

            // Double the freq for the next chip until max freq reached (nyquist) then reset to start
            // at nearly DC:
//            lambda /= 2;
//            if (lambda == 0)
//               lambda = 4*n;
         }
      }
   }
#endif

   // Create output image chain:
   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(outImage);
   memSource->setImageGeometry(geometry.get());

   ossimRefPtr<ossimImageSource> pointer = memSource.get();
   return pointer;
}

ossimRefPtr<ossimImageSource> doFFT(ossimImageSource* inImage, int dx, int dy, bool forward)
{
   ossimRefPtr<ossimImageGeometry> geom = inImage->getImageGeometry();
   ossimIpt image_size = geom->getImageSize();

   // Set up FFT filter. Will first try to use the FFTW3 library if plugin was loaded, otherwise
   // will fallback to use the core newmat implementation:
   ossimImageSourceFactoryRegistry* isf = ossimImageSourceFactoryRegistry::instance();
   ossimString name ("ossimFftw3Filter");
   ossimRefPtr<ossimFftFilter> fft = (ossimFftFilter*) isf->createImageSource(name);
   if (fft.valid())
      ossimNotify(ossimNotifyLevel_INFO)<<"Using FFTW3 plugin."<<endl;
   else
   {
      ossimNotify(ossimNotifyLevel_INFO)<<"FFTW3 plugin was not available. Using core ossimFftFilter."<<endl;
      fft = new ossimFftFilter;
   }

   // Allocate output image buffer:
   ossimRefPtr<ossimImageData> outImage;
   ossimImageDataFactory* idf = ossimImageDataFactory::instance();
   if (forward)
   {
      fft->setDirectionType(ossimFftFilter::FORWARD);
      outImage = idf->create(0, inImage->getOutputScalarType(), 3, image_size.x, image_size.y);
   }
   else
   {
      fft->setDirectionType(ossimFftFilter::INVERSE);
      outImage = idf->create(0, inImage->getOutputScalarType(), 1, image_size.x, image_size.y);
   }
   outImage->initialize();
   fft->connectMyInputTo(inImage);

   // Loop over input data chips (n x n):
   if (dy == 0)
      dy = 256;
   if (dx == 0)
      dx = 256;

   for (int y=0; y<image_size.y; y+=dy)
   {
      for (int x=0; x<image_size.x; x+=dx)
      {
         ossimIrect chipRect (x, y, x+dx-1, y+dy-1);
         ossimRefPtr<ossimImageData> fft_chip = fft->getTile(chipRect);
         if (forward)
         {
            outImage->loadBand(fft_chip->getBuf(0), chipRect, 0);
            outImage->loadBand(fft_chip->getBuf(1), chipRect, 1);
         }
         else
         {
            outImage->loadTile(fft_chip.get());
         }
      }
   }

   // Create output image chain:
   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(outImage);
   memSource->setImageGeometry(geom.get());

   ossimRefPtr<ossimImageSource> pointer = memSource.get();
   return pointer;
}

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);
   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);

   if ( ap.read("-h") || ap.read("--help") )
   {
      usage(ap[0]);
      return 0;
   }

   ossimFilename inputFilename;
   if ( ap.read("-i", stringParam))
      inputFilename = tempString;

   int dx = 256;
   if ( ap.read("-x", stringParam))
      dx = tempString.toInt();

   int dy = 256;
   if ( ap.read("-y", stringParam))
      dy = tempString.toInt();

   ostringstream genfile;
   genfile<<"gen-"<<dx<<"-"<<dy<<".tif";
   ossimFilename genFilename (genfile.str());
   genFilename.setPath(inputFilename.path());

   ostringstream fftfile;
   fftfile<<"fft-"<<dx<<"-"<<dy<<".tif";
   ossimFilename fftFilename (fftfile.str());
   fftFilename.setPath(inputFilename.path());

   ostringstream invfile;
   invfile<<"inv-"<<dx<<"-"<<dy<<".tif";
   ossimFilename invFilename (invfile.str());
   invFilename.setPath(inputFilename.path());

   ap.reportRemainingOptionsAsUnrecognized();
   if ( ap.errors() )
   {
      ap.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
      return -1;
   }

   ossimRefPtr<ossimImageSource> inputImage = 0;
   if (inputFilename.empty())
   {
      inputImage = synthesizeInput(dx, dy);
      if (!genFilename.empty())
      {
         ossimRefPtr<ossimTiffWriter> writer = new ossimTiffWriter();
         writer->connectMyInputTo(0, inputImage.get());
         writer->setFilename(genFilename);
         writer->setGeotiffFlag(true);
         bool success = writer->execute();
         if (success)
            cout<<"Wrote synthesized image to <"<<genFilename<<">"<<endl;
      }
   }
   else
   {
      inputImage = ossimImageHandlerRegistry::instance()->open(inputFilename);
      if (!inputImage.valid())
      {
         cout<< "Could not load input image <"<<inputFilename<<">."<<endl;
         return -1;
      }
   }

   // Do forward FFT:
//   ossimRefPtr<ossimImageSource> fwdImage = doFFT(inputImage.get(), dx, dy, true);
//   ossimRefPtr<ossimTiffWriter> writer = new ossimTiffWriter();
//   writer->connectMyInputTo(0, fwdImage.get());
//   writer->setFilename(fftFilename);
//   writer->setGeotiffFlag(true);
//   bool success = writer->execute();
//   if (success)
//      cout<<"Wrote Forward FFT output to <"<<fftFilename<<">"<<endl;

   // Do inverse FFT:
   //ossimRefPtr<ossimImageSource> invImage = doFFT(fwdImage.get(), dx, dy, false);
   ossimRefPtr<ossimImageSource> invImage = doFFT(inputImage.get(), dx, dy, false);
   ossimRefPtr<ossimTiffWriter> writer2 = new ossimTiffWriter();
   writer2->connectMyInputTo(0, invImage.get());
   writer2->setFilename(invFilename);
   writer2->setGeotiffFlag(true);
   if (writer2->execute())
      cout<<"Wrote Inverse FFT output to <"<<invFilename<<">"<<endl;

   return 0;
}

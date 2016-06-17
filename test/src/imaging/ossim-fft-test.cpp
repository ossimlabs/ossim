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
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimMapProjection.h>
#include <cmath>
#include <sstream>
#include <iostream>

using namespace std;

void usage(char* appName)
{
   cout << "\nUsage: "<<appName<<" [-i <input-image-name>] [-n <int>]\n"<<endl;
}

ossimRefPtr<ossimImageSource> synthesizeInput(int n)
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
         ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, image_size.x, image_size.y);
   outImage->initialize();
   outImage->fill(128);
   ossim_uint8* buffer = (ossim_uint8*) outImage->getBuf();
   int lambda = n; // wavelength in pixels per cycle
   ossim_uint8 A = 127; // amplitude
   ossimIpt chip_origin;
   ossim_uint32 i = 0;

   // Generate data chips (n x n) at varying frequencies:
   for (chip_origin.y=0; chip_origin.y<image_size.y; chip_origin.y+=n)
   {
      for (chip_origin.x=0; chip_origin.x<image_size.x; chip_origin.x+=n)
      {
         double phase = 0; // Chip always starts at 0 phase angle
         double dp = 2*M_PI/lambda; // phase rate (radians per pixel)

         // Loops to fill one n x n chip with a single freq (1/lambda) component:
         for (int y=0; y<n; y++)
         {
            //phase = y*dp;
            phase = 0;
            i = (y+chip_origin.y)*image_size.x + chip_origin.x;
            for (int x=0; x<n; x++)
            {
               buffer[i++] = (ossim_uint8) A*cos(phase) + A;
               phase += dp;
            }
         }

         // Double the freq for the next chip until max freq reached (nyquist) then reset to start
         // at nearly DC:
         lambda /= 2;
         if (lambda == 0)
            lambda = 4*n;
      }
   }

   // Create output image chain:
   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(outImage);
   memSource->setImageGeometry(geometry.get());

   ossimRefPtr<ossimImageSource> pointer = memSource.get();
   return pointer;
}

ossimRefPtr<ossimImageSource> doFFT(ossimImageSource* inImage, int n, bool forward)
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
   for (int y=0; y<image_size.y; y+=n)
   {
      for (int x=0; x<image_size.x; x+=n)
      {
         ossimIrect chipRect (x, y, x+n-1, y+n-1);
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

   int n = 256;
   if ( ap.read("-n", stringParam))
      n = tempString.toInt();

   ostringstream gfile;
   gfile<<"gen-"<<n<<".tif";
   ossimFilename genFilename (gfile.str());
   genFilename.setPath(inputFilename.path());

   ostringstream fftfile;
   fftfile<<"fft-"<<n<<".tif";
   ossimFilename fftFilename (fftfile.str());
   fftFilename.setPath(inputFilename.path());

   ostringstream invfile;
   invfile<<"inv-"<<n<<".tif";
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
      inputImage = synthesizeInput(n);
      if (!genFilename.empty())
      {
         ossimRefPtr<ossimTiffWriter> writer = new ossimTiffWriter();
         writer->connectMyInputTo(0, inputImage.get());
         writer->setFilename(genFilename);
         writer->setGeotiffFlag(true);
         writer->execute();
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
   ossimRefPtr<ossimImageSource> fwdImage = doFFT(inputImage.get(), n, true);
   ossimRefPtr<ossimTiffWriter> writer = new ossimTiffWriter();
   writer->connectMyInputTo(0, fwdImage.get());
   writer->setFilename(fftFilename);
   writer->setGeotiffFlag(true);
   bool success = writer->execute();
   if (success)
      cout<<"Wrote Forward FFT output to <"<<fftFilename<<">"<<endl;

   // Do inverse FFT:
   ossimRefPtr<ossimImageSource> invImage = doFFT(fwdImage.get(), n, false);
   writer = new ossimTiffWriter();
   writer->connectMyInputTo(0, invImage.get());
   writer->setFilename(invFilename);
   writer->setGeotiffFlag(true);
   success = writer->execute();
   if (success)
      cout<<"Wrote Inverse FFT output to <"<<invFilename<<">"<<endl;

   return 0;
}

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
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimMapProjection.h>
#include <cmath>
#include <sstream>
#include <iostream>

using namespace std;

void usage(char* appName)
{
   cout << "\nUsage: "<<appName<<" -i <input-image-name> [-p <int>] [-z]"<<endl;
   cout << "       "<<appName<<" -x <int> -y <int> \n"<<endl;
   cout << "Options:\n"<<endl;
   cout << "  -i  Specifiy an input image in lieu of having one auto-generated."<<endl;
   cout << "  -p  Chip size to use (in pixels) for computing FFTs."<<endl;
   cout << "  -z  Skip forward and do only inverse FFT. Presumes input transform image is provided with -i."<<endl;
   cout << "  -x, -y  Wavelengths in pixels along x and/or y directions for generated image."<<endl;
   cout << endl;
}

bool writeFile(ossimImageData* tile, ossimFilename& fname)
{
   // Create geometry:
   ossimIpt image_size (256 , 256);
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

   // Create output image chain:
   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(tile);
   memSource->setImageGeometry(geometry.get());
   ossimImageSource* last_source = memSource.get();

//   if (memSource->getOutputScalarType() != OSSIM_UINT8)
//   {
//      ossimRefPtr<ossimScalarRemapper> remapper = new ossimScalarRemapper;
//      remapper->connectMyInputTo(memSource.get());
//      remapper->setOutputScalarType(OSSIM_UINT8);
//      last_source = remapper.get();
//   }

   ossimRefPtr<ossimTiffWriter> writer = new ossimTiffWriter();
   writer->connectMyInputTo(0, last_source);
   writer->setFilename(fname);
   writer->setGeotiffFlag(false);
   bool success = writer->execute();
   if (success)
   {
      writer->writeExternalGeometryFile();
      cout<<"Wrote <"<<fname<<">"<<endl;
   }
   else
      cout<<"Error encountered writing <"<<fname<<">"<<endl;

   return success;
}

bool synthesizeInput(int dx, int dy, ossimFilename& inputFilename)
{
#
   // Allocate image buffer:
   ossimIpt image_size (256 , 256);
   ossimRefPtr<ossimImageData> outImage =
         ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, image_size.x, image_size.y);
   outImage->initialize();

   double A = 128; // amplitude
   outImage->fill(A);

   if ((dx != 0) || (dy != 0))
   {
      ossim_uint8* buffer = (ossim_uint8*) outImage->getBuf(0);
      ossim_uint32 i = 0;
      double phase = 0; // Chip always starts at 0 phase angle
      double dpy = 0;
      if (dy != 0)
         dpy = 2*M_PI/dy; // phase rate (radians per pixel)

      double dpx = 0;
      if (dx != 0)
         dpx = 2*M_PI/dx; // phase rate (radians per pixel)

      // Loops to fill one n x n chip with a single freq (1/lambda) component:
      for (int y=0; y<image_size.y; y++)
      {
         phase = y*dpy;
         for (int x=0; x<image_size.x; x++)
         {
            buffer[i++] = (ossim_uint8) A*cos(phase) + A;
            phase += dpx;
         }
      }
   }

   return writeFile(outImage.get(), inputFilename);
}

bool doFFT(ossimFilename& inputFname, int chip_size, bool forward, ossimFilename& fname)
{
   ossimRefPtr<ossimImageSource> inputImage =
         ossimImageHandlerRegistry::instance()->open(inputFname);
   if (!inputImage.valid())
   {
      cout<< "Could not load image <"<<inputFname<<">."<<endl;
      return -1;
   }

   ossimRefPtr<ossimImageGeometry> geom = inputImage->getImageGeometry();
   ossimIpt image_size = geom->getImageSize();

   // Set up FFT filter. Will first try to use the FFTW3 library if plugin was loaded, otherwise
   // will fallback to use the core newmat implementation:
   ossimImageSourceFactoryRegistry* isf = ossimImageSourceFactoryRegistry::instance();
   ossimString name ("ossimFftw3Filter");
   ossimRefPtr<ossimFftFilter> fft = (ossimFftFilter*) isf->createImageSource(name);
   if (!fft.valid())
   {
      ossimNotify(ossimNotifyLevel_INFO)<<"FFTW3 plugin was not available. Using core ossimFftFilter."<<endl;
      fft = new ossimFftFilter;
   }
   if (forward)
      fft->setDirectionType(ossimFftFilter::FORWARD);
   else
      fft->setDirectionType(ossimFftFilter::INVERSE);
   fft->connectMyInputTo(inputImage.get());
   ossimImageSource* last_source = fft.get();

   // Allocate output image buffer:
   ossimRefPtr<ossimImageData> outImage;
   ossimImageDataFactory* idf = ossimImageDataFactory::instance();
   if (forward)
      outImage = idf->create(0, fft->getOutputScalarType(), 3, image_size.x, image_size.y);
   else
      outImage = idf->create(0, fft->getOutputScalarType(), 1, image_size.x, image_size.y);

   outImage->initialize();
   outImage->setNullPix(0.0);

   // Loop over input data chips (n x n):
   if (chip_size == 0)
      chip_size = 256;
   for (int y=0; y<image_size.y; y+=chip_size)
   {
      for (int x=0; x<image_size.x; x+=chip_size)
      {
         ossimIrect chipRect (x, y, x+chip_size-1, y+chip_size-1);
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

   return writeFile(outImage.get(), fname);
}

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);
   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);

   if ( ap.read("-h") || ap.read("--help") || (argc < 2))
   {
      usage(ap[0]);
      return 0;
   }

   bool generate_image = true;
   ossimFilename inputFilename;
   if ( ap.read("-i", stringParam))
   {
      inputFilename = tempString;
      generate_image = false;
   }

   int dx = 0;
   if ( ap.read("-x", stringParam))
      dx = tempString.toInt();

   int dy = 0;
   if ( ap.read("-y", stringParam))
      dy = tempString.toInt();

   int chip_size = 256;
   if ( ap.read("-p", stringParam))
      chip_size = tempString.toInt();

   bool inverse_only = false;
   if ( ap.read("-z"))
      inverse_only = true;

   ostringstream f0, f1, f2;
   ossimFilename fftFilename, invFilename;

   if (generate_image)
   {
      f0 << "gen-" << dx << "-" << dy << ".tif";
      inputFilename = f0.str();
   }

   f1 << inputFilename.fileNoExtension() << "-FFT.tif";
   fftFilename = f1.str();
   f2 << inputFilename.fileNoExtension() << "-INV.tif";
   invFilename = f2.str();

   ap.reportRemainingOptionsAsUnrecognized();
   if ( ap.errors() )
   {
      ap.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
      return -1;
   }

   if (generate_image && !synthesizeInput(dx, dy, inputFilename))
   {
      cout<< "Could not load input image <"<<inputFilename<<">."<<endl;
      return -1;
   }

   // Do forward FFT:
   if (!inverse_only)
   {
      if (!doFFT(inputFilename, chip_size, true, fftFilename))
         return -1;
   }
   else
      fftFilename = inputFilename;

   if (!doFFT(fftFilename, chip_size, false, invFilename))
      return -1;

   return 0;
}

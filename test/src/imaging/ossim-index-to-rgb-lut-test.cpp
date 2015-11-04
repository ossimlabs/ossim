//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// Description: Test of ossimIndexToRgbLutFilter.
//
// $Id: ossim-index-to-rgb-lut-test.cpp 23068 2015-01-07 23:08:29Z okramer $
//----------------------------------------------------------------------------

#include <iostream>
using namespace std;

#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimMemoryImageSource.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimTiffWriter.h>

static const std::string kwl_literal =
      "type: ossimIndexToRgbLutFilter \n"
      "mode: literal  \n"
      "entry0.index: 0 \n"
      "entry0.color: 1 2 3 \n"
      "entry1.index: 1.0 \n"
      "entry1.color: 255 1 1 \n"
      "entry2.index: 2.0 \n"
      "entry2.color: 1 255 1 \n";

static const std::string kwl_vertices =
      "type: ossimIndexToRgbLutFilter \n"
      "mode: vertices  \n"
      "entry0.index: 0.5 \n"
      "entry0.color: 1 2 3 \n"
      "entry1.index: 127.5 \n"
      "entry1.color: 255 1 1 \n"
      "entry2.index: 250.0 \n"
      "entry2.color: 1 255 1 \n";

static const std::string kwl_regular =
      "type: ossimIndexToRgbLutFilter \n"
      "mode: regular \n"
      "entry0: 1 1 255 \n"
      "entry1: 1 255 1 \n"
      "entry2: 255 1 1 \n"
      "max_value:  250 \n"
      "min_value:  1 \n";

static const std::string kwl_regular2 =
      "type: ossimIndexToRgbLutFilter \n"
      "mode: regular \n"
      "entry0.r: 1 \n"
      "entry0.g: 1 \n"
      "entry0.b: 255 \n"
      "entry1.r: 1 \n"
      "entry1.g: 255 \n"
      "entry1.b: 1 \n"
      "entry2.r: 255 \n"
      "entry2.g: 1 \n"
      "entry2.b: 1 \n"
      "max_value:  250 \n"
      "min_value:  1 \n";

static const std::string kwl_regnorm =
      "type: ossimIndexToRgbLutFilter \n"
      "mode: regular \n"
      "entry0: 1 1 255 \n"
      "entry1: 1 255 1 \n"
      "entry2: 255 1 1 \n"
      "max_value:  1.0 \n"
      "min_value:  0 \n";


bool runTest(const std::string& kwlString, ossimRefPtr<ossimImageSourceFilter>& lutFilter)
{
   ossimKeywordlist kwl;
   kwl.parseString(kwlString);
   if (!lutFilter->loadState(kwl))
   {
      cout << "ERROR: bad state returned from ossimIndexToRgbLutFilter::loadState()" << endl;
      return false;
   }

   ossimKeywordlist savedKwl;
   lutFilter->saveState(savedKwl);
   cout << savedKwl << endl;

   lutFilter->initialize();
   ossimIrect rect (lutFilter->getBoundingRect());
   ossimRefPtr<ossimImageData> rgbBuffer = lutFilter->getTile(rect);

   ossim_uint8* outBuf[3];
   outBuf[0] = (ossim_uint8*)(rgbBuffer->getBuf(0));
   outBuf[1] = (ossim_uint8*)(rgbBuffer->getBuf(1));
   outBuf[2] = (ossim_uint8*)(rgbBuffer->getBuf(2));
   for (int x=0; x<256; ++x)
   {
      cout << "[" <<  x  << "] : " << (int)outBuf[0][x] << "  " << (int)outBuf[1][x] << "  "
            << (int)outBuf[2][x] << endl;
   }
   cout << endl;

   return true;
}

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   // For batch testing, permit assigning output dir for test tiff:
   ossimFilename tiffOutFile ("./test-tif.tif");
   if (argc > 1)
      tiffOutFile = argv[1];

   // Initialize the input index buffer:
   ossimRefPtr<ossimImageData> indexBuffer =
         ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, 256, 1);
   if(!indexBuffer.valid())
      return -1;
   indexBuffer->initialize();
   for (int x=0; x<256; ++x)
      indexBuffer->setValue(x, 0, x);
   indexBuffer->validate();

   // Create output image chain:
   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(indexBuffer);
   memSource->setRect(0, 0, 256, 1);
   ossimRefPtr<ossimImageSourceFilter> lutFilter = new ossimIndexToRgbLutFilter();
   lutFilter->connectMyInputTo(memSource.get());
   bool success;

   // Test 1: LITERAL mode
   cout << "\nTEST 1 -- Running LITERAL mode test...\n"<<endl;
   if (!runTest(kwl_literal, lutFilter))
      return -1;

   // Test 2: VERTICES mode
   cout << "\nTEST 2 -- Running VERTICES mode test...\n"<<endl;
   if (!runTest(kwl_vertices, lutFilter))
      return -1;

   // Test 3: REGULAR mode, compact-format
   cout << "\nTEST 3 -- Running REGULAR mode, compact-format test...\n"<<endl;
   if (!runTest(kwl_regular, lutFilter))
      return -1;

   // Test 4: REGULAR mode, bloated-format
   cout << "\nTEST 4 -- Running REGULAR mode, bloated-format test...\n"<<endl;
   if (!runTest(kwl_regular2, lutFilter))
      return -1;

   // Test 5: Heat map tile output using type double input index:
   cout << "\nTEST 5 --Output of heatmap tile from floating point normalized index data...\n"<<endl;
   indexBuffer = ossimImageDataFactory::instance()->create(0, OSSIM_FLOAT64, 1, 512, 512);
   indexBuffer->initialize();
   for (int y=0; y<512; ++y)
   {
      for (int x=0; x<512; ++x)
      {
         double distance = sqrt((double) ((x-256)*(x-256) + (y-256)*(y-256)));
         double d = (256.0 - distance) / 256.0;
         if (d <= 0)
            d = 0.0;
         indexBuffer->setValue(x, y, d);
      }
   }
   indexBuffer->validate();

   ossimRefPtr<ossimMemoryImageSource> idxSource = new ossimMemoryImageSource;
   idxSource->setImage(indexBuffer);

   ossimRefPtr<ossimImageSourceFilter> lut2Filter = new ossimIndexToRgbLutFilter();
   lut2Filter->connectMyInputTo(idxSource.get());
   ossimKeywordlist kwl;
   kwl.parseString(kwl_regnorm);
   lut2Filter->loadState(kwl);
   lut2Filter->initialize();
   ossimIrect rect (lut2Filter->getBoundingRect());

   ossimRefPtr<ossimImageData> rgbBuffer = lut2Filter->getTile(rect);

   ossimRefPtr<ossimMemoryImageSource> rgbSource = new ossimMemoryImageSource;
   rgbSource->setImage(rgbBuffer);
   ossimRefPtr<ossimTiffWriter> writer = new ossimTiffWriter;
   writer->setFilename(tiffOutFile);
   writer->connectMyInputTo(0, rgbSource.get());
   writer->setAreaOfInterest(rect);
   writer->setGeotiffFlag(false);
   success = writer->execute();
   if (success)
      cout << "Heat map image successfully written."<<endl;
   else
      return -1;

   return 0;
}

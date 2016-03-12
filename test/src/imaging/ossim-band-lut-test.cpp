//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// Description: Test of ossimBandLutFilter.
//
// $Id$
//----------------------------------------------------------------------------

#include <iostream>
using namespace std;

#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/imaging/ossimBandLutFilter.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimMemoryImageSource.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimTiffWriter.h>

void initKwl(ossimKeywordlist& kwl)
{
   kwl.add("type", "ossimBandLutFilter");
   kwl.add("scalar_type", "F32");

   // Entry 0 (1,1,1) --> (1,1,0.1) yellow
   kwl.add("band0.entry0.in", "1");
   kwl.add("band1.entry0.in", "1");
   kwl.add("band2.entry0.in", "1");
   kwl.add("band0.entry0.out", "1.0");
   kwl.add("band1.entry0.out", "1.0");
   kwl.add("band2.entry0.out", "0.1");

   // Entry 1 (128,128,128) --> (1,0.5,0.5) red
   kwl.add("band0.entry1.in", "128");
   kwl.add("band1.entry1.in", "128");
   kwl.add("band2.entry1.in", "128");
   kwl.add("band0.entry1.out", "0.1");
   kwl.add("band1.entry1.out", "0.1");
   kwl.add("band2.entry1.out", "1.0");

   // Entry 2 (255,255,255) --> (1,0.1,1.0) violet
   kwl.add("band0.entry2.in", "255");
   kwl.add("band1.entry2.in", "255");
   kwl.add("band2.entry2.in", "255");
   kwl.add("band0.entry2.out", "1.0");
   kwl.add("band1.entry2.out", "0.1");
   kwl.add("band2.entry2.out", "1.0");
}

bool writeTile(ossimFilename& fname, ossimImageSource* source)
{
   ossimRefPtr<ossimTiffWriter> writer = new ossimTiffWriter;
   writer->setFilename(fname);
   writer->connectMyInputTo(0, source);
   writer->setGeotiffFlag(false);
   if ( !writer->execute() )
      return false;

   cout << "Wrote result tile to <"<<fname<<">."<<endl;
   return true;
}

bool runTest(const ossimKeywordlist& kwl, ossimRefPtr<ossimImageSourceFilter>& lutFilter)
{
   lutFilter->loadState(kwl);
   lutFilter->initialize();

   ossimFilename fname = kwl.find("output_dir");
   if (!fname.empty())
      fname += "/";
   fname += "bandLutTest-";
   fname += kwl.find("mode");
   fname.setExtension("tif");

   return writeTile(fname, lutFilter.get());
}


int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   ossimKeywordlist kwl;
   initKwl(kwl);

   // Accept test directory on command line:
   if (argc > 1)
      kwl.add("output_dir", argv[1]);

   // Initialize the input index buffer:
   ossimRefPtr<ossimImageData> indexTile =
         ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 3, 256, 256);
   if(!indexTile.valid())
      return -1;
   indexTile->initialize();
   for (int band=0; band<3; ++band)
   {
      ossim_uint8 p = 0;
      ossim_uint8* bandBuf = indexTile->getUcharBuf(band);
      ossim_uint32 pixel = 0;
      for (int y=0; y<256; ++y)
      {
         for (int x=0; x<256; ++x)
            bandBuf[pixel++] = p;
         p += 1;
      }
   }
   indexTile->validate();

   // Create output image chain:
   ossimRefPtr<ossimMemoryImageSource> memSource = new ossimMemoryImageSource;
   memSource->setImage(indexTile);
   memSource->setRect(0, 0, 256, 256);

   ossimRefPtr<ossimImageSourceFilter> lutFilter = new ossimBandLutFilter();
   lutFilter->connectMyInputTo(memSource.get());

   // Test 1: LITERAL mode
   cout << "\nTEST 1 -- Running LITERAL mode test...\n"<<endl;
   kwl.add("mode", "literal");
   if (!runTest(kwl, lutFilter))
      return -1;

   // Test 2: VERTICES mode
   cout << "\nTEST 2 -- Running INTERPOLATED mode test...\n"<<endl;
   kwl.add("mode", "interpolated");
   if (!runTest(kwl, lutFilter))
      return -1;

   return 0;
}

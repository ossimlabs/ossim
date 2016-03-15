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
#include <ossim/base/ossimMultiResLevelHistogram.h>
#include <ossim/imaging/ossimHistogramThreshholdFilter.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimTiffWriter.h>

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

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   ossimRefPtr<ossimImageHandler> handler =
         ossimImageHandlerRegistry::instance()->open("float_tile.tif",1,0);
   if(!handler.valid())
      return -1;

   ossimRefPtr<ossimMultiResLevelHistogram> histo = new ossimMultiResLevelHistogram;
   histo->importHistogram("float_tile.his");
   ossimRefPtr<ossimHistogramThreshholdFilter> filter =
         new  ossimHistogramThreshholdFilter(45, 45, handler.get(), histo.get());
   filter->connectMyInputTo(handler.get());
   filter->initialize();

   ossimFilename fname = "thresholded-histo.tif";
   if (!writeTile(fname, filter.get()))
      return -1;

   return 0;
}

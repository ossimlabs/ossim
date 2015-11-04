//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// Description: Unit test application for WKT Projection Factory.
//
// $Id$
//----------------------------------------------------------------------------

#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/point_cloud/ossimPointCloudImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageSourceSequencer.h>

int main(int argc, char* argv[])
{
   cout << "ossim-point-cloud-image-handler Test:" << endl;
   ossimInit::instance()->initialize(argc, argv);

   if (argc != 2)
   {
      cout  << "Missing input point-cloud data file name.\n"
            << "Usage: " << argv[0] << " <filename.las>" << endl;
      return -1;
   }
   ossimFilename fname (argv[1]);

   cout << "  Testing open() via image handler registry... "; cout.flush();
   ossimImageHandlerRegistry* registry = ossimImageHandlerRegistry::instance();
   ossimRefPtr<ossimImageHandler> iHandler = registry->open(fname, true, true);
   ossimPointCloudImageHandler* pciHandler = dynamic_cast<ossimPointCloudImageHandler*>(iHandler.get());
   assert(pciHandler);
   cout << "  Passed.\n";

   cout << "  Testing getImageGeometry()... "; cout.flush();
   ossimRefPtr<ossimImageGeometry> igeom = iHandler->getImageGeometry();
   assert(igeom.valid() && igeom->hasProjection());
   cout << "  Passed.\n";


   ossimRefPtr<ossimImageSourceSequencer> seq = new ossimImageSourceSequencer (iHandler.get());
   seq->initialize();
   seq->setToStartOfSequence();
   int numTiles = seq->getNumberOfTiles();
   cout << "Number of Tiles: "<<numTiles<<endl;
   cout << "Number of Tiles Horizontal: "<<seq->getNumberOfTilesHorizontal()<<endl;
   cout << "Number of Tiles Vertical: "<<seq->getNumberOfTilesVertical()<<endl;

   cout << "  Testing getTile()... "; cout.flush();
   int count = 1;
   while (1) {
      ossimRefPtr<ossimImageData> tile = seq->getNextTile();
      if (!tile.valid())
         break;

      //ossim_uint32 numBandsIn  = pciHandler->getNumberOfInputBands();
      //ossim_uint32 numBandsOut = tile->getNumberOfBands();
      //const ossim_float64* minpix = tile->getMinPix();
      //const ossim_float64* maxpix = tile->getMaxPix();
      if (count == 20)
         break;
   }
   cout << "  Passed.\n";

   seq = 0;
   igeom = 0;
   iHandler = 0;

   return 0;
}



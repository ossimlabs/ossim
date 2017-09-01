//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Contains application definition "foo" app.
//
// NOTE:  This is supplied for simple quick test.  Makefile links with
//        libossim so you don't have to muck with that.
//        DO NOT checkin your test to the svn repository.  Simply
//        edit foo.cc (Makefile if needed) and run your test.
//        After completion you can do a "svn revert foo.cpp" if you want to
//        keep your working repository up to snuff.  Enjoy!
//
// $Id: ossim-pixel-flipper-test.cpp 22197 2013-03-12 02:00:55Z dburken $
//----------------------------------------------------------------------------

#include <ossim/base/ossimCommon.h>  
#include <ossim/base/ossimRefPtr.h> 
#include <ossim/base/ossimException.h> 
#include <ossim/base/ossimNotify.h> 
#include <ossim/init/ossimInit.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>


#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimPixelFlipper.h>

static int expected_0[] = {0,0,0,0,255,0,0,255,255,5,5,5,5,255,5,128,255,255};
static int expected_1[] = {11,11,11,11,255,11,11,255,255,11,11,11,11,255,11,128,255,255};
static int expected_2[] = {0,0,0,11,255,11,11,255,255,5,5,5,11,255,11,128,255,255};
static int expected_3[] = {0,0,0,11,11,11,11,11,11,5,5,5,11,11,11,128,255,255};
static int expected_4[] = {11,11,11,0,255,0,0,255,255,11,11,11,5,255,5,128,255,255};
static int expected_5[] = {8,8,8,8,200,8,8,200,200,8,8,8,8,200,8,128,200,200};
static int expected_6[] = {11,11,11,11,11,11,11,11,11,8,8,8,8,200,8,128,200,200};

//*************************************************************************************************
void showPixels(const ossimRefPtr<ossimImageData> tile, const int* expected_values)
{
   int j = 0;
   for (int i=0; i<6; i++)
   {
      int r = (int) tile->getPix(i, 0);
      int g = (int) tile->getPix(i, 1);
      int b = (int) tile->getPix(i, 2);

      // bool correct = true;
      if ((r==expected_values[j])&&(g==expected_values[j+1])&&(b==expected_values[j+2]))
      {
         cout << "  PASSED ["<<i<<"] = "<<r<<", "<<g<<", "<<b<<endl;
      }
      else
      {
         cout << "  FAILED ["<<i<<"] = "<<r<<", "<<g<<", "<<b<<" -- expected "
            <<expected_values[j]<<", "<<expected_values[j+1]<<", "<<expected_values[j+2]
         <<endl;
      }
      j += 3;
   }
}

//*************************************************************************************************
int main(int argc, char *argv[])
{
   try
   {
      ossimInit::instance()->initialize(argc, argv);
      ossimFilename fname ("PixelFlipperTestChip.tif");
      ossimRefPtr<ossimImageHandler> handler = ossimImageHandlerRegistry::instance()->open(fname);
      vector<ossimIpt> valid_vertices;
      valid_vertices.push_back(ossimIpt(0,0));
      valid_vertices.push_back(ossimIpt(127,0));
      valid_vertices.push_back(ossimIpt(127,127));
      valid_vertices.push_back(ossimIpt(0,127));
      handler->writeValidImageVertices(valid_vertices);

      // Fetch raw tile:
      ossimRefPtr<ossimPixelFlipper> flipper = new ossimPixelFlipper();  
      ossimIrect tile_rect(0,0,127,127);
      flipper->connectMyInputTo(handler.get());
      ossimRefPtr<ossimImageData> tile = handler->getTile(tile_rect);
      cout<<"\nRaw Tile:"<<endl;
      showPixels(tile, expected_0);

      // Target Replacement:
      flipper->setTargetRange(0, 10);  
      flipper->setReplacementValue(11);
      flipper->setReplacementMode(ossimPixelFlipper::REPLACE_BAND_IF_TARGET);
      tile = flipper->getTile(tile_rect);
      cout<<"\nTarget Replacement REPLACE_BAND_IF_TARGET:"<<endl;
      showPixels(tile, expected_1);

      // Target Replacement:
      flipper->setTargetRange(0, 10);  
      flipper->setReplacementMode(ossimPixelFlipper::REPLACE_BAND_IF_PARTIAL_TARGET);
      tile = flipper->getTile(tile_rect);
      cout<<"\nTarget Replacement REPLACE_BAND_IF_PARTIAL_TARGET:"<<endl;
      showPixels(tile, expected_2);

      // Target Replacement:
      flipper->setTargetRange(0, 10);  
      flipper->setReplacementMode(ossimPixelFlipper::REPLACE_ALL_BANDS_IF_PARTIAL_TARGET);
      tile = flipper->getTile(tile_rect);
      cout<<"\nTarget Replacement REPLACE_ALL_BANDS_IF_PARTIAL_TARGET:"<<endl;
      showPixels(tile, expected_3);

      // Target Replacement:
      flipper->setTargetRange(0, 10);  
      flipper->setReplacementMode(ossimPixelFlipper::REPLACE_ONLY_FULL_TARGETS);
      tile = flipper->getTile(tile_rect);
      cout<<"\nTarget Replacement REPLACE_ONLY_FULL_TARGETS:"<<endl;
      showPixels(tile, expected_4);

      // Clamping:
      flipper->setClampValues(8, 200);  
      tile = flipper->getTile(tile_rect);
      cout<<"\nClamping:"<<endl;
      showPixels(tile, expected_5);

      // Valid Vertices:
      valid_vertices.clear();
      valid_vertices.push_back(ossimIpt(3,0));
      valid_vertices.push_back(ossimIpt(127,0));
      valid_vertices.push_back(ossimIpt(127,127));
      valid_vertices.push_back(ossimIpt(3,127));
      handler->writeValidImageVertices(valid_vertices);
      flipper->initialize();
      flipper->setClipMode(ossimPixelFlipper::VALID_VERTICES);
      tile = flipper->getTile(tile_rect);
      cout<<"\nValid Vertices (with clamping):"<<endl;
      showPixels(tile, expected_6);
   }
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << e.what() << std::endl;
   }

   cout << "FINISHED" << endl;
   return 0;
}

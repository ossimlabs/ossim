//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-foo.cpp
//
// Author:  Oscar Kramer
//
// Description: Compares pixel data between two images. Returns with 0 if same or 1 of different.
//              The input formats can be different -- the pixels are compared after any 
//              unpacking and decompression. Only R0 is compared.
//
// $Id: ossim-image-compare.cpp 19753 2011-06-13 15:20:31Z dburken $
//----------------------------------------------------------------------------

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/init/ossimInit.h>
#include <iostream>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageSourceSequencer.h>

using namespace std;

template <typename T> bool tilesAreDifferent(const ossimImageData* t1, const ossimImageData* t2)
{
   ossimIpt size1 = t1->getImageRectangle().size();
   ossimIpt size2 = t2->getImageRectangle().size();
   int nbands1 = t1->getNumberOfBands();
   int nbands2 = t2->getNumberOfBands();
   if ((size1 != size2) || (nbands1 != nbands2))
      return true;

   ossim_uint32 num_pixels = (ossim_uint32) (size1.x * size1.y);
   
   for (int band=0; band<nbands1; band++)
   {
      T* buf1 = (T*) t1->getBuf(band);
      T* buf2 = (T*) t2->getBuf(band);
      
      for ( ossim_uint32 p=0; p<num_pixels; ++p)
      {
         if (buf1[p] != buf2[p])
            return true;
      }
   }
   return false;
}

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   try
   {
      if (ap.argc() != 3)
      {
         cout << "\nUsage: "<<ap.getApplicationName()<<" <image1> <image2>"<<endl;
         return 1;
      }
      
      ossimFilename f1 (argv[1]);
      ossimFilename f2 (argv[2]);
      cout << "\nComparing <"<<f1<<"> to <"<<f2<<">..."<<endl;
      // Establish input image handlers:
      ossimImageHandlerRegistry* registry = ossimImageHandlerRegistry::instance();
      ossimRefPtr<ossimImageHandler> h1 = registry->open(ossimFilename(argv[1]));
      if (!h1.valid())
      {
         cout<<"  Could not open first image at <"<<argv[1]<<">. Aborting..."<<endl;
         return 1;
      }
      ossimRefPtr<ossimImageHandler> h2 = registry->open(ossimFilename(argv[2]));
      if (!h1.valid())
      {
         cout<<"  Could not open second image at <"<<argv[2]<<">. Aborting..."<<endl;
         h1->close();
         return 1;
      }

      // Establish the tile sequencer to loop over all tiles:
      ossimRefPtr<ossimImageSourceSequencer> sequencer1 = new ossimImageSourceSequencer(h1.get());
      ossimRefPtr<ossimImageSourceSequencer> sequencer2 = new ossimImageSourceSequencer(h2.get());
      sequencer1->setToStartOfSequence();
      sequencer2->setToStartOfSequence();
      
      // Begin loop over all tiles, checking them for any non-empty status:
      int tile_count = 0;
      bool diff_found = false;

      ossimRefPtr<ossimImageData> tile1 = sequencer1->getNextTile();
      ossimRefPtr<ossimImageData> tile2 = sequencer2->getNextTile();
      ossimScalarType stype = tile1->getScalarType();
      while (tile1.valid() && tile2.valid() && !diff_found)
      {
         switch (stype)
         {
         case OSSIM_UINT8:
         case OSSIM_SINT8:
            diff_found = tilesAreDifferent<ossim_uint8>(tile1.get(), tile2.get());
            break;

         case OSSIM_UINT16:
         case OSSIM_SINT16:
         case OSSIM_USHORT11:
            diff_found = tilesAreDifferent<ossim_uint16>(tile1.get(), tile2.get());
            break;

         case OSSIM_UINT32:
         case OSSIM_SINT32:
            diff_found = tilesAreDifferent<ossim_uint32>(tile1.get(), tile2.get());
            break;

         case OSSIM_FLOAT32:
            diff_found = tilesAreDifferent<ossim_float32>(tile1.get(), tile2.get());
            break;

         case OSSIM_FLOAT64:
            diff_found = tilesAreDifferent<ossim_float64>(tile1.get(), tile2.get());
            break;

         default:
            cout<<"  This datatype is not supported. Aborting..."<<endl;
            diff_found = true;
         }

         tile1 = sequencer1->getNextTile();
         tile2 = sequencer2->getNextTile();
         ++tile_count;
      }

      h1->close();
      h2->close();
      
      if (diff_found)
      {
         cout << "  DIFFERENCE FOUND AT TILE "<<tile_count<<"."<<endl;
         return 1;
      }
      cout << "  No differences found."<<endl;
      return 0;
   }
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      return 1;
   }
   
}

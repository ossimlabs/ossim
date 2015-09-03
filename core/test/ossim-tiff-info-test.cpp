//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Test app for ossimTiffInfo class.
// 
//----------------------------------------------------------------------------
// $Id: ossim-tiff-info-test.cpp 19751 2011-06-13 15:13:07Z dburken $

#include <iostream>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/support_data/ossimTiffInfo.h>
#include <ossim/support_data/ossimGeoTiff.h>



int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   if (argc != 2)
   {
      cout << argv[0] << "<tiff_file>"
           << "\nPrint dump and geometry info for tiff_file." << endl;
      return 0;
   }

   cout << "ossimTiffInfo test:\n";
   
   ossimTiffInfo* info = new ossimTiffInfo();

   if ( info->open( ossimFilename(argv[1]) ) )
   {
      cout << "ossimTiffInfo dump info:\n";
      info->print(cout);

      ossimKeywordlist kwl;
      if ( info->getImageGeometry(kwl, 0) )
      {
         cout << "ossimTiffInfo geometry info:\n" << kwl << endl;
      }
      else
      {
         cout << "ossimTiffInfo get image geometry failed..." << endl;
      }
   }
   else
   {
      cout << "Could not open: " << argv[1] << endl;
   }

   delete info;
   info = 0;

   cout << "ossimGeoTiff test:\n";
   
   ossimGeoTiff* gtif = new ossimGeoTiff();
   if ( gtif->readTags(argv[1], 0) )
   {
      ossimKeywordlist kwl;
      if ( gtif->addImageGeometry(kwl, 0) )
      {
         cout << "ossimGeoTiff geometry info:\n" << kwl << endl;
      }
      else
      {
         cout << "ossimGeoTiff get image geometry failed..." << endl;
      }
   }

   delete gtif;
   gtif = 0;

   return 0;
}

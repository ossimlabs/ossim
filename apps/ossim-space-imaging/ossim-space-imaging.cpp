//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
// Description:  Converts SpaceImaging geometry file to
//               OSSIM Geometry file.
//
//*******************************************************************
//  $Id: space_imaging.cpp 9966 2006-11-29 02:01:07Z gpotts $

#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/support_data/ossimSpaceImagingGeom.h>
#include <iostream>
using namespace std;

void usage()
{
   cout << "space_imaging <input_geom> [out_geom]" << endl
        << endl
        << "Note: out_geom is defaulted to output.kwl" << endl;
}

int main(int argc, char* argv[])
{
   ossimFilename inputFile  = "";
   ossimFilename outputFile = "output.kwl";

   if(argc <= 1)
   {
      usage();
   }
   else if(argc == 2)
   {
      inputFile = argv[1];
   }
   else
   {
      inputFile  = argv[1];
      outputFile = argv[2];
   }

   if(inputFile != "")
   {
      ossimKeywordlist kwl;
      
      ossimSpaceImagingGeom spaceImg(inputFile);
      spaceImg.exportToOssim(kwl);

      kwl.write(ossimFilename(outputFile));
   }
}

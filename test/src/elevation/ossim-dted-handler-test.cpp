//----------------------------------------------------------------------------
// Copyright (C) 2005 David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Test application / example code of opening a dted handler
// and using it.
//
// $Id: ossim-dted-handler-test.cpp 23495 2015-08-28 15:25:19Z okramer $
//----------------------------------------------------------------------------

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/init/ossimInit.h>
#include <ossim/elevation/ossimDtedHandler.h>

using namespace std;


int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   //---
   // Note: You must tweak the next three variables...
   // This is a 3601x3601 cell.
   //---
   ossimFilename dtedCell = "/work/drb/image_formats/dted/1arc/w092/n40.dt1";
   double neLat = 41.0;
   double neLon = -92.0;
   
   ossimDtedHandler* dh = new ossimDtedHandler(dtedCell);
   double p00 = dh->getPostValue(ossimIpt(0, 3599));
   double p01 = dh->getPostValue(ossimIpt(0, 3600));
   double p10 = dh->getPostValue(ossimIpt(1, 3599));
   double p11 = dh->getPostValue(ossimIpt(1, 3600));

   double ps  = 1.0/3601.0; // post spacing
   double hps = ps/2.0;     // half post spacing
   
   ossimGpt gp00( (neLat-ps),    neLon,      0.0 );
   ossimGpt gp01(  neLat,        neLon,      0.0 );
   ossimGpt gp10( (neLat-ps),   (neLon+ps),  0.0);
   ossimGpt gp11(  neLat,       (neLon+ps),  0.0 );
   ossimGpt gpMid( (neLat-hps), (neLon+hps), 0.0 );
   
   double hp00  =  dh->getHeightAboveMSL(gp00);   
   double hp01  =  dh->getHeightAboveMSL(gp01);
   double hp10  =  dh->getHeightAboveMSL(gp10);
   double hp11  =  dh->getHeightAboveMSL(gp11);
   double hpMid =  dh->getHeightAboveMSL(gpMid);

   cout << "\ngp00:   " << gp00
        << "\np00:    " << p00
        << "\nhp00:   " << hp00
      
        << "\ngp01:   " << gp01
        << "\np01:    " << p01
        << "\nhp01:   " << hp01
      
        << "\ngp10:   " << gp10
        << "\np10:    " << p10
        << "\nhp10:   " << hp10

        << "\ngp11:   " << gp11
        << "\np11:    " << p11
        << "\nhp11:   " << hp11

        << "\ngpMid:  " << gpMid
        << "\nhpMid:  " << hpMid

        << endl;

   delete dh;
   
   return 0;
}

//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// This is less of a test and more of an example of how to create a
// relationship between a local space rectangular (LSR) coordinate system
// and the earth-centered, earth-fixed (ECEF) system.
//
// $Id: ossim-lsr-space-test.cpp 19751 2011-06-13 15:13:07Z dburken $
//----------------------------------------------------------------------------

#include <iostream>
using namespace std;


#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/base/ossimEcefPoint.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimLsrSpace.h>
#include <ossim/base/ossimLsrPoint.h>
#include <ossim/init/ossimInit.h>

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   // Create a point to use as the origin for our local space.
   ossimGpt origin(28.0, -81.0, 0.0);

   // Create the lsr space which can go lsr <--> ecef.
   ossimLsrSpace frame(origin);

   // For a shift from the lsr origin in decimal degrees.
   const double ONE_ARC_SEC = 1.0 / 3600.0;
   const double TWO_ARC_SEC = ONE_ARC_SEC * 2.0;

   //---
   // Example 1:
   // Make a test lsr point shifted one second in latitude and
   // two in longitude from the origin and then back from lsr to a ecef
   // point to a ground point.
   //---
   ossimGpt gpt1(origin.latd() + ONE_ARC_SEC,
                 origin.lond() + TWO_ARC_SEC, 0.0);

   // Local point.
   ossimLsrPoint lsrPt1(gpt1, frame);
   
   // Shift it back to ground point.
   ossimEcefPoint ecefPt1(lsrPt1);
   ossimGpt gpt2(ecefPt1);

   //---
   // Example 2:
   // Make a test lsr point shifted in meters from the origin and
   // then back from lsr to a ecef point to a ground point.
   //
   // Note this is the same shift in meter as example 1 in decimal degrees.
   //---
   // Local point.
   ossimLsrPoint lsrPt2(54.645342075675977, 30.783305741876902, 0.0, frame);
   
   // Shift it back to ground point.
   ossimEcefPoint ecefPt2(lsrPt2);
   ossimGpt gpt3(ecefPt2);
  
   cout << "origin:       " << origin
        << "\necef pt:      " << ossimEcefPoint(origin)
        << "\necef pt to origin:      " << ossimGpt(ossimEcefPoint(origin))
        << "\nframe:     " << frame

        << "\n\nExample 1 ground pt:\n"
        << gpt1
        << "\n\nExample 1 lsr pt:\n"
        << lsrPt1
        << "\n\Example 1 lsr pt shifted back to ground:\n"
        << gpt2

        << "\n\nExample 2 lsr pt:\n"
        << lsrPt2
        << "\nExample 2 lsr pt shifted back to ground:\n"
        << gpt3
        << endl;

   return 0;
}

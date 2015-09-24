//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description: Test app for ossimString class.
//
// $Id: ossim-duration-test.cpp 19751 2011-06-13 15:13:07Z dburken $
//----------------------------------------------------------------------------
#include <iostream>
#include <ossim/base/ossimDuration.h>
#include <ossim/base/ossimCommon.h>


int main(int argc, char *argv[])
{
   ossimDuration duration;
   ossimString convertBack;
   ossimString testString("-P1DT25S");
   
   duration.setByIso8601DurationString(testString);
   duration.toIso8601DurationString(convertBack);
   if(testString != convertBack)
   {
      std::cout << "ERROR: " << testString << " != " << convertBack << std::endl;
   }
   if(!ossim::almostEqual(duration.toSeconds(), -86425.0, .000000001))
   {
      std::cout << "Seconds calculation is bad for " << testString << std::endl;
   }
   testString = "P12Y1M1DT10M25.5S";
   duration.setByIso8601DurationString(testString);
   duration.toIso8601DurationString(convertBack);
   if(testString != convertBack)
   {
      std::cout << "ERROR: " << testString << " != " << convertBack << std::endl;
   }
   
   return 0;
}

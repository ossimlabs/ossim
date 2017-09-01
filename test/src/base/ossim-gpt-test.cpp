//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Test code for ossimGpt class.
// 
//----------------------------------------------------------------------------
// $Id: ossim-gpt-test.cpp 19751 2011-06-13 15:13:07Z dburken $

#include <iostream>
using namespace std;

#include <ossim/base/ossimGpt.h>
#include <ossim/init/ossimInit.h>

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   cout << "ossimGpt::wrap test:\n";
   
   ossimGpt gpt(0.0, 0.0, 0.0);
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = 45.0;
   gpt.lon = 45.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";
   
   gpt.lat = 90.0;
   gpt.lon = 180.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = 91.0;
   gpt.lon = 181.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = 179.0;
   gpt.lon = 359.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = 181.0;
   gpt.lon = 361.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = 271.0;
   gpt.lon = 361.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = 359.0;
   gpt.lon = 361.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = 361.0;
   gpt.lon = 721.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = 451.0;
   gpt.lon = 901.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = -45.0;
   gpt.lon = -45.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";
   
   gpt.lat = -90.0;
   gpt.lon = -180.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = -91.0;
   gpt.lon = -181.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = -179.0;
   gpt.lon = -359.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = -181.0;
   gpt.lon = -361.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = -271.0;
   gpt.lon = -361.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = -359.0;
   gpt.lon = -361.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = -361.0;
   gpt.lon = -721.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   gpt.lat = -451.0;
   gpt.lon = -901.0;
   cout << "gpt: " << gpt << "\n";
   gpt.wrap();
   cout << "gpt::wrap: " << gpt << "\n\n";

   
   return 0;
}

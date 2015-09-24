//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Contains application for simple ossimDms class code examples.
//
// $Id: ossim-dms-test.cpp 23494 2015-08-28 15:11:03Z okramer $
//----------------------------------------------------------------------------

#include <ossim/base/ossimDms.h>
#include <iostream>
using namespace std;

int main()   
{
   // Test default constructor.
   ossimDms dms;
   cout << "dms using default constuctor:"
        << "\ndms.getDegrees():  " << dms.getDegrees()
        << "\ndms.toString():    " << dms.toString()
        << endl;
   
   const ossimString LAT_FORMAT = "dd@ mm' ss.ssss\" C";
   const ossimString LON_FORMAT = "ddd@ mm' ss.ssss\" C";

   ossimDms d(0.0);

   ossimString dmsstr = "S30 20 20";
   
   if (d.setDegrees(dmsstr) == true)
   {
      cout << "dmsstr:          " << dmsstr
           << "\nd.getDegrees():  " << d.getDegrees()
           << "\nd.toString():    " << d.toString(LAT_FORMAT)
           << endl;
   }
   else
   {
      cout << "setDegrees failed..." << endl;
   }

   dmsstr = " N30 20 20";

   if (d.setDegrees(dmsstr))
   {
      cout << "dmsstr:          " << dmsstr
           << "\nd.getDegrees():  " << d.getDegrees()
           << "\nd.toString():    " << d.toString(LAT_FORMAT)
           << endl;
   }
   else
   {
      cout << "setDegrees failed..." << endl;
   }

   dmsstr = "/some/one/punched/in/a/file/name...";

   if (d.setDegrees(dmsstr))
   {
      cout << "dmsstr:          " << dmsstr
           << "\nd.getDegrees():  " << d.getDegrees()
           << "\nd.toString():    " << d.toString(LAT_FORMAT)
           << endl;
   }
   else
   {
      cout << "setDegrees failed..." << endl;
   }

   dmsstr = "32 59 00 N";
   d.setLatFlag(true);
   if (d.setDegrees(dmsstr))
   {
      cout << "dmsstr:          " << dmsstr
           << "\nd.getDegrees():  " << d.getDegrees()
           << "\nd.toString():    " << d.toString(LAT_FORMAT)
           << endl;
   }
   else
   {
      cout << "setDegrees failed..." << endl;
   }

   dmsstr = "085 00 00 E";
   d.setLatFlag(false);
   if (d.setDegrees(dmsstr))
   {
      cout << "dmsstr:          " << dmsstr
           << "\nd.getDegrees():  " << d.getDegrees()
           << "\nd.toString():    " << d.toString(LAT_FORMAT)
           << endl;
   }
   else
   {
      cout << "setDegrees failed..." << endl;
   }
   return 0;
}

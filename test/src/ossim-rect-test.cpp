//----------------------------------------------------------------------------
//
// File: ossim-rect-test.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:
//
// Test app for ossimIrect and ossimDrect classes.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimIrect.h>

#include <iostream>
using namespace std;

int main()
{
   // IRECTS:

   // left handed irects
   ossimIrect ir1(0, 0, 10, 10);
   ossimIrect ir2(1, 1, 9, 9);
   ossimIrect ir3 = ir1.clipToRect(ir2);
   cout << "ossimIrect Left Handed:"
        << "\nrect ir1:            " << ir1
        << "\nrect ir2:            " << ir2
        << "\nir1.clipToRect(ir2): " << ir3
        << "\nexpected: ul=(1,1), ur=(9,1), lr=(9,9), ll=(1,9)\n\n";
   
   // right handed irects
   ossimIrect ir4(0,10, 10,0,OSSIM_RIGHT_HANDED);
   ossimIrect ir5(-5,8,14,1,OSSIM_RIGHT_HANDED);
   ossimIrect ir6 = ir4.clipToRect(ir5);
   cout << "ossimIrect Right Handed:"
        << "\nrect ir4:          " << ir4
        << "\nrect ir5:          " << ir5
        << "\nd.clipToRect(e):   " << ir6
        << "\nexpected: ul=(0,8), ur=(10,8), lr=(10,1), ll=(0,1)\n\n";

   // DRECTS:

   // left handed irects
   ossimDrect dr1(0, 0, 10, 10);
   ossimDrect dr2(1, 1, 9, 9);
   ossimDrect dr3 = dr1.clipToRect(dr2);
   cout << "ossimDrect Left Handed:"
        << "\nrect dr1:            " << dr1
        << "\nrect dr2:            " << dr2
        << "\ndr1.clipToRect(dr2): " << dr3
        << "\nexpected: ul=(1,1), ur=(9,1), lr=(9,9), ll=(1,9)\n\n";
   
   // right handed drects
   ossimDrect dr4(0,10, 10,0,OSSIM_RIGHT_HANDED);
   ossimDrect dr5(-5,8,14,1,OSSIM_RIGHT_HANDED);
   ossimDrect dr6 = dr4.clipToRect(dr5);
   cout << "ossimDrect Right Handed:"
        << "\nrect dr4:          " << dr4
        << "\nrect dr5:          " << dr5
        << "\nd.clipToRect(e):   " << dr6
        << "\nexpected: ul=(0,8), ur=(10,8), lr=(10,1), ll=(0,1)\n"
        << endl;

   return 0;
}


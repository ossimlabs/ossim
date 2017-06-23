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
#include <ossim/base/ossimGrect.h>

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


  ossimDrect dComletelyWithinSource(0,0,10,10);
  ossimDrect dWithinEqual(0,0,10,10);
  ossimDrect dWithinLess(5,5,8,8);
  ossimDrect dWithinTop(0,-1,10,9);
  ossimDrect dWithinLeft(-1,0,9,10);
  ossimDrect dWithinRight(10,0,20,20);
  ossimDrect dWithinBottom(0,10,20,20);
  ossimIrect dWithinWayOut(50000,50000,100000,100000);

  std::cout << "ossimDrect completely within tests:" 
        << "\nrect source:          " << dComletelyWithinSource
        << "\nrect dWithinEqual:          " << dWithinEqual
        << "\nrect dWithinLess:          " << dWithinLess
        << "\nrect dWithinTop:          " << dWithinTop
        << "\nrect dWithinLeft:          " << dWithinLeft
        << "\nrect dWithinRight:          " << dWithinRight
        << "\nrect dWithinBottom:          " << dWithinBottom
        << "\nrect dWithinWayOut:          " << dWithinWayOut
        << "\nrect  dWithinEqual  " << dWithinEqual.completely_within(dComletelyWithinSource)
        << "\nrect  dWithinLess  " << dWithinLess.completely_within(dComletelyWithinSource)
        << "\nrect  dWithinTop  " << dWithinTop.completely_within(dComletelyWithinSource)
        << "\nrect  dWithinLeft  " << dWithinLeft.completely_within(dComletelyWithinSource)
        << "\nrect  dWithinRight  " << dWithinRight.completely_within(dComletelyWithinSource)
        << "\nrect  dWithinBottom  " << dWithinBottom.completely_within(dComletelyWithinSource)
        << "\nrect  dWithinWayOut  " << dWithinWayOut.completely_within(dComletelyWithinSource)
        << "\nexpected: 1, 1, 0, 0, 0, 0, 0\n"
        << endl;

  ossimIrect iComletelyWithinSource(0,0,10,10);
  ossimIrect iWithinEqual(0,0,10,10);
  ossimIrect iWithinLess(5,5,8,8);
  ossimIrect iWithinTop(0,-1,10,9);
  ossimIrect iWithinLeft(-1,0,9,10);
  ossimIrect iWithinRight(10,0,20,20);
  ossimIrect iWithinBottom(0,10,20,20);
  ossimIrect iWithinWayOut(50000,50000,100000,100000);

  std::cout << "ossimIrect completely within tests:" 
        << "\nrect source:          " << iComletelyWithinSource
        << "\nrect iWithinEqual:          " << iWithinEqual
        << "\nrect iWithinLess:          " << iWithinLess
        << "\nrect iWithinTop:          " << iWithinTop
        << "\nrect iWithinLeft:          " << iWithinLeft
        << "\nrect iWithinRight:          " << iWithinRight
        << "\nrect iWithinBottom:          " << iWithinBottom
        << "\nrect iWithinWayOut:          " << iWithinWayOut
        << "\nrect  iWithinEqual  " << iWithinEqual.completely_within(iComletelyWithinSource)
        << "\nrect  iWithinLess  " << iWithinLess.completely_within(iComletelyWithinSource)
        << "\nrect  iWithinTop  " << iWithinTop.completely_within(iComletelyWithinSource)
        << "\nrect  iWithinLeft  " << iWithinLeft.completely_within(iComletelyWithinSource)
        << "\nrect  iuWithinRight  " << iWithinRight.completely_within(iComletelyWithinSource)
        << "\nrect  uWithinBottom  " << iWithinBottom.completely_within(iComletelyWithinSource)
        << "\nrect  iWithinWayOut  " << iWithinWayOut.completely_within(iComletelyWithinSource)
        << "\nexpected: 1, 1, 0, 0, 0, 0, 0\n"
        << endl;


  ossimGrect gComletelyWithinSource(ossimGpt(0,0),ossimGpt(10,10));
  ossimGrect gWithinEqual(ossimGpt(0,0),ossimGpt(10,10));
  ossimGrect gWithinLess(ossimGpt(5,5),ossimGpt(8,8));
  ossimGrect gWithinTop(ossimGpt(0,-1),ossimGpt(10,9));
  ossimGrect gWithinLeft(ossimGpt(-1,0),ossimGpt(9,10));
  ossimGrect gWithinRight(ossimGpt(10,0),ossimGpt(20,20));
  ossimGrect gWithinBottom(ossimGpt(0,10),ossimGpt(20,20));
  ossimGrect gWithinWayOut(ossimGpt(45,45),ossimGpt(50,50));

  std::cout << "ossimGrect completely within tests:" 
        << "\nrect source:          " << gComletelyWithinSource
        << "\nrect gWithinEqual:          " << gWithinEqual
        << "\nrect gWithinLess:          " << gWithinLess
        << "\nrect gWithinTop:          " << gWithinTop
        << "\nrect gWithinLeft:          " << gWithinLeft
        << "\nrect gWithinRight:          " << gWithinRight
        << "\nrect gWithinBottom:          " << gWithinBottom
        << "\nrect gWithinWayOut:          " << gWithinWayOut
        << "\nrect  gWithinEqual  " << gWithinEqual.completely_within(gComletelyWithinSource)
        << "\nrect  gWithinLess  " << gWithinLess.completely_within(gComletelyWithinSource)
        << "\nrect  gWithinTop  " << gWithinTop.completely_within(gComletelyWithinSource)
        << "\nrect  gWithinLeft  " << gWithinLeft.completely_within(gComletelyWithinSource)
        << "\nrect  gWithinRight  " << gWithinRight.completely_within(gComletelyWithinSource)
        << "\nrect  gWithinBottom  " << gWithinBottom.completely_within(gComletelyWithinSource)
        << "\nrect  gWithinWayOut  " << gWithinWayOut.completely_within(gComletelyWithinSource)
        << "\nexpected: 1, 1, 0, 0, 0, 0, 0\n"
        << endl;



   return 0;
}


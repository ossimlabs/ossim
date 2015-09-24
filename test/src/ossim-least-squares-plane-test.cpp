//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Test code for generic ossim test.
//
// $Id: ossim-least-squares-plane-test.cpp 23148 2015-02-13 16:39:40Z okramer $
//----------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
using namespace std;

#include <ossim/base/ossimLeastSquaresPlane.h>
#include <ossim/base/ossimCommon.h>
//#include <ossim/init/ossimInit.h>


int main(int argc, char *argv[])
{
   //ossimInit::instance()->initialize(argc, argv);

   ossimLeastSquaresPlane lsp;

   lsp.addSample(0, 0, 4.5);
   lsp.addSample(1, 0, 5.5);
   lsp.addSample(0, 1, 7.5);
   lsp.addSample(1, 1, 7.5);
   lsp.addSample(-1, 0, 2.5);
   lsp.addSample(0, -1, 0.5);
   lsp.addSample(-1, -1, -0.5);

   lsp.solveLS();

   double a, b, c;
   lsp.getLSParms(a, b, c);

   cout << "\n   a = "<<a<<"\n   b = "<<b<<"\n   c = "<<c<<endl;

   return 0;
}

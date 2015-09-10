//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Test normalized remap table.
// 
//----------------------------------------------------------------------------
// $Id: ossim-remap-table-test.cpp 19751 2011-06-13 15:13:07Z dburken $

#include <iostream>
#include <iomanip>
using namespace std;

#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/imaging/ossimNormalizedS16RemapTable.h>
#include <ossim/imaging/ossimNormalizedU16RemapTable.h>
#include <ossim/imaging/ossimNormalizedU11RemapTable.h>
#include <ossim/imaging/ossimNormalizedU8RemapTable.h>


int main(int argc, char *argv[])
{
   cout << setiosflags(std::ios::fixed) << setprecision(15);

   ossim_float64 n;
   ossim_int32   p;
   ossim_int32   i;
   
   cout << "//------------------------------------------\n"
        << "// *** ossimNormalizedS16RemapTable test ***\n"
        << "//------------------------------------------\n"
        << endl;
   
   ossimNormalizedS16RemapTable s16tbl;
   
   i = -32768;
   while (i < 32768)
   {
      n = s16tbl[i];
      p = s16tbl.pixFromNorm(n);
      cout << "idx[" << i << "]: n (" << n << "), p(" << p << ")\n";
      ++i;
   }

   n = -0.1;
   p = s16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";
    
   n = 0.0;
   p = s16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.000000250; // Just above null pixel
   p = s16tbl.pixFromNorm(n);
   cout << "Just above null\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";
   
   n = 0.000015250; // Slightly less than -32767
   p = s16tbl.pixFromNorm(n);
   cout << "Slightly less than -32767\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.0000152588; // -32767
   p = s16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";
   
   n = 0.000015258;  // Slightly bigger than -32767
   p = s16tbl.pixFromNorm(n);
   cout << "Slightly bigger than -32767\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.000030518043793; // -32766
   p = s16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";
   
   n = 0.0000305180436;  // Slightly to the left of -32766
   p = s16tbl.pixFromNorm(n);
   cout << "Slightly less than -32766\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.5;
   p = s16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.999984740977; // Slightly less than 32766
   p = s16tbl.pixFromNorm(n);
   cout << "Slightly less than 32766\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.999984740978103; // 32766
   p = s16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.99998474098; // Slightly greater than 32766
   p = s16tbl.pixFromNorm(n);
   cout << "Slightly greater than 32766\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";
   
   n = 1.0;
   p = s16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";
   
   n = 1.1;
   p = s16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   p = -32768;
   n = s16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";
   
   p = -32767;
   n = s16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 0;
   n = s16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 32766.99999999999;
   n = s16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 32767;
   n = s16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 32768;
   n = s16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   cout << "//------------------------------------------\n"
        << "// *** ossimNormalizedU16RemapTable test ***\n"
        << "//------------------------------------------\n"
        << endl;
   
   ossimNormalizedU16RemapTable u16tbl;
   
   i = 0;
   while (i < 65536)
   {
      n = u16tbl[i];
      p = u16tbl.pixFromNorm(n);
      cout << "idx[" << i << "]: n (" << n << "), p(" << p << ")\n";
      ++i;
   }

   n = -0.1;
   p = u16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";


   n = 0.0;
   p = u16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.001;
   p = u16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.000000250; // Just above null pixel
   p = u16tbl.pixFromNorm(n);
   cout << "Just above null\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 1.0;
   p = u16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 1.1;
   p = u16tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   p = -1;
   n = u16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 0;
   n = u16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 1;
   n = u16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 65534;
   n = u16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 65535;
   n = u16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 65536;
   n = u16tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";
      
   cout << "\n\n//------------------------------------------\n"
        << "// *** ossimNormalizedU11RemapTable test ***\n"
        << "//------------------------------------------\n"
        << endl;
   
   ossimNormalizedU11RemapTable u11tbl;
   
   i = 0;
   while (i < 2048)
   {
      n = u11tbl[i];
      p = u11tbl.pixFromNorm(n);
      cout << "idx[" << i << "]: n (" << n << "), p(" << p << ")\n";
      ++i;
   }

   n = -0.1;
   p = u11tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";
    
   n = 0.0;
   p = u11tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.00000852; // Just above null pixel
   p = u11tbl.pixFromNorm(n);
   cout << "Just above null\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.000;
   p = u11tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";
   
   n = .000488519785; // Slightly less than 1
   p = u11tbl.pixFromNorm(n);
   cout << "Slightly less than 1\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.000488519785051; // 1
   cout << "1\n";
   p = u11tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.000488519786;  // Slightly bigger than 1
   p = u11tbl.pixFromNorm(n);
   cout << "Slightly bigger than 1\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.9995114802149; // Slightly less than 2046
   p = u11tbl.pixFromNorm(n);
   cout << "Slightly less than 2046\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.999511480214949; // 2046
   p = u11tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.999511480215; // Slightly greater than 2046
   p = u11tbl.pixFromNorm(n);
   cout << "Slightly greater than 2046\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";
   
   n = 1.0;
   p = u11tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";
   
   n = 1.1;
   p = u11tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   p = -1;
   n = u11tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";
   
   p = 0;
   n = u11tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 2046;
   n = u11tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 2047;
   n = u11tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 2048;
   n = u11tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   cout << "\n\n//------------------------------------------\n"
        << "// *** ossimNormalizedU8RemapTable test ***\n"
        << "//------------------------------------------\n"
        << endl;
   
   ossimNormalizedU8RemapTable u8tbl;
   
   i = 0;
   while (i < 256)
   {
      n = u8tbl[i];
      p = u8tbl.pixFromNorm(n);
      cout << "idx[" << i << "]: n (" << n << "), p(" << p << ")\n";
      ++i;
   }

   n = -0.1;
   p = u8tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";
    
   n = 0.0;
   p = u8tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.00000625; // Just above null pixel
   p = u8tbl.pixFromNorm(n);
   cout << "Just above null\n";
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 0.001;
   p = u8tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 1.0;
   p = u8tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 1.1;
   p = u8tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   n = 1.0;
   p = u8tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";
   
   n = 1.1;
   p = u8tbl.pixFromNorm(n);
   cout << "pixFromNorm(" << n << "): " << p << "\n";

   p = -1;
   n = u8tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";
   
   p = 0;
   n = u8tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 254;
   n = u8tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 255;
   n = u8tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";

   p = 256;
   n = u8tbl.normFromPix(p);
   cout << "normFromPix(" << p << "): " << n << "\n";
   
   return 0;
}

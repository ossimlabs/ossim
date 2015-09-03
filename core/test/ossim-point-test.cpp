//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
//----------------------------------------------------------------------------
// $Id: ossim-point-test.cpp 19751 2011-06-13 15:13:07Z dburken $

#include <string>
#include <iostream>
#include <sstream>

#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimDpt3d.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimEcefPoint.h>
#include <ossim/base/ossimEcefVector.h>

int main()
{
   //---
   // Test ossiIpt::operator>>
   //---
   std::string is1(" ( 0, 1 )");
   std::string is2(" (2,3)");
   std::string is3(" ( 4, 5 )");
   std::string is4_5_6_7(" ( 6, 7 )(8, 9) ( 10, 11 ) ( 12, 13) 9876");

   ossimIpt ip1;
   ossimIpt ip2;
   ossimIpt ip3;
   ossimIpt ip4;
   ossimIpt ip5;
   ossimIpt ip6;
   ossimIpt ip7;
   

   ip1.toPoint(is1);
   ip2.toPoint(is2);
   ip3.toPoint(is3);
   int i;

   std::istringstream istr(is4_5_6_7);
   istr >> ip4 >> ip5 >> ip6 >> ip7 >> i;

   //---
   // Test ossiDpt::operator>>
   //---
   std::string ds1(" ( 0.0, 1.1 )");
   std::string ds2(" (2.2,3.3)");
   std::string ds3(" ( 4.4, 5.5 )");
   std::string ds4_5_6_7(" ( 6.6, 7.7 )(8.8, 9.9) ( 10.0, 11.0 ) ( 12.0, 13.0) 9876.12345678");
   std::string ds8("12 20");

   ossimDpt dp1;
   ossimDpt dp2;
   ossimDpt dp3;
   ossimDpt dp4;
   ossimDpt dp5;
   ossimDpt dp6;
   ossimDpt dp7;
   ossimDpt dp8;

   dp1.toPoint(ds1);
   dp2.toPoint(ds2);
   dp3.toPoint(ds3);
   double d;

   std::istringstream istr2(ds4_5_6_7);
   istr2 >> dp4 >> dp5 >> dp6 >> dp7 >> d;

   dp8.toPoint(ds8); // Test an invalid string "12 20"

    //---
   // Test ossiDpt3d
   //---
   std::string ds3d1  = " ( 0.0, 1.1, 2.2 )";
   std::string ds3d2 = "(1.0,2.0,3.0)";
   
   ossimDpt3d dp3d1;
   ossimDpt3d dp3d2;
   dp3d1.toPoint(ds3d1);
   dp3d2.toPoint(ds3d2);


   //---
   // Test ossiGpt::operator>>
   //---
   std::string gs1("(0.0,0.0,0.0,WGE)");
   std::string gs2("(1.1,2.2,3.3,NAR-C)");
   std::string gs3(" (4.4,5.5,6.6,NAS-C )");
   std::string gs4_5_6_7(" (4.4,5.5,6.6,NAS-C )( 10.0, 10.0 ,5.0, TOY-C ) (17, -89, 50.0, xxx) (28.2, -44.5, 10000.0, NAS-B) 12345.6789");

   ossimGpt gp1;
   ossimGpt gp2;
   ossimGpt gp3;
   ossimGpt gp4;
   ossimGpt gp5;
   ossimGpt gp6;
   ossimGpt gp7;
   double d2;

   gp1.toPoint(gs1);
   gp2.toPoint(gs2);
   gp3.toPoint(gs3);

   std::istringstream istr4(gs4_5_6_7);
   istr4 >> gp4 >> gp5 >> gp6 >> gp7 >> d2;


   //---
   // Test ossimEcefPoint toString and toPoint methods.
   //---
   std::string es1("(1.0,2.0,3.0)");
   ossimEcefPoint ep1;
   ep1.toPoint(es1);
   std::string es2 = ep1.toString(10).string();

   //---
   // Test ossimEcefPoint toString and toPoint methods.
   //---
   ossimEcefVector ev1;
   ev1.toPoint(es1);
   std::string es3 = ev1.toString(10).string();
  
   std::cout
      << "\nis1:       " << is1
      << "\nip1:       " << ip1
      << "\nis2:       " << is2
      << "\nip2:       " << ip2
      << "\nis3:       " << is3
      << "\nip3:       " << ip3
      << "\nis4_5_6_7: " << is4_5_6_7
      << "\nip4:       " << ip4
      << "\nip5:       " << ip5
      << "\nip6:       " << ip6
      << "\nip7:       " << ip7
      << "\ni:         " << i

      << "\n\n\nds1:       " << ds1
      << "\ndp1:       " << dp1
      << "\nds2:       " << ds2
      << "\ndp2:       " << dp2
      << "\nds3:       " << ds3
      << "\ndp3:       " << dp3
      << "\nds4_5_6_7: " << ds4_5_6_7
      << "\ndp4:       " << dp4
      << "\ndp5:       " << dp5
      << "\ndp6:       " << dp6
      << "\ndp7:       " << dp7
      << "\nds8:       " << ds8
      << "\ndp8:       " << dp8
      << "\nd:         " << d

      << "\n\nds3d1:       " << ds3d1
      << "\nds3d2:     " << ds3d2
      << "\ndp3d1:     " << dp3d1
      << "\ndp3d2:     " << dp3d2

      << "\n\n\ngs1:       " << gs1
      << "\ngp1:       " << gp1
      << "\ngs2:       " << gs2
      << "\ngp2:       " << gp2
      << "\ngs3:       " << gs3
      << "\ngp3:       " << gp3
      << "\ngs4_5_6_7: " << gs4_5_6_7
      << "\ngp4:       " << gp4
      << "\ngp5:       " << gp5
      << "\ngp6:       " << gp6
      << "\ngp7:       " << gp7
      << "\nd2:         " << d2

      << "\n\n\nes1:       " << es1
      << "\nep1:       " << ep1
      << "\nes2:       " << es2
      << "\nev1:       " << ev1
      << "\nes3:       " << es3

      << std::endl;

   return 0;
}

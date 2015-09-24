//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Test code for generic ossim test.
//
// 
//----------------------------------------------------------------------------
// $Id: ossim-date-test.cpp 19751 2011-06-13 15:13:07Z dburken $

#include <iostream>
using namespace std;

#include <ossim/base/ossimDate.h>
#include <ossim/init/ossimInit.h>

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   std::string s1 = "2009-02-05T11:26:13.458650Z";
   ossimLocalTm d1;
   d1.setIso8601(s1);
   
   std::string s2 = "2009-01-11T09:06:05.606000Z";
   ossimLocalTm d2;
   d2.setIso8601(s2);

   cout << "s1: " << s1 << "\n";
   d1.dump(std::cout);
   cout << "s2: " << s2 << "\n";
   d2.dump(std::cout);

   ossimLocalTm d4;
   cout << "local time:\n";
   d4.dump(cout);
 
   ossimLocalTm d5 = d4.convertToGmt();
   cout << "local time converted to GMT:\n";
   d5.dump(cout);

   
   ossimDate d6;
   cout << "Date:\n";
   d6.print(cout);
   cout << "\n";
   d6.dump(cout);
   cout << endl;

   return 0;
}


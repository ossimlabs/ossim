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

#include <ossim/base/ossimArgumentParser.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>

using namespace std;

template <typename T, size_t N> size_t SizeOfArray( const T(&)[ N ] ) { return N; }

#define DO_TEST true

void fail()
{
#if DO_TEST
   cout<<"\nFAILED ossimArgumentParser TEST"<<endl;
   exit(-1);
#endif
}

int main(int argc, char *argv[])
{
   // define alternate command line:
#if DO_TEST
   char* test_argv[]  = { argv[0],
                          "-b", "B1", "B2",
                          "-a", "A1",
                          "-a", "A2",
                          "-c", "C1,", "C2,C3,", "C4",
                          "arg1", "arg2" };
   int test_argc = SizeOfArray(test_argv);
   ossimArgumentParser ap (&test_argc, test_argv);
#else
   ossimArgumentParser ap (&argc, argv);
#endif

   std::string ts1;
   std::string ts2;

   if ( !ap.read("-a", ts1) || (ts1 != "A1"))
      fail();

   if ( !ap.read("-a", ts1) || (ts1 != "A2"))
      fail();

   if ( !ap.read("-b", ts1, ts2) || (ts1 != "B1") || (ts2 != "B2"))
      fail();

   vector<ossimString> param_list;
   if ( !ap.read("-c", param_list) || (param_list.size() != 4) ||
         (param_list[0] != "C1") ||
         (param_list[1] != "C2") ||
         (param_list[2] != "C3") ||
         (param_list[3] != "C4"))
   {
      fail();
   }

   if (( ap.argc() != 3 ) || (strcmp(ap[1], "arg1") != 0) || (strcmp(ap[2], "arg2") != 0))
      fail();


   cout<<"\nPASSED ossimArgumentParser TEST"<<endl;
   exit(0);
}

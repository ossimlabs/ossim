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


   //---
   // insert() must leave the parser owning every element it will later free.
   //
   // Built from a main()-style argv -- the usual case -- the original strings
   // belong to the process. insert() copied those pointers into its new array
   // and then set its "memory allocated" flag, so the destructor delete[]'d
   // memory it never allocated and the process aborted at exit. Any use of
   // insert() did this.
   //
   // The parser below is SCOPED so its destructor runs here. Every test above
   // ends at exit(), which skips destructors, so none of them could have
   // caught this no matter what they asserted.
   //---
   {
      char* ins_argv[] = { argv[0], (char*)"keep1", (char*)"keep2" };
      int ins_argc = SizeOfArray(ins_argv);
      ossimArgumentParser ins_ap (&ins_argc, ins_argv);

      ins_ap.insert(1, "--flag value");

      if (ins_ap.argc() != 5)
         fail();
      if ((strcmp(ins_ap[1], "--flag") != 0) || (strcmp(ins_ap[2], "value") != 0))
         fail();
      if ((strcmp(ins_ap[3], "keep1") != 0) || (strcmp(ins_ap[4], "keep2") != 0))
         fail();

      std::string ins_val;
      if ( !ins_ap.read("--flag", ins_val) || (ins_val != "value") )
         fail();
   }  // <-- destructor runs HERE; before the fix this aborted the process


   cout<<"\nPASSED ossimArgumentParser TEST"<<endl;
   exit(0);
}

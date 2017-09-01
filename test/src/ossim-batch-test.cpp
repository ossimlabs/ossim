//----------------------------------------------------------------------------
//
// File ossim-batch-test.cpp
// 
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// Description: Test code application ossim batch test.
//
//----------------------------------------------------------------------------
// $Id: ossim-batch-test.cpp 20675 2012-03-10 16:30:04Z dburken $

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/util/ossimBatchTest.h>
#include <ossim/init/ossimInit.h>
#include <ostream>

//**************************************************************************************************
// MAIN MODULE
// See ossimBatchTest.cpp for usage, or run executable with no args for usage.
// Returns:
//   -1 if any error occurred during test
//    1 if any test failed
//    0 if all tests passed
//**************************************************************************************************
int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);


   ossimApplicationUsage* au = ap.getApplicationUsage();
   au->setDescription(
         "This application is used for functional testing of OSSIM components. The tests are"
         "\nspecified within the configuration file passed in.  The following status codes are"
         "\nreturned on exit:\n"
         "\n    0  Test passed"
         "\n    1  No test performed (disabled)"
         "\n    2  Problem encountered during test initialization."
         "\n    3  Error occurred during test execution."
         "\n    4  Test ran but failed.\n"
         "\nFor enabling individual tests, you can edit the \"enabled\" flag in the configuration"
         "\nfile or use the \"-r\" options described below. ");
   au->setCommandLineUsage(ap.getApplicationName()+" <test-configuration-file.kwl>");

   int return_status = 0;
   ossimBatchTest obt;
   if (!obt.initialize(ap))
   {
      return_status = 2;
   }
   else
   {
      ossim_uint8 status = obt.execute();
      
      // Bitwise and ops to isolate status bits (see ossimBatchTest.h for bit definitions)
      if (status == ossimBatchTest::TEST_TBD)
         return_status = 1; // nothing was done
      else if (status & ossimBatchTest::TEST_ERROR)
         return_status = 3; // an error occurred running a command
      else if (status & ossimBatchTest::TEST_FAILED)
         return_status = 4;  // A test failed post-processing (diff)
      else
         return_status = 0;  // Everything passed
   }

   return return_status;
}


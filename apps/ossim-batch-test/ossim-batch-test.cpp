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
#include <ossim/util/ossimBatchTest.h>
#include <ossim/init/ossimInit.h>

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

   int return_status = 0;
   ossimBatchTest obt;
   if (!obt.initialize(ap))
   {
      return_status = -1;
   }
   else
   {
      ossim_uint8 status = obt.execute();
      
      // Bitwise and ops to isolate status bits (see ossimBatchTest.h for bit definitions)
      if (status == ossimBatchTest::TEST_TBD)
         return_status = -1; // nothing was done
      else if (status & ossimBatchTest::TEST_ERROR)
         return_status = -1; // an error occurred running a command
      else if (status & ossimBatchTest::TEST_FAILED)
         return_status = 1;  // A test failed post-processing (diff)
      else
         return_status = 0;  // Everything passed
   }

   return return_status;
}


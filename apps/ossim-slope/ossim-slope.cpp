//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-slope.cpp
//
// Author:  Oscar Kramer
//
// Description: Application main() for computing the corresponding slope image (floating point
// degrees) of an input DEM image.
//
// $Id: ossim-slope.cpp 23430 2015-07-15 15:01:50Z okramer $
//----------------------------------------------------------------------------

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/util/ossimSlopeUtil.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/init/ossimInit.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->initialize(ap);
   
   ossimRefPtr<ossimSlopeUtil> slopeUtil = new ossimSlopeUtil;
   if (!slopeUtil->initialize(ap))
      exit(1);

   if (!slopeUtil->execute())
      exit(1);

   exit(0);
}

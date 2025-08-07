//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//---
// $Id$

#include <ossim/base/ossimCommon.h>  // ossim contants...
#include <ossim/init/ossimInit.h>
#include <iostream>

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);
   int rv = 0;
   std::string uuid;
   if ( ossim::generate_uuid(uuid) )
   {
      std::cout << "generated uuid: " << uuid << std::endl;
   }
   else
   {
      rv = 1;
      std::cerr << "Call to ossim::generate_uuid(...) failed!" << std::endl;
   }
   return rv;
}

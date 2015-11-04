//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Test application to spit out projection names from the
// projection factory registry.
//
// $Id: ossim-projection-factory-test.cpp 19751 2011-06-13 15:13:07Z dburken $
//----------------------------------------------------------------------------

#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimString.h>
#include <ossim/projection/ossimEpsgProjectionFactory.h>

// #include <ossim/projection/ossimGcsCodeProjectionFactory.h>
// #include <ossim/projection/ossimPcsCodeProjectionFactory.h>
// #include <ossim/projection/ossimSrsProjectionFactory.h>
// #include <ossim/projection/ossimStatePlaneProjectionFactory.h>

int main(int argc, char* argv[])   
{
   ossimInit::instance()->initialize(argc, argv);
   
   std::vector<ossimString> typeList;
   std::vector<ossimString>::const_iterator i;

   cout << "ossimEpsgProjectionFactory:\n";
   typeList.clear();
   ossimEpsgProjectionFactory::instance()->getTypeNameList(typeList);
   i = typeList.begin();
   while (i < typeList.end())
   {
      std::cout << (*i) << std::endl;
      ++i;
   }
   
   return 0;
}

#if 0
int main()   
{
   ossimInit::instance()->initialize();
   
   std::vector<ossimString> typeList;
   ossimProjectionFactoryRegistry::instance()->getTypeNameList(typeList);

   std::vector<ossimString>::const_iterator i = typeList.begin();
   while (i < typeList.end())
   {
      std::cout << (*i) << std::endl;
      ++i;
   }
   
   return 0;
}
#endif


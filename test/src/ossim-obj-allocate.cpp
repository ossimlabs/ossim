//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-foo.cpp
//
// Author:  David Burken
//
// Description: Contains application definition "ossim-foo" app.
//
// NOTE:  This is supplied for simple quick test. DO NOT checkin your test to
//        the svn repository.  Simply edit ossim-foo.cpp and run your test.
//        After completion you can do a "svn revert foo.cpp" if you want to
//        keep your working repository up to snuff.
//
// $Id: ossim-foo.cpp 20095 2011-09-14 14:37:26Z dburken $
//----------------------------------------------------------------------------

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
// Put your includes here:

#include <iostream>
#include <sstream>
using namespace std;
static void usage()
{
   cout << "ossim-obj-allocate <kwl file>" << endl;
}

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   try
   {
   	if(argc == 1)
   	{
   		usage();
   		return 0;
   	} 
		ossimKeywordlist kwl;

		if(!kwl.addFile(argv[1]))
		{
			std::cout << "FILE '" << argv[1] << "' is not a keywordlist with a type: keyword\n"; 
		}
		ossimRefPtr<ossimObject> obj = ossimObjectFactoryRegistry::instance()->createObject(kwl);
		ossimKeywordlist kwl2;
		if(obj.valid())
		{
			obj->saveState(kwl2);
			std::cout << kwl2 << std::endl;
		}
		else
		{
			std::cout << "Unable to allocate object with passed in keywordlist file\n";
		}
	      // Put your code here.
	}
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      return 1;
   }

   return 0;
}
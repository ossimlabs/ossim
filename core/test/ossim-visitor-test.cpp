//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-foo.cpp
//
// Author:  David Burken
//
// Description: Contains application definition "foo" app.
//
// NOTE:  This is supplied for simple quick test.  Makefile links with
//        libossim so you don't have to muck with that.
//        DO NOT checkin your test to the svn repository.  Simply
//        edit foo.cc (Makefile if needed) and run your test.
//        After completion you can do a "svn revert foo.cpp" if you want to
//        keep your working repository up to snuff.  Enjoy!
//
// $Id: ossim-visitor-test.cpp 22790 2014-05-16 19:19:30Z gpotts $
//----------------------------------------------------------------------------

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/base/ossimCommon.h>  // ossim contants...
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/init/ossimInit.h>


// Put your includes here:

#include <iostream>
#include <sstream>
#include <iomanip>
#include <list>
#include <algorithm>

static const char* kwl1String = "type: ossimImageChain\n"
"object1.type: ossimRectangleCutFilter\n";

static const char* kwl2String = "type: ossimImageChain\n"
"object2.type: ossimBrightnessContrastSource\n";

static const char* kwl3NestedString = "type: ossimImageChain\n"
"object10.type: ossimScalarRemapper\n"
"object30.type: ossimImageChain\n"
"object30.object1.type: ossimImageRenderer\n";

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);
   ossimKeywordlist kwl1;
   ossimKeywordlist kwl2;
   ossimKeywordlist kwl3;
   if(kwl1.parseString(kwl1String)&&
      kwl2.parseString(kwl2String)&&
      kwl3.parseString(kwl3NestedString))
   {
      ossimRefPtr<ossimObject> obj1 =  ossimObjectFactoryRegistry::instance()->createObject(kwl1);
      ossimRefPtr<ossimObject> obj2 =  ossimObjectFactoryRegistry::instance()->createObject(kwl2);
      ossimRefPtr<ossimObject> obj3 =  ossimObjectFactoryRegistry::instance()->createObject(kwl3);
      
      ossimRefPtr<ossimConnectableObject> connectable1 = dynamic_cast<ossimConnectableObject*> (obj1.get());
      ossimRefPtr<ossimConnectableObject> connectable2 = dynamic_cast<ossimConnectableObject*> (obj2.get());
      ossimRefPtr<ossimConnectableObject> connectable3 = dynamic_cast<ossimConnectableObject*> (obj3.get());
      
      if(connectable1.valid()&&connectable2.valid()&&connectable3.valid())
      {
         connectable2->connectMyInputTo(connectable1.get());
         connectable3->connectMyInputTo(connectable2.get());
         
         ossimTypeNameVisitor visitor("ossimConnectableObject", false, ossimVisitor::VISIT_CHILDREN|ossimVisitor::VISIT_INPUTS);
         connectable3->accept(visitor);
         
         
         ossimCollectionVisitor::ListRef& collection = visitor.getObjects();
         if(collection.size() == 8)
         {
            std::cout << "PASSED..." << std::endl;
         }
         else 
         {
            std::cout << "FAILED..." << std::endl;
         }

         ossimCollectionVisitor::ListRef::iterator iter=collection.begin();
//         while(iter != collection.end())
//         {
//            std::cout << (*iter)->getClassName() << std::endl;
//            ++iter;
//         }

         ossimSourceInitializeVisitor sourceInitVisitor;

         obj1->accept(sourceInitVisitor);
      }
   }
   
   return 0;
}

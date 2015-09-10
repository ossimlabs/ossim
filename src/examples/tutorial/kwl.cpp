/*!
 *
 * OVERVIEW:
 *
 * This app uses factory and keywordlist to instatiate objects within OSSIM.  Among these
 * objects are projections, and image processing filters/mosaics and annotations.  Basically any
 * object within OSSIM that must have a state saved or loaded uses a keywordlist to do so.
 * The main keyword required by all objects is a TYPE_KW which has a value "type".  Most
 * keywords that are used in OSSIM are located in base/common/ossimKeywordNames.h
 *
 * We will also demonstrate the use of object factory registry to instantiate objects from
 * type name.
 *
 * A keyword list is basically a name value pair and you want to make sure that the name portion
 * is unique for all objects being saved to the keywordlist.  for example I may want to save 2 objects
 * to the keyword list and object 1 may have a prefix value of object1. and object 2 may have a
 * prefix value of object2. and so forth.  See sample code for demonstration.
 *
 * Note: Since multiple objects may reside within a keywodlist a prefix value is used to prefix all
 *       attributes of an object.  
 *
 *
 * PURPOSE:
 *
 * Learn how to instantiate different types of objects from a keywordlist
 * through the  ossimObjectFactoryRegistry.
 *
 */

#include <iostream>

#include "base/common/ossimCommon.h"
#include "base/data_types/ossimFilename.h"
#include "base/data_types/ossimKeywordlist.h"
#include "base/factory/ossimObjectFactoryRegistry.h"
#include "init/ossimInit.h"

using namespace std;

ossimObject* createObject(const ossimString& objectType);
void demo1();
void demo2();
void demo3();

int main(int argc, char* argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   cout << "_______________________________DEMO 1__________________________\n";
   demo1();
   
   cout << "_______________________________DEMO 2__________________________\n";
   demo2();

   cout << "_______________________________DEMO 3__________________________\n";
   demo3();
   
   ossimInit::instance()->finalize();
   
   return 0;
}


ossimObject* createObject(const ossimString& objectType)
{
   return ossimObjectFactoryRegistry::instance()->createObject(objectType);
}

/*!
 * Demo1 will just instantiate and object of the passed in type and then execute a
 * save state and print that keywordlist out.
 */
void demo1()
{
   ossimObject* obj1 = createObject("ossimRgbToGreyFilter");
   if(obj1)
   {
      ossimKeywordlist kwl;
      
      obj1->saveState(kwl);

      cout << "------------------ object 1 = " << obj1->getClassName() << " keyword list is -----------------" << endl;
      cout << kwl << endl;

      kwl.clear();
      cout << "------------------ object 1 = " << obj1->getClassName() << " keyword list with prefix  -----------------" << endl;
      obj1->saveState(kwl, "object1.");
      cout << kwl << endl;
      
   }
   else
   {
      cout << "This shouldn't happen" << endl;
   }
}

/*!
 * Save 2 objects to a keywordlist without conflict in values.  This will force us to
 * use a prefix in order to save 2 objects to the same list
 */
void demo2()
{
   ossimObject* obj1 = createObject("ossimRgbToGreyFilter");
   ossimObject* obj2 = createObject("ossimBrightnessContrastSource");
   if(obj1&&obj2)
   {
      ossimKeywordlist kwl;
      
      obj1->saveState(kwl, "object1.");
      obj2->saveState(kwl, "object2.");

      cout << "Objects " << obj1->getClassName() << " and "
           << obj2->getClassName() << " saved to the same keywordlist."
           << endl;

      cout << kwl << endl;
   }
   else
   {
      cout << "This shouldn't happen" << endl;
   }
   
}

/*!
 * Save object to keywordlist and then pass keyword list to factory and
 * instantitate an object from keywordlist.  We will do it twice, the first time
 * without a prefix and then the second time with a prefix.
 *
 * We will use 2 object types: the first one will be a filter and the second one will
 * be a projection.
 *
 * Note:  you should get identical outputs for both outputs.
 *
 */
void demo3()
{
   ossimObject* obj  = createObject("ossimRgbToGreyFilter");
   ossimObject* proj = createObject("ossimUtmProjection");
   
   if(obj&&proj)
   {
      ossimKeywordlist kwl;
      obj->saveState(kwl);
      ossimObject* objNew = ossimObjectFactoryRegistry::instance()->createObject(kwl);

      if(objNew)
      {
         ossimKeywordlist kwl2;
         objNew->saveState(kwl2);

         cout << "____________________ The Original object " << obj->getClassName() << " ___________________" << endl;
         cout << kwl << endl;
         cout << "____________________ The New object " << objNew->getClassName() << " ___________________" << endl;
         cout << kwl2 << endl;

         delete objNew;
      }
      else
      {
         cout << "ERROR: This shouldn't happen" << endl;
      }
      kwl.clear();

      proj->saveState(kwl);
      objNew = ossimObjectFactoryRegistry::instance()->createObject(kwl);

      if(objNew)
      {
         ossimKeywordlist kwl2;
         objNew->saveState(kwl2);

         cout << "____________________ The Original object " << proj->getClassName() << " ___________________" << endl;
         cout << kwl << endl;
         cout << "____________________ The New object " << objNew->getClassName() << " ___________________" << endl;
         cout << kwl2 << endl;

         delete objNew;
      }
      else
      {
         cout << "ERROR: This shouldn't happen" << endl;
      }
      
      delete obj;
      delete proj;
   }
   else
   {
      cout << "This shouldn't happen" << endl;
   }
   
}

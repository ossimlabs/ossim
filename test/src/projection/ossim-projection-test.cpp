//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Test application to for projection classes.
//
//----------------------------------------------------------------------------
// $Id: ossim-projection-test.cpp 2777 2011-06-27 19:14:17Z david.burken $

#include <ossim/base/ossimKeywordlist.h>
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimGoogleProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;


int main(int argc, char* argv[])   
{
   ossimInit::instance()->initialize(argc, argv);

   ossimRefPtr<ossimMapProjection> proj1 = new ossimEquDistCylProjection();

   cout << "\n\nproj1 orgin=(0,0):\n";
   proj1->print(cout);

   ossim_float64 dpp = 1.0/3600.0; // Degrees per pixel.
   ossimDpt scale(dpp, dpp);
   proj1->setDecimalDegreesPerPixel(scale);
   cout << "\n\nproj1 after setDecimalDegreesPerPixel(0.000277778, 0.000277778):\n";
   proj1->print(cout);

   ossimGpt origin(38.0, -123.0, 0.0);
   proj1->setOrigin(origin);

   cout << "\n\nproj1 after setOrgin=(38, -123):\n";
   proj1->print(cout);

   proj1->setDecimalDegreesPerPixel(scale);
   cout << "\n\nproj1 after setDecimalDegreesPerPixel(0.000277778, 0.000277778) with origin lat of 38N:\n";
   proj1->print(cout);

   ossimRefPtr<ossimMapProjection> proj2 =  static_cast<ossimMapProjection*>(proj1->dup());
   cout << "\n\nproj2 a dup of proj1:\n";
   proj2->print(cout);

   ossimKeywordlist kwl;
   proj2->saveState(kwl);

   cout << "\n\nproj2 save state:\n"
        << kwl << endl;

   ossimRefPtr<ossimProjection> proj3 = ossimProjectionFactoryRegistry::instance()->
      createProjection(kwl);
   cout << "\n\nproj3 created from proj2's saveState via ossimProjectionFactoryRegistry::createProjection\n";
   if ( proj3.valid() )
   {
      proj3->print(cout);
   }

   ossimRefPtr<ossimMapProjection> proj4 = new ossimEquDistCylProjection();
   proj4->loadState(kwl);
   
   cout << "\n\nproj4 from proj2 save state via ossimEquDistCylProjection::loadState:\n";
   proj4->print(cout);

   ossimRefPtr<ossimMapProjection> proj5 = new ossimGoogleProjection();

   ossimDpt dpt(-20037508.3428, 20037508.3428);
   ossimGpt gpt = proj5->inverse( dpt );
   
   cout << "\n\nossimGoogleProjection test:"
        << "\ndpt:         " << dpt
        << "\ninverse gpt: " << gpt
        << endl;

   dpt.x = 20037508.3428;
   dpt.y = -20037508.3428;
   gpt = proj5->inverse( dpt );

   cout << "\ndpt:         " << dpt
        << "\ninverse gpt: " << gpt
        << "\n" << endl;

   return 0;
}

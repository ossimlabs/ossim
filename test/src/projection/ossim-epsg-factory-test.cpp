//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-epsg-factory-test.cpp
//
// Author:  Oscar Kramer
//
// Description: Contains test of EPSG projection factory functionality
//
// NOTE:  Returns 0 on success, 1 on failure. Outputs test results to stdout.
//
// $Id: ossim-epsg-factory-test.cpp 19751 2011-06-13 15:13:07Z dburken $
//----------------------------------------------------------------------------

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/init/ossimInit.h>
#include <iostream>
#include <ossim/projection/ossimEpsgProjectionFactory.h>

using namespace std;

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   bool passed = true;
   try
   {
      ossimEpsgProjectionFactory* factory = ossimEpsgProjectionFactory::instance();
      ossimRefPtr<ossimMapProjection> proj = 0;
      

      ossimString code = "EPSG:2236"; // "NAD83 / Florida East (ftUS)"
      proj = dynamic_cast<ossimMapProjection*>(factory->createProjection(code));
      if (proj.valid() && (proj->getProjectionName()=="ossimTransMercatorProjection"))
         cout << "\nPASSED for code <"<<code<<">"<<endl;
      else
      {
         passed =false;
         cout<<"\nFAILED for code <"<<code<<">"<<endl;
      }

      ossimString name = "NAD83 / Virginia North (ftUS)"; // 2283
      proj = dynamic_cast<ossimMapProjection*>(factory->createProjection(name));
      if (proj.valid() && (proj->getPcsCode() == 2283))
         cout << "\nPASSED for name <"<<name<<">"<<endl;
      else
      {
         passed =false;
         cout<<"\nFAILED for name <"<<name<<">"<<endl;
      }

      name = "NAD_1983_HARN_StatePlane_Florida_East_FIPS_0901"; // 2777
      proj = dynamic_cast<ossimMapProjection*>(factory->createProjection(name));
      if (proj.valid() && (proj->getPcsCode() == 2777))
         cout << "\nPASSED for name <"<<name<<">"<<endl;
      else
      {
         passed =false;
         cout<<"\nFAILED for name <"<<name<<">"<<endl;
      }

      name = "NAD_1983_StatePlane_Virginia_North_FIPS_4501"; // 32146
      proj = dynamic_cast<ossimMapProjection*>(factory->createProjection(name));
      if (proj.valid() && (proj->getPcsCode() == 32146))
         cout << "\nPASSED For name <"<<name<<">"<<endl;
      else
      {
         passed =false;
         cout<<"\nFAILED for name <"<<name<<">"<<endl;
      }

      name = "NAD83_SPCS_0101"; // 26929
      proj = dynamic_cast<ossimMapProjection*>(factory->createProjection(name));
      if (proj.valid() && (proj->getPcsCode() == 26929))
         cout << "\nPASSED for name <"<<name<<">"<<endl;
      else
      {
         passed =false;
         cout<<"\nFAILED for name <"<<name<<">"<<endl;
      }

      if (proj.valid())
      {
         proj->setPcsCode(0);
         ossim_uint32 lookup = proj->getPcsCode();
         if ((lookup == 26929) || (lookup == 3465))
            cout<<"\nPASSED PCS code lookup."<<endl;
         else
         {
            passed =false;
            cout<<"\nFAILED PCS code lookup."<<endl;
         }
      }

      cout<<"\nDone"<<endl;
      proj = 0;
   }
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      passed = false;
   }
   
   if (passed)
      return 0;
   return 1;
}

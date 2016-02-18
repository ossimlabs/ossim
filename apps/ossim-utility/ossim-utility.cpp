//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <iostream>
#include <map>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/base/ossimString.h>
#include <ossim/util/ossimUtilityRegistry.h>
#include <ossim/base/ossimException.h>

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ap.getApplicationUsage()->setApplicationName(argv[0]);

   try
   {
      // Initialize ossim stuff, factories, plugin, etc.
      ossimInit::instance()->initialize(ap);

      ossimUtilityFactoryBase* factory = ossimUtilityRegistry::instance();
      map<string, string> capabilities;
      factory->getCapabilities(capabilities);

      // Display list of capabilities:
      map<string, string>::iterator iter = capabilities.begin();
      cout<<"\nCapabilities:"<<endl;
      for (;iter != capabilities.end(); ++iter)
      {
         cout<<"  "<<iter->first<<" -- "<<iter->second<<endl;
      }

      while (1)
      {
         // Query for operation:
         ossimString input;
         cout << "\nEnter utility class ('x' to exit): ";
         cin >> input;
         if (input == "x")
            break;

         // Fetch the utility object:
         ossimRefPtr<ossimUtility> utility = factory->createUtility(input.string());
         if (!utility.valid())
         {
            cout << "\nCould not create utility <"<<input<<">"<<endl;
            continue;
         }

         // Query for config filename:
         ossimKeywordlist kwl;
         cout << "\nEnter config file name or <return> for template: ";
         getline(cin,input);
         if (!input.empty())
         {
            //if (kwl.)
            //utility->initialize()
         }

         // Display API:
         ossimString api;
         utility->getKwlTemplate(api);
         kwl.parseString(api);
         cout << "\nUtility template specification: "<<endl;
         cout << kwl << endl;

         // Accept inputs:
         while (getline(cin,input))
         {
            if (!kwl.parseString(input))
               break;
         }

         // Display final KWL:
         cout << "\nUtility final specification: "<<endl;
         cout << kwl << endl;
         cout << "\nPerform operation? [y|n]: "<<endl;
         cin >> input;
         if (input == "n")
            continue;

         // Perform operation:


      }
   }
   catch  (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<e.what()<<endl;
      exit(1);
   }
   catch( ... )
   {
      cerr << "Caught unknown exception!" << endl;
   }

   exit(0);
}

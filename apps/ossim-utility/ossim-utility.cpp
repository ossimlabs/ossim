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

      do
      {
         char input[256];

         // Query for operation:
         cout << "\nEnter utility class ('x' to exit): ";
         cin.getline(input, 256);
         if (input[0] == 'x')
            break;

         // Fetch the utility object:
         ossimRefPtr<ossimUtility> utility = factory->createUtility(string(input));
         if (!utility.valid())
         {
            cout << "\nCould not create utility <"<<input<<">"<<endl;
            continue;
         }

         // Query for config filename:
         ossimKeywordlist kwl;
         bool valid_kwl = false;
         cout << "\nEnter config file name or <return> for template: " << ends;
         cin.getline(input, 256);
         if (input[0])
         {
            valid_kwl = kwl.addFile(input);
            if (!valid_kwl)
               cout<<"\nCould not load config file at <"<<input<<">"<<endl;
         }

         if (!valid_kwl)
         {
            // Display API:
            ossimKeywordlist kwl_template;
            utility->getKwlTemplate(kwl_template);
            cout << "\nUtility template specification: "<<endl;
            cout << kwl_template << endl;

            // Accept inputs:
            do
            {
               cout << "Enter keyword: value (or 'x' to finish): ";
               cin.getline(input, 256);
               if (input[0] == 'x' || (!kwl.parseString(string(input))))
                  break;
            } while (1);

            // Display final KWL:
            cout << "\nUtility final specification: "<<endl;
            cout << kwl << endl;
         }

         // Perform operation:
         while (1)
         {
            cout << "Perform operation? [y|n]: ";
            cin.getline(input, 256);
            if (input[0] == 'n')
               break;
            else if (input[0] == 'y')
            {
               utility->initialize(kwl);
               utility->execute();
               break;
            }
         }

      } while (false);
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

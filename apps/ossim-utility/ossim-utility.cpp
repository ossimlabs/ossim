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
#include <ossim/base/ossimKeywordlist.h>
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
      ossimRefPtr<ossimUtility> utility = 0;
      ossimString toolName;
      ossimKeywordlist kwl;
      char input[256];

      if ((argc > 1) && (ossimString(argv[1]) == "--help"))
      {
         cout << "\nUsage: "<<argv[0]<<" [ <job_kwl> ] \n"<<endl;
      }
      if ((argc > 1) && kwl.addFile(argv[1]))
         toolName = kwl.find("tool");

      do
      {
         if (toolName.empty())
         {
            map<string, string>::iterator iter = capabilities.begin();
            cout<<"\nCapabilities:"<<endl;
            for (;iter != capabilities.end(); ++iter)
               cout<<"  "<<iter->first<<" -- "<<iter->second<<endl;

            // Query for operation:
            cout << "\nEnter utility class ('x' to exit): ";
            cin.getline(input, 256);
            if (input[0] == 'x')
               break;
            toolName = input;
         }

         // Fetch the utility object:
         ossimRefPtr<ossimUtility> utility = factory->createUtility(toolName);
         if (!utility.valid())
         {
            cout << "\nCould not create utility <"<<toolName<<">"<<endl;
            continue;
         }

         if (kwl.getSize() == 0)
         {
            // Query for config filename:
            ossimKeywordlist kwl;
            cout << "\nEnter config file name or <return> for template: " << ends;
            cin.getline(input, 256);
            if (input[0] && !kwl.addFile(input))
               cout<<"\nCould not load config file at <"<<input<<">"<<endl;
         }

         if (kwl.getSize())
         {
            utility->initialize(kwl);
            utility->execute();
            break;
         }

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

         // Query go-aheadPerform operation:
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

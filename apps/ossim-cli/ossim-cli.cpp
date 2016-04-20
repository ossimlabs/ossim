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
      char input[4096];
      bool usingCmdLineMode = false;

      if ((argc > 1) && (ossimString(argv[1]).contains("help")))
      {
         cout << "\nUsages: "<<endl;
         cout << "    "<<argv[0]<<" <command> [command options and parameters]"<<endl;
         cout << "    "<<argv[0]<<" --version"<<endl;
         cout << "    "<<argv[0]<<"  (with no args, displays command descriptions)\n"<<endl;
         exit (0);
      }

      if  (argc > 1)
      {
         if (ossimString(argv[1]).contains("--"))
         {
            // Support ossim-info style system queries by interpreting any options as options to
            // info tool:
            toolName = "info";
            usingCmdLineMode = true;
         }
         else if (kwl.addFile(argv[1]))
         {
            // KWL filename provided, get tool name from it:
            toolName = kwl.find("tool");
            ap.remove(0);
         }
         else
         {
            // The tool name was explicitely provided on command line:
            toolName = argv[1];
            if (argc > 2)
               usingCmdLineMode = true;
            ap.remove(0);
         }
      }

      // Using one-time do-loop for breaking out when finished processing:
      do
      {
         if (toolName.empty())
         {
            map<string, string>::iterator iter = capabilities.begin();
            cout<<"\n\nAvailable commands:"<<endl;
            for (;iter != capabilities.end(); ++iter)
               cout<<"  "<<iter->first<<" -- "<<iter->second<<endl;

            // Query for operation:
            cout << "\nossim> ";
            cin.getline(input, 256);
            if (input[0] == 'q')
               break;
            toolName = input;
         }

         // Fetch the utility object:
         ossimRefPtr<ossimUtility> utility = factory->createUtility(toolName);
         if (!utility.valid())
         {
            cout << "\nDid not understand <"<<toolName<<">"<<endl;
            continue;
         }

         if (usingCmdLineMode)
         {
            // Check if user provided options along with tool name:
            // Init utility with command line
            if (!utility->initialize(ap))
            {
               cout << "\nCould not execute command with options provided."<<endl;
               continue;
            }
            if (!utility->execute())
            {
               cout << "\nAn error was encountered executing the command. Check options."<<endl;
            }
            break;
         }

         if (utility.valid() && !toolName.empty())
         {
            // Have toolname but no command line options:
            cout << "\nEnter command arguments/options or return for usage. "<<endl;
            cout << "\nossim> ";
            cin.getline(input, 4096);
            if (input[0] == 'q')
               break;

            // Create the command line with either "help" or inputs:
            ossimString cmdLine (toolName);
            cmdLine += " ";
            if (input[0] == '\0')
               cmdLine += "--help";
            else
               cmdLine += input;

            // Run command:
            ossimArgumentParser tap (cmdLine);
            utility->initialize(tap);
            if (cmdLine.contains("--help"))
            {
               toolName = "";
               continue;
            }
            utility->execute();
            break;
         }
         if (kwl.getSize() == 0)
         {
            //
            // Query for config filename:
            ossimKeywordlist kwl;
            cout << "\nEnter config file name or <return> for template: " << ends;
            cin.getline(input, 4096);
            if (input[0] && !kwl.addFile(input))
            {
               cout<<"\nCould not load config file at <"<<input<<">"<<endl;
               break;
            }
         }

         // Init utility with KWL if available:
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
            cout << "Enter \"<keyword>: <value>\" with colon separator (or 'x' to finish): ";
            cin.getline(input, 4096);
            if (input[0] == 'x' || (!kwl.parseString(string(input))))
               break;
         } while (1);

         if (kwl.getSize() == 0)
            break;

         // Display final KWL:
         cout << "\nUtility final specification: "<<endl;
         cout << kwl << endl;

         // Query go-ahead. Perform operation:
         while (1)
         {
            cout << "Perform operation? [y|n]: ";
            cin.getline(input, 4096);
            if (input[0] == 'n')
               break;
            else if (input[0] == 'y')
            {
               utility->initialize(kwl);
               utility->execute();
               break;
            }
         }

      } while (true);
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

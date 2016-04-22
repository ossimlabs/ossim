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

void usage(char* appName)
{
   cout << "\nUsages: "<<endl;
   cout << "    "<<appName<<" <command> [command options and parameters]"<<endl;
   cout << "    "<<appName<<" --spec <keyword-file>"<<endl;
   cout << "    "<<appName<<" --version"<<endl;
   cout << "    "<<appName<<"  (with no args, displays command descriptions)\n"<<endl;
}

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
      ossimKeywordlist kwl;
      char input[4096];
      bool usingCmdLineMode = false;
      ossimString cmdLine;
      vector<ossimString> cmdComponents;

      if  (argc > 1)
      {
         // Pile everything into a command line:
         for (int i=1; i<argc; ++i)
            cmdComponents.push_back(argv[i]);

         // Check for KWL command spec:
         if (  cmdComponents[0].contains("--spec") && (cmdComponents.size() >= 2) &&
               kwl.addFile(cmdComponents[1].chars()))
         {
            // KWL filename provided, get tool name from it:
            ossimString toolName = kwl.find("tool");
            if (!toolName.empty())
            {
               cout<<"\nCould not find the \"tool\" keyword in the configuration file."<<endl;
               exit(1);
            }
            cmdComponents.clear();
            cmdComponents.push_back(toolName);
         }
         else if (cmdComponents[0].contains("--") && !cmdComponents[0].contains("--help"))
         {
            // Support ossim-info style system queries by interpreting any options as options to
            // info tool (e.g., --plugins, --version, etc):
            cmdComponents.insert(cmdComponents.begin(), "info");
         }
      }

      // Using one-time do-loop for breaking out when finished processing:
      do
      {
         if (cmdComponents.empty() && (kwl.getSize() == 0))
         {
            // Need to get a command from console:
            map<string, string>::iterator iter = capabilities.begin();
            cout<<"\nAvailable commands:\n"<<endl;
            for (;iter != capabilities.end(); ++iter)
               cout<<"  "<<iter->first<<" -- "<<iter->second<<endl;
            cout<<"  "<<"help <command> -- To get help on specific command. Same as \"<command> --help\""<<endl;

            // Query for operation:
            cout << "\nossim> ";
            cin.getline(input, 256);
            if (input[0] == 'q')
               break;
            cmdLine = input;
            cmdComponents = cmdLine.split(" ");
            if (cmdComponents.empty())
               continue;
         }

         // Should have a tool name and possibly a command line. Check if help requested first:
         if (cmdComponents[0] == "help")
         {
            vector<ossimString>::iterator iter = cmdComponents.begin();
            iter = cmdComponents.erase(iter); // remove help;
            if (iter == cmdComponents.end())
            {
               usage(argv[0]);
               cmdComponents.clear();
               continue;
            }
            cmdComponents.push_back(" --help");
         }

         // Fetch the utility object based on tool name:
         ossimRefPtr<ossimUtility> utility = factory->createUtility(cmdComponents[0]);
         if (!utility.valid())
         {
            cout << "\nDid not understand <"<<cmdComponents[0]<<">"<<endl;
            cmdComponents.clear();
            continue;
         }

         // Check first for existence of KWL config:
         if (kwl.getSize())
         {
            // Init utility with KWL if available:
            utility->initialize(kwl);
            utility->execute();
            break;
         }

         if (cmdComponents.size() == 1)
         {
            // Have toolname but no command line options:
            cout << "\nEnter command arguments/options or return for usage. "<<endl;
            cout << "\nossim> ";
            cin.getline(input, 4096);
            if (input[0] == 'q')
               break;

            // Create the command line with either "help" or inputs:
            if (input[0] == '\0')
               cmdComponents.push_back("--help");
            else
               cmdComponents.push_back(input);
         }

         // Run command:
         cmdLine.join(cmdComponents, " ");
         ossimArgumentParser tap (cmdLine);
         if (cmdLine.contains("--help"))
         {
            utility->initialize(tap);
            cmdComponents.clear();
            continue;
         }
         if (!utility->initialize(tap))
         {
            cout << "\nCould not execute command <"<<cmdComponents[0]<<"> with options provided."<<endl;
            cmdComponents.clear();
            continue;
         }
         if (!utility->execute())
            cout << "\nAn error was encountered executing the command. Check options."<<endl;
         break;

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

//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <iostream>
#include <sstream>
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

void showAvailableCommands()
{
   ossimUtilityFactoryBase* factory = ossimUtilityRegistry::instance();
   map<string, string> capabilities;
   factory->getCapabilities(capabilities);
   map<string, string>::iterator iter = capabilities.begin();
   cout<<"\nAvailable commands:\n"<<endl;
   for (;iter != capabilities.end(); ++iter)
      cout<<"  "<<iter->first<<" -- "<<iter->second<<endl;
   cout<<endl;

}

void usage(char* appName)
{
   cout << "\nUsages: "<<endl;
   cout << "    "<<appName<<" <command> [command options and parameters]"<<endl;
   cout << "    "<<appName<<" --spec <command-spec-file>"<<endl;
   cout << "    "<<appName<<" --version"<<endl;
   cout << "    "<<appName<<" help <command> -- To get help on specific command."<<endl;

   showAvailableCommands();
}

bool runCommand(ossimArgumentParser& ap)
{
   bool status_ok = true;
   ossimString command = ap[1];
   ap.remove(1);
   ossimUtilityFactoryBase* factory = ossimUtilityRegistry::instance();
   ossimRefPtr<ossimUtility> utility = factory->createUtility(command);

   if (!utility.valid())
   {
      cout << "\nDid not understand command <"<<command<<">"<<endl;
      showAvailableCommands();
      status_ok = false;
   }
   else if (!utility->initialize(ap))
   {
      cout << "\nCould not execute command <"<<command<<"> with arguments and options "
            "provided."<<endl;
      status_ok = false;
   }
   else if (!utility->execute())
   {
      cout << "\nAn error was encountered executing the command. Check options."<<endl;
      status_ok = false;
   }

   return status_ok;
}

int main(int argc, char *argv[])
{
   ossimArgumentParser ap (&argc, argv);
   ap.getApplicationUsage()->setApplicationName(argv[0]);

   bool status_ok = true;
   try
   {
      // Initialize ossim stuff, factories, plugin, etc.
      ossimInit::instance()->initialize(ap);

      do
      {
         if (argc == 1)
         {
            // Blank command line:
            usage(argv[0]);
            break;
         }

         // Spec file provided containing command's arguments and options?
         ossimString argv1 = argv[1];
         if (( argv1 == "--spec") && (argc > 2))
         {
            // Command line provided in spec file:
            ifstream ifs (argv[2]);
            if (ifs.fail())
            {
               cout<<"\nCould not open the spec file at <"<<argv[2]<<">\n"<<endl;
               status_ok = false;
               break;
            }
            ossimString spec;
            ifs >> spec;
            ifs.close();
            ossimArgumentParser new_ap(spec);
            status_ok = runCommand(new_ap);
            break;
         }

         // Need help with app or specific command?
         if (argv1.contains("help"))
         {
            // If no command was specified, then general usage help shown:
            if (argc == 2)
            {
               // Blank command line:
               usage(argv[0]);
               break;
            }
            else
            {
               // rearrange command line for individual utility help:
               ostringstream cmdline;
               cmdline<<argv[0]<<" "<<argv[2]<<" --help";
               ossimArgumentParser new_ap(cmdline.str());
               status_ok = runCommand(new_ap);
               break;
            }
         }

         if (argv1[0] == '-')
         {
            // Treat as info call:
            ap.insert(1, "info");
            status_ok = runCommand(ap);
            break;
         }

         // Conventional command line, just remove this app's name:
         status_ok = runCommand(ap);

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

   if (status_ok)
      exit(0);
   exit(1);
}

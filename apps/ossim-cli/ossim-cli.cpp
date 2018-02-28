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
#include <ossim/util/ossimToolRegistry.h>
#include <ossim/base/ossimException.h>

#define CINFO  ossimNotify(ossimNotifyLevel_INFO)
#define CWARN  ossimNotify(ossimNotifyLevel_WARN)
#define CFATAL ossimNotify(ossimNotifyLevel_FATAL)

void showAvailableCommands()
{
   ossimToolFactoryBase* factory = ossimToolRegistry::instance();
   map<string, string> capabilities;
   factory->getCapabilities(capabilities);
   map<string, string>::iterator iter = capabilities.begin();
   CINFO<<"\nAvailable commands:\n"<<endl;
   for (;iter != capabilities.end(); ++iter)
      CINFO<<"  "<<iter->first<<" -- "<<iter->second<<endl;
   CINFO<<endl;

}

void usage(char* appName)
{
   CINFO << "\nUsages: \n"
         << "    "<<appName<<" <command> [command options and parameters]\n"
         << "    "<<appName<<" --spec <command-spec-file>\n"
         << "    "<<appName<<" --version\n"
         << "    "<<appName<<" help <command> -- To get help on specific command."<<endl;

   showAvailableCommands();
}

bool runCommand(ossimArgumentParser& ap)
{
   ossimString command = ap[1];
   ap.remove(1);
   ossimToolFactoryBase* factory = ossimToolRegistry::instance();
   ossimRefPtr<ossimTool> utility = factory->createTool(command);

   if (!utility.valid())
   {
      CWARN << "\nDid not understand command <"<<command<<">"<<endl;
      showAvailableCommands();
      return false;
   }

   ossimTimer* timer = ossimTimer::instance();
   timer->setStartTick();

   if (!utility->initialize(ap))
   {
      CWARN << "\nCould not execute command <"<<command<<"> with arguments and options "
            "provided."<<endl;
      return false;
   }

   if (utility->helpRequested())
      return true;

   if (!utility->execute())
   {
      CWARN << "\nAn error was encountered executing the command. Check options."<<endl;
      return false;
   }

   //CINFO << "\nElapsed time after initialization: "<<timer->time_s()<<" s.\n"<<endl;
   return true;
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
               CWARN<<"\nCould not open the spec file at <"<<argv[2]<<">\n"<<endl;
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

         if (argv1.string()[0] == '-')
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
   catch  (const exception& e)
   {
      CFATAL<<e.what()<<endl;
      exit(1);
   }

   if (status_ok)
      exit(0);
   exit(1);
}

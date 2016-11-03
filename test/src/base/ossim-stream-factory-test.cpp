//---
//
// License: MIT
//
// File: ossim-stream-factory-test.cpp
//
// Description: Test app for ossim::StreamFactoryRegistry and associated
// factories.
//
// $Id$
//---

// ossim includes:  These are here just to save time/typing...
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/init/ossimInit.h>

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);

   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " [options]";
   cout << "usageString: " << usageString << endl;
   
   // if (usageString != "ossim-info")
   //    usageString += " info";
   // usageString += " [options] <file>";
   au->setCommandLineUsage(usageString);
   au->addCommandLineOption("--in",  "<file> open ossim::istream");
   au->addCommandLineOption("--out", "<file> open ossim::ostream");
   au->addCommandLineOption("--io",  "<file> open ossim::iostream");
   
   ossimInit::instance()->initialize(ap);

   if ( ap.argc() > 1)
   {
      try
      {
         std::string ts1;
         ossimArgumentParser::ossimParameter sp1(ts1);
         
         if( ap.read("--in", sp1) )
         {
            ossimString connnectionString(ts1);
            shared_ptr<ossim::istream> str = ossim::StreamFactoryRegistry::instance()->
               createIstream( connnectionString, std::ios_base::in|std::ios_base::binary );

            if ( str )
            {
               ossimNotify(ossimNotifyLevel_INFO)
                  << "Opened ossim::istream for " << connnectionString << endl;
            }
            else
            {
               ossimNotify(ossimNotifyLevel_INFO)
                  << "Could not open ossim::istream for " << connnectionString << endl;
            }
         }

         if( ap.read("--out", sp1) )
         {
            ossimString connnectionString(ts1);
            shared_ptr<ossim::ostream> str = ossim::StreamFactoryRegistry::instance()->
               createOstream( connnectionString, std::ios_base::in|std::ios_base::binary );

            if ( str )
            {
               ossimNotify(ossimNotifyLevel_INFO)
                  << "Opened ossim::ostream for " << connnectionString << endl;
            }
            else
            {
               ossimNotify(ossimNotifyLevel_INFO)
                  << "Could not open ossim::ostream for " << connnectionString << endl;
            }
         }

         if( ap.read("--io", sp1) )
         {
            ossimString connnectionString(ts1);
            shared_ptr<ossim::iostream> str = ossim::StreamFactoryRegistry::instance()->
               createIOstream( connnectionString, std::ios_base::in|std::ios_base::binary );
            
            if ( str )
            {
               ossimNotify(ossimNotifyLevel_INFO)
                  << "Opened ossim::iostream for " << connnectionString << endl;
            }
            else
            {
               ossimNotify(ossimNotifyLevel_INFO)
                  << "Could not open iossim::ostream for " << connnectionString << endl;
            }
         }
      }
      catch(const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         returnCode = 1;
      }
      catch( ... )
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossim-foo caught unhandled exception!" << std::endl;
         returnCode = 1;
      }
   }
   else
   {
      ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
   }
   return returnCode;
}

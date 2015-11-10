//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimUtility.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/init/ossimInit.h>
#include <iostream>

using namespace std;

ossimUtility::ossimUtility()
{
}

ossimUtility::~ossimUtility()
{
}

void ossimUtility::setUsage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimInit::instance()->addOptions(ap);

   std::string appName = ap.getApplicationName();
   ossimApplicationUsage* au = ap.getApplicationUsage();
   au->setApplicationName( ossimString( appName ) );

   au->addCommandLineOption(
         "--write-api <filename>",
         "Writes a JSON API specification to the specified filename.");
   au->addCommandLineOption(
         "--write-template <filename>",
         "Writes a template keyword-list to the specified filename.");
}

bool ossimUtility::initialize(ossimArgumentParser& ap)
{
   if ( (ap.argc() == 1) || ap.read("-h") || ap.read("--help") )
   {
      // Write usage.
      setUsage(ap);
      ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      return false;
   }

   std::string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);

   if ( ap.read("--write-api", sp1))
   {
      ofstream ofs ( ts1.c_str() );
      ossimString json_str;
      getAPI(json_str);
      ofs << json_str <<endl;
      return false;
   }

   if ( ap.read("--write-template", sp1))
   {
      ofstream ofs ( ts1.c_str() );
      ossimString kwl_str;
      getKwlTemplate(kwl_str);
      ofs << kwl_str <<endl;
      return false;
   }

   return true;
}

void ossimUtility::getKwlTemplate(ossimString& kwl)
{
   ossimFilename kwl_path (ossimPreferences::instance()->findPreference("ossim_share_directory"));
   kwl_path += "/ossim/util/" + getClassName() + ".kwl";
   readFile(kwl_path, kwl);
}

void ossimUtility::getAPI(ossimString& json) const
{
   ossimFilename json_path (ossimPreferences::instance()->findPreference("ossim_share_directory"));
   json_path += "/ossim/util/" + getClassName() + ".json";
   readFile(json_path, json);
}

bool ossimUtility::readFile(const ossimFilename& filename, ossimString& contents) const
{
   contents.clear();

   std::ifstream is(filename.chars());
   if (!is)
      return false;

   // get length of file:
   is.seekg (0, is.end);
   int length = is.tellg();
   is.seekg (0, is.beg);

   // allocate memory:
   char *buffer = new char [length];

   // read data as a block:
   is.read (buffer,length);
   is.close();

   contents.append(buffer);
   delete [] buffer;

   return true;
}


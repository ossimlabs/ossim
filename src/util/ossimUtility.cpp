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

void ossimUtility::initialize(ossimArgumentParser& ap)
{
   if ( (ap.argc() == 1) || ap.read("-h") || ap.read("--help") )
   {
      // Write usage.
      setUsage(ap);
      ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      return;
   }

   std::string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);

   if ( ap.read("--write-api", sp1))
   {
      ofstream ofs ( ts1.c_str() );
      ossimString json_str;
      getAPI(json_str);
      ofs << json_str <<endl;
      return;
   }

   if ( ap.read("--write-template", sp1))
   {
      ofstream ofs ( ts1.c_str() );
      ossimString kwl_str;
      getKwlTemplate(kwl_str);
      ofs << kwl_str <<endl;
      return;
   }
}

void ossimUtility::getKwlTemplate(ossimString& kwl)
{
   ossimFilename kwl_path (ossimPreferences::instance()->findPreference("ossim_share_directory"));
   kwl_path += "/ossim/util/" + getClassName() + ".kwl";
   readTextFile(kwl_path, kwl);
}

void ossimUtility::getAPI(string& json) const
{
   ossimFilename json_path (ossimPreferences::instance()->findPreference("ossim_share_directory"));
   json_path += "/ossim/util/" + getClassName() + ".json";
   readTextFile(json_path, json);
}

string ossimUtility::getAPI() const
{
   string result;
   getAPI(result);
   return result;
}

bool ossimUtility::readTextFile(const ossimFilename& filename, string& contents) const
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

void ossimUtility::getBuildDate(std::string& s) const
{
#ifdef OSSIM_BUILD_DATE
   s = OSSIM_BUILD_DATE;
#else
   s = "unknown";
#endif
}

void ossimUtility::getRevision(std::string& s) const
{
#ifdef OSSIM_REVISION
   s = OSSIM_REVISION;
#else
   s = "unknown";
#endif
}

void ossimUtility::getVersion(std::string& s) const
{
#ifdef OSSIM_VERSION
   s = OSSIM_VERSION;
#else
   s = "unknown";
#endif
}


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

using namespace std;

ossimUtility::ossimUtility()
   : m_kwl(),
     m_consoleStream (&cout)
{
}

ossimUtility::~ossimUtility()
{
}

void ossimUtility::clear()
{
}

void ossimUtility::abort()
{
}

ossimString ossimUtility::getClassName() const
{
   return "ossimUtility";
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
      return true;
   }

   if ( ap.read("--write-template", sp1))
   {
      ofstream ofs ( ts1.c_str() );
      ossimKeywordlist kwl;
      getKwlTemplate(kwl);
      ofs << kwl <<endl;
      return true;
   }

   return true;
}

void ossimUtility::initialize(const ossimKeywordlist& kwl)
{
   m_kwl = kwl;
}

void ossimUtility::getKwlTemplate(ossimKeywordlist& kwl)
{
   ossimFilename share_dir = ossimPreferences::instance()->
      preferencesKWL().findKey( std::string( "ossim_share_directory" ) );
   ossimFilename classTemplate = "util/" + getClassName() + ".kwl";
   ossimFilename kwl_path = share_dir.dirCat( classTemplate );

   if (!kwl.addFile(kwl_path))
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"ossimUtility::getKwlTemplate() -- Could not find <"
            <<kwl_path<<">. Ignoring"<<endl;
   }
}

void ossimUtility::getAPI(string& json) const
{
   ossimFilename share_dir = ossimPreferences::instance()->
      preferencesKWL().findKey( std::string( "ossim_share_directory" ) );
   ossimFilename classJson = "util/" + getClassName() + ".json";
   ossimFilename json_path = share_dir.dirCat( classJson );

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
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"ossimUtility::readTextFile() -- Could not find <"
            <<filename<<">. Ignoring."<<endl;
      return false;
   }

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


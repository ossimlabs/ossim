//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/init/ossimInit.h>
#include <ossim/util/ossimTool.h>

using namespace std;

ossimTool::ossimTool()
   : m_kwl(),
     m_consoleStream (&cout),
     m_helpRequested (false)
{
}

ossimTool::~ossimTool()
{
}

void ossimTool::clear()
{
}

void ossimTool::abort()
{
}

ossimString ossimTool::getClassName() const
{
   return "ossimTool";
}

void ossimTool::setUsage(ossimArgumentParser& ap)
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

bool ossimTool::initialize(ossimArgumentParser& ap)
{
   m_helpRequested = false;
   if (ap.read("-h") || ap.read("--help") )
   {
      // Write usage.
      setUsage(ap);
      ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      m_helpRequested = true;
      return true;
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

void ossimTool::initialize(const ossimKeywordlist& kwl)
{
   m_helpRequested = false;
   m_kwl = kwl;
}

void ossimTool::initialize(const std::string& request)
{
   m_helpRequested = false;
}

void ossimTool::getKwlTemplate(ossimKeywordlist& kwl)
{
   ossimFilename share_dir = ossimPreferences::instance()->
      preferencesKWL().findKey( std::string( "ossim_share_directory" ) );
   ossimFilename classTemplate = "util/" + getClassName() + ".kwl";
   ossimFilename kwl_path = share_dir.dirCat( classTemplate );

   if (!kwl.addFile(kwl_path))
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"ossimTool::getKwlTemplate() -- Could not find <"
            <<kwl_path<<">. Ignoring"<<endl;
   }
}

void ossimTool::getAPI(string& json) const
{
   ossimFilename share_dir = ossimPreferences::instance()->
      preferencesKWL().findKey( std::string( "ossim_share_directory" ) );
   ossimFilename classJson = "util/" + getClassName() + ".json";
   ossimFilename json_path = share_dir.dirCat( classJson );

   readTextFile(json_path, json);
}

string ossimTool::getAPI() const
{
   string result;
   getAPI(result);
   return result;
}

bool ossimTool::readTextFile(const ossimFilename& filename, string& contents) const
{
   contents.clear();

   std::ifstream is(filename.chars());
   if (!is)
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"ossimTool::readTextFile() -- Could not find <"
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

void ossimTool::getBuildDate(std::string& s) const
{
#ifdef OSSIM_BUILD_DATE
   s = OSSIM_BUILD_DATE;
#else
   s = "unknown";
#endif
}

void ossimTool::getRevision(std::string& s) const
{
#ifdef OSSIM_REVISION
   s = OSSIM_REVISION;
#else
   s = "unknown";
#endif
}

void ossimTool::getVersion(std::string& s) const
{
#ifdef OSSIM_VERSION
   s = OSSIM_VERSION;
#else
   s = "unknown";
#endif
}


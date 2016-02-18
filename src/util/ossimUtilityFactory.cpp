//**************************************************************************************************
//
//     OSSIM Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimUtilityFactory.h>
#include <ossim/util/ossimHillshadeUtil.h>
#include <ossim/util/ossimHlzUtil.h>
#include <ossim/util/ossimViewshedUtil.h>
#include <ossim/util/ossimSlopeUtil.h>
#include <ossim/util/ossimUtilityRegistry.h>

ossimUtilityFactory* ossimUtilityFactory::s_Instance = 0;

ossimUtilityFactory* ossimUtilityFactory::instance()
{
   if (!s_Instance)
      s_Instance = new ossimUtilityFactory;
   return s_Instance;
}

ossimUtilityFactory::ossimUtilityFactory()
{
}

ossimUtilityFactory::~ossimUtilityFactory()
{
   ossimUtilityRegistry::instance()->unregisterFactory(this);
}

ossimUtility* ossimUtilityFactory::createUtility(const std::string& argName) const
{
   ossimString utilName (argName);
   utilName.downcase();

   if ((utilName == "hillshade") || (utilName == "ossimHillshadeUtil"))
      return new ossimHillshadeUtil;

   if ((utilName == "viewshed") || (utilName == "ossimViewshedUtil"))
      return new ossimViewshedUtil;

   if ((utilName == "slope") || (utilName == "ossimSlopeUtil"))
      return new ossimSlopeUtil;

   if ((utilName == "hlz") || (utilName == "ossimHlzUtil"))
      return new ossimHlzUtil;

   return 0;
}

void ossimUtilityFactory::getCapabilities(std::map<std::string, std::string>& capabilities) const
{
   capabilities.insert(pair<string, string>("hillshade", ossimHillshadeUtil::DESCRIPTION));
   capabilities.insert(pair<string, string>("viewshed", ossimViewshedUtil::DESCRIPTION));
   capabilities.insert(pair<string, string>("slope", ossimSlopeUtil::DESCRIPTION));
   capabilities.insert(pair<string, string>("hlz", ossimHlzUtil::DESCRIPTION));
}

std::map<std::string, std::string> ossimUtilityFactory::getCapabilities() const
{
   std::map<std::string, std::string> result;
   getCapabilities(result);
   return result;
}

void ossimUtilityFactory::getTypeNameList(vector<ossimString>& typeList) const
{
   typeList.push_back("ossimHillshadeUtil");
   typeList.push_back("ossimViewshedUtil");
   typeList.push_back("ossimSlopeUtil");
   typeList.push_back("ossimHLZUtil");
}


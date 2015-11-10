//**************************************************************************************************
//
//     OSSIM Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimUtilityFactory.h>
#include <ossim/util/ossimUtilityManager.h>
#include <ossim/util/ossimHillshadeUtil.h>
#include <ossim/util/ossimViewshedUtil.h>
#include <ossim/util/ossimHLZUtil.h>
#include <ossim/util/ossimSlopeUtil.h>

ossimUtilityFactory* ossimUtilityFactory::s_Instance = 0;

ossimUtilityFactory* ossimUtilityFactory::instance()
{
   if (!s_Instance)
      s_Instance = new ossimUtilityFactory;
   return s_Instance;
}

ossimUtilityFactory::ossimUtilityFactory()
{
   // Register this factory:
   ossimUtilityManager::instance()->registerFactory(this, true);
}

ossimUtilityFactory::~ossimUtilityFactory()
{
   ossimUtilityManager::instance()->unregisterFactory(this);
}

ossimUtility* ossimUtilityFactory::createUtility(const ossimString& argName) const
{
   ossimString utilName (argName);
   utilName.downcase();

   if ((utilName == "hillshade") || (utilName == "ossimHillshadeUtil"))
      return new ossimHillshadeUtil;

   if ((utilName == "viewshed") || (utilName == "ossimViewshedUtil"))
      return new ossimViewshedUtil;

   if ((utilName == "slope") || (utilName == "ossimSlopeUtil"))
      return new ossimSlopeUtil;

   if ((utilName == "hlz") || (utilName == "ossimHLZUtil"))
      return new ossimHLZUtil;

   return 0;
}

void ossimUtilityFactory::getCapabilities(std::map<std::string, std::string>& capabilities) const
{
   capabilities.insert(pair<string, string>("hillshade", ossimHillshadeUtil::DESCRIPTION));
   capabilities.insert(pair<string, string>("viewshed", ossimViewshedUtil::DESCRIPTION));
   capabilities.insert(pair<string, string>("slope", ossimSlopeUtil::DESCRIPTION));
   capabilities.insert(pair<string, string>("hlz", ossimHLZUtil::DESCRIPTION));
}

void ossimUtilityFactory::getTypeNameList(vector<ossimString>& typeList) const
{
   typeList.push_back("ossimHillshadeUtil");
   typeList.push_back("ossimViewshedUtil");
   typeList.push_back("ossimSlopeUtil");
   typeList.push_back("ossimHLZUtil");
}


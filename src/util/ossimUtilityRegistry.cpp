//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimUtilityRegistry.h>
#include <ossim/util/ossimUtilityFactory.h>

using namespace std;

ossimUtilityRegistry* ossimUtilityRegistry::s_instance = 0;

ossimUtilityRegistry* ossimUtilityRegistry::instance()
{
   if (!s_instance)
      s_instance = new ossimUtilityRegistry;
   return s_instance;
}

ossimUtilityRegistry::ossimUtilityRegistry()
{
   registerFactory(ossimUtilityFactory::instance(), true);
}

ossimUtilityRegistry::~ossimUtilityRegistry()
{

}

bool ossimUtilityRegistry::initialize()
{
   return false;
}

void ossimUtilityRegistry::getCapabilities(map<string, string>& capabilities) const
{
   capabilities.clear();
   ossimString name, descr;

   // Loop over all factories and get capabilities which is list of (name, descr) pairs for all
   // registered operations:
   vector<ossimUtilityFactoryBase*>::const_iterator iter = m_factoryList.begin();
   while (iter != m_factoryList.end())
   {
      (*iter)->getCapabilities(capabilities);
      ++iter;
   }
}

std::map<std::string, std::string> ossimUtilityRegistry::getCapabilities() const
{
   std::map<std::string, std::string> result;
   getCapabilities(result);
   return result;
}

ossimUtility* ossimUtilityRegistry::createUtility(const std::string& argName) const
{
   ossimUtility* result = 0;
   vector<ossimUtilityFactoryBase*>::const_iterator iter = m_factoryList.begin();
   while ((iter != m_factoryList.end()) && (!result))
   {
      result = (*iter)->createUtility(argName);
      ++iter;
   }
   return result;
}

void ossimUtilityRegistry::getTypeNameList(vector<ossimString>& typeList) const
{
   typeList.clear();
   vector<ossimUtilityFactoryBase*>::const_iterator iter = m_factoryList.begin();
   while (iter != m_factoryList.end())
   {
      (*iter)->getTypeNameList(typeList);
      ++iter;
   }
}


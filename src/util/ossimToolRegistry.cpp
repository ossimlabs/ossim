//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimToolFactory.h>
#include <ossim/util/ossimToolRegistry.h>

using namespace std;

ossimToolRegistry* ossimToolRegistry::s_instance = 0;

ossimToolRegistry* ossimToolRegistry::instance()
{
   if (!s_instance)
      s_instance = new ossimToolRegistry;
   return s_instance;
}

ossimToolRegistry::ossimToolRegistry()
{
   registerFactory(ossimToolFactory::instance(), true);
}

ossimToolRegistry::~ossimToolRegistry()
{

}

bool ossimToolRegistry::initialize()
{
   return false;
}

void ossimToolRegistry::getCapabilities(map<string, string>& capabilities) const
{
   capabilities.clear();
   ossimString name, descr;

   // Loop over all factories and get capabilities which is list of (name, descr) pairs for all
   // registered operations:
   vector<ossimToolFactoryBase*>::const_iterator iter = m_factoryList.begin();
   while (iter != m_factoryList.end())
   {
      (*iter)->getCapabilities(capabilities);
      ++iter;
   }
}

std::map<std::string, std::string> ossimToolRegistry::getCapabilities() const
{
   std::map<std::string, std::string> result;
   getCapabilities(result);
   return result;
}

ossimTool* ossimToolRegistry::createTool(const std::string& argName) const
{
   ossimTool* result = 0;
   vector<ossimToolFactoryBase*>::const_iterator iter = m_factoryList.begin();
   while ((iter != m_factoryList.end()) && (!result))
   {
      result = (*iter)->createTool(argName);
      ++iter;
   }
   return result;
}

void ossimToolRegistry::getTypeNameList(vector<ossimString>& typeList) const
{
   typeList.clear();
   vector<ossimToolFactoryBase*>::const_iterator iter = m_factoryList.begin();
   while (iter != m_factoryList.end())
   {
      (*iter)->getTypeNameList(typeList);
      ++iter;
   }
}


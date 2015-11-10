//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimUtilityManager.h>

using namespace std;

ossimUtilityManager* ossimUtilityManager::s_instance = 0;

ossimUtilityManager* ossimUtilityManager::instance()
{
   if (!s_instance)
      s_instance = new ossimUtilityManager;
   return s_instance;
}

ossimUtilityManager::ossimUtilityManager()
{

}

ossimUtilityManager::~ossimUtilityManager()
{

}

bool ossimUtilityManager::initialize()
{
   return false;
}

void ossimUtilityManager::getCapabilities(map<string, string>& capabilities) const
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

ossimUtility* ossimUtilityManager::createUtility(const ossimString& argName) const
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

void ossimUtilityManager::getTypeNameList(vector<ossimString>& typeList) const
{
   typeList.clear();
   vector<ossimUtilityFactoryBase*>::const_iterator iter = m_factoryList.begin();
   while (iter != m_factoryList.end())
   {
      (*iter)->getTypeNameList(typeList);
      ++iter;
   }
}


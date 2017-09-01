//*******************************************************************
//
// LICENSE: See top level LICENSE.txt file.
//
// Author: 
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfDesFactoryRegistry.cpp 22875 2014-08-27 13:52:03Z dburken $

#include <ossim/support_data/ossimNitfDesFactoryRegistry.h>
#include <ossim/support_data/ossimNitfRegisteredDesFactory.h>

#include <algorithm> /* for std::find */

ossimNitfDesFactoryRegistry::ossimNitfDesFactoryRegistry()
{
   initializeDefaults();
}

ossimNitfDesFactoryRegistry::~ossimNitfDesFactoryRegistry()
{
}

void ossimNitfDesFactoryRegistry::registerFactory(ossimNitfDesFactory* aFactory)
{
   if(aFactory&&!exists(aFactory))
   {
      theFactoryList.push_back(aFactory);
   }
}

void ossimNitfDesFactoryRegistry::unregisterFactory
(ossimNitfDesFactory* aFactory)
{
   std::vector<ossimNitfDesFactory*>::iterator iter =
      std::find(theFactoryList.begin(),
                theFactoryList.end(),
                aFactory);
   if(iter != theFactoryList.end())
   {
      theFactoryList.erase(iter);
   }
}

ossimNitfDesFactoryRegistry* ossimNitfDesFactoryRegistry::instance()
{
   static ossimNitfDesFactoryRegistry inst;
   return &inst;
}

ossimRefPtr<ossimNitfRegisteredDes> ossimNitfDesFactoryRegistry::create(const ossimString &desName)const
{
   std::vector<ossimNitfDesFactory*>::const_iterator factory;
   ossimRefPtr<ossimNitfRegisteredDes> result = 0;

   factory = theFactoryList.begin();
   while(factory != theFactoryList.end())
   {
      result = (*factory)->create(desName);

      if(result.valid())
      {
         return result;
      }
      ++factory;
   }
   
   return result;
   //ossimRefPtr<ossimNitfRegisteredDes> des = new ossimNitfUnknownDes;
   //return des;
}

void ossimNitfDesFactoryRegistry::initializeDefaults()
{
   theFactoryList.push_back(ossimNitfRegisteredDesFactory::instance());
}

bool ossimNitfDesFactoryRegistry::exists(ossimNitfDesFactory* factory)const
{
   std::vector<ossimNitfDesFactory*>::const_iterator iter = std::find(theFactoryList.begin(),
                                                                      theFactoryList.end(),
                                                                      factory);

   return (iter != theFactoryList.end());
   
}

ossimNitfDesFactoryRegistry::ossimNitfDesFactoryRegistry(
   const ossimNitfDesFactoryRegistry&)
{
}

const ossimNitfDesFactoryRegistry& ossimNitfDesFactoryRegistry::operator=(
   const ossimNitfDesFactoryRegistry&)
{
   return *this;
}

//*******************************************************************
//
// License: MIT
//
// Author: Garrett Potts
//
//*******************************************************************
// $Id$

#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/base/ossimStreamFactory.h>
#include <ossim/base/ossimIoStream.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/BlockIStream.h>

#include <fstream>
#include <algorithm>
#include <ossim/base/ossimTrace.h>


ossim::StreamFactoryRegistry* ossim::StreamFactoryRegistry::m_instance = 0;
static const ossimString ISTREAM_BUFFER_KW = "ossim.stream.factory.registry.istream.buffer";
static ossimTrace traceDebug("ossimStreamFactoryRegistry:debug");
static std::mutex m_instanceMutex;
ossim::StreamFactoryRegistry::StreamFactoryRegistry()
{
}


ossim::StreamFactoryRegistry::~StreamFactoryRegistry()
{
}

ossim::StreamFactoryRegistry* ossim::StreamFactoryRegistry::instance()
{
   // because of the loadPreferences for this factory and the fact that
   // create stream could be recursive the lock and unlock are
   // a little different here.  We will unlock as quickly as we can
   // After the unlock then call load preferences
   m_instanceMutex.lock();
   if(!m_instance)
   {
      m_instance = new ossim::StreamFactoryRegistry();
      m_instance->registerFactory(ossim::StreamFactory::instance());
      m_instanceMutex.unlock();
      m_instance->loadPreferences();
   }
   else
   {
      m_instanceMutex.unlock();
   }

   return m_instance;
}

void ossim::StreamFactoryRegistry::loadPreferences()
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_WARN)
      << "ossim::StreamFactoryRegistry::loadPreferences: ....... entered!\n";
   }
   std::vector<ossimString> sortedList;
   ossimPreferences::instance()->preferencesKWL()
         .getSortedList(sortedList, ISTREAM_BUFFER_KW);
   if(sortedList.size())
   {
      m_bufferInfoList.resize(sortedList.size());
   }
   else
   {
      m_bufferInfoList.clear();
   }
   ossim_uint32 idx=0;
   for(std::vector<ossimString>::const_iterator iter = sortedList.begin();
       iter!=sortedList.end();
       ++iter,++idx)
   {
      ossimString prefix = *iter;
      ossimString bufferIStreamEnabled        = ossimPreferences::instance()->findPreference(prefix+".enabled");
      ossimString bufferIStreamBlockEnabled   = ossimPreferences::instance()->findPreference(prefix+".enableBlocked");
      ossimString bufferIStreamIncludePattern = ossimPreferences::instance()->findPreference(prefix+".includePattern");
      ossimString bufferIStreamSize           = ossimPreferences::instance()->findPreference(prefix+".size");

      if(!bufferIStreamSize.empty())
      {
         m_bufferInfoList[idx].m_size = bufferIStreamSize.toUInt64();
      }
      if(!bufferIStreamEnabled.empty())
      {
         m_bufferInfoList[idx].m_enabled = bufferIStreamEnabled.toBool();
      }
      if(!bufferIStreamBlockEnabled.empty())
      {
         m_bufferInfoList[idx].m_enableBlocked = bufferIStreamBlockEnabled.toBool();
      }
      if(!bufferIStreamIncludePattern.empty())
      {
         m_bufferInfoList[idx].m_pattern = bufferIStreamIncludePattern;
      }
      if(traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
         << "ossim::StreamFactoryRegistry adding BufferInfo: \n"
         << "enabled:       " << ossimString::toString(m_bufferInfoList[idx].m_enabled)<< "\n"
         << "enableBlocked: " << ossimString::toString(m_bufferInfoList[idx].m_enableBlocked)<< "\n"
         << "size:          " << m_bufferInfoList[idx].m_size << "\n"
         << "pattern:       " << m_bufferInfoList[idx].m_pattern << "\n";
      }

   }
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_WARN)
      << "ossim::StreamFactoryRegistry::loadPreferences: ....... leaving!\n";
   }
}

bool ossim::StreamFactoryRegistry::getBufferInfo(BufferInfo& bufferInfo, 
                                              const ossimString& connectionString)const
{
   bool result = false;

   for(std::vector<BufferInfo>::const_iterator iter = m_bufferInfoList.begin();
      iter != m_bufferInfoList.end(); 
      ++iter)
   {
      if(iter->m_enabled)
      {
         m_patternMatcher.compile(iter->m_pattern);
         if(m_patternMatcher.is_valid())
         {
            if(m_patternMatcher.find(connectionString.c_str()))
            {
               bufferInfo = *iter;
               result = true;
               break;
            }            
         }
      }
   }

   return result;
}


std::shared_ptr<ossim::istream> ossim::StreamFactoryRegistry::createIstream(
   const std::string& connectionString,
   const ossimKeywordlist& options,
   std::ios_base::openmode openMode) const
{
   std::shared_ptr<ossim::istream> result(0);
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_WARN)
      << "ossim::StreamFactoryRegistry::createIstream: ....... entered: "<< connectionString << "\n";
   }

   ossim_uint32 i = 0;
   for(i = 0; (i < m_factoryList.size())&&(!result); ++i)
   {
      result = m_factoryList[i]->createIstream(connectionString, options, openMode);

      if(result)
      {
         BufferInfo bufferInfo;
         if(getBufferInfo(bufferInfo, connectionString))
         {
            if(bufferInfo.m_enableBlocked)
            {
               result = std::make_shared<ossim::BlockIStream>(result, bufferInfo.m_size);
            }
            else
            {
               result = std::make_shared<ossimBufferedInputStream>(result, bufferInfo.m_size);
            }
         }
      }
   }

   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_WARN)
      << "ossim::StreamFactoryRegistry::createIstream: ....... leaving!\n";
   }

   return result;
}
      
std::shared_ptr<ossim::ostream> ossim::StreamFactoryRegistry::createOstream(
   const std::string& /*connectionString*/,
   const ossimKeywordlist& /* options */,
   std::ios_base::openmode /*openMode*/) const
{
   std::shared_ptr<ossim::ostream> result(0);
   return result;
}

std::shared_ptr<ossim::iostream> ossim::StreamFactoryRegistry::createIOstream(
   const std::string& /*connectionString*/,
   const ossimKeywordlist& /* options */,
   std::ios_base::openmode /*openMode*/) const
{
   std::shared_ptr<ossim::iostream> result(0);
   return result;
}

bool ossim::StreamFactoryRegistry::exists(const std::string& connectionString) const
{
   bool continueFlag = true;
   return exists( connectionString, continueFlag );
}

bool ossim::StreamFactoryRegistry::exists(const std::string& connectionString,
                                          bool& continueFlag) const
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_WARN)
      << "ossim::StreamFactoryRegistry::exists: ....... entered: "<< connectionString << "\n";
   }
   bool result = false;
   for(auto factory:m_factoryList)
   {

      result = factory->exists( connectionString, continueFlag );
      if ( ( result == true ) || (continueFlag == false ) ) break;
   }

   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_WARN)
      << "ossim::StreamFactoryRegistry::createIstream: ....... leaving!\n";
   }
   return result;
}

void ossim::StreamFactoryRegistry::registerFactory(ossim::StreamFactoryBase* factory)
{
   std::vector<ossim::StreamFactoryBase*>::iterator iter = std::find(
      m_factoryList.begin(), m_factoryList.end(), factory);
   if(iter == m_factoryList.end())
   {
      m_factoryList.push_back(factory);
   }
}

void ossim::StreamFactoryRegistry::unregisterFactory(StreamFactoryBase* factory)
{
   // std::lock_guard<std::mutex> lock(m_factoryListMutex);
   std::vector<ossim::StreamFactoryBase*>::iterator iter = std::find(
      m_factoryList.begin(), m_factoryList.end(), factory);
   if(iter != m_factoryList.end())
   {
      m_factoryList.erase( iter );
   }
}

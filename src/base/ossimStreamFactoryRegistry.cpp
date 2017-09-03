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
#include <ossim/base/ossimBlockIStream.h>

#include <fstream>
#include <algorithm>
#include <ossim/base/ossimTrace.h>

static ossimTrace traceDebug("ossimStreamFactoryRegistry:debug");

ossim::StreamFactoryRegistry* ossim::StreamFactoryRegistry::m_instance = 0;
static const ossimString ISTREAM_BUFFER_KW = "ossim.stream.factory.registry.istream.buffer";
ossim::StreamFactoryRegistry::StreamFactoryRegistry()
{
   loadPreferences();
}


ossim::StreamFactoryRegistry::~StreamFactoryRegistry()
{
}

ossim::StreamFactoryRegistry* ossim::StreamFactoryRegistry::instance()
{
   if(!m_instance)
   {
      m_instance = new ossim::StreamFactoryRegistry();

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
   bool result = false;
   std::vector<ossim::StreamFactoryBase*>::const_iterator i = m_factoryList.begin();
   while ( i != m_factoryList.end() )
   {
      result = (*i)->exists( connectionString, continueFlag );
      if ( ( result == true ) || (continueFlag == false ) ) break;
      ++i;
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
   // OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_factoryListMutex);
   std::vector<ossim::StreamFactoryBase*>::iterator iter = std::find(
      m_factoryList.begin(), m_factoryList.end(), factory);
   if(iter != m_factoryList.end())
   {
      m_factoryList.erase( iter );
   }
}

// Deprecated code:
ossimStreamFactoryRegistry* ossimStreamFactoryRegistry::theInstance = 0;

ossimStreamFactoryRegistry::ossimStreamFactoryRegistry()
{
}

ossimStreamFactoryRegistry::~ossimStreamFactoryRegistry()
{
}

ossimStreamFactoryRegistry* ossimStreamFactoryRegistry::instance()
{
   if(!theInstance)
   {
      theInstance = new ossimStreamFactoryRegistry();
   }
   return theInstance;
}

std::shared_ptr<ossim::ifstream> ossimStreamFactoryRegistry::createIFStream(
   const ossimFilename& file, std::ios_base::openmode openMode) const
{
   std::shared_ptr<ossim::ifstream>result(0);
   
   for(ossim_uint32 idx = 0; ((idx < theFactoryList.size())&&(!result)); ++idx)
   {
      result = theFactoryList[idx]->createIFStream(file, openMode);
   }

   if(!result)
   {
      if(file.exists())
      {
         // there is a bug in gcc < 5.0 and we can't use constructors in the 
         // C++11 build.  Will refactor to do a new ifstream then use open
         //

         result = std::make_shared<ossim::ifstream>();
         result->open(file.c_str(), openMode);
         if(!result->is_open())
         {
            result.reset();
         }
      }
   }
   
   return result; 
   
}

ossimRefPtr<ossimIFStream>
ossimStreamFactoryRegistry::createNewIFStream(
   const ossimFilename& file,
   std::ios_base::openmode openMode) const
{
   ossim_uint32 idx = 0;
   ossimRefPtr<ossimIFStream> result = 0;
   for(idx = 0; ((idx < theFactoryList.size())&&(!result)); ++idx)
   {
      result = theFactoryList[idx]->createNewIFStream(file, openMode);
   }

   if(!result)
   {
      result = new ossimIFStream(file.c_str(),
                                 openMode);
//       result = new std::ifstream(file.c_str(),
//                                  openMode);
   }
   
   return result;
}


void ossimStreamFactoryRegistry::registerFactory(ossimStreamFactoryBase* factory)
{
   std::vector<ossimStreamFactoryBase*>::iterator iter = std::find(theFactoryList.begin(),
                                                                  theFactoryList.end(),
                                                                  factory);
   if(iter == theFactoryList.end())
   {
      theFactoryList.push_back(factory);
   }
}

ossimStreamFactoryRegistry::ossimStreamFactoryRegistry(const ossimStreamFactoryRegistry&)
{}

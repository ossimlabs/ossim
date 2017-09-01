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

ossim::StreamFactoryRegistry* ossim::StreamFactoryRegistry::m_instance = 0;
static const ossimString ISTREAM_BLOCK_KW = "ossim.stream.factory.registry.istream.block";
ossim::StreamFactoryRegistry::StreamFactoryRegistry()
{
   loadPatterns();
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

void ossim::StreamFactoryRegistry::loadPatterns()
{
   const ossimKeywordlist& kwl   = ossimPreferences::instance()->preferencesKWL();
   ossimString regExpression     =  ossimString("^(") + ISTREAM_BLOCK_KW+ "[0-9]+.)";
   std::vector<ossimString> keys = kwl.getSubstringKeyList( regExpression );
   long numberOfBlocks           = (long)keys.size();

   int offset = (int)ossimString(ISTREAM_BLOCK_KW).size();
   int idx = 0;
   std::vector<int> numberList(numberOfBlocks);
   for(idx = 0; idx < (int)numberList.size();++idx)
     {
       ossimString numberStr(keys[idx].begin() + offset,
              keys[idx].end());
       numberList[idx] = numberStr.toInt();
     }
   std::sort(numberList.begin(), numberList.end());
   if(numberList.size())
   {
      m_blockInfoList.resize(numberList.size());
   }
   else
   {
      m_blockInfoList.clear();
   }
   for(idx=0;idx < (int)numberList.size();++idx)
   {
      ossimString newPrefix = ISTREAM_BLOCK_KW;
      newPrefix += ossimString::toString(numberList[idx]);
      newPrefix += ossimString(".");

      ossimString blockIStreamEnabled        = ossimPreferences::instance()->findPreference(newPrefix+"enabled");
      ossimString blockIStreamIncludePattern = ossimPreferences::instance()->findPreference(newPrefix+"includePattern");
      ossimString blockIStreamSize           = ossimPreferences::instance()->findPreference(newPrefix+"size");

      if(!blockIStreamSize.empty())
      {
         m_blockInfoList[idx].m_size = blockIStreamSize.toUInt64();
      }
      if(!blockIStreamEnabled.empty())
      {
         m_blockInfoList[idx].m_enabled = blockIStreamEnabled.toBool();
      }
      if(m_blockInfoList[idx].m_enabled&&!blockIStreamIncludePattern.empty())
      {
         m_blockInfoList[idx].m_pattern.compile(blockIStreamIncludePattern.c_str());
      }
      else
      {
         m_blockInfoList[idx].m_pattern.set_invalid();
      }

   }

}

bool ossim::StreamFactoryRegistry::getBlocked(ossim_uint64& blockSize, 
                                              const ossimString& connectionString)const
{
   bool result = false;

   for(std::vector<BlockInfo>::const_iterator iter = m_blockInfoList.begin();
      iter != m_blockInfoList.end(); ++iter)
   {
      if(iter->m_enabled&&iter->m_pattern.is_valid())
      {
         if(iter->m_pattern.find(connectionString.c_str()))
         {
            blockSize = iter->m_size;
            result = true;
            break;
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
         ossim_uint64 blockSize;
         if(getBlocked(blockSize, connectionString))
         {
            result = std::make_shared<ossim::BlockIStream>(result, blockSize);
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

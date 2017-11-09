//**************************************************************************************************
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Class declaration of ossimFactoryListInterface.
//
//**************************************************************************************************
// $Id$
#ifndef ossimFactoryListBase_HEADER
#define ossimFactoryListBase_HEADER 1
#include <vector>
#include <ossim/base/ossimString.h>
#include <ossim/base/RWLock.h>

namespace ossim
{
  template <class T>
  class FactoryListBase
     {
     public:
        typedef std::vector<T> FactoryListType;
        typedef std::vector<ossimString> TypeNameList;

        FactoryListBase(){}
        
        /**
         * This is for backward compatability and calls registerFactory for simple adds.
         */
        void addFactory(T factory)
        {
           registerFactory(factory);
        }
        
        /**
         * Public access method to determine if a factory is already registered to this
         * list
         */
        bool isFactoryRegistered(T factory)const
        {
           if(!factory) return false;
           ScopeReadLock scopedReadLock(m_rwlock);
           
           return findFactory(factory);
        }
        
        /**
         * Will register a factory to the factory list.  Will append the passed in factory if not
         * already registered to the list.
         */
        void registerFactory(T factory, bool pushToFrontFlag=false)
        {
           if(!factory) return;
           ScopeWriteLock scopedWriteLock(m_rwlock);
           if(!findFactory(factory))
           {
              if (pushToFrontFlag)
              {
                 m_factoryList.insert(m_factoryList.begin(), factory);
              }
              else
              {
                 m_factoryList.push_back(factory);
              }
           }
        }
        /**
         * Will remove the factory from the registry.
         */
        void unregisterFactory(T factory)
        {
          ScopeWriteLock scopedWriteLock(m_rwlock);
          ossim_uint32 idx = 0;
          for(idx = 0; idx < m_factoryList.size(); ++idx)
          {
            if(factory == m_factoryList[idx])
            {
               m_factoryList.erase(m_factoryList.begin() + idx);
               return;
            }
          }
        }
        
        /**
         * Will remove all factories from the registry.
         */
        void unregisterAllFactories()
        {
          ScopeWriteLock scopedWriteLock(m_rwlock);
          m_factoryList.clear();
        }
        
        /**
         * Inserts the factory to the front of the list.
         */
        void registerFactoryToFront(T factory)
        {
          ScopeWriteLock scopedWriteLock(m_rwlock);
          if(!findFactory(factory))
          {
            m_factoryList.insert(m_factoryList.begin(), factory);
          }
        }
        
        /**
         * Will insert the factory before the beforeThisFactory.  If not found
         * it will do a simple append.
         */
        void registerFactoryBefore(T factory, T beforeThisFactory)
        {
          ScopeWriteLock scopedWriteLock(m_rwlock);

           if(!findFactory(factory))
           {
              ossim_uint32 idx = 0;
              for(idx = 0; idx < m_factoryList.size(); ++idx)
              {
                 if(beforeThisFactory == m_factoryList[idx])
                 {
                    m_factoryList.insert(m_factoryList.begin() + idx, factory);
                    return;
                 }
              }
              m_factoryList.push_back(factory);
           }
        }
        
     protected:
        /**
         * Utility to find a factory in the list
         */
        bool findFactory(T factory)const
        {
           if(!factory) return false;
           ossim_uint32 idx = 0;
           for(const auto& testFactory:m_factoryList)
           {
              if(factory == testFactory)
              {
                return true;
              }
           }
           return false;
        }

        mutable ossim::RWLock m_rwlock;
        FactoryListType m_factoryList;
     };
}

#endif

#include <ossim/imaging/ImageHandlerStateCache.h>

std::shared_ptr<ossim::ImageHandlerState> ossim::ImageHandlerStateCache::getState(const ossimString& key)
{
   ossim::ScopeReadLock lock(m_stateCacheMutex);
   std::shared_ptr<ossim::ImageHandlerState> result;
   CacheType::iterator iter = m_cache.find(key);
   if(iter != m_cache.end())
   {
      result = iter->second->m_state;
      std::cout << "Touching node === " << key << "\n";
      touchNode(iter->second);
   }
   return result;
}

std::shared_ptr<const ossim::ImageHandlerState> ossim::ImageHandlerStateCache::getState(const ossimString& key)const
{
   ossim::ScopeReadLock lock(m_stateCacheMutex);
   std::shared_ptr<const ossim::ImageHandlerState> result;
   CacheType::const_iterator iter = m_cache.find(key);
   if(iter != m_cache.end())
   {
      result = iter->second->m_state;
      touchNode(iter->second);
   }
   return result;
}

void ossim::ImageHandlerStateCache::addState(const ossimString& key, 
                                             std::shared_ptr<ossim::ImageHandlerState> state)
{
   ossim::ScopeWriteLock lock(m_stateCacheMutex);
   protectedAddState(key, state);
}

void ossim::ImageHandlerStateCache::protectedAddState(const ossimString& key, 
                                                      std::shared_ptr<ossim::ImageHandlerState> state)
{
   if(m_cache.size() > m_maxStatesToCache)
   {
      std::cout << "shrinkCache\n";
      shrinkCache();
   }
   CacheType::iterator iter = m_cache.find(key);
   if(iter!= m_cache.end())
   {
      std::cout << "UPDATING KEY: " << key << "\n";
      // update the state and LRU
      //
      std::shared_ptr<Node> node = iter->second;
      node->m_state = state;

      touchNode(node);
   }
   else
   {
      std::shared_ptr<Node> nodePtr = std::make_shared<Node>();
      nodePtr->m_cacheId = key;
      nodePtr->m_state = state;
      nodePtr->m_lruId = nextId();
      m_cache.insert(std::make_pair(key, nodePtr));
      {
         ossim::ScopeWriteLock lock(m_lruCacheMutex);
         if(m_lruCache.size() > 0)
         {
            m_lruCache.insert(--m_lruCache.end(), 
                              std::make_pair(nodePtr->m_lruId, 
                                             nodePtr));

         }
         else
         {
            m_lruCache.insert(std::make_pair(nodePtr->m_lruId, 
                                             nodePtr));            
         }
      }
   }
}


void ossim::ImageHandlerStateCache::touchNode(std::shared_ptr<Node> node)const
{
   m_lruCache.erase(node->m_lruId);
   node->m_lruId = nextId();
   {
      ossim::ScopeWriteLock lock(m_lruCacheMutex);
      if(m_lruCache.size() > 0)
      {
         m_lruCache.insert(m_lruCache.end(), std::make_pair(node->m_lruId, node));
      }
      else
      {
         m_lruCache.insert(std::make_pair(node->m_lruId, node));
      }
   }
}

ossim_uint64 ossim::ImageHandlerStateCache::nextId()const
{
   return ++m_currentId;
}

void ossim::ImageHandlerStateCache::setMaxStatesToCache(ossim_uint32 maxStatesToCache)
{
  ossim::ScopeWriteLock lock(m_stateCacheMutex);
  m_maxStatesToCache = maxStatesToCache;
}

void ossim::ImageHandlerStateCache::shrinkCache()
{
   ossim_uint64 shrinkToSize = ossim::round<ossim_uint64,ossim_float64>(m_maxStatesToCache * 0.8);

   if(shrinkToSize < 1)
   {
      m_lruCache.clear();
      m_cache.clear();
   }
   else
   {
      LruType::iterator iter = m_lruCache.begin();
      while((m_cache.size() > shrinkToSize)&&(iter != m_lruCache.end()))
      {
         std::cout << "REMOVING: " << iter->second->m_cacheId << "\n";
         if(!removeStateFromCache(iter->second->m_cacheId)) std::cout << "NOT REMOVED???\n";
         iter = m_lruCache.erase(iter);
      }
   }
}

ossim_uint32 ossim::ImageHandlerStateCache::getMaxStatesToCache()const
{
   ossim::ScopeReadLock lock(m_stateCacheMutex);
   return m_maxStatesToCache;
}

void ossim::ImageHandlerStateCache::removeState(const ossimString& stateCacheId)
{
   std::shared_ptr<Node> node = removeStateFromCache(stateCacheId);
   if(node)
   {
      removeStateFromLruCache(node->m_lruId);
   }
}

std::shared_ptr<ossim::ImageHandlerStateCache::Node> ossim::ImageHandlerStateCache::removeStateFromCache(const ossimString& stateCacheId)
{
   std::shared_ptr<Node> result;

   CacheType::iterator iter = m_cache.find(stateCacheId);
   if(iter != m_cache.end())
   {
      result = iter->second;
      m_cache.erase(iter);
   }   

   return result;
}

std::shared_ptr<ossim::ImageHandlerStateCache::Node> ossim::ImageHandlerStateCache::removeStateFromLruCache(ossim_uint64 stateCacheId)const
{
   std::shared_ptr<Node> result;

   LruType::iterator iter = m_lruCache.find(stateCacheId);
   if(iter != m_lruCache.end())
   {
      result = iter->second;
      m_lruCache.erase(iter);
   }   

   return result;
}  

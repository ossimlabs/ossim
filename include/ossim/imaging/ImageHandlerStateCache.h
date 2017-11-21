#ifndef ossimItemCache_HEADER
#define ossimItemCache_HEADER
#include <ossim/base/RWLock.h>
#include <map>
#include <memory>
#include <mutex>

namespace ossim
{
   template<class ItemType>
   class ItemCache
   {
   public:
      typename std::shared_ptr<ItemType> SharedItemType;
      struct Node
      {
         ossim_uint64              m_lruId;
         ossimString               m_cacheId;
         std::shared_ptr<ItemType> m_item;
      };
      typedef std::map<ossimString, std::shared_ptr<Node> > CacheType;
      typedef std::map<ossim_uint64, std::shared_ptr<Node> > LruType;
   
      std::shared_ptr<ItemType> getItem(const ossimString& key);
      std::shared_ptr<ItemType> getItem(const ossimString& key)const;

      void addItem(const ossimString& key, 
                   std::shared_ptr<ItemType> item);

      void setMaxItemsToCache(ossim_uint32 maxItemsToCache);
      ossim_uint32 getMaxItemsToCache()const;

   protected:
      mutable RWLock       m_itemCacheMutex;
      mutable RWLock       m_lruCacheMutex;
      mutable ossim_uint64 m_currentId{0};
      CacheType            m_cache;
      mutable LruType      m_lruCache;
      ossim_uint32         m_maxItemsToCache{100};

      void protectedAddItem(const ossimString& key, 
                             std::shared_ptr<ItemType> item);

      void shrinkCache();
      void touchNode(std::shared_ptr<Node> node)const;
      void removeItem(const ossimString& ItemCacheId);
      std::shared_ptr<Node> removeItemFromCache(const ossimString& ItemCacheId);
      std::shared_ptr<Node> removeItemFromLruCache(ossim_uint64 ItemCacheId)const;
      ossim_uint64 nextId()const;
   };

   template<class ItemType>
   typename std::shared_ptr<ItemType> ItemCache<ItemType>::getItem(const ossimString& key)
   {
      ossim::ScopeReadLock lock(m_itemCacheMutex);
      std::shared_ptr<ItemType> result;
      typename CacheType::iterator iter = m_cache.find(key);
      if(iter != m_cache.end())
      {
         result = iter->second->m_item;
         touchNode(iter->second);
      }
      return result;
   }

   template<class ItemType>
   typename std::shared_ptr<ItemType> ItemCache<ItemType>::getItem(const ossimString& key)const
   {
      ossim::ScopeReadLock lock(m_itemCacheMutex);
      std::shared_ptr<ItemType> result;
      typename CacheType::const_iterator iter = m_cache.find(key);
      if(iter != m_cache.end())
      {
         result = iter->second->m_item;
         touchNode(iter->second);
      }
      return result;
   }

   template<class ItemType>
   void ItemCache<ItemType>::addItem(const ossimString& key, 
                                                  std::shared_ptr<ItemType> item)
   {
      ossim::ScopeWriteLock lock(m_itemCacheMutex);
      protectedAddItem(key, item);
   }

   template<class ItemType>
   void ItemCache<ItemType>::protectedAddItem(const ossimString& key, 
                                                     std::shared_ptr<ItemType> item)
   {
      if(m_cache.size() > m_maxItemsToCache)
      {
         shrinkCache();
      }
      typename CacheType::iterator iter = m_cache.find(key);
      if(iter!= m_cache.end())
      {
         // update the item and LRU
         //
         std::shared_ptr<Node> node = iter->second;
         node->m_item = item;

         touchNode(node);
      }
      else
      {
         std::shared_ptr<Node> nodePtr = std::make_shared<Node>();
         nodePtr->m_cacheId = key;
         nodePtr->m_item = item;
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

   template<class ItemType>
   void ItemCache<ItemType>::touchNode(std::shared_ptr<Node> node)const
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

   template<class ItemType>
   ossim_uint64 ItemCache<ItemType>::nextId()const
   {
      return m_currentId++;
   }

   template<class ItemType>
   void ItemCache<ItemType>::setMaxItemsToCache(ossim_uint32 maxItemsToCache)
   {
     ossim::ScopeWriteLock lock(m_itemCacheMutex);
     m_maxItemsToCache = maxItemsToCache;
   }

   template<class ItemType>
   void ItemCache<ItemType>::shrinkCache()
   {
      ossim_uint64 shrinkToSize = ossim::round<ossim_uint64,ossim_float64>(m_maxItemsToCache * 0.8);

      if(shrinkToSize < 1)
      {
         m_lruCache.clear();
         m_cache.clear();
      }
      else
      {
         typename LruType::iterator iter = m_lruCache.begin();
         while((m_cache.size() > shrinkToSize)&&(iter != m_lruCache.end()))
         {
            removeItemFromCache(iter->second->m_cacheId);
            iter = m_lruCache.erase(iter);
         }
      }
   }

   template<class ItemType>
   ossim_uint32 ItemCache<ItemType>::getMaxItemsToCache()const
   {
      ossim::ScopeReadLock lock(m_itemCacheMutex);
      return m_maxItemsToCache;
   }

   template<class ItemType>
   void ItemCache<ItemType>::removeItem(const ossimString& ItemCacheId)
   {
      std::shared_ptr<Node> node = removeItemFromCache(ItemCacheId);
      if(node)
      {
         removeItemFromLruCache(node->m_lruId);
      }
   }

   template<class ItemType>
   std::shared_ptr< typename ItemCache<ItemType>::Node> ItemCache<ItemType>::removeItemFromCache(const ossimString& ItemCacheId)
   {
      std::shared_ptr<Node> result;

      typename CacheType::iterator iter = m_cache.find(ItemCacheId);
      if(iter != m_cache.end())
      {
         result = iter->second;
         m_cache.erase(iter);
      }   

      return result;
   }

   template<class ItemType>
   std::shared_ptr<typename ItemCache<ItemType>::Node> ItemCache<ItemType>::removeItemFromLruCache(ossim_uint64 ItemCacheId)const
   {
      std::shared_ptr<Node> result;

      typename LruType::iterator iter = m_lruCache.find(ItemCacheId);
      if(iter != m_lruCache.end())
      {
         result = iter->second;
         m_lruCache.erase(iter);
      }   

      return result;
   } 

}



#endif

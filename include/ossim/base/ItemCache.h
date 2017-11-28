#ifndef ossimItemCache_HEADER
#define ossimItemCache_HEADER
#include <ossim/base/RWLock.h>
#include <map>
#include <memory>
#include <mutex>

namespace ossim
{
   /**
   * This is a generic cache.  The only requirement is that it expects 
   * the item to support shared_ptr.  The ItemCache allows one to specify the
   * maximum items to cache and will use a "Least Recently Used" (LRU) to purge old
   * items from the cache back to a minimum cache size.
   *
   * Example Use:
   *
   * @code
    #include <ossim/base/ItemCache.h>
    #include <ossim/support_data/TiffHandlerState.h>
    int main(int argc, char *argv[])
    {
      int returnCode = 0;
       
       ossimArgumentParser ap(&argc, argv);
       ossimInit::instance()->addOptions(ap);
       ossimInit::instance()->initialize(ap);
    
       try
       {
          ossim_uint32 maxStates=10;
          ossim::ItemCache<ossim::ImageHandlerState> stateCache;
          ossim_uint32 idx = 0;
          stateCache.setMaxItemsToCache(maxStates);
          while(idx < maxStates)
          {
            ossimString id = ossimString::toString(idx);
            stateCache.addItem(id, std::make_shared<ossim::TiffHandlerState>()); 
            ++idx;  
         }
         stateCache.getItem(ossimString::toString(0)
         stateCache.getItem(ossimString::toString(maxStates-1);

         // should be shrinking cache size back to a minimum size of default
         // 80% capacity
         stateCache.addItem(ossimString::toString(idx++), std::make_shared<ossim::TiffHandlerState>()); 
         stateCache.addItem(ossimString::toString(idx++), std::make_shared<ossim::TiffHandlerState>()); 
         stateCache.addItem(ossimString::toString(idx++), std::make_shared<ossim::TiffHandlerState>()); 

      }
      catch(const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         returnCode = 1;
      }
      catch( ... )
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossim-foo caught unhandled exception!" << std::endl;
         returnCode = 1;
      }
      
      return returnCode;
   }
   * @endCode
   */
   template<class ItemType>
   class ItemCache
   {
   public:
      typename std::shared_ptr<ItemType> SharedItemType;
      /**
      * Holds information about the Item we are chaching.  
      * Holds the current lruId and the cacheId and the item we
      * are caching
      *
      * Lru is adjusted when the item is searched with the getItem
      * 
      */
      struct Node
      {
         ossim_uint64              m_lruId;
         ossimString               m_cacheId;
         std::shared_ptr<ItemType> m_item;
      };
      typedef std::map<ossimString,  std::shared_ptr<Node> > CacheType;
      typedef std::map<ossim_uint64, std::shared_ptr<Node> > LruType;
   
      /**
      * Get item will adjust the LRU if the item is present.
      *
      * @param key Is the key used to identify the item you are
      *            trying to retrieve.
      * @return the shared pointer to the item you are returning or
      *         null otherwise
      */
      std::shared_ptr<ItemType> getItem(const ossimString& key);
      std::shared_ptr<ItemType> getItem(const ossimString& key)const;

      void addItem(const ossimString& key, 
                   std::shared_ptr<ItemType> item);

      std::shared_ptr<ItemType> removeItem(const ossimString& key);

      void reset();
      void setMinAndMaxItemsToCache(ossim_uint32 minItemsToCache, ossim_uint32 maxItemsToCache);
      ossim_uint32 getMaxItemsToCache()const;
      ossim_uint32 getMinItemsToCache()const;
      
   protected:
      mutable RWLock       m_itemCacheMutex;
      mutable RWLock       m_lruCacheMutex;
      mutable ossim_uint64 m_currentId{0};
      CacheType            m_cache;
      mutable LruType      m_lruCache;
      ossim_uint32         m_maxItemsToCache{100};
      ossim_uint32         m_minItemsToCache{80};

      void protectedAddItem(const ossimString& key, 
                             std::shared_ptr<ItemType> item);

      void shrinkCache();
      void touchNode(std::shared_ptr<Node> node)const;
      std::shared_ptr<Node> removeItemFromCache(const ossimString& key);
      std::shared_ptr<Node> removeItemFromLruCache(ossim_uint64 key)const;
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
   typename std::shared_ptr<ItemType> ItemCache<ItemType>::removeItem(const ossimString& key)
   {
      ossim::ScopeWriteLock lock(m_itemCacheMutex);
      std::shared_ptr<ItemType> result;
      std::shared_ptr<Node> node = removeItemFromCache(key);
      if(node)
      {
         result = node->m_item;
         removeItemFromLruCache(node->m_lruId);
      }

      return result;
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
   void ItemCache<ItemType>::setMinAndMaxItemsToCache(ossim_uint32 maxItemsToCache, 
                                                ossim_uint32 minItemsToCache)
   {
     ossim::ScopeWriteLock lock(m_itemCacheMutex);
     m_maxItemsToCache = maxItemsToCache;
     m_minItemsToCache = minItemsToCache;
   }
   template<class ItemType>
   void ItemCache<ItemType>::reset()
   {
     ossim::ScopeWriteLock lock(m_itemCacheMutex);
     m_cache.clear();
     m_lruCache.clear();
     m_currentId = 0;      
   }

   template<class ItemType>
   void ItemCache<ItemType>::shrinkCache()
   {
      if(m_minItemsToCache < 1)
      {
         m_lruCache.clear();
         m_cache.clear();
      }
      else
      {
         typename LruType::iterator iter = m_lruCache.begin();
         ossim_uint32 previousSize = m_cache.size();
         while((m_cache.size() > m_minItemsToCache)&&
               (iter != m_lruCache.end()))
         {
            removeItemFromCache(iter->second->m_cacheId);
            iter = m_lruCache.erase(iter);

            // sanity check to make sure we continue to shrink at
            // each iteration
            // avoids infinite loop
            if(m_cache.size() >= previousSize)
            {
               break;
            }
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
   ossim_uint32 ItemCache<ItemType>::getMinItemsToCache()const
   {
      ossim::ScopeReadLock lock(m_itemCacheMutex);
      return m_minItemsToCache;
   }

   template<class ItemType>
   std::shared_ptr< typename ItemCache<ItemType>::Node> ItemCache<ItemType>::removeItemFromCache(const ossimString& key)
   {
      std::shared_ptr<Node> result;

      typename CacheType::iterator iter = m_cache.find(key);
      if(iter != m_cache.end())
      {
         result = iter->second;
         m_cache.erase(iter);
      }   

      return result;
   }

   template<class ItemType>
   std::shared_ptr<typename ItemCache<ItemType>::Node> ItemCache<ItemType>::removeItemFromLruCache(ossim_uint64 key)const
   {
      std::shared_ptr<Node> result;

      typename LruType::iterator iter = m_lruCache.find(key);
      if(iter != m_lruCache.end())
      {
         result = iter->second;
         m_lruCache.erase(iter);
      }   

      return result;
   } 

}



#endif

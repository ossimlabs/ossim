#ifndef ossimImageHandlerStateCache_HEADER
#define ossimImageHandlerStateCache_HEADER
#include <ossim/support_data/ImageHandlerState.h>
#include <ossim/base/RWLock.h>
#include <map>
#include <memory>
#include <mutex>

namespace ossim
{
   class ImageHandlerStateCache
   {
   public:
      struct Node
      {
         ossim_uint64                              m_lruId;
         ossimString                               m_cacheId;
         std::shared_ptr<ossim::ImageHandlerState> m_state;
      };
      typedef std::map<ossimString, std::shared_ptr<Node> > CacheType;
      typedef std::map<ossim_uint64, std::shared_ptr<Node> > LruType;
   
      std::shared_ptr<ossim::ImageHandlerState>       getState(const ossimString& key);
      std::shared_ptr<const ossim::ImageHandlerState> getState(const ossimString& key)const;

      void addState(const ossimString& key, 
                    std::shared_ptr<ossim::ImageHandlerState> state);

      void setMaxStatesToCache(ossim_uint32 maxStatesToCache);
      ossim_uint32 getMaxStatesToCache()const;

   protected:
      mutable RWLock       m_stateCacheMutex;
      mutable RWLock       m_lruCacheMutex;
      mutable ossim_uint64 m_currentId{0};
      CacheType            m_cache;
      mutable LruType      m_lruCache;
      ossim_uint32         m_maxStatesToCache{100};

      void protectedAddState(const ossimString& key, 
                             std::shared_ptr<ossim::ImageHandlerState> state);

      void shrinkCache();
      void touchNode(std::shared_ptr<Node> node)const;
      void removeState(const ossimString& stateCacheId);
      std::shared_ptr<Node> removeStateFromCache(const ossimString& stateCacheId);
      std::shared_ptr<Node> removeStateFromLruCache(ossim_uint64 stateCacheId)const;
      ossim_uint64 nextId()const;
   };
}

#endif

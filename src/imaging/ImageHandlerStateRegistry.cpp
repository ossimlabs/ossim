#include <ossim/imaging/ImageHandlerStateRegistry.h>
#include <ossim/imaging/ImageHandlerStateFactory.h>


ossim::ImageHandlerStateRegistry::ImageHandlerStateRegistry()
{
}

std::shared_ptr<ossim::ImageHandlerStateRegistry> ossim::ImageHandlerStateRegistry::instance()
{

   static std::shared_ptr<ossim::ImageHandlerStateRegistry> imageHandlerStateRegistry = std::make_shared<ImageHandlerStateRegistry>();

   return imageHandlerStateRegistry;
}

std::shared_ptr<ossim::ImageHandlerState> ossim::ImageHandlerStateRegistry::createState(const ossimKeywordlist& kwl,
                                                                    const ossimString& prefix)const
{
   ScopeReadLock scopedReadLock(m_rwlock);
   std::shared_ptr<ossim::ImageHandlerState> result;
   for(const auto& factory:m_factoryList)
   {
      result = factory->createState(kwl, prefix);
      if(result) break;
   }

   return result;
}

std::shared_ptr<ossim::ImageHandlerState> ossim::ImageHandlerStateRegistry::createState(const ossimString& typeName)const
{
   ScopeReadLock scopedReadLock(m_rwlock);
   std::shared_ptr<ossim::ImageHandlerState> result;
   for(const auto& factory:m_factoryList)
   {
      result = factory->createState(typeName);
      if(result) break;
   }

   return result;
}


//*******************************************************************
//
// License: MIT
// 
// Author:  Garrett Potts
//
// Description:
//
// Contains class definition for ImageHandlerRegistry.
//
//*******************************************************************
// $Id$

#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerFactory.h>
#include <ossim/imaging/ossimImageHandlerFactoryBase.h>
#include <algorithm>

using namespace std;

static ossimTrace traceDebug("ossimImageHandlerRegistry:debug");


RTTI_DEF1(ossimImageHandlerRegistry, "ossimImageHandlerRegistry", ossimObjectFactory);

//ossimImageHandlerRegistry* ossimImageHandlerRegistry::theInstance = 0;

ossimImageHandlerRegistry::ossimImageHandlerRegistry()
{
   ossimObjectFactoryRegistry::instance()->registerFactory(this);
   registerFactory(ossimImageHandlerFactory::instance());
   initializeStateCache();
}

ossimImageHandlerRegistry* ossimImageHandlerRegistry::instance()
{
   static ossimImageHandlerRegistry sharedInstance;
   
   return &sharedInstance;
}

ossimImageHandlerRegistry::~ossimImageHandlerRegistry()
{
   unregisterAllFactories();
}


ossimObject* ossimImageHandlerRegistry::createObject(const ossimKeywordlist& kwl,
                                                     const char* prefix)const
{
   ossimObject* result = createObjectFromRegistry(kwl, prefix);
   if(!result)
   {
      result = open(kwl, prefix);
   }
   return result;
}

ossimRefPtr<ossimImageHandler> ossimImageHandlerRegistry::openBySuffix(const ossimFilename& file,
                                                                       bool openOverview)const
{
   std::vector<ossimRefPtr<ossimImageHandler> > handlers;
   
   getImageHandlersBySuffix(handlers, file.ext());
   ossim_uint32 idx = 0;
   ossim_uint32 size = (ossim_uint32) handlers.size();
   
   for(idx = 0; idx < size; ++idx)
   {
      handlers[idx]->setOpenOverviewFlag(openOverview);
      if(handlers[idx]->open(file))
      {
         return handlers[idx];
      }
   }
   
   return ossimRefPtr<ossimImageHandler>(0);
}

void ossimImageHandlerRegistry::getImageHandlersBySuffix(ossimImageHandlerFactoryBase::ImageHandlerList& result,
                                                         const ossimString& ext)const
{
   vector<ossimImageHandlerFactoryBase*>::const_iterator iter = m_factoryList.begin();
   ossimImageHandlerFactoryBase::ImageHandlerList temp;
   while(iter != m_factoryList.end())
   {
      temp.clear();
      (*iter)->getImageHandlersBySuffix(temp, ext);
      
      if(!temp.empty())
      {
         
         // now append to the end of the typeList.
         result.insert(result.end(),
                       temp.begin(),
                       temp.end());
      }
      ++iter;
   }
}

void ossimImageHandlerRegistry::getImageHandlersByMimeType(
   ossimImageHandlerFactoryBase::ImageHandlerList& result, const ossimString& mimeType)const
{
   vector<ossimImageHandlerFactoryBase*>::const_iterator iter = m_factoryList.begin();
   ossimImageHandlerFactoryBase::ImageHandlerList temp;
   while(iter != m_factoryList.end())
   {
      temp.clear();
      (*iter)->getImageHandlersByMimeType(temp, mimeType);
      
      if(!temp.empty())
      {
         
         // now append to the end of the typeList.
         result.insert(result.end(),
                       temp.begin(),
                       temp.end());
      }
      ++iter;
   }
}

void ossimImageHandlerRegistry::getTypeNameList( std::vector<ossimString>& typeList ) const
{
   getAllTypeNamesFromRegistry(typeList);
}

void ossimImageHandlerRegistry::getSupportedExtensions(
   ossimImageHandlerFactoryBase::UniqueStringList& extensionList)const
{
   vector<ossimString> result;
   vector<ossimImageHandlerFactoryBase*>::const_iterator iter = m_factoryList.begin();

   while(iter != m_factoryList.end())
   {
      (*iter)->getSupportedExtensions(extensionList);

      ++iter;
   }
   
}


std::shared_ptr<ossim::ImageHandlerState> ossimImageHandlerRegistry::getState(const ossimString& connectionString, 
                                                                              ossim_uint32 entry)const
{
   return getState(connectionString + "_e" + ossimString::toString(entry));
}

std::shared_ptr<ossim::ImageHandlerState> ossimImageHandlerRegistry::getState(const ossimString& id)const
{
   std::shared_ptr<ossim::ImageHandlerState> result;

   if(m_stateCache)
   {
      result = m_stateCache->getItem(id);
   }

   return result;
}


ossimRefPtr<ossimImageHandler> ossimImageHandlerRegistry::openConnection(
   const ossimString& connectionString, bool openOverview )const
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::openConnection: entered.........." << std::endl;
   }
   ossimRefPtr<ossimImageHandler> result(0);

   std::string myConnectionString = connectionString.downcase().string();
   std::string fileStr = "file://";
   std::size_t found = myConnectionString.find( fileStr );
   if ( found == 0 )
   {
      myConnectionString = connectionString.string().substr( fileStr.size() );
   }
   else
   {
      myConnectionString = connectionString.string();
   }
 
   // add entry 0
   std::shared_ptr<ossim::ImageHandlerState> state = getState(myConnectionString, 0);
   if(state)
   {
      if(traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::openConnection: leaving with open(state).........." << std::endl;;
      }
      return open(state);
   }  

   std::shared_ptr<ossim::istream> str = ossim::StreamFactoryRegistry::instance()->
      createIstream( myConnectionString, std::ios_base::in|std::ios_base::binary);

   if ( str )
   {
      result = open( str, myConnectionString, openOverview );
   }

   if ( !result.valid() )
   {
      ossimFilename f = myConnectionString;
      if ( f.exists() )
      {
         result = open( f, true, openOverview );
      }
   }

   if(result)
   {
      addToStateCache(result.get());
   }
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::openConnection: leaving.........." << std::endl;
   }
   
   return result;
}

ossimImageHandler* ossimImageHandlerRegistry::open(const ossimFilename& filename,
                                                   bool trySuffixFirst,
                                                   bool openOverview)const
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::open(file, trySuffix,openOverview): entered.........." << std::endl;
   }
   std::shared_ptr<ossim::ImageHandlerState> state = getState(filename, 0);

   if(state)
   {
      ossimRefPtr<ossimImageHandler> h = open(state);
      if(h)
      {
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::open(file, trySuffix,openOverview): returning with state open.........." << std::endl;;
         }
         return h.release();
      }
   }

   if(trySuffixFirst)
   {
      ossimRefPtr<ossimImageHandler> h = openBySuffix(filename, openOverview);
      if(h.valid())
      {
         addToStateCache(h.get());
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::open(file, trySuffix,openOverview): leaving.........." << std::endl;
         }
         return h.release();
      }
   }
   
   // now try magic number opens
   //
   ossimImageHandler*                   result = NULL;
   vector<ossimImageHandlerFactoryBase*>::const_iterator factory = m_factoryList.begin();
   while((factory != m_factoryList.end()) && !result)
   {
      result = (*factory)->open(filename, openOverview);
      ++factory;
   }
 
   if(result)
   {
      addToStateCache(result);
   }  
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::open(file, trySuffix,openOverview): leaving.........." << std::endl;
   }
   return result;
}

ossimImageHandler* ossimImageHandlerRegistry::open(const ossimKeywordlist& kwl,
                                                   const char* prefix)const
{
   ossimImageHandler*                   result = NULL;
   vector<ossimImageHandlerFactoryBase*>::const_iterator factory;
   
   factory = m_factoryList.begin();
   while((factory != m_factoryList.end()) && !result)
   {
      result = (*factory)->open(kwl, prefix);
      ++factory;
   }

   if(result)
   {
      addToStateCache(result);
   }  
   
   return result;
}

ossimRefPtr<ossimImageHandler> ossimImageHandlerRegistry::open(
   std::shared_ptr<ossim::istream>& str,
   const std::string& connectionString,
   bool openOverview ) const
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::open(stream,connectionString,openOverview): entered.........." << std::endl;
   }
   ossimRefPtr<ossimImageHandler> result = 0;
   std::shared_ptr<ossim::ImageHandlerState> state = getState(connectionString, 0);
   
   if(state)
   {
      result = open(state);
      if(result)
      {
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::open(stream,connectionString,openOverview): leaving with state open..........Valid? " 
                                               << result.valid() <<std::endl;
         }
         return result;
      }
   }
   if ( str )
   {
      vector<ossimImageHandlerFactoryBase*>::const_iterator factory = m_factoryList.begin();
      while( factory != m_factoryList.end() )
      {
         result = (*factory)->open( str, connectionString, openOverview );
         if ( result.valid() )
         {
            break;
         }
         ++factory;
      }
   }
   if(result)
   {
      addToStateCache(result.get());
   }  
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::open(stream,connectionString,openOverview): leaving..........Valid? " 
                                         << result.valid()<<std::endl;
   }
   return result; 
}

ossimRefPtr<ossimImageHandler> ossimImageHandlerRegistry::open(std::shared_ptr<ossim::ImageHandlerState> state)const
{
   ossimRefPtr<ossimImageHandler> result = 0;
   vector<ossimImageHandlerFactoryBase*>::const_iterator factory = m_factoryList.begin();
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::open(state): Entered......." << std::endl;
   }
   while( (!result)&&(factory != m_factoryList.end()) )
   {
      result = (*factory)->open( state );
      if ( result )
      {
         break;
      }
      ++factory;
   }
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::open(state): Leaving.......valid?" << result.valid()<<std::endl;
   }
   return result; 
}

#if 0
ossimRefPtr<ossimImageHandler> ossimImageHandlerRegistry::open( ossim::istream* str,
                                                                std::streamoff restartPosition,
                                                                bool youOwnIt ) const
{
   ossimRefPtr<ossimImageHandler> result = 0;
   vector<ossimImageHandlerFactoryBase*>::const_iterator factory = m_factoryList.begin();
   while( factory != m_factoryList.end() )
   {
      result = (*factory)->open( str, restartPosition, youOwnIt );
      if ( result.valid() )
      {
         break;
      }
      ++factory;
   }  
   return result; 
}
#endif

ossimRefPtr<ossimImageHandler> ossimImageHandlerRegistry::openOverview(
   const ossimFilename& file ) const
{
   ossimRefPtr<ossimImageHandler> result = 0;

   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::openOverview: Entered......." << std::endl;
   }

   std::shared_ptr<ossim::ImageHandlerState> state = getState(file, 0);

   if(state)
   {
      result = open(state);

   }

   if(!result)
   {
      // See if we can open via the stream interface:
      std::shared_ptr<ossim::istream> str = ossim::StreamFactoryRegistry::instance()->
         createIstream( file, std::ios_base::in|std::ios_base::binary);
      
      if ( str )
      {
         std::vector<ossimImageHandlerFactoryBase*>::const_iterator factory = m_factoryList.begin();
         while( factory != m_factoryList.end() )
         {
            result = (*factory)->openOverview( str, file );
            if ( result.valid() )
            {
               break;
            }
            ++factory;
         }

         str = 0;
      }

      if ( (result.valid() == false) && file.exists() )
      {  
         vector<ossimImageHandlerFactoryBase*>::const_iterator factory = m_factoryList.begin();
         while( factory != m_factoryList.end() )
         {
            result = (*factory)->openOverview( file );
            if ( result.valid() )
            {
               break;
            }
            ++factory;
         }
      }

      if(result)
      {
         addToStateCache(result.get());
      }
   }
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::openOverview: Leaving.......Valid? " 
                                         << result.valid() << std::endl;
   }
   return result;
}

ossimObject* ossimImageHandlerRegistry::createObject(const ossimString& typeName) const
{
   return createObjectFromRegistry(typeName);
}

std::ostream& ossimImageHandlerRegistry::printReaderProps(std::ostream& out) const
{
   // Loop through factories:
   vector<ossimImageHandlerFactoryBase*>::const_iterator factory = m_factoryList.begin();
   while( factory != m_factoryList.end() )
   {
      out << "factory: " << (*factory)->getClassName() << "\n";
      
      // Loop through factory image handlers:
      std::vector<ossimString> readerList;
      (*factory)->getTypeNameList(readerList);

      std::vector<ossimString>::const_iterator i = readerList.begin();
      while ( i != readerList.end() )
      {
         ossimRefPtr<ossimImageHandler> ih =
            dynamic_cast<ossimImageHandler*>( (*factory)->createObject( (*i) ) );
         if ( ih.valid() )
         {
            out << "reader: " << ih->getClassName() << "\n";

            // Loop through image handler properties:
            std::vector<ossimString> propNames;
            ih->getPropertyNames(propNames);
            if ( propNames.size() )
            {
               out << "\nproperties:\n";
               ossimRefPtr<ossimProperty> prop = 0;
               std::vector<ossimString>::const_iterator p = propNames.begin();
               while ( p != propNames.end() )
               {
                  out << "   " << (*p) << "\n";
                  prop = ih->getProperty( *p );
                  if ( prop.valid() )
                  {
                     ossimStringProperty* stringProp =
                        dynamic_cast<ossimStringProperty*>(prop.get());
                     if ( stringProp )
                     {
                        if ( stringProp->getConstraints().size() )
                        {
                           out << "      constraints:\n";
                           std::vector<ossimString>::const_iterator strPropIter =
                              stringProp->getConstraints().begin();
                           while( strPropIter != stringProp->getConstraints().end() )
                           {
                              out << "         " << (*strPropIter) << "\n";
                              ++strPropIter;
                           }
                        }
                     }
                  }
                  
                  ++p;
               }
               out << "\n";
            }
         }
         ++i;
      }
      ++factory;
   }
   out << std::endl;
   return out;
}

ossimImageHandlerRegistry::ossimImageHandlerRegistry(const ossimImageHandlerRegistry& /* rhs */)
   :  ossimObjectFactory()
{}

void ossimImageHandlerRegistry::initializeStateCache()const
{
   m_stateCache = 0;
   ossimString enabledString = ossimPreferences::instance()->findPreference("ossim.imaging.handler.registry.state_cache.enabled");
   ossimString minSizeString = ossimPreferences::instance()->findPreference("ossim.imaging.handler.registry.state_cache.min_size");
   ossimString maxSizeString = ossimPreferences::instance()->findPreference("ossim.imaging.handler.registry.state_cache.max_size");

   ossim_uint32 maxSize = 0;
   ossim_uint32 minSize = 0;

   if(!enabledString.empty())
   {
      if(enabledString.toBool())
      {
         m_stateCache = std::make_shared<ossim::ItemCache<ossim::ImageHandlerState> >();
         if(!maxSizeString.empty())
         {
            maxSize = maxSizeString.toUInt32();
         }
         if(!minSizeString.empty())
         {
            minSize = minSizeString.toUInt32();
         }
         else if(maxSize)
         {
            minSize = ossim::round<ossim_uint32, ossim_float32>(maxSize*.8);
         }

         if(minSize < maxSize)
         {
            m_stateCache->setMinAndMaxItemsToCache(minSize, maxSize);
         }
      }

   }
}

void ossimImageHandlerRegistry::addToStateCache(ossimImageHandler* handler)const
{
   if(handler)
   {
      std::shared_ptr<ossim::ImageHandlerState> state = handler->getState();
      if(state&&m_stateCache)
      {
         ossimString id = handler->getFilename()+"_e"+ossimString::toString(state->getCurrentEntry());
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)<< "ossimImageHandlerRegistry::addToStateCache: " << id << std::endl;
         }
         m_stateCache->addItem(id, state);
      }
   }
}



const ossimImageHandlerRegistry&
ossimImageHandlerRegistry::operator=(const ossimImageHandlerRegistry& rhs)
{ return rhs; }

extern "C"
{
  void* ossimImageHandlerRegistryGetInstance()
  {
    return ossimImageHandlerRegistry::instance();
  }
}

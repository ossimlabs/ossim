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
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerFactory.h>
#include <ossim/imaging/ossimImageHandlerFactoryBase.h>
#include <algorithm>

RTTI_DEF1(ossimImageHandlerRegistry, "ossimImageHandlerRegistry", ossimObjectFactory);

//ossimImageHandlerRegistry* ossimImageHandlerRegistry::theInstance = 0;

ossimImageHandlerRegistry::ossimImageHandlerRegistry()
{
   ossimObjectFactoryRegistry::instance()->registerFactory(this);
   registerFactory(ossimImageHandlerFactory::instance());
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

ossimRefPtr<ossimImageHandler> ossimImageHandlerRegistry::openConnection(
   const ossimString& connectionString, bool openOverview )const
{
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
   
   std::shared_ptr<ossim::istream> str = ossim::StreamFactoryRegistry::instance()->
      createIstream( myConnectionString, std::ios_base::in|std::ios_base::binary);

   if ( str )
   {
      result = open( str, myConnectionString, openOverview );
   }

   if ( result.valid() == false )
   {
      ossimFilename f = myConnectionString;
      if ( f.exists() )
      {
         result = this->open( f, true, openOverview );
      }
   }
   
   return result;
}

ossimImageHandler* ossimImageHandlerRegistry::open(const ossimFilename& fileName,
                                                   bool trySuffixFirst,
                                                   bool openOverview)const
{
   if(trySuffixFirst)
   {
      ossimRefPtr<ossimImageHandler> h = openBySuffix(fileName, openOverview);
      if(h.valid())
      {
         return h.release();
      }
   }
   
   // now try magic number opens
   //
   ossimImageHandler*                   result = NULL;
   vector<ossimImageHandlerFactoryBase*>::const_iterator factory = m_factoryList.begin();
   while((factory != m_factoryList.end()) && !result)
   {
      result = (*factory)->open(fileName, openOverview);
      ++factory;
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
   
   return result;
}

ossimRefPtr<ossimImageHandler> ossimImageHandlerRegistry::open(
   std::shared_ptr<ossim::istream>& str,
   const std::string& connectionString,
   bool openOverview ) const
{
   ossimRefPtr<ossimImageHandler> result = 0;
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

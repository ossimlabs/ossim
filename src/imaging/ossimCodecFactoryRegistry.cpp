#include <ossim/imaging/ossimCodecFactoryRegistry.h>
#include <ossim/imaging/ossimCodecFactory.h>

ossimCodecFactoryRegistry* ossimCodecFactoryRegistry::m_instance=0;

/** hidden from use default constructor */
ossimCodecFactoryRegistry::ossimCodecFactoryRegistry()
{
   m_instance = this;
}

/** hidden from use copy constructor */
ossimCodecFactoryRegistry::ossimCodecFactoryRegistry(const ossimCodecFactoryRegistry& /* obj */)
{
   m_instance = this;
}

/** hidden from use operator = */
const ossimCodecFactoryRegistry& ossimCodecFactoryRegistry::operator=(const ossimCodecFactoryRegistry& /* rhs */)
{
   return *this;
}

ossimCodecFactoryRegistry* ossimCodecFactoryRegistry::instance()
{
   if(!m_instance)
   {
      m_instance = new ossimCodecFactoryRegistry();
      m_instance->registerFactory(ossimCodecFactory::instance());
   }
   
   return m_instance;
}

ossimCodecFactoryRegistry::~ossimCodecFactoryRegistry()
{
   m_instance = 0;
}


ossimCodecBase* ossimCodecFactoryRegistry::createCodec(const ossimString& type)const
{
   return createNativeObjectFromRegistry(type);
}

ossimCodecBase* ossimCodecFactoryRegistry::createCodec(const ossimKeywordlist& kwl, 
                                                       const char* prefix)const
{
   return createNativeObjectFromRegistry(kwl, prefix);
}

void ossimCodecFactoryRegistry::getTypeNameList(std::vector<ossimString>& typeNames)const
{
   getAllTypeNamesFromRegistry(typeNames);
}

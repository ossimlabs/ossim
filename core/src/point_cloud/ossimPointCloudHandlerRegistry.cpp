//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id$

#include <ossim/point_cloud/ossimPointCloudHandlerRegistry.h>
#include <ossim/point_cloud/ossimPointCloudHandler.h>
#include <ossim/point_cloud/ossimPointCloudHandlerFactory.h>

ossimPointCloudHandlerRegistry* ossimPointCloudHandlerRegistry::m_instance = 0;

RTTI_DEF1(ossimPointCloudHandlerRegistry, "ossimPointCloudHandlerRegistry" , ossimObjectFactory);

ossimPointCloudHandlerRegistry::ossimPointCloudHandlerRegistry()
{
   // TODO Auto-generated constructor stub
}

ossimPointCloudHandlerRegistry::~ossimPointCloudHandlerRegistry()
{
   // TODO Auto-generated destructor stub
}

ossimPointCloudHandlerRegistry* ossimPointCloudHandlerRegistry::instance()
{
   if (m_instance)
      return m_instance;
   m_instance = new ossimPointCloudHandlerRegistry;
   return m_instance;
}

ossimPointCloudHandler* ossimPointCloudHandlerRegistry::open(const ossimFilename& fileName) const
{
   ossimPointCloudHandler*  result = NULL;
   std::vector<ossimPointCloudHandlerFactory*>::const_iterator factory;

   factory = m_factoryList.begin();
   while((factory != m_factoryList.end()) && !result)
   {
      result = (*factory)->open(fileName);
      ++factory;
   }

   return result;
}

ossimPointCloudHandler* ossimPointCloudHandlerRegistry::open(const ossimKeywordlist& kwl,
                                                             const char* prefix) const
{
   ossimPointCloudHandler* result = NULL;
   std::vector<ossimPointCloudHandlerFactory*>::const_iterator factory;

   factory = m_factoryList.begin();
   while((factory != m_factoryList.end()) && !result)
   {
      result = (*factory)->open(kwl, prefix);
      ++factory;
   }

   return result;
}

ossimRefPtr<ossimPointCloudHandler> ossimPointCloudHandlerRegistry::open(std::istream* /*str*/,
                                                 std::streamoff /*restartPosition*/,
                                                 bool /*youOwnIt*/) const
{
   // TODO Auto-generated constructor stub
   return 0;
}

ossimObject* ossimPointCloudHandlerRegistry::createObject(const ossimString& typeName) const
{
   return createObjectFromRegistry(typeName);
}

ossimObject* ossimPointCloudHandlerRegistry::createObject(const ossimKeywordlist& kwl,
                                                          const char* prefix) const
{
   return createObjectFromRegistry(kwl, prefix);
}

const ossimPointCloudHandlerRegistry& ossimPointCloudHandlerRegistry::operator=(
      const ossimPointCloudHandlerRegistry& /*rhs*/)
{
   return *this;
}

void ossimPointCloudHandlerRegistry::getTypeNameList(std::vector<ossimString>& typeList) const
{
   getAllTypeNamesFromRegistry(typeList);
}



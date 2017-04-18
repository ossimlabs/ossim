//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimUtilityFactory_HEADER
#define ossimUtilityFactory_HEADER 1

#include <ossim/base/ossimString.h>
#include <ossim/util/ossimToolFactoryBase.h>

class ossimCoreUtility;
class ossimFilename;
class ossimKeywordlist;

class OSSIMDLLEXPORT ossimToolFactory : public ossimToolFactoryBase
{
public:
   virtual ~ossimToolFactory();
   static ossimToolFactory* instance();

   virtual ossimTool* createTool(const std::string& typeName) const;

   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamically and this name must be unique.
    */
   virtual void getTypeNameList(std::vector<ossimString>& typeList) const;

   /**
    * Appends map with available utilities along with descriptions as <name, description> pairs.
    * The names are the human-readable name of the utility/service, typically the utility's class
    * name without the "ossim" prefix nor "Tool" suffix, all lowercase.
    */
   virtual void getCapabilities(std::map<std::string, std::string>& capabilities) const;
   virtual std::map<std::string, std::string> getCapabilities() const;

protected:
   ossimToolFactory();
   ossimToolFactory(const ossimToolFactory&){}
   void operator = (const ossimToolFactory&){}

   static ossimToolFactory* s_Instance;

};

#endif

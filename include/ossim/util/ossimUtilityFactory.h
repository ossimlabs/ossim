//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimUtilityFactory_HEADER
#define ossimUtilityFactory_HEADER 1

#include <ossim/util/ossimUtilityFactoryBase.h>
#include <ossim/base/ossimString.h>

class ossimCoreUtility;
class ossimFilename;
class ossimKeywordlist;

class OSSIMDLLEXPORT ossimUtilityFactory : public ossimUtilityFactoryBase
{
public:
   virtual ~ossimUtilityFactory();
   static ossimUtilityFactory* instance();

   virtual ossimUtility* createUtility(const ossimString& typeName) const;

   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamically and this name must be unique.
    */
   virtual void getTypeNameList(std::vector<ossimString>& typeList) const;

   /**
    * Appends map with available utilities along with descriptions as <name, description> pairs.
    * The names are the human-readable name of the utility/service, typically the utility's class
    * name without the "ossim" prefix nor "Util" suffix, all lowercase.
    */
   virtual void getCapabilities(std::map<std::string, std::string>& capabilities) const;

protected:
   ossimUtilityFactory();
   ossimUtilityFactory(const ossimUtilityFactory&){}
   void operator = (const ossimUtilityFactory&){}

   static ossimUtilityFactory* s_Instance;

};

#endif

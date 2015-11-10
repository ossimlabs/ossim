//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimUtilityFactoryBase_HEADER
#define ossimUtilityFactoryBase_HEADER

#include <ossim/base/ossimObjectFactory.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>

class ossimUtility;

class OSSIM_DLL ossimUtilityFactoryBase : public ossimObjectFactory
{
public:
   virtual ossimObject* createObject(const ossimString& typeName) const
   { return (ossimObject*) createUtility(typeName); }

   virtual ossimObject* createObject(const ossimKeywordlist& kwl, const char* prefix=0) const
   { return (ossimObject*) createUtility(kwl, prefix); }

   virtual ossimUtility* createUtility(const ossimString& typeName) const=0;

   virtual ossimUtility* createUtility(const ossimKeywordlist& kwl, const char* prefix=0) const
   {
      ossimString type = kwl.find(prefix, ossimKeywordNames::TYPE_KW);
      return createUtility(type);
   }

   /**
    * Appends map with available utilities along with descriptions as <name, decription> pairs.
    * The names are the human-readable name of the utility/service, typically the utility's class
    * name without the "ossim" prefix nor "Util" suffix, all lowercase.
    */
   virtual void getCapabilities(std::map<std::string, std::string>& capabilities) const = 0;
};

#endif

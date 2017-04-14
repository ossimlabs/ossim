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
#include <ossim/util/ossimTool.h>
#include <string>

class OSSIM_DLL ossimToolFactoryBase : public ossimObjectFactory
{
public:
   virtual ossimObject* createObject(const ossimString& typeName) const
   { return (ossimObject*) createTool(typeName); }

   virtual ossimObject* createObject(const ossimKeywordlist& kwl, const char* prefix=0) const
   { return (ossimObject*) createTool(kwl, prefix); }

   virtual ossimTool* createTool(const std::string& typeName) const=0;

   virtual ossimTool* createTool(const ossimKeywordlist& kwl, const char* prefix=0) const
   {
      ossimString type = kwl.find(prefix, ossimKeywordNames::TYPE_KW);
      return createTool(type);
   }

   /**
    * Appends map with available utilities along with descriptions as <name, decription> pairs.
    * The names are the human-readable name of the utility/service, typically the utility's class
    * name without the "ossim" prefix nor "Util" suffix, all lowercase.
    */
   virtual void getCapabilities(std::map<std::string, std::string>& capabilities) const = 0;
   virtual std::map<std::string, std::string> getCapabilities() const = 0;
};

#endif

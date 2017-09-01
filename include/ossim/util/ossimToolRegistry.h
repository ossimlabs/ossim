//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimUtilityManager_HEADER
#define ossimUtilityManager_HEADER

#include <ossim/base/ossimFactoryListInterface.h>
#include <ossim/util/ossimToolFactoryBase.h>
#include <map>
#include <string>

class ossimTool;

/**
 * Registry of all utility factories. Presently only one factory exists, ossimUtilityFactory, but
 * eventually plugins can provide their own utilities and corresponding factories.
 */
class OSSIMDLLEXPORT ossimToolRegistry : public ossimToolFactoryBase,
                                           public ossimFactoryListInterface<ossimToolFactoryBase, ossimTool>
{
public:
   ossimToolRegistry();
   ~ossimToolRegistry();

   static ossimToolRegistry* instance();

   /**
    * Initializes the utility factory. Returns TRUE if successful.
    * @note Throws ossimException on error.
    */
   virtual bool initialize();

   /**
    * Appends map with available utilities along with descriptions as <name, decription> pairs.
    * The names are the human-readable name of the utility/service, typically the utility's class
    * name without the "ossim" prefix nor "Util" suffix, all lowercase.
    */
   virtual void getCapabilities(std::map<std::string, std::string>& capabilities) const;
   virtual std::map<std::string, std::string> getCapabilities() const;

   virtual ossimTool* createTool(const std::string& typeName) const;

   virtual ossimString getClassName() const { return "ossimToolRegistry"; }

   virtual void getTypeNameList(std::vector<ossimString>& typeList) const;

private:
   static ossimToolRegistry* s_instance;
};

#endif

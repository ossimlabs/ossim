//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimUtilityManager_HEADER
#define ossimUtilityManager_HEADER

#include <ossim/base/ossimFactoryListInterface.h>
#include <ossim/util/ossimUtilityFactoryBase.h>
#include <map>
#include <string>

class ossimUtility;

class OSSIMDLLEXPORT ossimUtilityRegistry : public ossimUtilityFactoryBase,
                                           public ossimFactoryListInterface<ossimUtilityFactoryBase, ossimUtility>
{
public:
   ossimUtilityRegistry();
   ~ossimUtilityRegistry();

   static ossimUtilityRegistry* instance();

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

   virtual ossimUtility* createUtility(const std::string& typeName) const;

   virtual ossimString getClassName() const { return "ossimUtilityManager"; }

   virtual void getTypeNameList(std::vector<ossimString>& typeList) const;

private:
   static ossimUtilityRegistry* s_instance;
};

#endif

#ifndef ossimElevationDatabaseRegistry_HEADER
#define ossimElevationDatabaseRegistry_HEADER
#include <ossim/elevation/ossimElevationDatabaseFactoryBase.h>
#include <ossim/elevation/ossimElevationDatabase.h>
#include <ossim/base/ossimFactoryListInterface.h>

/**
*
* This is the elevation database registry.  
* @see ossimElevationDatabaseFactory for more details on the factories
* supported within the ossim core.
*
*/
class OSSIM_DLL ossimElevationDatabaseRegistry : public ossimFactoryListInterface<ossimElevationDatabaseFactoryBase, ossimElevationDatabase>

{
public:
   ossimElevationDatabaseRegistry()
   {
      m_instance = 0;
   }
   static ossimElevationDatabaseRegistry* instance();
   void getTypeNameList(std::vector<ossimString>& typeList) const 
   {
      getAllTypeNamesFromRegistry(typeList);
   }
   /**
   * @param typeName is the type name of the database you wish to
   *        create
   * @return the elevation database or null otherwise
   */
   ossimElevationDatabase* createDatabase(const ossimString& typeName)const;
   
   /**
   * @param kwl kewyord list that has state information about how to
   *        create a database
   * @param prefix prefix key to use when loading the state information.
   * @return the elevation database or null otherwise
   */
   ossimElevationDatabase* createDatabase(const ossimKeywordlist& kwl,
                                          const char* prefix=0)const;

   /**
   * @param connectionString used to determine which database can support
   *        that connection.
   * @return the elevation database or null otherwise
   */
   ossimElevationDatabase* open(const ossimString& connectionString);

protected:
   static ossimElevationDatabaseRegistry* m_instance;
};
#endif

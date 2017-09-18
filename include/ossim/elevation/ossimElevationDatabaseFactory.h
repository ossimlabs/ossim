#ifndef ossimElevationDatabaseFactory_HEADER
#define ossimElevationDatabaseFactory_HEADER
#include "ossimElevationDatabaseFactoryBase.h"

/**
* This is our default elevation factory in the ossim core.  It supports
* general_raster, general imagery, DTED and SRTM directory layouts.
*
* Example DTED Keywordlist:
* @code
* elevation_manager.elevation_source1.type: dted_directory
* elevation_manager.elevation_source1.connection_string: /data/elevation/dted/level0
* elevation_manager.elevation_source1.min_open_cells: 5
* elevation_manager.elevation_source1.max_open_cells: 25
* elevation_manager.elevation_source1.enabled: true
* elevation_manager.elevation_source1.memory_map_cells: true
* @endCode
* Note:  only use memory_map_cells to true if they are small files.  DTED0 is 
* small so we set it to true.
* Where the dted file tree has the form: <rootdir>/w056/n49.dt0
*
* Example General Raster directory:
* @code
* elevation_manager.elevation_source2.type: general_raster_directory
* elevation_manager.elevation_source2.connection_string: /data/tutorial/data/elevation/general_raster_pk_90m
* elevation_manager.elevation_source2.min_open_cells: 25
* elevation_manager.elevation_source2.max_open_cells: 50 
* elevation_manager.elevation_source2.enabled: false 
* elevation_manager.elevation_source2.memory_map_cells: false
* @endCode
* where the directory is a flat directory with general raster files
*
* Example SRTM directory:
* @code
* elevation_manager.elevation_source10.type: srtm_directory
* elevation_manager.elevation_source10.connection_string: /Volumes/DataDrive/data/elevation/srtm1/
* elevation_manager.elevation_source10.min_open_cells: 25
* elevation_manager.elevation_source10.max_open_cells: 50 
* elevation_manager.elevation_source10.memory_map_cells: false
* elevation_manager.elevation_source10.enabled: true
* @endCode
* where the directory has files of the form N39W112.hgt and are 1x1 degree
* cells
*
* Example image directory format:
* @code
* elevation_manager.elevation_source20.type: image_directory
* elevation_manager.elevation_source20.connection_string: /data/elevation/image
* elevation_manager.elevation_source20.min_open_cells: 25
* elevation_manager.elevation_source20.max_open_cells: 50 
* elevation_manager.elevation_source20.memory_map_cells: false
* elevation_manager.elevation_source20.enabled: false 
* @endCode
* 
* where the directory can have any image the ossim supports and is a flat directory
*/
class OSSIM_DLL ossimElevationDatabaseFactory : public ossimElevationDatabaseFactoryBase
{
public:
   ossimElevationDatabaseFactory()
   {
      m_instance = this;
   }

   /**
   * @return a singleton instance of this factory
   */
   static ossimElevationDatabaseFactory* instance();
   
   /**
   * @param typeName is the type name of the database you wish to
   *        create.
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
   virtual ossimElevationDatabase* open(const ossimString& connectionString)const;

   /**
   * @param typeList appends to the typeList all the type names this factory
   * supports
   */
   virtual void getTypeNameList(std::vector<ossimString>& typeList)const;

protected:
   static ossimElevationDatabaseFactory* m_instance;
};
#endif

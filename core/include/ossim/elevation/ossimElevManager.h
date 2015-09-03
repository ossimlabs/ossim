//*****************************************************************************
// FILE: ossimElevManager.h
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains declaration of class ossimElevManager. This object provides a
//   single interface to an imaging chain for accessing multiple elevation
//   sources. This object owns one or more elevation sources in an ordered
//   list. When queried for an elevation at a particular point, it searches
//   the available sources for the best result, instantiating new sources if
//   necessary.
//
// SOFTWARE HISTORY:
//>
//   13Apr2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
#ifndef ossimElevManager_HEADER
#define ossimElevManager_HEADER

#include <vector>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/elevation/ossimElevSource.h>
#include <ossim/elevation/ossimElevationDatabase.h>
#include <OpenThreads/ReadWriteMutex>

class OSSIM_DLL ossimElevManager : public ossimElevSource
{
public: 
   typedef std::vector<ossimRefPtr<ossimElevationDatabase> > ElevationDatabaseListType;
   
   class OSSIM_DLL ConnectionStringVisitor : public ossimVisitor
   {
   public:
      ConnectionStringVisitor(const ossimString& value);
      virtual ossimRefPtr<ossimVisitor> dup()const{return new ConnectionStringVisitor(*this);}
      const ossimString& getConnectionString()const{return m_connectionString;}
      virtual void visit(ossimObject* obj);
      ossimElevationDatabase* getElevationDatabase(){return m_database.get();}
      
   protected:
      ossimString m_connectionString;
      ossimRefPtr<ossimElevationDatabase> m_database;
   };
   
   virtual ~ossimElevManager();
   
   /**
    * METHOD: instance()
    * Implements singelton pattern
    */
   static ossimElevManager* instance();
   
   virtual double getHeightAboveEllipsoid(const ossimGpt& gpt);
   virtual double getHeightAboveMSL(const ossimGpt& gpt);
   virtual bool pointHasCoverage(const ossimGpt&) const;

   /**
    * Returns the mean post spacing (in meters) for the highest resolution DEM in the list or NaN
    * if no DEMs have been loaded. Recommended to perform a getHeight() call for a central
    * ground pt first to load the relevant cells before calling this method.
    */
   virtual double getMeanSpacingMeters() const;

   virtual bool getAccuracyInfo(ossimElevationAccuracyInfo& info, const ossimGpt& gpt) const;
  
   ossim_uint32 getNumberOfElevationDatabases()const
   {
      return (ossim_uint32) m_dbRoundRobin[0].size();
   }
   ossimElevationDatabase* getElevationDatabase(ossim_uint32 idx)
   {
      return m_dbRoundRobin[0][idx].get();
   }
   const ossimElevationDatabase* getElevationDatabase(ossim_uint32 idx)const
   {
      return m_dbRoundRobin[0][idx].get();
   }
   void addDatabase(ossimElevationDatabase* database);
   bool loadElevationPath(const ossimFilename& path);
   
   void setDefaultHeightAboveEllipsoid(double meters) {m_defaultHeightAboveEllipsoid=meters;}
   void setElevationOffset(double meters) {m_elevationOffset=meters;}
   double getElevationOffset() const { return m_elevationOffset; }
   
   void getOpenCellList(std::vector<ossimFilename>& list) const;

   /**
    * @brief Gets a list of elevation cells needed to cover bounding box.
    * @param connectionString Typically elevation repository, e.g.:
    * "/data1/elevation/srtm/1arc"
    * @param minLat Minimum latitude of bounding box.
    * @param minLon Minimum longitude of bounding box.
    * @param maxLat Maximum latitude of bounding box.
    * @param maxLon Maximum longitude of bounding box.
    * @param cells Initialized by this.
    * @param maxNumberOfCells Value of 0 indicates return as many as you can.  Any positive
    *        number will only return that number of cells.
    */
   void getCellsForBounds( const std::string& connectionString,
                           const ossim_float64& minLat,
                           const ossim_float64& minLon,
                           const ossim_float64& maxLat,
                           const ossim_float64& maxLon,
                           std::vector<std::string>& cells,
                           ossim_uint32 maxNumberOfCells=0 );

   /**
    * @brief Gets a list of elevation cells needed to cover bounding box.
    *
    * This implementation sorts elevation repositories by resolution, best
    * first, then searches cells for bounds. Search is stopped on the first
    * repository that has cells.
    * 
    * @param minLat Minimum latitude of bounding box.
    * @param minLon Minimum longitude of bounding box.
    * @param maxLat Maximum latitude of bounding box.
    * @param maxLon Maximum longitude of bounding box.
    * @param cells Initialized by this.
    * @param maxNumberOfCells Value of 0 indicates return as many as you can.  Any positive
    *        number will only return that number of cells.
    */
   void getCellsForBounds( const ossim_float64& minLat,
                           const ossim_float64& minLon,
                           const ossim_float64& maxLat,
                           const ossim_float64& maxLon,
                           std::vector<std::string>& cells,
                           ossim_uint32 maxNumberOfCells=0 );
   void getCellsForBounds( const ossimGrect& bounds,
                           std::vector<std::string>& cells,
                           ossim_uint32 maxNumberOfCells=0 );

   void setUseGeoidIfNullFlag(bool flag) { m_useGeoidIfNullFlag = flag; }
   bool getUseGeoidIfNullFlag() const { return m_useGeoidIfNullFlag; }
   void setRoundRobinMaxSize(ossim_uint32 size);

   void clear();
   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0) const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);
   
   virtual void accept(ossimVisitor& visitor);
   
   virtual std::ostream& print(std::ostream& out) const;

protected:
   virtual ossimObject* dup() const { return 0; } // required to implement but not permitted to call

   ossimElevManager();
   void loadStandardElevationPaths();

   ElevationDatabaseListType& getNextElevDbList() const; // for multithreading
   
   //static ossimElevManager* m_instance;
   mutable std::vector<ElevationDatabaseListType> m_dbRoundRobin;
   ossim_uint32 m_maxRoundRobinSize;
   ossim_float64 m_defaultHeightAboveEllipsoid;
   ossim_float64 m_elevationOffset;
   
   // if an elevation is returned that's null for ellipsoid then use the geoid manager to calculate a shift
   bool m_useGeoidIfNullFlag;
   
   mutable ossim_uint32 m_currentDatabaseIdx;
   
   /**
    * I have tried the readwrite lock interfaces but have found it unstable.  I am using the standard Mutex
    * and it seems to be much more stable across all platforms.  More testing needs to occur for the ReadWriteMutex.
    * For now we will use Mutex.
    */
   mutable OpenThreads::Mutex m_mutex;
};

#endif

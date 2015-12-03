#ifndef ossimElevationCellDatabase_HEADER
#define ossimElevationCellDatabase_HEADER 1
#include <ossim/elevation/ossimElevationDatabase.h>

class OSSIM_DLL ossimElevationCellDatabase : public ossimElevationDatabase
{
public:
   struct CellInfo : ossimReferenced
   {
      CellInfo(ossim_uint64 id, ossimElevCellHandler* handler = 0)
         :ossimReferenced(),
          m_id(id),
          m_handler(handler),
          m_timestamp(0)
      {
            m_timestamp = ossimTimer::instance()->tick();
      }
      CellInfo(const CellInfo& src)
         :ossimReferenced(src),
          m_id(src.m_id),
          m_handler(src.m_handler),
          m_timestamp(src.m_timestamp)
      {
      }
      CellInfo()
         :ossimReferenced(),
          m_id(0),
          m_handler(0),
          m_timestamp(0)
      {
      }
      const CellInfo& operator =(const CellInfo& src)
      {
         if (this != &src)
         {
            m_id = src.m_id;
            m_handler = src.m_handler;
            m_timestamp = src.m_timestamp;
         }
         return *this;
      }
      void updateTimestamp()
      {
         m_timestamp = ossimTimer::instance()->tick();
      }
      ossim_uint64 id()const
      {
         return m_id;
      }
      ossim_uint64                      m_id;
      ossimRefPtr<ossimElevCellHandler> m_handler;
      ossimTimer::Timer_t               m_timestamp;
   };

   typedef std::map<ossim_uint64, ossimRefPtr<CellInfo> > CellMap;
   
   ossimElevationCellDatabase()
      :ossimElevationDatabase(),
      m_minOpenCells(5),
      m_maxOpenCells(10),
      m_memoryMapCellsFlag(false)
   {
   }
   ossimElevationCellDatabase(const ossimElevationCellDatabase& src)
      :ossimElevationDatabase(src),
      m_minOpenCells(src.m_minOpenCells),
      m_maxOpenCells(src.m_maxOpenCells),
      m_cacheMap(src.m_cacheMap),
      m_memoryMapCellsFlag(src.m_memoryMapCellsFlag)
   {
   }

   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;
   virtual ossim_uint32 getMinOpenCells()const
   {
      return m_minOpenCells;
   }
   virtual ossim_uint32 getMaxOpenCells()const
   {
      return m_maxOpenCells;
   }
   virtual void setMinMaxOpenCells(ossim_uint64 minCellCount,
                                   ossim_uint64 maxCellCount)
   {
      m_minOpenCells = minCellCount;
      m_maxOpenCells = maxCellCount;
   }
   virtual bool getMemoryMapCellsFlag()const
   {
      return m_memoryMapCellsFlag;
   }
   virtual void setMemoryMapCellsFlag(bool flag)
   {
      m_memoryMapCellsFlag = flag;
   }

   virtual void getOpenCellList(std::vector<ossimFilename>& list) const;

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
   void getCellsForBounds( const ossim_float64& minLat,
                           const ossim_float64& minLon,
                           const ossim_float64& maxLat,
                           const ossim_float64& maxLon,
                           std::vector<std::string>& cells,
                           ossim_uint32 maxNumberOfCells=0 );

   virtual ossim_uint64 createId(const ossimGpt& /* pt */)const
   {
      return 0;
   }
   virtual ossimRefPtr<ossimElevCellHandler> getOrCreateCellHandler(const ossimGpt& gpt);

   virtual std::ostream& print(std::ostream& out) const;

protected:
   virtual ossimRefPtr<ossimElevCellHandler> createCell(const ossimGpt& /* gpt */)
   {
      return 0;
   }
   virtual void remove(ossim_uint64 id)
   {
      CellMap::iterator iter = m_cacheMap.find(id);
      if(iter != m_cacheMap.end())
      {
         m_cacheMap.erase(iter);
      }
   }
   void flushCacheToMinOpenCells()
   {
      // lets flush the cache from least recently used to recent.
      //
      CellMap sortedMap;
      CellMap::iterator iter = m_cacheMap.begin();
      
      while(iter != m_cacheMap.end())
      {
         sortedMap.insert(std::make_pair(iter->second->m_timestamp, iter->second));
         ++iter;
      }
      
      iter = sortedMap.begin();
      while((iter!=sortedMap.end())&&(m_cacheMap.size() > m_minOpenCells))
      {
         remove(iter->second->id());
         ++iter;
      }
   }
   
   ossim_uint32               m_minOpenCells;
   ossim_uint32               m_maxOpenCells;
   mutable OpenThreads::Mutex m_cacheMapMutex;
   CellMap                    m_cacheMap;
   ossim_uint32               m_memoryMapCellsFlag;
   
   TYPE_DATA;
};

#endif /* #ifndef ossimElevationCellDatabase_HEADER */

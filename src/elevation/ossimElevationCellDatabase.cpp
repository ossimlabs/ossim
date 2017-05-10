#include <ossim/elevation/ossimElevationCellDatabase.h>

RTTI_DEF1(ossimElevationCellDatabase, "ossimElevationCellDatabase", ossimElevationDatabase);

void ossimElevationCellDatabase::getOpenCellList(std::vector<ossimFilename>& list) const
{
   CellMap::const_iterator iter = m_cacheMap.begin();

   while(iter!=m_cacheMap.end())
   {
      if ( iter->second->m_handler.valid() )
      {
         list.push_back(iter->second->m_handler->getFilename());
      }
      ++iter;
   }

}

void ossimElevationCellDatabase::getCellsForBounds( const ossim_float64& minLat,
                                                    const ossim_float64& minLon,
                                                    const ossim_float64& maxLat,
                                                    const ossim_float64& maxLon,
                                                    std::vector<ossimFilename>& cells,
                                                    ossim_uint32 maxNumberOfCells )
{
   const ossim_float64 SEARCH_SPACING = 0.1;
   ossimGpt gpt(0.0, 0.0, 0.0);
   ossimFilename f;
   ossim_uint32 limitNumberOfCells = maxNumberOfCells>0?maxNumberOfCells:9999999999;
   // Latitude loop:
   ossim_float64 lat = minLat;
   while ( (lat <= maxLat) &&(cells.size() < limitNumberOfCells))
   {
      gpt.lat = lat;

      // Longitude loop:
      ossim_float64 lon = minLon;
      while ( (lon <= maxLon ) &&(cells.size()< limitNumberOfCells))
      {
         gpt.lon = lon;

         ossimRefPtr<ossimElevCellHandler> h = getOrCreateCellHandler( gpt );
         if ( h.valid() )
         {
            // Get the file name:
            f = h->getFilename();
            
            if ( f.size() )
            {
               // See if it's already in the list, i.e. duplicate:
               std::vector<ossimFilename>::const_iterator i = cells.begin();
               while ( i != cells.end() )
               {
                  if ( f == (*i) )
                  {
                     break;
                  }
                  ++i;
               }
               
               if ( i == cells.end() )
               {
                  // Add it to the list:
                  cells.push_back( f );
               }
            }
         }

         if ( lon < maxLon )
         {
            lon = ossim::min<ossim_float64>( (lon+SEARCH_SPACING), maxLon );
         }
         else
         {
            break;
         }
         
      } // Matches: while ( lon <= maxLon )

      if ( lat < maxLat )
      {
         lat = ossim::min<ossim_float64>( (lat+SEARCH_SPACING), maxLat );
      }
      else
      {
         break;
      }
      
   } // Matches: while ( lat <= maxLat )
   
} // End: ossimElevationCellDatabase::getCellsForBounds( ... )

/* 
 * New code caches null handlers so that createCell() is not called
 * unnecessarily. drb - 20170509
 */
#if 1 
ossimRefPtr<ossimElevCellHandler> ossimElevationCellDatabase::getOrCreateCellHandler(
   const ossimGpt& gpt)
{
   ossimRefPtr<ossimElevCellHandler> result = 0;

   ossim_uint64 id = createId(gpt);

   m_cacheMapMutex.lock();

   CellMap::iterator iter = m_cacheMap.find(id);
   if(iter != m_cacheMap.end())
   {
      iter->second->updateTimestamp();
      result = iter->second->m_handler.get();
   }
   else
   {
      m_cacheMapMutex.unlock();                                                                  

      result = createCell(gpt);

      m_cacheMapMutex.lock();

      //---
      // Code speed up:
      // Add it to the cache even if it's not valid so this database will not
      // call createCell(...) again.
      //--- 
      m_cacheMap.insert(std::make_pair(id, new CellInfo(id, result.get())));
      
      // Check the map size and purge cells if needed.
      if(m_cacheMap.size() > m_maxOpenCells)
      {
         flushCacheToMinOpenCells();
      }
   }

   m_cacheMapMutex.unlock();
              
   return result;
}
#else
ossimRefPtr<ossimElevCellHandler> ossimElevationCellDatabase::getOrCreateCellHandler(const ossimGpt& gpt)
{
  ossimRefPtr<ossimElevCellHandler> result = 0;
  ossim_uint64 id = createId(gpt);
  
  {
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_cacheMapMutex);
    CellMap::iterator iter = m_cacheMap.find(id);
    if(iter != m_cacheMap.end())
    {
      iter->second->updateTimestamp();
      result = iter->second->m_handler.get();
      
      return result.get();
    }
  }
  
  result = createCell(gpt);
  
  {
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_cacheMapMutex);
    if(result.valid())
    {
      m_cacheMap.insert(std::make_pair(id, new CellInfo(id, result.get())));

      // Check the map size and purge cells if needed.
      if(m_cacheMap.size() > m_maxOpenCells)
      {
         flushCacheToMinOpenCells();
      }
    }
  }

  return result;
}
#endif

bool ossimElevationCellDatabase::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimString minOpenCells = kwl.find(prefix, "min_open_cells");
   ossimString maxOpenCells = kwl.find(prefix, "max_open_cells");
   if(!minOpenCells.empty()&&
      !maxOpenCells.empty())
   {
      m_minOpenCells = minOpenCells.toUInt32();
      m_maxOpenCells = maxOpenCells.toUInt32();
      if(m_maxOpenCells < m_minOpenCells)
      {
         std::swap(m_minOpenCells, m_maxOpenCells);
      }
   }
   ossimString memoryMapCellsFlag = kwl.find(prefix, "memory_map_cells");
   if(!memoryMapCellsFlag.empty())
   {
      m_memoryMapCellsFlag = memoryMapCellsFlag.toBool();
   }
   return ossimElevationDatabase::loadState(kwl, prefix);
}

bool ossimElevationCellDatabase::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix, "memory_map_cells", m_memoryMapCellsFlag, true);
   kwl.add(prefix, "min_open_cells", m_minOpenCells, true);
   kwl.add(prefix, "max_open_cells", m_maxOpenCells, true);

   if(m_geoid.valid())
   {
      kwl.add(prefix, "geoid.type", m_geoid->getShortName(), true);
   }

   return ossimElevationDatabase::saveState(kwl, prefix);
}

std::ostream& ossimElevationCellDatabase::print(ostream& out) const
{
   ossimKeywordlist kwl;
   saveState(kwl);
   out << "\nossimElevationCellDatabase @ "<< (ossim_uint64) this << kwl << ends;
   return out;
}



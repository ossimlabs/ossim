//**************************************************************************
// FILE: ossimElevManager.cpp
//
// License: MIT
//
// DESCRIPTION:
//   Contains implementation of class ossimElevManager. This object 
//   provides a single interface for accessing multiple elevation
//   sources. This object owns one or more elevation sources in an ordered
//   list. When queried for an elevation at a particular point, it searches
//   the available sources for the best result, instantiating new sources if
//   necessary.
//
// SOFTWARE HISTORY:
//>
//   23Apr2001  Oscar Kramer
//              Initial coding.
//<
//**************************************************************************
// $Id$

#include <ossim/elevation/ossimElevManager.h>
#include <ossim/base/ossimEnvironmentUtility.h>
#include <ossim/elevation/ossimElevationCellDatabase.h>
#include <ossim/base/ossimDirectory.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimGeoidManager.h>
#include <ossim/elevation/ossimElevationDatabaseRegistry.h>
#include <ossim/base/ossimKeywordNames.h>
#include <algorithm>

ossimElevManager* ossimElevManager::m_instance = 0;
static ossimTrace traceDebug("ossimElevManager:debug");

using namespace std;

//---
// For std::sort of ElevationDatabaseListType
// e.g.: std::sort( dbList.begin(), dbList.end(), dbSort );
//---
bool dbSort(ossimRefPtr<ossimElevationDatabase> a, ossimRefPtr<ossimElevationDatabase> b)
{
   bool result = false;
   if ( a.valid() && b.valid() )
   {
      result = ( a->getMeanSpacingMeters() < b->getMeanSpacingMeters() );
   }
   return result;
}

ossimElevManager::ConnectionStringVisitor::ConnectionStringVisitor( const ossimString& value )
   :
   m_connectionString(value),
   m_database(0)
{}

void ossimElevManager::ConnectionStringVisitor::visit(ossimObject* obj)
{
   if(!hasVisited(obj))
   {
      ossimElevationDatabase* databsase = dynamic_cast<ossimElevationDatabase*>(obj);
      if(databsase)
      {
         if(m_connectionString == databsase->getConnectionString())
         {
            m_database = databsase;
            m_stopTraversalFlag = true;
         }
      }
   }
}

ossimElevManager* ossimElevManager::instance()
{
   if(!m_instance)
   {
      new ossimElevManager();
      ossimElevationDatabaseRegistry::instance();
   }
   return m_instance;
}

ossimElevManager::ossimElevManager()
   :ossimElevSource(),
    m_maxRoundRobinSize(1),
    m_defaultHeightAboveEllipsoid(ossim::nan()),
    m_elevationOffset(ossim::nan()),
    m_useGeoidIfNullFlag(false),
    m_useStandardPaths(false),
    m_currentDatabaseIdx(0),
    m_mutex()
{
   m_instance = this;
   //---
   // Auto load removed to avoid un-wanted directory scanning.
   // Use ossim preferences.  drb - 28 March 2016.
   //---
   // loadStandardElevationPaths();
}

ossimElevManager::~ossimElevManager()
{
   clear();
}

double ossimElevManager::getHeightAboveEllipsoid(const ossimGpt& gpt)
{
   double result = ossim::nan();

   if (!isSourceEnabled())
      return result;

   ElevationDatabaseListType& elevDbList = getNextElevDbList();
   for (ossim_uint32 idx = 0; (idx < elevDbList.size()) && ossim::isnan(result); ++idx)
   {
      result = elevDbList[idx]->getHeightAboveEllipsoid(gpt);
   }

   if (ossim::isnan(result))
   {
      // No elevation value was returned from the database, so try next best alternatives depending
      // on ossim_preferences settings. Priority goes to default ellipsoid height if available:
      if (!ossim::isnan(m_defaultHeightAboveEllipsoid))
      {
         result = m_defaultHeightAboveEllipsoid;
      }
      else if (m_useGeoidIfNullFlag)
      {
         result = ossimGeoidManager::instance()->offsetFromEllipsoid(gpt);
      }
   }

   // Next, ossim_preferences may have indicated an elevation offset to use (top of trees, error
   // bias, etc):
   if (!ossim::isnan(m_elevationOffset) && !ossim::isnan(result))
      result += m_elevationOffset;

   return result;
}

double ossimElevManager::getHeightAboveMSL(const ossimGpt& gpt)
{
   double result = ossim::nan();

   if (!isSourceEnabled())
      return result;

   ElevationDatabaseListType& elevDbList = getNextElevDbList();
   for (ossim_uint32 idx = 0; (idx < elevDbList.size()) && ossim::isnan(result); ++idx)
   {
      result = elevDbList[idx]->getHeightAboveMSL(gpt);
   }

   if (ossim::isnan(result) && m_useGeoidIfNullFlag)
   {
      // No elevation value was returned from the database, so try next best alternatives depending
      // on ossim_preferences settings. First default to height at MSL itself:
      result = 0.0; // MSL
      if (!ossim::isnan(m_defaultHeightAboveEllipsoid))
      {
         // Use the default height above ellipsoid corrected for best guess of MSL above ellipsoid
         // (i.e., the geoid):
         double dh = ossimGeoidManager::instance()->offsetFromEllipsoid(gpt);
         if (!ossim::isnan(dh))
            result = m_defaultHeightAboveEllipsoid - dh;
      }
   }

   // ossim_preferences may have indicated an elevation offset to use (top of trees, error bias, etc)
   if (!ossim::isnan(result) && (!ossim::isnan(m_elevationOffset)))
      result += m_elevationOffset;

   return result;
}

void ossimElevManager::loadStandardElevationPaths()
{
   if (!m_useStandardPaths)
      return;

   ossimFilename userDir    = ossimEnvironmentUtility::instance()->getUserOssimSupportDir();
   ossimFilename installDir = ossimEnvironmentUtility::instance()->getInstalledOssimSupportDir();
   
   userDir = userDir.dirCat("elevation");
   installDir = installDir.dirCat("elevation");
   
   loadElevationPath(userDir);
   loadElevationPath(installDir);
   
   ossimString paths = ossimEnvironmentUtility::instance()->getEnvironmentVariable("OSSIM_ELEVATION_PATH");
   std::vector<ossimString> pathArray;
   ossimString pathSeparator = ":";
#if defined(WIN32) && !defined(CYGWIN)
   pathSeparator = ";";
#endif
   
   if(!paths.empty())
   {
      paths.split(pathArray, pathSeparator);
      if(!pathArray.empty())
      {
         ossim_uint32 idx = 0;
         for(idx = 0; idx < pathArray.size(); ++idx)
         {
            ossimFilename file(pathArray[idx]);
            
            if(file.exists())
            {
               loadElevationPath(file);
            }
         }
      }
   }
}

bool ossimElevManager::loadElevationPath(const ossimFilename& path, bool set_as_first)
{
   bool result = false;
   ossimElevationDatabase* database = ossimElevationDatabaseRegistry::instance()->open(path);
   
   if(!database && path.isDir())
   {
      ossimDirectory dir;
      
      if(dir.open(path))
      {
         ossimFilename file;
         dir.getFirst(file, ossimDirectory::OSSIM_DIR_DIRS);
         do
         {
            database = ossimElevationDatabaseRegistry::instance()->open(file);
            if(database)
            {
               result = true;
               addDatabase(database, set_as_first);
            }
         }while(dir.getNext(file));
      }
   }
   else if(database)
   {
      result = true;
      addDatabase(database, set_as_first);
   }
   
   return result;
}

ossimRefPtr<ossimElevationDatabase> ossimElevManager::getElevationDatabaseForPoint(
   const ossimGpt& gpt )
{
   ossimRefPtr<ossimElevationDatabase> db = 0;

   if ( m_dbRoundRobin.size() )
   {
      // "m_dbRoundRobin[0]" is a std::vector<ossimRefPtr<ossimElevationDatabase> >
      const auto& cv = m_dbRoundRobin[0];
      for ( auto&& i : cv )
      {
         if ( i.valid() )
         {
            if ( i->pointHasCoverage( gpt ) )
            {
               db = i;
               break;
            }
         }
      }
   }

   return db;
}

ossimRefPtr<ossimElevCellHandler> ossimElevManager::getCellForPoint( const ossimGpt& gpt )
{
   ossimRefPtr<ossimElevCellHandler> cell = 0;

   if ( m_dbRoundRobin.size() )
   {
      // "elevDbList" is a std::vector<ossimRefPtr<ossimElevationDatabase> >
      ElevationDatabaseListType elevDbList = m_dbRoundRobin[0];
      for ( auto&& i : elevDbList )
      {
         if ( i.valid() )
         {
            if ( i->pointHasCoverage( gpt ) )
            {
               // See if it's a cell based db.
               ossimElevationCellDatabase* cellDb =
                  dynamic_cast<ossimElevationCellDatabase*>( i.get() );
               if ( cellDb )
               {
                  cell = cellDb->getOrCreateCellHandler( gpt );
                  if ( cell )
                  {
                     break;
                  }
               }
            }
         }
      }
   }

   return cell;
}

void ossimElevManager::getCellFilenameForPoint(const ossimGpt& gpt, ossimFilename& file)
{
   ossimRefPtr<ossimElevCellHandler> elevCell = getCellForPoint( gpt );
   if ( elevCell.valid() )
   {
      file = elevCell->getFilename();
   }
}

void ossimElevManager::getOpenCellList(std::vector<ossimFilename>& list) const
{
   if ( m_dbRoundRobin.size() )
   {
      ElevationDatabaseListType& elevDbList = m_dbRoundRobin[0];
      for(ossim_uint32 idx = 0; idx < elevDbList.size(); ++idx)
      {
         // If this is a cell based database get the files:
         ossimRefPtr<const ossimElevationCellDatabase> db =
            dynamic_cast<const ossimElevationCellDatabase*>( elevDbList[idx].get() );
         if ( db.valid() )
         {
            db->getOpenCellList(list);
         }
      }
   }
}

void ossimElevManager::getCellsForBounds( const std::string& connectionString,
                                          const ossim_float64& minLat,
                                          const ossim_float64& minLon,
                                          const ossim_float64& maxLat,
                                          const ossim_float64& maxLon,
                                          std::vector<ossimFilename>& cells,
                                          ossim_uint32 maxNumberOfCells)
{
   ossimRefPtr<ossimElevationCellDatabase> cellDatabase = 0;

   // See if the connectionString is already opened:
   ossimString os = connectionString;
   ossimElevManager::ConnectionStringVisitor visitor( os );
   accept( visitor );
   cellDatabase = dynamic_cast<ossimElevationCellDatabase*>( visitor.getElevationDatabase() );

   if ( cellDatabase.valid() == false )
   {
      // Try to open it:
      ossimRefPtr<ossimElevationDatabase> db =
         ossimElevationDatabaseRegistry::instance()->open( ossimString(connectionString) );

      if ( db.valid() )
      {
         cellDatabase = dynamic_cast<ossimElevationCellDatabase*>( db.get() );
      }
   }

   // Lastly get the files:
   if ( cellDatabase.valid() )
   {
      cellDatabase->getCellsForBounds( minLat, minLon, maxLat, maxLon, cells, maxNumberOfCells );
   }
   else
   {
      // Bummer:
      cells.clear();
   }
}

void ossimElevManager::getCellsForBounds( const ossim_float64& minLat,
                                          const ossim_float64& minLon,
                                          const ossim_float64& maxLat,
                                          const ossim_float64& maxLon,
                                          std::vector<ossimFilename>& cells,
                                          ossim_uint32 maxNumberOfCells )
{
   //TODO: Presently incrementing by 0.1 deg. If an elev cell
   // is smaller than this, it may be missed. Need to generalize to support arbitrary cell sizes.

   //TODO: This method relies on the caching of open cells. If the bounds are too large too permit
   // all cells to remain open, this method will incorrectly return a subset of all cells providing
   // coverage.

   // Ping the collection of databases for elevation values at regular intervals inside the bounds.
   // This will autoload the best cells:
   cells.clear();
   const ossim_float64 DELTA_DEG = 0.1; // degree
   ossimGpt gpt;
   for (gpt.lat=minLat; gpt.lat<=maxLat; gpt.lat+=DELTA_DEG)
   {
      for (gpt.lon=minLon; gpt.lon<=maxLon; gpt.lon+=DELTA_DEG)
      {
         getHeightAboveMSL(gpt);
      }
   }

   // Convert filename list to string list (why are they different)?
   ossim_uint32 limitCells = maxNumberOfCells>0?maxNumberOfCells:static_cast<ossim_uint32>(9999999999);
   std::vector<ossimFilename> open_cells;
   getOpenCellList(open_cells);
   std::vector<ossimFilename>::iterator iter = open_cells.begin();
   while ((iter != open_cells.end()) && (cells.size() < limitCells))
   {
      cells.push_back(*iter);
      ++iter;
   }
}


void ossimElevManager::getCellsForBounds( const ossimGrect& bbox,
                                          std::vector<ossimFilename>& cells,
                                          ossim_uint32 maxCells)
{
   getCellsForBounds(bbox.lr().lat, bbox.ul().lon, bbox.ul().lat, bbox.lr().lon, cells, maxCells);
}

void ossimElevManager::clear()
{
   std::vector<ElevationDatabaseListType>::iterator i = m_dbRoundRobin.begin();
   while ( i != m_dbRoundRobin.end() )
   {
      i->clear();
      ++i;
   }
}

void ossimElevManager::accept(ossimVisitor& visitor)
{
   std::vector<ElevationDatabaseListType>::iterator rri = m_dbRoundRobin.begin();
   while ( rri != m_dbRoundRobin.end() )
   {
      ElevationDatabaseListType& elevDbList = *rri;

      ossimElevManager::ElevationDatabaseListType::iterator i = elevDbList.begin();
      while ( i != elevDbList.end() )
      {
         if ( (*i).valid() )
         {
            (*i)->accept( visitor );

            if ( visitor.stopTraversal() )
            {
               break;
            }
         }
         ++i;
      }
      ++rri;
   }
}

bool ossimElevManager::getAccuracyInfo(ossimElevationAccuracyInfo& info, const ossimGpt& gpt) const
{
   ElevationDatabaseListType& elevDbList = getNextElevDbList();
   for(ossim_uint32 idx = 0;(idx < elevDbList.size()); ++idx)
   {
      if(elevDbList[idx]->getAccuracyInfo(info, gpt))
      {
         return true;
      }
   }

   return false;
}

bool ossimElevManager::pointHasCoverage(const ossimGpt& gpt) const
{
   if ( m_dbRoundRobin.size() )
   {
      ElevationDatabaseListType& elevDbList = m_dbRoundRobin[0];
      for(ossim_uint32 idx = 0;(idx < elevDbList.size()); ++idx)
      {
         if(elevDbList[idx]->pointHasCoverage(gpt))
         {
            return true;
         }
      }
   }
   return false;
}

bool ossimElevManager::saveState(ossimKeywordlist& kwl, const char* prefix) const
{
   kwl.add(prefix, "elevation_offset", m_elevationOffset, true);
   kwl.add(prefix, "default_height_above_ellipsoid", m_defaultHeightAboveEllipsoid, true);
   kwl.add(prefix, "use_geoid_if_null", m_useGeoidIfNullFlag, true);
   kwl.add(prefix, "use_standard_elev_paths", m_useStandardPaths, true);
   kwl.add(prefix, "threads", ossimString::toString(m_maxRoundRobinSize), true);

   return ossimElevSource::saveState(kwl, prefix);
}

/**
 * Method to the load (recreate) the state of an object from a keyword
 * list.  Return true if ok or false on error.
 */
bool ossimElevManager::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
      << "DEBUG ossimElevManager::loadState: Entered..."
      << std::endl;
   }
   if(!ossimElevSource::loadState(kwl, prefix))
   {
      return false;
   }
   ossimString copyPrefix(prefix);
   ossimString elevationOffset = kwl.find(copyPrefix, "elevation_offset");
   ossimString defaultHeightAboveEllipsoid = kwl.find(copyPrefix, "default_height_above_ellipsoid");
   ossimString elevRndRbnSize = kwl.find(copyPrefix, "threads");

   kwl.getBoolKeywordValue(m_useGeoidIfNullFlag, "use_geoid_if_null", copyPrefix.chars());
   kwl.getBoolKeywordValue(m_useStandardPaths, "use_standard_elev_paths", copyPrefix.chars());

   if(!elevationOffset.empty())
      m_elevationOffset = elevationOffset.toDouble();

   if(!defaultHeightAboveEllipsoid.empty())
      m_defaultHeightAboveEllipsoid = defaultHeightAboveEllipsoid.toDouble();

   ossim_uint32 numThreads = 1;
   if(!elevRndRbnSize.empty())
   {
      if (elevRndRbnSize.contains("yes") || elevRndRbnSize.contains("true"))
         numThreads = ossim::getNumberOfThreads();
      else if (elevRndRbnSize.contains("no") || elevRndRbnSize.contains("false"))
         numThreads = 1;
      else
      {
         numThreads = elevRndRbnSize.toUInt32();
         numThreads = numThreads > 0 ? numThreads : 1;
      }
   }
   setRoundRobinMaxSize(numThreads);

   ossimString regExpression =  ossimString("^(") + copyPrefix + "elevation_source[0-9]+.)";
   vector<ossimString> keys = kwl.getSubstringKeyList( regExpression );
   long numberOfSources = (long)keys.size();
   ossim_uint32 offset = (ossim_uint32)(copyPrefix+"elevation_source").size();
   ossim_uint32 idx = 0;
   std::vector<int> theNumberList(numberOfSources);
   for(idx = 0; idx < theNumberList.size();++idx)
   {
      ossimString numberStr(keys[idx].begin() + offset,
                            keys[idx].end());
      theNumberList[idx] = numberStr.toInt();
   }
   std::sort(theNumberList.begin(), theNumberList.end());
   
   for(idx=0;idx < theNumberList.size();++idx)
   {
      ossimString newPrefix = copyPrefix;
      newPrefix += ossimString("elevation_source");
      newPrefix += ossimString::toString(theNumberList[idx]);
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossimElevManager::loadState:"
         << "\nLooking for key:  " << newPrefix
         << std::endl;
      }

      //---
      // Check for enabled key first.  Default, if not found is true for
      // legacy compatibility.
      //---
      bool enabled = true;
      std::string key = newPrefix.string();
      key += ".";
      key += ossimKeywordNames::ENABLED_KW;
      std::string value = kwl.findKey( key );
      if ( value.size() )
      {
         enabled = ossimString(value).toBool();
      }

      if ( enabled )
      {
         // first check if new way is supported
         ossimRefPtr<ossimElevationDatabase> database =
            ossimElevationDatabaseRegistry::instance()->createDatabase(kwl, newPrefix+".");
         if(database.valid())
         {
            if (traceDebug())
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                  << "DEBUG ossimElevManager::loadState:"
                  << "\nadding elevation database:  "
                  << database->getClassName()
                  << ": " << database->getConnectionString()
                  << std::endl;
            }  
            addDatabase(database.get());
         }
         else
         {
            // if not new elevation load verify the old way by
            // looking at the filename
            //
            ossimString fileKey = newPrefix;
            fileKey += ".";
            fileKey += ossimKeywordNames::FILENAME_KW;
            ossimString lookup = kwl.find(prefix, fileKey.c_str());
            if (!lookup.empty())
            {
               loadElevationPath(ossimFilename(lookup));
            } // end if lookup
         }
      }

   } // end for loop

   return true;
}

void ossimElevManager::setRoundRobinMaxSize(ossim_uint32 new_size)
{
   m_maxRoundRobinSize = new_size;

#ifdef DYNAMICALLY_ALLOCATE_ROUND_ROBIN
   // Resize the round robin, preserves at least the first entry if any:
   if (new_size < m_dbRoundRobin.size())
      m_dbRoundRobin.resize(new_size);
#else
   // OLK 02/2015 -- Was hoping to dynamically allocate the round robin as needed but was running
   // into issues, so just allocating full set here (containing empty lists at this point). The
   // heavy lifting is in addDatabase() where copies are made for each entry in the round robin. It
   // would have been best to keep the size of the round robin to an as-needed basis to avoid
   // unnecessary duplicate lists. However, this is only relevant for small apps (like ossim-info)
   // where minimal number of elevation look-ups are done. TODO: Figure out why MT run core dumps
   // when dynamically sizing m_dbRoundRobin.
   m_dbRoundRobin.resize(new_size);
#endif

}

inline ossimElevManager::ElevationDatabaseListType& ossimElevManager::getNextElevDbList() const
{
   std::lock_guard<std::mutex> lock (m_mutex);

   // Quickly grab the DB to be used by this thread and increment DB index to be used by next thread
   // May need to grow the list round robin as it is dynamically set as needed):
   if (m_currentDatabaseIdx >= m_dbRoundRobin.size())
   {
      ossim_uint32 index = m_dbRoundRobin.size();
      m_dbRoundRobin.resize(index+1);

      // An entry existed prior, duplicate its contents for all Db lists in the round robin. Only
      // duplicate as needed beyond what already existed:
      ElevationDatabaseListType::iterator iter = m_dbRoundRobin[0].begin();
      while (iter != m_dbRoundRobin[0].end())
      {
         ossimRefPtr<ossimElevationDatabase> dupDb = (ossimElevationDatabase*) (*iter)->dup();
         m_dbRoundRobin[index].push_back(dupDb);
         ++iter;
      }
   }

   ossim_uint32 index = m_currentDatabaseIdx++;
   if (m_currentDatabaseIdx == m_maxRoundRobinSize)
      m_currentDatabaseIdx = 0;

   return m_dbRoundRobin[index];
}

void ossimElevManager::addDatabase(ossimElevationDatabase* database, bool set_as_first)
{
   if(!database)
      return;

   if (m_dbRoundRobin.empty())
      m_dbRoundRobin.resize(1);

   std::vector<ElevationDatabaseListType>::iterator rri = m_dbRoundRobin.begin();
   if (std::find(rri->begin(), rri->end(), database) == rri->end())
   {
      if (set_as_first)
         rri->insert(rri->begin(), database);
      else
         rri->push_back(database);

      // Populate the parallel lists in the round-robin with duplicates:
      ++rri;
      while ( rri != m_dbRoundRobin.end() )
      {
         ossimRefPtr<ossimElevationDatabase> dupDb = (ossimElevationDatabase*) database->dup();
         if (set_as_first)
            rri->insert(rri->begin(), dupDb);
         else
            rri->push_back(dupDb);
         ++rri;
      }
   }
}

double ossimElevManager::getMeanSpacingMeters() const
{
   double spacing = ossim::nan();
   if ( m_dbRoundRobin.size() )
   {
       ElevationDatabaseListType& elevDbList = m_dbRoundRobin[0];
      for(ossim_uint32 idx = 0;(idx < elevDbList.size()); ++idx)
      {
         double d_idx = elevDbList[idx]->getMeanSpacingMeters();
         if (ossim::isnan(spacing) || (d_idx < spacing))
         {
            spacing = d_idx;
         }
      }
   }
   return spacing;
}

std::ostream& ossimElevManager::print(ostream& out) const
{
   out << "\nossimElevManager @ "<< (ossim_uint64) this
         << "\nm_defaultHeightAboveEllipsoid = "<<m_defaultHeightAboveEllipsoid
         << "\nm_elevationOffset = "<<m_elevationOffset
         << "\nm_useGeoidIfNullFlag = "<<m_useGeoidIfNullFlag
         << "\nm_currentDatabaseIdx = "<<m_currentDatabaseIdx
         << "\nm_dbRoundRobin.size = "<<m_dbRoundRobin.size();
   for (ossim_uint32 i=0; i<m_dbRoundRobin.size(); ++i)
   {
      out<<"\nm_dbRoundRobin["<<i<<"].size = "<<m_dbRoundRobin[i].size()<<endl;
      for (ossim_uint32 j=0; j<m_dbRoundRobin[i].size(); ++j)
      {
         out<<"m_dbRoundRobin["<<i<<"]["<<j<<"] = ";
         // GP: We have to separate this line.  On MS it will not compile
         // otherwise
         m_dbRoundRobin[i][j]->print(out);

      }
         out<<endl;
   }
   out<<"\n";
   ossimElevSource::print(out);
   return out;
}


//---
//
// File: ossimImageElevationDatabase.h
// 
// License: MIT
// 
// Author:  David Burken
//
// Description: See description for class below.
//
//---
// $Id$

#ifndef ossimImageElevationDatabase_HEADER
#define ossimImageElevationDatabase_HEADER 1

#include <ossim/elevation/ossimElevationCellDatabase.h>
#include <ossim/base/ossimFileProcessorInterface.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimRtti.h>
#include <iosfwd>
#include <map>

class ossimString;

/**
 * @class ossimTiledElevationDatabase
 *
 * Elevation source used for working with generic images opened by an
 * ossimImageHandler. This class is typically utilized through the
 * ossimElevManager.
 */
class OSSIM_DLL ossimImageElevationDatabase :
   public ossimElevationCellDatabase, public ossimFileProcessorInterface
{
public:

   /** default constructor */
   ossimImageElevationDatabase();

   /**
    * @brief Open a connection to a database.
    *
    * @param connectionString File or directory to open.  In most cases this
    * will point to a directory containing DEMs. Satisfies pure virtual
    * ossimElevationDatabase::open().
    *
    * @return true on success, false on error.
    */   
   virtual bool open(const ossimString& connectionString);

   /** @brief close method. Unreferences all data. */
   virtual void close();

   virtual ossimObject* dup() const
   {
      ossimImageElevationDatabase* duped = new ossimImageElevationDatabase;
      duped->open(m_connectionString);
      return duped;
   }

   /**
    * @brief Maps elevation data for region to a grid.
    *
    * This uses connectionString passed to open method as starting point.
    */
   void mapRegion(const ossimGrect& region);

   /**
    * @brief Get height above MSL for point.
    *
    * Satisfies pure virtual ossimElevSource::getHeightAboveMSL().
    * 
    * @return Height above MSL.
    */
   virtual double getHeightAboveMSL(const ossimGpt& gpt);

   /**
    * @brief Get height above ellipsoid for point.
    *
    * Satisfies pure virtual ossimElevSource::getHeightAboveMSL().
    * 
    * @return Height above MSL.
    */
   virtual double getHeightAboveEllipsoid(const ossimGpt&);
   
   /**
    * Satisfies pure virtual ossimElevSource::pointHasCoverage
    * 
    * @return true if database has coverage for point.
    */
   virtual bool pointHasCoverage(const ossimGpt& gpt) const;


   virtual bool getAccuracyInfo(ossimElevationAccuracyInfo& info, const ossimGpt& gpt) const;


   /**
    * Statisfies pure virtual ossimElevSource::getAccuracyLE90.
    * @return The vertical accuracy (90% confidence) in the
    * region of gpt:
    */
   //virtual double getAccuracyLE90(const ossimGpt& gpt) const;
   
   /**
    * Statisfies pure virtual ossimElevSource::getAccuracyCE90.
    * @return The horizontal accuracy (90% confidence) in the
    * region of gpt.
    */
   //virtual double getAccuracyCE90(const ossimGpt& gpt) const;

   /** @brief Initialize from keyword list. */
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

   /** @brief Save the state to a keyword list. */
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;

   /**
    * @brief Gets the bounding rectangle/coverage of elevation.
    *
    * @param rect Rectangle to initialize.
    */
   void getBoundingRect(ossimGrect& rect) const;

   /**
    * @brief ProcessFile method.
    *
    * Satisfies pure virtual ossimFileProcessorInterface::processFile.
    *
    * This method is linked to the ossimFileWalker::walk method via a pointer
    * to this class.
    * 
    * @param file to process.
    */
   virtual void processFile(const ossimFilename& file);
   
   virtual std::ostream& print(std::ostream& out) const;

protected:
   /**
    * @Brief Protected destructor.
    *
    * This class is derived from ossimReferenced so users should always use
    * ossimRefPtr<ossimImageElevationDatabase> to hold instance.
    */
   virtual ~ossimImageElevationDatabase();

   virtual ossimRefPtr<ossimElevCellHandler> createCell(const ossimGpt& gpt);

   // virtual ossim_uint64 createId(const ossimGpt& pt) const;

   /**
    * @brief Gets cell for point.
    *
    * This override ossimElevationCellDatabase::getOrCreateCellHandler as we cannot use
    * the createId as our cells could be of any size.
    */
   virtual ossimRefPtr<ossimElevCellHandler> getOrCreateCellHandler(const ossimGpt& gpt);

   /**
    * @brief Removes an entry from the m_cacheMap and m_entryMap maps.
    */
   virtual void remove(ossim_uint64 id);

private:

   // Private container to hold bounding rect, nominal GSD, and image handler.
   struct ossimImageElevationFileEntry
   {
      /** @brief default constructor */
      ossimImageElevationFileEntry();

      /** @brief Constructor that takes a file name. */
      ossimImageElevationFileEntry(const ossimFilename& file);

      ossimImageElevationFileEntry(const ossimImageElevationFileEntry& copy_this);

      void saveState( ossimKeywordlist& kwl, const std::string& prefix ) const;

      bool loadState(const ossimKeywordlist& kwl, const std::string& prefix, const ossimFilename& base );

      /** file name */
      ossimFilename m_file;

      /** Bounding rectangle in decimal degrees. */
      ossimGrect m_rect;

      // post spacing at center
      // ossimDpt m_nominalGSD;

      /** True if in ossimElevationCellDatabase::m_cacheMap. */
      bool m_loadedFlag;
   };  

   /**
    * @brief Initializes m_entryMap with all loadable files from
    * m_connectionString.
    */
   void loadFileMap();

   /**
    * @brief Initializes m_entryMap with all loadable files from
    * m_connectionString.
    */
   bool loadMapFromKwl();

   /** Hidden from use copy constructor */
   ossimImageElevationDatabase(const ossimImageElevationDatabase& copy_this);
   
   std::map<ossim_uint64, ossimImageElevationFileEntry> m_entryMap;
   ossim_uint64       m_lastMapKey;
   ossim_uint64       m_lastAccessedId;

   TYPE_DATA 
};

inline void ossimImageElevationDatabase::remove(ossim_uint64 id)
{
   std::map<ossim_uint64, ossimImageElevationFileEntry>::iterator entryIter = m_entryMap.find(id);
   if ( entryIter != m_entryMap.end() )
   {
      (*entryIter).second.m_loadedFlag = false;
   }
   ossimElevationCellDatabase::remove(id);
}

#endif /* ossimImageElevationDatabase_HEADER */

//----------------------------------------------------------------------------
//
// File: ossimImageElevationHandler.h
// 
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: See description for class below.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimImageElevationHandler_HEADER
#define ossimImageElevationHandler_HEADER 1

#include <ossim/elevation/ossimElevCellHandler.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimTileCache.h>
#include <mutex>

/**
 * @class ossimImageElevationHandler
 *
 * Elevation source for a generic image opened via ossimImageHandler.
 */
class OSSIM_DLL ossimImageElevationHandler : public ossimElevCellHandler
{
public:

   /** default constructor */
   ossimImageElevationHandler();
   ossimImageElevationHandler(const ossimFilename& file);

   /**
    * METHOD: getHeightAboveMSL
    * Height access methods.
    */
   virtual double getHeightAboveMSL(const ossimGpt&);

   /**
    *  METHOD:  getSizeOfElevCell
    *  Returns the number of post in the cell.  Satisfies pure virtual.
    *  Note:  x = longitude, y = latitude
    */
   virtual ossimIpt getSizeOfElevCell() const;
      
   /**
    *  METHOD:  getPostValue
    *  Returns the value at a given grid point as a double.
    *  Satisfies pure virtual.
    */
   virtual double getPostValue(const ossimIpt& gridPt) const;

   /** @return True if open, false if not. */
   virtual bool isOpen()const;
   
   /**
    * Opens a stream to the srtm cell.
    *
    * @return Returns true on success, false on error.
    */
   virtual bool open(const ossimFilename& file);

   /** @brief Closes the stream to the file. */
   virtual void close();

   /**
    * @brief pointHasCoverage(gpt)
    *
    * Overrides ossimElevCellHandler::pointHasCoverage
    * @return TRUE if coverage exists over gpt.
    */
   virtual bool pointHasCoverage(const ossimGpt&) const;

   virtual ossimObject* dup () const { return new ossimImageElevationHandler(this->getFilename()); }

protected:
   /**
    * @Brief Protected destructor.
    *
    * This class is derived from ossimReferenced so users should always use
    * ossimRefPtr<ossimImageElevationHandler> to hold instance.
    */
   virtual ~ossimImageElevationHandler();
   
private:

   class TileCacheEntry
   {
   public:
      TileCacheEntry() : id(99999) {}
      TileCacheEntry(ossim_uint32 xid, ossimImageData* xdata) : id(xid), data(xdata) {}
      TileCacheEntry(const TileCacheEntry& copy) : id(copy.id), data(copy.data) {}

      const TileCacheEntry& operator=(const TileCacheEntry& copy)
      {  id = copy.id; data = copy.data; return *this;  }

      ossim_uint32 id;
      ossimRefPtr<ossimImageData> data;
   };

   /** Hidden from use copy constructor */
   ossimImageElevationHandler(const ossimImageElevationHandler&);
   
   /** Hidden from use assignment operator */
   const ossimImageElevationHandler& operator= (const ossimImageElevationHandler& rhs);

   /** Looks for an elevation tile in the cache first before reading the tile from the input handler */
   ossimImageData* getTile(ossim_uint32 x, ossim_uint32 y) const;

   /** Pointers to links in chain. */
   mutable ossimRefPtr<ossimImageHandler> m_ih;
   ossimRefPtr<ossimImageGeometry>     m_geom;
   mutable std::vector<TileCacheEntry> m_tileCache;

   /** Image space rect stored as drect for inlined pointHasCoverage method. */
   ossimDrect                          m_rect;
   ossimIpt                            m_tileSize;
   ossim_uint32                        m_numTilesPerRow;

   mutable std::mutex  m_mutex;

   TYPE_DATA
};

inline bool ossimImageElevationHandler::isOpen() const
{
   return m_ih.valid();
}

inline void ossimImageElevationHandler::close()
{
   m_geom  = 0;
   m_ih    = 0;
}

inline bool ossimImageElevationHandler::pointHasCoverage(const ossimGpt& gpt) const
{
   if ( m_geom.valid() )
   {
      ossimDpt dpt;
      m_geom->worldToLocal(gpt, dpt);
      return m_rect.pointWithin(dpt);
   }
   return false;
}

#endif /* ossimImageElevationHandler_HEADER */

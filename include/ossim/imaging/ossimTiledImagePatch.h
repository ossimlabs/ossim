//---
// License: MIT
//
// Author: David Burken
// 
// Description: See class declaration below.
// 
//---
// $Id$

#ifndef ossimTiledImagePatch_HEADER
#define ossimTiledImagePatch_HEADER 1

#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/imaging/ossimImageData.h>
#include <vector>


/**
 * @class ossimTiledImagePatch
 *
 * Class to fill a patch from input tiles requested on even tile boundaries
 * with a tile size typically matching the input, with some output tile size
 * different from the input.
 */
class OSSIMDLLEXPORT ossimTiledImagePatch : public ossimImageSourceFilter
{
public:

   /** default constructor */
   ossimTiledImagePatch();

   /**
    * @brief Initializes bounding rects and tile size(if not set) from input.
    */
   virtual void initialize();

   /**
    *  Returns a pointer to a tile given an origin representing the upper left
    *  corner of the tile to grab from the image.
    *  Satisfies pure virtual from TileSource class.
    */
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tileRect,
                                               ossim_uint32 resLevel=0);

   /**
    * Method to get a tile.   
    *
    * @param result The tile to stuff.  Note The requested rectangle in full
    * image space and bands should be set in the result tile prior to
    * passing.
    *
    * @return true on success false on error.  If return is false, result
    * is undefined so caller should handle appropriately with makeBlank or
    * whatever.
    */
   virtual bool getTile(ossimImageData* result, ossim_uint32 resLevel=0);
   
   /** @return "ossimTiledImagePatch" */
   virtual ossimString getClassName() const;

   /** @return "OSSIM tiled image patch" */
   virtual ossimString getLongName()  const;

   /** @return "tiled_image_patch" */
   virtual ossimString getShortName() const; 
   
   /** @return The reference to the input tile size. */
   const ossimIpt& getInputTileSize() const;

   /**
    * @brief Sets the input tile size.
    * @param tileSize
    * @return true on success, false on error.
    * On error m_inpuTileSize is set to nans.
    */
   bool setInputTileSize( const ossimIpt& tileSize );

   /**
    * @biref Method to the load the class state from a keyword list.
    * @param kwl
    * @param prefix
    * @return true on success, false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   /**
    * Method to save the state of an object to a keyword list.
    * @param kwl
    * @param prefix
    * @return true on success, false on error.        
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;

protected:

   /** virtual protected destructor */
   virtual ~ossimTiledImagePatch();

private:

   /** @brief Allocates the tile.  Called on first getTile. */
   void allocateTile();

   /** @brief Test the bounds of resLevel. */
   bool isValidRLevel(ossim_uint32 resLevel) const;
   
   /** @brief Private to disallow use... */
   ossimTiledImagePatch(const ossimTiledImagePatch&);

   /** @brief Private to disallow use... */
   ossimTiledImagePatch& operator=(const ossimTiledImagePatch&);
   
   ossimRefPtr<ossimImageData> m_tile;
   ossimIpt                    m_inputTileSize;
   std::vector<ossimIrect>     m_inputBoundingRect;

TYPE_DATA
};

#endif /* #ifndef ossimTiledImagePatch_HEADER */

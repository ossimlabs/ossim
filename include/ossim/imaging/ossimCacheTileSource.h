//*******************************************************************
//
// License: MIT
//
// Author:  Garrett Potts
//
// Description:  ossimCacheTileSource
// 
//*******************************************************************
//  $Id$
#ifndef ossimCacheTileSource_HEADER
#define ossimCacheTileSource_HEADER 1
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/imaging/ossimAppFixedTileCache.h>
#include <ossim/base/ossimProcessProgressEvent.h>
#include <mutex>

/** Cache Tile Source */
class OSSIMDLLEXPORT ossimCacheTileSource : public ossimImageSourceFilter
{
public:
   typedef std::vector<ossimAppFixedTileCache::ossimAppFixedCacheId> RLevelCacheList;
   /**
    * Will construct a new Application cache
    */
   ossimCacheTileSource();
   // ossimCacheTileSource(ossimImageSource* inputSource);

   virtual ~ossimCacheTileSource();

   virtual ossimString getLongName()  const;
   virtual ossimString getShortName() const;
   
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tileRect,
                                               ossim_uint32 resLevel=0);
   virtual void initialize();
   virtual void flush();
   virtual void setCachingEnabledFlag(bool value);
   virtual void setEventProgressFlag(bool value);

   void getTileSize(ossimIpt& size) const;

   /**
    * @brief Set the tile size.  This changes underlying cache tile size.
    * @param Size of cache tile.
    */
   void setTileSize(const ossimIpt& size);
   
   virtual ossim_uint32 getTileWidth() const;
   virtual ossim_uint32 getTileHeight() const;
   
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;

   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;
   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;
   

   
protected:
   void allocate();
   void deleteRlevelCache();
   void initializeRlevelCache();
   
   ossimAppFixedTileCache::ossimAppFixedCacheId getCacheId(ossim_uint32 resLevel);

      virtual void fireProgressEvent(double percentComplete);
   
   // virtual ossimRefPtr<ossimImageData> fillTile(ossim_uint32 resLevel);
   virtual ossimRefPtr<ossimImageData> fillTile(
      const ossimIrect& tileRect, ossim_uint32 resLevel,
      ossimAppFixedTileCache::ossimAppFixedCacheId cacheId );
                                                

   ossimRefPtr<ossimImageData> theTile;
   ossimIpt                    theFixedTileSize;
   bool                        theCachingEnabled;
   bool                        theEventProgressFlag;
   bool                        theUseInputTileSizeFlag;
   RLevelCacheList             theRLevelCacheList;
   ossimIpt                    theTileSizeXY;
   
   /** For lock and unlock. */

TYPE_DATA
};

#endif /* end of "#ifndef ossimCacheTileSource_HEADER" */

//---
// License: MIT
//
// Author: David Burken
// 
// Description:
//
// Class to fill a patch from input tiles requested on even tile boundaries with
// a tile size typically matching the input, with some output tile size
// different from the input.
// 
//---
// $Id$

#include <ossim/imaging/ossimTiledImagePatch.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/imaging/ossimImageDataFactory.h>

RTTI_DEF1(ossimTiledImagePatch, "ossimTiledImagePatch", ossimImageSourceFilter)

ossimTiledImagePatch::ossimTiledImagePatch()
   :
   ossimImageSourceFilter(),  // base class
   m_tile(0),
   m_inputTileSize()
{
   m_inputTileSize.makeNan();
}

ossimTiledImagePatch::~ossimTiledImagePatch()
{
}

void ossimTiledImagePatch::initialize()
{
   //---
   // Call the base class initialize.
   // Note:  This will reset "theInputConnection" if it changed...
   //---
   ossimImageSourceFilter::initialize();

   // Clear everything.  The tile will be allocated on first getTile call.
   m_tile = 0;

   // Get the input tile sizes:
   if ( theInputConnection )
   {
      if ( m_inputTileSize.hasNans() )
      {
         m_inputTileSize.x = theInputConnection->getTileWidth();
         m_inputTileSize.y = theInputConnection->getTileHeight();
      }
   }
}

ossimRefPtr<ossimImageData> ossimTiledImagePatch::getTile(
   const ossimIrect& tileRect, ossim_uint32 resLevel)
{
   if ( m_tile.valid() == false )
   {
      allocateTile(); // First time through...
   }
   
   if ( m_tile.valid() )
   {
      // Image rectangle must be set prior to calling getTile.
      m_tile->setImageRectangle(tileRect);

      if ( getTile( m_tile.get(), resLevel ) == false )
      {
         if (m_tile->getDataObjectStatus() != OSSIM_NULL)
         {
            m_tile->makeBlank();
         }
      }
   }

   return m_tile;
}

bool ossimTiledImagePatch::getTile(ossimImageData* result, ossim_uint32 resLevel)
{
   bool status = false;

   if ( isSourceEnabled() && theInputConnection && isValidRLevel(resLevel) &&
        result && (m_inputTileSize.hasNans() == false) )
   {
      status = true;
      
      // See if any point of the requested tile is in the image.
      ossimIrect tile_rect = result->getImageRectangle();

      ossimIrect input_rect;
      theInputConnection->getBoundingRect( input_rect, resLevel );

      if ( tile_rect.intersects( input_rect ) )         
      {
         // Initialize the tile if needed as we're going to stuff it.
         if (result->getDataObjectStatus() == OSSIM_NULL)
         {
            result->initialize();
         }

         // If empty imput tiles are pulled the entire output tile might not be filled.
         result->makeBlank();
         
         // Clip rect:
         ossimIrect clip_rect = tile_rect.clipToRect( input_rect );
         
         // Zero based start point.
         ossimIpt inputOrigin = clip_rect.ul() - input_rect.ul();         

         // Zero based point on input tile boundary.
         inputOrigin.x = (inputOrigin.x / m_inputTileSize.x) * m_inputTileSize.x;
         inputOrigin.y = (inputOrigin.y / m_inputTileSize.y) * m_inputTileSize.y;

         // Shift back to original space:
         inputOrigin += input_rect.ul();

         // Line loop:
         for ( ossim_int32 y = inputOrigin.y; y < clip_rect.lr().y; y += m_inputTileSize.y )
         {
            // Sample loop:
            for ( ossim_int32 x = inputOrigin.x; x < clip_rect.lr().x; x += m_inputTileSize.x )
            {
               ossimIrect rect( x, y, x + m_inputTileSize.x - 1, y + m_inputTileSize.y - 1 );

               ossimRefPtr<ossimImageData> tile = theInputConnection->getTile( rect, resLevel );
               if ( tile.valid() )
               {
                  if ( (tile->getDataObjectStatus() != OSSIM_NULL) &&
                       (tile->getDataObjectStatus() != OSSIM_EMPTY) )
                  {
                     result->loadTile( tile.get() );
                  }
               }
            }
         }

         result->validate();
               
      }
      else 
      {
         // No part of requested tile within the image rectangle.
         result->makeBlank();
      }
      
   } // matches: if( isOpen() && isSourceEnabled() && isValidRLevel(level) )

   return status;
}

ossimString ossimTiledImagePatch::getClassName() const
{
   return ossimString("ossimTiledImagePatch");
}

ossimString ossimTiledImagePatch::getLongName()const
{
   return ossimString("OSSIM tiled image patch");
}

ossimString ossimTiledImagePatch::getShortName()const
{
   return ossimString("tiled_image_patch");
}

const ossimIpt& ossimTiledImagePatch::getInputTileSize() const
{
   return m_inputTileSize;
}

bool ossimTiledImagePatch::setInputTileSize( const ossimIpt& tileSize )
{
   bool status = true;
   if ( ( tileSize.hasNans() == false ) && (tileSize.x > 0) && (tileSize.y > 0) )
   {
      m_inputTileSize = tileSize;
   }
   else
   {
      m_inputTileSize.makeNan();
      status = false;
   }
   return status;
}

bool ossimTiledImagePatch::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   std::string myPrefix = (prefix ? prefix : "" );
   std::string key = "tile_size";
   std::string value = kwl.findKey( myPrefix, key );
   if ( value.size() )
   {
      m_inputTileSize.toPoint( value );
   }
   return ossimImageSourceFilter::loadState(kwl, prefix);
}

bool ossimTiledImagePatch::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   std::string myPrefix = (prefix ? prefix : "" );
   std::string key = "tile_size";
   kwl.addPair( myPrefix, key, m_inputTileSize.toString().string() );
   return ossimImageSourceFilter::saveState(kwl, prefix);
}

void ossimTiledImagePatch::allocateTile()
{
   m_tile = ossimImageDataFactory::instance()->create(this,this);
   m_tile->initialize();
}

bool ossimTiledImagePatch::isValidRLevel(ossim_uint32 resLevel) const
{
   // return (resLevel < m_inputBoundingRect.size());
   return (resLevel < theInputConnection->getNumberOfDecimationLevels());
}

// Private to disallow use...
ossimTiledImagePatch::ossimTiledImagePatch(const ossimTiledImagePatch&)
   : m_tile(0),
     m_inputTileSize()
{
}

// Private to disallow use...
ossimTiledImagePatch& ossimTiledImagePatch::operator=(const ossimTiledImagePatch&)
{
   return *this;
}



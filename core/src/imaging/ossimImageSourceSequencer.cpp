//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
//*******************************************************************
//  $Id: ossimImageSourceSequencer.cpp 23082 2015-01-15 12:30:25Z dburken $

#include <ossim/imaging/ossimImageSourceSequencer.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageWriter.h>

RTTI_DEF2(ossimImageSourceSequencer, "ossimImageSourceSequencer",
          ossimImageSource, ossimConnectableObjectListener);

static ossimTrace traceDebug("ossimImageSourceSequencer:debug");
   
ossimImageSourceSequencer::ossimImageSourceSequencer(ossimImageSource* inputSource,
                                                     ossimObject* owner)
:ossimImageSource(owner,
                  1,
                  1,
                  true,
                  false),
    theBlankTile(NULL),
    theAreaOfInterest(0,0,0,0),
    theTileSize(OSSIM_DEFAULT_TILE_WIDTH, OSSIM_DEFAULT_TILE_HEIGHT),
    theNumberOfTilesHorizontal(0),
    theNumberOfTilesVertical(0),
    theCurrentTileNumber(0)
{
   ossim::defaultTileSize(theTileSize);
   theAreaOfInterest.makeNan();
   theInputConnection    = inputSource;
   if(inputSource)
   {
     connectMyInputTo(0, inputSource);
     initialize(); // Derived class depends on this initialization to happen now. DO NOT MOVE.
   }
   addListener( (ossimConnectableObjectListener*)this);
}

ossimImageSourceSequencer::~ossimImageSourceSequencer()
{
   removeListener((ossimConnectableObjectListener*)this);
}

ossim_int64 ossimImageSourceSequencer::getNumberOfTiles()const
{
   return (theNumberOfTilesHorizontal*theNumberOfTilesVertical);
}


ossim_int64 ossimImageSourceSequencer::getNumberOfTilesHorizontal()const
{
   return theNumberOfTilesHorizontal;
}

ossim_int64 ossimImageSourceSequencer::getNumberOfTilesVertical()const
{
   return theNumberOfTilesVertical;
}

ossimScalarType ossimImageSourceSequencer::getOutputScalarType() const
{
   if(theInputConnection)
   {
      return theInputConnection->getOutputScalarType();
   }
   
   return OSSIM_SCALAR_UNKNOWN;
}

ossimIpt ossimImageSourceSequencer::getTileSize()const
{
   return theTileSize;
}

void ossimImageSourceSequencer::setTileSize(const ossimIpt& tileSize)
{
   theTileSize = tileSize;
   updateTileDimensions();
//   initialize();
}

void ossimImageSourceSequencer::setTileSize(ossim_int32 width, ossim_int32 height)
{
   setTileSize(ossimIpt(width, height));
}

void ossimImageSourceSequencer::updateTileDimensions()
{
   bool status = false;
   if( !theAreaOfInterest.hasNans() && !theTileSize.hasNans() )
   {
      ossim_float64 aoiWidth   = theAreaOfInterest.width();
      ossim_float64 aoiHeight  = theAreaOfInterest.height();
      ossim_float64 tileWidth  = theTileSize.x;
      ossim_float64 tileHeight = theTileSize.y;
      
      if ( (aoiWidth > 0.0) && (aoiHeight > 0.0) &&
           (tileWidth > 0.0) && (tileHeight > 0.0) )
      {
         theNumberOfTilesHorizontal
            = static_cast<ossim_int64>(std::ceil(aoiWidth/tileWidth));
         theNumberOfTilesVertical
            = static_cast<ossim_int64>(std::ceil(aoiHeight/tileHeight));
         status = true;
      }
   }

   if ( !status )
   {
      theNumberOfTilesHorizontal = 0;
      theNumberOfTilesVertical   = 0;
   }
}

void ossimImageSourceSequencer::initialize()
{
   theInputConnection = PTR_CAST(ossimImageSource, getInput(0));

   if(theInputConnection)
   {
      if(theTileSize.hasNans())
      {
         theTileSize.x = theInputConnection->getTileWidth();
         theTileSize.y = theInputConnection->getTileHeight();
      }

      ossimDrect rect = theInputConnection->getBoundingRect();
      if(rect.hasNans())
      {
         theAreaOfInterest.makeNan();
      }
      else
      {
         rect.stretchOut();
         setAreaOfInterest(rect);
      }
      updateTileDimensions();
      
      theBlankTile  = ossimImageDataFactory::instance()->create(this,
                                                                this);
      if(theBlankTile.valid())
      {
         theBlankTile->initialize();
      }
   }
}

bool ossimImageSourceSequencer::canConnectMyInputTo(ossim_int32 /* inputIndex */,
						    const ossimConnectableObject* object)const
{      
   return (object&& PTR_CAST(ossimImageSource, object));
}

void ossimImageSourceSequencer::connectInputEvent(ossimConnectionEvent& /* event */)
{
   initialize();
}

void ossimImageSourceSequencer::disconnectInputEvent(ossimConnectionEvent& /* event */)
{
   theInputConnection = PTR_CAST(ossimImageSource, getInput(0));
}

ossimIrect ossimImageSourceSequencer::getBoundingRect(ossim_uint32 resLevel)const
{
   ossimDrect temp;
   temp.makeNan();
   
   if(!theInputConnection) return temp;
   
   if(theAreaOfInterest.hasNans())
   {
      return theInputConnection->getBoundingRect(resLevel);
   }
   
   return theAreaOfInterest;
}

void ossimImageSourceSequencer::getDecimationFactor(ossim_uint32 resLevel,
                                                    ossimDpt& result) const
{
   if(theInputConnection)
   {
      theInputConnection->getDecimationFactor(resLevel, result);
   }
   result.makeNan();
}

void ossimImageSourceSequencer::getDecimationFactors(vector<ossimDpt>& decimations) const
{
   if(theInputConnection)
   {
      theInputConnection->getDecimationFactors(decimations);
   }   
}

ossim_uint32 ossimImageSourceSequencer::getNumberOfDecimationLevels()const
{
   ossim_uint32 result = 0;
   if(theInputConnection)
   {
      result = theInputConnection->getNumberOfDecimationLevels();
   }
   return result;
}

void ossimImageSourceSequencer::setAreaOfInterest(const ossimIrect& areaOfInterest)
{
   if(areaOfInterest.hasNans())
   {
      theAreaOfInterest.makeNan();
      theNumberOfTilesHorizontal = 0;
      theNumberOfTilesVertical   = 0;
   }

   // let's round it to the nearest pixel value before setting it.
   theAreaOfInterest = areaOfInterest;
   updateTileDimensions();
}

const ossimIrect& ossimImageSourceSequencer::getAreaOfInterest()const
{
   return theAreaOfInterest;
}

void ossimImageSourceSequencer::setToStartOfSequence()
{
   theCurrentTileNumber = 0;
}

ossimRefPtr<ossimImageData> ossimImageSourceSequencer::getTile(
   const ossimIrect& rect, ossim_uint32 resLevel)
{
   if(theInputConnection)
   {
      ossimRefPtr<ossimImageData> tile =
         theInputConnection->getTile(rect, resLevel);
      if (tile.valid()&&tile->getBuf())
      {
         return tile;
      }
      else
      {
         // We should return a valid tile for the writers.
         theBlankTile->setImageRectangle(rect);
         return theBlankTile;
      }
   }

   return 0;
}

ossimRefPtr<ossimImageData> ossimImageSourceSequencer::getNextTile( ossim_uint32 resLevel )
{
   ossimRefPtr<ossimImageData> result = 0;
   if ( theInputConnection )
   {
      ossimIrect tileRect;
      if ( getTileRect( theCurrentTileNumber, tileRect ) )
      {
         ++theCurrentTileNumber;
         result = theInputConnection->getTile(tileRect, resLevel);
         if( !result.valid() || !result->getBuf() )
         {	 
            theBlankTile->setImageRectangle(tileRect);
            result = theBlankTile;
         }
      }
   }
   return result;
}

ossimRefPtr<ossimImageData> ossimImageSourceSequencer::getTile(
   ossim_int64 id, ossim_uint32 resLevel)
{
   static const char* MODULE= "ossimImageSourceSequencer::getTile(id, resLevel)";
   if(traceDebug())
   {
      CLOG << "entering.."<<endl;
   }

   ossimRefPtr<ossimImageData> result = 0;

   if(theInputConnection)
   {
      // if we have no tiles try to initialize.
      if(getNumberOfTiles() == 0)
      {
         initialize();
      }

      ossimIrect tileRect;
      if ( getTileRect( id, tileRect ) )
      {
         result = theInputConnection->getTile(tileRect, resLevel);
         if( !result.valid() || !result->getBuf() )
         {	 
            theBlankTile->setImageRectangle(tileRect);
            result = theBlankTile;
         }
      }
      else // getTileRect failed...
      {
         if(traceDebug())
         {
            CLOG << "was not able to get an origin for id = " << id << endl;
         }
      }
   }
   else // no connection...
   {
      if(traceDebug())
      {
         CLOG << "No input connection so returing NULL" << endl;
      }
   }
   if(traceDebug())
   {
      CLOG << "leaving.."<<endl;
   }
   
   return result;
}

bool ossimImageSourceSequencer::getTileOrigin(ossim_int64 id, ossimIpt& origin) const
{
   bool result = false;
   if( id >= 0 )
   {
      if( (theNumberOfTilesHorizontal > 0) && (theCurrentTileNumber < getNumberOfTiles()) )
      {
         ossim_int64 y = id / theNumberOfTilesHorizontal;
         ossim_int64 x = id % theNumberOfTilesHorizontal;
         if( (x < theNumberOfTilesHorizontal) && (y < theNumberOfTilesVertical) )
         {
            ossim_int64 ulx = theAreaOfInterest.ul().x;
            ossim_int64 uly = theAreaOfInterest.ul().y;
            ossim_int64 tx  = theTileSize.x;
            ossim_int64 ty  = theTileSize.y; 
            x = ulx + x * tx;
            y = uly + y * ty;

            //---
            // ossimIpt currently signed 32 bit so make sure we didn't bust the
            // bounds.
            //---
            if ( (x <= OSSIM_DEFAULT_MAX_PIX_SINT32) && ( y <= OSSIM_DEFAULT_MAX_PIX_SINT32) )
            {
               origin.x = (ossim_int32)x;
               origin.y = (ossim_int32)y;
               result = true;
            }
         }
      }
   }
   return result;
}

bool ossimImageSourceSequencer::getTileRect(ossim_int64 tile_id, ossimIrect& tileRect) const
{
   // Fetch tile origin for this tile:
   ossimIpt origin;
   bool result = getTileOrigin(tile_id, origin);
   if ( result )
   {
      // Establish the tile rect of interest for this tile:
      tileRect.set_ul (origin);
      tileRect.set_lrx(origin.x + theTileSize.x - 1);
      tileRect.set_lry(origin.y + theTileSize.y - 1);
   }
   return result;
}

double ossimImageSourceSequencer::getNullPixelValue(ossim_uint32 band)const
{
   if (theInputConnection)
   {
      return theInputConnection->getNullPixelValue(band);
   }

   return ossimImageSource::getNullPixelValue(band);
}

double ossimImageSourceSequencer::getMinPixelValue(ossim_uint32 band)const
{
   if (theInputConnection)
   {
      return theInputConnection->getMinPixelValue(band);
   }
   
   return ossimImageSource::getMinPixelValue(band);  
}

double ossimImageSourceSequencer::getMaxPixelValue(ossim_uint32 band)const
{
   if (theInputConnection)
   {
      return theInputConnection->getMaxPixelValue(band);
   }
   
   return ossimImageSource::getMaxPixelValue(band);  
}

ossim_uint32 ossimImageSourceSequencer::getNumberOfInputBands()const
{
   if(theInputConnection)
   {
      return theInputConnection->getNumberOfOutputBands();
   }

   return 0;
}

ossim_uint32 ossimImageSourceSequencer::getTileWidth() const
{
   return theTileSize.x;
}

ossim_uint32 ossimImageSourceSequencer::getTileHeight()const
{
   return theTileSize.y;
}

void ossimImageSourceSequencer::slaveProcessTiles()
{
}

bool ossimImageSourceSequencer::isMaster() const
{
   return true;
}



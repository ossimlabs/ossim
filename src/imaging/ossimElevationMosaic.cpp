//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: 
// 
// Description: implementation for image mosaic
//
//*************************************************************************
// $Id: ossimElevationMosaic.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <ossim/imaging/ossimElevationMosaic.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/base/ossimTrace.h>
static const ossimTrace traceDebug("ossimElevationMosaic:debug");
static const ossim_float32 MIN_ELEVATION = -500.0;
static const ossim_float32 MAX_ELEVATION = 32000.0;

using namespace std;

RTTI_DEF1(ossimElevationMosaic, "ossimElevationMosaic", ossimImageCombiner)
ossimElevationMosaic::ossimElevationMosaic()
   :ossimImageCombiner(),
    theTile(NULL)
{

}

ossimElevationMosaic::ossimElevationMosaic(ossimConnectableObject::ConnectableObjectList& inputSources)
    : ossimImageCombiner(inputSources),
      theTile(NULL)
{
}


ossimElevationMosaic::~ossimElevationMosaic()
{
}

ossimRefPtr<ossimImageData> ossimElevationMosaic::getTile(
   const ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   long size = getNumberOfInputs();
   ossim_uint32 layerIdx = 0;
   // If there is only one in the mosaic then just return it.
   if(size == 1)
   {
      return getNextTile(layerIdx, 0, tileRect, resLevel);
   }
   
   ossimIpt origin = tileRect.ul();
   ossim_uint32 w = tileRect.width();
   ossim_uint32 h = tileRect.height();
   
   if(!theTile.valid())
   {
      // try to initialize
      allocate();

      // if we still don't have a buffer
      // then we will leave
      if(!theTile.valid())
      {
         return ossimRefPtr<ossimImageData>();
      }
   }
   
   ossim_uint32 tileW = theTile->getWidth();
   ossim_uint32 tileH = theTile->getHeight();
   if((w != tileW)||
      (h != tileH))
   {
      theTile->setWidth(w);
      theTile->setHeight(h);
      if((w*h)!=(tileW*tileH))
      {
         theTile->initialize();
      }
   }
   theTile->setOrigin(origin);

   //---
   // General Note:
   //
   // Note: I will not check for disabled or enabled since we have
   // no clear way to handle this within a mosaic. The default will be
   // to do a simple a A over B type mosaic.  Derived classes should
   // check for the enabled and disabled and always
   // use this default implementation if they are disabled.
   //---

   theTile->setOrigin(origin);
   theTile->makeBlank();
   switch(theTile->getScalarType())
   {
      case OSSIM_UCHAR:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<ossim_uint8>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<ossim_uint8>(0),
                               tileRect,
                               resLevel);
         }
      }
      case OSSIM_SINT8:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<ossim_sint8>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<ossim_sint8>(0),
                               tileRect,
                               resLevel);
         }
      }
      case OSSIM_FLOAT: 
      case OSSIM_NORMALIZED_FLOAT:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<float>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<float>(0),
                               tileRect,
                               resLevel);
         }
      }
      case OSSIM_USHORT16:
      case OSSIM_USHORT11:
      case OSSIM_USHORT12:
      case OSSIM_USHORT13:
      case OSSIM_USHORT14:
      case OSSIM_USHORT15:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<ossim_uint16>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<ossim_uint16>(0),
                               tileRect,
                               resLevel);
         }
      }
      case OSSIM_SSHORT16:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<ossim_sint16>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<ossim_sint16>(0),
                               tileRect,
                               resLevel);
         }
      }
      case OSSIM_SINT32:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<ossim_sint32>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<ossim_sint32>(0),
                               tileRect,
                               resLevel);
         }
      }
      case OSSIM_UINT32:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<ossim_uint32>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<ossim_uint32>(0),
                               tileRect,
                               resLevel);
         }
      }
      case OSSIM_DOUBLE:
      case OSSIM_NORMALIZED_DOUBLE:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<double>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<double>(0),
                               tileRect,
                               resLevel);
         }
      }
      case OSSIM_SCALAR_UNKNOWN:
      default:
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "Scalar type = " << theTile->getScalarType()
            << " Not supported by ossimElevationMosaic" << endl;
      }
   }

   return ossimRefPtr<ossimImageData>();
}

void ossimElevationMosaic::initialize()
{
  ossimImageCombiner::initialize();
  theTile = NULL;
}

void ossimElevationMosaic::allocate()
{
   theTile = NULL;
   
   if( (getNumberOfInputs() > 0) && getInput(0) )
   {
      theTile = ossimImageDataFactory::instance()->create(this, this);
      theTile->initialize();
   }
}

bool ossimElevationMosaic::saveState(ossimKeywordlist& kwl,
                                 const char* prefix)const
{
   return ossimImageCombiner::saveState(kwl, prefix);
}

bool ossimElevationMosaic::loadState(const ossimKeywordlist& kwl,
                                 const char* prefix)
{
   return ossimImageCombiner::loadState(kwl, prefix);
}

template <class T> ossimRefPtr<ossimImageData> ossimElevationMosaic::combineNorm(
   T,// dummy template variable 
   const ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   ossim_uint32 layerIdx = 0;
   //---
   // Get the first tile from the input sources.  If this(index 0) is blank
   // that means there are no layers so go no further.
   //---
   ossimRefPtr<ossimImageData> currentImageData =
      getNextNormTile(layerIdx, 0, tileRect, resLevel);
   if(!currentImageData)
   {
      return theTile;
   }
   
   ossimRefPtr<ossimImageData> destination = theTile;
   ossimDataObjectStatus destinationStatus = theTile->getDataObjectStatus();

   
   float** srcBands         = new float*[theLargestNumberOfInputBands];
   float*  srcBandsNullPix  = new float[theLargestNumberOfInputBands];
   T**     destBands        = new T*[theLargestNumberOfInputBands];
   T*      destBandsNullPix = new T[theLargestNumberOfInputBands];
   T*      destBandsMinPix  = new T[theLargestNumberOfInputBands];
   T*      destBandsMaxPix  = new T[theLargestNumberOfInputBands];
   
   ossim_uint32 band;
   ossim_uint32 upperBound = destination->getWidth()*destination->getHeight();
   ossim_uint32 minNumberOfBands = currentImageData->getNumberOfBands();
   for(band = 0; band < minNumberOfBands; ++band)
   {
      srcBands[band]  = static_cast<float*>(currentImageData->getBuf(band));
      srcBandsNullPix[band]  = static_cast<float>(currentImageData->getNullPix(band));
      
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
      destBandsNullPix[band] = static_cast<T>(theTile->getNullPix(band));
      destBandsMinPix[band] = static_cast<T>(theTile->getMinPix(band));
      destBandsMaxPix[band] = static_cast<T>(theTile->getMaxPix(band));
   }
   
   // if the src is smaller than the destination in number
   // of bands we will just duplicate the last band.
   for(;band < theLargestNumberOfInputBands; ++band)
   {
      srcBands[band]  = static_cast<float*>(srcBands[minNumberOfBands - 1]);
      srcBandsNullPix[band] = static_cast<float>(currentImageData->getNullPix(minNumberOfBands - 1));
      
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
      destBandsNullPix[band] = static_cast<T>(theTile->getNullPix(band));
      destBandsMinPix[band] = static_cast<T>(theTile->getMinPix(band));
      destBandsMaxPix[band] = static_cast<T>(theTile->getMaxPix(band));
   }

   // Loop to copy from layers to output tile.
   while(currentImageData.valid())
   {
      //---
      // Check the status of the source tile.  If empty get the next source
      // tile and loop back.
      //---
       ossimDataObjectStatus currentStatus =
         currentImageData->getDataObjectStatus();
      if ( (currentStatus == OSSIM_EMPTY) || (currentStatus == OSSIM_NULL) )
      {
         currentImageData = getNextNormTile(layerIdx, tileRect, resLevel);
         continue;
      }
      
      ossim_uint32 minNumberOfBands = currentImageData->getNumberOfBands();
      
      for(band = 0; band < minNumberOfBands; ++band)
      {
         srcBands[band] = static_cast<float*>(currentImageData->getBuf(band));
         srcBandsNullPix[band] = static_cast<float>(currentImageData->getNullPix(band));
      }
      // if the src is smaller than the destination in number
      // of bands we will just duplicate the last band.
      for(;band < theLargestNumberOfInputBands; ++band)
      {
         srcBands[band] = srcBands[minNumberOfBands - 1];
         srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(minNumberOfBands - 1));
      }
      
      if ( (currentStatus == OSSIM_FULL) &&
           (destinationStatus == OSSIM_EMPTY) )
      {
         // Copy full tile to empty tile.
         for(band=0; band < theLargestNumberOfInputBands; ++band)
         {
            float delta = destBandsMaxPix[band] - destBandsMinPix[band];
            float minP  = destBandsMinPix[band];
            
            for(ossim_uint32 offset = 0; offset < upperBound; ++offset)
            {
               destBands[band][offset] =
                  (T)( minP + delta*srcBands[band][offset]);
            }
         }
      }
      else // Copy tile checking all the pixels...
      {
         for(band = 0; band < theLargestNumberOfInputBands; ++band)
         {
            float delta = destBandsMaxPix[band] - destBandsMinPix[band];
            float minP  = destBandsMinPix[band];
            
            for(ossim_uint32 offset = 0; offset < upperBound; ++offset)
            {
               if (destBands[band][offset] < MIN_ELEVATION || destBands[band][offset] > MAX_ELEVATION) //== destBandsNullPix[band])
               {
                  if (srcBands[band][offset] > MIN_ELEVATION && srcBands[band][offset] < MAX_ELEVATION) //!= srcBandsNullPix[band])
                  {
                     destBands[band][offset] =
                        (T)(minP + delta*srcBands[band][offset]);
                  }
               }
            }
         }
      }

      // Validate output tile and return if full.
      destinationStatus = destination->validate();
      if (destinationStatus == OSSIM_FULL)
      {
         
         break;//return destination;
      }

      // If we get here we're are still not full.  Get a tile from next layer.
      currentImageData = getNextNormTile(layerIdx, tileRect, resLevel);
   }

   // Cleanup...
   delete [] srcBands;
   delete [] destBands;
   delete [] srcBandsNullPix;
   delete [] destBandsNullPix;
   delete [] destBandsMinPix;
   delete [] destBandsMaxPix;

   return destination;
}

template <class T> ossimRefPtr<ossimImageData> ossimElevationMosaic::combine(
   T,// dummy template variable 
   const ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   ossim_uint32 layerIdx = 0;
   bool hasInvalidElevations = false;
   //---
   // Get the first tile from the input sources.  If this(index 0) is blank
   // that means there are no layers so go no further.
   //---
   ossimRefPtr<ossimImageData> currentImageData =
      getNextTile(layerIdx, 0, tileRect, resLevel);
   if (!currentImageData)
   {
      return theTile;
   }

   ossimRefPtr<ossimImageData> destination = theTile;
   ossimDataObjectStatus destinationStatus = theTile->getDataObjectStatus();

   T** srcBands         = new T*[theLargestNumberOfInputBands];
   T*  srcBandsNullPix  = new T[theLargestNumberOfInputBands];
   T** destBands        = new T*[theLargestNumberOfInputBands];
   T*  destBandsNullPix = new T[theLargestNumberOfInputBands];
      
   ossim_uint32 band;
   ossim_uint32 upperBound = destination->getWidth()*destination->getHeight();
   ossim_uint32 minNumberOfBands = currentImageData->getNumberOfBands();
   for(band = 0; band < minNumberOfBands; ++band)
   {
      srcBands[band]  = static_cast<T*>(currentImageData->getBuf(band));
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
      srcBandsNullPix[band]  = static_cast<T>(currentImageData->getNullPix(band));
      destBandsNullPix[band] = static_cast<T>(theTile->getNullPix(band));
   }
   // if the src is smaller than the destination in number
   // of bands we will just duplicate the last band.
   for(;band < theLargestNumberOfInputBands; ++band)
   {
      srcBands[band]  = static_cast<T*>(srcBands[minNumberOfBands - 1]);
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
      srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(minNumberOfBands - 1));
      destBandsNullPix[band] = static_cast<T>(theTile->getNullPix(band));
   }

   // Loop to copy from layers to output tile.
   while(currentImageData.valid())
   {
      //---
      // Check the status of the source tile.  If empty get the next source
      // tile and loop back.
      //---
      ossimDataObjectStatus currentStatus =
         currentImageData->getDataObjectStatus();
      if ( (currentStatus == OSSIM_EMPTY) || (currentStatus == OSSIM_NULL) )
      {
         currentImageData = getNextNormTile(layerIdx, tileRect, resLevel);
         continue;
      }
      
      ossim_uint32 minNumberOfBands = currentImageData->getNumberOfBands();

      for(band = 0; band < minNumberOfBands; ++band)
      {
         srcBands[band] = static_cast<T*>(currentImageData->getBuf(band));
         srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(band));
      }
      // if the src is smaller than the destination in number
      // of bands we will just duplicate the last band.
      for(;band < theLargestNumberOfInputBands; ++band)
      {
         srcBands[band] = srcBands[minNumberOfBands - 1];
         srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(minNumberOfBands - 1));
      }

      if ( (currentStatus == OSSIM_FULL) &&
           (destinationStatus == OSSIM_EMPTY) )
      {
         // Copy full tile to empty tile.
         for(ossim_uint32 band=0; band < theLargestNumberOfInputBands; ++band)
         {
            for(ossim_uint32 offset = 0; offset < upperBound; ++offset)
            {
               destBands[band][offset] = srcBands[band][offset];
	       if(destBands[band][offset] < MIN_ELEVATION || destBands[band][offset] > MAX_ELEVATION)
	       {
		 hasInvalidElevations = true;
	       }
            }
         }
      }
      else // Copy tile checking all the pixels...
      {
         for(band = 0; band < theLargestNumberOfInputBands; ++band)
         {
            
            for(ossim_uint32 offset = 0; offset < upperBound; ++offset)
            {
               if(destBands[band][offset] < MIN_ELEVATION || destBands[band][offset] > MAX_ELEVATION) //== destBandsNullPix[band])
               {
		  hasInvalidElevations = true;
                  destBands[band][offset] = srcBands[band][offset];
               }
            }
         }
      }

      // Validate output tile and return if full.
      destinationStatus = destination->validate();
      if (!hasInvalidElevations)
      {
	 break;
      }
      //if (destinationStatus == OSSIM_FULL)
      //{
      //   break;//return destination;
      //}

      // If we get here we're are still not full.  Get a tile from next layer.
      currentImageData = getNextTile(layerIdx, tileRect, resLevel);
   }
   
   // Cleanup...
   delete [] srcBands;
   delete [] destBands;
   delete [] srcBandsNullPix;
   delete [] destBandsNullPix;
   
   return destination;
}

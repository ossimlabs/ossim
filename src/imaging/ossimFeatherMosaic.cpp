//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: ossimFeatherMosaic.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <ossim/imaging/ossimFeatherMosaic.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimLine.h>
#include <ossim/base/ossimTrace.h>

#include <algorithm>
#include <cmath>

static ossimTrace traceDebug("ossimFeatherMosaic:debug");

RTTI_DEF1(ossimFeatherMosaic, "ossimFeatherMosaic", ossimImageMosaic);

using namespace std;

ossimFeatherMosaic::ossimFeatherMosaic()
   :ossimImageMosaic(),
    theInputFeatherInformation(NULL),
    theAlphaSum(NULL),
    theResult(NULL),
    theFeatherInfoSize(0)
{
}

ossimFeatherMosaic::ossimFeatherMosaic(ossimConnectableObject::ConnectableObjectList& inputSources)
   :ossimImageMosaic(inputSources),
    theInputFeatherInformation(NULL),
    theAlphaSum(NULL),
    theResult(NULL),
    theFeatherInfoSize(0)
{
   initialize();
}

ossimFeatherMosaic::~ossimFeatherMosaic()
{
   if(theInputFeatherInformation)
   {
      delete [] theInputFeatherInformation;
      theInputFeatherInformation = NULL;
   }
   theFeatherInfoSize = 0;
}

ossimRefPtr<ossimImageData> ossimFeatherMosaic::getTile(const ossimIrect& tileRect,
                                            ossim_uint32 resLevel)
{
   long w = tileRect.width();
   long h = tileRect.height();
   ossimIpt origin = tileRect.ul();
   
   if(!isSourceEnabled())
   {
      return ossimImageMosaic::getTile(tileRect, resLevel);
   }
   if(!theTile||!theAlphaSum||!theResult||!theInputFeatherInformation)
   {
      initialize();

      if(!theTile||!theAlphaSum||!theResult||!theInputFeatherInformation)
      {
         return ossimImageMosaic::getTile(tileRect, resLevel);
      }
   }
   ossim_uint32 size = getNumberOfInputs();
   theAlphaSum->setImageRectangle(tileRect);
   theResult->setImageRectangle(tileRect);
   
   if(size == 0)
   {
      return ossimRefPtr<ossimImageData>();
   }

   if(size == 1)
   {
      return ossimImageMosaic::getTile(tileRect, resLevel);
   }

   long tileW = theTile->getWidth();
   long tileH = theTile->getHeight();
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
   theTile->makeBlank();
      
   switch(theTile->getScalarType())
   {
      case OSSIM_UCHAR:
      {
         if(hasDifferentInputs())
         {
            return combineNorm(tileRect, resLevel);
         }
         return combine(static_cast<ossim_uint8>(0),
                        tileRect, resLevel);
      }
      case OSSIM_USHORT16:
      case OSSIM_USHORT11:
      case OSSIM_USHORT12:
      case OSSIM_USHORT13:
      case OSSIM_USHORT14:
      case OSSIM_USHORT15:
      {
         if(hasDifferentInputs())
         {
            return combineNorm(tileRect, resLevel);
         }
         return combine(static_cast<ossim_uint16>(0),
                        tileRect, resLevel);
      }
      case OSSIM_SSHORT16:
      {
         if(hasDifferentInputs())
         {
            return combineNorm(tileRect, resLevel);
         }
         return combine(static_cast<ossim_sint16>(0),
                        tileRect, resLevel);
      }
      case OSSIM_DOUBLE:
      case OSSIM_NORMALIZED_DOUBLE:
      {
         if(hasDifferentInputs())
         {
            return combineNorm(tileRect, resLevel);
         }
         return combine(static_cast<double>(0),
                        tileRect, resLevel);
      }
      case OSSIM_FLOAT:
      case OSSIM_NORMALIZED_FLOAT:
      {
         if(hasDifferentInputs())
         {
            return combineNorm(tileRect, resLevel);
         }
         return combine(static_cast<float>(0),
                        tileRect, resLevel);
      }
      case OSSIM_SCALAR_UNKNOWN:
      default:
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimFeatherMosaic::getTile: error, unknown scalar type!!!"
            << std::endl;
      }
   }
   return ossimRefPtr<ossimImageData>();
}

ossimRefPtr<ossimImageData> ossimFeatherMosaic::combineNorm(
   const ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   const long upperBound = theTile->getWidth()*theTile->getHeight();
   const long outputWidth = static_cast<long>(theTile->getWidth());
   const long outputHeight = static_cast<long>(theTile->getHeight());
   const ossimIpt outputOrigin = tileRect.ul();
   float* sumBand = static_cast<float*>(theAlphaSum->getBuf());

   theAlphaSum->fill(0.0);
   theResult->fill(0.0);

   float** srcBands = new float*[theLargestNumberOfInputBands];
   ossim_uint32 layerIdx = 0;
   ossim_uint32 numberOfTilesProcessed = 0;
   ossimRefPtr<ossimImageData> currentImageData =
      getNextNormTile(layerIdx, 0, tileRect, resLevel);

   while(currentImageData.valid())
   {
      const ossimDataObjectStatus currentStatus =
         currentImageData->getDataObjectStatus();
      if((currentStatus != OSSIM_EMPTY) && (currentStatus != OSSIM_NULL))
      {
         ++numberOfTilesProcessed;
         const long h = static_cast<long>(currentImageData->getHeight());
         const long w = static_cast<long>(currentImageData->getWidth());
         const ossimIpt point = currentImageData->getOrigin();
         ossim_uint32 band = 0;
         const ossim_uint32 minNumberOfBands =
            currentImageData->getNumberOfBands();
         for(; band < minNumberOfBands; ++band)
         {
            srcBands[band] = static_cast<float*>(currentImageData->getBuf(band));
         }
         for(; band < theLargestNumberOfInputBands; ++band)
         {
            srcBands[band] = srcBands[minNumberOfBands - 1];
         }

         long sourceOffset = 0;
         for(long row = 0; row < h; ++row)
         {
            const long outputY = point.y + row - outputOrigin.y;
            for(long col = 0; col < w; ++col, ++sourceOffset)
            {
               const long outputX = point.x + col - outputOrigin.x;
               if((outputX < 0) || (outputX >= outputWidth) ||
                  (outputY < 0) || (outputY >= outputHeight))
               {
                  continue;
               }
               if((currentStatus == OSSIM_PARTIAL) &&
                  currentImageData->isNull(sourceOffset))
               {
                  continue;
               }

               const long outputOffset = outputY*outputWidth + outputX;
               const double weight = computeWeight(
                  layerIdx, ossimDpt(point.x + col, point.y + row));
               for(band = 0; band < theLargestNumberOfInputBands; ++band)
               {
                  float* weightedBand =
                     static_cast<float*>(theResult->getBuf(band));
                  weightedBand[outputOffset] +=
                     static_cast<float>(srcBands[band][sourceOffset]*weight);
               }
               sumBand[outputOffset] += static_cast<float>(weight);
            }
         }
      }
      currentImageData = getNextNormTile(layerIdx, tileRect, resLevel);
   }

   if(numberOfTilesProcessed)
   {
      for(long offset = 0; offset < upperBound; ++offset)
      {
         for(ossim_uint32 band = 0;
             band < theLargestNumberOfInputBands;
             ++band)
         {
            float* weightedBand =
               static_cast<float*>(theResult->getBuf(band));
            if(sumBand[offset] != 0.0f)
            {
               weightedBand[offset] /= sumBand[offset];
            }
            else
            {
               weightedBand[offset] = 0.0f;
            }
         }
      }
      theResult->validate();
      theTile->copyNormalizedBufferToTile(
         static_cast<float*>(theResult->getBuf()));
      theTile->validate();
   }

   delete [] srcBands;
   return theTile;
}


template <class T> ossimRefPtr<ossimImageData> ossimFeatherMosaic::combine(
   T,
   const ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   ossimRefPtr<ossimImageData> currentImageData;
   ossim_uint32 band;
   long upperBound = theTile->getWidth()*theTile->getHeight();
   long offset = 0;
   long row    = 0;
   long col    = 0;
   long numberOfTilesProcessed = 0;
   float *sumBand       = static_cast<float*>(theAlphaSum->getBuf());
   float         *bandRes = NULL;
   ossimIpt point;
   
   theAlphaSum->fill(0.0);
   theResult->fill(0.0);
   
   T** srcBands  = new T*[theLargestNumberOfInputBands];
   T** destBands = new T*[theLargestNumberOfInputBands];
   ossim_uint32 layerIdx = 0;
   currentImageData  = getNextTile(layerIdx,
                                   0,
                                   tileRect,
                                   resLevel);
   if(!currentImageData.valid())
   {
      delete [] srcBands;
      delete [] destBands;
      return currentImageData;
   }
   ossim_uint32 minNumberOfBands = currentImageData->getNumberOfBands();
   for(band = 0; band < minNumberOfBands; ++band)
   {
      srcBands[band]  = static_cast<T*>(currentImageData->getBuf(band));
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
   }
   // if the src is smaller than the destination in number
   // of bands we will just duplicate the last band.
   for(;band < theLargestNumberOfInputBands; ++band)
   {
      srcBands[band]  = srcBands[minNumberOfBands - 1];
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
   }

   // most of the time we will not overlap so let's
   // copy the first tile into destination and check later.
   //
   for(band = 0; band < theTile->getNumberOfBands();++band)
   {
      T* destBand = destBands[band];
      T* srcBand  = srcBands[band];
      if(destBand&&srcBand)
      {
         for(offset = 0; offset < upperBound;++offset)
         {
            *destBand = *srcBand;
            ++srcBand; ++destBand;
         }
      }
   }
   theTile->setDataObjectStatus(currentImageData->getDataObjectStatus());

   while(currentImageData.valid())
   {
      ossimDataObjectStatus currentStatus     = currentImageData->getDataObjectStatus();
      point = currentImageData->getOrigin();
      long h = (long)currentImageData->getHeight();
      long w = (long)currentImageData->getWidth();
      if( (currentStatus != OSSIM_EMPTY) &&
          (currentStatus != OSSIM_NULL))
      {
         ++numberOfTilesProcessed;
         offset = 0;
         minNumberOfBands = currentImageData->getNumberOfBands();
         for(band = 0; band < minNumberOfBands; ++band)
         {
            srcBands[band]  = static_cast<T*>(currentImageData->getBuf(band));
         }
         // if the src is smaller than the destination in number
         // of bands we will just duplicate the last band.
         for(;band < theLargestNumberOfInputBands; ++band)
         {
            srcBands[band]  = srcBands[minNumberOfBands - 1];
         }
          if(currentStatus == OSSIM_PARTIAL)
          {
            for(row = 0; row < h; ++row)
            {
               for(col = 0; col < w; ++col)
               {
                  if(!currentImageData->isNull(offset))
                  {
                     double weight = computeWeight(layerIdx,
                                                   ossimDpt(point.x+col,
                                                            point.y+row));
                     
                     for(band = 0; band < theLargestNumberOfInputBands; ++band)
                     {
                        bandRes = static_cast<float*>(theResult->getBuf(band));
                        bandRes[offset] += (srcBands[band][offset]*weight);
                     }
                     sumBand[offset] += weight;
                  }
                  ++offset;
               }
            }
         }
         else
         {
            offset = 0;
            
            for(row = 0; row < h; ++row)
            {
               for(col = 0; col < w; ++col)
               {
                     double weight = computeWeight(layerIdx,
                                                   ossimDpt(point.x+col,
                                                            point.y+row));
                     
                     for(band = 0; band < theLargestNumberOfInputBands; ++band)
                     {
                        bandRes     = static_cast<float*>(theResult->getBuf(band));
                        
                        bandRes[offset] += (srcBands[band][offset]*weight);
                     }
                     sumBand[offset] += weight;
                     ++offset;
               }
            }
         }
      }
      currentImageData = getNextTile(layerIdx, tileRect, resLevel);
   }
   upperBound = theTile->getWidth()*theTile->getHeight();

   if(numberOfTilesProcessed > 1)
   {
      const double* minPix = theTile->getMinPix();
      const double* maxPix = theTile->getMaxPix();
      const double* nullPix= theTile->getNullPix();
      for(offset = 0; offset < upperBound;++offset)
      {
         for(band = 0; band < theTile->getNumberOfBands();++band)
         {
            T* destBand      = static_cast<T*>(theTile->getBuf(band));
            float* weightedBand = static_cast<float*>(theResult->getBuf(band));

            // this should be ok to test 0.0 instead of
            // FLT_EPSILON range for 0 since we set it.
            if(sumBand[offset] != 0.0)
            {            
               weightedBand[offset] = (weightedBand[offset])/sumBand[offset];
               if(weightedBand[offset]<minPix[band])
               {
                  weightedBand[offset] = minPix[band];   
               }
               else if(weightedBand[offset] > maxPix[band])
               {
                  weightedBand[offset] = maxPix[band];                   
               }
            }
            else
            {
               weightedBand[offset] = nullPix[band];
            }
            destBand[offset] = static_cast<T>(weightedBand[offset]);
         }
      }
      theTile->validate();
   }

   delete [] srcBands;
   delete [] destBands;

   return theTile;
}

double ossimFeatherMosaic::computeWeight(long index,
                                         const ossimDpt& point)const
{
   ossimFeatherInputInformation& info = theInputFeatherInformation[index];
   const ossimDpt delta = point-info.theCenter;
   const bool validAxis1 = std::isfinite(info.theAxis1Length) &&
                           (info.theAxis1Length > 0.0);
   const bool validAxis2 = std::isfinite(info.theAxis2Length) &&
                           (info.theAxis2Length > 0.0);

   // At coarse display scales a small projected image can collapse to one
   // pixel after getValidImageVertices rounds to integer view coordinates.
   // It must still contribute to the mosaic instead of producing NaN weights.
   if(!validAxis1 && !validAxis2)
   {
      return 1.0;
   }

   double normalizedDistance = 0.0;
   if(validAxis1)
   {
      normalizedDistance = fabs(delta.x*info.theAxis1.x +
                                delta.y*info.theAxis1.y)/info.theAxis1Length;
   }
   if(validAxis2)
   {
      const double distance2 = fabs(delta.x*info.theAxis2.x +
                                    delta.y*info.theAxis2.y)/info.theAxis2Length;
      normalizedDistance = validAxis1 ?
         std::max(normalizedDistance, distance2) : distance2;
   }
   if(!std::isfinite(normalizedDistance))
   {
      return 1.0;
   }
   return std::max(0.0, std::min(1.0, 1.0-normalizedDistance));
}

void ossimFeatherMosaic::initialize()
{
   ossimImageMosaic::initialize();

   allocate();
   if(theTile.valid())
   {
      theAlphaSum = new ossimImageData(this,
                                       OSSIM_FLOAT,
                                       1,
                                       theTile->getWidth(),
                                       theTile->getHeight());
      theResult = new ossimImageData(this,
                                     OSSIM_FLOAT,
                                     theLargestNumberOfInputBands,
                                     theTile->getWidth(),
                                     theTile->getHeight());
      theAlphaSum->initialize();
      theResult->initialize();
   }
   std::vector<ossimIpt> validVertices;
   if(!getNumberOfInputs()) return;

   if(theInputFeatherInformation)
   {
      delete [] theInputFeatherInformation;
      theInputFeatherInformation = NULL;
   }
   theFeatherInfoSize = getNumberOfInputs();
   theInputFeatherInformation = new ossimFeatherInputInformation[theFeatherInfoSize];
   for(long index = 0; index < theFeatherInfoSize; ++ index)
   {
      validVertices.clear();
      ossimImageSource* temp = PTR_CAST(ossimImageSource, getInput(index));
      if(temp)
      {
         temp->getValidImageVertices(validVertices, OSSIM_CLOCKWISE_ORDER);
         theInputFeatherInformation[index].setVertexList(validVertices);
      }
   }
}


ossimFeatherMosaic::ossimFeatherInputInformation::ossimFeatherInputInformation(const std::vector<ossimIpt>& validVertices)
{
   setVertexList(validVertices);
}


void ossimFeatherMosaic::ossimFeatherInputInformation::setVertexList(const std::vector<ossimIpt>& validVertices)
{
   const char* MODULE = "ossimFeatherMosaic::ossimFeatherInputInformation::setVertexList()";
   
   theValidVertices = validVertices;

   theCenter       = ossimDpt(0,0);
   theAxis1        = ossimDpt(1, 0);
   theAxis2        = ossimDpt(0, 1);
   theAxis1Length  = 1;
   theAxis2Length  = 1;

   double xSum=0.0, ySum=0.0;
   ossim_uint32 upperBound = (ossim_uint32)validVertices.size();
   if(upperBound)
   {
      for(ossim_uint32 index = 0; index < upperBound; ++index)
      {
         xSum += validVertices[index].x;
         ySum += validVertices[index].y;
      }

      theCenter.x = xSum/upperBound;
      theCenter.y = ySum/upperBound;

      // for now we just want a quick implementation of something
      // and we know that we have 4 vertices for the bounding valid
      // vertices.
      //
      if(upperBound == 4)
      {
         ossimDpt edgeDirection1 = validVertices[1] - validVertices[0];
         ossimDpt edgeDirection2 = validVertices[2] - validVertices[1];

         const double edgeLength1 = edgeDirection1.length();
         const double edgeLength2 = edgeDirection2.length();
         if((edgeLength1 <= 0.0) || (edgeLength2 <= 0.0))
         {
            theAxis1Length = 0.0;
            theAxis2Length = 0.0;
            return;
         }

         theAxis1 = ossimDpt(-edgeDirection1.y, edgeDirection1.x)/edgeLength1;
         theAxis2 = ossimDpt(-edgeDirection2.y, edgeDirection2.x)/edgeLength2;

         ossimLine line1(theCenter,
                         theCenter + theAxis1*2);
         ossimLine line2(validVertices[1],
                         validVertices[0]);
         ossimLine line3(theCenter,
                         theCenter + theAxis2*2);
         ossimLine line4(validVertices[2],
                         validVertices[1]);
         
         ossimDpt intersectionPoint1 = line1.intersectInfinite(line2);
         ossimDpt intersectionPoint2 = line3.intersectInfinite(line4);

         
         theAxis1Length = (theCenter-intersectionPoint1).length();
         theAxis2Length = (theCenter-intersectionPoint2).length();

          if(traceDebug())
          {
             CLOG << "theAxis1Length:       " << theAxis1Length << endl
                  << "theAxis2Length:       " << theAxis2Length << endl
                  << "center:               " << theCenter      << endl;
          }
      }
   }
}

ostream& operator<<(ostream& out,
                    const ossimFeatherMosaic::ossimFeatherInputInformation& data)
{
   out << "center: " << data.theCenter << endl
       << "axis1:  " << data.theAxis1  << endl
       << "axis2:  " << data.theAxis2  << endl
       << "axis1_length: " << data.theAxis1Length << endl
       << "axis2_length: " << data.theAxis2Length << endl
       << "valid vertices: " << endl;
   std::copy(data.theValidVertices.begin(),
             data.theValidVertices.end(),
             std::ostream_iterator<ossimDpt>(out, "\n"));
   return out;
}

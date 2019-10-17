//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: ossimImageSharpenFilter.cpp 18021 2010-09-01 12:11:05Z gpotts $
#include <cstdlib> /* for abs() */
#include <ossim/imaging/ossimImageSharpenFilter.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/base/ossimNumericProperty.h>

RTTI_DEF1(ossimImageSharpenFilter, "ossimImageSharpenFilter", ossimImageSourceFilter);

// static const char* KERNEL_WIDTH_KW = "kernel_width";
// static const char* KERNEL_SIGMA_KW = "kernel_sigma";
static const char* SHARPEN_PERCENT_KW = "sharpen_percent";

ossimImageSharpenFilter::ossimImageSharpenFilter(ossimObject *owner)
    : ossimImageSourceFilter(owner),
      // theWidth(3),
      // theSigma(.5),
      m_sharpenPercent(0.0)
{
   // theConvolutionSource = new ossimConvolutionSource;
}

ossimImageSharpenFilter::~ossimImageSharpenFilter()
{
}

// void ossimImageSharpenFilter::setWidthAndSigma(ossim_uint32 w, ossim_float64 sigma)
// {
//    theWidth = w;
//    theWidth |= 1;
//    if(theWidth < 3) theWidth = 3;
//    theSigma = sigma;
//    buildConvolutionMatrix();
// }

void ossimImageSharpenFilter::setSharpenPercent(ossim_float64 percent)
{
   m_sharpenPercent = percent;
   buildConvolutionLuts();
}

ossimRefPtr<ossimImageData> ossimImageSharpenFilter::getTile( 
   const ossimIrect &tileRect,
   ossim_uint32 resLevel)
{
   if (!theInputConnection)
   {
      return NULL;
   }
   if (!isSourceEnabled() || (std::fabs(m_sharpenPercent) < FLT_EPSILON))
   {
      return theInputConnection->getTile(tileRect, resLevel);
   }

   if (!m_tile.valid())
   {
      allocate();
      if (!m_tile.valid()) // Throw exception???
      {
         return theInputConnection->getTile(tileRect, resLevel);
      }
   }
   ossimIrect requestRect(tileRect);
   requestRect.expand(ossimIpt(1, 1));
   m_tile->makeBlank();
   m_tile->setImageRectangle(tileRect);
   ossimRefPtr<ossimImageData> input = theInputConnection->getTile(requestRect, resLevel);

   if(input.valid())
   {
      switch(input->getScalarType())
      {
         case OSSIM_UINT8:
            sharpenLut(static_cast<ossim_uint8>(0), input, m_tile);
            break;
         case OSSIM_UINT9:
         case OSSIM_UINT10:
         case OSSIM_UINT11:
         case OSSIM_UINT12:
         case OSSIM_UINT13:
         case OSSIM_UINT14:
         case OSSIM_UINT15:
         case OSSIM_UINT16:
            sharpenLut(static_cast<ossim_uint16>(0), input, m_tile);
            break;
         case OSSIM_SINT16:
            sharpenLutRemap(static_cast<ossim_uint16>(0), input, m_tile);
            break;
         default:
            m_tile->loadTile(input.get());
            break;
      }
   }

   return m_tile;
     // return theConvolutionSource->getTile(tileRect, resLevel);   
}
void ossimImageSharpenFilter::allocate()
{
   if (theInputConnection)
   {
      m_tile = ossimImageDataFactory::instance()->create(this,
                                                          theInputConnection);
      m_tile->initialize();
   }
}

void ossimImageSharpenFilter::initialize()
{
   ossimImageSourceFilter::initialize();
   // if(theConvolutionSource->getInput()!=getInput())
   // {
   //    theConvolutionSource->disconnectAllInputs();
   //    theConvolutionSource->connectMyInputTo(0, getInput());
   // }
   // buildConvolutionMatrix();
   buildConvolutionLuts();
}

void ossimImageSharpenFilter::buildConvolutionLuts()
{
   // Gimp port of the sharpen kernel.  I will describe what appears to be happening in the Gimp code
   // and then port to work with any bit depths.
   //
   // the Laplacian kernel is a 3x3 matrix kernel where the center is 8 and the surrounding kernel weights are -1
   // by multiplying through the positive coefficient by 100/100 ( essentially 1) giving you
   //
   //    100*8*<pixel>/(100 - m_sharpenPercent*100)
   //    giving you
   //    800*<pixel>/(100 - m_sharpenPercent*100)
   //
   // If you didn't multiply by 1 then we would need to have a floating buffer and this would have the same
   // affect as 
   //
   //    8*<pixel>/(1 - m_sharpenPercent)
   //
   ossim_float64 fact = 100 - m_sharpenPercent*100.0;
   ossim_int32 size = 0;

   if( fact < FLT_EPSILON ) fact = 1.0;
   // precompute luts for 8, 11, 12 and 16 bit data
   //
   m_remapValue = false;
   if (theInputConnection && fact >= 1)
   {
      ossimScalarType outputScalarType = theInputConnection->getOutputScalarType();
      switch (outputScalarType)
      {
      case OSSIM_UINT8:
         size = 256;
         break;
      case OSSIM_UINT9:
         size = 512;
         break;
      case OSSIM_UINT10:
         size = 1024;
         break;
      case OSSIM_UINT11:
         size = 2048;
         break;
      case OSSIM_UINT12:
         size = 4096;
         break;
      case OSSIM_UINT13:
         size = 8192;
         break;
      case OSSIM_UINT14:
         size = 16384;
         break;
      case OSSIM_UINT15:
         size = 32768;
         break;
      case OSSIM_UINT16:
         size = 65536;
         break;
      default:
         // for remapping we will just a set of sample
         // values and then do a normalization
         // we will enable remap and do a 256 samples
         // for the laplacian.
         m_remapValue = true;
         size = 256;
      }
   }
   else
   {
      m_posLut.clear();
      m_posNegLut.clear();
   }

   if(size)
   {
      m_posLut.resize(size);
      m_posNegLut.resize(size);
   }
   
   for (ossim_int32 i = 0; i < (ossim_int32)m_posLut.size(); i++)
   {
      m_posLut[i]    = 800 * i / fact;
      m_posNegLut[i] = (4 + m_posLut[i] - (i << 3))/ 8.0;
   }
}

void ossimImageSharpenFilter::connectInputEvent(ossimConnectionEvent &event)
{
   ossimImageSourceFilter::connectInputEvent(event);
   buildConvolutionLuts();
}

void ossimImageSharpenFilter::disconnectInputEvent(ossimConnectionEvent &event)
{
   ossimImageSourceFilter::disconnectInputEvent(event);
}

ossimString ossimImageSharpenFilter::getShortName() const
{
   return ossimString("Sharpen");
}

ossimString ossimImageSharpenFilter::getLongName() const
{
   return ossimString("Sharpens the input");
}

template <class T>
void ossimImageSharpenFilter::sharpenLut(T,
                                      const ossimRefPtr<ossimImageData> &inputTile,
                                      ossimRefPtr<ossimImageData> &outputTile)
{
   ossimDataObjectStatus status = inputTile->getDataObjectStatus();
   ossim_int64 tileHeight     = outputTile->getHeight();
   ossim_int64 tileWidth      = outputTile->getWidth();
   // ossim_int64 inputTileHeight= inputTile->getHeight();
   ossim_int64 inputTileWidth = inputTile->getWidth();
   ossim_int64 outputBands    = outputTile->getNumberOfBands();
   // ossim_int64 convolutionTopLeftOffset = 0;
   // ossim_int64 convolutionMiddleLeftOffset = 0;
   // ossim_int64 convolutionBottomLeftOffset = 0;
   // ossim_int64 convolutionCenterOffset = inputTileWidth + 1; // kernel is a 3x3  so move it over by 1
   // ossim_int64 outputOffset = 0;
   std::vector<T *> inputBandBuf(outputBands);
   std::vector<T *> outputBandBuf(outputBands);

   for (ossim_int64 b = 0; b < outputBands; ++b)
   {
      inputBandBuf[b]  = (T *)(inputTile->getBuf(b));
      outputBandBuf[b] = (T *)(outputTile->getBuf(b));
   }

   if (status == OSSIM_NULL || status == OSSIM_EMPTY)
   {
      outputTile->makeBlank();
   }
   else if(status == OSSIM_FULL)
   {
      for (ossim_int64 b = 0; b < outputBands; ++b)
      {
         ossim_float64 minPix = outputTile->getMinPix(b);
         ossim_float64 maxPix = outputTile->getMaxPix(b);
         // convolutionTopLeftOffset = 0;
         // convolutionCenterOffset = convolutionTopLeftOffset + inputTileWidth + 1;
         // convolutionMiddleLeftOffset = convolutionTopLeftOffset + inputTileWidth;
         // convolutionBottomLeftOffset = convolutionMiddleLeftOffset + inputTileWidth;
         T *inBuf = (T *)(inputBandBuf[b]);
         T *outBuf = (T *)(outputBandBuf[b]);

         for (ossim_int64 y = 0; y < tileHeight; y++)
         {
            for (ossim_int64 x = 0; x < tileWidth; ++x, ++inBuf, ++outBuf)
            {
                  ossim_float64 convolveResult = 0.0;
                  ossim_float64 pos = m_posLut[*(inBuf + inputTileWidth + 1)];
                  ossim_float64 neg = m_posNegLut[*(inBuf)] + m_posNegLut[*(inBuf + 1)] + m_posNegLut[*(inBuf + 2)]; // top
                  neg += (m_posNegLut[*(inBuf + inputTileWidth)] + m_posNegLut[*(inBuf + inputTileWidth + 2)]);      // middle without positive
                  neg += (m_posNegLut[*(inBuf + (inputTileWidth << 1))] + 
                          m_posNegLut[*(inBuf + (inputTileWidth << 1) + 1)] +
                          m_posNegLut[*(inBuf + (inputTileWidth << 1) + 2)]); // bottom

                  convolveResult = ((pos - neg) + 4)/ 8.0;

                  convolveResult = convolveResult < minPix ? minPix : convolveResult;
                  convolveResult = convolveResult > maxPix ? maxPix : convolveResult;

                  *outBuf = (T)convolveResult;
            }

            inBuf +=2;
         }
      }
   }
   else
   {
      ossim_float64 inputValues[9];
      for (ossim_int64 b = 0; b < outputBands; ++b)
      {
         ossim_float64 minPix     = inputTile->getMinPix(b);
         ossim_float64 maxPix     = inputTile->getMaxPix(b);
         ossim_float64 nullPix    = inputTile->getNullPix(b);
         // convolutionTopLeftOffset = 0;
         // convolutionCenterOffset  = convolutionTopLeftOffset + inputTileWidth + 1;
         // convolutionMiddleLeftOffset = convolutionTopLeftOffset + inputTileWidth;
         // convolutionBottomLeftOffset = convolutionMiddleLeftOffset + inputTileWidth;
         T *inBuf = (T *)(inputBandBuf[b]);
         T *outBuf = (T *)(outputBandBuf[b]);

         for (ossim_int64 y = 0; y < tileHeight; y++)
         {
            for (ossim_int64 x = 0; x < tileWidth; ++x, ++inBuf, ++outBuf)
            {
               ossim_float64 convolveResult = 0.0;
               inputValues[0] = *(inBuf);
               inputValues[1] = *(inBuf + 1);
               inputValues[2] = *(inBuf + 2);
               inputValues[3] = *(inBuf + inputTileWidth);
               inputValues[4] = *(inBuf + inputTileWidth + 1);
               inputValues[5] = *(inBuf + inputTileWidth + 2);
               inputValues[6] = *(inBuf + (inputTileWidth<<1) );
               inputValues[7] = *(inBuf + (inputTileWidth<<1)+1);
               inputValues[8] = *(inBuf + (inputTileWidth<<1)+2);

               if((inputValues[0]==nullPix)||
                  (inputValues[1]==nullPix)||
                  (inputValues[2]==nullPix)||
                  (inputValues[3]==nullPix)||
                  (inputValues[4]==nullPix)||
                  (inputValues[5]==nullPix)||
                  (inputValues[6]==nullPix)||
                  (inputValues[7]==nullPix)||
                  (inputValues[8]==nullPix))
               {
                  *outBuf = static_cast<T>(inputValues[4]);
               } 
               else
               {
                  ossim_float64 pos = m_posLut[inputValues[4]];
                  ossim_float64 neg = m_posNegLut[inputValues[0]] + m_posNegLut[inputValues[1]] +
                                      m_posNegLut[inputValues[2]] + m_posNegLut[inputValues[3]] +
                                      m_posNegLut[inputValues[5]] + m_posNegLut[inputValues[6]] +
                                      m_posNegLut[inputValues[7]] + m_posNegLut[inputValues[8]];
                  convolveResult = ((pos - neg) + 4) / 8.0;

                  convolveResult = convolveResult < minPix ? minPix : convolveResult;
                  convolveResult = convolveResult > maxPix ? maxPix : convolveResult;

                  *outBuf = static_cast<T>(convolveResult);
                  /* code */
               }
            }

            inBuf += 2;
         }
      }
   }

   outputTile->validate();
}

template <class T>
void ossimImageSharpenFilter::sharpenLutRemap(T,
                                         const ossimRefPtr<ossimImageData> &inputTile,
                                         ossimRefPtr<ossimImageData> &outputTile)
{
   ossim_float64 inputValues[9];
   ossimDataObjectStatus status = inputTile->getDataObjectStatus();
   ossim_int64 tileHeight = outputTile->getHeight();
   ossim_int64 tileWidth = outputTile->getWidth();
   // ossim_int64 inputTileHeight = inputTile->getHeight();
   ossim_int64 inputTileWidth = inputTile->getWidth();
   ossim_int64 outputBands = outputTile->getNumberOfBands();
   // ossim_int64 convolutionTopLeftOffset = 0;
   // ossim_int64 convolutionMiddleLeftOffset = 0;
   // ossim_int64 convolutionBottomLeftOffset = 0;
   // kernel is a 3x3  so move it over by 1
   // ossim_int64 convolutionCenterOffset = inputTileWidth + 1;
   // ossim_int64 outputOffset = 0;
   ossim_float64 convolveResult = 0.0;
   std::vector<T *> inputBandBuf(outputBands);
   std::vector<T *> outputBandBuf(outputBands);

   for (ossim_int64 b = 0; b < outputBands; ++b)
   {
      inputBandBuf[b] = (T *)(inputTile->getBuf(b));
      outputBandBuf[b] = (T *)(outputTile->getBuf(b));
   }

   if (status == OSSIM_NULL || status == OSSIM_EMPTY)
   {
      outputTile->makeBlank();
   }
   else if (status == OSSIM_FULL) // whether partial or null we must check nulls over entire matrix
   {
      for (ossim_int64 b = 0; b < outputBands; ++b)
      {
         ossim_float64 minPix = outputTile->getMinPix(b);
         ossim_float64 maxPix = outputTile->getMaxPix(b);
         // convolutionTopLeftOffset = 0;
         // convolutionCenterOffset = convolutionTopLeftOffset + inputTileWidth + 1;
         // convolutionMiddleLeftOffset = convolutionTopLeftOffset + inputTileWidth;
         // convolutionBottomLeftOffset = convolutionMiddleLeftOffset + inputTileWidth;
         T *inBuf = (T *)(inputBandBuf[b]);
         T *outBuf = (T *)(outputBandBuf[b]);
         ossim_float64 deltaMinMax = (maxPix-minPix);
         // ossimScalarType inputScalar = inputTile->getScalarType();
         for (ossim_int64 y = 0; y < tileHeight; y++)
         {
            for (ossim_int64 x = 0; x < tileWidth; ++x, ++inBuf, ++outBuf)
            {
               inputValues[0] = *(inBuf);
               inputValues[1] = *(inBuf + 1);
               inputValues[2] = *(inBuf + 2);
               inputValues[3] = *(inBuf + inputTileWidth);
               inputValues[4] = *(inBuf + inputTileWidth + 1);
               inputValues[5] = *(inBuf + inputTileWidth + 2);
               inputValues[6] = *(inBuf + (inputTileWidth << 1));
               inputValues[7] = *(inBuf + (inputTileWidth << 1) + 1);
               inputValues[8] = *(inBuf + (inputTileWidth << 1) + 2);
               for (ossim_uint32 idx = 0; idx < 9; ++idx)
               {
                  inputValues[idx] = ((inputValues[idx] - minPix) / deltaMinMax) * m_posLut.size();
                  if (inputValues[idx] < 0)
                     inputValues[idx] = 0;
                  if (inputValues[idx] >= m_posLut.size())
                     inputValues[idx] = m_posLut.size() - 1;
               }

               ossim_float64 pos = m_posLut[inputValues[4]];
               ossim_float64 neg = m_posNegLut[inputValues[0]] + m_posNegLut[inputValues[1]] +
                                   m_posNegLut[inputValues[2]] + m_posNegLut[inputValues[3]] +
                                   m_posNegLut[inputValues[5]] + m_posNegLut[inputValues[6]] +
                                   m_posNegLut[inputValues[7]] + m_posNegLut[inputValues[8]];
               convolveResult = ((pos - neg) + 4) / 8.0;
               convolveResult = convolveResult < 0 ? 0 : convolveResult;
               convolveResult = convolveResult > (m_posLut.size() - 1) ? m_posLut.size() - 1 : convolveResult;
               ossim_float64 t = convolveResult / (m_posLut.size() - 1);
               *outBuf = static_cast<T>(minPix * (1 - t) + deltaMinMax * t);
            }

            inBuf += 2;
         }
      }
   }
   else
   {
      for (ossim_int64 b = 0; b < outputBands; ++b)
      {
         ossim_float64 minPix = outputTile->getMinPix(b);
         ossim_float64 maxPix = outputTile->getMaxPix(b);
         ossim_float64 nullPix = inputTile->getNullPix(b);
         // convolutionTopLeftOffset = 0;
         // convolutionCenterOffset = convolutionTopLeftOffset + inputTileWidth + 1;
         // convolutionMiddleLeftOffset = convolutionTopLeftOffset + inputTileWidth;
         // convolutionBottomLeftOffset = convolutionMiddleLeftOffset + inputTileWidth;
         T *inBuf = (T *)(inputBandBuf[b]);
         T *outBuf = (T *)(outputBandBuf[b]);
         ossim_float64 deltaMinMax = (maxPix - minPix);
         for (ossim_int64 y = 0; y < tileHeight; y++)
         {
            for (ossim_int64 x = 0; x < tileWidth; ++x, ++inBuf, ++outBuf)
            {
               inputValues[0] = *(inBuf);
               inputValues[1] = *(inBuf + 1);
               inputValues[2] = *(inBuf + 2);
               inputValues[3] = *(inBuf + inputTileWidth);
               inputValues[4] = *(inBuf + inputTileWidth + 1);
               inputValues[5] = *(inBuf + inputTileWidth + 2);
               inputValues[6] = *(inBuf + (inputTileWidth << 1));
               inputValues[7] = *(inBuf + (inputTileWidth << 1) + 1);
               inputValues[8] = *(inBuf + (inputTileWidth << 1) + 2);

               if ((inputValues[0] == nullPix) &&
                   (inputValues[1] == nullPix) &&
                   (inputValues[2] == nullPix) &&
                   (inputValues[3] == nullPix) &&
                   (inputValues[4] == nullPix) &&
                   (inputValues[5] == nullPix) &&
                   (inputValues[6] == nullPix) &&
                   (inputValues[7] == nullPix) &&
                   (inputValues[8] == nullPix))
               {
                  *outBuf = static_cast<T>(inputValues[4]);
               }
               else
               {
                  for (ossim_uint32 idx = 0; idx < 9; ++idx)
                  {
                     inputValues[idx] = ((inputValues[idx] - minPix) / deltaMinMax) * m_posLut.size();

                     if (inputValues[idx] < 0)
                        inputValues[idx] = 0;
                     if (inputValues[idx] >= m_posLut.size())
                        inputValues[idx] = m_posLut.size() - 1;
                  }

                  ossim_float64 pos = m_posLut[inputValues[4]];
                  ossim_float64 neg = m_posNegLut[inputValues[0]] + m_posNegLut[inputValues[1]] +
                                      m_posNegLut[inputValues[2]] + m_posNegLut[inputValues[3]] +
                                      m_posNegLut[inputValues[5]] + m_posNegLut[inputValues[6]] +
                                      m_posNegLut[inputValues[7]] + m_posNegLut[inputValues[8]];
                  convolveResult = ((pos - neg) + 4.0) / 8.0;
                  convolveResult = convolveResult < 0 ? 0 : convolveResult;
                  convolveResult = convolveResult > (m_posLut.size() - 1) ? m_posLut.size() - 1 : convolveResult;
                  ossim_float64 t = convolveResult / (m_posLut.size() - 1);
                  *outBuf = static_cast<T>(minPix * (1 - t) + deltaMinMax * t);
               }
            }

            inBuf += 2;
         }
      }
   }

   outputTile->validate();
}

void ossimImageSharpenFilter::setProperty(ossimRefPtr<ossimProperty> property)
{
   if(!property) return;
   if(property->getName() == SHARPEN_PERCENT_KW)
   {
      m_sharpenPercent = property->valueToString().toDouble();
      initialize();
   }
   else
   {
      ossimImageSourceFilter::setProperty(property.get());
   }
}

ossimRefPtr<ossimProperty> ossimImageSharpenFilter::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> property = 0;
   if (name == SHARPEN_PERCENT_KW)
   {
      property = new ossimNumericProperty(name,
                                          ossimString::toString(m_sharpenPercent),
                                          0.0,
                                          1.0);
      property->setCacheRefreshBit();
   }
   else
   {
      property = ossimImageSourceFilter::getProperty(name);
   }
   
   return property.get();
}

void ossimImageSharpenFilter::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   ossimImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back(SHARPEN_PERCENT_KW);
}

bool ossimImageSharpenFilter::loadState(const ossimKeywordlist& kwl,
                                        const char* prefix)
{
   bool result = ossimImageSourceFilter::loadState(kwl, prefix);
   ossimString sharpenPercent = kwl.find(prefix, SHARPEN_PERCENT_KW);
   if(!sharpenPercent.empty())
   {
      m_sharpenPercent = sharpenPercent.toDouble();
   }
   buildConvolutionLuts();

   return result;
}

bool ossimImageSharpenFilter::saveState(ossimKeywordlist& kwl,
                                        const char* prefix)const
{
   kwl.add(prefix, SHARPEN_PERCENT_KW, m_sharpenPercent, true);

   return ossimImageSourceFilter::saveState(kwl, prefix);
}

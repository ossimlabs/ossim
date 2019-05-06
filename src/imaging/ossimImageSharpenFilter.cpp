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
// $Id: ossimImageSharpenFilter.cpp 18021 2010-09-01 12:11:05Z gpotts $
#include <cstdlib> /* for abs() */
#include <ossim/imaging/ossimImageSharpenFilter.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/base/ossimNumericProperty.h>

RTTI_DEF1(ossimImageSharpenFilter, "ossimImageSharpenFilter", ossimImageSourceFilter);

static const char* KERNEL_WIDTH_KW = "kernel_width";
static const char* KERNEL_SIGMA_KW = "kernel_sigma";
static const char* SHARPEN_PERCENT_KW = "sharpen_percent";

ossimImageSharpenFilter::ossimImageSharpenFilter(ossimObject *owner)
    : ossimImageSourceFilter(owner),
      theWidth(3),
      theSigma(.5),
      m_sharpenPercent(0.0)
{
   theConvolutionSource = new ossimConvolutionSource;
}

ossimImageSharpenFilter::~ossimImageSharpenFilter()
{
}

void ossimImageSharpenFilter::setWidthAndSigma(ossim_uint32 w, ossim_float64 sigma)
{
   theWidth = w;
   theWidth |= 1;
   if(theWidth < 3) theWidth = 3;
   theSigma = sigma;
   buildConvolutionMatrix();
}

ossimRefPtr<ossimImageData> ossimImageSharpenFilter::getTile(
   const ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return NULL;
   }
   if (!isSourceEnabled() || (std::fabs(m_sharpenPercent) < FLT_EPSILON))
   {
      return theInputConnection->getTile(tileRect, resLevel);
   }

   ossimIrect requestRect(tileRect);
   requestRect.expand(ossimIpt(1,1));

   ossimRefPtr<ossimImageData> result = theInputConnection->getTile(requestRect, resLevel);
   
   
  // return theConvolutionSource->getTile(tileRect, resLevel);   
}

void ossimImageSharpenFilter::initialize()
{
   ossimImageSourceFilter::initialize();
   if(theConvolutionSource->getInput()!=getInput())
   {
      theConvolutionSource->disconnectAllInputs();
      theConvolutionSource->connectMyInputTo(0, getInput());
   }
   buildConvolutionMatrix();
   buildConvolutionLuts();
}

void ossimImageSharpenFilter::buildConvolutionMatrix()
{
   std::vector<double> theKernel(theWidth*theWidth);
   double* kernel = &theKernel.front();
   //width = findWidth(sigma);
   ossim_int32 i = 0 ;
   ossim_int32 u = 0 ;
   ossim_int32 v = 0 ;
   double normalize = 0.0 ;
   ossim_int32 w2 = theWidth>>1;
   for (v = -w2; v <= w2; ++v)
   {
      for (u = -w2; u <= w2; ++u)
      {
         double value = laplacianOfGaussian(u, v, theSigma);
         kernel[i] = value;
         normalize += kernel[i] ;
         ++i ;
      }
   }
   if (fabs(normalize) <= 1e-6)
   {
      normalize = 1.0;
   }
   normalize=(1.0/normalize);
   ossim_int32 size = static_cast<ossim_int32>(theWidth*theWidth);
   for (i = 0; i < size; ++i)
   {
      kernel[i]=kernel[i]*normalize;
   } 
      
#if 0
   // print the kernel
   for (i = 0; i < theWidth*theWidth; ++i)
   {
      if((i%theWidth)==0)
      {
         std::cout << std::endl;
      }
      std::cout << kernel[i] <<", ";
   } 
   std::cout << std::endl;
#endif
   theConvolutionSource->setConvolution(kernel, theWidth, theWidth, false);
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
   ossim_int32 fact = static_cast<ossim_int32>(100 - m_sharpenPercent*100.0);

   // precompute luts for 8, 11 and 12 bit data
   //
   if (theInputConnection&&fact>=1)
   {
      ossimScalarType outputScalarType = theInputConnection->getOutputScalarType();
      ossim_int32 size = 0;
      switch(outputScalarType)
      {
         case OSSIM_UINT8:                  /**<  8 bit unsigned integer */
         {
            size = 256;
            break;
         }
         case OSSIM_UINT11:              /**< 16 bit unsigned integer (9 bits used) */
         {
            size = 2048;
         }
         case OSSIM_UINT12:
         {
            size = 4096;
         }
      }
   }
   else
   {
      m_posLut.clear();
      m_posNegLut.clear();
   }

   for (ossim_int32 i = 0; i < m_posLut.size(); i++)
   {
      m_posLut[i]    = 800 * i / fact;
      m_posNegLut[i] = (4 + m_posLut[i] - (i << 3)) >> 3;
   }
}

void ossimImageSharpenFilter::connectInputEvent(ossimConnectionEvent &event)
{
   ossimImageSourceFilter::connectInputEvent(event);
   theConvolutionSource->connectMyInputTo(0, getInput());
   buildConvolutionMatrix();
}

void ossimImageSharpenFilter::disconnectInputEvent(ossimConnectionEvent &event)
{
   ossimImageSourceFilter::disconnectInputEvent(event);
   theConvolutionSource->connectMyInputTo(0, getInput());
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
void ossimImageSharpenFilter::sharpen(T,
                                      const ossimRefPtr<ossimImageData> &inputData,
                                      ossimRefPtr<ossimImageData> &outputData)
{

}

void ossimImageSharpenFilter::setProperty(ossimRefPtr<ossimProperty> property)
{
   //std::cout << "ossimImageSharpenFilter::setProperty with name = " << property->getName() << std::endl;
   if(!property) return;
   if(property->getName() == KERNEL_WIDTH_KW)
   {
      theWidth = property->valueToString().toUInt32();
      theWidth |=1;
      if(theWidth < 3) theWidth = 3;
      initialize();
   }
   else if(property->getName() == KERNEL_SIGMA_KW)
   {
      theSigma = property->valueToString().toDouble();
      if(theSigma < .1) theSigma = .1;
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
   if(name == KERNEL_WIDTH_KW)
   {
      property = new ossimNumericProperty(name,
                                          ossimString::toString(theWidth),
                                          3.0,
                                          64.0);
      property->setCacheRefreshBit();
      
   }
   else if (name == KERNEL_SIGMA_KW)
   {
      property = new ossimNumericProperty(name,
                                          ossimString::toString(theSigma),
                                          .1,
                                          32);
      property->setCacheRefreshBit();
   }
   else if (name == SHARPEN_PERCENT_KW)
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
   propertyNames.push_back(KERNEL_WIDTH_KW);
   propertyNames.push_back(KERNEL_SIGMA_KW);
}

bool ossimImageSharpenFilter::loadState(const ossimKeywordlist& kwl,
                                        const char* prefix)
{
   bool result = ossimImageSourceFilter::loadState(kwl, prefix);
   ossimString kernelWidth = kwl.find(prefix, KERNEL_WIDTH_KW);
   ossimString kernelSigma = kwl.find(prefix, KERNEL_SIGMA_KW);
   kernelWidth = kernelWidth.trim();
   kernelSigma = kernelSigma.trim();
   if(!kernelWidth.empty())
   {
      theWidth = kernelWidth.toUInt32();
      theWidth |= 1; // make it odd and check for size
      if(theWidth < 3) theWidth = 3;
   }
   if(!kernelSigma.empty())
   {
      theSigma = kernelSigma.toFloat64();
   }
   buildConvolutionMatrix();
   return result;
}

bool ossimImageSharpenFilter::saveState(ossimKeywordlist& kwl,
                                        const char* prefix)const
{
   kwl.add(prefix, KERNEL_WIDTH_KW, theWidth, true);
   kwl.add(prefix, KERNEL_SIGMA_KW, theSigma, true);
   return ossimImageSourceFilter::saveState(kwl, prefix);
}

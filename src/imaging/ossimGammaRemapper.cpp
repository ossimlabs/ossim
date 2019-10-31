//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc. 
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
//*************************************************************************
// $Id: ossimGammaRemapper.cpp 12980 2008-06-04 00:50:33Z dburken $

#include <cstdlib>
#include <cmath>

#include <ossim/imaging/ossimGammaRemapper.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/imaging/ossimImageDataFactory.h>


RTTI_DEF1(ossimGammaRemapper, "ossimGammaRemapper", ossimImageSourceFilter)

static const char GAMMA_KW[] = "gamma";
const ossim_float64 ossimGammaRemapper::MIN_GAMMA=0.0001;
const ossim_float64 ossimGammaRemapper::MAX_GAMMA=10.0;
const ossim_float64 ossimGammaRemapper::DEFAULT_GAMMA = 1.0;

static ossimTrace traceDebug("ossimGammaRemapper:debug");

ossimGammaRemapper::ossimGammaRemapper()
   :
      m_gamma(DEFAULT_GAMMA),
      m_dirtyFlag(true)
{
}

ossimGammaRemapper::ossimGammaRemapper(const double &gamma)
   : 
      m_gamma(gamma),
      m_dirtyFlag(true)
{

}

ossimGammaRemapper::~ossimGammaRemapper()
{
}

ossimRefPtr<ossimImageData> ossimGammaRemapper::getTile(const ossimIrect& tileRect,
                                            ossim_uint32 resLevel)
{
   ossimRefPtr<ossimImageData> result = 0;

   if ( theInputConnection )
   {
      ossimRefPtr<ossimImageData> inputTile = theInputConnection->getTile(tileRect, resLevel);


      if(!m_tile||!m_tile->getBuf())
      {
         allocate();
      }

      if (!m_tile || ossim::almostEqual(m_gamma, 0.0))
         return inputTile;

      m_tile->setImageRectangle(tileRect);
      m_tile->makeBlank();
      if ( !inputTile.valid() ||
         (inputTile->getDataObjectStatus() == OSSIM_NULL) ||
         (inputTile->getDataObjectStatus() == OSSIM_EMPTY) )
      {
         //---
         // Since the filter is enabled, return theTile which is of the
         // correct scalar type.
         //---
         return m_tile;
      }
      if (m_dirtyFlag)
      {
         computeLookup();
      }

      if(!m_lookupTable.empty())
      {
         calculateGammaWithLookup(inputTile);
      }
      else
      {
         calculateGamma(inputTile);
      }
      m_tile->validate();

      result = m_tile;
   }

   return result;
}

void ossimGammaRemapper::calculateGammaWithLookup(ossimRefPtr<ossimImageData> inputTile)
{
   switch(inputTile->getScalarType())
   {
      case OSSIM_UINT8:
      {
         calculateGammaWithLookupTemplate(inputTile, (ossim_uint8)0);
         break;
      }
      case OSSIM_UINT9:
      case OSSIM_UINT10:
      case OSSIM_UINT11:
      case OSSIM_UINT12:
      case OSSIM_UINT13:
      case OSSIM_UINT14:
      case OSSIM_UINT15:
      case OSSIM_UINT16:
      {
         calculateGammaWithLookupTemplate(inputTile, (ossim_uint16)0);
         break;
      }
      default:
      {
         break;
      }
   }
}


template<class T>
void ossimGammaRemapper::calculateGammaWithLookupTemplate(ossimRefPtr<ossimImageData> inputTile, T /*dummy*/)
{
   T* input  = reinterpret_cast<T*>(inputTile->getBuf());
   T* output = reinterpret_cast<T*>(m_tile->getBuf());
   T nullPix = static_cast<T>(inputTile->getNullPix(0));
   ossim_uint32 nValues = inputTile->getNumberOfBands()*inputTile->getImageRectangle().area();
   ossim_uint32 idx = 0;
   ossim_float32* table = &m_lookupTable.front();
   if(inputTile->getDataObjectStatus() == OSSIM_PARTIAL)
   {
      for(idx = 0; idx < nValues; ++idx,++input,++output)
      {
         if(*input != nullPix)
         {
            *output = table[*input];
         }
      }
   }
   else
   {
      for(idx = 0; idx < nValues; ++idx,++input,++output)
      {
         *output = table[*input];
      }
   }
}

void ossimGammaRemapper::calculateGamma(ossimRefPtr<ossimImageData> inputTile)
{
   m_normalizedTile->setImageRectangle(inputTile->getImageRectangle());
   m_normalizedTile->makeBlank();
   ossim_float64 minPix = ossim::defaultMin(OSSIM_NORMALIZED_FLOAT);
   ossim_float64 maxPix = ossim::defaultMax(OSSIM_NORMALIZED_FLOAT);

   ossim_float64 delta = maxPix;

   inputTile->copyTileToNormalizedBuffer(reinterpret_cast<ossim_float32*>(m_normalizedTile->getBuf()));
   ossim_float32* input = reinterpret_cast<ossim_float32*>(m_normalizedTile->getBuf());
   ossim_uint32 idx = 0;
   ossim_float64 v = 0.0;
   ossim_uint32 nValues = m_normalizedTile->getNumberOfBands()*inputTile->getImageRectangle().area();

   if(inputTile->getDataObjectStatus() == OSSIM_PARTIAL)
   {
      for(idx = 0; idx < nValues; ++idx,++input)
      {
         if(*input != 0.0)
         {
            v = std::pow(*input, m_gamma)*delta;
            if(v > maxPix) v = maxPix;
            if(v < minPix) v = minPix;
            *input = v;
         }
      }
   }
   else
   {
      for(idx = 0; idx < nValues; ++idx,++input)
      {
         v = std::pow(*input, m_gamma)*delta;
         if(v > maxPix) v = maxPix;
         if(v < minPix) v = minPix;
         *input = v;
      }

   }

   m_tile->copyNormalizedBufferToTile(reinterpret_cast<ossim_float32*>(m_normalizedTile->getBuf()));
   m_tile->validate();
}

void ossimGammaRemapper::initialize()
{
   ossimImageSourceFilter::initialize();
   m_tile = 0;
   m_dirtyFlag = true;
   m_lookupTable.clear();
}

void ossimGammaRemapper::allocate()
{
   m_tile = ossimImageDataFactory::instance()->create(this,this);
   if(m_tile.valid())
   {
      m_tile->initialize();
      m_normalizedTile = new ossimImageData(0, OSSIM_NORMALIZED_FLOAT, m_tile->getNumberOfBands(), m_tile->getWidth(), m_tile->getHeight());
      m_normalizedTile->initialize();
   }
}
void ossimGammaRemapper::computeLookup()
{
   ossim_uint32 nValues=0;
   ossimScalarType scalarType = getOutputScalarType();
   switch(scalarType)
   {
      case OSSIM_UINT8:
      case OSSIM_UINT9:
      case OSSIM_UINT10:
      case OSSIM_UINT11:
      case OSSIM_UINT12:
      case OSSIM_UINT13:
      case OSSIM_UINT14:
      case OSSIM_UINT15:
      case OSSIM_UINT16:
      {
         nValues = ossim::defaultMax(scalarType)+1;
      }
      default:
      {
         break;
      }
   }
   if(nValues)
   {
      ossim_uint32 idx = 0;
      ossim_float64 maxPix = nValues-1;
      ossim_float64 outputMaxPix = getMaxPixelValue();
      ossim_float64 outputMinPix = getMinPixelValue();

      ossim_float64 v = 0.0;
      m_lookupTable.resize(nValues);
      for(idx = 0; idx < m_lookupTable.size();++idx)
      {
         ossim_float64 input = static_cast<double>(idx)/maxPix;
         v = std::pow(input, m_gamma)*maxPix;
         if(v > outputMaxPix) v = outputMaxPix;
         if(v < outputMinPix) v = outputMinPix;
         m_lookupTable[idx] = v;
      }
   }
   m_dirtyFlag = false;
}

bool ossimGammaRemapper::loadState(const ossimKeywordlist& kwl,
                                   const char* prefix)
{
   bool result = ossimImageSourceFilter::loadState(kwl, prefix);

   ossimString lookup = kwl.find(prefix, GAMMA_KW);

   if(!lookup.empty())
   {
      setGamma(lookup.toFloat64());
   }   

   return result;
}

bool ossimGammaRemapper::saveState(ossimKeywordlist& kwl,
                            const char* prefix)
{
   bool result = ossimImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix, GAMMA_KW, getGamma(), true);

   return result;
}

void ossimGammaRemapper::setGamma(const double& gamma)
{
   m_gamma = gamma;
   if(m_gamma < MIN_GAMMA) m_gamma = MIN_GAMMA;
   if(m_gamma > MAX_GAMMA) m_gamma = MAX_GAMMA;

   m_dirtyFlag = true;
}

ossimString ossimGammaRemapper::getShortName() const
{
   return ossimString("Gamma Remapper");
}

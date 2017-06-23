//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Oscar Kramer
//
//*************************************************************************
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/base/ossimRgbVector.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimFilenameProperty.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimNumericProperty.h>
#include <ossim/imaging/ossimLinearStretchRemapper.h>

RTTI_DEF1(ossimLinearStretchRemapper, "ossimLinearRemapper", ossimImageSourceFilter);

static const char* MIN_VALUE_KW = "min_value";
static const char* MAX_VALUE_KW = "max_value";

ossimLinearStretchRemapper::ossimLinearStretchRemapper()
{
   setDescription("Remaps input from min-max to full range (0-1) of normalized float..");
}

ossimLinearStretchRemapper::~ossimLinearStretchRemapper()
{
}

ossimRefPtr<ossimImageData> ossimLinearStretchRemapper::getTile(const ossimIrect& tileRect,
                                                              ossim_uint32 resLevel)
{
   if(!theInputConnection)
      return 0;

   ossimRefPtr<ossimImageData> tile = theInputConnection->getTile(tileRect, resLevel);
   if ( !theEnableFlag )
      return tile;

   if (!tile || !tile->getBuf())
      return 0;
   if(!m_tile)
   {
      initialize();
      if (!m_tile)
         return 0;
   }

   m_tile->setImageRectangle(tileRect);
   m_tile->makeBlank();

   // Quick handling special case of empty input tile:
   if (tile->getDataObjectStatus() == OSSIM_EMPTY)
      return m_tile;

   ossim_uint32 maxLength = tile->getWidth()*tile->getHeight();
   double pixel;
   double null_pixel = theInputConnection->getNullPixelValue();

   ossim_uint32 numBands = theInputConnection->getNumberOfOutputBands();
   for (int i=0; i<numBands; ++i)
   {
      double delta = m_maxValues[i] - m_minValues[i];
      if (delta == 0)
         continue;
      double* outBuf = m_tile->getDoubleBuf(i);
      for (ossim_uint32 offset=0; offset<maxLength; ++offset)
      {
         // Convert input pixel to a double index value:
         switch(tile->getScalarType())
         {
         case OSSIM_DOUBLE:
         case OSSIM_NORMALIZED_DOUBLE:
            pixel = tile->getDoubleBuf(i)[offset];
            break;
         case OSSIM_SSHORT16:
            pixel = (double)(tile->getSshortBuf(i)[offset]);
            break;
         case OSSIM_FLOAT:
         case OSSIM_NORMALIZED_FLOAT:
            pixel = (double)(tile->getFloatBuf(i)[offset]);
            break;
         case OSSIM_UCHAR:
            pixel = (double)(tile->getUcharBuf(i)[offset]);
            break;
         case OSSIM_USHORT16:
         case OSSIM_USHORT11:
         case OSSIM_USHORT12:
         case OSSIM_USHORT13:
         case OSSIM_USHORT14:
         case OSSIM_USHORT15:
            pixel = (double)(tile->getUshortBuf(i)[offset]);
            break;
         default:
            break;
         }

         // Do not remap null pixels, leave the output pixel "blank" which is null-pixel value:
         if (pixel == null_pixel)
            continue;

         // Do linear remap:
         pixel = (pixel-m_minValues[i])/delta;
         if (pixel < 0.0)
            pixel = 0.0;
         if (pixel > 1.0)
            pixel = 1.0;
         outBuf[offset] = pixel;
      }
   }
   m_tile->validate();
   return m_tile;
}

void ossimLinearStretchRemapper::allocate()
{
   if(!theInputConnection)
      return;

   m_tile = ossimImageDataFactory::instance()->create(this,
                                                      OSSIM_NORMALIZED_DOUBLE,
                                                      theInputConnection->getNumberOfOutputBands(),
                                                      theInputConnection->getTileWidth(),
                                                      theInputConnection->getTileHeight());
   if(m_tile.valid())
   {
      m_tile->initialize();
   }
}

void ossimLinearStretchRemapper::initialize()
{
   // This assigns theInputConnection if one is there.
   ossimImageSourceFilter::initialize();

   m_tile = 0;
   if ( theInputConnection )
   {
      // Initialize the chain on the left hand side of us.
      //theInputConnection->initialize(); This is done by base class
      int numBands = theInputConnection->getNumberOfOutputBands();
      if ((m_minValues.size() != numBands) && (m_maxValues.size() != numBands))
      {
         m_minValues.resize(numBands);
         m_maxValues.resize(numBands);
         for (int i=0; i<numBands; ++i)
         {
            m_minValues[i] = theInputConnection->getMinPixelValue(i);
            m_maxValues[i] = theInputConnection->getMaxPixelValue(i);
         }
      }
      allocate();
   }
}

bool ossimLinearStretchRemapper::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   bool rtn_stat = true;
   ostringstream minstr, maxstr;
   int numBands = m_minValues.size();
   if ((numBands == 0) || (numBands != m_maxValues.size()))
      rtn_stat = false;

   for (int i=0; i<numBands; ++i)
   {
      minstr << " " << m_minValues[i];
      maxstr << " " << m_maxValues[i];
   }

   kwl.add(prefix, MIN_VALUE_KW, minstr.str().c_str(), true);
   kwl.add(prefix, MAX_VALUE_KW, maxstr.str().c_str(), true);

   rtn_stat &= ossimImageSourceFilter::saveState(kwl, prefix);
   return rtn_stat;
}

bool ossimLinearStretchRemapper::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   bool return_state = true;
   m_minValues.clear();
   m_maxValues.clear();

   vector<ossimString> minimums;
   vector<ossimString> maximums;
   ossimString minVals = kwl.find(prefix, MIN_VALUE_KW);
   ossimString maxVals = kwl.find(prefix, MAX_VALUE_KW);
   if(!minVals.empty() && !maxVals.empty())
   {
      minimums = minVals.split(" ");
      maximums = maxVals.split(" ");
      if (minimums.size() != maximums.size())
         return false;

      for (int i=0; i<minimums.size(); ++i)
      {
         m_minValues[i] = minimums[i].toDouble();
         m_maxValues[i] = maximums[i].toDouble();
      }
   }
   return true;
}

double ossimLinearStretchRemapper::getMinPixelValue(ossim_uint32 band)const
{
   if (band< m_minValues.size())
      return m_minValues[band];
   return ossim::nan();
}

double ossimLinearStretchRemapper::getMaxPixelValue(ossim_uint32 band)const
{
   if (band< m_maxValues.size())
      return m_maxValues[band];
   return ossim::nan();
}

void ossimLinearStretchRemapper::setMinPixelValue(double value, ossim_uint32 band)
{
   if (m_minValues.size() <= band)
      m_minValues.resize(band+1);
   m_minValues[band] = value;
}

void ossimLinearStretchRemapper::setMaxPixelValue(double value, ossim_uint32 band)
{
   if (m_maxValues.size() <= band)
      m_maxValues.resize(band+1);
   m_maxValues[band] = value;
}




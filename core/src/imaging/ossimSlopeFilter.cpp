//*******************************************************************
// OSSIM
//
// License:  See top level LICENSE.txt file.
// 
//*******************************************************************
//  $Id: ossimSlopeFilter.cpp 23451 2015-07-27 15:42:17Z okramer $

#include <iostream>

#include <ossim/imaging/ossimSlopeFilter.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimStringProperty.h>

RTTI_DEF1(ossimSlopeFilter, "ossimSlopeFilter", ossimImageSourceFilter)

const char* SLOPE_TYPE_KW = "slope_type";
   
ossimSlopeFilter::ossimSlopeFilter()
   :  ossimImageSourceFilter(),
      m_slopeType (DEGREES)
{
}

ossimSlopeFilter::ossimSlopeFilter(ossimImageSource* inputSource)
   :
   ossimImageSourceFilter(inputSource),
   m_slopeType (DEGREES)
{
}

ossimSlopeFilter::~ossimSlopeFilter()
{
   m_normals = 0;
}

ossimRefPtr<ossimImageData> ossimSlopeFilter::getTile(const ossimIrect& rect, ossim_uint32 rLevel)
{
   if(!theInputConnection)
      return ossimRefPtr<ossimImageData>();

   if ( !isSourceEnabled() )
      return theInputConnection->getTile(rect, rLevel);

   if (!m_normals.valid())
      initialize();

   ossimRefPtr<ossimImageData> normals = m_normals->getTile(rect, rLevel);
   if (!normals.valid())
      return ossimRefPtr<ossimImageData>();

   ossimRefPtr<ossimImageData> outputTile = new ossimImageData(this, OSSIM_FLOAT32, 1);
   outputTile->setImageRectangle(rect);
   outputTile->initialize();
   ossim_float32* output_buf = outputTile->getFloatBuf();
   ossim_float32 null_output = (ossim_float32) outputTile->getNullPix(0);

   double z, theta;
   double null_input = normals->getNullPix(2);
   ossim_uint32 num_pix = normals->getSizePerBand();
   for (ossim_uint32 i=0; i<num_pix; ++i)
   {
      z = normals->getPix(i, 2);
      if (z == null_input)
      {
         theta = null_output;
      }
      else
      {
         switch (m_slopeType)
         {
         case RADIANS:
            theta = acos(z);
            break;
         case RATIO:
            theta = z;
            break;
         case NORMALIZED:
            theta = fabs(acos(z)/M_PI);
            break;
         default: // Degrees
            theta = ossim::acosd(z);
         };
      }
      output_buf[i] = theta;
   }

   outputTile->validate();
   return outputTile;
}

void ossimSlopeFilter::initialize()
{
   if (!m_normals.valid())
      m_normals = new ossimImageToPlaneNormalFilter(theInputConnection);
   m_normals->initialize();
}

void ossimSlopeFilter::setProperty(ossimRefPtr<ossimProperty> property)
{
   if(!property) return;

   if(property->getName() == SLOPE_TYPE_KW)
   {
      ossimString pts (property->valueToString());
      pts.upcase();
      if (pts.contains("RADIANS"))
         m_slopeType = RADIANS;
      else if (pts.contains("DEGREES"))
         m_slopeType = DEGREES;
      else if (pts.contains("RATIO"))
         m_slopeType = RATIO;
      else if (pts.contains("NORMALIZED"))
         m_slopeType = NORMALIZED;
      else
         return;

      initialize();
   }
   else
   {
      ossimImageSourceFilter::setProperty(property);
   }
}

ossimRefPtr<ossimProperty> ossimSlopeFilter::getProperty(const ossimString& name)const
{
   if(name == SLOPE_TYPE_KW)
   {
      ossimString propValue;
      switch (m_slopeType)
      {
      case RADIANS:
         propValue = getSlopeTypeString(RADIANS);
         break;
      case RATIO:
         propValue = getSlopeTypeString(RATIO);
         break;
      case NORMALIZED:
         propValue = getSlopeTypeString(NORMALIZED);
      default:
         propValue = getSlopeTypeString(DEGREES);
      }

      vector<ossimString> list;
      list.push_back(getSlopeTypeString(RADIANS));
      list.push_back(getSlopeTypeString(DEGREES));
      list.push_back(getSlopeTypeString(RATIO));
      list.push_back(getSlopeTypeString(NORMALIZED));
      
      return new ossimStringProperty(SLOPE_TYPE_KW, propValue, false, list);
   }

   return ossimImageSourceFilter::getProperty(name);
}

void ossimSlopeFilter::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   ossimImageSourceFilter::getPropertyNames(propertyNames);
   propertyNames.push_back(SLOPE_TYPE_KW);
}

bool ossimSlopeFilter::saveState(ossimKeywordlist& kwl, const char* prefix) const
{
   ossimImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix, SLOPE_TYPE_KW, getSlopeTypeString(m_slopeType).c_str(), true);

   return true;
}

bool ossimSlopeFilter::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimImageSourceFilter::loadState(kwl, prefix);

   ossimString lookup = kwl.find(prefix, SLOPE_TYPE_KW);
   if (!lookup.empty())
   {
      ossimRefPtr<ossimProperty> prop =  new ossimStringProperty(SLOPE_TYPE_KW, lookup, false);
      setProperty(prop);
   }

   return true;
}

ossimString ossimSlopeFilter::getSlopeTypeString(SlopeType t)
{
   if (t == RADIANS)
      return "RADIANS";
   if (t == DEGREES)
      return "DEGREES";
   if (t == RATIO)
      return "RATIO";
   if (t == NORMALIZED)
      return "NORMALIZED";
   return "";
}

ossimString ossimSlopeFilter::getLongName()const
{
   return ossimString("Slope Filter, Computes the change in pixel value in x, Y direction as a "
         "slope quantity. Assumes that the input source represents elevation in meters.");
}

ossimString ossimSlopeFilter::getShortName()const
{
   return ossimString("Slope Filter");
}


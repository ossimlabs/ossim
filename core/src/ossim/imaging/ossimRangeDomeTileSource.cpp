/*
 * ossimRangeDomeTileSource.cpp
 *
 *  Created on: Aug 3, 2015
 *      Author: okramer
 */

#include <ossim/imaging/ossimRangeDomeTileSource.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <iostream>

using namespace std;

RTTI_DEF1(ossimRangeDomeTileSource, "ossimRangeDomeTileSource", ossimImageHandler);

const char* OSSIM_RANGE_DOME_SPEC_MAGIC_NUMBER = "OSSIM_RANGE_DOMES";

ossimRangeDome::ossimRangeDome(vector<ossimString>& tokens)
: valid (false),
  id (0),
  radiusMeters (0),
  radiusPixelsSq (0),
  classification (UNASSIGNED),
  startAz (0),
  endAz (0)
{
   if ((tokens.size() < 6) || (tokens[0].size() && (tokens[0].chars()[0] == '#')))
      return;

   ossim_uint32 i = 0;
   id             = tokens[i++].toUInt32();
   centerGpt.lat  = tokens[i++].toDouble();
   centerGpt.lon  = tokens[i++].toDouble();
   centerGpt.hgt  = tokens[i++].toDouble();
   radiusMeters   = tokens[i++].toDouble();
   classification = (Classification) tokens[i++].toInt32();
   valid = true;

   if (tokens.size() >= 8)
   {
      startAz = tokens[i++].toDouble();
      endAz = tokens[i++].toDouble();
      if (endAz == 360.0)
         endAz = 0.0;
   }

   if ( tokens.size()-i )
   {
      description = tokens[i];
      description.trim();
   }
}

ossimGrect ossimRangeDome::boundingRect() const
{
   ossimDpt scale = centerGpt.metersPerDegree();
   double dlat = radiusMeters/scale.y;
   double dlon = radiusMeters/scale.x;
   return ossimGrect(centerGpt.lat+dlat, centerGpt.lon-dlon,
                     centerGpt.lat-dlat, centerGpt.lon+dlon);
}

ossimRangeDomeTileSource::ossimRangeDomeTileSource()
:  m_gsd(1.0, 1.0) // default GSD
{
}


ossimRangeDomeTileSource::~ossimRangeDomeTileSource()
{
   close();
}


bool ossimRangeDomeTileSource::open()
{
   if (!theImageFile.isReadable())
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"ossimRangeDomeTileSource::open() -- Error. Coould not"
            " open CSV file at <"<<theImageFile<<">" << endl;
      return false;
   }

   ossimString dome_spec;
   vector<ossimString> tokens;

   // Open the CSV and check proper file type:
   ifstream indata (theImageFile.chars());
   getline(indata, dome_spec);
   dome_spec.split(tokens, " ", false);
   if ((tokens.size() < 2) || !tokens[0].contains(OSSIM_RANGE_DOME_SPEC_MAGIC_NUMBER))
      return false;

   // loop over each record/dome spec:
   while (indata.good())
   {
      tokens.clear();
      getline(indata, dome_spec);
      dome_spec.split(tokens, ",", true);
      ossimRangeDome dome (tokens);
      if (dome.valid)
         m_rangeDomes.push_back(dome);
   }

   // The GSD may be incorrect, but init anyway then redo whe GSD is set:
   initialize();

   return isOpen();
}

void ossimRangeDomeTileSource::initialize()
{
   if (!isOpen())
      return;

   // Establish bounding rect:
   ossimGrect boundingRect;
   boundingRect.makeNan();
   vector<ossimRangeDome>::iterator dome = m_rangeDomes.begin();
   while (dome != m_rangeDomes.end())
   {
      boundingRect.expandToInclude(dome->boundingRect());
      ++dome;
   }

   // Set up the geometry:
   ossimRefPtr<ossimMapProjection> mapProj = new ossimEquDistCylProjection();
   mapProj->setOrigin(boundingRect.midPoint());
   mapProj->setMetersPerPixel(m_gsd);
   ossimDpt degPerPixel (mapProj->getDecimalDegreesPerPixel());
   mapProj->setElevationLookupFlag(false);
   mapProj->setUlTiePoints(boundingRect.ul());
   ossimIpt image_size(boundingRect.width()/degPerPixel.x, boundingRect.height()/degPerPixel.y);
   theGeometry = new ossimImageGeometry(0, mapProj.get());
   theGeometry->setImageSize(image_size);

   // Transform the domes to image space coordinates:
   dome = m_rangeDomes.begin();
   double r;
   while (dome != m_rangeDomes.end())
   {
      theGeometry->worldToLocal(dome->centerGpt, dome->centerIpt);
      r = dome->radiusMeters/m_gsd.x;
      dome->radiusPixelsSq = r*r;
      ++dome;
   }
}


bool ossimRangeDomeTileSource::isOpen() const
{
   return (!m_rangeDomes.empty());
}


void ossimRangeDomeTileSource::close()
{
   m_rangeDomes.clear();
   theGeometry = 0;
}


ossimRefPtr<ossimImageData> ossimRangeDomeTileSource::getTile(const ossimIrect& rect,
                                                              ossim_uint32 resLevel)
{
   ossimRefPtr<ossimImageData> tile = ossimImageDataFactory::instance()->create(this, this);

   tile->setImageRectangle(rect);
   tile->initialize();

   getTile(tile.get(), resLevel);

   return tile;
}


bool ossimRangeDomeTileSource::getTile(ossimImageData* result, ossim_uint32 resLevel)
{
   if (!result || !theGeometry.valid())
      return false;

   // Start with null fill:
   result->fill(getNullPixelValue());

   // Verify intersection with dataset. Everything done in R0 space:
   ossimIrect tile_rect (result->getImageRectangle());
   ossim_uint32 coord_scale = resLevel + 1;
   ossimIrect tile_rect_r0 (coord_scale*tile_rect);
   ossimIrect boundingImgRect;
   theGeometry->getBoundingRect(boundingImgRect);

   if (!tile_rect_r0.intersects(boundingImgRect))
   {
      result->setDataObjectStatus(OSSIM_EMPTY);
      return true;
   }

   vector<ossimRangeDome>::iterator dome;
   double d2, x0, y0, dx, dy, az;

   // Nested loop over all pixels in tile:
   for (ossim_int32 yn=tile_rect.ul().y; yn<=tile_rect.lr().y; ++yn)
   {
      y0 = yn * coord_scale;

      for (ossim_int32 xn=tile_rect.ul().x; xn<=tile_rect.lr().x; ++xn)
      {
         x0 = xn * coord_scale;

         // Loop over each dome in the list to see if this pixel is affected:
         dome = m_rangeDomes.begin();
         while (dome != m_rangeDomes.end())
         {
            // Check distance to center:
            dx = x0 - dome->centerIpt.x;
            dy = y0 - dome->centerIpt.y;
            d2 = dx*dx + dy*dy;

            if (d2 > dome->radiusPixelsSq)
            {
               ++dome;
               continue;
            }

            // Passed range test, do azimuth test. First check for simple 360 dome:
            if (dome->startAz == dome->endAz)
            {
               result->setValue(xn, yn, (ossim_uint8) dome->classification);
               ++dome;
               continue;
            }

            // Need full azimuth test:
            az = ossim::atan2d(dx, -dy);
            if (az < 0)
               az += 360;
            if (((dome->startAz < dome->endAz) && (az >= dome->startAz) && (az <= dome->endAz)) ||
                ((dome->startAz > dome->endAz) && (
                     ((az >= dome->startAz) && (az > dome->endAz)) ||
                     ((az < dome->startAz) && (az <= dome->endAz)))))
            {
               result->setValue(xn, yn, (ossim_uint8) dome->classification);
            }
            ++dome;
         }
      }
   }

   result->validate();
   return true;
}


ossim_uint32 ossimRangeDomeTileSource::getNumberOfInputBands() const
{
   return 1;
}


ossim_uint32 ossimRangeDomeTileSource::getNumberOfLines(ossim_uint32 resLevel) const
{
   if (theGeometry.valid())
      return ( theGeometry->getImageSize().y / (resLevel+1) );
   return 0;
}


ossim_uint32 ossimRangeDomeTileSource::getNumberOfSamples(ossim_uint32 resLevel) const
{
   if (theGeometry.valid())
      return ( theGeometry->getImageSize().x / (resLevel+1) );
   return 0;
}


bool ossimRangeDomeTileSource::saveState(ossimKeywordlist& kwl, const char* prefix) const
{
   static const char MODULE[] = "ossimRangeDomeTileSource::saveState()";

   ossimImageHandler::saveState(kwl, prefix);
   if (kwl.getErrorStatus() == ossimErrorCodes::OSSIM_ERROR)
   {
      ossimNotify(ossimNotifyLevel_WARN) << MODULE
            << " ERROR detected in keyword list!  State not saved." << std::endl;
      return false;
   }

   kwl.add(prefix, ossimKeywordNames::METERS_PER_PIXEL_KW, m_gsd.x, true);

   return true;
}


bool ossimRangeDomeTileSource::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   static const char MODULE[] = "ossimRangeDomeTileSource::loadState()";
   theDecimationFactors.clear();

   ossimImageHandler::loadState(kwl, prefix);
   if (kwl.getErrorStatus() == ossimErrorCodes::OSSIM_ERROR)
   {
      ossimNotify(ossimNotifyLevel_WARN)<< MODULE
            << "WARNING: error detected in keyword list!  State not load." << std::endl;
      return false;
   }

   ossimDpt gsd;
   ossimString value = kwl.find(prefix, ossimKeywordNames::METERS_PER_PIXEL_KW);
   if (!value.empty())
      gsd.x = value.toDouble();

   value = kwl.find(prefix, ossimKeywordNames::METERS_PER_PIXEL_Y_KW);
   if (!value.empty())
      setGSD(value.toDouble());

   // The rest of the state is established by opening the file:
   bool good_open = open();
   return good_open;
}


void ossimRangeDomeTileSource::setProperty(ossimRefPtr<ossimProperty> property)
{
   if (!property.valid())
      return;

   ossimString s;
   property->valueToString(s);
   if (s.empty())
      return;

   // The user should select either explicit GSD or relative GSD factor, never both:
   if ( property->getName() == ossimKeywordNames::METERS_PER_PIXEL_KW )
   {
      ossim_float64 gsd = s.toFloat64();
      if (!ossim::isnan(gsd))
         setGSD(gsd);
   }
   else
   {
      ossimImageHandler::setProperty(property);
   }
}


ossimRefPtr<ossimProperty> ossimRangeDomeTileSource::getProperty(const ossimString& name) const
{
   ossimRefPtr<ossimProperty> prop = 0;
   if ( name == ossimKeywordNames::METERS_PER_PIXEL_KW )
   {
      ossimString value = ossimString::toString(m_gsd.x);
      prop = new ossimStringProperty(name, value);
   }
   else
   {
      prop = ossimImageHandler::getProperty(name);
   }
   return prop;
}



void ossimRangeDomeTileSource::getGSD(ossimDpt& gsd, ossim_uint32 resLevel) const
{
   gsd.x =(1.0/(resLevel+1)) * m_gsd.x;
   gsd.y =(1.0/(resLevel+1)) * m_gsd.y;
}


void ossimRangeDomeTileSource::setGSD( const ossim_float64& gsd )
{
   if (gsd == m_gsd.x)
      return;

   m_gsd.x = gsd;
   m_gsd.y = gsd;

   initialize();
}




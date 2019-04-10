//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id$

#include <ossim/point_cloud/ossimPointCloudImageHandler.h>
#include <ossim/point_cloud/ossimPointCloudSource.h>
#include <ossim/point_cloud/ossimPointCloudHandlerRegistry.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimEllipsoid.h>
#include <ossim/base/ossimDatum.h>
#include <ossim/base/ossimNumericProperty.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/projection/ossimEpsgProjectionFactory.h>

using namespace std;

RTTI_DEF1(ossimPointCloudImageHandler, "ossimPointCloudImageHandler", ossimImageHandler);

static ossimTrace traceDebug("ossimPointCloudImageHandler:debug");
static const char* GSD_FACTOR_KW = "gsd_factor";
static const char* COMPONENT_KW = "component";

// The member m_activeComponent should be one of the following strings. This is set either in a
// state KWL or by a call to setProperty(<"active_component", <string> >)
static const char* INTENSITY_KW = "INTENSITY";
static const char* HIGHEST_KW = "HIGHEST";
static const char* LOWEST_KW = "LOWEST";
static const char* RETURNS_KW = "RETURNS";
static const char* RGB_KW = "RGB";

ossimPointCloudImageHandler::PcrBucket::PcrBucket(const ossim_float32* init_value,
                                                  ossim_uint32 numBands)
:  m_numSamples (1)
{
   m_bucket = new ossim_float32[numBands];
   for (ossim_uint32 i=0; i<numBands; i++)
      m_bucket[i] = init_value[i];
}


ossimPointCloudImageHandler::PcrBucket::PcrBucket(const ossim_float32& R,
      const ossim_float32& G,
      const ossim_float32& B)
:  m_numSamples (1)
{
   m_bucket = new ossim_float32[3];
   m_bucket[0] = R;
   m_bucket[1] = G;
   m_bucket[2] = B;

}


ossimPointCloudImageHandler::PcrBucket::PcrBucket(const ossim_float32& init_value)
:  m_numSamples (1)
{
   m_bucket = new ossim_float32[1];
   m_bucket[0] = init_value;
}


ossimPointCloudImageHandler::PcrBucket::~PcrBucket()
{
   delete [] m_bucket;
}


ossimPointCloudImageHandler::ossimPointCloudImageHandler()
      : ossimImageHandler(),
        m_maxPixel(1.0),
        m_minPixel(0.0),
        m_gsd(),
        m_gsdFactor (1.0),
        m_tile(0),
        m_mutex(),
        m_activeComponent(INTENSITY)
{
   //---
   // Nan out as can be set in several places, i.e. setProperty,
   // loadState and initProjection.
   //---
   m_gsd.makeNan();

   m_componentNames.emplace_back(INTENSITY_KW);
   m_componentNames.emplace_back(HIGHEST_KW);
   m_componentNames.emplace_back(LOWEST_KW);
   m_componentNames.emplace_back(RETURNS_KW);
   m_componentNames.emplace_back(RGB_KW);
}

ossimPointCloudImageHandler::~ossimPointCloudImageHandler()
{
   close();
}

bool ossimPointCloudImageHandler::open()
{
   close();

   // Need to utilize the Point Cloud handler registry to open the PC file:
   m_pch = ossimPointCloudHandlerRegistry::instance()->open(theImageFile);
   if (!m_pch.valid())
      return false;

   getImageGeometry();
   ossimImageHandler::completeOpen();

   // Needed here after open to make sure that min/max pixels are set for active component/entry
   setCurrentEntry((ossim_uint32)m_activeComponent);

   return true;
}

bool ossimPointCloudImageHandler::setPointCloudHandler(ossimPointCloudHandler* pch)
{
   close();

   // Need to utilize the Point Cloud handler registry to open the PC file:
   m_pch = pch;
   if (!m_pch.valid())
      return false;

   getImageGeometry();
   ossimImageHandler::completeOpen();

   // Needed here after open to make sure that min/max pixels are set for active component/entry
   setCurrentEntry((ossim_uint32)m_activeComponent);

   return true;
}

void ossimPointCloudImageHandler::close()
{
   if (isOpen())
   {
      m_pch->close();
      m_tile = 0;
      ossimImageHandler::close();
   }
}

ossimRefPtr<ossimImageGeometry> ossimPointCloudImageHandler::getImageGeometry()
{
   if (!isOpen())
      return 0;

   if (theGeometry.valid())
      return theGeometry;

   // Check for external geom (i.e., a *.geom file)
   theGeometry = getExternalImageGeometry();
   if (theGeometry.valid())
      return theGeometry;

   theGeometry = new ossimImageGeometry();
   ossimString epsgCode ("EPSG:4326");
   ossimMapProjection* proj = dynamic_cast<ossimMapProjection*>( // NOLINT
           ossimEpsgProjectionFactory::instance()->createProjection(epsgCode));
   if (!proj)
      return 0;
   theGeometry->setProjection(proj);

   // Need to establish image bounds and optimal GSD given ground rect. Use this switch to also
   // initialize the UL tiepoint of image projection:
   ossimGrect bounds;
   m_pch->getBounds(bounds);
   proj->setOrigin(bounds.ul());
   proj->setUlTiePoints(bounds.ul());

   // The GSD depends on the point density on the ground. count all final returns and
   // assume they are evenly distributed over the bounding ground rect. Note that this can have
   // up to a sqrt(2)X error if the collection was taken in a cardinal-diagonal direction.
   // Also use this point loop to latch the ground quad vertices vertices.
   ossim_uint32 numPulses = m_pch->getNumPoints(); // count of final returns
   if (numPulses == 0)
   {
      // Not yet determined. Set GSD to NAN and expect it will be set later:
      m_gsd.makeNan();
   }
   else if (m_gsd.hasNans())
   {
      ossimDpt delta (bounds.widthMeters(), bounds.heightMeters());
      ossim_float64 gsd = sqrt(delta.x * delta.y / numPulses) * m_gsdFactor;
      setGSD(gsd); // also recomputes the image size
   }

   return theGeometry;
}

ossimRefPtr<ossimImageData> ossimPointCloudImageHandler::getTile(const ossimIrect& tile_rect,
                                                                 ossim_uint32 resLevel)
{
   if (!m_tile.valid())
      initTile();

   // Image rectangle must be set prior to calling getTile.
   m_tile->setImageRectangle(tile_rect);
   if (!getTile(m_tile.get(), resLevel))
   {
      if (m_tile->getDataObjectStatus() != OSSIM_NULL)
         m_tile->makeBlank();
   }

   return m_tile;
}

bool ossimPointCloudImageHandler::getTile(ossimImageData* result, ossim_uint32 resLevel)
{
   // check for all systems go and valid args:
   if (!m_pch.valid() || !result || (result->getScalarType() != OSSIM_FLOAT32)
       || (result->getDataObjectStatus() == OSSIM_NULL) || m_gsd.hasNans())
   {
      return false;
   }

   // Overviews achieved with GSD setting. This may be too slow.
   ossimDpt gsd (m_gsd);
   if (resLevel > 0)
      getGSD(gsd, resLevel);

   // Establish the ground and image rects for this tile:
   const ossimIrect img_tile_rect = result->getImageRectangle();
   const ossimIpt tile_offset (img_tile_rect.ul());
   const ossim_uint32 tile_width = img_tile_rect.width();
   const ossim_uint32 tile_height = img_tile_rect.height();
   const ossim_uint32 tile_size = img_tile_rect.area();

   ossimGpt gnd_ul, gnd_lr;
   ossimDpt dpt_ul (img_tile_rect.ul().x - 0.5, img_tile_rect.ul().y - 0.5);
   ossimDpt dpt_lr (img_tile_rect.lr().x + 0.5, img_tile_rect.lr().y + 0.5);
   theGeometry->rnToWorld(dpt_ul, resLevel, gnd_ul);
   theGeometry->rnToWorld(dpt_lr, resLevel, gnd_lr);
   const ossimGrect gnd_rect (gnd_ul, gnd_lr);

   // Create array of buckets to store accumulated point data.
   ossim_uint32 numBands = result->getNumberOfBands();
   if (numBands > getNumberOfInputBands())
   {
      // This should never happen;
      ossimNotify(ossimNotifyLevel_FATAL)
            << "ossimPointCloudImageHandler::getTile() ERROR: \n"
            << "More bands were requested than was available from the point cloud source. Returning "
            << "blank tile." << endl;
      result->makeBlank();
      return false;
   }
   std::map<ossim_int32, PcrBucket*> accumulator;

   // initialize a point block with desired fields as requested in the reader properties
   ossimPointBlock pointBlock (this);
   pointBlock.setFieldCode(componentToFieldCode());
   m_pch->rewind();

   ossimDpt ipt;
   ossimGpt pos;

#define USE_GETBLOCK
#ifdef USE_GETBLOCK
   m_pch->getBlock(gnd_rect, pointBlock);
   for (ossim_uint32 id=0; id<pointBlock.size(); ++id)
   {
      pos = pointBlock[id]->getPosition();
      theGeometry->worldToRn(pos, resLevel, ipt);
      ipt.x = ossim::round<double,double>(ipt.x) - tile_offset.x;
      ipt.y = ossim::round<double,double>(ipt.y) - tile_offset.y;

      ossim_int32 bucketIndex = ipt.y*tile_width + ipt.x;
      if ((bucketIndex >= 0) && (bucketIndex < (ossim_int32)tile_size))
         addSample(accumulator, bucketIndex, pointBlock[id]);
   }

#else // using getFileBlock
   ossim_uint32 numPoints = m_pch->getNumPoints();
   if (numPoints > ossimPointCloudHandler::DEFAULT_BLOCK_SIZE)
      numPoints = ossimPointCloudHandler::DEFAULT_BLOCK_SIZE;

   // Loop to read all point blocks:
   do
   {
      pointBlock.clear();
      m_pch->getNextFileBlock(pointBlock, numPoints);
      //m_pch->normalizeBlock(pointBlock);

      for (ossim_uint32 id=0; id<pointBlock.size(); ++id)
      {
         // Check that each point in read block is inside the ROI before accumulating it:
         pos = pointBlock[id]->getPosition();
         if (gnd_rect.pointWithin(pos))
         {
            theGeometry->worldToRn(pos, resLevel, ipt);
            ipt.x = ossim::round<double,double>(ipt.x) - tile_offset.x;
            ipt.y = ossim::round<double,double>(ipt.y) - tile_offset.y;

            ossim_int32 bucketIndex = ipt.y*tile_width + ipt.x;
            if ((bucketIndex >= 0) && (bucketIndex < (ossim_int32)tile_size))
               addSample(accumulator, bucketIndex, pointBlock[id]);
         }
      }
   } while (pointBlock.size() == numPoints);
#endif

   // Finished accumulating, need to normalize and fill the tile.
   // We must always blank out the tile as we may not have a point for every pixel.
   normalize(accumulator);
   auto buf = new ossim_float32*[numBands];
   std::map<ossim_int32, PcrBucket*>::iterator accum_iter;
   ossim_float32 null_pixel = OSSIM_DEFAULT_NULL_PIX_FLOAT;
   result->setNullPix(null_pixel);
   for (ossim_uint32 band = 0; band < numBands; band++)
   {
      ossim_uint32 index = 0;
      buf[band] = result->getFloatBuf(band);
      for (ossim_uint32 y = 0; y < tile_height; y++)
      {
         for (ossim_uint32 x = 0; x < tile_width; x++)
         {
            accum_iter = accumulator.find(index);
            if (accum_iter != accumulator.end())
               buf[band][index] = accum_iter->second->m_bucket[band];
            else
               buf[band][index] = null_pixel;
            ++index;
         }
      }
   }

   delete [] buf;
   buf = 0;

   auto pcr_iter = accumulator.begin();
   while (pcr_iter != accumulator.end())
   {
      delete pcr_iter->second;
      pcr_iter++;
   }

   result->validate();
   return true;
}

void ossimPointCloudImageHandler::addSample(std::map<ossim_int32, PcrBucket*>& accumulator,
                                            ossim_int32 index,
                                            const ossimPointRecord* sample)
{
   if (sample == 0)
      return;

   //cout << "sample: "<<*sample<<endl;//TODO: REMOVE DEBUG

   // Search map for exisiting point in that location:
   auto iter = accumulator.find(index);
   if (iter == accumulator.end())
   {
      // First hit. Initialize location with current sample:
      if (m_activeComponent == INTENSITY)
      {
         accumulator[index] = new PcrBucket(sample->getField(ossimPointRecord::Intensity));
      }
      else if (m_activeComponent == RGB)
      {
         ossim_float32 color[3];
         color[0] = sample->getField(ossimPointRecord::Red);
         color[1] = sample->getField(ossimPointRecord::Green);
         color[2] = sample->getField(ossimPointRecord::Blue);
         accumulator[index] = new PcrBucket(color, 3);
      }
      else if ((m_activeComponent == LOWEST) || (m_activeComponent == HIGHEST))
         accumulator[index] = new PcrBucket(sample->getPosition().hgt);
      else if (m_activeComponent == RETURNS)
         accumulator[index] = new PcrBucket(sample->getField(ossimPointRecord::NumberOfReturns));
   }
   else
   {
      // Not the first hit at this location, accumulate:
      // First hit. Initialize location with current sample:
      if (m_activeComponent == INTENSITY)
      {
         iter->second->m_bucket[0] += sample->getField(ossimPointRecord::Intensity);
      }
      else if (m_activeComponent == RGB)
      {
         iter->second->m_bucket[0] += sample->getField(ossimPointRecord::Red);
         iter->second->m_bucket[1] += sample->getField(ossimPointRecord::Green);
         iter->second->m_bucket[2] += sample->getField(ossimPointRecord::Blue);
      }
      else if ((m_activeComponent == HIGHEST) &&
            (sample->getPosition().hgt > iter->second->m_bucket[0]))
         iter->second->m_bucket[0] = sample->getPosition().hgt;
      else if ((m_activeComponent == LOWEST) &&
            (sample->getPosition().hgt < iter->second->m_bucket[0]))
         iter->second->m_bucket[0] = sample->getPosition().hgt;
      else if (m_activeComponent == RETURNS)
         iter->second->m_bucket[0] += sample->getField(ossimPointRecord::NumberOfReturns);

      iter->second->m_numSamples++;
   }
}

void ossimPointCloudImageHandler::normalize(std::map<ossim_int32, PcrBucket*>& accumulator)
{
   // highest and lowest elevations latch extremes, no mean is computed but needs to be normalized
   if ((m_activeComponent == LOWEST) || (m_activeComponent == HIGHEST) ||
         (m_activeComponent == RETURNS))
      return;

   int numBands = 1;
   if (m_activeComponent == RGB)
      numBands = 3;

   auto iter = accumulator.begin();
   ossim_float32 avg;
   while (iter != accumulator.end())
   {
      for (int i=0; i<numBands; i++)
      {
         avg = iter->second->m_bucket[i] / iter->second->m_numSamples;
         iter->second->m_bucket[i] = avg;
      }
      iter++;
   }
}

ossim_uint32 ossimPointCloudImageHandler::getNumberOfInputBands() const
{
   ossim_uint32 numBands = 0;
   if (m_pch.valid())
   {
      if (m_activeComponent == INTENSITY)
         numBands = 1;
      else if (m_activeComponent == RGB)
         numBands = 3;
      else if ((m_activeComponent == LOWEST) || (m_activeComponent == HIGHEST))
         numBands = 1;
      else if (m_activeComponent == RETURNS)
         numBands = 1;
   }
   return numBands;
}

ossim_uint32 ossimPointCloudImageHandler::getNumberOfLines(ossim_uint32 resLevel) const
{
   ossim_uint32 result = 0;
   if (isOpen() && theGeometry.valid())
   {
      ossimIpt image_size(theGeometry->getImageSize());
      result = image_size.line;
      if (resLevel)
         result = (result >> resLevel);
   }
   return result;
}

ossim_uint32 ossimPointCloudImageHandler::getNumberOfSamples(ossim_uint32 resLevel) const
{
   ossim_uint32 result = 0;
   if (isOpen() && theGeometry.valid())
   {
      ossimIpt image_size(theGeometry->getImageSize());
      result = image_size.samp;
      if (resLevel)
         result = (result >> resLevel);
   }
   return result;
}

ossim_uint32 ossimPointCloudImageHandler::getImageTileWidth() const
{
   return getTileWidth();
}

ossim_uint32 ossimPointCloudImageHandler::getImageTileHeight() const
{
   return getTileHeight();
}

ossim_uint32 ossimPointCloudImageHandler::getTileWidth() const
{
   ossimIpt ipt;
   ossim::defaultTileSize(ipt);
   return ipt.x;
}

ossim_uint32 ossimPointCloudImageHandler::getTileHeight() const
{
   ossimIpt ipt;
   ossim::defaultTileSize(ipt);
   return ipt.y;
}

ossimScalarType ossimPointCloudImageHandler::getOutputScalarType() const
{
   return OSSIM_FLOAT32;
}

void ossimPointCloudImageHandler::getEntryList(std::vector<ossim_uint32>& entryList) const
{
   entryList.clear();
   for (ossim_uint32 i = 0; i < m_componentNames.size(); i++)
   {
      entryList.emplace_back(i);
   }
}

void ossimPointCloudImageHandler::getEntryNames(std::vector<ossimString>& entryNames) const
{
   entryNames = m_componentNames;
}

ossim_uint32 ossimPointCloudImageHandler::getCurrentEntry() const
{
   return (ossim_uint32) m_activeComponent;
}

bool ossimPointCloudImageHandler::setCurrentEntry(ossim_uint32 entryIdx)
{
   if (entryIdx >= NUM_COMPONENTS)
      return false;

   m_activeComponent = (Components) entryIdx;
   if (m_pch.valid() && m_pch->getMinPoint() && m_pch->getMaxPoint())
   {
      if (m_activeComponent == INTENSITY)
      {
         m_minPixel = m_pch->getMinPoint()->getField(ossimPointRecord::Intensity);
         m_maxPixel = m_pch->getMaxPoint()->getField(ossimPointRecord::Intensity);
      }
      else if (m_activeComponent == RGB)
      {
         m_minPixel = m_pch->getMinPoint()->getField(ossimPointRecord::Red);
         m_maxPixel = m_pch->getMaxPoint()->getField(ossimPointRecord::Red);
      }
      else if ((m_activeComponent == LOWEST) || (m_activeComponent == HIGHEST))
      {
         m_minPixel = m_pch->getMinPoint()->getPosition().hgt;
         m_maxPixel = m_pch->getMaxPoint()->getPosition().hgt;
      }
      else if (m_activeComponent == RETURNS)
      {
         m_minPixel = 0;
         m_maxPixel = m_pch->getMaxPoint()->getField(ossimPointRecord::NumberOfReturns);
      }
   }

   return true;
}

ossimString ossimPointCloudImageHandler::getShortName() const
{
   return ossimString("Point Cloud Image Handler");
}

ossimString ossimPointCloudImageHandler::getLongName() const
{
   return ossimString("ossim point cloud to image renderer");
}

double ossimPointCloudImageHandler::getMinPixelValue(ossim_uint32 /* band */) const
{
   return m_minPixel;
}

double ossimPointCloudImageHandler::getMaxPixelValue(ossim_uint32 /* band */) const
{
   return m_maxPixel;
}

double ossimPointCloudImageHandler::getNullPixelValue(ossim_uint32 /* band */) const
{
   return OSSIM_DEFAULT_NULL_PIX_FLOAT;
}

ossim_uint32 ossimPointCloudImageHandler::getNumberOfDecimationLevels() const
{
   // Can support any number of rlevels.
   ossim_uint32 result = 1;
   const ossim_uint32 STOP_DIMENSION = 16;
   ossim_uint32 largestImageDimension =
         getNumberOfSamples(0) > getNumberOfLines(0) ? getNumberOfSamples(0) : getNumberOfLines(0);
   while (largestImageDimension > STOP_DIMENSION)
   {
      largestImageDimension /= 2;
      ++result;
   }
   return result;
}

void ossimPointCloudImageHandler::initTile()
{
   const ossim_uint32 BANDS = getNumberOfOutputBands();

   m_tile = new ossimImageData(this, getOutputScalarType(), BANDS, getTileWidth(), getTileHeight());

   for (ossim_uint32 band = 0; band < BANDS; ++band)
   {
      m_tile->setMinPix(getMinPixelValue(band), band);
      m_tile->setMaxPix(getMaxPixelValue(band), band);
      m_tile->setNullPix(getNullPixelValue(band), band);
   }

   m_tile->initialize();
}

void ossimPointCloudImageHandler::getGSD(ossimDpt& gsd, ossim_uint32 resLevel) const
{
   // std::pow(2.0, 0) returns 1.
   ossim_float64 d = std::pow(2.0, static_cast<double>(resLevel));
   gsd.x = m_gsd.x * d;
   gsd.y = m_gsd.y * d;
}

void ossimPointCloudImageHandler::setGSD(const ossim_float64& gsd)
{
   if (ossim::isnan(gsd) || (gsd<=0.0) || !theGeometry.valid())
         return;

   m_gsd = ossimDpt(gsd, gsd);
   m_gsdFactor = 1.0; // resets after GSD adjusted

   ossimMapProjection* proj = dynamic_cast<ossimMapProjection*>(theGeometry->getProjection());
   if (!proj)
      return;

   proj->setMetersPerPixel(m_gsd);

   ossimGrect bounds;
   m_pch->getBounds(bounds);

   ossimDpt ipt_ul, ipt_lr;
   theGeometry->worldToLocal(bounds.ul(), ipt_ul);
   theGeometry->worldToLocal(bounds.lr(), ipt_lr);
   ossimIpt image_size;
   image_size.x = ossim::round<ossim_int32,double>(ipt_lr.x - ipt_ul.x) + 1;
   image_size.y = ossim::round<ossim_int32,double>(ipt_lr.y - ipt_ul.y) + 1;

   theGeometry->setImageSize(image_size);
}

bool ossimPointCloudImageHandler::saveState(ossimKeywordlist& kwl, const char* prefix) const
{
   static const char MODULE[] = "ossimPointCloudImageHandler::saveState()";

   ossimImageHandler::saveState(kwl, prefix);
   if (kwl.getErrorStatus() == ossimErrorCodes::OSSIM_ERROR)
   {
      ossimNotify(ossimNotifyLevel_WARN) << MODULE
            << " ERROR detected in keyword list!  State not saved." << std::endl;
      return false;
   }

   kwl.add(prefix, ossimKeywordNames::ENTRY_KW, (int) m_activeComponent, true);
   kwl.add(prefix, ossimKeywordNames::METERS_PER_PIXEL_KW, m_gsd.x, true);

   return true;
}

bool ossimPointCloudImageHandler::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   static const char MODULE[] = "ossimPointCloudImageHandler::loadState()";
   theDecimationFactors.clear();
   if(traceDebug())
      ossimNotify(ossimNotifyLevel_DEBUG)<< MODULE << " DEBUG: entered ..."<< std::endl;

   ossimImageHandler::loadState(kwl, prefix);
   if (kwl.getErrorStatus() == ossimErrorCodes::OSSIM_ERROR)
   {
      ossimNotify(ossimNotifyLevel_WARN)<< MODULE
            << "WARNING: error detected in keyword list!  State not load." << std::endl;
      return false;
   }

   m_activeComponent = INTENSITY;
   ossimString value = kwl.find(prefix, ossimKeywordNames::ENTRY_KW);
   if (!value.empty())
   {
      ossim_uint32 i = value.toUInt32();
      if (i < NUM_COMPONENTS)
         m_activeComponent = (Components) i;
   }

   value = kwl.find(prefix, ossimKeywordNames::METERS_PER_PIXEL_KW);
   if (!value.empty())
      setGSD(value.toDouble());

   // The rest of the state is established by opening the file:
   bool good_open = open();

   return good_open;
}

void ossimPointCloudImageHandler::getValidImageVertices(std::vector<ossimIpt>& validVertices,
                                   ossimVertexOrdering ordering,
                                   ossim_uint32 resLevel) const
{
   validVertices.clear();
   if (!m_pch.valid())
      return;
   int divisor = 1;
   if (resLevel)
      divisor = resLevel<<1;

   // Transform the world coords for the four vertices into image vertices:
   ossimDpt r0Pt;
   ossimGrect bounds;
   m_pch->getBounds(bounds);
   theGeometry->worldToLocal(bounds.ul(), r0Pt);
   validVertices.emplace_back(r0Pt);
   theGeometry->worldToLocal(bounds.ur(), r0Pt);
   validVertices.emplace_back(r0Pt);
   theGeometry->worldToLocal(bounds.lr(), r0Pt);
   validVertices.emplace_back(r0Pt);
   theGeometry->worldToLocal(bounds.ll(), r0Pt);
   validVertices.emplace_back(r0Pt);

   if (ordering == OSSIM_COUNTERCLOCKWISE_ORDER)
   {
      for (int i=3; i>=0; i--)
         validVertices.emplace_back(validVertices[i]/divisor);
      validVertices.erase(validVertices.begin(), validVertices.begin()+4);
   }
}

void ossimPointCloudImageHandler::setProperty(ossimRefPtr<ossimProperty> property)
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
   else if ( property->getName() == GSD_FACTOR_KW )
   {
      m_gsdFactor = s.toDouble();
      if (!ossim::isnan(m_gsdFactor))
      {
         if (!m_gsd.hasNans())
            setGSD(m_gsd.x * m_gsdFactor);
      }
      else
         m_gsdFactor = 1.0;
   }
   else if ( property->getName() == ossimKeywordNames::ENTRY_KW )
   {
      m_activeComponent = (Components) s.toUInt32();
   }
   else if ( property->getName() == COMPONENT_KW )
   {
      for (int i=0; i<NUM_COMPONENTS; i++)
      {
         if (s.upcase() == m_componentNames[i])
         {
            m_activeComponent = (Components) i;
            break;
         }
      }
   }
   else
   {
      ossimImageHandler::setProperty(property);
   }
}

ossimRefPtr<ossimProperty> ossimPointCloudImageHandler::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> prop = 0;
   if ( name == ossimKeywordNames::METERS_PER_PIXEL_KW )
   {
      ossimString value = ossimString::toString(m_gsd.x);
      prop = new ossimStringProperty(name, value);
   }
   else if ( name == GSD_FACTOR_KW )
   {
      prop = new ossimNumericProperty(name, ossimString::toString(m_gsdFactor));
   }
   else if ( name == ossimKeywordNames::ENTRY_KW )
   {
      prop = new ossimNumericProperty(name, ossimString::toString((ossim_uint32) m_activeComponent));
   }
   else if ( name == COMPONENT_KW )
   {
      prop = new ossimStringProperty(name, m_componentNames[m_activeComponent]);
   }
   else
   {
      prop = ossimImageHandler::getProperty(name);
   }
   return prop;
}

ossim_uint32 ossimPointCloudImageHandler::componentToFieldCode() const
{
   ossim_uint32 field_code = 0;
   switch (m_activeComponent)
   {
   case INTENSITY:
      field_code = ossimPointRecord::Intensity;
      break;
   case RETURNS:
      field_code = ossimPointRecord::NumberOfReturns;
      break;
   case RGB:
      field_code = ossimPointRecord::Red | ossimPointRecord::Green | ossimPointRecord::Blue;
      break;
   default:
      break;
   }
   return field_code;
}



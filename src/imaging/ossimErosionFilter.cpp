//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/imaging/ossimErosionFilter.h>
#include <ossim/base/ossimNumericProperty.h>
#include <ossim/base/ossimBooleanProperty.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/imaging/ossimImageData.h>
#include <vector>
#include <numeric>

using namespace std;

RTTI_DEF1(ossimErosionFilter, "ossimErosionFilter", ossimImageSourceFilter);

// Keywords used throughout.
static const ossimString WINDOW_SIZE_KW = "window_size";

ossimErosionFilter::ossimErosionFilter(ossimObject* owner)
   :ossimImageSourceFilter(owner),
    theTile(0),
    theTempTile(0),
    theWindowSize(3)
{
   setDescription(ossimString("Dilation Filter"));
}

ossimErosionFilter::~ossimErosionFilter()
{
}

void ossimErosionFilter::initialize()
{
   ossimImageSourceFilter::initialize();
   theTile = NULL;
}

ossimRefPtr<ossimImageData> ossimErosionFilter::getTile(const ossimIrect& rect,
                                                        ossim_uint32 resLevel)
{
   if(!isSourceEnabled())
      return ossimImageSourceFilter::getTile(rect, resLevel);

   ossimRefPtr<ossimImageData> inputData;
   ossim_uint32 halfSize = getWindowSize()>>1;
   ossimIrect requestRect(rect.ul().x - halfSize,
                          rect.ul().y - halfSize,
                          rect.lr().x + halfSize,
                          rect.lr().y + halfSize);

   inputData =  ossimImageSourceFilter::getTile(requestRect, resLevel);

   if(!inputData.valid() || !inputData->getBuf())
      return inputData;

   vector<ossimIpt> viv;
   ossimImageSourceFilter::getValidImageVertices(viv, OSSIM_CLOCKWISE_ORDER, resLevel);
   theValidImagePoly = ossimPolygon(viv);

   if(!theTile.valid())
   {
      theTile = (ossimImageData*)inputData->dup();
      theTile->setImageRectangle(rect);
   }
   else
   {
      theTile->setImageRectangleAndBands(rect, inputData->getNumberOfBands());
   }

   switch(inputData->getScalarType())
   {
      case OSSIM_UINT8:
         doErosion(ossim_uint8(0), inputData);
         break;
      case OSSIM_USHORT11:
      case OSSIM_UINT16:
         doErosion(ossim_uint16(0), inputData);
         break;
      case OSSIM_SINT16:
         doErosion(ossim_sint16(0), inputData);
         break;
      case OSSIM_UINT32:
         doErosion(ossim_uint32(0), inputData);
         break;
      case OSSIM_FLOAT32:
      case OSSIM_NORMALIZED_FLOAT:
         doErosion(ossim_float32(0), inputData);
         break;
      case OSSIM_FLOAT64:
      case OSSIM_NORMALIZED_DOUBLE:
         doErosion(ossim_float64(0), inputData);
         break;
      default:
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimErosionFilter::applyFilter WARNING:\n"
            << "Unhandled scalar type!" << endl;
      }
   }
   return theTile;
}

template <class T>
void ossimErosionFilter::doErosion(T scalarType , ossimRefPtr<ossimImageData>& inputData)
{
   ossimDataObjectStatus status = inputData->getDataObjectStatus();

   if ((status == OSSIM_FULL) || (status == OSSIM_EMPTY))
   {
      // Nothing to do just copy the tile.
      theTile->loadTile(inputData.get());
      return;
   }

   ossim_int32 halfWindow = (ossim_int32)(theWindowSize >> 1);
   ossim_int32 x, y, kernelX, kernelY;   
   ossim_int32 iw = (ossim_int32)inputData->getWidth();
   ossim_int32 ih = (ossim_int32)inputData->getHeight();
   ossim_int32 ow = (ossim_int32)theTile->getWidth();
   ossim_int32 oh = (ossim_int32)theTile->getHeight();
   ossim_uint32 numBands = ossim::min(theTile->getNumberOfBands(), inputData->getNumberOfBands());

   // It may be that the input rect is the same size as the output (i.e., the tile bounds aren't
   // expanded in the input's request to permit full kernels for output edge pixels:
   ossim_uint32 i_offset = 0;
   ossim_int32 delta = (ossim_int32)((iw - ow) >> 1);
   ossim_int32 xi, yi;
   if (delta > 0)
      i_offset = (ossim_uint32) halfWindow*(iw + 1);

   ossimIpt tile_ul (theTile->getImageRectangle().ul());
   vector<double> values;
   for(ossim_uint32 bandIdx = 0; bandIdx < numBands; ++bandIdx)
   {
      T* inputBuf     = (T*)inputData->getBuf(bandIdx);
      T* outputBuf    = (T*)theTile->getBuf(bandIdx);
      if (!inputBuf || !outputBuf)
      {
         return; // Shouldn't happen...
      }

      const T NP = (T)inputData->getNullPix(bandIdx);

      for(y = 0; y < oh; ++y)
      {
         for(x = 0; x < ow; ++x)
         {
            bool null_found = true;
            const T CP = *(inputBuf+i_offset);
            ossimDpt ipt (tile_ul.x+x, tile_ul.y+y);

            if ((CP != NP) && theValidImagePoly.isPointWithin(ipt))
            {
               // Valid pixel found, need to check neighbors for null:
               null_found = false;
               values.clear();
               for(kernelY = -halfWindow; (kernelY<=halfWindow) && !null_found; ++kernelY)
               {
                  yi = y + kernelY + delta;
                  if ((yi < 0) || (y > ih))
                     continue;

                  for(kernelX = -halfWindow; (kernelX<=halfWindow) && !null_found; ++kernelX)
                  {
                     xi = x + kernelX + delta;
                     if ((xi < 0) || (x > iw))
                        continue;

                     T neighborPixel = *(inputBuf+kernelX+delta + (kernelY+delta)*iw);
                     if(neighborPixel == NP)
                     {
                        // Found a NULL pixel:
                        (*outputBuf) = NP;
                        null_found = true;
                     }
                  }
               }
            }

            if (null_found)
               (*outputBuf) = NP;
            else
               (*outputBuf) = CP;

            // Move over...
            ++inputBuf;
            ++outputBuf;

         } // End of loop in x direction.

         // Move down...
         inputBuf += iw - ow;

      }  // End of loop in y direction.

   }  // End of band loop.

   theTile->validate();

}

void ossimErosionFilter::setProperty(ossimRefPtr<ossimProperty> property)
{
   if (!property.valid())
      return;

   ossimString name = property->getName();

   if (name == WINDOW_SIZE_KW)
   {
      theWindowSize = property->valueToString().toUInt32();
   }
   else
   {
      ossimImageSourceFilter::setProperty(property);
   }
}

ossimRefPtr<ossimProperty> ossimErosionFilter::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> prop = 0;
   if (name == WINDOW_SIZE_KW)
   {
      prop = new ossimNumericProperty(WINDOW_SIZE_KW, ossimString::toString(theWindowSize), 3, 25);
      prop->setCacheRefreshBit();
      return prop;
   }
   return ossimImageSourceFilter::getProperty(name);
}

void ossimErosionFilter::getPropertyNames(vector<ossimString>& propertyNames) const
{
   propertyNames.push_back(WINDOW_SIZE_KW);
   ossimImageSourceFilter::getPropertyNames(propertyNames);
}

bool ossimErosionFilter::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix, WINDOW_SIZE_KW.c_str(), theWindowSize, true);
   return ossimImageSourceFilter::saveState(kwl, prefix);
}

bool ossimErosionFilter::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   const char* lookup = kwl.find(prefix, WINDOW_SIZE_KW.c_str());
   if (lookup)
      theWindowSize = ossimString(lookup).toUInt32();
   
   return ossimImageSourceFilter::loadState(kwl, prefix);
}

//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  RP
//
// Description:
//
// 
//*******************************************************************
//  $Id$
#include <ossim/imaging/ossimElevRemapper.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/base/ossimGeoidManager.h>
#include <ossim/base/ossimTrace.h>

RTTI_DEF1(ossimElevRemapper, "ossimElevRemapper", ossimImageSourceFilter);
static ossimTrace traceDebug("ossimElevRemapper::debug");

const char ossimElevRemapper::REMAP_MODE_KW[]  = "remap_mode";

ossimElevRemapper::ossimElevRemapper()
:m_replacementType(ReplacementType_ELLIPSOID)
{
}

ossimElevRemapper::~ossimElevRemapper()
{
  m_imageGeometry = 0;
}

void ossimElevRemapper::initialize()
{
   if (theInputConnection && (!m_imageGeometry.valid() || !m_imageGeometry->getProjection())) 
   {
     m_imageGeometry = theInputConnection->getImageGeometry();
   }
}

ossimRefPtr<ossimImageData> ossimElevRemapper::getTile(const ossimIrect& tile_rect,
                                                        ossim_uint32 resLevel)
{
   if (traceDebug())
   {
     ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimElevRemapper::getTile entered..." << endl;
   }

   ossimRefPtr<ossimImageData> result = ossimImageSourceFilter::getTile(tile_rect, resLevel);
   if(!isSourceEnabled()||!result.valid())
   {
      return result.get();
   }
   ossimDataObjectStatus status =  result->getDataObjectStatus();
   
   if(status == OSSIM_NULL)
   { 
	return result.get();
   }
   // Call the appropriate load method.
   switch (result->getScalarType())
   {
      case OSSIM_UCHAR:
      {
         elevRemap(ossim_uint8(0), result.get(), resLevel);
         break;
      }
         
      case OSSIM_UINT16:
      case OSSIM_USHORT11:
      case OSSIM_USHORT12:
      case OSSIM_USHORT13:
      case OSSIM_USHORT14:
      case OSSIM_USHORT15:
      {
         elevRemap(ossim_uint16(0), result.get(), resLevel);
         break;
      }
         
      case OSSIM_SSHORT16:
      {
         elevRemap(ossim_sint16(0), result.get(), resLevel);
         break;
      }
      case OSSIM_UINT32:
      {
         elevRemap(ossim_uint32(0), result.get(), resLevel);
         break;
      }
      case OSSIM_SINT32:
      {
         elevRemap(ossim_sint32(0), result.get(), resLevel);
         break;
      }
      case OSSIM_FLOAT32:
      case OSSIM_NORMALIZED_FLOAT:
      {
         elevRemap(ossim_float32(0), result.get(), resLevel);
         break; } case OSSIM_NORMALIZED_DOUBLE: case OSSIM_FLOAT64:
      {
         elevRemap(ossim_float64(0), result.get(), resLevel);
         break;
      }
         
      case OSSIM_SCALAR_UNKNOWN:
      default:
      {
         ossimNotify(ossimNotifyLevel_WARN)
         << "ossimElevRemapper::getTile Unsupported scalar type!" << endl;
         break;
      }
   }
   if (traceDebug())
   {
     ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimElevRemapper::getTile leaving..." << endl;
   }
   
   return result;
}

template <class T>
void ossimElevRemapper::elevRemap(T /* dummy */,
                                   ossimImageData* inputTile,
                                   ossim_uint32 resLevel)
{
   if (!inputTile) return;
   ossimIrect rect = inputTile->getImageRectangle();
   ossimIrect imageBounds = getBoundingRect(resLevel);
   ossimIrect clipRect;
   if(!rect.intersects(imageBounds))
   {
      return;
   }
   clipRect = rect.clipToRect(imageBounds);

   if (m_imageGeometry.valid() && m_imageGeometry->getProjection())
   {
      ossimGpt ul, ll, lr, ur;
      ossim_float32 hul, hll, hlr, hur;
      ossim_float32 height = 0.0, xpercent, ypercent, uppery, lowery;

      m_imageGeometry->rnToWorld(clipRect.ul(), resLevel, ul);
      m_imageGeometry->rnToWorld(clipRect.ur(), resLevel, ur);
      m_imageGeometry->rnToWorld(clipRect.ll(), resLevel, ll);
      m_imageGeometry->rnToWorld(clipRect.lr(), resLevel, lr);

      //std::cout << "UL: " << clipRect.ul() << " RES: " << ossimString::toString(resLevel) << " ULGROUND: " << ul << "\n";

      hul = ossimGeoidManager::instance()->offsetFromEllipsoid(ul);
      hll = ossimGeoidManager::instance()->offsetFromEllipsoid(ll);
      hlr = ossimGeoidManager::instance()->offsetFromEllipsoid(lr);
      hur = ossimGeoidManager::instance()->offsetFromEllipsoid(ur);

//	std::cout << "HUL: " << ossimString::toString(hul) << " HUR: " << ossimString::toString(hur) << " HLR: " << ossimString::toString(hlr) << " HUR: " << ossimString::toString(hur) << "\n";

     if(!rect.completely_within(imageBounds))
     {
        ossim_uint32 bands = inputTile->getNumberOfBands();
        ossimIpt origin = clipRect.ul() - rect.ul();
        ossim_uint32 bandIdx = 0;
        ossim_uint32 inputW = inputTile->getWidth();
        ossim_uint32 originOffset = origin.y*inputW + origin.x;
        ossim_uint32 w = clipRect.width();
        ossim_uint32 h = clipRect.height();
        ossim_uint32 x = 0;
        ossim_uint32 y = 0;
        ossim_float32 height = 0.0, xpercent, ypercent, uppery, lowery;
        for(bandIdx = 0; bandIdx < bands; ++bandIdx)
        {
           T* bandPtr = static_cast<T*>(inputTile->getBuf(bandIdx)) + originOffset;
           for(y = 0; y < h; ++y)
           {
              for(x = 0; x < w; ++x)
              {
	         xpercent = x / clipRect.width();
	         ypercent = y / clipRect.height();
	         uppery = hul + xpercent * ( hur - hul);
	         lowery = hll + xpercent * ( hlr - hll);
	         height = uppery + ypercent * ( lowery - uppery );
		 if (m_replacementType==ReplacementType_GEOID) height*=-1.0;

	         bandPtr[x] += height;
              }
              bandPtr += inputW;
           }
        }
     }
     else
     {
        ossim_uint32 bands = inputTile->getNumberOfBands();
        ossim_uint32 bandIdx = 0;
        ossim_uint32 size = inputTile->getWidth()*inputTile->getHeight();
        ossim_float32 height = 0.0;
        for(bandIdx = 0; bandIdx < bands; ++bandIdx)
        {
           T* bandPtr = static_cast<T*>(inputTile->getBuf(bandIdx));
                               
           ossim_uint32 idx = 0;
           for(idx = 0; idx < size;++idx)
           {
	      ossim_uint32 y = idx / rect.width();
              ossim_uint32 x = idx % rect.width();
	      xpercent = x / clipRect.width();
              ypercent = y / clipRect.height();
              uppery = hul + xpercent * ( hur - hul);
              lowery = hll + xpercent * ( hlr - hll);
              height = uppery + ypercent * ( lowery - uppery );
	      if (m_replacementType==ReplacementType_GEOID) height*=-1.0;

	      (*bandPtr) += height;
	      ++bandPtr;
           }
        }
     }
   }
}

bool ossimElevRemapper::saveState(ossimKeywordlist& kwl,
                                   const char* prefix)const
{
   bool result = ossimImageSourceFilter::saveState(kwl, prefix);
   ossimString remapMode = "";
   switch(m_replacementType)
   {
      case ReplacementType_ELLIPSOID:
         remapMode = "ellipsoid"; 
	 break;
      case ReplacementType_GEOID:
         remapMode = "geoid";
	 break;
      default:
	 remapMode = "ellipsoid";
         break;
   }
   kwl.add(prefix, REMAP_MODE_KW,  remapMode.c_str());

   ossimString imagePrefix = ossimString(prefix)+"image_geometry.";

   if(m_imageGeometry.valid())
   {
      m_imageGeometry->saveState(kwl, imagePrefix.c_str());
   }
   //std::cout << kwl << " SAVESTATE\n";
   return result;
}

bool ossimElevRemapper::loadState(const ossimKeywordlist& kwl,
                                   const char* prefix)
{
   //std::cout << kwl << " LOADSTATE\n";
   if (traceDebug())
   {
     ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimElevRemapper::loadState entered..." << endl;
   }
   bool result = ossimImageSourceFilter::loadState(kwl, prefix);

   const char* lookup;
   lookup = kwl.find(prefix, REMAP_MODE_KW);
   if(lookup)
   {
      ossimString mode = lookup;
      mode.upcase();
      if (mode == "ELLIPSOID")
      {
         m_replacementType = ReplacementType_ELLIPSOID;
      }
      else if (mode == "GEOID")
      {
	 m_replacementType = ReplacementType_GEOID;
      }
   }

   ossimString imagePrefix = ossimString(prefix)+"image_geometry.";
   if(kwl.numberOf(imagePrefix.c_str())>0)
   {
      m_imageGeometry = new ossimImageGeometry();
      m_imageGeometry->loadState(kwl, imagePrefix.c_str());
      //setImageGeometry(m_igeometry.get());
   }

   return result;
}

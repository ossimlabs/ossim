//----------------------------------------------------------------------------
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Filter for shifting input to an output range.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/imaging/ossimShiftFilter.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>

using namespace std;

RTTI_DEF1(ossimShiftFilter, "ossimShiftFilter", ossimImageSourceFilter)

static ossimTrace traceDebug("ossimShiftFilter:debug");

ossimShiftFilter::ossimShiftFilter()
   :
   ossimImageSourceFilter(),  // base class
   m_tile(0),
   m_min(ossim::nan()),
   m_max(ossim::nan()),
   m_null(ossim::nan())
{
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimShiftFilter::ossimShiftFilter entered...\n";
   }
}

ossimShiftFilter::~ossimShiftFilter()
{
}

void ossimShiftFilter::initialize()
{
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimShiftFilter::initialize entered..." << endl;
   }

   //---
   // Call the base class initialize.
   // Note:  This will reset "theInputConnection" if it changed...
   //---
   ossimImageSourceFilter::initialize();
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimShiftFilter::initialize exited..." << endl;
   }
}

ossimRefPtr<ossimImageData> ossimShiftFilter::getTile(
   const ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   ossimRefPtr<ossimImageData> result = 0;
   
   if ( theInputConnection )
   {
      ossimRefPtr<ossimImageData> inputTile = theInputConnection->getTile( tileRect, resLevel );
      
      if ( inputTile.get() && isSourceEnabled() &&
           !ossim::isnan(m_null) && !ossim::isnan(m_min) && !ossim::isnan(m_max) )
      {
         // Get its status of the input tile.
         ossimDataObjectStatus tile_status = inputTile->getDataObjectStatus();
         
         if ( tile_status != OSSIM_NULL )
         {
            if ( !m_tile )
            {
               allocate(); // First time through.
            }
            
            if ( tile_status != OSSIM_EMPTY )
            {
               // Set the origin,bands of the output tile.
               m_tile->setImageRectangle(tileRect);

               switch(inputTile->getScalarType())
               {
                  case OSSIM_UINT8:
                  {
                     fillTile( ossim_uint8(0), inputTile.get(), m_tile.get() );
                     break;
                  }
                  case OSSIM_SINT8:
                  {
                     fillTile( ossim_sint8(0), inputTile.get(), m_tile.get() );
                     break;
                  }
                  case OSSIM_UINT16:
                  case OSSIM_USHORT11:
                  case OSSIM_USHORT12:
                  case OSSIM_USHORT13:
                  case OSSIM_USHORT14:
                  case OSSIM_USHORT15:
                  {
                     fillTile( ossim_uint16(0), inputTile.get(), m_tile.get() );
                     break;
                  }
                  case OSSIM_SINT16:
                  {
                     fillTile( ossim_sint16(0), inputTile.get(), m_tile.get() ); 
                     break;
                  }
                  case OSSIM_SINT32:
                  {
                     fillTile( ossim_sint32(0), inputTile.get(), m_tile.get() );
                     break;
                  }
                  case OSSIM_UINT32:
                  {
                     fillTile( ossim_uint32(0), inputTile.get(), m_tile.get() );
                     break;
                  }
                  case OSSIM_FLOAT32: 
                  case OSSIM_NORMALIZED_FLOAT:
                  {
                     fillTile( ossim_float32(0), inputTile.get(), m_tile.get() );
                     break;
                  }
                  case OSSIM_FLOAT64:
                  case OSSIM_NORMALIZED_DOUBLE:
                  {
                     fillTile( ossim_float64(0), inputTile.get(), m_tile.get() );
                     break;
                  }
                  case OSSIM_SCALAR_UNKNOWN:
                  default:
                  {
                     ossimNotify(ossimNotifyLevel_WARN)
                        << "ossimShiftFilter::getTile ERROR Unhandled scalar!" << endl;
                        break;
                  }
            
               } // Matches: switch(inputTile->getScalarType())

               m_tile->validate();
            }
            else
            {
               m_tile->makeBlank();
            }

            result = m_tile;
            
         } // Matches: if ( tile_status != OSSIM_NULL )
            
      } // Matches: if ( inputTile.get() ... )

      if ( !result && inputTile.get() )
      {
         result = inputTile;
      }
           
   } // Matches: if ( theInputConnection ) 
   
   return result;
}

template <class T> void ossimShiftFilter::fillTile(T /* dummy */,
                                                   const ossimImageData* inputTile,
                                                   ossimImageData* outputTile) const
{
   const double BANDS = inputTile->getNumberOfBands();
   const ossim_uint32 SPB = inputTile->getSizePerBand();
   std::vector<double> inNull(BANDS);
   std::vector<double> inMin(BANDS);
   std::vector<double> inMax(BANDS);
   std::vector<double> coef(BANDS);
   ossim_uint32 band = 0;

   for( ; band < BANDS; ++band )
   {
      inNull[band] = inputTile->getNullPix(band);
      inMin[band]  = inputTile->getMinPix(band);
      inMax[band]  = inputTile->getMaxPix(band);
      coef[band]   = (m_max-m_min)/(inMax[band]-inMin[band]);
   }
   
   double pix = 0;
   for( band = 0; band < BANDS; ++band )
   {
      const T* inBuf = static_cast<const T*>(inputTile->getBuf(band));
      T* outBuf = static_cast<T*>(outputTile->getBuf(band));
      
      for ( ossim_uint32 i = 0; i < SPB; ++i )
      {
         pix = inBuf[i];
         if (  pix == inNull[band] )
         {
            pix = m_null;
         }
         else
         {
            // Shift and multiply:
            pix = m_min + (pix - inMin[band]) * coef[band];
            
            // Range check:
            pix = pix <= m_max ? (pix >= m_min ? pix : m_min) : m_max;
         }
         
         outBuf[i] = static_cast<T>(pix);
      }
   }

   outputTile->validate();
}

ossimString ossimShiftFilter::getClassName() const
{
   return ossimString("ossimShiftFilter");
}

ossimString ossimShiftFilter::getLongName()const
{
   return ossimString("OSSIM shift filter");
}

ossimString ossimShiftFilter::getShortName()const
{
   return ossimString("shift filter");
}

double ossimShiftFilter::getNullPixelValue(ossim_uint32 band )const
{
   double result = 0;;
   if ( theEnableFlag && !ossim::isnan(m_null) )
   {
      result = m_null;
   }
   else
   {
      result = ossimImageSourceFilter::getNullPixelValue( band );
   }
   return result;
}

double ossimShiftFilter::getMinPixelValue(ossim_uint32 band )const
{
   double result = 0;;
   if ( theEnableFlag && !ossim::isnan(m_min) )
   {
      result = m_min;
   }
   else
   {
      result = ossimImageSourceFilter::getMinPixelValue( band );
   }
   return result;
}

double ossimShiftFilter::getMaxPixelValue(ossim_uint32 band )const
{
   double result = 0;;
   if ( theEnableFlag && !ossim::isnan(m_max) )
   {
      result = m_max;
   }
   else
   {
      result = ossimImageSourceFilter::getMaxPixelValue( band );
   }
   return result;
}

void ossimShiftFilter::setNullPixelValue(double null)
{
   m_null = null;
}

void ossimShiftFilter::setMinPixelValue(double min)
{
   m_min = min;
}

void ossimShiftFilter::setMaxPixelValue(double max)
{
   m_max = max;
}

void ossimShiftFilter::allocate()
{
   m_tile = ossimImageDataFactory::instance()->create(this,this);
   m_tile->initialize();
}

// Private to disallow use...
ossimShiftFilter::ossimShiftFilter(const ossimShiftFilter&)
: m_min(0),m_max(0),m_null(0)
{
}

// Private to disallow use...
ossimShiftFilter& ossimShiftFilter::operator=(const ossimShiftFilter&)
{
   return *this;
}



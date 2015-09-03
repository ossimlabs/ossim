//----------------------------------------------------------------------------
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Piecewise remapper class definition. Derived from ossimTableRemapper.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/imaging/ossimPiecewiseRemapper.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageData.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

RTTI_DEF1(ossimPiecewiseRemapper, "ossimPiecewiseRemapper", ossimTableRemapper)


static const std::string REMAP_KW      = "remap";
static const std::string REMAP_TYPE_KW = "remap_type";

static ossimTrace traceDebug("ossimPiecewiseRemapper:debug");

#ifdef OSSIM_ID_ENABLED
static const char OSSIM_ID[] = "$Id$";
#endif

// Private container class for points.
ossimPiecewiseRemapper::ossimRemapSet::ossimRemapSet()
   :
   m_set(0)
{
}

ossimPiecewiseRemapper::ossimRemapSet::ossimRemapSet(const ossimRemapSet& obj)
   :
   m_set(obj.m_set)
{
}

const ossimPiecewiseRemapper::ossimRemapSet&
ossimPiecewiseRemapper::ossimRemapSet::operator=( const ossimRemapSet& rhs )
{
   if ( this != &rhs )
   {
      m_set = rhs.m_set;
   }
   return *this;
}

// Private container class for points.
ossimPiecewiseRemapper::ossimBandRemap::ossimBandRemap()
   :
   m_remap(0)
{
}

ossimPiecewiseRemapper::ossimBandRemap::ossimBandRemap(const ossimBandRemap& obj)
   :
   m_remap(obj.m_remap)
{
}

const ossimPiecewiseRemapper::ossimBandRemap&
ossimPiecewiseRemapper::ossimBandRemap::operator=( const ossimBandRemap& rhs )
{
   if ( this != &rhs )
   {
      m_remap = rhs.m_remap;
   }
   return *this;
}

void ossimPiecewiseRemapper::ossimBandRemap::loadState( const ossimKeywordlist& kwl,
                                                        const std::string& prefix,
                                                        ossim_uint32 band )
{
   //---
   // Band Remap set example:
   // band0.remap0:((0, 127, 0, 127), (128, 255, 128, 382))
   // band0.remap1:((0, 382, 0, 255))
   //---

   // Clear the sets:
   m_remap.clear();
   
   // Get the number of remaps for this band.
   std::string keyBase = "band";
   keyBase += ossimString::toString(band).string();
   keyBase += ".";
   keyBase += REMAP_KW;
   
   ossim_uint32 NUMBER_REMAPS = kwl.numberOf(prefix.c_str(), keyBase.c_str());
   ossim_uint32 found = 0;
   ossim_uint32 index = 0;
   
   // Loop to find band remaps.  This allows for skipping indexes. 
   while ( found < NUMBER_REMAPS )
   {
      std::string key = keyBase + ossimString::toString(index).string();
      std::string value = kwl.findKey( prefix, key );
      if ( value.size() )
      {
         ossimPiecewiseRemapper::ossimRemapSet set;
         if ( initRemapSetFromString( value, set ) )
         {
            m_remap.push_back( set );
         }
         ++found;
      }
      
      ++index;
      if ( index > (NUMBER_REMAPS+100) )
      {
         break;
      }
   }
   
} // End: ossimPiecewiseRemapper::ossimBandRemap::loadState
      
void ossimPiecewiseRemapper::ossimBandRemap::saveState(
   ossimKeywordlist& kwl,
   const std::string& prefix,
   ossimPiecewiseRemapper::PiecewiseRemapType remapType,
   ossim_uint32 band ) const
{
   //---
   // Remap set:
   // Remap set: "band0.remap0":
   // band0.remap0:((0, 127, 0, 127), (128, 255, 128, 382))
   // band0.remap1:((0, 382, 0, 255))
   //---
   ossim_uint32 remapIndex = 0;
   std::vector<ossimRemapSet>::const_iterator i = m_remap.begin();
   while ( i != m_remap.end() )
   {
      std::string key = "band";
      key += ossimString::toString(band).string();
      key += ".";
      key += REMAP_KW;
      key += ossimString::toString(remapIndex).string();
      std::string value;
      getRemapSetString( remapType, (*i), value );
      kwl.addPair( prefix, key, value );
      ++i;
      ++remapIndex;
   }
   
} // End: ossimPiecewiseRemapper::ossimBandRemap::saveState

bool ossimPiecewiseRemapper::ossimBandRemap::initRemapSetFromString(
   const std::string& s, ossimPiecewiseRemapper::ossimRemapSet& set ) const
{
   //---
   // Example:
   // ((0, 127, 0, 127), (128, 255, 128, 382))
   //---

   bool result = false;

   if ( s.size() )
   {
      std::istringstream in( s );
      char c;
      ossim_float64 d;
      
      // Gobble the open '('
      while ( !in.bad() && !in.eof() )
      {
         c = in.get();
         if ( c == '(' ) break;
      }
      
      // Main loop:
      while( !in.bad() && !in.eof() )
      {
         c = in.get();
         
         if ( c == ')' ) // Possible end of quadruple...
         {
            // Gobble next comma:
            while( !in.bad() && !in.eof() )
            {
               c = in.get();
               if ( c == ',' )
               {
                  break;
               }
            }
            c = in.get();
         }
         
         if ( (c == '(') || (c == ',') )
         {
            // Next string should be a number:
            in >> d;
            if ( in.good() )
            {
               set.m_set.push_back(d);
            }
            else
            {
               break;
            }
         }
      }

      if ( set.m_set.size() )
      {
         result = true;
      }
   }

   if ( !result )
   {
      set.m_set.clear();
   }

   return result;
   
} // End: ossimPiecewiseRemapper::ossimBandRemap::initRemapSetFromString( ... )

void ossimPiecewiseRemapper::ossimBandRemap::getRemapSetString(
   ossimPiecewiseRemapper::PiecewiseRemapType remapType,
   const ossimPiecewiseRemapper::ossimRemapSet& set, 
   std::string& s ) const
{
   if ( remapType == ossimPiecewiseRemapper::LINEAR_NATIVE )
   {
      getLinearRemapSetString( set, s );
   }
}

void ossimPiecewiseRemapper::ossimBandRemap::getLinearRemapSetString(
   const ossimPiecewiseRemapper::ossimRemapSet& set, 
   std::string& s ) const
{
   //---
   // Save in the form of:
   // ((0, 127, 0, 127), (128, 255, 128, 382))
   //---
   s.clear();
   if ( set.m_set.size() )
   {
      const ossim_uint32 TUPLES = set.m_set.size() / 4;
      if ( TUPLES )
      {  
         std::ostringstream os;
         os << std::setprecision(15)
            << "(";
         for ( ossim_uint32 i = 0; i < TUPLES; ++i )
         {
            ossim_uint32 setIdx = i*4;
            os << "("
               << set.m_set[ setIdx ]
               << ","
               << set.m_set[ setIdx + 1 ]
               << ","
               << set.m_set[ setIdx + 2 ]
               << ","
               << set.m_set[ setIdx + 3 ]
               << ")";
            if ( i < (TUPLES-1) )
            {
               os << ","; // Comma between quadruples.
            }
         }
         os << ")";
         s = os.str();
      }
   }
   
} // End: ossimPiecewiseRemapper::ossimBandRemap::getLinearRemapSetString( ... )

ossimPiecewiseRemapper::ossimPiecewiseRemapper()
   :
   ossimTableRemapper(),  // base class
   m_dirty(false),
   m_remapType(ossimPiecewiseRemapper::UNKNOWN),
   m_bandRemap(0),
   m_min(0),
   m_max(0)
{
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimPiecewiseRemapper::ossimPiecewiseRemapper entered...\n";
#ifdef OSSIM_ID_ENABLED
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "OSSIM_ID:  " << OSSIM_ID << "\n";
#endif      
   }
}

ossimPiecewiseRemapper::~ossimPiecewiseRemapper()
{
}

void ossimPiecewiseRemapper::initialize()
{
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimPiecewiseRemapper::initialize entered..." << endl;
   }

   //---
   // Call the base class initialize.
   // Note:  This will reset "theInputConnection" if it changed...
   //---
   ossimTableRemapper::initialize();
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimPiecewiseRemapper::initialize exited..." << endl;
   }
}

void ossimPiecewiseRemapper::setRemapType( const std::string& type )
{
   if ( (type == "linear_native") ||
        (ossimString(type).downcase() == "linear_native") )
   {
      m_remapType = ossimPiecewiseRemapper::LINEAR_NATIVE;
   }
   else
   {
      m_remapType = ossimPiecewiseRemapper::UNKNOWN;
   }
}

ossimRefPtr<ossimImageData> ossimPiecewiseRemapper::getTile(
   const ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   ossimRefPtr<ossimImageData> result = 0;
   
   if ( theInputConnection )
   {
      if ( m_dirty )
      {
         // Rebuild the table if dirty flag set:
         buildTable();
      }

      if ( theEnableFlag && theTable.size() ) 
      {
         //---
         // Not bypassed and has a table...
         // Base handles the rest...
         //---
         result = ossimTableRemapper::getTile(tileRect, resLevel);
      }
      else
      {
         // Fetch tile from pointer from the input source.
         result = theInputConnection->getTile(tileRect, resLevel);
      }
   }

   return result;
}

void ossimPiecewiseRemapper::getRemapTypeString(
   ossimPiecewiseRemapper::PiecewiseRemapType remapType, std::string& s ) const
{
   if ( remapType == ossimPiecewiseRemapper::LINEAR_NATIVE )
   {
      s = "linear_native";
   }
   else
   {
      s = "unknown";
   }
}

bool ossimPiecewiseRemapper::loadState(const ossimKeywordlist& kwl,
                                       const char* prefix)
{
   static const char MODULE[] = "ossimPiecewiseRemapper::loadState";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " Entered..." << "\nprefix:  " << (prefix?prefix:"null") << endl;
   }

   bool status = false;
   std::string p = ( prefix ? prefix : "" );

   // Check type first before going on:
   std::string key = ossimKeywordNames::TYPE_KW;
   std::string value;
   value = kwl.findKey( p, key );

   if ( value == "ossimPiecewiseRemapper" )
   {
      // Load the base class states...
      status = ossimTableRemapper::loadState(kwl, prefix);
      
      if (status)
      {
         // Look for scalar type keyword.
         // ossim_int32 st = ossimScalarTypeLut::instance()->getEntryNumber(kwl, p.c_str(), true);

         // Lookup table returns -1 if not found so check return...
         // if ( (st != -1) && (st != OSSIM_SCALAR_UNKNOWN) )
         // {
         // m_outputScalarType = static_cast<ossimScalarType>(st);
         //}
         
         // Remap type "remap_type":
         value = kwl.findKey( p, REMAP_TYPE_KW );
         if ( value.size() )
         {
            setRemapType( value );
            
            if ( m_remapType != ossimPiecewiseRemapper::UNKNOWN )
            {
               // Get the number of bands "number_bands":
               ossim_uint32 bands = 0;
               key = ossimKeywordNames::NUMBER_BANDS_KW;
               value = kwl.findKey( p, key );
               if ( value.size() )
               {
                  bands = ossimString(value).toUInt32();
               }
               
               if ( bands )
               {
                  // Start with clean remap set:
                  m_bandRemap.clear();
                  m_bandRemap.resize( bands );
                  
                  m_min.clear();
                  m_max.clear();

                  // Loop through bands:
                  for ( ossim_uint32 band = 0; band < bands; ++band )
                  {
                     // Band remap:
                     m_bandRemap[band].loadState( kwl, p, band );
                     
                     // Min:
                     std::string keyBase = ossimKeywordNames::MIN_VALUE_KW;
                     keyBase += ".band";
                     key = keyBase + ossimString::toString(band).string();
                     value =  kwl.findKey( p, key );
                     if ( value.size() )
                     {
                        m_min.push_back( ossimString(value).toFloat64() );
                     }
                     
                     // Max:
                     keyBase = ossimKeywordNames::MAX_VALUE_KW;
                     keyBase += ".band";
                     key = keyBase + ossimString::toString(band).string();
                     value =  kwl.findKey( p, key );
                     if ( value.size() )
                     {
                        m_max.push_back( ossimString(value).toFloat64() );
                     }
                     
                  } // End: Band loop
                  
                  if ( m_bandRemap.size() && ( !m_min.size() || !m_max.size() ) )
                  {
                     initMinMax(); // Initialize from the m_reampSet tuples.
                  }
               }
            }
         }

         if ( m_bandRemap.size() )
         {
            m_dirty = true;
         }
         
      } // Matches: status = ossimTableRemapper::loadState(kwl, prefix); if (status){...
      else
      {
         // Sets base: ossimSource::theEnableFlag
         setEnableFlag(false);
      }


      // Tmp drb:
      // initMinMax();
      
   } // Matches: if ( value == "ossimPiecewiseRemapper" )

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " DEBUG:\n";
      this->print( ossimNotify(ossimNotifyLevel_DEBUG) );
      ossimNotify(ossimNotifyLevel_DEBUG) << "\nExited..." << endl;
   }
   return status;
   
} // End: ossimPiecewiseRemapper::loadState

bool ossimPiecewiseRemapper::saveState( ossimKeywordlist& kwl,
                                        const char* prefix) const
{
   bool result = false;
   
   const ossim_uint32 BANDS = getNumberOfOutputBands();
   
   if ( ( m_remapType != ossimPiecewiseRemapper::UNKNOWN ) &&
        ( m_bandRemap.size() == BANDS ) )
   {
      std::string p = ( prefix ? prefix : "" );

      // Bands:
      std::string value = ossimString::toString(BANDS).string();
      kwl.addPair( p, std::string(ossimKeywordNames::NUMBER_BANDS_KW), value );
      
      // Remap type:
      getRemapTypeString( m_remapType, value );
      kwl.addPair( p, REMAP_TYPE_KW, value );

      // Save the band remaps.
      if ( m_bandRemap.size() == BANDS )
      {
         for ( ossim_uint32 band = 0; band < BANDS; ++band )
         {
            m_bandRemap[band].saveState( kwl, p, m_remapType, band );
         }
      }
      
      // Min/max values:
      if ( ( m_min.size() == BANDS ) && ( m_max.size() == BANDS ) )
      {
         std::string minBase = ossimKeywordNames::MIN_VALUE_KW;
         minBase += ".band";
         std::string maxBase = ossimKeywordNames::MAX_VALUE_KW;
         maxBase += ".band";
         
         for ( ossim_uint32 band = 0; band < BANDS; ++band )
         {
            // Min:
            std::string key = minBase + ossimString::toString(band).string();
            kwl.add( p.c_str(), key.c_str(), m_min[band] );

            // Max:
            key = maxBase + ossimString::toString(band).string();
            kwl.add( p.c_str(), key.c_str(), m_max[band] );
         }
      }

      // Base class:
      result = ossimTableRemapper::saveState(kwl, prefix);
   }
   
   return result;
}

ostream& ossimPiecewiseRemapper::print(ostream& os) const
{
   ossimKeywordlist kwl;
   saveState( kwl, 0 );
   
   os << setiosflags(ios::fixed) << setprecision(15)
      << "\nossimPiecewiseRemapper::print:\n"
      << kwl
      << "\n";
   
   return os;
}

ossimString ossimPiecewiseRemapper::getClassName() const
{
   return ossimString("ossimPiecewiseRemapper");
}

ossimString ossimPiecewiseRemapper::getLongName()const
{
   return ossimString("OSSIM Piecewise Remapper");
}

ossimString ossimPiecewiseRemapper::getShortName()const
{
   return ossimString("Piecewise Remapper");
}

double ossimPiecewiseRemapper::getMinPixelValue(ossim_uint32 band)const
{
   ossim_float64 result = ossimTableRemapper::getMinPixelValue(band);
   if ( theEnableFlag && (band < m_min.size()) )
   {
      result = m_min[band];
   }
   return result;
}

ossim_float64 ossimPiecewiseRemapper::getMaxPixelValue(ossim_uint32 band) const
{
   ossim_float64 result = ossimTableRemapper::getMaxPixelValue(band);
   if ( theEnableFlag && (band < m_max.size()) )
   {
      result = m_max[band];
   }
   return result;
}

void ossimPiecewiseRemapper::initMinMax()
{
#if 0
   if ( m_remapType == ossimPiecewiseRemapper::LINEAR_NATIVE )
   {
      //---
      // Each remap set holds a group of tuples for the band.
      // Tuple format example: <min_in> <max_in> <min_out> <max_out>
      //---
      const ossim_uint32 BANDS = getNumberOfOutputBands();

      if ( m_bandRemap.size() == BANDS )
      {
         m_min.resize( BANDS );
         m_max.resize( BANDS );
         
         // First time through:
         for( ossim_uint32 band = 0; band < BANDS; ++band )
         {
            const ossim_uint32 TUPLES = m_bandRemap[band].m_set.size() / 4;
            for ( ossim_uint32 tuple = 0; tuple < TUPLES; ++tuple )
            {
               //---
               // Min: get min of "min_out" from sets.
               // Max: get max of "max_out" from sets.
               //---
               if ( tuple == 0 )
               {
                  m_min[band] = m_bandRemap[band].m_set[tuple*4+2];
                  m_max[band] = m_bandRemap[band].m_set[tuple*4+3];
               }
               else
               {
                  if ( m_bandRemap[band].m_set[tuple*4+2] < m_min[band] )
                  {
                     m_min[band] = m_bandRemap[band].m_set[tuple*4+2];
                  }
                  if ( m_bandRemap[band].m_set[tuple*4+3] > m_max[band] )
                  {
                     m_max[band] = m_bandRemap[band].m_set[tuple*4+3];
                  }
               }  
            }

            //---
            // Clamp min to scalar min as this is used for getMinPixelValue.
            // This will keep a remap of:
            // ((0, 127, 0, 127), (128, 255, 128, 382))
            // From having a min and null of 0...
            //---
            if ( m_outputScalarType != OSSIM_SCALAR_UNKNOWN )
            {
               if ( m_min[band] < ossim::defaultMin( m_outputScalarType ) )
               {
                  m_min[band] = ossim::defaultMin( m_outputScalarType );
               }
            }
            
         } // End: band loop...
      }
   }
   else
   {
      m_min.clear();
      m_max.clear();
   }
#endif

   // Disabled for now (drb)
   m_min.clear();
   m_max.clear();
   
   
} // End: ossimPiecewiseRemapper::initMinMax()

void ossimPiecewiseRemapper::buildTable()
{
   const ossim_uint32 BANDS = getNumberOfOutputBands();
   if ( BANDS && (m_bandRemap.size() == BANDS) &&
        ( m_remapType == ossimPiecewiseRemapper::LINEAR_NATIVE ) )
   {
      setupTable();

      if ( theTable.size() )
      {
         if ( m_remapType == ossimPiecewiseRemapper::LINEAR_NATIVE )
         {
            buildLinearNativeTable();
         }
      }
   }
   else
   {
      // No remaps:
      theTable.clear();
   }

   // Clear the dirty flag.
   m_dirty = false;
   
} // End: ossimPiecewiseRemapper::buildTable()

void ossimPiecewiseRemapper::buildLinearNativeTable()
{
   switch ( getOutputScalarType() )
   {
      case OSSIM_UINT8:
      {
         buildLinearNativeTable(ossim_uint8(0));
         break;
      }
      case OSSIM_USHORT11:
      case OSSIM_UINT16:
      {
         buildLinearNativeTable(ossim_uint16(0));
         break;
      }
      case OSSIM_SINT16:
      {
         buildLinearNativeTable(ossim_sint16(0));
         break;
      }
      case OSSIM_UINT32:
      {
         buildLinearNativeTable(ossim_uint32(0));
         break;
      }
      case OSSIM_SINT32:
      {
         buildLinearNativeTable(ossim_sint32(0));
         break;
      }
      case OSSIM_SCALAR_UNKNOWN:
      default:
      {
         if(traceDebug())
         {
            // Shouldn't hit this.
            ossimNotify(ossimNotifyLevel_WARN)
               << "ossimHistogramRemapper::buildLinearNativeTable OSSIM_SCALAR_UNKNOWN!" << endl;
         }
         break;
      }
			
   } // End of "switch (theTableType)"
      
} // End: void ossimPiecewiseRemapper::buildLinearNativeTable()

template <class T> void ossimPiecewiseRemapper::buildLinearNativeTable(T /* dummy */)
{
   const ossim_uint32 BANDS = getNumberOfOutputBands();
   
   if ( BANDS && (m_bandRemap.size() == BANDS) && theTable.size() )
   {
      T* table = reinterpret_cast<T*>(&theTable.front());

      ossim_uint32 index = 0;
      
      bool isInteger = std::numeric_limits<T>::is_integer; // Flag to round or not.
      
      // Band loop:
      for(ossim_uint32 band = 0; band < BANDS; ++band)
      {
         // First bin is always for null.
         table[index++] = (T)getNullPixelValue(band);
         
         const ossim_float64 MIN_PIX = getMinPixelValue(band);
         const ossim_float64 MAX_PIX = getMaxPixelValue(band);

         for( ossim_uint32 bin = 1; bin < theTableBinCount; ++bin )
         {
            ossim_float64 p = MIN_PIX + bin - 1;

#if 0 /* Please keep for debug. (drb) */
            cout << "\ninput pix[" << bin << "]: " << p << endl;
#endif
            
            // Loop through remaps:
            std::vector<ossimRemapSet>::const_iterator i = m_bandRemap[band].m_remap.begin();
            while ( i != m_bandRemap[band].m_remap.end() )
            {
               //---
               // Each remap set holds a group of tuples for the band.
               // Tuple format example: <min_in> <max_in> <min_out> <max_out>
               //---
               const ossim_uint32 TUPLES = (*i).m_set.size() / 4;
               for ( ossim_uint32 set = 0; set < TUPLES; ++set )
               {
                  // Range check it:
                  if ( ( p >= (*i).m_set[set*4] ) &&  // input min
                       ( p <= (*i).m_set[set*4+1] ) ) // input max
                  {
                     //---
                     // p = (p - output_min) * (output_max-output_min)/(input_max-input_min)
                     // + output_min;
                     //---
                     p = (p - (*i).m_set[set*4]) *
                        ((*i).m_set[set*4+3]-(*i).m_set[set*4+2]) /
                        ((*i).m_set[set*4+1]-(*i).m_set[set*4]) +
                        (*i).m_set[set*4];

#if 0 /* Please keep for debug. (drb) */
                     cout << "remapp_pix[" << bin << "][" << set << "]: " << p << endl;
#endif
                  }
                  
               } // End: TUPLE loop

               ++i; // Next remap.

            } // End: remap loop:

            if ( isInteger )
            {
               p = ossim::round<ossim_float64>(p); // Round to integer correctly.
            }
            
#if 0 /* Please keep for debug. (drb) */
            cout << "output_pix[" << bin << "]: " << p << endl;
#endif
            
            // Assign to table with min/max clip:
            table[bin] = (T)( ( p >= MIN_PIX ) ? ( ( p <= MAX_PIX ) ? p : MAX_PIX ) : MIN_PIX);
            
         } // End: bin loop
         
      } // End: band loop
      
   } // Matches: if ( theTable.size() )

#if 0 /* Please leave for debug. (drb) */
   ossimTableRemapper::print( cout );
#endif
   
} // End: template <class T> void ossimPiecewiseRemapper::buildLinearNativeTable(T dummy)

void ossimPiecewiseRemapper::setupTable()
{
   const ossim_uint32 BANDS = getNumberOfOutputBands();
   if ( BANDS )
   {
      ossim_uint32 values_per_band = 0;
      ossim_uint32 bytes_per_pixel = 0;
      
      switch (theOutputScalarType)
      {
         case OSSIM_UINT8:
            values_per_band = 256;  // 2 ^ 8
            bytes_per_pixel = 1;
            theTableType = ossimTableRemapper::NATIVE;
            break;
            
         case OSSIM_USHORT11:
            values_per_band = 2048; // 2 ^ 11
            bytes_per_pixel = 2;
            theTableType = ossimTableRemapper::NATIVE;
            break;
            
         case OSSIM_UINT16:
         case OSSIM_SINT16:
            values_per_band = 65536; // 2 ^ 16
            bytes_per_pixel = 2;
            theTableType = ossimTableRemapper::NATIVE;
            break;
            
         case OSSIM_UINT32:
         case OSSIM_SINT32:
            values_per_band = 65536; // 2 ^ 16
            bytes_per_pixel = 4;
            theTableType = ossimTableRemapper::NATIVE;
            break;
            
         case OSSIM_NORMALIZED_FLOAT:
         case OSSIM_FLOAT:
            bytes_per_pixel = 4;
            break;
            
         case OSSIM_NORMALIZED_DOUBLE:         
         case OSSIM_DOUBLE:
            bytes_per_pixel = 8;
            theTableType = ossimTableRemapper::NORMALIZED;
            break;
            
         default:
            break;
      }
   
      theTableBinCount  = values_per_band;
      theTableBandCount = BANDS;

      ossim_uint32 size_in_bytes = values_per_band * BANDS * bytes_per_pixel;
      theTable.resize(size_in_bytes);
   }
   
} // End: ossimPiecwiseRemapper::getTableSize()

// Private to disallow use...
ossimPiecewiseRemapper::ossimPiecewiseRemapper(const ossimPiecewiseRemapper&)
{
}

// Private to disallow use...
ossimPiecewiseRemapper& ossimPiecewiseRemapper::operator=(const ossimPiecewiseRemapper&)
{
   return *this;
}



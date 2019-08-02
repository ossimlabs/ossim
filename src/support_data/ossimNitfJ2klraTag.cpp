//----------------------------------------------------------------------------
//
// File: ossimNitfJ2klraTag.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: NITF J2KLRA tag.
//
// See:  ISO/IEC BIIF Profile BPJ2K01.00 Table 9-3.
// 
//----------------------------------------------------------------------------
// $Id$


#include <ossim/support_data/ossimNitfJ2klraTag.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/base/ossimString.h>

#include <cstring> /* for memcpy */
#include <iomanip>
#include <istream>
#include <ostream>

using namespace std;

RTTI_DEF1(ossimNitfJ2klraTag, "ossimNitfJ2klraTag", ossimNitfRegisteredTag);

ossimNitfJ2klraTag::ossimNitfJ2klraTag()
   : ossimNitfRegisteredTag(std::string("J2KLRA"), 0),
     m_layer(1)
{
   clearFields();
}

void ossimNitfJ2klraTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(m_orig, ORIG_SIZE);
   in.read(m_levels_o, NLEVELS_O_SIZE);
   in.read(m_bands_o, NBANDS_O_SIZE);
   in.read(m_layers_o, NLAYERS_O_SIZE);

   ossim_uint32 layers = getNumberOfLayersOriginal();
   if ( layers && (layers < 1000) ) // 999 max
   {
      m_layer.resize(layers);
      for ( ossim_uint32 i = 0; i < layers; ++i )
      {
         in.read(m_layer[i].m_layer_id, LAYER_ID_SIZE);
         in.read(m_layer[i].m_bitrate, BITRATE_SIZE);
         m_layer[i].m_layer_id[LAYER_ID_SIZE] = '\0';
         m_layer[i].m_bitrate[BITRATE_SIZE]   = '\0';
      }
   }
   else
   {
      m_layer.clear();
   }

   // Conditional:
   if ( isParsed() )
   {
      in.read(m_nlevels_i, NLEVELS_I_SIZE);
      in.read(m_nbands_i,  NBANDS_I_SIZE);
      in.read(m_nlayers_i, NLAYERS_I_SIZE);
   }
   
   // Set the base tag length.
   setTagLength( getSizeInBytes() );
}

void ossimNitfJ2klraTag::writeStream(std::ostream& out)
{
   out.write(m_orig, ORIG_SIZE);
   out.write(m_levels_o, NLEVELS_O_SIZE);
   out.write(m_bands_o, NBANDS_O_SIZE);
   out.write(m_layers_o, NLAYERS_O_SIZE);
   
   ossim_uint32 size = m_layer.size();
   for ( ossim_uint32 i = 0; i < size; ++i )
   {
      out.write(m_layer[i].m_layer_id, LAYER_ID_SIZE);
      out.write(m_layer[i].m_bitrate, BITRATE_SIZE);
   }

   // Conditional:
   if ( isParsed() )
   {
      out.write(m_nlevels_i, NLEVELS_I_SIZE);
      out.write(m_nbands_i, NBANDS_I_SIZE);
      out.write(m_nlayers_i, NLAYERS_I_SIZE);
   }
}

ossim_uint32 ossimNitfJ2klraTag::getSizeInBytes()const
{
   ossim_uint32 result = 11 + (m_layer.size() * 12);

   // Conditional:
   ossim_uint32 orig = getOriginNumber();
   if ( orig )
   {
      if ( orig % 2 ) // Odd origins are "parsed".
      {   
         result += 10;
      }
   }
   return result;
}

void ossimNitfJ2klraTag::clearFields()
{
   memset(m_orig, 0, ORIG_SIZE);

   memset(m_levels_o, 0, NLEVELS_O_SIZE);
   memset(m_bands_o, 0, NBANDS_O_SIZE);
   memset(m_layers_o, 0, NLAYERS_O_SIZE);

   m_layer.clear();

   memset(m_nlevels_i, 0, NLEVELS_I_SIZE);
   memset(m_nbands_i, 0, NBANDS_I_SIZE);
   memset(m_nlayers_i, 0, NLAYERS_I_SIZE);
   
   m_orig[ORIG_SIZE]= '\0';
   
   m_levels_o[NLEVELS_O_SIZE]= '\0';
   m_bands_o[NBANDS_O_SIZE]= '\0';
   m_layers_o[NLAYERS_O_SIZE]= '\0';

   m_nlevels_i[NLEVELS_I_SIZE]= '\0';
   m_nbands_i[NBANDS_I_SIZE] = '\0';
   m_nlayers_i[NLAYERS_I_SIZE] = '\0';

   // Set the base tag length.
   setTagLength( 0 );
}

std::ostream& ossimNitfJ2klraTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   out << setiosflags(ios::left)
       << pfx << std::setw(24) << "CETAG:"     << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"       << getTagLength() << "\n"
       << pfx << std::setw(24) << "ORIG:"      << m_orig << "\n"
       << pfx << std::setw(24) << "NLEVELS_O:" << m_levels_o << "\n"
       << pfx << std::setw(24) << "NBANDS_O:"  << m_bands_o << "\n"
       << pfx << std::setw(24) << "NLAYERS_O:" << m_layers_o << "\n";

   ossim_uint32 size = m_layer.size();
   for ( ossim_uint32 i = 0; i < size; ++i )
   {
      out << pfx << "LAYER_ID[" << i << std::setw(14) << "]:" << m_layer[i].m_layer_id << "\n"
          << pfx << "BITRATE[" << i << std::setw(15) << "]:" << m_layer[i].m_bitrate << "\n";
   }

   // Conditional:
   if ( isParsed() )
   {
      out << pfx << std::setw(24) << "NLEVELS_I:"     << m_nlevels_i << "\n"
          << pfx << std::setw(24) << "NBANDS_I_SIZE:" << m_nbands_i << "\n"
          << pfx << std::setw(24) << "NLAYERS_I:"     << m_nlayers_i << "\n";
   }
   
   return out;
}
   
ossim_uint32 ossimNitfJ2klraTag::getOriginNumber() const
{
   return ossimString(m_orig).toUInt32();
}
   
ossim_uint32 ossimNitfJ2klraTag::getNumberOfLayersOriginal() const
{
   return ossimString(m_layers_o).toUInt32();
}

bool ossimNitfJ2klraTag::setOrigin( ossim_uint32 origin )
{
   bool result = true;
   if ( origin <= 9 )
   {
      // Ascii 0 to 9
      m_orig[0] = 0x30 + origin;
   }
   else
   {
      result = false;
   }
   return result;
}

bool ossimNitfJ2klraTag::setLevelsO( ossim_uint32 levels )
{
   bool result = false;
   if ( levels <= 32 )
   {
      ossimString os = ossimNitfCommon::convertToUIntString( levels, NLEVELS_O_SIZE );
      if ( os.size() == NLEVELS_O_SIZE )
      {
         strncpy( m_levels_o, os.string().c_str(), NLEVELS_O_SIZE );
         result = true;
      }
   }
   return result;
}
   
bool ossimNitfJ2klraTag::setBandsO( ossim_uint32 bands )
{
   bool result = false;
   if ( ( bands >= 1 ) && ( bands <= 16384 ) )
   {
      ossimString os = ossimNitfCommon::convertToUIntString( bands, NBANDS_O_SIZE );
      if ( os.size() == NBANDS_O_SIZE )
      {
         strncpy( m_bands_o, os.string().c_str(), NBANDS_O_SIZE );
         result = true;
      }
   }
   return result;
}

bool ossimNitfJ2klraTag::setLayersO( ossim_uint32 layers )
{
   bool result = false;
   if ( ( layers >= 1 ) && ( layers <= 999 ) )
   {
      ossimString os = ossimNitfCommon::convertToUIntString( layers, NLAYERS_O_SIZE );
      if ( os.size() == NLAYERS_O_SIZE )
      {
         strncpy( m_layers_o, os.string().c_str(), NLAYERS_O_SIZE );

         // Conditional repeating field:
         m_layer.resize( layers );
         
         result = true;
      }
   }
   return result;
}

bool ossimNitfJ2klraTag::setLevelsI( ossim_uint32 levels )
{
   bool result = false;
   if ( levels <= 32 )
   {
      ossimString os = ossimNitfCommon::convertToUIntString( levels, NLEVELS_I_SIZE );
      if ( os.size() == NLEVELS_I_SIZE )
      {
         strncpy( m_nlevels_i, os.string().c_str(), NLEVELS_I_SIZE );
         result = true;
      }
   }
   return result;
}
   
bool ossimNitfJ2klraTag::setBandsI( ossim_uint32 bands )
{
   bool result = false;
   if ( ( bands >= 1 ) && ( bands <= 16384 ) )
   {
      ossimString os = ossimNitfCommon::convertToUIntString( bands, NBANDS_I_SIZE );
      if ( os.size() == NBANDS_I_SIZE )
      {
         strncpy( m_nbands_i, os.string().c_str(), NBANDS_I_SIZE );
         result = true;
      }
   }
   return result;
}

bool ossimNitfJ2klraTag::setLayersI( ossim_uint32 layers )
{
   bool result = false;
   if ( ( layers >= 1 ) && ( layers <= 999 ) )
   {
      ossimString os = ossimNitfCommon::convertToUIntString( layers, NLAYERS_I_SIZE );
      if ( os.size() == NLAYERS_I_SIZE )
      {
         strncpy( m_nlayers_i, os.string().c_str(), NLAYERS_I_SIZE );
         result = true;
      }
   }
   return result;
}

bool ossimNitfJ2klraTag::setLayerId( ossim_uint32 index, ossim_uint32 id )
{
   bool result = false;
   if ( index < m_layer.size() && ( id <= 998 ) )
   {
      ossimString os = ossimNitfCommon::convertToUIntString( id, LAYER_ID_SIZE );
      if ( os.size() ==  LAYER_ID_SIZE)
      {
         strncpy( m_layer[index].m_layer_id, os.string().c_str(), LAYER_ID_SIZE );
         result = true;
      }
   }
   return result;
}

bool ossimNitfJ2klraTag::setLayerBitRate( ossim_uint32 index, ossim_float64 bitRate )
{
   bool result = false;
   if ( index < m_layer.size() && ( bitRate >= 0.0 ) && ( bitRate <= 37.0 ) )
   {
      // 00.000000 – 37.000000
      ossimString os = ossimNitfCommon::convertToDoubleString( bitRate, 6, BITRATE_SIZE );
      if ( os.size() == BITRATE_SIZE)
      {
         strncpy( m_layer[index].m_bitrate, os.string().c_str(), BITRATE_SIZE);
         result = true;
      }
   }
   return result;
}

bool ossimNitfJ2klraTag::isParsed() const
{
   bool result = false;
   ossim_uint32 orig = getOriginNumber();
   if ( orig )
   {
      /*  
       * 0 - Original NPJE
       * 1 – Parsed NPJE
       * 2 – Original EPJE
       * 3 – Parsed EPJE*
       * 4 - Original TPJE
       * 5 - Parsed TPJE
       * 6 - Original LPJE
       * 7 - Parsed LPJE
       * 8 – Original other
       * 9 – Parsed other
       */
      if ( orig % 2 ) // Odd origins are "parsed".
      {
         result = true;
      }
   }
   return result;
}

//---
// 
// License: MIT
//
// Author: David Burken
// 
// Description: Container class definition for J2K Tile-part lengths record.
// See document BPJ2K01.10 Table 7-21 for detailed description.
//
// $Id$
//---

#include <ossim/support_data/ossimJ2kTlmRecord.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimIoStream.h>
#include <ossim/base/ossimNotify.h>
#include <iostream>
#include <iomanip>
#include <cstring>


ossimJ2kTlmRecord::ossimJ2kTlmRecord()
   :
   m_Ltlm(0),
   m_Ztlm(0),
   m_Stlm(0x40), // Tiles in order 32 bit Ptlm
   m_Ttlm(0),
   m_Ptlm(0)
{
}

ossimJ2kTlmRecord::~ossimJ2kTlmRecord()
{
   clear();
}

void ossimJ2kTlmRecord::clear()
{
   clearTtlm();
   clearPtlm();
}

void ossimJ2kTlmRecord::clearTtlm()
{
   if ( m_Ttlm )
   {
      ossim_uint8 st = getSt();
      if ( st == 1 )
      {
         ossim_uint8* p = (ossim_uint8*)m_Ttlm;
         delete [] p;
      }
      else if ( st == 2 )
      {
         ossim_uint16* p = (ossim_uint16*)m_Ttlm;
         delete [] p;
      }
      m_Ttlm = 0;
   }
}

void ossimJ2kTlmRecord::clearPtlm()
{
   if ( m_Ptlm )
   {
      ossim_uint8 sp = getSp();
      if ( sp == 0 )
      {
         ossim_uint16* p = (ossim_uint16*)m_Ptlm;
         delete [] p;
      }
      else if ( sp == 1 )
      {
         ossim_uint32* p = (ossim_uint32*)m_Ptlm;
         delete [] p;
      }
      m_Ptlm = 0;
   }
}

void ossimJ2kTlmRecord::parseStream(ossim::istream& in)
{
   clear(); // Deletes m_Ttlm and m_Ptlm arrays if they exist.
   
   ossimEndian* endian = 0;
   if (ossim::byteOrder() == OSSIM_LITTLE_ENDIAN)
   {
      // Stored big endian, must swap.
      endian = new ossimEndian();
   }
   
   // Get the stream posistion.
   std::streamoff pos = in.tellg();
   
   // Note: Marker is not read.

   // Length of segment minus marker.
   in.read((char*)&m_Ltlm, 2);
   if ( endian )
   {
      endian->swap(m_Ltlm);
   }

   // Index of marker segment relative to all other TLM marker segments.
   in.read((char*)&m_Ztlm, 1);

   // Stlm contains two variables in one byte, ST and SP.
   in.read((char*)&m_Stlm, 1);

   ossim_uint16 tile_count = getTileCount();
   if ( tile_count )
   {
      ossim_uint8 st = getSt();

      //---
      // Get the Ttlm array if any:
      // st value of 0 means tiles are in order and no Ttlm array.
      //---
      if ( st == 1 ) // 8 bit
      {
         ossim_uint8* p = new ossim_uint8[tile_count];
         in.read((char*)p, tile_count);
         m_Ttlm = p;
      }
      else if ( st == 2 ) // 16 bit
      {
         ossim_uint16* p = new ossim_uint16[tile_count];
         in.read((char*)p, tile_count*2);
         if ( endian )
         {
            endian->swap(p, tile_count);
         }
         m_Ttlm = p;
      }
      else if ( st > 2 )
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimJ2kTlmRecord::parseStream(...) Bad tlm ST value!"
            << std::endl;
      }
      
      // Get the Ptlm array:
      ossim_uint8 sp = getSp();
      if ( sp == 0 ) // 16 bit
      {
         ossim_uint16* p = new ossim_uint16[tile_count];
         in.read((char*)p, tile_count*2);
         if ( endian )
         {
            endian->swap(p, tile_count);
         }
         m_Ptlm = p;
      }
      else if ( sp == 1 ) // 32 bit
      {
         ossim_uint32* p = new ossim_uint32[tile_count];
         in.read((char*)p, tile_count*4);
         if ( endian )
         {
            endian->swap(p, tile_count);
         }
         m_Ptlm = p;
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimJ2kTlmRecord::parseStream(...) Bad tlm SP value!"
            << std::endl;
      }
   }

   // Clean up::
   if ( endian )
   {
      delete endian;
      endian = 0;
   }  
   
   //---
   // Seek to next record in case there was a parse error and we didn't read
   // all bytes.
   //---
   in.seekg(pos + m_Ltlm, std::ios_base::beg);
}

void ossimJ2kTlmRecord::writeStream(std::ostream& out)
{
   ossim_uint16 tileCount = getTileCount();
   ossim_uint8 st = getSt();
   ossim_uint8 sp = getSp();

   ossimEndian* endian = 0;
   if (ossim::byteOrder() == OSSIM_LITTLE_ENDIAN)
   {
      // Stored in file big endian, must swap.
      endian = new ossimEndian();
      endian->swap( m_Ltlm );

      // Conditional ttlm array:
      if ( st == 2 )
      {
         endian->swap( (ossim_uint16*)m_Ttlm, tileCount );
      }

      // Conditional ptlm array:
      if (sp == 0 )
      {
         endian->swap( (ossim_uint16*)m_Ptlm, tileCount );
      }
      else if ( sp == 1 )
      {
         endian->swap( (ossim_uint32*)m_Ptlm, tileCount );
      }
   }

   // Marker 0xff55:
   out.put( 0xff );
   out.put( 0x55 );
   
   out.write( (char*)&m_Ltlm, 2);
   out.write( (char*)&m_Ztlm, 1);
   out.write( (char*)&m_Stlm, 1);

   // Conditional array of tile indexes:
   if ( st == 1 ) // 8 bit array
   {
      out.write( (char*)m_Ttlm, tileCount );
   }
   else if ( st == 2 ) // 16 bit array
   {
      out.write( (char*)m_Ttlm, tileCount*2 );
   }

   // Conditional array of tile byte counts:
   if ( sp == 0 )  // 16 bit array
   {
      out.write( (char*)m_Ptlm, tileCount*2 );
   }
   else if ( sp == 1 ) // 32 bit array
   {
      out.write( (char*)m_Ptlm, tileCount*4 ); 
   }

   if ( endian )
   {
      // Swap back to native:
      endian->swap( m_Ltlm );

      // Conditional ttlm array:
      if ( st == 2 )
      {
         endian->swap( (ossim_uint16*)m_Ttlm, tileCount );
      }

      // Conditional ptlm array:
      if (sp == 0 )
      {
         endian->swap( (ossim_uint16*)m_Ptlm, tileCount );
      }
      else if ( sp == 1 )
      {
         endian->swap( (ossim_uint32*)m_Ptlm, tileCount );
      }

      // Cleanup:      
      delete endian;
      endian = 0;
   }
}

ossim_uint8 ossimJ2kTlmRecord::getSt() const
{
   // 5th and 6th bits.
   return (m_Stlm & 0x30) >> 4;
}

bool ossimJ2kTlmRecord::setSt(ossim_uint8 bits )
{
   bool result = true;
   
   // 5th and 6th bits
   if ( bits == 0 )
   {
      // Clear both 5th and 6th bit.
      m_Stlm = m_Stlm & 0xcf;
   }
   else if ( bits == 1 )
   {
      // Set the 5th bit clear the 6th bit.
      m_Stlm = (m_Stlm & 0xdf) | 0x10;
   }
   else if ( bits == 2 )
   {
      // Set the 6th bit, clear the 5th bit.
      m_Stlm = (m_Stlm & 0xef) | 0x20;
   }
   else
   {
      result = false;
   }  

   return result;
}

ossim_uint8 ossimJ2kTlmRecord::getZtlm() const
{
   return m_Ztlm;
}

ossim_uint8  ossimJ2kTlmRecord::getSp() const
{
   // Last two bits.
   return m_Stlm >> 6;
}

bool ossimJ2kTlmRecord::setSp(ossim_uint8 bit )
{
   bool result = true;
   
   // 7th bit
   if ( bit == 0 )
   {
      // Clear the 7th bit:
      m_Stlm = m_Stlm & 0xbf;
   }
   else if ( bit == 1 )
   {
      // Set the 7th bit.
      m_Stlm = m_Stlm | 0x40;
   }
   else
   {
      result = false;
   }
   return result;
}

bool ossimJ2kTlmRecord::initPtlmArray( ossim_uint8 spBit, ossim_uint16 count )
{
   bool result = setSp( spBit );

   clearPtlm();

   if ( spBit == 0 )
   {
      m_Ptlm = new ossim_uint16[count];
      std::memset( m_Ptlm, 0, count*2 );
   }
   else if ( spBit == 1 )
   {
      m_Ptlm = new ossim_uint32[count];
      std::memset( m_Ptlm, 0, count*4 );
   }

   m_Ltlm = computeLength( count );

   return result;
}

ossim_uint16 ossimJ2kTlmRecord::getTileCount() const
{
   // See Table 7-21 BPJ2K01.10:
   ossim_uint16 result = 0;
   if ( m_Ltlm )
   {
      ossim_uint8 st = getSt();
      ossim_uint8 sp = getSp();
      ossim_uint16 x = st + (sp==0?2:4);
      if ( x )
      {
         result = (m_Ltlm - 4) / x;
      }
   }
   return result;
}

bool ossimJ2kTlmRecord::getTileLength( ossim_int32 index, ossim_uint32& length ) const
{
   bool status = false;
   if ( (index >= 0) && ( index < getTileCount() ) )
   {
      ossim_uint8 sp = getSp();
      if ( sp == 0 )
      {
         ossim_uint16* p = (ossim_uint16*)m_Ptlm;
         length = p[index];
         status = true;
      }
      else if ( sp == 1 )
      {
         ossim_uint32* p = (ossim_uint32*)m_Ptlm;
         length = p[index];
         status = true;
      }
   }

   return status;
}

bool ossimJ2kTlmRecord::setTileLength(
   ossim_int32 index, ossim_uint32 length )
{
   bool status = false;
   if ( (index >= 0) && ( index < getTileCount() ) )
   {
      ossim_uint8 sp = getSp();
      if ( sp == 0 )
      {
         ossim_uint16* p = (ossim_uint16*)m_Ptlm;
         p[index] = static_cast<ossim_uint16>(length);
         status = true;
      }
      else if ( sp == 1 )
      {
         ossim_uint32* p = (ossim_uint32*)m_Ptlm;
         p[index] = length;
         status = true;
      }
   }
   
   return status;
}

bool ossimJ2kTlmRecord::accumulate(
   ossim_int32 first, ossim_int32 last, std::streampos& init ) const
{
   bool status = false;
   if ( (first >= 0) && (last>first) && (last <= getTileCount()) )
   {
      ossim_uint8 sp = getSp();
      if ( sp == 0 )
      {
         ossim_uint16* p = (ossim_uint16*)m_Ptlm;
         for ( ossim_int32 i = first; i < last; ++i )
         {
            init += p[i];
            status = true;
         }            
      }
      else if ( sp == 1 )
      {
         ossim_uint32* p = (ossim_uint32*)m_Ptlm;
         for ( ossim_int32 i = first; i < last; ++i )
         {
            init += p[i];
            status = true;
         }
      }
   }
   return status;
}

ossim_uint16 ossimJ2kTlmRecord::computeLength( ossim_uint16 tileCount ) const
{
   return 4 + ( getSt() + (getSp()==1?4:2) ) * tileCount;
}

std::ostream& ossimJ2kTlmRecord::print(std::ostream& out,
                                       const std::string& prefix) const
{
   // Capture the original flags.
   std::ios_base::fmtflags f = out.flags();

   std::string pfx = prefix;
   pfx += "tlm.";
   
   out << pfx << "marker:  0xff55\n"
       << pfx << "Ltlm:    " << m_Ltlm       << "\n"
       << pfx << "Ztlm:    " << int(m_Ztlm)  << "\n";

   out.setf(std::ios_base::hex, std::ios_base::basefield);
   out << pfx << "Stlm:    0x" << int(m_Stlm) << "\n";
   out.setf(std::ios_base::fmtflags(0), std::ios_base::basefield);
   out << pfx << "Stlm.ST: " << int(getSt()) << "\n"
       << pfx << "Stlm.SP: " << int(getSp()) << "\n";

   ossim_uint16 tile_count = getTileCount();
   if ( tile_count )
   {
      ossim_uint8 st = getSt();
      if ( m_Ttlm == 0 )
      {
         out << pfx << "Ttlm:    null\n";
      }
      else if ( st == 1 ) // 8 bit
      {
         ossim_uint8* p = (ossim_uint8*)m_Ttlm;
         for ( ossim_uint16 i = 0; i < tile_count; ++i )
         {
            out << pfx << "Ttlm[" << i << "]: " << (int)p[i] << "\n";
         }
      }
      else if ( st == 2 ) // 16 bit
      {
         ossim_uint16* p = (ossim_uint16*)m_Ttlm;
         for ( ossim_uint16 i = 0; i < tile_count; ++i )
         {
            out << pfx << "Ttlm[" << i << "]: " << p[i] << "\n";
         }
      }

      ossim_uint8 sp = getSp();
      if ( sp == 0 ) // 16 bit
      {
         ossim_uint16* p = (ossim_uint16*)m_Ptlm;
         for ( ossim_uint16 i = 0; i < tile_count; ++i )
         {
            out << pfx << "Ptlm[" << i << "]: " << p[i] << "\n";
         }
      }
      else if ( sp == 1 ) // 32 bit
      {
         ossim_uint32* p = (ossim_uint32*)m_Ptlm;
         for ( ossim_uint16 i = 0; i < tile_count; ++i )
         {
            out << pfx << "Ptlm[" << i << "]: " << p[i] << "\n";
         }
      }
   }
   
   out.flush();

   // Reset flags.
   out.setf(f);

   return out;
}

std::ostream& operator<<(std::ostream& out, const ossimJ2kTlmRecord& obj)
{
   return obj.print(out);
}



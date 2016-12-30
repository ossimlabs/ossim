//---
//
// License: MIT
//
// Author:  David Burken
//
// Description: Container class for J2K Image and tile size (SIZ) record.
//
// See document BPJ2K01.00 Table 7-6 Image and tile size (15444-1 Annex A5.1)
// 
//---
// $Id$

#include <ossim/support_data/ossimJ2kSizRecord.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimIoStream.h>
#include <iostream>
#include <iomanip>


ossimJ2kSizRecord::ossimJ2kSizRecord()
   :
   m_marker(0xff51),
   m_Lsiz(0),
   m_Rsiz(0),
   m_Xsiz(0),
   m_Ysiz(0),
   m_XOsiz(0),
   m_YOsiz(0),
   m_XTsiz(0),
   m_YTsiz(0),
   m_XTOsiz(0),
   m_YTOsiz(0),
   m_Csiz(0),
   m_Ssiz(0),
   m_XRsiz(0),
   m_YRsiz(0)
{
}

ossimJ2kSizRecord::~ossimJ2kSizRecord()
{
}

void ossimJ2kSizRecord::parseStream(ossim::istream& in)
{
   // Get the stream posistion.
   std::streamoff pos = in.tellg();

   // Note: Marker is not read.
   in.read((char*)&m_Lsiz,      2);
   in.read((char*)&m_Rsiz,      2);
   in.read((char*)&m_Xsiz,      4);
   in.read((char*)&m_Ysiz,      4);
   in.read((char*)&m_XOsiz,     4);
   in.read((char*)&m_YOsiz,     4);
   in.read((char*)&m_XTsiz,     4);
   in.read((char*)&m_YTsiz,     4);
   in.read((char*)&m_XTOsiz,    4);
   in.read((char*)&m_YTOsiz,    4);
   in.read((char*)&m_Csiz,      2);
   in.read((char*)&m_Ssiz,      1);
   in.read((char*)&m_XRsiz,     1);
   in.read((char*)&m_YRsiz,     1);

   if (ossim::byteOrder() == OSSIM_LITTLE_ENDIAN)
   {
      // Stored big endian, must swap.
      ossimEndian s;
      s.swap(m_Lsiz);
      s.swap(m_Rsiz);
      s.swap(m_Xsiz);
      s.swap(m_Ysiz);
      s.swap(m_XOsiz);
      s.swap(m_YOsiz);
      s.swap(m_XTsiz);
      s.swap(m_YTsiz);
      s.swap(m_XTOsiz);
      s.swap(m_YTOsiz);
      s.swap(m_Csiz);
   }

   //---
   // Seek to next record.  This is needed because there are sometimes extra
   // bytes.
   //---
   in.seekg(pos + m_Lsiz, std::ios_base::beg);
}

ossimScalarType ossimJ2kSizRecord::getScalarType() const
{
   ossimScalarType result = OSSIM_SCALAR_UNKNOWN;

   // Bits per pixel first seven bits plus one.
   ossim_uint8 bpp = ( m_Ssiz & 0x3f ) + 1;

   // Signed bit is msb.
   bool isSigned = ( m_Ssiz & 0x80 ) ? true : false;

   // std::cout << "bpp: " << int(bpp) << " signed: " << isSigned << std::endl;
      
   if ( bpp <= 8 )
   {
      if ( isSigned == 0 )
      {
         result = OSSIM_UINT8;
      }
      else if (isSigned == 1)
      {
         result = OSSIM_SINT8;
      }
   }
   else if ( bpp == 11 )
   {
      if ( isSigned == 0 )
      {
         result = OSSIM_USHORT11;
      }
      else
      {
         result = OSSIM_SINT16;
      }
   }
   else if( bpp <= 16 )
   {
      if( isSigned == 0 )
      {
         result = OSSIM_UINT16;
      }
      else if( isSigned == 1 )
      {
         result = OSSIM_SINT16;
      }
   }
   return result;
}

std::ostream& ossimJ2kSizRecord::print(std::ostream& out,
                                       const std::string& prefix) const
{
   // Capture the original flags.
   std::ios_base::fmtflags f = out.flags();

   std::string pfx = prefix;
   pfx += "siz.";

   out.setf(std::ios_base::hex, std::ios_base::basefield);
   out << pfx << "marker: 0x" << m_marker << "\n";
   out.setf(std::ios_base::fmtflags(0), std::ios_base::basefield);

   out << pfx << "Lsiz:   " << m_Lsiz       << "\n"
       << pfx << "Rsiz:   " << m_Rsiz       << "\n"
       << pfx << "Xsiz:   " << m_Xsiz       << "\n"
       << pfx << "Yziz:   " << m_Ysiz       << "\n"
       << pfx << "XOsiz:  " << m_XOsiz      << "\n"
       << pfx << "YOsiz:  " << m_YOsiz      << "\n"
       << pfx << "XTsiz:  " << m_XTsiz      << "\n"
       << pfx << "YTsiz:  " << m_YTsiz      << "\n"
       << pfx << "XTOsiz: " << m_XTOsiz     << "\n"
       << pfx << "YTOsiz: " << m_YTOsiz     << "\n"
       << pfx << "Csiz:   " << m_Csiz       << "\n"
       << pfx << "Ssiz:   " << int(m_Ssiz)  << "\n"
       << pfx << "XRsiz:  " << int(m_XRsiz) << "\n"
       << pfx << "YRsiz:  " << int(m_YRsiz)
       << std::endl;

   // Reset flags.
   out.setf(f);

   return out;
}

std::ostream& operator<<(std::ostream& out, const ossimJ2kSizRecord& obj)
{
   return obj.print(out);
}

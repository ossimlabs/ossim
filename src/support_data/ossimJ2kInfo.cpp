//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: J2K Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/support_data/ossimJ2kInfo.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
// #include <ossim/support_data/ossimJ2kCommon.h>
#include <ossim/support_data/ossimJ2kCodRecord.h>
#include <ossim/support_data/ossimJ2kSizRecord.h>
#include <ossim/support_data/ossimJ2kSotRecord.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

// Static trace for debugging
static ossimTrace traceDebug("ossimJ2kInfo:debug");
static ossimTrace traceDump("ossimJ2kInfo:dump"); // This will dump offsets.

ossimJ2kInfo::ossimJ2kInfo()
   : ossimInfoBase(),
     m_file(),
     m_endian(0)
{
   // See if we need to byte swap.  J2k, JP2 boxes are big endian.
   if (ossim::byteOrder() == OSSIM_LITTLE_ENDIAN)
   {
      m_endian = new ossimEndian();
   }
}

ossimJ2kInfo::~ossimJ2kInfo()
{
   if (m_endian)
   {
      delete m_endian;
      m_endian = 0;
   }
}

bool ossimJ2kInfo::open(const ossimFilename& file)
{
   bool result = false;

   //---
   // Open the file.
   //---
   std::ifstream str(file.c_str(), std::ios_base::binary|std::ios_base::in);
   if (str.good()) 
   {
      //---
      // Check for the Start Of Codestream (SOC) and Size (SIZ) markers which
      // are required as first and second fields in the main header.
      //---
      ossim_uint16 soc;
      ossim_uint16 siz;
      readUInt16(soc, str);
      readUInt16(siz, str);

      const ossim_uint16 SOC_MARKER = 0xff4f; // start of codestream marker
      const ossim_uint16 SIZ_MARKER = 0xff51; // size maker
      
      if ( (soc == SOC_MARKER) && (siz == SIZ_MARKER) )
      {
         result = true; // Is a j2k...
      }
   }

   if (result)
   {
      
      m_file = file;
   }
   else
   {
      m_file.clear();
      if (m_endian)
      {
         delete m_endian;
         m_endian = 0;
      }
   }

   return result;
}

std::ostream& ossimJ2kInfo::print(std::ostream& out) const
{
   static const char MODULE[] = "ossimJ2kInfo::print";

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " DEBUG Entered...\n";
   }

   if ( m_file.size() )
   {  
      // Open the file.
      std::ifstream str(m_file.c_str(), std::ios_base::binary|std::ios_base::in);
      if (str.good())
      {
         ossim_uint16 marker;
         readUInt16(marker, str); // SOC
         readUInt16(marker, str); // SIZ
         
         std::string prefix = "j2k.";
         
         // SIZ marker required next.
         printSizMarker(out, prefix, str);
         
         readUInt16(marker, str);

         const ossim_uint16 COD_MARKER = 0xff52; // cod maker
         const ossim_uint16 EOC_MARKER = 0xffd9; // End of codestream marker.
         const ossim_uint16 SOT_MARKER = 0xff90; // start of tile marker
         
         while ( str.good() && (marker != EOC_MARKER) )
         {
            switch(marker)
            {
               case COD_MARKER:
               {
                  printCodMarker(out, prefix, str);
                  break;
               }
               case SOT_MARKER:
               {
                  printSotMarker(out, prefix, str);
                  break;
               }
               default:
               {
                  printUnknownMarker(out, prefix, str, marker);
               }
            }
            
            readUInt16(marker, str);
            
         }
      }
   }

   return out;
}

void ossimJ2kInfo::readUInt16(ossim_uint16& s, std::ifstream& str) const
{
   str.read((char*)&s, 2);
   if (m_endian)
   {
      m_endian->swap(s);
   }
}

std::ostream& ossimJ2kInfo::printCodMarker(std::ostream& out,
                                           const std::string& prefix,
                                           std::ifstream& str) const
{
   ossimJ2kCodRecord siz;
   siz.parseStream(str);
   siz.print(out, prefix);
   return out;
}

std::ostream& ossimJ2kInfo::printSizMarker(std::ostream& out,
                                           const std::string& prefix,
                                           std::ifstream& str) const
{
   ossimJ2kSizRecord siz;
   siz.parseStream(str);
   siz.print(out, prefix);
   return out;
}

std::ostream& ossimJ2kInfo::printSotMarker(std::ostream& out,
                                           const std::string& prefix,
                                           std::ifstream& str) const
{
   // Get the stream posistion.
   std::streamoff pos = str.tellg();
   
   ossimJ2kSotRecord sot;
   sot.parseStream(str);
   pos += sot.thePsot - 2;

   // Seek past the tile to the next marker.
   str.seekg(pos, std::ios_base::beg);
   
   sot.print(out,prefix);
   return out;
}


std::ostream& ossimJ2kInfo::printUnknownMarker(std::ostream& out,
                                               const std::string& prefix,
                                               std::ifstream& str,
                                               ossim_uint16 marker) const
{
   // Capture the original flags.
   std::ios_base::fmtflags f = out.flags();

   ossim_uint16 segmentLength;
   readUInt16(segmentLength, str);

   std::string pfx = prefix;
   pfx += "unknown.";

   out.setf(std::ios_base::hex, std::ios_base::basefield);
   out << pfx << "marker: 0x" << std::setfill('0') << std::setw(4)
       << marker << "\n";
   out.setf(std::ios_base::fmtflags(0), std::ios_base::basefield);
   
   out << pfx << "length: " << segmentLength
       << std::endl;

   // Reset flags.
   out.setf(f);

   // Seek to the next marker.
   str.seekg( (segmentLength-2), std::ios_base::cur);

   return out;
}
      


//---
// 
// License: MIT
//
// Author:  David Burken
//
// Description: BNDPLB tag class definition.
//
// Reference documents:
// - MIL-PRF-32466A, Appendix C, C.2.1.7
// - Digital Geographic Information Exchange Standard (DIGEST),
//   Part 2 - Annex D, D1.2.7.7 BNDPL - Bounding Polygon
//
//---
// $Id$

#include <ossim/support_data/ossimNitfBndplbTag.h>
#include <ossim/base/ossimString.h>

#include <cstring> /* for memset */
#include <iomanip>
#include <iostream>

// static const ossimTrace traceDebug(ossimString("ossimNitfBndplbTag:debug"));

RTTI_DEF1(ossimNitfBndplbTag, "ossimNitfBndplbTag", ossimNitfRegisteredTag);

ossimNitfBndplbTag::ossimNitfBndplbTag()
   : ossimNitfRegisteredTag(),
     m_lon(),
     m_lat()
{
   // Set the tag name. Note the tag length is variable.
   setTagName(std::string("BNDPLB"));
   
   clearFields();
}

void ossimNitfBndplbTag::parseStream(std::istream& in)
{
   clearFields();

   // Points in polygon:
   in.read(m_numPts, NUM_PTS_SIZE);
   const ossim_uint32 POINTS = ossimString(m_numPts).toUInt32();
   
   m_lon.resize(POINTS);
   m_lat.resize(POINTS);
   char tempStr[PT_SIZE+1];
   tempStr[PT_SIZE] = '\0';
   for ( ossim_uint32 i = 0; i < POINTS; ++i )
   {
      in.read( tempStr, PT_SIZE );
      m_lon[i] = tempStr;

      in.read( tempStr, PT_SIZE );
      m_lat[i] = tempStr;
   }

   // Set the variable tag length:
   setTagLength( 4 + POINTS * 30 );
}

void ossimNitfBndplbTag::writeStream(std::ostream& out)
{
   out.write(m_numPts, NUM_PTS_SIZE);
   ossim_uint32 POINTS = ossimString(m_numPts).toUInt32();
   if ( ((ossim_uint32)m_lon.size() == POINTS) && ((ossim_uint32)m_lat.size() == POINTS) )
   {
      for ( ossim_uint32 i = 0; i < POINTS; ++i )
      {
         out.write( m_lon[i].data(), PT_SIZE );
         out.write( m_lat[i].data(), PT_SIZE );
      }
   }
   // else error...
}

void ossimNitfBndplbTag::clearFields()
{
   memset(m_numPts, 0, NUM_PTS_SIZE);
   m_numPts[NUM_PTS_SIZE] = '\0';
   m_lon.clear();
   m_lat.clear();
}

std::ostream& ossimNitfBndplbTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   // Grab the corners parsed into points.
   out << std::setiosflags(std::ios_base::left)
       << pfx << std::setw(24) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"   << getTagLength() << "\n"
       << pfx << std::setw(24) << "NUM_PTS:" << m_numPts << "\n";

   ossim_uint32 POINTS = ossimString(m_numPts).toUInt32();
   if ( ((ossim_uint32)m_lon.size() == POINTS) && ((ossim_uint32)m_lat.size() == POINTS) )
   {
      for ( ossim_uint32 i = 0; i < POINTS; ++i )
      {
         out << pfx << "LON" << i << std::setw(20) << ":" << m_lon[i] << "\n"
             << pfx << "LAT" << i << std::setw(20) << ":" << m_lat[i] << "\n";
      }
   }

   return out;
}

ossim_uint32 ossimNitfBndplbTag::getNumberOfPoints() const
{
   return ossimString(m_numPts).toUInt32();
}

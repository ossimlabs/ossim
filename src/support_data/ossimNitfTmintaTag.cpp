//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//
// Time Interval Definition TRE(TMINTA) class definition.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 14 for more info.
//
//---
// $Id

#include <ossim/support_data/ossimNitfTmintaTag.h>
#include <ossim/base/ossimString.h>
#include <cstring>
#include <iomanip>
#include <iostream>

void ossimNitfTmintaTime::parseStream(std::istream& in)
{
   clearFields();

   in.read(m_timeIntervalIndex, TIME_INTERVAL_INDEX_SIZE);
   in.read(m_startTimestamp, START_TIMESTAMP_SIZE);
   in.read(m_endTimestamp, END_TIMESTAMP_SIZE);   
}

void ossimNitfTmintaTime::writeStream(std::ostream& out) const
{
   out.write(m_timeIntervalIndex, TIME_INTERVAL_INDEX_SIZE);
   out.write(m_startTimestamp, START_TIMESTAMP_SIZE);
   out.write(m_endTimestamp, END_TIMESTAMP_SIZE);
}

std::ostream& ossimNitfTmintaTime::print( std::ostream& out,
                                          const std::string& prefix,
                                          ossim_uint32 index) const
{
   std::string pfx = prefix;
   pfx += "TIME";
   pfx += ossimString::toString(index).string();
   pfx += ".";

   const ossim_int32 W = 21;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(W) << "TIME_INTERVAL_INDEX:" << m_timeIntervalIndex << "\n"
       << pfx << std::setw(W) << "START_TIMESTAMP:" << m_startTimestamp << "\n"
       << pfx << std::setw(W) << "END_TIMESTAMP:" << m_endTimestamp << "\n";

   return out;
}

void ossimNitfTmintaTime::clearFields()
{
   std::memset( m_timeIntervalIndex, '0', TIME_INTERVAL_INDEX_SIZE );
   std::memset( m_startTimestamp, ' ', START_TIMESTAMP_SIZE );
   std::memset( m_endTimestamp, ' ', END_TIMESTAMP_SIZE );   
   
   m_timeIntervalIndex[TIME_INTERVAL_INDEX_SIZE] = '\0';
   m_startTimestamp[START_TIMESTAMP_SIZE] = '\0';
   m_endTimestamp[END_TIMESTAMP_SIZE] = '\0';   
}

// For ossimRtti stuff...
RTTI_DEF1(ossimNitfTmintaTag, "ossimNitfTmintaTag", ossimNitfRegisteredTag);

ossimNitfTmintaTag::ossimNitfTmintaTag()
   : ossimNitfRegisteredTag(std::string("TMINTA"), 0)
{
   clearFields();
}

void ossimNitfTmintaTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(m_numTimInt, NUM_TIM_INT_SIZE);
   ossim_uint32 count = ossimString(m_numTimInt).toUInt32();
   if ( count )
   {
      m_time.resize( count );
      for ( auto&& i : m_time )
      {
         i.parseStream( in );
      }
   }
}

void ossimNitfTmintaTag::writeStream(std::ostream& out)
{
   out.write(m_numTimInt, NUM_TIM_INT_SIZE);

   const auto& cv = m_time;
   for (auto&& i : cv)
   {
      i.writeStream( out );
   }
}

void ossimNitfTmintaTag::clearFields()
{
   std::memset(m_numTimInt, '0', NUM_TIM_INT_SIZE);
   m_numTimInt[NUM_TIM_INT_SIZE] = '\0';
   m_time.clear();
}

std::ostream& ossimNitfTmintaTag::print( std::ostream& out,
                                         const std::string& prefix ) const
{
   std::string pfx = prefix;
   pfx += "TMINTA.";

   const ossim_int32 W = 27;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(W) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(W) << "CEL:" << getTagLength() << "\n"
       << pfx << std::setw(W) << "NUM_TIM_INT:" << m_numTimInt << "\n";

   for (ossim_uint32 i = 0; i < (ossim_uint32)m_time.size(); ++i )
   {
      m_time[i].print(out, pfx, i);
   } 
   
   return out;
}

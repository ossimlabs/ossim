//---
//
// License: MIT
// 
// Author:  David Burken
//
// Description: GEOLOB tag class definition.
//
// References:
//
// 1) DIGEST 2.1 Part 2 - Annex D
// Appendix 1 - NSIF Standard Geospatial Support Data Extension
//
// 2) STDI-0006
//
//---
// $Id: ossimNitfBlockaTag.cpp 23245 2015-04-08 20:53:04Z rashadkm $

#include <cstring> /* for memcpy */
#include <sstream>
#include <iomanip>

#include <ossim/support_data/ossimNitfGeolobTag.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimDms.h>
#include <ossim/base/ossimDpt.h>

static const ossimTrace traceDebug(ossimString("ossimNitfBlockaTag:debug"));

RTTI_DEF1(ossimNitfGeolobTag, "ossimNitfGeolobTag", ossimNitfRegisteredTag);

ossimNitfGeolobTag::ossimNitfGeolobTag()
   : ossimNitfRegisteredTag(std::string("GEOLOB"), ossimNitfGeolobTag::TAG_SIZE),
     m_tagData(ossimNitfGeolobTag::TAG_SIZE, '0')
{
}

void ossimNitfGeolobTag::parseStream(std::istream& in)
{
   in.read((char*)m_tagData.data(), ossimNitfGeolobTag::TAG_SIZE);
}

void ossimNitfGeolobTag::writeStream(std::ostream& out)
{
   out.write(m_tagData.data(), ossimNitfGeolobTag::TAG_SIZE);
}

std::ostream& ossimNitfGeolobTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   out << setiosflags(ios::left)
       << pfx << std::setw(24) << "CETAG:" << getTagName()   << "\n"
       << pfx << std::setw(24) << "CEL:"   << getTagLength() << "\n"
       << pfx << std::setw(24) << "ARV:"   << getArvString() << "\n"
       << pfx << std::setw(24) << "BRV:"   << getBrvString() << "\n"
       << pfx << std::setw(24) << "LSO:"   << getLsoString() << "\n"
       << pfx << std::setw(24) << "PSO:"   << getPsoString() << "\n";

   return out;
}

// Longitude density:
std::string ossimNitfGeolobTag::getArvString() const
{
   return m_tagData.substr(0, ossimNitfGeolobTag::ARV_SIZE);
}

ossim_uint32 ossimNitfGeolobTag::getArv() const
{
   ossim_uint32 result = 0;
   std::string s = getArvString();
   if ( s.size() )
   {
      result = ossimString(s).toUInt32();
   }
   return result;
}

ossim_float64 ossimNitfGeolobTag::getDegreesPerPixelLon() const
{
   ossim_float64 result = 0.0;
   ossim_uint32 arv = getArv(); 
   if ( arv > 0 )
   {
      result = 360.0 / arv;
   }
   return result;
}

void ossimNitfGeolobTag::setArv(ossim_uint32 arv)
{
   if ( (arv >= 2) && (arv <= 999999999) )
   {
      std::ostringstream s;
      s.fill('0');
      s << std::setw(ossimNitfGeolobTag::ARV_SIZE) << arv;
      m_tagData.replace( 0, ossimNitfGeolobTag::ARV_SIZE, s.str() );
   }
}

void ossimNitfGeolobTag::setDegreesPerPixelLon(const ossim_float64& deltaLon)
{
   if ( deltaLon > 0.0 )
   {
      ossim_uint32 pixels = (ossim_uint32)((1.0/deltaLon)*360.0 + .5);
      setArv(pixels);
   }
}

// Latitude density:
std::string ossimNitfGeolobTag::getBrvString() const
{
   return m_tagData.substr(9, ossimNitfGeolobTag::BRV_SIZE);
}

ossim_float64 ossimNitfGeolobTag::getDegreesPerPixelLat() const
{
   ossim_float64 result = 0.0;
   ossim_uint32 brv = getBrv();
   if ( brv > 0 )
   {
      result = 360.0 / brv;
   }
   return result;
}

ossim_uint32 ossimNitfGeolobTag::getBrv() const
{
   ossim_uint32 result = 0;
   std::string s = getBrvString();
   if ( s.size() )
   {
      result = ossimString(s).toUInt32();
   }
   return result;
}

void ossimNitfGeolobTag::setBrv(ossim_uint32 brv)
{
   if ( (brv >= 2) && (brv <= 999999999) )
   {
      std::ostringstream s;
      s.fill('0');
      s << std::setw(ossimNitfGeolobTag::BRV_SIZE) << brv;

      m_tagData.replace( 9, ossimNitfGeolobTag::BRV_SIZE, s.str() );
   }
}

void ossimNitfGeolobTag::setDegreesPerPixelLat(const ossim_float64& deltaLat)
{
   if ( deltaLat > 0.0 )
   {
      ossim_uint32 pixels = (ossim_uint32)((1.0/deltaLat)*360.0 + .5);
      setBrv(pixels);
   }
}

// Longitude of Reference Origin:
std::string ossimNitfGeolobTag::getLsoString() const
{
   return m_tagData.substr(18, ossimNitfGeolobTag::LSO_SIZE);
}

ossim_float64 ossimNitfGeolobTag::getLso() const
{
   ossim_float64 result = 0;
   std::string s = getLsoString();
   if ( s.size() )
   {
      result = ossimString(s).toFloat64();
   }
   return result;
}

void ossimNitfGeolobTag::setLso(const ossim_float64& lso)
{
   if ( (lso >= -180.0) && (lso <= 180.0) )
   {
      std::ostringstream s;
      s.precision(10);
      s.fill('0');
      s << std::left << std::showpos << std::fixed
        << std::setw(ossimNitfGeolobTag::LSO_SIZE) << lso;

      m_tagData.replace( 18, ossimNitfGeolobTag::LSO_SIZE, s.str() );
   }
}

// Latitude of Reference Origin:
std::string ossimNitfGeolobTag::getPsoString() const
{
   return m_tagData.substr(33, ossimNitfGeolobTag::PSO_SIZE);
}

ossim_float64 ossimNitfGeolobTag::getPso() const
{
   ossim_float64 result = 0;
   std::string s = getPsoString();
   if ( s.size() )
   {
      result = ossimString(s).toFloat64();
   }
   return result;
}

void ossimNitfGeolobTag::setPso(const ossim_float64& pso)
{
   if ( (pso >= -180.0) && (pso <= 180.0) )
   {
      std::ostringstream s;
      s.precision(10);
      s.fill('0');
      s << std::left << std::showpos << std::fixed
        << std::setw(ossimNitfGeolobTag::PSO_SIZE) << pso;

      m_tagData.replace( 33, ossimNitfGeolobTag::PSO_SIZE, s.str() );
   }
}





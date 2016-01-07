//---
// File: ossimRsmpia.cpp
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//---

#include <ossim/support_data/ossimRsmpia.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/support_data/ossimNitfRsmpiaTag.h>

static std::string IID_KW     = "iid";
static std::string EDITION_KW = "edition";

static std::string R0_KW      = "r0";
static std::string RX_KW      = "rx";
static std::string RY_KW      = "ry";
static std::string RZ_KW      = "rz";

static std::string RXX_KW     = "rxx";
static std::string RXY_KW     = "rxy";
static std::string RXZ_KW     = "rxz";

static std::string RYY_KW     = "ryy";
static std::string RYZ_KW     = "ryz";
static std::string RZZ_KW     = "rzz";

static std::string C0_KW      = "c0";
static std::string CX_KW      = "cx";
static std::string CY_KW      = "cy";
static std::string CZ_KW      = "cz";

static std::string CXX_KW     = "cxx";
static std::string CXY_KW     = "cxy";
static std::string CXZ_KW     = "cxz";

static std::string CYY_KW     = "cyy";
static std::string CYZ_KW     = "cyz";
static std::string CZZ_KW     = "czz";

static std::string RNIS_KW    = "rnis";
static std::string CNIS_KW    = "cnis";
static std::string TNIS_KW    = "tnis";

static std::string RSSIZ_KW   = "rssiz";
static std::string CSSIZ_KW   = "cssiz";

ossimRsmpia::ossimRsmpia()
   :
   m_iid(),
   m_edition(),

   m_r0(0.0),
   m_rx(0.0),
   m_ry(0.0),
   m_rz(0.0),

   m_rxx(0.0),
   m_rxy(0.0),
   m_rxz(0.0),

   m_ryy(0.0),
   m_ryz(0.0),
   m_rzz(0.0),

   m_c0(0.0),
   m_cx(0.0),
   m_cy(0.0),
   m_cz(0.0),

   m_cxx(0.0),
   m_cxy(0.0),
   m_cxz(0.0),   

   m_cyy(0.0),
   m_cyz(0.0),
   m_czz(0.0),

   m_rnis(0),
   m_cnis(0),
   m_tnis(0),

   m_rssiz(0.0),
   m_cssiz(0.0)
{
}

ossimRsmpia::ossimRsmpia( const ossimRsmpia& obj )
   :
   m_iid( obj.m_iid ),
   m_edition( obj.m_edition ),

   m_r0( obj.m_r0 ),
   m_rx( obj.m_rx ),
   m_ry( obj.m_ry ),
   m_rz( obj.m_rz ),

   m_rxx( obj.m_rxx ),
   m_rxy( obj.m_rxy ),
   m_rxz( obj.m_rxz ),

   m_ryy( obj.m_ryy ),
   m_ryz( obj.m_ryz ),
   m_rzz( obj.m_rzz ),

   m_c0( obj.m_c0 ),
   m_cx( obj.m_cx ),
   m_cy( obj.m_cy ),
   m_cz( obj.m_cz ),

   m_cxx( obj.m_cxx ),
   m_cxy( obj.m_cxy ),
   m_cxz( obj.m_cxz ),   

   m_cyy( obj.m_cyy ),
   m_cyz( obj.m_cyz ),   
   m_czz( obj.m_czz ),

   m_rnis( obj.m_rnis ),
   m_cnis( obj.m_cnis ),
   m_tnis( obj.m_tnis ),

   m_rssiz( obj.m_rssiz ),
   m_cssiz( obj.m_cssiz )
{
}

const ossimRsmpia& ossimRsmpia::operator=( const ossimRsmpia& rhs )
{
   if (this != &rhs)
   {
      m_iid = rhs.m_iid;
      m_edition = rhs.m_edition;

      m_r0 = rhs.m_r0;
      m_rx = rhs.m_rx;
      m_ry = rhs.m_ry;
      m_rz = rhs.m_rz;

      m_rxx = rhs.m_rxx;
      m_rxy = rhs.m_rxy;
      m_rxz = rhs.m_rxz;

      m_ryy = rhs.m_ryy;
      m_ryz = rhs.m_ryz;
      m_rzz = rhs.m_rzz;

      m_c0 = rhs.m_c0;
      m_cx = rhs.m_cx;
      m_cy = rhs.m_cy;
      m_cz = rhs.m_cz;

      m_cxx = rhs.m_cxx;
      m_cxy = rhs.m_cxy;
      m_cxz = rhs.m_cxz;      

      m_cyy = rhs.m_cyy;
      m_cyz = rhs.m_cyz;      
      m_czz = rhs.m_czz;

      m_rnis = rhs.m_rnis;
      m_cnis = rhs.m_cnis;
      m_tnis = rhs.m_tnis;

      m_rssiz = rhs.m_rssiz;
      m_cssiz = rhs.m_cssiz;
   }
   return *this;
}

void ossimRsmpia::saveState( ossimKeywordlist& kwl,
                             const std::string& prefix ) const
{
   std::string pfx = prefix + std::string("rsmpia.");
   
   kwl.add(pfx.c_str(), IID_KW.c_str(), m_iid.c_str());
   kwl.add(pfx.c_str(), EDITION_KW.c_str(), m_edition.c_str());

   kwl.add(pfx.c_str(), R0_KW.c_str(), m_r0);
   kwl.add(pfx.c_str(), RX_KW.c_str(), m_rx);
   kwl.add(pfx.c_str(), RY_KW.c_str(), m_ry);
   kwl.add(pfx.c_str(), RZ_KW.c_str(), m_rz);

   kwl.add(pfx.c_str(), RXX_KW.c_str(), m_rxx);
   kwl.add(pfx.c_str(), RXY_KW.c_str(), m_rxy);
   kwl.add(pfx.c_str(), RXZ_KW.c_str(), m_rxz);

   kwl.add(pfx.c_str(), RYY_KW.c_str(), m_ryy);
   kwl.add(pfx.c_str(), RYZ_KW.c_str(), m_ryz);
   kwl.add(pfx.c_str(), RZZ_KW.c_str(), m_rzz);

   kwl.add(pfx.c_str(), C0_KW.c_str(), m_c0);
   kwl.add(pfx.c_str(), CX_KW.c_str(), m_cx);
   kwl.add(pfx.c_str(), CY_KW.c_str(), m_cy);
   kwl.add(pfx.c_str(), CZ_KW.c_str(), m_cz);

   kwl.add(pfx.c_str(), CXX_KW.c_str(), m_cxx);
   kwl.add(pfx.c_str(), CXY_KW.c_str(), m_cxy);
   kwl.add(pfx.c_str(), CXZ_KW.c_str(), m_cxz);

   kwl.add(pfx.c_str(), CYY_KW.c_str(), m_cyy);
   kwl.add(pfx.c_str(), CYZ_KW.c_str(), m_cyz);
   kwl.add(pfx.c_str(), CZZ_KW.c_str(), m_czz);

   kwl.add(pfx.c_str(), RNIS_KW.c_str(), m_rnis);
   kwl.add(pfx.c_str(), CNIS_KW.c_str(), m_cnis);
   kwl.add(pfx.c_str(), TNIS_KW.c_str(), m_tnis);

   kwl.add(pfx.c_str(), RSSIZ_KW.c_str(), m_rssiz);
   kwl.add(pfx.c_str(), CSSIZ_KW.c_str(), m_cssiz);
   
} // End: ossimRsmpia::saveState( ... )

bool ossimRsmpia::loadState( const ossimKeywordlist& kwl,
                             const std::string& prefix )
{
   std::string pfx = prefix + std::string("rsmpia.");
   std::string key;
   std::string value;
   
   bool result = false; // Set to true on last key.
      
   while( 1 ) // Break out on error.
   {
      key = IID_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_iid = value;
      }
      else
      {
         break;
      }

      key = EDITION_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
        m_edition = value;
      }
      else
      {
         break;
      }

      key = R0_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_r0 = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = RX_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_rx = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }
      
      key = RY_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_ry = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = RZ_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_rz = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = RXX_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_rxx = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = RXY_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_rxy = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = RXZ_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_rxz = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = RYY_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_ryy = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = RYZ_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_ryz = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = RZZ_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_rzz = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = C0_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_c0 = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = CX_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_cx = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = CY_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_cy = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = CZ_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_cz = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = CXX_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_cxx = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = CXY_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_cxy = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = CXZ_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_cxz = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = CYY_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_cyy = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = CYZ_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_cyz = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = CZZ_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_czz = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = RNIS_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_rnis = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      key = CNIS_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_cnis = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      key = TNIS_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_tnis = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      key = RSSIZ_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_rssiz = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      key = CSSIZ_KW;  
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_cssiz = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      // If we get here we're good, so set the status for downstream code.
      result = true;

      // Final break from while forever loop.
      break;
      
   } // Matches: while( FOREVER )

   if ( result == false )
   {  
      // Find on key failed...
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossimRsmpia::loadState WARNING:\n"
         << "Error encountered parsing the following required keyword: "
         << "<" << key << ">. Check the keywordlist for proper syntax."
         << std::endl;
   }
   
   return result;
   
} // End: ossimRsmpia::loadState( ... )

bool ossimRsmpia::initialize( const ossimNitfRsmpiaTag* rsmpiaTag )
{
   bool status = false;
   
   if ( rsmpiaTag )
   {
      m_iid = rsmpiaTag->getIid().trim().string();
      m_edition = rsmpiaTag->getEdition().trim().string();

      m_r0 = rsmpiaTag->getR0().toFloat64();
      m_rx = rsmpiaTag->getRx().toFloat64();
      m_ry = rsmpiaTag->getRy().toFloat64();
      m_rz = rsmpiaTag->getRz().toFloat64();

      m_rxx = rsmpiaTag->getRxx().toFloat64();
      m_rxy = rsmpiaTag->getRxy().toFloat64();
      m_rxz = rsmpiaTag->getRxz().toFloat64();

      m_ryy = rsmpiaTag->getRyy().toFloat64();
      m_ryz = rsmpiaTag->getRyz().toFloat64();
      m_rzz = rsmpiaTag->getRzz().toFloat64();

      m_c0 = rsmpiaTag->getC0().toFloat64();
      m_cx = rsmpiaTag->getCx().toFloat64();
      m_cy = rsmpiaTag->getCy().toFloat64();
      m_cz = rsmpiaTag->getCz().toFloat64();

      m_cxx = rsmpiaTag->getCxx().toFloat64();
      m_cxy = rsmpiaTag->getCxy().toFloat64();
      m_cxz = rsmpiaTag->getCxz().toFloat64();

      m_cyy = rsmpiaTag->getCyy().toFloat64();
      m_cyz = rsmpiaTag->getCyz().toFloat64();
      m_czz = rsmpiaTag->getCzz().toFloat64();

      m_rnis = rsmpiaTag->getRnis().toUInt32();
      m_cnis = rsmpiaTag->getCnis().toUInt32();
      m_tnis = rsmpiaTag->getTnis().toUInt32();

      m_rssiz = rsmpiaTag->getRssiz().toFloat64();
      m_cssiz = rsmpiaTag->getCssiz().toFloat64();

      status = true;
      
   } // Matches: if ( rsmpiaTag )

   return status;
   
} // End: ossimRsmpia::initializeModel( rsmpiaTag )

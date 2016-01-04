//---
// File: ossimRsmida.cpp
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//---

#include <ossim/support_data/ossimRsmida.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/support_data/ossimNitfRsmidaTag.h>

static std::string IID_KW     = "iid";
static std::string EDITION_KW = "edition";

static std::string ISID_KW = "isid";
static std::string SID_KW = "sid";
static std::string STID_KW = "stid";

static std::string YEAR_KW = "year";
static std::string MONTH_KW = "month";
static std::string DAY_KW = "day";
static std::string HOUR_KW = "hour";
static std::string MINUTE_KW = "minute";
static std::string SECOND_KW = "second";

static std::string NRG_KW = "nrg";
static std::string NCG_KW = "ncg";

static std::string TRG_KW = "trg";
static std::string TCG_KW = "tcg";

static std::string GRNDD_KW = "grndd";

static std::string XUOR_KW = "xuor";
static std::string YUOR_KW = "yuor";
static std::string ZUOR_KW = "zuor";

static std::string XUXR_KW = "xuxr";
static std::string XUYR_KW = "xuyr";
static std::string XUZR_KW = "xuzr";

static std::string YUXR_KW = "yuxr";
static std::string YUYR_KW = "yuyr";
static std::string YUZR_KW = "yuzr";

static std::string ZUXR_KW = "zuxr";
static std::string ZUYR_KW = "zuyr";
static std::string ZUZR_KW = "zuzr";

static std::string V1X_KW = "v1x";
static std::string V1Y_KW = "v1y";
static std::string V1Z_KW = "v1z";

static std::string V2X_KW = "v2x";
static std::string V2Y_KW = "v2y";
static std::string V2Z_KW = "v2z";

static std::string V3X_KW = "v3x";
static std::string V3Y_KW = "v3y";
static std::string V3Z_KW = "v3z";

static std::string V4X_KW = "v4x";
static std::string V4Y_KW = "v4y";
static std::string V4Z_KW = "v4z";

static std::string V5X_KW = "v5x";
static std::string V5Y_KW = "v5y";
static std::string V5Z_KW = "v5z";

static std::string V6X_KW = "v6x";
static std::string V6Y_KW = "v6y";
static std::string V6Z_KW = "v6z";

static std::string V7X_KW = "v7x";
static std::string V7Y_KW = "v7y";
static std::string V7Z_KW = "v7z";

static std::string V8X_KW = "v8x";
static std::string V8Y_KW = "v8y";
static std::string V8Z_KW = "v8z";

static std::string GRPX_KW = "grpx";
static std::string GRPY_KW = "grpy";
static std::string GRPZ_KW = "grpz";

static std::string FULLR_KW = "fullr";
static std::string FULLC_KW = "fullc";

static std::string MINR_KW = "minr";
static std::string MAXR_KW = "maxr";
static std::string MINC_KW = "minc";
static std::string MAXC_KW = "maxc";

static std::string IE0_KW = "ie0";
static std::string IER_KW = "ier";
static std::string IEC_KW = "iec";
static std::string IERR_KW = "ierr";
static std::string IERC_KW = "ierc";
static std::string IECC_KW = "iecc";

static std::string IA0_KW = "ia0";
static std::string IAR_KW = "iar";
static std::string IAC_KW = "iac";
static std::string IARR_KW = "iarr";
static std::string IARC_KW = "iarc";
static std::string IACC_KW = "iacc";

static std::string SPX_KW = "spx";
static std::string SVX_KW = "svx";
static std::string SAX_KW = "sax";

static std::string SPY_KW = "spy";
static std::string SVY_KW = "svy";
static std::string SAY_KW = "say";

static std::string SPZ_KW = "spz";
static std::string SVZ_KW = "svz";
static std::string SAZ_KW = "saz";

ossimRsmida::ossimRsmida()
   :
   m_iid(),
   m_edition(),

   m_isid(),
   m_sid(),
   m_stid(),
   
   m_year(0),
   m_month(0),
   m_day(0),
   m_hour(0),
   m_minute(0),
   m_second(0.0),

   m_nrg(0),
   m_ncg(0),
   
   m_trg(0.0),
   m_tcg(0.0),
   
   m_grndd(),

   m_xuor(0.0),
   m_yuor(0.0),
   m_zuor(0.0),
   
   m_xuxr(0.0),
   m_xuyr(0.0),
   m_xuzr(0.0),
   
   m_yuxr(0.0),
   m_yuyr(0.0),
   m_yuzr(0.0),
   
   m_zuxr(0.0),
   m_zuyr(0.0),
   m_zuzr(0.0),
   
   m_v1x(0.0),
   m_v1y(0.0),
   m_v1z(0.0),
   
   m_v2x(0.0),
   m_v2y(0.0),
   m_v2z(0.0),
   
   m_v3x(0.0),
   m_v3y(0.0),
   m_v3z(0.0),
   
   m_v4x(0.0),
   m_v4y(0.0),
   m_v4z(0.0),
   
   m_v5x(0.0),
   m_v5y(0.0),
   m_v5z(0.0),
   
   m_v6x(0.0),
   m_v6y(0.0),
   m_v6z(0.0),
   
   m_v7x(0.0),
   m_v7y(0.0),
   m_v7z(0.0),
   
   m_v8x(0.0),
   m_v8y(0.0),
   m_v8z(0.0),
   
   m_grpx(0.0),
   m_grpy(0.0),
   m_grpz(0.0),
   
   m_fullr(0),
   m_fullc(0),
   
   m_minr(0),
   m_maxr(0),
   m_minc(0),
   m_maxc(0),
   
   m_ie0(0.0),
   m_ier(0.0),
   m_iec(0.0),
   m_ierr(0.0),
   m_ierc(0.0),
   m_iecc(0.0),
   
   m_ia0(0.0),
   m_iar(0.0),
   m_iac(0.0),
   m_iarr(0.0),
   m_iarc(0.0),
   m_iacc(0.0),
   
   m_spx(0.0),
   m_svx(0.0),
   m_sax(0.0),
   
   m_spy(0.0),
   m_svy(0.0),
   m_say(0.0),
   
   m_spz(0.0),
   m_svz(0.0),
   m_saz(0.0)
{
}

ossimRsmida::ossimRsmida( const ossimRsmida& obj )
   :
   m_iid( obj.m_iid ),
   m_edition( obj.m_edition ),

   m_isid( obj.m_isid ),
   m_sid( obj.m_sid ),
   m_stid( obj.m_stid ),
   
   m_year( obj.m_year ),
   m_month( obj.m_month ),
   m_day( obj.m_day ),
   m_hour( obj.m_hour ),
   m_minute( obj.m_minute ),
   m_second( obj.m_second ),

   m_nrg( obj.m_nrg ),
   m_ncg( obj.m_ncg ),
   
   m_trg( obj.m_trg ),
   m_tcg( obj.m_tcg ),
   
   m_grndd( obj.m_grndd ),

   m_xuor( obj.m_xuor ),
   m_yuor( obj.m_yuor ),
   m_zuor( obj.m_zuor ),
   
   m_xuxr( obj.m_xuxr ),
   m_xuyr( obj.m_xuyr ),
   m_xuzr( obj.m_xuzr ),
   
   m_yuxr( obj.m_yuxr ),
   m_yuyr( obj.m_yuyr ),
   m_yuzr( obj.m_yuzr ),
   
   m_zuxr( obj.m_zuxr ),
   m_zuyr( obj.m_zuyr ),
   m_zuzr( obj.m_zuzr ),
   
   m_v1x( obj.m_v1x ),
   m_v1y( obj.m_v1y ),
   m_v1z( obj.m_v1z ),
   
   m_v2x( obj.m_v2x ),
   m_v2y( obj.m_v2y ),
   m_v2z( obj.m_v2z ),
   
   m_v3x( obj.m_v3x ),
   m_v3y( obj.m_v3y ),
   m_v3z( obj.m_v3z ),
   
   m_v4x( obj.m_v4x ),
   m_v4y( obj.m_v4y ),
   m_v4z( obj.m_v4z ),
   
   m_v5x( obj.m_v5x ),
   m_v5y( obj.m_v5y ),
   m_v5z( obj.m_v5z ),
   
   m_v6x( obj.m_v6x ),
   m_v6y( obj.m_v6y ),
   m_v6z( obj.m_v6z ),
   
   m_v7x( obj.m_v7x ),
   m_v7y( obj.m_v7y ),
   m_v7z( obj.m_v7z ),
   
   m_v8x( obj.m_v8x ),
   m_v8y( obj.m_v8y ),
   m_v8z( obj.m_v8z ),
   
   m_grpx( obj.m_grpx ),
   m_grpy( obj.m_grpy ),
   m_grpz( obj.m_grpz ),
   
   m_fullr( obj.m_fullr ),
   m_fullc( obj.m_fullc ),
   
   m_minr( obj.m_minr ),
   m_maxr( obj.m_maxr ),
   m_minc( obj.m_minc ),
   m_maxc( obj.m_maxc ),
   
   m_ie0( obj.m_ie0 ),
   m_ier( obj.m_ier ),
   m_iec( obj.m_iec ),

   m_ierr( obj.m_ierr ),
   m_ierc( obj.m_ierc ),
   m_iecc( obj.m_iecc ),
   
   m_ia0( obj.m_ia0 ),
   m_iar( obj.m_iar ),
   m_iac( obj.m_iac ),

   m_iarr( obj.m_iarr ),
   m_iarc( obj.m_iarc ),
   m_iacc( obj.m_iacc ),
   
   m_spx( obj.m_spx ),
   m_svx( obj.m_svx ),
   m_sax( obj.m_sax ),
   
   m_spy( obj.m_spy ),
   m_svy( obj.m_svy ),
   m_say( obj.m_say ),
   
   m_spz( obj.m_spz ),
   m_svz( obj.m_svz ),
   m_saz( obj.m_saz )
{
}

const ossimRsmida& ossimRsmida::operator=( const ossimRsmida& rhs )
{
   if (this != &rhs)
   {
      m_iid = rhs.m_iid;
      m_edition = rhs.m_edition;
      
      m_isid = rhs.m_isid;
      m_sid  = rhs.m_sid;
      m_stid = rhs.m_stid;
      
      m_year   = rhs.m_year;
      m_month  = rhs.m_month;
      m_day    = rhs.m_day;
      m_hour   = rhs.m_hour;
      m_minute = rhs.m_minute;
      m_second = rhs.m_second;
      
      m_nrg = rhs.m_nrg;
      m_ncg = rhs.m_ncg;
      
      m_trg = rhs.m_trg;
      m_tcg = rhs.m_tcg;
      
      m_grndd = rhs.m_grndd;
      
      m_xuor = rhs.m_xuor;
      m_yuor = rhs.m_yuor;
      m_zuor = rhs.m_zuor;
      
      m_xuxr = rhs.m_xuxr;
      m_xuyr = rhs.m_xuyr;
      m_xuzr = rhs.m_xuzr;
      
      m_yuxr = rhs.m_yuxr;
      m_yuyr = rhs.m_yuyr;
      m_yuzr = rhs.m_yuzr;
      
      m_zuxr = rhs.m_zuxr;
      m_zuyr = rhs.m_zuyr;
      m_zuzr = rhs.m_zuzr;
      
      m_v1x = rhs.m_v1x;
      m_v1y = rhs.m_v1y;
      m_v1z = rhs.m_v1z;
      
      m_v2x = rhs.m_v2x;
      m_v2y = rhs.m_v2y;
      m_v2z = rhs.m_v2z;
      
      m_v3x = rhs.m_v3x;
      m_v3y = rhs.m_v3y;
      m_v3z = rhs.m_v3z;
      
      m_v4x = rhs.m_v4x;
      m_v4y = rhs.m_v4y;
      m_v4z = rhs.m_v4z;
      
      m_v5x = rhs.m_v5x;
      m_v5y = rhs.m_v5y;
      m_v5z = rhs.m_v5z;
      
      m_v6x = rhs.m_v6x;
      m_v6y = rhs.m_v6y;
      m_v6z = rhs.m_v6z;
      
      m_v7x = rhs.m_v7x;
      m_v7y = rhs.m_v7y;
      m_v7z = rhs.m_v7z;
      
      m_v8x = rhs.m_v8x;
      m_v8y = rhs.m_v8y;
      m_v8z = rhs.m_v8z;
      
      m_grpx = rhs.m_grpx;
      m_grpy = rhs.m_grpy;
      m_grpz = rhs.m_grpz;
      
      m_fullr = rhs.m_fullr;
      m_fullc = rhs.m_fullc;
      
      m_minr = rhs.m_minr;
      m_maxr = rhs.m_maxr;
      m_minc = rhs.m_minc;
      m_maxc = rhs.m_maxc;
      
      m_ie0 = rhs.m_ie0;
      m_ier = rhs.m_ier;
      m_iec = rhs.m_iec;
      
      m_ierr = rhs.m_ierr;
      m_ierc = rhs.m_ierc;
      m_iecc = rhs.m_iecc;
      
      m_ia0 = rhs.m_ia0;
      m_iar = rhs.m_iar;
      m_iac = rhs.m_iac;
      
      m_iarr = rhs.m_iarr;
      m_iarc = rhs.m_iarc;
      m_iacc = rhs.m_iacc;
      
      m_spx = rhs.m_spx;
      m_svx = rhs.m_svx;
      m_sax = rhs.m_sax;
      
      m_spy = rhs.m_spy;
      m_svy = rhs.m_svy;
      m_say = rhs.m_say;
      
      m_spz = rhs.m_spz;
      m_svz = rhs.m_svz;
      m_saz = rhs.m_saz;
   }
   return *this;
}

void ossimRsmida::saveState( ossimKeywordlist& kwl,
                             const std::string& prefix ) const
{
   std::string pfx = prefix + std::string("rsmida.");
   
   kwl.add(pfx.c_str(), IID_KW.c_str(), m_iid.c_str());
   kwl.add(pfx.c_str(), EDITION_KW.c_str(), m_edition.c_str());
   
   kwl.add(pfx.c_str(), ISID_KW.c_str(), m_isid.c_str());   
   kwl.add(pfx.c_str(), SID_KW.c_str(), m_sid.c_str());   
   kwl.add(pfx.c_str(), STID_KW.c_str(), m_stid.c_str());   

   kwl.add(pfx.c_str(), YEAR_KW.c_str(), m_year);   
   kwl.add(pfx.c_str(), MONTH_KW.c_str(), m_month);   
   kwl.add(pfx.c_str(), DAY_KW.c_str(), m_day);   
   kwl.add(pfx.c_str(), HOUR_KW.c_str(), m_hour);   
   kwl.add(pfx.c_str(), MINUTE_KW.c_str(), m_minute);
   kwl.add(pfx.c_str(), SECOND_KW.c_str(), m_second);

   kwl.add(pfx.c_str(), NRG_KW.c_str(), m_nrg);
   kwl.add(pfx.c_str(), NCG_KW.c_str(), m_ncg);

   kwl.add(pfx.c_str(), TRG_KW.c_str(), m_trg);
   kwl.add(pfx.c_str(), TCG_KW.c_str(), m_tcg);
   
   kwl.add(pfx.c_str(), GRNDD_KW.c_str(), m_grndd.c_str());

   kwl.add(pfx.c_str(), XUOR_KW.c_str(), m_xuor);   
   kwl.add(pfx.c_str(), YUOR_KW.c_str(), m_yuor);
   kwl.add(pfx.c_str(), ZUOR_KW.c_str(), m_zuor);   

   kwl.add(pfx.c_str(), XUXR_KW.c_str(), m_xuxr);
   kwl.add(pfx.c_str(), XUYR_KW.c_str(), m_xuyr);
   kwl.add(pfx.c_str(), XUZR_KW.c_str(), m_xuzr);   

   kwl.add(pfx.c_str(), YUXR_KW.c_str(), m_yuxr); 
   kwl.add(pfx.c_str(), YUYR_KW.c_str(), m_yuyr);
   kwl.add(pfx.c_str(), YUZR_KW.c_str(), m_yuzr);   

   kwl.add(pfx.c_str(), ZUXR_KW.c_str(), m_zuxr);   
   kwl.add(pfx.c_str(), ZUYR_KW.c_str(), m_zuyr);
   kwl.add(pfx.c_str(), ZUZR_KW.c_str(), m_zuzr);   

   kwl.add(pfx.c_str(), V1X_KW.c_str(), m_v1x);
   kwl.add(pfx.c_str(), V1Y_KW.c_str(), m_v1y);
   kwl.add(pfx.c_str(), V1Z_KW.c_str(), m_v1z);   

   kwl.add(pfx.c_str(), V2X_KW.c_str(), m_v2x);
   kwl.add(pfx.c_str(), V2Y_KW.c_str(), m_v2y);
   kwl.add(pfx.c_str(), V2Z_KW.c_str(), m_v2z);

   kwl.add(pfx.c_str(), V3X_KW.c_str(), m_v3x);
   kwl.add(pfx.c_str(), V3Y_KW.c_str(), m_v3y);
   kwl.add(pfx.c_str(), V3Z_KW.c_str(), m_v3z);

   kwl.add(pfx.c_str(), V4X_KW.c_str(), m_v4x);  
   kwl.add(pfx.c_str(), V4Y_KW.c_str(), m_v4y);
   kwl.add(pfx.c_str(), V4Z_KW.c_str(), m_v4z);  

   kwl.add(pfx.c_str(), V5X_KW.c_str(), m_v5x);  
   kwl.add(pfx.c_str(), V5Y_KW.c_str(), m_v5y);
   kwl.add(pfx.c_str(), V5Z_KW.c_str(), m_v5z);  

   kwl.add(pfx.c_str(), V6X_KW.c_str(), m_v6x);  
   kwl.add(pfx.c_str(), V6Y_KW.c_str(), m_v6y);
   kwl.add(pfx.c_str(), V6Z_KW.c_str(), m_v6z);  

   kwl.add(pfx.c_str(), V7X_KW.c_str(), m_v7x);  
   kwl.add(pfx.c_str(), V7Y_KW.c_str(), m_v7y);
   kwl.add(pfx.c_str(), V7Z_KW.c_str(), m_v7z);   

   kwl.add(pfx.c_str(), V8X_KW.c_str(), m_v8x);  
   kwl.add(pfx.c_str(), V8Y_KW.c_str(), m_v8y);
   kwl.add(pfx.c_str(), V8Z_KW.c_str(), m_v8z);

   kwl.add(pfx.c_str(), GRPX_KW.c_str(), m_grpx);
   kwl.add(pfx.c_str(), GRPY_KW.c_str(), m_grpy);
   kwl.add(pfx.c_str(), GRPZ_KW.c_str(), m_grpz);

   kwl.add(pfx.c_str(), FULLR_KW.c_str(), m_fullr);
   kwl.add(pfx.c_str(), FULLC_KW.c_str(), m_fullc);

   kwl.add(pfx.c_str(), MINR_KW.c_str(), m_minr);
   kwl.add(pfx.c_str(), MAXR_KW.c_str(), m_maxr);
   kwl.add(pfx.c_str(), MINC_KW.c_str(), m_minc);   
   kwl.add(pfx.c_str(), MAXC_KW.c_str(), m_maxc);

   kwl.add(pfx.c_str(), IE0_KW.c_str(), m_ie0);
   kwl.add(pfx.c_str(), IER_KW.c_str(), m_ier);  
   kwl.add(pfx.c_str(), IEC_KW.c_str(), m_iec);

   kwl.add(pfx.c_str(), IERR_KW.c_str(), m_ierr);
   kwl.add(pfx.c_str(), IERC_KW.c_str(), m_ierc);   
   kwl.add(pfx.c_str(), IECC_KW.c_str(), m_iecc);

   kwl.add(pfx.c_str(), SPX_KW.c_str(), m_spx);
   kwl.add(pfx.c_str(), SVX_KW.c_str(), m_svx);   
   kwl.add(pfx.c_str(), SAX_KW.c_str(), m_sax);

   kwl.add(pfx.c_str(), SPY_KW.c_str(), m_spy);
   kwl.add(pfx.c_str(), SVY_KW.c_str(), m_svy);   
   kwl.add(pfx.c_str(), SAY_KW.c_str(), m_say);

   kwl.add(pfx.c_str(), SPZ_KW.c_str(), m_spz);
   kwl.add(pfx.c_str(), SVZ_KW.c_str(), m_svz);   
   kwl.add(pfx.c_str(), SAZ_KW.c_str(), m_saz);

} // End: ossimRsmida::saveState( ... )

bool ossimRsmida::loadState( const ossimKeywordlist& kwl,
                             const std::string& prefix )
{
   std::string pfx = prefix + std::string("rsmida.");
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

      key = ISID_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
        m_isid = value;
      }
      else
      {
         break;
      }

      key = SID_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
        m_sid = value;
      }
      else
      {
         break;
      }

      key = STID_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
        m_stid = value;
      }
      else
      {
         break;
      }

      key = YEAR_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_year = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      key = MONTH_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_month = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      key = HOUR_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_hour = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      key = MINUTE_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_minute = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      key = SECOND_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_second = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = NRG_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_nrg = ossimString(value).toInt32();
      }
      else
      {
         break;
      }

      key = NCG_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_ncg = ossimString(value).toInt32();
      }
      else
      {
         break;
      }

      key = TRG_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_trg = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = TCG_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_tcg = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = GRNDD_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_grndd = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = XUOR_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_xuor = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = YUOR_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_yuor = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = ZUOR_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_zuor = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = XUXR_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_zuor = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = XUYR_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_zuor = ossimString(value).toFloat64();
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
         << "ossimRsmida::loadState WARNING:\n"
         << "Error encountered parsing the following required keyword: "
         << "<" << key << ">. Check the keywordlist for proper syntax."
         << std::endl;
   }
   
   return result;
   
} // End: ossimRsmida::loadState( ... )

bool ossimRsmida::initialize( const ossimNitfRsmidaTag* rsmidaTag )
{
   bool status = false;
   
   if ( rsmidaTag )
   {
      m_iid = rsmidaTag->getIid().trim().string();
      m_edition = rsmidaTag->getEdition().trim().string();

      m_isid = rsmidaTag->getIsid().trim().string();
      m_sid  = rsmidaTag->getSid().trim().string();
      m_stid = rsmidaTag->getStid().trim().string();

      m_year   = rsmidaTag->getYear().toUInt32();
      m_month  = rsmidaTag->getMonth().toUInt32();
      m_day    = rsmidaTag->getDay().toUInt32();
      m_hour   = rsmidaTag->getHour().toUInt32();
      m_minute = rsmidaTag->getMinute().toUInt32();
      m_second = rsmidaTag->getSecond().toFloat64();
      
      m_nrg = rsmidaTag->getNrg().toUInt32();
      m_ncg = rsmidaTag->getNcg().toUInt32();
      
      m_trg = rsmidaTag->getTrg().toFloat64();
      m_tcg = rsmidaTag->getTcg().toFloat64();

      m_grndd = rsmidaTag->getGrndd().string();
      
      m_xuor = rsmidaTag->getXuor().toFloat64();
      m_yuor = rsmidaTag->getYuor().toFloat64();
      m_zuor = rsmidaTag->getZuor().toFloat64();
      
      m_xuxr = rsmidaTag->getXuxr().toFloat64();
      m_xuyr = rsmidaTag->getXuyr().toFloat64();
      m_xuzr = rsmidaTag->getXuzr().toFloat64();
      
      m_yuxr = rsmidaTag->getYuxr().toFloat64();
      m_yuyr = rsmidaTag->getYuyr().toFloat64();
      m_yuzr = rsmidaTag->getYuzr().toFloat64();
      
      m_zuxr = rsmidaTag->getZuxr().toFloat64();
      m_zuyr = rsmidaTag->getZuyr().toFloat64();
      m_zuzr = rsmidaTag->getZuzr().toFloat64();
      
      m_v1x = rsmidaTag->getV1x().toFloat64();
      m_v1y = rsmidaTag->getV1y().toFloat64();
      m_v1z = rsmidaTag->getV1z().toFloat64();
      
      m_v2x = rsmidaTag->getV2x().toFloat64();
      m_v2y = rsmidaTag->getV2y().toFloat64();
      m_v2z = rsmidaTag->getV2z().toFloat64();
      
      m_v3x = rsmidaTag->getV3x().toFloat64();
      m_v3y = rsmidaTag->getV3y().toFloat64();
      m_v3z = rsmidaTag->getV3z().toFloat64();
      
      m_v4x = rsmidaTag->getV4x().toFloat64();
      m_v4y = rsmidaTag->getV4y().toFloat64();
      m_v4z = rsmidaTag->getV4z().toFloat64();
      
      m_v5x = rsmidaTag->getV5x().toFloat64();
      m_v5y = rsmidaTag->getV5y().toFloat64();
      m_v5z = rsmidaTag->getV5z().toFloat64();
      
      m_v6x = rsmidaTag->getV6x().toFloat64();
      m_v6y = rsmidaTag->getV6y().toFloat64();
      m_v6z = rsmidaTag->getV6z().toFloat64();
      
      m_v7x = rsmidaTag->getV7x().toFloat64();
      m_v7y = rsmidaTag->getV7y().toFloat64();
      m_v7z = rsmidaTag->getV7z().toFloat64();
      
      m_v8x = rsmidaTag->getV8x().toFloat64();
      m_v8y = rsmidaTag->getV8y().toFloat64();
      m_v8z = rsmidaTag->getV8z().toFloat64();
      
      m_grpx = rsmidaTag->getGrpx().toFloat64();
      m_grpy = rsmidaTag->getGrpy().toFloat64();
      m_grpz = rsmidaTag->getGrpz().toFloat64();
      
      m_fullr = rsmidaTag->getFullr().toUInt32();
      m_fullc = rsmidaTag->getFullc().toUInt32();
      
      m_minr = rsmidaTag->getMinr().toUInt32();
      m_maxr = rsmidaTag->getMaxr().toUInt32();
      m_minc = rsmidaTag->getMinc().toUInt32();
      m_maxc = rsmidaTag->getMaxc().toUInt32();
      
      m_ie0 = rsmidaTag->getIe0().toFloat64();
      m_ier = rsmidaTag->getIer().toFloat64();
      m_iec = rsmidaTag->getIec().toFloat64();
      
      m_ierr = rsmidaTag->getIerr().toFloat64();
      m_ierc = rsmidaTag->getIerc().toFloat64();
      m_iecc = rsmidaTag->getIecc().toFloat64();
      
      m_ia0 = rsmidaTag->getIa0().toFloat64();
      m_iar = rsmidaTag->getIar().toFloat64();
      m_iac = rsmidaTag->getIac().toFloat64();
      
      m_iarr = rsmidaTag->getIarr().toFloat64();
      m_iarc = rsmidaTag->getIarc().toFloat64();
      m_iacc = rsmidaTag->getIacc().toFloat64();
      
      m_spx = rsmidaTag->getSpx().toFloat64();
      m_svx = rsmidaTag->getSvx().toFloat64();
      m_sax = rsmidaTag->getSax().toFloat64();
      
      m_spy = rsmidaTag->getSpy().toFloat64();
      m_svy = rsmidaTag->getSvy().toFloat64();
      m_say = rsmidaTag->getSay().toFloat64();
      
      m_spz = rsmidaTag->getSpz().toFloat64();
      m_svz = rsmidaTag->getSvz().toFloat64();
      m_saz = rsmidaTag->getSaz().toFloat64();

      status = true;
      
   } // Matches: if ( rsmidaTag )

   return status;
   
} // End: ossimRsmida::initializeModel( rsmidaTag )

//---
// File: ossimRsmpca.cpp
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//---

#include <ossim/support_data/ossimRsmpca.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/support_data/ossimNitfRsmpcaTag.h>

static std::string IID_KW = "iid";
static std::string EDITION_KW = "edition";

static std::string RSN_KW = "rsn";
static std::string CSN_KW = "csn";

static std::string RFEP_KW = "rfep";
static std::string CFEP_KW = "cfep";

static std::string RNRMO_KW = "rnrmo";
static std::string CNRMO_KW = "cnrmo";
static std::string XNRMO_KW = "xnrmo";
static std::string YNRMO_KW = "ynrmo";
static std::string ZNRMO_KW = "znrmo";

static std::string RNRMSF_KW = "rnrmsf";
static std::string CNRMSF_KW = "cnrmsf";
static std::string XNRMSF_KW = "xnrmsf";
static std::string YNRMSF_KW = "ynrmsf";
static std::string ZNRMSF_KW = "znrmsf";

static std::string RNPWRX_KW = "rnpwrx";
static std::string RNPWRY_KW = "rnpwry";
static std::string RNPWRZ_KW = "rnpwrz";
static std::string RNTRMS_KW = "rntrms";

static std::string RNPCF_KW = "rnpcf";

static std::string RDPWRX_KW = "rdpwrx";
static std::string RDPWRY_KW = "rdpwry";
static std::string RDPWRZ_KW = "rdpwrz";
static std::string RDTRMS_KW = "rdtrms";

static std::string RDPCF_KW = "rdpcf";

static std::string CNPWRX_KW = "cnpwrx";
static std::string CNPWRY_KW = "cnpwry";
static std::string CNPWRZ_KW = "cnpwrz";
static std::string CNTRMS_KW = "cntrms";

static std::string CNPCF_KW = "cnpcf";

static std::string CDPWRX_KW = "cdpwrx";
static std::string CDPWRY_KW = "cdpwry";
static std::string CDPWRZ_KW = "cdpwrz";
static std::string CDTRMS_KW = "cdtrms";

static std::string CDPCF_KW = "cdpcf";

ossimRsmpca::ossimRsmpca()
   :
   m_iid(),
   m_edition(),

   m_rsn(0),
   m_csn(0),

   m_rfep(0.0),
   m_cfep(0.0),

   m_rnrmo(0.0),
   m_cnrmo(0.0),
   m_xnrmo(0.0),
   m_ynrmo(0.0),
   m_znrmo(0.0),

   m_rnrmsf(0.0),
   m_cnrmsf(0.0),
   m_xnrmsf(0.0),
   m_ynrmsf(0.0),
   m_znrmsf(0.0),
   
   m_rnpwrx(0),
   m_rnpwry(0),
   m_rnpwrz(0),
   m_rntrms(0),

   m_rnpcf(),
   
   m_rdpwrx(0),
   m_rdpwry(0),
   m_rdpwrz(0),
   m_rdtrms(0),

   m_rdpcf(),
   
   m_cnpwrx(0),
   m_cnpwry(0),
   m_cnpwrz(0),
   m_cntrms(0),

   m_cnpcf(),
   
   m_cdpwrx(0),
   m_cdpwry(0),
   m_cdpwrz(0),
   m_cdtrms(0),

   m_cdpcf()
{
}

ossimRsmpca::ossimRsmpca( const ossimRsmpca& obj )
   :
   m_iid( obj.m_iid ),
   m_edition( obj.m_edition ),

   m_rsn( obj.m_rsn ),
   m_csn( obj.m_csn ),

   m_rfep( obj.m_rfep ),
   m_cfep( obj.m_cfep ),

   m_rnrmo( obj.m_rnrmo ),
   m_cnrmo( obj.m_cnrmo ),
   m_xnrmo( obj.m_xnrmo ),
   m_ynrmo( obj.m_ynrmo ),
   m_znrmo( obj.m_znrmo ),

   m_rnrmsf( obj.m_rnrmsf ),
   m_cnrmsf( obj.m_cnrmsf ),
   m_xnrmsf( obj.m_xnrmsf ),
   m_ynrmsf( obj.m_ynrmsf ),
   m_znrmsf( obj.m_znrmsf ),
   
   m_rnpwrx( obj.m_rnpwrx ),
   m_rnpwry( obj.m_rnpwry ),
   m_rnpwrz( obj.m_rnpwrz ),
   m_rntrms( obj.m_rntrms ),

   m_rnpcf( obj.m_rnpcf ),
   
   m_rdpwrx( obj.m_rdpwrx ),
   m_rdpwry( obj.m_rdpwry ),
   m_rdpwrz( obj.m_rdpwrz ),
   m_rdtrms( obj.m_rdtrms ),

   m_rdpcf( obj.m_rdpcf ),
   
   m_cnpwrx( obj.m_cnpwrx ),
   m_cnpwry( obj.m_cnpwry ),
   m_cnpwrz( obj.m_cnpwrz ),
   m_cntrms( obj.m_cntrms ),

   m_cnpcf( obj.m_cnpcf ),
   
   m_cdpwrx( obj.m_cdpwrx ),
   m_cdpwry( obj.m_cdpwry ),
   m_cdpwrz( obj.m_cdpwrz ),
   m_cdtrms( obj.m_cdtrms ),

   m_cdpcf( obj.m_cdpcf )
{
}

const ossimRsmpca& ossimRsmpca::operator=( const ossimRsmpca& rhs )
{
   if (this != &rhs)
   {
      m_iid = rhs.m_iid;
      m_edition = rhs.m_edition;
      
      m_rsn = rhs.m_rsn;
      m_csn = rhs.m_csn;
      
      m_rfep = rhs.m_rfep;
      m_cfep = rhs.m_cfep;

      m_rnrmo = rhs.m_rnrmo;
      m_cnrmo = rhs.m_cnrmo;
      m_xnrmo = rhs.m_xnrmo;
      m_ynrmo = rhs.m_ynrmo;
      m_znrmo = rhs.m_znrmo;

      m_rnrmsf = rhs.m_rnrmsf;
      m_cnrmsf = rhs.m_cnrmsf;
      m_xnrmsf = rhs.m_xnrmsf;
      m_ynrmsf = rhs.m_ynrmsf;
      m_znrmsf = rhs.m_znrmsf;
      
      m_rnpwrx = rhs.m_rnpwrx;
      m_rnpwry = rhs.m_rnpwry;
      m_rnpwrz = rhs.m_rnpwrz;
      m_rntrms = rhs.m_rntrms;
      
      m_rnpcf = rhs.m_rnpcf;
      
      m_rdpwrx = rhs.m_rdpwrx;
      m_rdpwry = rhs.m_rdpwry;
      m_rdpwrz = rhs.m_rdpwrz;
      m_rdtrms = rhs.m_rdtrms;
      
      m_rdpcf = rhs.m_rdpcf;
      
      m_cnpwrx = rhs.m_cnpwrx;
      m_cnpwry = rhs.m_cnpwry;
      m_cnpwrz = rhs.m_cnpwrz;
      m_cntrms = rhs.m_cntrms;
      
      m_cnpcf = rhs.m_cnpcf;
      
      m_cdpwrx = rhs.m_cdpwrx;
      m_cdpwry = rhs.m_cdpwry;
      m_cdpwrz = rhs.m_cdpwrz;
      m_cdtrms = rhs.m_cdtrms;
      
      m_cdpcf = rhs.m_cdpcf;
   }
   return *this;
}

void ossimRsmpca::saveState(
   ossimKeywordlist& kwl, const std::string& prefix, ossim_uint32 index ) const
{
   std::string pfx = prefix + std::string("rsmpca");
   pfx += ossimString::toString( index ).string();
   pfx += std::string(".");
   
   kwl.add( pfx.c_str(), IID_KW.c_str(), m_iid.c_str() );
   kwl.add(pfx.c_str(), EDITION_KW.c_str(), m_edition.c_str());

   kwl.add(pfx.c_str(), RSN_KW.c_str(), m_rsn);
   kwl.add(pfx.c_str(), CSN_KW.c_str(), m_csn);

   kwl.add(pfx.c_str(), RFEP_KW.c_str(), m_rfep);
   kwl.add(pfx.c_str(), CFEP_KW.c_str(), m_cfep);

   kwl.add(pfx.c_str(), RNRMO_KW.c_str(), m_rnrmo);
   kwl.add(pfx.c_str(), CNRMO_KW.c_str(), m_cnrmo);
   kwl.add(pfx.c_str(), XNRMO_KW.c_str(), m_xnrmo);
   kwl.add(pfx.c_str(), YNRMO_KW.c_str(), m_ynrmo);
   kwl.add(pfx.c_str(), ZNRMO_KW.c_str(), m_znrmo);

   kwl.add(pfx.c_str(), RNRMSF_KW.c_str(), m_rnrmsf);
   kwl.add(pfx.c_str(), CNRMSF_KW.c_str(), m_cnrmsf);
   kwl.add(pfx.c_str(), XNRMSF_KW.c_str(), m_xnrmsf);
   kwl.add(pfx.c_str(), YNRMSF_KW.c_str(), m_ynrmsf);
   kwl.add(pfx.c_str(), ZNRMSF_KW.c_str(), m_znrmsf);

   kwl.add(pfx.c_str(), RNPWRX_KW.c_str(), m_rnpwrx);
   kwl.add(pfx.c_str(), RNPWRY_KW.c_str(), m_rnpwry);
   kwl.add(pfx.c_str(), RNPWRZ_KW.c_str(), m_rnpwrz);
   kwl.add(pfx.c_str(), RNTRMS_KW.c_str(), m_rntrms);

   for (ossim_uint32 i=0; i<m_rntrms; ++i)
   {
      ossimString key;
      key = RNPCF_KW.c_str();
      key += ossimString::toString(i);
      kwl.add(pfx.c_str(), key.c_str(), m_rnpcf[i]);
   }

   kwl.add(pfx.c_str(), RDPWRX_KW.c_str(), m_rdpwrx);
   kwl.add(pfx.c_str(), RDPWRY_KW.c_str(), m_rdpwry);
   kwl.add(pfx.c_str(), RDPWRZ_KW.c_str(), m_rdpwrz);
   kwl.add(pfx.c_str(), RDTRMS_KW.c_str(), m_rdtrms);

   for (ossim_uint32 i=0; i<m_rdtrms; ++i)
   {
	
      ossimString key;
      key = RDPCF_KW.c_str();
      key += ossimString::toString(i);
      kwl.add(pfx.c_str(), key.c_str(), m_rdpcf[i]);
   }

   kwl.add(pfx.c_str(), CNPWRX_KW.c_str(), m_cnpwrx);
   kwl.add(pfx.c_str(), CNPWRY_KW.c_str(), m_cnpwry);
   kwl.add(pfx.c_str(), CNPWRZ_KW.c_str(), m_cnpwrz);
   kwl.add(pfx.c_str(), CNTRMS_KW.c_str(), m_cntrms);

   for (ossim_uint32 i=0; i<m_cntrms; ++i)
   {
      ossimString key;
      key = CNPCF_KW.c_str();
      key += ossimString::toString(i);
      kwl.add(pfx.c_str(), key.c_str(), m_cnpcf[i]);
   }

   kwl.add(pfx.c_str(), CDPWRX_KW.c_str(), m_cdpwrx);
   kwl.add(pfx.c_str(), CDPWRY_KW.c_str(), m_cdpwry);
   kwl.add(pfx.c_str(), CDPWRZ_KW.c_str(), m_cdpwrz);
   kwl.add(pfx.c_str(), CDTRMS_KW.c_str(), m_cdtrms);

   for (ossim_uint32 i=0; i<m_cdtrms; ++i)
   {
      ossimString key;
      key = CDPCF_KW.c_str();
      key += ossimString::toString(i);
      kwl.add(pfx.c_str(), key.c_str(), m_cdpcf[i]);
   }

   
} // End: ossimRsmpca::saveState( ... )


bool ossimRsmpca::loadState( const ossimKeywordlist& kwl,
                             const std::string& prefix,
                             ossim_uint32 index )
{
   std::string pfx = prefix + std::string("rsmpca");
   pfx += ossimString::toString( index ).string();
   pfx += std::string(".");

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

      key = RSN_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rsn = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
     
      key = CSN_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_csn = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = RFEP_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rfep = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }
     
      key = CFEP_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cfep = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }
      
      key = RNRMO_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rnrmo = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }
      
      key = CNRMO_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cnrmo = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = XNRMO_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_xnrmo = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = YNRMO_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_ynrmo = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = ZNRMO_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_znrmo = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }
      
      key = RNRMSF_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rnrmsf = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }
      
      key = CNRMSF_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cnrmsf = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }
      
      key = XNRMSF_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_xnrmsf = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }
      
      key = YNRMSF_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_ynrmsf = ossimString(value).toFloat64();
      }
      else
      {
            break;
      }
      
      key = ZNRMSF_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_znrmsf = ossimString(value).toFloat64();
      }
      else
      {
         break;
      }

      key = RNPWRX_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rnpwrx = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = RNPWRY_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rnpwry = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      key = RNPWRZ_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rnpwrz = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = RNTRMS_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rntrms = ossimString(value).toUInt32();
         m_rnpcf.resize(m_rntrms);
         bool keysParsed = true; // Set to false if not found.
         for (ossim_uint32 i=0; i<m_rntrms; ++i)
         {
            key = RNPCF_KW;
            key += ossimString::toString(i).string();
            value = kwl.findKey(pfx, key);
            if ( !value.size() )
            {
               keysParsed = false;
               break; // Break from for loop.
            }
            m_rnpcf[i] = ossimString(value).toFloat64();
         }
         if ( keysParsed == false )
         {
            break; // Break from while (FOREVER) loop.
         }
      }

      key = RDPWRX_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rdpwrx = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = RDPWRY_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rdpwry = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = RDPWRZ_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rdpwrz = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = RDTRMS_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_rdtrms = ossimString(value).toUInt32();
         m_rdpcf.resize(m_rdtrms);
         bool keysParsed = true; // Set to false if not found.
         for (ossim_uint32 i=0; i<m_rdtrms; ++i)
         {
            key = RDPCF_KW;
            key += ossimString::toString(i).string();
            value = kwl.findKey(pfx, key);
            if ( !value.size() )
            {
               keysParsed = false;
               break; // Break from for loop.
            }
            m_rdpcf[i] = ossimString(value).toFloat64();
         }
         if ( keysParsed == false )
         {
            break; // Break from while (FOREVER) loop.
         }
      }
      
      key = CNPWRX_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cnpwrx = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }

      key = CNPWRY_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cnpwry = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = CNPWRZ_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cnpwrz = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = CNTRMS_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cntrms = ossimString(value).toUInt32();
         m_cnpcf.resize(m_cntrms);
         bool keysParsed = true; // Set to false if not found.
         for (ossim_uint32 i=0; i<m_cntrms; ++i)
         {
            key = CNPCF_KW;
            key += ossimString::toString(i).string();
            value = kwl.findKey(pfx, key);
            if ( !value.size() )
            {
               keysParsed = false;
               break; // Break from for loop.
            }
            m_cnpcf[i] = ossimString(value).toFloat64();
         }
         if ( keysParsed == false )
         {
            break; // Break from while (FOREVER) loop.
         }
      }
      else
      {
         break;
      }
      
      key = CDPWRX_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cdpwrx = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = CDPWRY_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cdpwry = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = CDPWRZ_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cdpwrz = ossimString(value).toUInt32();
      }
      else
      {
         break;
      }
      
      key = CDTRMS_KW;
      value = kwl.findKey(pfx, key);
      if ( value.size() )
      {
         m_cdtrms = ossimString(value).toUInt32();
         m_cdpcf.resize(m_cdtrms);
         bool keysParsed = true; // Set to false if not found.
         for (ossim_uint32 i=0; i<m_cdtrms; ++i)
         {
            key = CDPCF_KW;
            key += ossimString::toString(i).string();
            value = kwl.findKey(pfx, key);
            if ( !value.size() )
            {
               keysParsed = false;
               break; // Break from for loop.
            }
            m_cdpcf[i] = ossimString(value).toFloat64();
         }
         if ( keysParsed == false )
         {
            break; // Break from while (FOREVER) loop.
         }
      }
      else
      {
         break;
      }

      // If we get here we're good, so set the status for downstream code.
      result = true;
      
      // Final break from while forever loop.
      break;
         
   } // Matches while( FOREVER ){ ...

   if ( result == false )
   {  
      // Find on key failed...
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossimRsmpca::loadState WARNING:\n"
         << "Error encountered parsing the following required keyword: "
         << "<" << key << ">. Check the keywordlist for proper syntax."
         << std::endl;
   }
   
   return result;
   
} // End: ossimRsmpca::loadState( ... )

bool ossimRsmpca::initialize( const ossimNitfRsmpcaTag* rsmpcaTag )
{
   bool status = false;
   
   if ( rsmpcaTag )
   {
      m_iid = rsmpcaTag->getIid().trim().string();
      m_edition = rsmpcaTag->getEdition().trim().string();

      m_rsn = rsmpcaTag->getRsn().toUInt32();
      m_csn = rsmpcaTag->getCsn().toUInt32();

      m_rfep = rsmpcaTag->getRfep().toFloat64();
      m_cfep = rsmpcaTag->getCfep().toFloat64();

      m_rnrmo = rsmpcaTag->getRnrmo().toFloat64();
      m_cnrmo = rsmpcaTag->getCnrmo().toFloat64();
      m_xnrmo = rsmpcaTag->getXnrmo().toFloat64();
      m_ynrmo = rsmpcaTag->getYnrmo().toFloat64();
      m_znrmo = rsmpcaTag->getZnrmo().toFloat64();

      m_rnrmsf = rsmpcaTag->getRnrmsf().toFloat64();
      m_cnrmsf = rsmpcaTag->getCnrmsf().toFloat64();
      m_xnrmsf = rsmpcaTag->getXnrmsf().toFloat64();
      m_ynrmsf = rsmpcaTag->getYnrmsf().toFloat64();
      m_znrmsf = rsmpcaTag->getZnrmsf().toFloat64();
         
      m_rnpwrx = rsmpcaTag->getRnpwrx().toUInt32();
      m_rnpwry = rsmpcaTag->getRnpwry().toUInt32();
      m_rnpwrz = rsmpcaTag->getRnpwrz().toUInt32();
      m_rntrms = rsmpcaTag->getRntrms().toUInt32();
      m_rnpcf.resize(m_rntrms);

      for (ossim_uint32 i = 0; i < m_rntrms; ++i)
      {
         m_rnpcf[i] = rsmpcaTag->getRnpcf(i).toFloat64();
      }
      
      m_rdpwrx = rsmpcaTag->getRdpwrx().toUInt32();
      m_rdpwry = rsmpcaTag->getRdpwry().toUInt32();
      m_rdpwrz = rsmpcaTag->getRdpwrz().toUInt32();
      m_rdtrms = rsmpcaTag->getRdtrms().toUInt32();
      m_rdpcf.resize(m_rdtrms);

      for (ossim_uint32 i = 0; i < m_rdtrms; ++i)
      {
         m_rdpcf[i] = rsmpcaTag->getRdpcf(i).toFloat64();
      }
      
      m_cnpwrx = rsmpcaTag->getCnpwrx().toUInt32();
      m_cnpwry = rsmpcaTag->getCnpwry().toUInt32();
      m_cnpwrz = rsmpcaTag->getCnpwrz().toUInt32();
      m_cntrms = rsmpcaTag->getCntrms().toUInt32();
      m_cnpcf.resize(m_cntrms);

      for (ossim_uint32 i = 0; i < m_cntrms; ++i)
      {
         m_cnpcf[i] = rsmpcaTag->getCnpcf(i).toFloat64();
      }
      
      m_cdpwrx = rsmpcaTag->getCdpwrx().toUInt32();
      m_cdpwry = rsmpcaTag->getCdpwry().toUInt32();
      m_cdpwrz = rsmpcaTag->getCdpwrz().toUInt32();
      m_cdtrms = rsmpcaTag->getCdtrms().toUInt32();
      m_cdpcf.resize(m_cdtrms);

      for (ossim_uint32 i = 0; i < m_cdtrms; ++i)
      {
         m_cdpcf[i] = rsmpcaTag->getCdpcf(i).toFloat64();
      }
         
      status = true;
      
   } // Matches: if ( rsmpcaTag )

   return status;
   
} // End: ossimRsmpca::initializeModel( rsmpcaTag )

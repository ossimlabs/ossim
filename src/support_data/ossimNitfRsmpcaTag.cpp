//---
// File: ossimNitfRsmpcaTag.cpp
//---
#include <ossim/support_data/ossimNitfRsmpcaTag.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimStringProperty.h>
#include <iostream>
#include <iomanip>

static const ossimTrace traceDebug(ossimString("ossimNitfRsmpcaTag:debug"));
static const ossimString CNPCF_KW = "CNPCF";
static const ossimString CDPCF_KW = "CDPCF";
static const ossimString RNPCF_KW = "RNPCF";
static const ossimString RDPCF_KW = "RDPCF";
static const ossimString POLYFILL = "                     ";

RTTI_DEF1(ossimNitfRsmpcaTag, "ossimNitfRsmpcaTag", ossimNitfRegisteredTag);


ossimNitfRsmpcaTag::ossimNitfRsmpcaTag()
   :
   ossimNitfRegisteredTag(std::string("RSMPCA"), 0),
   m_iid(),
   m_edition(),
   m_rsn(),
   m_csn(),
   m_rfep(),
   m_cfep(),
   m_rnrmo(),
   m_cnrmo(),
   m_xnrmo(),
   m_ynrmo(),
   m_znrmo(),
   m_rnrmsf(),
   m_cnrmsf(),
   m_xnrmsf(),
   m_ynrmsf(),
   
   m_znrmsf(),
   
   m_rnpwrx(),
   m_rnpwry(),
   m_rnpwrz(),
   m_rntrms(),
   
   m_rnpcf(),
   
   m_rdpwrx(),
   m_rdpwry(),
   m_rdpwrz(),
   m_rdtrms(),
   
   m_rdpcf(),
   
   m_cnpwrx(),
   m_cnpwry(),
   m_cnpwrz(),
   m_cntrms(),
   
   m_cnpcf(),
   
   m_cdpwrx(),
   m_cdpwry(),
   m_cdpwrz(),
   m_cdtrms(),
   
   m_cdpcf(),
   
   m_rowNumNumTerms(0),
   m_rowDenNumTerms(0),
   m_colNumNumTerms(0),
   m_colDenNumTerms(0)
{
}

void ossimNitfRsmpcaTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(m_iid, IID_SIZE);
   in.read(m_edition, EDITION_SIZE);
   in.read(m_rsn, RSN_SIZE);
   in.read(m_csn, CSN_SIZE);
   in.read(m_rfep, FLOAT21_SIZE);
   in.read(m_cfep, FLOAT21_SIZE);
   in.read(m_rnrmo, FLOAT21_SIZE);
   in.read(m_cnrmo, FLOAT21_SIZE);
   in.read(m_xnrmo, FLOAT21_SIZE);
   in.read(m_ynrmo, FLOAT21_SIZE);
   in.read(m_znrmo, FLOAT21_SIZE);
   in.read(m_rnrmsf, FLOAT21_SIZE);
   in.read(m_cnrmsf, FLOAT21_SIZE);
   in.read(m_xnrmsf, FLOAT21_SIZE);
   in.read(m_ynrmsf, FLOAT21_SIZE);
   in.read(m_znrmsf, FLOAT21_SIZE);

   in.read(m_rnpwrx, MAXPOWER_SIZE);
   in.read(m_rnpwry, MAXPOWER_SIZE);
   in.read(m_rnpwrz, MAXPOWER_SIZE);
   in.read(m_rntrms, NUMTERMS_SIZE);
   ossim_uint32 idx = 0;
   m_rowNumNumTerms = ossimString(m_rntrms).toUInt32();
   m_rnpcf.resize(m_rowNumNumTerms);
   char temp1[FLOAT21_SIZE+1];
   temp1[FLOAT21_SIZE] = '\0';
   for(idx = 0; idx < m_rowNumNumTerms; ++idx)
   {
      in.read(temp1, FLOAT21_SIZE);
      m_rnpcf[idx] = POLYFILL; 
      m_rnpcf[idx] = temp1;
   }

   in.read(m_rdpwrx, MAXPOWER_SIZE);
   in.read(m_rdpwry, MAXPOWER_SIZE);
   in.read(m_rdpwrz, MAXPOWER_SIZE);
   in.read(m_rdtrms, NUMTERMS_SIZE);
   m_rowDenNumTerms = ossimString(m_rdtrms).toUInt32();
   m_rdpcf.resize(m_rowDenNumTerms);
   char temp2[FLOAT21_SIZE+1];
   temp2[FLOAT21_SIZE] = '\0';
   for(idx = 0; idx < m_rowDenNumTerms; ++idx)
   {
      in.read(temp2, FLOAT21_SIZE);
      m_rdpcf[idx] = POLYFILL; 
      m_rdpcf[idx] = temp2;
   }

   in.read(m_cnpwrx, MAXPOWER_SIZE);
   in.read(m_cnpwry, MAXPOWER_SIZE);
   in.read(m_cnpwrz, MAXPOWER_SIZE);
   in.read(m_cntrms, NUMTERMS_SIZE);
   m_colNumNumTerms = ossimString(m_cntrms).toUInt32();
   m_cnpcf.resize(m_colNumNumTerms); 
   char temp3[FLOAT21_SIZE+1];
   temp3[FLOAT21_SIZE] = '\0';
   for(idx = 0; idx < m_colNumNumTerms; ++idx)
   {
      in.read(temp3, FLOAT21_SIZE);
      m_cnpcf[idx] = POLYFILL; 
      m_cnpcf[idx] = temp3;
   }

   in.read(m_cdpwrx, MAXPOWER_SIZE);
   in.read(m_cdpwry, MAXPOWER_SIZE);
   in.read(m_cdpwrz, MAXPOWER_SIZE);
   in.read(m_cdtrms, NUMTERMS_SIZE);
   m_colDenNumTerms = ossimString(m_cdtrms).toUInt32();
   m_cdpcf.resize(m_colDenNumTerms); 
   char temp4[FLOAT21_SIZE+1];
   temp4[FLOAT21_SIZE] = '\0';
   for(idx = 0; idx < m_colDenNumTerms; ++idx)
   {
      in.read(temp4, FLOAT21_SIZE);
      m_cdpcf[idx] = POLYFILL; 
      m_cdpcf[idx] = temp4;
   }
}

void ossimNitfRsmpcaTag::writeStream(std::ostream& out)
{
   out.write(m_iid, IID_SIZE);
   out.write(m_edition, EDITION_SIZE);
   out.write(m_rsn, RSN_SIZE);
   out.write(m_csn, CSN_SIZE);
   out.write(m_rfep, FLOAT21_SIZE);
   out.write(m_cfep, FLOAT21_SIZE);
   out.write(m_rnrmo, FLOAT21_SIZE);
   out.write(m_cnrmo, FLOAT21_SIZE);
   out.write(m_xnrmo, FLOAT21_SIZE);
   out.write(m_ynrmo, FLOAT21_SIZE);
   out.write(m_znrmo, FLOAT21_SIZE);
   out.write(m_rnrmsf, FLOAT21_SIZE);
   out.write(m_cnrmsf, FLOAT21_SIZE);
   out.write(m_xnrmsf, FLOAT21_SIZE);
   out.write(m_ynrmsf, FLOAT21_SIZE);
   out.write(m_znrmsf, FLOAT21_SIZE);

   out.write(m_rnpwrx, MAXPOWER_SIZE);
   out.write(m_rnpwry, MAXPOWER_SIZE);
   out.write(m_rnpwrz, MAXPOWER_SIZE);
   out.write(m_rntrms, NUMTERMS_SIZE);
   ossim_uint32 idx = 0;
   for(idx = 0; idx < m_rowNumNumTerms; ++idx)
   {
      out.write(m_rnpcf[idx], FLOAT21_SIZE);
   }

   out.write(m_rdpwrx, MAXPOWER_SIZE);
   out.write(m_rdpwry, MAXPOWER_SIZE);
   out.write(m_rdpwrz, MAXPOWER_SIZE);
   out.write(m_rdtrms, NUMTERMS_SIZE);
   for(idx = 0; idx < m_rowDenNumTerms; ++idx)
   {
      out.write(m_rdpcf[idx], FLOAT21_SIZE);
   }

   out.write(m_cnpwrx, MAXPOWER_SIZE);
   out.write(m_cnpwry, MAXPOWER_SIZE);
   out.write(m_cnpwrz, MAXPOWER_SIZE);
   out.write(m_cntrms, NUMTERMS_SIZE);
   for(idx = 0; idx < m_colNumNumTerms; ++idx)
   {
      out.write(m_cnpcf[idx], FLOAT21_SIZE);
   }

   out.write(m_cdpwrx, MAXPOWER_SIZE);
   out.write(m_cdpwry, MAXPOWER_SIZE);
   out.write(m_cdpwrz, MAXPOWER_SIZE);
   out.write(m_cdtrms, NUMTERMS_SIZE);
   for(idx = 0; idx < m_colDenNumTerms; ++idx)
   {
      out.write(m_cdpcf[idx], FLOAT21_SIZE);
   }
}

void ossimNitfRsmpcaTag::clearFields()
{
   memset(m_iid, ' ', IID_SIZE);
   memset(m_edition, ' ', EDITION_SIZE);
   memset(m_rsn, ' ', RSN_SIZE);
   memset(m_csn, ' ', CSN_SIZE);
   memset(m_rfep, ' ', FLOAT21_SIZE);
   memset(m_cfep, ' ', FLOAT21_SIZE);
   memset(m_rnrmo, ' ', FLOAT21_SIZE);
   memset(m_cnrmo, ' ', FLOAT21_SIZE);
   memset(m_xnrmo, ' ', FLOAT21_SIZE);
   memset(m_ynrmo, ' ', FLOAT21_SIZE);
   memset(m_znrmo, ' ', FLOAT21_SIZE);
   memset(m_rnrmsf, ' ', FLOAT21_SIZE);
   memset(m_cnrmsf, ' ', FLOAT21_SIZE);
   memset(m_xnrmsf, ' ', FLOAT21_SIZE);
   memset(m_ynrmsf, ' ', FLOAT21_SIZE);
   memset(m_znrmsf, ' ', FLOAT21_SIZE);
   memset(m_rnpwrx, ' ', MAXPOWER_SIZE);
   memset(m_rnpwry, ' ', MAXPOWER_SIZE);
   memset(m_rnpwrz, ' ', MAXPOWER_SIZE);
   memset(m_rntrms, ' ', NUMTERMS_SIZE);
   memset(m_rdpwrx, ' ', MAXPOWER_SIZE);
   memset(m_rdpwry, ' ', MAXPOWER_SIZE);
   memset(m_rdpwrz, ' ', MAXPOWER_SIZE);
   memset(m_rdtrms, ' ', NUMTERMS_SIZE);
   memset(m_cnpwrx, ' ', MAXPOWER_SIZE);
   memset(m_cnpwry, ' ', MAXPOWER_SIZE);
   memset(m_cnpwrz, ' ', MAXPOWER_SIZE);
   memset(m_cntrms, ' ', NUMTERMS_SIZE);
   memset(m_cdpwrx, ' ', MAXPOWER_SIZE);
   memset(m_cdpwry, ' ', MAXPOWER_SIZE);
   memset(m_cdpwrz, ' ', MAXPOWER_SIZE);
   memset(m_cdtrms, ' ', NUMTERMS_SIZE);
   
   m_iid[IID_SIZE] = '\0';
   m_edition[EDITION_SIZE] = '\0';
   m_rsn[RSN_SIZE] = '\0';
   m_csn[CSN_SIZE] = '\0';
   m_rfep[FLOAT21_SIZE] = '\0';
   m_cfep[FLOAT21_SIZE] = '\0';
   m_rnrmo[FLOAT21_SIZE] = '\0';
   m_cnrmo[FLOAT21_SIZE] = '\0';
   m_xnrmo[FLOAT21_SIZE] = '\0';
   m_ynrmo[FLOAT21_SIZE] = '\0';
   m_znrmo[FLOAT21_SIZE] = '\0';
   m_rnrmsf[FLOAT21_SIZE] = '\0';
   m_cnrmsf[FLOAT21_SIZE] = '\0';
   m_xnrmsf[FLOAT21_SIZE] = '\0';
   m_ynrmsf[FLOAT21_SIZE] = '\0';
   m_znrmsf[FLOAT21_SIZE] = '\0';
   m_rnpwrx[MAXPOWER_SIZE] = '\0';
   m_rnpwry[MAXPOWER_SIZE] = '\0';
   m_rnpwrz[MAXPOWER_SIZE] = '\0';
   m_rntrms[NUMTERMS_SIZE] = '\0';
   m_rdpwrx[MAXPOWER_SIZE] = '\0';
   m_rdpwry[MAXPOWER_SIZE] = '\0';
   m_rdpwrz[MAXPOWER_SIZE] = '\0';
   m_rdtrms[NUMTERMS_SIZE] = '\0';
   m_cnpwrx[MAXPOWER_SIZE] = '\0';
   m_cnpwry[MAXPOWER_SIZE] = '\0';
   m_cnpwrz[MAXPOWER_SIZE] = '\0';
   m_cntrms[NUMTERMS_SIZE] = '\0';
   m_cdpwrx[MAXPOWER_SIZE] = '\0';
   m_cdpwry[MAXPOWER_SIZE] = '\0';
   m_cdpwrz[MAXPOWER_SIZE] = '\0';
   m_cdtrms[NUMTERMS_SIZE] = '\0';
}

std::ostream& ossimNitfRsmpcaTag::print(std::ostream& out,
                                      const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";

   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:" << getSizeInBytes() << "\n"
       << pfx << std::setw(24) << "IID:" << m_iid << "\n" 
       << pfx << std::setw(24) << "EDITION:" << m_edition << "\n"
       << pfx << std::setw(24) << "RSN:" << m_rsn << "\n" 
       << pfx << std::setw(24) << "CSN:" << m_csn << "\n"
       << pfx << std::setw(24) << "RFEP:" << m_rfep << "\n" 
       << pfx << std::setw(24) << "CFEP:" << m_cfep << "\n"
       << pfx << std::setw(24) << "RNRMO:" << m_rnrmo << "\n" 
       << pfx << std::setw(24) << "CNRMO:" << m_cnrmo << "\n"                
       << pfx << std::setw(24) << "XNRMO:" << m_xnrmo << "\n"                
       << pfx << std::setw(24) << "YNRMO:" << m_ynrmo << "\n"                
       << pfx << std::setw(24) << "ZNRMO:" << m_znrmo << "\n"                
       << pfx << std::setw(24) << "RNRMSF:" << m_rnrmsf << "\n"                
       << pfx << std::setw(24) << "CNRMSF:" << m_cnrmsf << "\n"
       << pfx << std::setw(24) << "XNRMSF:" << m_xnrmsf << "\n"
       << pfx << std::setw(24) << "YNRMSF:" << m_ynrmsf << "\n"
       << pfx << std::setw(24) << "ZNRMSF:" << m_znrmsf << "\n"
       << pfx << std::setw(24) << "RNPWRX:" << m_rnpwrx << "\n"
       << pfx << std::setw(24) << "RNPWRY:" << m_rnpwry << "\n"
       << pfx << std::setw(24) << "RNPWRZ:" << m_rnpwrz << "\n"
       << pfx << std::setw(24) << "RNTRMS:" << m_rntrms << "\n"
       << pfx << std::setw(24) << "RDPWRX:" << m_rdpwrx << "\n"
       << pfx << std::setw(24) << "RDPWRY:" << m_rdpwry << "\n"
       << pfx << std::setw(24) << "RDPWRZ:" << m_rdpwrz << "\n"
       << pfx << std::setw(24) << "RDTRMS:" << m_rdtrms << "\n"
       << pfx << std::setw(24) << "CNPWRX:" << m_rnpwrx << "\n"
       << pfx << std::setw(24) << "CNPWRY:" << m_rnpwry << "\n"
       << pfx << std::setw(24) << "CNPWRZ:" << m_rnpwrz << "\n"
       << pfx << std::setw(24) << "CNTRMS:" << m_rntrms << "\n"
       << pfx << std::setw(24) << "CDPWRX:" << m_rdpwrx << "\n"
       << pfx << std::setw(24) << "CDPWRY:" << m_rdpwry << "\n"
       << pfx << std::setw(24) << "CDPWRZ:" << m_rdpwrz << "\n"
       << pfx << std::setw(24) << "CDTRMS:" << m_rdtrms << "\n";


   ossim_uint32 i;
   ossimString s;
   
   for (i=0; i<m_rowNumNumTerms; ++i)
   {
      s = ossimString(RNPCF_KW);
      s += ossimString::toString(i);
      s += ":";
      out << pfx << std::setw(24) << s
          << m_rnpcf[i] << "\n";
   }
   for (i=0; i<m_rowDenNumTerms; ++i)
   {
      s = ossimString(RDPCF_KW);
      s += ossimString::toString(i);
      s += ":";
      out << pfx << std::setw(24) << s
          << m_rdpcf[i] << "\n";
   }
   for (i=0; i<m_colNumNumTerms; ++i)
   {
      s = ossimString(CNPCF_KW);
      s += ossimString::toString(i);
      s += ":";
      out << pfx << std::setw(24) << s
          << m_cnpcf[i] << "\n";
   }
   for (i=0; i<m_colDenNumTerms; ++i)
   {
      s = ossimString(CDPCF_KW);
      s += ossimString::toString(i);
      s += ":";
      out << pfx << std::setw(24) << s
          << m_cdpcf[i] << "\n";
   }

   out.flush();
   
   return out;
}

ossimString ossimNitfRsmpcaTag::getIid() const
{
   return ossimString(m_iid);
}

ossimString ossimNitfRsmpcaTag::getEdition() const
{
   return ossimString(m_edition);
}


ossimString ossimNitfRsmpcaTag::getRsn() const
{
   return ossimString(m_rsn);
}

ossimString ossimNitfRsmpcaTag::getCsn() const
{
   return ossimString(m_csn);
}

ossimString ossimNitfRsmpcaTag::getRfep() const
{
   return ossimString(m_rfep);
}

ossimString ossimNitfRsmpcaTag::getCfep() const
{
   return ossimString(m_cfep);
}

ossimString ossimNitfRsmpcaTag::getRnrmo() const
{
   return ossimString(m_rnrmo);
}

ossimString ossimNitfRsmpcaTag::getCnrmo() const
{
   return ossimString(m_cnrmo);
}

ossimString ossimNitfRsmpcaTag::getXnrmo() const
{
   return ossimString(m_xnrmo);
}

ossimString ossimNitfRsmpcaTag::getYnrmo() const
{
   return ossimString(m_ynrmo);
}

ossimString ossimNitfRsmpcaTag::getZnrmo() const
{
   return ossimString(m_znrmo);
}

ossimString ossimNitfRsmpcaTag::getRnrmsf() const
{
   return ossimString(m_rnrmsf);
}

ossimString ossimNitfRsmpcaTag::getCnrmsf() const
{
   return ossimString(m_cnrmsf);
}

ossimString ossimNitfRsmpcaTag::getXnrmsf() const
{
   return ossimString(m_xnrmsf);
}

ossimString ossimNitfRsmpcaTag::getYnrmsf() const
{
   return ossimString(m_ynrmsf);
}

ossimString ossimNitfRsmpcaTag::getZnrmsf() const
{
   return ossimString(m_znrmsf);
}

ossimString ossimNitfRsmpcaTag::getRnpwrx() const
{
   return ossimString(m_rnpwrx);
}

ossimString ossimNitfRsmpcaTag::getRnpwry() const
{
   return ossimString(m_rnpwry);
}

ossimString ossimNitfRsmpcaTag::getRnpwrz() const
{
   return ossimString(m_rnpwrz);
}

ossimString ossimNitfRsmpcaTag::getRntrms() const
{
   return ossimString(m_rntrms);
}

ossimString ossimNitfRsmpcaTag::getRnpcf(ossim_uint32 index) const
{
   ossimString result;
   if (index < m_rowNumNumTerms)
   {
      result = m_rnpcf[index];
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmpiaTag::getRnpcf range error!" << std::endl;
      }
   }
   return result;
}

ossimString ossimNitfRsmpcaTag::getRdpwrx() const
{
   return ossimString(m_rdpwrx);
}

ossimString ossimNitfRsmpcaTag::getRdpwry() const
{
   return ossimString(m_rdpwry);
}

ossimString ossimNitfRsmpcaTag::getRdpwrz() const
{
   return ossimString(m_rdpwrz);
}

ossimString ossimNitfRsmpcaTag::getRdtrms() const
{
   return ossimString(m_rdtrms);
}

ossimString ossimNitfRsmpcaTag::getRdpcf(ossim_uint32 index) const
{
   ossimString result;
   
   if (index < m_rowDenNumTerms)
   {
      result = m_rdpcf[index];
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmpiaTag::getRdpcf range error!" << std::endl;
      }
   }
   return result;
}

ossimString ossimNitfRsmpcaTag::getCnpwrx() const
{
   return ossimString(m_cnpwrx);
}

ossimString ossimNitfRsmpcaTag::getCnpwry() const
{
   return ossimString(m_cnpwry);
}

ossimString ossimNitfRsmpcaTag::getCnpwrz() const
{
   return ossimString(m_cnpwrz);
}

ossimString ossimNitfRsmpcaTag::getCntrms() const
{
   return ossimString(m_cntrms);
}

ossimString ossimNitfRsmpcaTag::getCnpcf(ossim_uint32 index) const
{
   ossimString result;
   
   if (index < m_colNumNumTerms)
   {
      result = m_cnpcf[index];
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmpiaTag::getCnpcf range error!" << std::endl;
      }
   }

   return result;
}

ossimString ossimNitfRsmpcaTag::getCdpwrx() const
{
   return ossimString(m_cdpwrx);
}

ossimString ossimNitfRsmpcaTag::getCdpwry() const
{
   return ossimString(m_cdpwry);
}

ossimString ossimNitfRsmpcaTag::getCdpwrz() const
{
   return ossimString(m_cdpwrz);
}

ossimString ossimNitfRsmpcaTag::getCdtrms() const
{
   return ossimString(m_cdtrms);
}

ossimString ossimNitfRsmpcaTag::getCdpcf(ossim_uint32 index) const
{
   ossimString result;
   
   if (index < m_colDenNumTerms)
   {
      result = m_cdpcf[index];
   }
   else
   {
      if ( traceDebug() )
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmpiaTag::getCdpcf range error!" << std::endl;
      }
   }
   return result;
}

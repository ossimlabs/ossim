//---
// File: ossimNitfRsmpcaTag.h
//---
#ifndef ossimNitfRsmpcaTag_HEADER
#define ossimNitfRsmpcaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <ossim/base/ossimString.h>
#include <vector>

/**
 * @class ossimNitfRsmpcaTag
 * RSM Polynomial Coefficients: Polynomials coefficients for a section.
 * RFC NTB 055, STDI-0002-1 v4.0, Appendix U: RSM
 */
class OSSIM_DLL ossimNitfRsmpcaTag : public ossimNitfRegisteredTag
{
public:
   enum 
   {
      IID_SIZE      = 80,
      EDITION_SIZE  = 40,
      RSN_SIZE      = 3,
      CSN_SIZE      = 3,
      FLOAT21_SIZE  = 21, 
      MAXPOWER_SIZE = 1,
      NUMTERMS_SIZE = 3
   };
   
   ossimNitfRsmpcaTag();
   
   virtual void parseStream(std::istream& in);
   
   virtual void writeStream(std::ostream& out);
   
   virtual void clearFields();
   
   virtual std::ostream& print( std::ostream& out,
                                const std::string& prefix=std::string() ) const;
   
   ossimString getIid() const;
   ossimString getEdition() const;
   ossimString getRsn() const;
   ossimString getCsn() const;
   ossimString getRfep() const; 
   ossimString getCfep() const;
   ossimString getRnrmo() const;
   ossimString getCnrmo() const;
   ossimString getXnrmo() const;
   ossimString getYnrmo() const;
   ossimString getZnrmo() const;
   ossimString getRnrmsf() const;
   ossimString getCnrmsf() const;
   ossimString getXnrmsf() const;
   ossimString getYnrmsf() const;
   ossimString getZnrmsf() const;
   ossimString getRnpwrx() const;
   ossimString getRnpwry() const;
   ossimString getRnpwrz() const;
   ossimString getRntrms() const;
   ossimString getRnpcf(ossim_uint32 index) const;
   ossimString getRdpwrx() const;
   ossimString getRdpwry() const;
   ossimString getRdpwrz() const;
   ossimString getRdtrms() const;
   ossimString getRdpcf(ossim_uint32 index) const;
   ossimString getCnpwrx() const;
   ossimString getCnpwry() const;
   ossimString getCnpwrz() const;
   ossimString getCntrms() const;
   ossimString getCnpcf(ossim_uint32 index) const;
   ossimString getCdpwrx() const;
   ossimString getCdpwry() const;
   ossimString getCdpwrz() const;
   ossimString getCdtrms() const;
   ossimString getCdpcf(ossim_uint32 index) const;
   
protected:
   char m_iid[IID_SIZE+1];
   char m_edition[EDITION_SIZE+1];
   char m_rsn[RSN_SIZE+1];
   char m_csn[CSN_SIZE+1];
   char m_rfep[FLOAT21_SIZE+1];
   char m_cfep[FLOAT21_SIZE+1];
   char m_rnrmo[FLOAT21_SIZE+1];
   char m_cnrmo[FLOAT21_SIZE+1];
   char m_xnrmo[FLOAT21_SIZE+1];
   char m_ynrmo[FLOAT21_SIZE+1];
   char m_znrmo[FLOAT21_SIZE+1];
   char m_rnrmsf[FLOAT21_SIZE+1];
   char m_cnrmsf[FLOAT21_SIZE+1];
   char m_xnrmsf[FLOAT21_SIZE+1];
   char m_ynrmsf[FLOAT21_SIZE+1];
   char m_znrmsf[FLOAT21_SIZE+1];
   
   char m_rnpwrx[MAXPOWER_SIZE+1];
   char m_rnpwry[MAXPOWER_SIZE+1];
   char m_rnpwrz[MAXPOWER_SIZE+1];
   char m_rntrms[NUMTERMS_SIZE+1];
   std::vector<ossimString> m_rnpcf;
   
   char m_rdpwrx[MAXPOWER_SIZE+1];
   char m_rdpwry[MAXPOWER_SIZE+1];
   char m_rdpwrz[MAXPOWER_SIZE+1];
   char m_rdtrms[NUMTERMS_SIZE+1];
   std::vector<ossimString> m_rdpcf;
   
   char m_cnpwrx[MAXPOWER_SIZE+1];
   char m_cnpwry[MAXPOWER_SIZE+1];
   char m_cnpwrz[MAXPOWER_SIZE+1];
   char m_cntrms[NUMTERMS_SIZE+1];
   std::vector<ossimString> m_cnpcf;
   
   char m_cdpwrx[MAXPOWER_SIZE+1];
   char m_cdpwry[MAXPOWER_SIZE+1];
   char m_cdpwrz[MAXPOWER_SIZE+1];
   char m_cdtrms[NUMTERMS_SIZE+1];
   std::vector<ossimString> m_cdpcf;
   
   ossim_uint32 m_rowNumNumTerms;
   ossim_uint32 m_rowDenNumTerms;
   ossim_uint32 m_colNumNumTerms;
   ossim_uint32 m_colDenNumTerms;
   
TYPE_DATA
   
}; // End: class ossimNitfRsmpcaTag

#endif /* #ifndef ossimNitfRsmpcaTag_HEADER */

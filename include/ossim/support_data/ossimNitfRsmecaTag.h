//---
// File: ossimNitfRsmecaTag.h
//---
#ifndef ossimNitfRsmecaTag_HEADER
#define ossimNitfRsmecaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <vector>

/**
 * @class ossimNitfRsmecaIsg
 * RSM Error Covariance: RSM indirect error covariance data.
 * RFC NTB 055, STDI-0002-1 v4.0, Appendix U: RSM
 */
class ossimNitfRsmecaIsg
{
public:
   enum
   {
      NUMOPG_SIZE  = 2,
      FLOAT21_SIZE = 21,
      TCDF_SIZE    = 1,
      NCSEG_SIZE   = 1
   };

   ossimNitfRsmecaIsg();

   /** @brief copy constructor */
   ossimNitfRsmecaIsg(const ossimNitfRsmecaIsg& obj);
   
   /** @brief assignment operator */
   const ossimNitfRsmecaIsg& operator=(const ossimNitfRsmecaIsg& rhs);
   
   void parseStream(std::istream& in);
   
   void writeStream(std::ostream& out);
   
   void clearFields();
   
   std::ostream& print( std::ostream& out,
                        const std::string& prefix=std::string(),
                        ossim_uint32 index=0) const;
   
protected:
   char m_numopg[NUMOPG_SIZE+1];
   std::vector<ossimString> m_errcvg;
   char m_tcdf[TCDF_SIZE+1];
   char m_ncseg[NCSEG_SIZE+1];
   std::vector<ossimString> m_corseg;
   std::vector<ossimString> m_tauseg;
   ossim_uint32 m_errCovNum;
   ossim_uint32 m_opgNum;
   ossim_uint32 m_corSegNum;
};

class OSSIM_DLL ossimNitfRsmecaTag : public ossimNitfRegisteredTag
{
public:
   enum 
   {
      IID_SIZE     = 80,
      EDITION_SIZE = 40,
      TID_SIZE     = 40,
      INCLIC_SIZE  = 1,
      INCLUC_SIZE  = 1,
      NPAR_SIZE    = 2,
      NPARO_SIZE   = 2,
      IGN_SIZE     = 2,
      CVDATE_SIZE  = 8,
      FLOAT21_SIZE = 21,
      IR_SIZE      = 2,
      IC_SIZE      = 2,
      G_SIZE       = 2,
      NUMOPG_SIZE  = 2,
      TCDF_SIZE    = 1,
      NCSEG_SIZE   = 1,
      UNCSR_SIZE   = 1,
      UNCSC_SIZE   = 1
  };

   ossimNitfRsmecaTag();
   
   virtual void parseStream(std::istream& in);
   
   virtual void writeStream(std::ostream& out);
   
   virtual void clearFields();
   
   virtual std::ostream& print( std::ostream& out,
                                const std::string& prefix=std::string() ) const;

   ossimString getIid() const;
   ossimString getEdition() const;
   ossimString getTid() const;
   ossimString getInclic() const;
   ossimString getIncluc() const;
   ossimString getNpar() const;
   ossimString getNparo() const;
   ossimString getIgn() const;
   ossimString getCvdate() const;
   ossimString getXuol() const;
   ossimString getYuol() const;
   ossimString getZuol() const;
   ossimString getXuxl() const;
   ossimString getXuyl() const;
   ossimString getXuzl() const;
   ossimString getYuxl() const;
   ossimString getYuyl() const;
   ossimString getYuzl() const;
   ossimString getZuxl() const;
   ossimString getZuyl() const;
   ossimString getZuzl() const;
   ossimString getIro() const;
   ossimString getIrx() const;
   ossimString getIry() const;
   ossimString getIrz() const;
   ossimString getIrxx() const;
   ossimString getIrxy() const;
   ossimString getIrxz() const;
   ossimString getIryy() const;
   ossimString getIryz() const;
   ossimString getIrzz() const;
   ossimString getIco() const;
   ossimString getIcx() const;
   ossimString getIcy() const;
   ossimString getIcz() const;
   ossimString getIcxx() const;
   ossimString getIcxy() const;
   ossimString getIcxz() const;
   ossimString getIcyy() const;
   ossimString getIcyz() const;
   ossimString getIczz() const;
   ossimString getGxo() const;
   ossimString getGyo() const;
   ossimString getGzo() const;
   ossimString getGxr() const;
   ossimString getGyr() const;
   ossimString getGzr() const;
   ossimString getGs() const;
   ossimString getGxx() const;
   ossimString getGxy() const;
   ossimString getGxz() const;
   ossimString getGyx() const;
   ossimString getGyy() const;
   ossimString getGyz() const;
   ossimString getGzx() const;
   ossimString getGzy() const;
   ossimString getGzz() const;
   
   ossimNitfRsmecaIsg getIsg(ossim_uint32 index) const;

   ossimString getMap(ossim_uint32 index) const;
   ossimString getUrr() const;
   ossimString getUrc() const;
   ossimString getUcc() const;
   ossimString getUncsr() const;
   ossimString getUcorsr(ossim_uint32 index) const;
   ossimString getUtausr(ossim_uint32 index) const;
   ossimString getUncsc() const;
   ossimString getUcorsc(ossim_uint32 index) const;
   ossimString getUtausc(ossim_uint32 index) const;
   bool        getInclicFlag() const;
   bool        getInclucFlag() const;
   ossimString getIgnNum() const;
   ossimString getMapNum() const;
   ossimString getRowCorSegNum() const;
   ossimString getColCorSegNum() const;
   
protected:
   char m_iid[IID_SIZE+1];
   char m_edition[EDITION_SIZE+1];
   char m_tid[TID_SIZE+1];
   char m_inclic[INCLIC_SIZE+1];
   char m_incluc[INCLUC_SIZE+1];
   char m_npar[NPAR_SIZE+1];
   char m_nparo[NPARO_SIZE+1];
   char m_ign[IGN_SIZE+1];
   char m_cvdate[CVDATE_SIZE+1];
   char m_xuol[FLOAT21_SIZE+1];
   char m_yuol[FLOAT21_SIZE+1];
   char m_zuol[FLOAT21_SIZE+1];
   char m_xuxl[FLOAT21_SIZE+1];
   char m_xuyl[FLOAT21_SIZE+1];
   char m_xuzl[FLOAT21_SIZE+1];
   char m_yuxl[FLOAT21_SIZE+1];
   char m_yuyl[FLOAT21_SIZE+1];
   char m_yuzl[FLOAT21_SIZE+1];
   char m_zuxl[FLOAT21_SIZE+1];
   char m_zuyl[FLOAT21_SIZE+1];
   char m_zuzl[FLOAT21_SIZE+1];
   char m_iro[IR_SIZE+1];
   char m_irx[IR_SIZE+1];
   char m_iry[IR_SIZE+1];
   char m_irz[IR_SIZE+1];
   char m_irxx[IR_SIZE+1];
   char m_irxy[IR_SIZE+1];
   char m_irxz[IR_SIZE+1];
   char m_iryy[IR_SIZE+1];
   char m_iryz[IR_SIZE+1];
   char m_irzz[IR_SIZE+1];
   char m_ico[IC_SIZE+1];
   char m_icx[IC_SIZE+1];
   char m_icy[IC_SIZE+1];
   char m_icz[IC_SIZE+1];
   char m_icxx[IC_SIZE+1];
   char m_icxy[IC_SIZE+1];
   char m_icxz[IC_SIZE+1];
   char m_icyy[IC_SIZE+1];
   char m_icyz[IC_SIZE+1];
   char m_iczz[IC_SIZE+1];
   char m_gxo[G_SIZE+1];
   char m_gyo[G_SIZE+1];
   char m_gzo[G_SIZE+1];
   char m_gxr[G_SIZE+1];
   char m_gyr[G_SIZE+1];
   char m_gzr[G_SIZE+1];
   char m_gs[G_SIZE+1];
   char m_gxx[G_SIZE+1];
   char m_gxy[G_SIZE+1];
   char m_gxz[G_SIZE+1];
   char m_gyx[G_SIZE+1];
   char m_gyy[G_SIZE+1];
   char m_gyz[G_SIZE+1];
   char m_gzx[G_SIZE+1];
   char m_gzy[G_SIZE+1];
   char m_gzz[G_SIZE+1];

   std::vector<ossimNitfRsmecaIsg> m_isg;
   std::vector<ossimString> m_map;

   char m_urr[FLOAT21_SIZE+1];
   char m_urc[FLOAT21_SIZE+1];
   char m_ucc[FLOAT21_SIZE+1];
   char m_uncsr[2];
   std::vector<ossimString> m_ucorsr;
   std::vector<ossimString> m_utausr;
   char m_uncsc[2];
   std::vector<ossimString> m_ucorsc;
   std::vector<ossimString> m_utausc;

   bool m_inclicFlag;
   bool m_inclucFlag;
   ossim_uint32 m_ignNum;
   ossim_uint32 m_mapNum;
   ossim_uint32 m_rowCorSegNum;
   ossim_uint32 m_colCorSegNum;

TYPE_DATA
   
}; // End: class ossimNitfRsmecaTag

#endif /* #ifndef ossimNitfRsmecaTag_HEADER */

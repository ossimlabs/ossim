//---
// File: ossimNitfRsmecaTag.cpp
//---

#include <ossim/support_data/ossimNitfRsmecaTag.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimStringProperty.h>

#include <iostream>
#include <iomanip>


static const ossimTrace traceDebug(ossimString("ossimNitfRsmecaTag:debug"));
static const ossimString RNPCF_KW = "RNPCF";
static const ossimString RDPCF_KW = "RDPCF";
//				  "0        1          2"; 
//				  "123456789012345678901";
static const ossimString FILL21 = "                     ";

ossimNitfRsmecaIsg::ossimNitfRsmecaIsg()
   :
   m_numopg(),
   m_errcvg(),
   m_tcdf(),
   m_ncseg(),
   m_corseg(),
   m_tauseg(),
   m_errCovNum(0),
   m_opgNum(0),
   m_corSegNum(0)
{
}

ossimNitfRsmecaIsg::ossimNitfRsmecaIsg(const ossimNitfRsmecaIsg& obj)
   :
   m_numopg(),
   m_errcvg(obj.m_errcvg),
   m_tcdf(),
   m_ncseg(),
   m_corseg(obj.m_corseg),
   m_tauseg(obj.m_tauseg),
   m_errCovNum(obj.m_errCovNum),
   m_opgNum(obj.m_opgNum),
   m_corSegNum(obj.m_corSegNum)
{
   strncpy( m_numopg, obj.m_numopg, (NUMOPG_SIZE+1) );
   strncpy( m_tcdf,   obj.m_tcdf,   (TCDF_SIZE+1)   );
   strncpy( m_ncseg,  obj.m_ncseg,  (NCSEG_SIZE+1)  );
}

const ossimNitfRsmecaIsg& ossimNitfRsmecaIsg::operator=(const ossimNitfRsmecaIsg& rhs)
{
   if (this != &rhs)
   {
      strncpy( m_numopg, rhs.m_numopg, (NUMOPG_SIZE+1) );
      m_errcvg    = rhs.m_errcvg;
      strncpy( m_tcdf,   rhs.m_tcdf,   (TCDF_SIZE+1)   );
      strncpy( m_ncseg,  rhs.m_ncseg,  (NCSEG_SIZE+1)  );
      m_corseg    = rhs.m_corseg;
      m_tauseg    = rhs.m_tauseg;
      m_errCovNum = rhs.m_errCovNum;
      m_opgNum    = rhs.m_opgNum;
      m_corSegNum = rhs.m_corSegNum;
   }
   return *this;
}

void ossimNitfRsmecaIsg::parseStream(std::istream& in)
{
   clearFields();
   in.read(m_numopg, NUMOPG_SIZE);
   m_opgNum = ossimString(m_numopg).toUInt32();
   m_errCovNum = (m_opgNum * (m_opgNum + 1)) / 2;
   ossim_uint32 i = 0;
   char temp1[FLOAT21_SIZE+1];
   temp1[FLOAT21_SIZE] = '\0';
   m_errcvg.resize(m_errCovNum);
   for (i=0; i < m_errCovNum; ++i)
   {
      in.read(temp1, FLOAT21_SIZE);
      m_errcvg[i] = FILL21;
      m_errcvg[i] = temp1;
   }
   in.read(m_tcdf, TCDF_SIZE);
   in.read(m_ncseg, NCSEG_SIZE);
   m_corSegNum = ossimString(m_ncseg).toUInt32();
   m_corseg.resize(m_corSegNum);
   m_tauseg.resize(m_corSegNum);

   for (i=0; i < m_corSegNum; ++i)
   {
      in.read(temp1, FLOAT21_SIZE);
      m_corseg[i] = FILL21;
      m_corseg[i] = temp1;
      in.read(temp1, FLOAT21_SIZE);
      m_tauseg[i] = FILL21;
      m_tauseg[i] = temp1;
   }
}

void ossimNitfRsmecaIsg::writeStream(std::ostream& out)
{
   out.write(m_numopg, NUMOPG_SIZE);
   ossim_uint32 i = 0;
   for (i=0; i < m_errCovNum; ++i)
   {
      out.write(m_errcvg[i], FLOAT21_SIZE);
   }
   out.write(m_tcdf, TCDF_SIZE);
   out.write(m_ncseg, NCSEG_SIZE);
   
   for (i=0; i < m_corSegNum; ++i)
   {
      out.write(m_corseg[i], FLOAT21_SIZE);
      out.write(m_tauseg[i], FLOAT21_SIZE);
   }
}

void ossimNitfRsmecaIsg::clearFields()
{
   memset(m_numopg, ' ', NUMOPG_SIZE);
   memset(m_tcdf, ' ', TCDF_SIZE);
   memset(m_ncseg, ' ', NCSEG_SIZE);
   m_numopg[NUMOPG_SIZE] = '\0';
   m_tcdf[TCDF_SIZE] = '\0';
   m_ncseg[NCSEG_SIZE] = '\0';
}

std::ostream& ossimNitfRsmecaIsg::print(
   std::ostream& out, const std::string& prefix, ossim_uint32 index) const
{
   std::string pfx = prefix;
   pfx += "RSMECA";
   pfx += ".ISG"; 
   pfx += ossimString::toString(index).c_str();
   pfx += ".";
   ossim_uint32 i = 0;

   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "NUMOPG:" << m_numopg << "\n";
   for(i=0;i<m_errCovNum;++i)
   {
      ossimString field  = "ERRCVG" + ossimString::toString(i) + ":";
      out << pfx << std::setw(24) <<  field << m_errcvg[i] << "\n";
   }

   out << pfx << std::setw(24) << "TCDF:" << m_tcdf << "\n"
       << pfx << std::setw(24) << "NCSEG:" << m_ncseg << "\n";

   for(i=0;i<m_corSegNum;++i)
   {
      ossimString field = "CORSEG" + ossimString::toString(i) + ":";
      out << pfx << std::setw(24) << field << m_corseg[i] << "\n";
   }
   for(i=0;i<m_corSegNum;++i)
   {
      ossimString field = "TAUSEG" + ossimString::toString(i) + ":";
      out << pfx << std::setw(24) << field << m_tauseg[i] << "\n";
   }

   return out;
}

RTTI_DEF1(ossimNitfRsmecaTag, "ossimNitfRsmecaTag", ossimNitfRegisteredTag);


ossimNitfRsmecaTag::ossimNitfRsmecaTag()
   :
   ossimNitfRegisteredTag(std::string("RSMECA"), 0),
   m_iid(),
   m_edition(),
   m_tid(),
   m_inclic(),
   m_incluc(),
   m_npar(),
   m_nparo(),
   m_ign(),
   m_cvdate(),
   m_xuol(),
   m_yuol(),
   m_zuol(),
   m_xuxl(),
   m_xuyl(),
   m_xuzl(),
   m_yuxl(),
   m_yuyl(),
   m_yuzl(),
   m_zuxl(),
   m_zuyl(),
   m_zuzl(),
   m_iro(),
   m_irx(),
   m_iry(),
   m_irz(),
   m_irxx(),
   m_irxy(),
   m_irxz(),
   m_iryy(),
   m_iryz(),
   m_irzz(),
   m_ico(),
   m_icx(),
   m_icy(),
   m_icz(),
   m_icxx(),
   m_icxy(),
   m_icxz(),
   m_icyy(),
   m_icyz(),
   m_iczz(),
   m_gxo(),
   m_gyo(),
   m_gzo(),
   m_gxr(),
   m_gyr(),
   m_gzr(),
   m_gs(),
   m_gxx(),
   m_gxy(),
   m_gxz(),
   m_gyx(),
   m_gyy(),
   m_gyz(),
   m_gzx(),
   m_gzy(),
   m_gzz(),

   m_isg(),
   m_map(),
   
   m_urr(),
   m_urc(),
   m_ucc(),
   m_uncsr(),
   m_ucorsr(),
   m_utausr(),
   m_uncsc(),
   m_ucorsc(),
   m_utausc(),

   m_inclicFlag(false),
   m_inclucFlag(false),
   m_ignNum(0),
   m_mapNum(0),
   m_rowCorSegNum(0),
   m_colCorSegNum(0)
{
}

void ossimNitfRsmecaTag::parseStream(std::istream& in)
{
   memset(m_iid,' ', IID_SIZE);
   memset(m_edition,' ', EDITION_SIZE);
   memset(m_tid,' ', TID_SIZE);
   memset(m_inclic,' ', INCLIC_SIZE);
   memset(m_incluc,' ', INCLUC_SIZE);
   m_iid[IID_SIZE] = '\0';
   m_edition[EDITION_SIZE] = '\0';
   m_tid[TID_SIZE] = '\0';
   m_inclic[INCLIC_SIZE] = '\0';
   m_incluc[INCLUC_SIZE] = '\0';
   in.read(m_iid, IID_SIZE);
   in.read(m_edition, EDITION_SIZE);
   in.read(m_tid, TID_SIZE);
   in.read(m_inclic, INCLIC_SIZE);
   m_inclicFlag = (ossimString(m_inclic) == "Y") ? true : false;
   in.read(m_incluc, INCLUC_SIZE);
   m_inclucFlag = (ossimString(m_incluc) == "Y") ? true : false;
   ossim_uint32 i = 0;

   if (m_inclicFlag)
   {
     memset(m_npar,' ', NPAR_SIZE);
     memset(m_nparo,' ', NPARO_SIZE);
     memset(m_ign,' ', IGN_SIZE);
     memset(m_cvdate,' ', CVDATE_SIZE);
     memset(m_xuol,' ', FLOAT21_SIZE);
     memset(m_yuol,' ', FLOAT21_SIZE);
     memset(m_zuol,' ', FLOAT21_SIZE);
     memset(m_xuxl,' ', FLOAT21_SIZE);
     memset(m_xuyl,' ', FLOAT21_SIZE);
     memset(m_xuzl,' ', FLOAT21_SIZE);
     memset(m_yuxl,' ', FLOAT21_SIZE);
     memset(m_yuyl,' ', FLOAT21_SIZE);
     memset(m_yuzl,' ', FLOAT21_SIZE);
     memset(m_zuxl,' ', FLOAT21_SIZE);
     memset(m_zuyl,' ', FLOAT21_SIZE);
     memset(m_zuzl,' ', FLOAT21_SIZE);
     memset(m_iro,' ', IR_SIZE);
     memset(m_irx,' ', IR_SIZE);
     memset(m_iry,' ', IR_SIZE);
     memset(m_irz,' ', IR_SIZE);
     memset(m_irxx,' ', IR_SIZE);
     memset(m_irxy,' ', IR_SIZE);
     memset(m_irxz,' ', IR_SIZE);
     memset(m_iryy,' ', IR_SIZE);
     memset(m_iryz,' ', IR_SIZE);
     memset(m_irzz,' ', IR_SIZE);
     memset(m_ico,' ', IC_SIZE);
     memset(m_icx,' ', IC_SIZE);
     memset(m_icy,' ', IC_SIZE);
     memset(m_icz,' ', IC_SIZE);
     memset(m_icxx,' ', IC_SIZE);
     memset(m_icxy,' ', IC_SIZE);
     memset(m_icxz,' ', IC_SIZE);
     memset(m_icyy,' ', IC_SIZE);
     memset(m_icyz,' ', IC_SIZE);
     memset(m_iczz,' ', IC_SIZE);
     memset(m_gxo,' ', G_SIZE);
     memset(m_gyo,' ', G_SIZE);
     memset(m_gzo,' ', G_SIZE);
     memset(m_gxr,' ', G_SIZE);
     memset(m_gyr,' ', G_SIZE);
     memset(m_gzr,' ', G_SIZE);
     memset(m_gs,' ', G_SIZE);
     memset(m_gxx,' ', G_SIZE);
     memset(m_gxy,' ', G_SIZE);
     memset(m_gxz,' ', G_SIZE);
     memset(m_gyx,' ', G_SIZE);
     memset(m_gyy,' ', G_SIZE);
     memset(m_gyz,' ', G_SIZE);
     memset(m_gzx,' ', G_SIZE);
     memset(m_gzy,' ', G_SIZE);
     memset(m_gzz,' ', G_SIZE);
     m_npar[NPAR_SIZE] = '\0';
     m_nparo[NPARO_SIZE] = '\0';
     m_ign[IGN_SIZE] = '\0';
     m_cvdate[CVDATE_SIZE] = '\0';
     m_xuol[FLOAT21_SIZE] = '\0';
     m_yuol[FLOAT21_SIZE] = '\0';
     m_zuol[FLOAT21_SIZE] = '\0';
     m_xuxl[FLOAT21_SIZE] = '\0';
     m_xuyl[FLOAT21_SIZE] = '\0';
     m_xuzl[FLOAT21_SIZE] = '\0';
     m_yuxl[FLOAT21_SIZE] = '\0';
     m_yuyl[FLOAT21_SIZE] = '\0';
     m_yuzl[FLOAT21_SIZE] = '\0';
     m_zuxl[FLOAT21_SIZE] = '\0';
     m_zuyl[FLOAT21_SIZE] = '\0';
     m_zuzl[FLOAT21_SIZE] = '\0';
     m_iro[IR_SIZE] = '\0';
     m_irx[IR_SIZE] = '\0';
     m_iry[IR_SIZE] = '\0';
     m_irz[IR_SIZE] = '\0';
     m_irxx[IR_SIZE] = '\0';
     m_irxy[IR_SIZE] = '\0';
     m_irxz[IR_SIZE] = '\0';
     m_iryy[IR_SIZE] = '\0';
     m_iryz[IR_SIZE] = '\0';
     m_irzz[IR_SIZE] = '\0';
     m_ico[IR_SIZE] = '\0';
     m_icx[IR_SIZE] = '\0';
     m_icy[IR_SIZE] = '\0';
     m_icz[IR_SIZE] = '\0';
     m_icxx[IR_SIZE] = '\0';
     m_icxy[IR_SIZE] = '\0';
     m_icxz[IR_SIZE] = '\0';
     m_icyy[IR_SIZE] = '\0';
     m_icyz[IR_SIZE] = '\0';
     m_iczz[IR_SIZE] = '\0';
     m_gxo[G_SIZE] = '\0';
     m_gyo[G_SIZE] = '\0';
     m_gzo[G_SIZE] = '\0';
     m_gxr[G_SIZE] = '\0';
     m_gyr[G_SIZE] = '\0';
     m_gzr[G_SIZE] = '\0';
     m_gs[G_SIZE] = '\0';
     m_gxx[G_SIZE] = '\0';
     m_gxy[G_SIZE] = '\0';
     m_gxz[G_SIZE] = '\0';
     m_gyx[G_SIZE] = '\0';
     m_gyy[G_SIZE] = '\0';
     m_gyz[G_SIZE] = '\0';
     m_gzx[G_SIZE] = '\0';
     m_gzy[G_SIZE] = '\0';
     m_gzz[G_SIZE] = '\0';

     in.read(m_npar, NPAR_SIZE);
     in.read(m_nparo, NPARO_SIZE);
     m_mapNum = ossimString(m_npar).toUInt32() * ossimString(m_nparo).toUInt32();
     in.read(m_ign, IGN_SIZE);
     m_ignNum = ossimString(m_ign).toUInt32();
     in.read(m_cvdate, CVDATE_SIZE);
     in.read(m_xuol, FLOAT21_SIZE);
     in.read(m_yuol, FLOAT21_SIZE);
     in.read(m_zuol, FLOAT21_SIZE);
     in.read(m_xuxl, FLOAT21_SIZE);
     in.read(m_xuyl, FLOAT21_SIZE);
     in.read(m_xuzl, FLOAT21_SIZE);
     in.read(m_yuxl, FLOAT21_SIZE);
     in.read(m_yuyl, FLOAT21_SIZE);
     in.read(m_yuzl, FLOAT21_SIZE);
     in.read(m_zuxl, FLOAT21_SIZE);
     in.read(m_zuyl, FLOAT21_SIZE);
     in.read(m_zuzl, FLOAT21_SIZE);
     in.read(m_iro, IR_SIZE);
     in.read(m_irx, IR_SIZE);
     in.read(m_iry, IR_SIZE);
     in.read(m_irz, IR_SIZE);
     in.read(m_irxx, IR_SIZE);
     in.read(m_irxy, IR_SIZE);
     in.read(m_irxz, IR_SIZE);
     in.read(m_iryy, IR_SIZE);
     in.read(m_iryz, IR_SIZE);
     in.read(m_irzz, IR_SIZE);
     in.read(m_ico, IC_SIZE);
     in.read(m_icx, IC_SIZE);
     in.read(m_icy, IC_SIZE);
     in.read(m_icz, IC_SIZE);
     in.read(m_icxx, IC_SIZE);
     in.read(m_icxy, IC_SIZE);
     in.read(m_icxz, IC_SIZE);
     in.read(m_icyy, IC_SIZE);
     in.read(m_icyz, IC_SIZE);
     in.read(m_iczz, IC_SIZE);
     in.read(m_gxo, G_SIZE);
     in.read(m_gyo, G_SIZE);
     in.read(m_gzo, G_SIZE);
     in.read(m_gxr, G_SIZE);
     in.read(m_gyr, G_SIZE);
     in.read(m_gzr, G_SIZE);
     in.read(m_gs, G_SIZE);
     in.read(m_gxx, G_SIZE);
     in.read(m_gxy, G_SIZE);
     in.read(m_gxz, G_SIZE);
     in.read(m_gyx, G_SIZE);
     in.read(m_gyy, G_SIZE);
     in.read(m_gyz, G_SIZE);
     in.read(m_gzx, G_SIZE);
     in.read(m_gzy, G_SIZE);
     in.read(m_gzz, G_SIZE);
 
     m_isg.resize(m_ignNum);
     m_map.resize(m_mapNum);

     for (i=0; i < m_ignNum; ++i)
     {
        ossimNitfRsmecaIsg isg;
        m_isg[i] = isg;
        m_isg[i].parseStream(in);
     }
     char temp1[FLOAT21_SIZE+1];
     temp1[FLOAT21_SIZE] = '\0';
     for (i=0; i < m_mapNum; ++i)
     {
        in.read(temp1, FLOAT21_SIZE);
        m_map[i] = FILL21;
        m_map[i] = temp1;
     }
   }
   if (m_inclucFlag)
   {
     memset(m_urr,' ', FLOAT21_SIZE);
     memset(m_urc,' ', FLOAT21_SIZE);
     memset(m_ucc,' ', FLOAT21_SIZE);
     memset(m_uncsr, ' ', 1);
     m_urr[FLOAT21_SIZE] = '\0';
     m_urc[FLOAT21_SIZE] = '\0';
     m_ucc[FLOAT21_SIZE] = '\0';
     m_uncsr[1] = '\0';
     in.read(m_urr, FLOAT21_SIZE);
     in.read(m_urc, FLOAT21_SIZE);
     in.read(m_ucc, FLOAT21_SIZE);
     in.read(m_uncsr, 1);
     m_rowCorSegNum = ossimString(m_uncsr).toUInt32();
     m_ucorsr.resize(m_rowCorSegNum);
     m_utausr.resize(m_rowCorSegNum);
     char temp1[FLOAT21_SIZE+1];
     temp1[FLOAT21_SIZE] = '\0';
     for (i=0; i < m_rowCorSegNum; ++i)
     {
	in.read(temp1, FLOAT21_SIZE);
        m_ucorsr[i] = FILL21;
        m_ucorsr[i] = temp1;
	in.read(temp1, FLOAT21_SIZE);
	m_utausr[i] = FILL21;
	m_utausr[i] = temp1;
     }
     memset(m_uncsc, ' ', 1);
     m_uncsc[1] = '\0';
     in.read(m_uncsc, 1);
     m_colCorSegNum = ossimString(m_uncsc).toUInt32();
     m_ucorsc.resize(m_colCorSegNum);
     m_utausc.resize(m_colCorSegNum);
     for (i=0; i < m_colCorSegNum; ++i)
     {
        in.read(temp1, FLOAT21_SIZE);
        m_ucorsc[i] = FILL21;
        m_ucorsc[i] = temp1;
	in.read(temp1, FLOAT21_SIZE);
        m_utausc[i] = FILL21;
        m_utausc[i] = temp1;
     }
   }

}

void ossimNitfRsmecaTag::writeStream(std::ostream& out)
{
   out.write(m_iid, IID_SIZE);
   out.write(m_edition, EDITION_SIZE);
   out.write(m_tid, TID_SIZE);
   out.write(m_inclic, INCLIC_SIZE);
   out.write(m_incluc, INCLUC_SIZE);
   ossim_uint32 i = 0;

   if (m_inclicFlag)
   {
     out.write(m_npar, NPAR_SIZE);
     out.write(m_nparo, NPARO_SIZE);
     out.write(m_ign, IGN_SIZE);
     out.write(m_cvdate, CVDATE_SIZE);
     out.write(m_xuol, FLOAT21_SIZE);
     out.write(m_yuol, FLOAT21_SIZE);
     out.write(m_zuol, FLOAT21_SIZE);
     out.write(m_xuxl, FLOAT21_SIZE);
     out.write(m_xuyl, FLOAT21_SIZE);
     out.write(m_xuzl, FLOAT21_SIZE);
     out.write(m_yuxl, FLOAT21_SIZE);
     out.write(m_yuyl, FLOAT21_SIZE);
     out.write(m_yuzl, FLOAT21_SIZE);
     out.write(m_zuxl, FLOAT21_SIZE);
     out.write(m_zuyl, FLOAT21_SIZE);
     out.write(m_zuzl, FLOAT21_SIZE);
     out.write(m_iro, IR_SIZE);
     out.write(m_irx, IR_SIZE);
     out.write(m_iry, IR_SIZE);
     out.write(m_irz, IR_SIZE);
     out.write(m_irxx, IR_SIZE);
     out.write(m_irxy, IR_SIZE);
     out.write(m_irxz, IR_SIZE);
     out.write(m_iryy, IR_SIZE);
     out.write(m_iryz, IR_SIZE);
     out.write(m_irzz, IR_SIZE);
     out.write(m_ico, IC_SIZE);
     out.write(m_icx, IC_SIZE);
     out.write(m_icy, IC_SIZE);
     out.write(m_icz, IC_SIZE);
     out.write(m_icxx, IC_SIZE);
     out.write(m_icxy, IC_SIZE);
     out.write(m_icxz, IC_SIZE);
     out.write(m_icyy, IC_SIZE);
     out.write(m_icyz, IC_SIZE);
     out.write(m_iczz, IC_SIZE);
     out.write(m_gxo, G_SIZE);
     out.write(m_gyo, G_SIZE);
     out.write(m_gzo, G_SIZE);
     out.write(m_gxr, G_SIZE);
     out.write(m_gyr, G_SIZE);
     out.write(m_gzr, G_SIZE);
     out.write(m_gs, G_SIZE);
     out.write(m_gxx, G_SIZE);
     out.write(m_gxy, G_SIZE);
     out.write(m_gxz, G_SIZE);
     out.write(m_gyx, G_SIZE);
     out.write(m_gyy, G_SIZE);
     out.write(m_gyz, G_SIZE);
     out.write(m_gzx, G_SIZE);
     out.write(m_gzy, G_SIZE);
     out.write(m_gzz, G_SIZE);

     for (i=0; i < m_ignNum; ++i)
     {
        m_isg[i].writeStream(out);
     }
     for (i=0; i < m_mapNum; ++i)
     {
        out.write(m_map[i], FLOAT21_SIZE);
     }
   }
   if (m_inclucFlag)
   {
      out.write(m_urr, FLOAT21_SIZE);
      out.write(m_urc, FLOAT21_SIZE);
      out.write(m_ucc, FLOAT21_SIZE);
      out.write(m_uncsr, 1);
      for (i=0; i < m_rowCorSegNum; ++i)
      {
         out.write(m_ucorsr[i], FLOAT21_SIZE);
         out.write(m_utausr[i], FLOAT21_SIZE);
      }
      out.write(m_uncsc, 1);
      for (i=0; i < m_colCorSegNum; ++i)
      {
         out.write(m_ucorsc[i], FLOAT21_SIZE);
         out.write(m_utausc[i], FLOAT21_SIZE);
      }
   }
}

void ossimNitfRsmecaTag::clearFields()
{
}

std::ostream& ossimNitfRsmecaTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   ossim_uint32 i = 0;

   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:" << getSizeInBytes() << "\n"
       << pfx << std::setw(24) << "IID:" << m_iid << "\n" 
       << pfx << std::setw(24) << "EDITION:" << m_edition << "\n"
       << pfx << std::setw(24) << "TID:" << m_tid << "\n"
       << pfx << std::setw(24) << "INCLIC:" << m_inclic << "\n"
       << pfx << std::setw(24) << "INCLUC:" << m_incluc << "\n";

   if(m_inclicFlag)
   {
      out << pfx << std::setw(24) << "NPAR:" << m_npar << "\n"
          << pfx << std::setw(24) << "NPARO:" << m_nparo << "\n"
          << pfx << std::setw(24) << "IGN:" << m_ign << "\n"
          << pfx << std::setw(24) << "CVDATE:" << m_cvdate << "\n"
          << pfx << std::setw(24) << "XUOL:" << m_xuol << "\n"
          << pfx << std::setw(24) << "YUOL:" << m_yuol << "\n"
          << pfx << std::setw(24) << "ZUOL:" << m_zuol << "\n"
          << pfx << std::setw(24) << "XUXL:" << m_xuxl << "\n"
          << pfx << std::setw(24) << "XUYL:" << m_xuyl << "\n"
          << pfx << std::setw(24) << "XUZL:" << m_xuzl << "\n"
          << pfx << std::setw(24) << "YUXL:" << m_yuxl << "\n"
          << pfx << std::setw(24) << "YUYL:" << m_yuyl << "\n"
          << pfx << std::setw(24) << "YUZL:" << m_yuzl << "\n"
          << pfx << std::setw(24) << "ZUXL:" << m_zuxl << "\n"
          << pfx << std::setw(24) << "ZUYL:" << m_zuyl << "\n"
          << pfx << std::setw(24) << "ZUZL:" << m_zuzl << "\n"
          << pfx << std::setw(24) << "IRO:" << m_iro << "\n"
          << pfx << std::setw(24) << "IRX:" << m_irx << "\n"
          << pfx << std::setw(24) << "IRY:" << m_iry << "\n"
          << pfx << std::setw(24) << "IRZ:" << m_irz << "\n"
          << pfx << std::setw(24) << "IRXX:" << m_irxx << "\n"
          << pfx << std::setw(24) << "IRXY:" << m_irxy << "\n"
          << pfx << std::setw(24) << "IRXZ:" << m_irxz << "\n"
          << pfx << std::setw(24) << "IRYY:" << m_iryy << "\n"
          << pfx << std::setw(24) << "IRYZ:" << m_iryz << "\n"
          << pfx << std::setw(24) << "IRZZ:" << m_irzz << "\n"
          << pfx << std::setw(24) << "ICO:" << m_ico << "\n"
          << pfx << std::setw(24) << "ICX:" << m_icx << "\n"
          << pfx << std::setw(24) << "ICY:" << m_icy << "\n"
          << pfx << std::setw(24) << "ICZ:" << m_icz << "\n"
          << pfx << std::setw(24) << "ICXX:" << m_icxx << "\n"
          << pfx << std::setw(24) << "ICXY:" << m_icxy << "\n"
          << pfx << std::setw(24) << "ICXZ:" << m_icxz << "\n"
          << pfx << std::setw(24) << "ICYY:" << m_icyy << "\n"
          << pfx << std::setw(24) << "ICYZ:" << m_icyz << "\n"
          << pfx << std::setw(24) << "ICZZ:" << m_iczz << "\n"
          << pfx << std::setw(24) << "GXO:" << m_gxo << "\n"
          << pfx << std::setw(24) << "GYO:" << m_gyo << "\n"
          << pfx << std::setw(24) << "GZO:" << m_gzo << "\n"
          << pfx << std::setw(24) << "GXR:" << m_gxr << "\n"
          << pfx << std::setw(24) << "GYR:" << m_gyr << "\n"
          << pfx << std::setw(24) << "GZR:" << m_gzr << "\n"
          << pfx << std::setw(24) << "GS:" << m_gs << "\n"
          << pfx << std::setw(24) << "GXX:" << m_gxx << "\n"
          << pfx << std::setw(24) << "GXY:" << m_gxy << "\n"
          << pfx << std::setw(24) << "GXZ:" << m_gxz << "\n"
          << pfx << std::setw(24) << "GYX:" << m_gyx << "\n"
          << pfx << std::setw(24) << "GYY:" << m_gyy << "\n"
          << pfx << std::setw(24) << "GYZ:" << m_gyz << "\n"
          << pfx << std::setw(24) << "GZX:" << m_gzx << "\n"
          << pfx << std::setw(24) << "GZY:" << m_gzy << "\n"
          << pfx << std::setw(24) << "GZZ:" << m_gzz << "\n";
	
      for (i=0; i<m_ignNum; ++i)
      {
         m_isg[i].print(out, prefix, i);
	
      }
      for (i=0; i<m_mapNum; ++i)
      {
         ossimString field = "MAP" + ossimString::toString(i) + ":";	
         out << pfx << std::setw(24) << field << m_map[i] << "\n"; 
      }
   }
   if (m_inclucFlag)
   {
      out << pfx << std::setw(24) << "URR:" << m_urr << "\n"
          << pfx << std::setw(24) << "URC:" << m_urc << "\n"
          << pfx << std::setw(24) << "UCC:" << m_ucc << "\n"
          << pfx << std::setw(24) << "UNCSR:" << m_uncsr << "\n";
     
      for (i=0; i<m_rowCorSegNum; ++i)
      {
	 ossimString field = "UCORSR" + ossimString::toString(i) + ":";
	 out << pfx << std::setw(24) << field << m_ucorsr[i] << "\n";
      }
      for (i=0; i<m_rowCorSegNum; ++i)
      {
         ossimString field = "UTAUSR" + ossimString::toString(i) + ":";
         out << pfx << std::setw(24) << field << m_utausr[i] << "\n";
      }
      
      for (i=0; i<m_colCorSegNum; ++i)
      {
         ossimString field = "UCORSC" + ossimString::toString(i) + ":";
         out << pfx << std::setw(24) << field << m_ucorsc[i] << "\n";
      }
      for (i=0; i<m_colCorSegNum; ++i)
      {
         ossimString field = "UTAUSC" + ossimString::toString(i) + ":";
         out << pfx << std::setw(24) << field << m_utausc[i] << "\n";
      }
   }
   
   out.flush();
   
   return out;
}

ossimString ossimNitfRsmecaTag::getIid() const
{
   return ossimString(m_iid);
}

ossimString ossimNitfRsmecaTag::getEdition() const
{
   return ossimString(m_edition);
}

ossimString ossimNitfRsmecaTag::getTid() const
{
   return ossimString(m_tid);
}

ossimString ossimNitfRsmecaTag::getInclic() const
{
   return ossimString(m_inclic);
}

ossimString ossimNitfRsmecaTag::getIncluc() const
{
   return ossimString(m_incluc);
}

ossimString ossimNitfRsmecaTag::getNpar() const
{
   return ossimString(m_npar);
}

ossimString ossimNitfRsmecaTag::getNparo() const
{
   return ossimString(m_nparo);
}

ossimString ossimNitfRsmecaTag::getIgn() const
{
   return ossimString(m_ign);
}

ossimString ossimNitfRsmecaTag::getCvdate() const
{
   return ossimString(m_cvdate);
}

ossimString ossimNitfRsmecaTag::getXuol() const
{
   return ossimString(m_xuol);
}

ossimString ossimNitfRsmecaTag::getYuol() const
{
   return ossimString(m_yuol);
}

ossimString ossimNitfRsmecaTag::getZuol() const
{
   return ossimString(m_zuol);
}

ossimString ossimNitfRsmecaTag::getXuxl() const
{
   return ossimString(m_xuxl);
}

ossimString ossimNitfRsmecaTag::getXuyl() const
{
   return ossimString(m_xuyl);
}

ossimString ossimNitfRsmecaTag::getXuzl() const
{
   return ossimString(m_xuzl);
}

ossimString ossimNitfRsmecaTag::getYuxl() const
{
   return ossimString(m_yuxl);
}

ossimString ossimNitfRsmecaTag::getYuyl() const
{
   return ossimString(m_yuyl);
}

ossimString ossimNitfRsmecaTag::getYuzl() const
{
   return ossimString(m_yuzl);
}

ossimString ossimNitfRsmecaTag::getZuxl() const
{
   return ossimString(m_zuxl);
}

ossimString ossimNitfRsmecaTag::getZuyl() const
{
   return ossimString(m_zuyl);
}

ossimString ossimNitfRsmecaTag::getZuzl() const
{
   return ossimString(m_zuzl);
}

ossimString ossimNitfRsmecaTag::getIro() const
{
   return ossimString(m_iro);
}

ossimString ossimNitfRsmecaTag::getIrx() const
{
   return ossimString(m_irx);
}
ossimString ossimNitfRsmecaTag::getIry() const
{
   return ossimString(m_iry);
}

ossimString ossimNitfRsmecaTag::getIrz() const
{
   return ossimString(m_irz);
}

ossimString ossimNitfRsmecaTag::getIrxx() const
{
   return ossimString(m_irxx);
}

ossimString ossimNitfRsmecaTag::getIrxy() const
{
   return ossimString(m_irxy);
}

ossimString ossimNitfRsmecaTag::getIrxz() const
{
   return ossimString(m_irxz);
}

ossimString ossimNitfRsmecaTag::getIryy() const
{
   return ossimString(m_iryy);
}

ossimString ossimNitfRsmecaTag::getIryz() const
{
   return ossimString(m_iryz);
}

ossimString ossimNitfRsmecaTag::getIrzz() const
{
   return ossimString(m_irzz);
}

ossimString ossimNitfRsmecaTag::getIco() const
{
   return ossimString(m_ico);
}

ossimString ossimNitfRsmecaTag::getIcx() const
{
   return ossimString(m_icx);
}

ossimString ossimNitfRsmecaTag::getIcy() const
{
   return ossimString(m_icy);
}

ossimString ossimNitfRsmecaTag::getIcz() const
{
   return ossimString(m_icz);
}

ossimString ossimNitfRsmecaTag::getIcxx() const
{
   return ossimString(m_icxx);
}

ossimString ossimNitfRsmecaTag::getIcxy() const
{
   return ossimString(m_icxy);
}

ossimString ossimNitfRsmecaTag::getIcxz() const
{
   return ossimString(m_icxz);
}

ossimString ossimNitfRsmecaTag::getIcyy() const
{
   return ossimString(m_icyy);
}

ossimString ossimNitfRsmecaTag::getIcyz() const
{
   return ossimString(m_icyz);
}

ossimString ossimNitfRsmecaTag::getIczz() const
{
   return ossimString(m_iczz);
}

ossimString ossimNitfRsmecaTag::getGxo() const
{
   return ossimString(m_gxo);
}

ossimString ossimNitfRsmecaTag::getGyo() const
{
   return ossimString(m_gyo);
}

ossimString ossimNitfRsmecaTag::getGzo() const
{
   return ossimString(m_gzo);
}

ossimString ossimNitfRsmecaTag::getGxr() const
{
   return ossimString(m_gxr);
}

ossimString ossimNitfRsmecaTag::getGyr() const
{
   return ossimString(m_gyr);
}

ossimString ossimNitfRsmecaTag::getGzr() const
{
   return ossimString(m_gzr);
}

ossimString ossimNitfRsmecaTag::getGs() const
{
   return ossimString(m_gs);
}

ossimString ossimNitfRsmecaTag::getGxx() const
{
   return ossimString(m_gxx);
}

ossimString ossimNitfRsmecaTag::getGxy() const
{
   return ossimString(m_gxy);
}

ossimString ossimNitfRsmecaTag::getGxz() const
{
   return ossimString(m_gxz);
}

ossimString ossimNitfRsmecaTag::getGyx() const
{
   return ossimString(m_gyx);
}

ossimString ossimNitfRsmecaTag::getGyy() const
{
   return ossimString(m_gyy);
}

ossimString ossimNitfRsmecaTag::getGyz() const
{
   return ossimString(m_gyz);
}

ossimString ossimNitfRsmecaTag::getGzx() const
{
   return ossimString(m_gzx);
}

ossimString ossimNitfRsmecaTag::getGzy() const
{
   return ossimString(m_gzy);
}

ossimString ossimNitfRsmecaTag::getGzz() const
{
   return ossimString(m_gzz);
}

ossimNitfRsmecaIsg ossimNitfRsmecaTag::getIsg(ossim_uint32 index) const
{
   ossimNitfRsmecaIsg result;
   if (index < m_isg.size())
   {
      result = m_isg[index];
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmecaTag::getIsg range error!" << std::endl;
      }
   }   
   return result;
}

ossimString ossimNitfRsmecaTag::getMap(ossim_uint32 index) const
{
   ossimString result;
   if (index < m_map.size())
   {
      result = m_map[index];
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmecaTag::getMap range error!" << std::endl;
      }
   }   
   return result;
}

ossimString ossimNitfRsmecaTag::getUrr() const
{
   return ossimString(m_urr);
}

ossimString ossimNitfRsmecaTag::getUrc() const
{
   return ossimString(m_urc);
}

ossimString ossimNitfRsmecaTag::getUcc() const
{
   return ossimString(m_ucc);
}

ossimString ossimNitfRsmecaTag::getUncsr() const
{
   return ossimString(m_uncsr);
}

ossimString ossimNitfRsmecaTag::getUcorsr(ossim_uint32 index) const
{
   ossimString result;
   if (index < m_ucorsr.size())
   {
      result = m_ucorsr[index];
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmecaTag::getUcorsr range error!" << std::endl;
      }
   }   
   return result;
}

ossimString ossimNitfRsmecaTag::getUtausr(ossim_uint32 index) const
{
   ossimString result;
   if (index < m_utausr.size())
   {
      result = m_utausr[index];
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmecaTag::getUtausr range error!" << std::endl;
      }
   }   
   return result;
}

ossimString ossimNitfRsmecaTag::getUncsc() const
{
   return ossimString(m_uncsr);
}

ossimString ossimNitfRsmecaTag::getUcorsc(ossim_uint32 index) const
{
   ossimString result;
   if (index < m_ucorsc.size())
   {
      result = m_ucorsc[index];
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmecaTag::getUcorsc range error!" << std::endl;
      }
   }   
   return result;
}

ossimString ossimNitfRsmecaTag::getUtausc(ossim_uint32 index) const
{
   ossimString result;
   if (index < m_utausc.size())
   {
      result = m_utausc[index];
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmecaTag::getUtausc range error!" << std::endl;
      }
   }   
   return result;
}

bool ossimNitfRsmecaTag::getInclicFlag() const
{
   return m_inclicFlag;
}

bool ossimNitfRsmecaTag::getInclucFlag() const
{
   return m_inclucFlag;
}

ossimString ossimNitfRsmecaTag::getIgnNum() const
{
   return ossimString(m_ignNum);
}

ossimString ossimNitfRsmecaTag::getMapNum() const
{
   return ossimString(m_mapNum);
}

ossimString ossimNitfRsmecaTag::getRowCorSegNum() const
{
   return ossimString(m_rowCorSegNum);
}

ossimString ossimNitfRsmecaTag::getColCorSegNum() const
{
   return ossimString(m_colCorSegNum);
}

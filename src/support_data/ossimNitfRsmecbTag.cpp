#include <ossim/support_data/ossimNitfRsmecbTag.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimBooleanProperty.h>

#include <iostream>
#include <iomanip>

RTTI_DEF1(ossimNitfRsmecbTag, "ossimNitfRsmecbTag", ossimNitfRegisteredTag);

static const ossimString IID_PROPERTY     = "IID";
static const ossimString EDITION_PROPERTY = "EDITION";
static const ossimString INCLIC_PROPERTY  = "INCLIC";
static const ossimString INCLUC_PROPERTY  = "INCLUC";
static const ossimString NPAR_PROPERTY    = "NPAR";
static const ossimString NPARO_PROPERTY   = "NPARO";
static const ossimString IGN_PROPERTY     = "IGN";
static const ossimString CVDATE_PROPERTY  = "CVDATE";
static const ossimString APTYP_PROPERTY   = "APTYP";
static const ossimString LOCTYP_PROPERTY  = "LOCTYP";
static const ossimString NSFX_PROPERTY    = "NSFX";
static const ossimString NSFY_PROPERTY    = "NSFY";
static const ossimString NSFZ_PROPERTY    = "NSFZ";
static const ossimString NOFFX_PROPERTY   = "NOFFX";
static const ossimString NOFFY_PROPERTY   = "NOFFY";
static const ossimString NOFFZ_PROPERTY   = "NOFFZ";
static const ossimString XUOL_PROPERTY    = "XUOL";
static const ossimString YUOL_PROPERTY    = "YUOL";
static const ossimString ZUOL_PROPERTY    = "ZUOL";
static const ossimString XUXL_PROPERTY    = "XUXL";
static const ossimString XUYL_PROPERTY    = "XUYL";
static const ossimString XUZL_PROPERTY    = "XUZL";
static const ossimString YUXL_PROPERTY    = "YUXL";
static const ossimString YUYL_PROPERTY    = "YUYL";
static const ossimString YUZL_PROPERTY    = "YUZL";
static const ossimString ZUXL_PROPERTY    = "ZUXL";
static const ossimString ZUYL_PROPERTY    = "ZUYL";
static const ossimString ZUZL_PROPERTY    = "ZUZL";
static const ossimString APBASE_PROPERTY  = "APBASE";
static const ossimString NISAP_PROPERTY   = "NISAP";
static const ossimString NISAPR_PROPERTY  = "NISAPR";
static const ossimString NISAPC_PROPERTY  = "NISAPC";
static const ossimString NGSAP_PROPERTY   = "NGSAP";
static const ossimString NBASIS_PROPERTY  = "NBASIS";
static const ossimString URR_PROPERTY     = "URR";
static const ossimString URC_PROPERTY     = "URC";
static const ossimString UCC_PROPERTY     = "UCC";
static const ossimString UACSMC_PROPERTY  = "UACSMC";
static const ossimString UNCSR_PROPERTY   = "UNCSR";
static const ossimString UNCSC_PROPERTY   = "UNCSC";
static const ossimString UACR_PROPERTY    = "UACR";
static const ossimString UALPCR_PROPERTY  = "UALPCR";
static const ossimString UBETCR_PROPERTY  = "UBETCR";
static const ossimString UTCR_PROPERTY    = "UTCR";
static const ossimString UACC_PROPERTY    = "UACC";
static const ossimString UALPCC_PROPERTY  = "UALPCC";
static const ossimString UBETCC_PROPERTY  = "UBETCC";
static const ossimString UTCC_PROPERTY    = "UTCC";

ossimNitfRsmecbTag::ImageSpaceAdjustableParameter::ImageSpaceAdjustableParameter()
{
   clearFields();
}
void ossimNitfRsmecbTag::ImageSpaceAdjustableParameter::parseStream(std::istream &in)
{
   clearFields();
   in.read(m_parameterPowerX, XPW_SIZE);
   in.read(m_parameterPowerY, XPW_SIZE);
   in.read(m_parameterPowerZ, XPW_SIZE);
}

void ossimNitfRsmecbTag::ImageSpaceAdjustableParameter::writeStream(std::ostream& /* out */)
{
}

void ossimNitfRsmecbTag::ImageSpaceAdjustableParameter::clearFields()
{
   memset(m_parameterPowerX, ' ', XPW_SIZE);
   memset(m_parameterPowerY, ' ', YPW_SIZE);
   memset(m_parameterPowerZ, ' ', XPW_SIZE);

   m_parameterPowerX[YPW_SIZE] = '\0';
   m_parameterPowerY[YPW_SIZE] = '\0';
   m_parameterPowerZ[ZPW_SIZE] = '\0';
}

std::ostream &ossimNitfRsmecbTag::ImageSpaceAdjustableParameter::print(
   std::ostream &out, const std::string& /* prefix */) const 
{
    return out;
}

ossim_int16 ossimNitfRsmecbTag::ImageSpaceAdjustableParameter::getPowerX() const 
{
    return ossimString(m_parameterPowerX).toInt16();
}

ossim_int16 ossimNitfRsmecbTag::ImageSpaceAdjustableParameter::getPowerY() const
{
   return ossimString(m_parameterPowerY).toInt16();
}

ossim_int16 ossimNitfRsmecbTag::ImageSpaceAdjustableParameter::getPowerZ() const
{
   return ossimString(m_parameterPowerZ).toInt16();
}

ossimNitfRsmecbTag::CorrelationSegment::CorrelationSegment()
{
   clearFields();
}

void ossimNitfRsmecbTag::CorrelationSegment::parseStream(std::istream &in)
{
   clearFields();

   in.read(m_segmentCorrelationValue, FLOAT21_SIZE);
   in.read(m_segmentTauValue, FLOAT21_SIZE);
}

void ossimNitfRsmecbTag::CorrelationSegment::writeStream(std::ostream& /* out */)
{
}

void ossimNitfRsmecbTag::CorrelationSegment::clearFields()
{
   memset(m_segmentCorrelationValue, ' ', FLOAT21_SIZE);
   memset(m_segmentTauValue, ' ', FLOAT21_SIZE);

   m_segmentCorrelationValue[FLOAT21_SIZE] = '\0';
   m_segmentTauValue[FLOAT21_SIZE]         = '\0';
}

std::ostream &ossimNitfRsmecbTag::CorrelationSegment::print(
   std::ostream &out, const std::string& /* prefix */) const
{
   return out;
}

ossim_float64 ossimNitfRsmecbTag::CorrelationSegment::getCorrelation() const
{
   return ossimString(m_segmentCorrelationValue).toFloat64();
}

ossim_float64 ossimNitfRsmecbTag::CorrelationSegment::getTau() const
{
   return ossimString(m_segmentTauValue).toFloat64();
}

ossimNitfRsmecbTag::IGNEntry::IGNEntry()
{
   clearFields();
}

void ossimNitfRsmecbTag::IGNEntry::parseStream(std::istream &in)
{
   in.read(m_numopg, NUMOPG_SIZE);
   ossim_int64 numopg = getNumopg();
   ossim_int64 totalNumopg = ((numopg + 1)*(numopg)) / 2;
   ossim_int64 idx = 0;
   m_errorCovarianceElement.resize(totalNumopg);
   for (idx = 0; idx < totalNumopg; ++idx)
   {
      char elem[FLOAT21_SIZE + 1] = {'\0'};
      in.read(elem, FLOAT21_SIZE);
      m_errorCovarianceElement[idx] = ossimString(elem).toFloat64();
   }
   in.read(m_tcdf, TCDF_SIZE);
   in.read(m_acsmc, ACSMC_SIZE);
   if (!getAcsmc())
   {
      in.read(m_ncseg, NCSEG_SIZE);
      ossim_int64 ncseg = getNcseg();
      m_correlationSegmentArray.resize(ncseg);
      for (idx = 0; idx < ncseg; ++idx)
      {
         m_correlationSegmentArray[idx].parseStream(in);
      }
   }
   else
   {
      in.read(m_ac, FLOAT21_SIZE);
      in.read(m_alpc, FLOAT21_SIZE);
      in.read(m_betc, FLOAT21_SIZE);
      in.read(m_tc, FLOAT21_SIZE);
   }
}

void ossimNitfRsmecbTag::IGNEntry::writeStream(std::ostream& /* out */)
{
}

void ossimNitfRsmecbTag::IGNEntry::clearFields()
{
   std::memset(m_numopg, ' ', NUMOPG_SIZE);
   m_errorCovarianceElement.clear();
   std::memset(m_tcdf, ' ', TCDF_SIZE);
   std::memset(m_acsmc, ' ', ACSMC_SIZE);
   std::memset(m_ncseg, ' ', NCSEG_SIZE);
   m_correlationSegmentArray.clear();
   std::memset(m_ac, ' ', FLOAT21_SIZE);
   std::memset(m_alpc, ' ', FLOAT21_SIZE);
   std::memset(m_betc, ' ', FLOAT21_SIZE);
   std::memset(m_tc, ' ', FLOAT21_SIZE);
   m_numopg[NUMOPG_SIZE] = '\0';
   m_tcdf[TCDF_SIZE] = '\0';
   m_acsmc[ACSMC_SIZE] = '\0';
   m_ncseg[NCSEG_SIZE] = '\0';
   m_ac[FLOAT21_SIZE] = '\0';
   m_alpc[FLOAT21_SIZE] = '\0';
   m_betc[FLOAT21_SIZE] = '\0';
   m_tc[FLOAT21_SIZE] = '\0';
}
std::ostream &ossimNitfRsmecbTag::IGNEntry::print(
   std::ostream &out, const std::string& /* prefix */) const
{
   return out;
}

ossim_int64 ossimNitfRsmecbTag::IGNEntry::getNumopg() const
{
   return ossimString(m_numopg).toInt64();
}

bool ossimNitfRsmecbTag::IGNEntry::getAcsmc() const
{
   return ossimString(m_acsmc).toBool();
}

ossim_int64 ossimNitfRsmecbTag::IGNEntry::getNcseg() const
{
   return ossimString(m_ncseg).toInt64();
}

ossimNitfRsmecbTag::ossimNitfRsmecbTag()
{
   clearFields();
}

void ossimNitfRsmecbTag::parseStream(std::istream &in)
{
   clearFields();

   in.read(m_iid, IID_SIZE);
   in.read(m_edition, EDITION_SIZE);
   in.read(m_tid, TID_SIZE);
   in.read(m_inclic, INCLIC_SIZE);
   in.read(m_incluc, INCLUC_SIZE);

   if(getInclicFlag())
   {
      in.read(m_nparo, NPARO_SIZE);
      in.read(m_ign, IGN_SIZE);
      in.read(m_cvdate, CVDATE_SIZE);
      in.read(m_npar, NPAR_SIZE);
      in.read(m_aptype, APTYP_SIZE);
      in.read(m_loctype, LOCTYP_SIZE);
      in.read(m_nsfx, FLOAT21_SIZE);
      in.read(m_nsfy, FLOAT21_SIZE);
      in.read(m_nsfz, FLOAT21_SIZE);
      in.read(m_noffx, FLOAT21_SIZE);
      in.read(m_noffy, FLOAT21_SIZE);
      in.read(m_noffz, FLOAT21_SIZE);
      if (getLocalCoordinateSystemType().upcase() == "R")
      {
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
      }
      in.read(m_apbase, APBASE_SIZE);
      ossimString adjParamType = getAdjustableParameterType().upcase();
      if (adjParamType == "I")
      {
         in.read(m_nisap, NISAP_SIZE);
         in.read(m_nisapr, NISAPR_SIZE);
         ossim_int64 numImageSpaceAdjParam = getNumberOfImageSpaceAdjParametersRow();
         ossim_int64 idx = 0;
         for (idx = 0; idx < numImageSpaceAdjParam;++idx)
         {
            ImageSpaceAdjustableParameter adjParameter;
            adjParameter.parseStream(in);
            m_imageSpaceAdjParamRowArray.push_back(adjParameter);
         }
         in.read(m_nisapc, NISAPC_SIZE);
         numImageSpaceAdjParam = getNumberOfImageSpaceAdjParametersCol();
         for (idx = 0; idx < numImageSpaceAdjParam; ++idx)
         {
            ImageSpaceAdjustableParameter adjParameter;
            adjParameter.parseStream(in);
            m_imageSpaceAdjParamColArray.push_back(adjParameter);
         }
      }
      else if (adjParamType == "G")
      {
         ossim_int64 idx = 0;
         char GSAPID[4] = {'\0'};

         in.read(m_ngsap, NGSAP_SIZE);
         ossim_int64 numberOfGroundParams = getNumberOfGroundSpaceAdjParameters();
         m_gsapidArray.resize(numberOfGroundParams);
         for(idx = 0; idx < numberOfGroundParams; ++idx)
         {
            in.read(GSAPID, GSAPID_SIZE);
            m_gsapidArray[idx] = ossimString(GSAPID, GSAPID + GSAPID_SIZE).upcase();
         }
      }
      if(getAdjustableParameterBasisOption())
      {
         in.read(m_nbasis, NBASIS_SIZE);
         ossim_int64 nBasis = getNumberOfBasisAdjustableParameters();
         ossim_int64 nPar   = getNumberOfActiveAdjustableParameters();
         ossim_int64 total  = nBasis*nPar;
         ossim_int64 idx = 0;
         m_aelArray.resize(total);
         for (idx = 0; idx < total; ++idx)
         {
            char ael[FLOAT21_SIZE+1]={'\0'};
            in.read(ael, FLOAT21_SIZE);
            m_aelArray[idx] = ossimString(ael).toFloat64();
         }
      }
      ossim_int64 idx = 0;
      ossim_int64 ign = getNumberOfIndependentSubgroups();
      m_ignEntryArray.resize(ign);
      for(idx=0;idx<ign;++idx)
      {
         m_ignEntryArray[idx].parseStream(in);
      }
   }
   if(getInclucFlag())
   {
      in.read(m_urr, FLOAT21_SIZE);
      in.read(m_urc, FLOAT21_SIZE);
      in.read(m_ucc, FLOAT21_SIZE);
      in.read(m_uacsmc, UACSMC_SIZE);
      if (!getUnmodeledCSMCorrelationOptionFlag())
      {
         in.read(m_uncsr, UNCSR_SIZE);
         ossim_int64 uncsr = getUncsr();
         ossim_int64 idx = 0;
         m_uncsrEntries.resize(uncsr);
         for(idx = 0; idx < uncsr; ++idx)
         {
            m_uncsrEntries[idx].parseStream(in);
         }
         in.read(m_uncsc, UNCSC_SIZE);
         ossim_int64 uncsc = getUncsc();
         m_uncscEntries.resize(uncsc);
         for (idx = 0; idx < uncsc; ++idx)
         {
            m_uncscEntries[idx].parseStream(in);
         }
      }
      else
      {
         in.read(m_uacr, FLOAT21_SIZE);
         in.read(m_ualpcr, FLOAT21_SIZE);
         in.read(m_ubetcr, FLOAT21_SIZE);
         in.read(m_utcr, FLOAT21_SIZE);
         in.read(m_uacc, FLOAT21_SIZE);
         in.read(m_ualpcc, FLOAT21_SIZE);
         in.read(m_ubetcc, FLOAT21_SIZE);
         in.read(m_utcc, FLOAT21_SIZE);
      }
   }
}

void ossimNitfRsmecbTag::writeStream(std::ostream& /* out */)
{
}

void ossimNitfRsmecbTag::clearFields()
{
   std::memset(m_iid, ' ', IID_SIZE);
   std::memset(m_edition, ' ', EDITION_SIZE);
   std::memset(m_tid, ' ', TID_SIZE);
   std::memset(m_inclic, ' ', INCLIC_SIZE);
   std::memset(m_incluc, ' ', INCLUC_SIZE);
   std::memset(m_nparo, ' ', NPAR_SIZE);
   std::memset(m_ign, ' ', IGN_SIZE);
   std::memset(m_cvdate, ' ', CVDATE_SIZE);
   std::memset(m_npar, ' ', NPAR_SIZE);
   std::memset(m_aptype, ' ', APTYP_SIZE);
   std::memset(m_loctype, ' ', LOCTYP_SIZE);
   std::memset(m_nsfx, ' ', FLOAT21_SIZE);
   std::memset(m_nsfy, ' ', FLOAT21_SIZE);
   std::memset(m_nsfz, ' ', FLOAT21_SIZE);
   std::memset(m_noffx, ' ', FLOAT21_SIZE);
   std::memset(m_noffy, ' ', FLOAT21_SIZE);
   std::memset(m_noffz, ' ', FLOAT21_SIZE);
   std::memset(m_xuol, ' ', FLOAT21_SIZE);
   std::memset(m_yuol, ' ', FLOAT21_SIZE);
   std::memset(m_zuol, ' ', FLOAT21_SIZE);
   std::memset(m_xuxl, ' ', FLOAT21_SIZE);
   std::memset(m_xuyl, ' ', FLOAT21_SIZE);
   std::memset(m_xuzl, ' ', FLOAT21_SIZE);
   std::memset(m_yuxl, ' ', FLOAT21_SIZE);
   std::memset(m_yuyl, ' ', FLOAT21_SIZE);
   std::memset(m_yuzl, ' ', FLOAT21_SIZE);
   std::memset(m_zuxl, ' ', FLOAT21_SIZE);
   std::memset(m_zuyl, ' ', FLOAT21_SIZE);
   std::memset(m_zuzl, ' ', FLOAT21_SIZE);
   std::memset(m_apbase, ' ', APBASE_SIZE);
   std::memset(m_nisap, ' ', NISAP_SIZE);
   std::memset(m_nisapr, ' ', NISAPR_SIZE);
   m_imageSpaceAdjParamRowArray.clear();
   std::memset(m_nisapc, ' ', NISAPC_SIZE);
   m_imageSpaceAdjParamColArray.clear();
   std::memset(m_ngsap, ' ', NGSAP_SIZE);
   m_gsapidArray.clear();
   std::memset(m_ngsap, ' ', NBASIS_SIZE);
   m_aelArray.clear();
   m_map.clear();
   std::memset(m_urr, ' ', FLOAT21_SIZE);
   std::memset(m_urc, ' ', FLOAT21_SIZE);
   std::memset(m_ucc, ' ', FLOAT21_SIZE);
   std::memset(m_uacsmc, ' ', UACSMC_SIZE);
   std::memset(m_uncsr, ' ', UNCSR_SIZE);
   m_uncsrEntries.clear();
   std::memset(m_uncsr, ' ', UNCSR_SIZE);
   std::memset(m_uncsc, ' ', UNCSC_SIZE);
   m_uncscEntries.clear();
   std::memset(m_uacr, ' ', FLOAT21_SIZE);
   std::memset(m_ualpcr, ' ', FLOAT21_SIZE);
   std::memset(m_ubetcr, ' ', FLOAT21_SIZE);
   std::memset(m_utcr, ' ', FLOAT21_SIZE);
   std::memset(m_uacc, ' ', FLOAT21_SIZE);
   std::memset(m_ualpcc, ' ', FLOAT21_SIZE);
   std::memset(m_ubetcc, ' ', FLOAT21_SIZE);
   std::memset(m_utcc, ' ', FLOAT21_SIZE);

   m_iid[IID_SIZE]         = '\0';
   m_edition[EDITION_SIZE] = '\0';
   m_tid[TID_SIZE]         = '\0';
   m_inclic[INCLIC_SIZE]   = '\0';
   m_incluc[INCLUC_SIZE]   = '\0';
   m_nparo[NPAR_SIZE]      = '\0';
   m_ign[IGN_SIZE]         = '\0';
   m_cvdate[CVDATE_SIZE]   = '\0';
   m_npar[NPAR_SIZE]       = '\0';
   m_aptype[APTYP_SIZE]    = '\0';
   m_loctype[LOCTYP_SIZE]  = '\0';
   m_nsfx[FLOAT21_SIZE]    = '\0';
   m_nsfy[FLOAT21_SIZE]    = '\0';
   m_nsfz[FLOAT21_SIZE]    = '\0';
   m_noffx[FLOAT21_SIZE]   = '\0';
   m_noffy[FLOAT21_SIZE]   = '\0';
   m_noffz[FLOAT21_SIZE]   = '\0';
   m_xuol[FLOAT21_SIZE]    = '\0';
   m_yuol[FLOAT21_SIZE]    = '\0';
   m_zuol[FLOAT21_SIZE]    = '\0';
   m_xuxl[FLOAT21_SIZE]    = '\0';
   m_xuyl[FLOAT21_SIZE]    = '\0';
   m_xuzl[FLOAT21_SIZE]    = '\0';
   m_yuxl[FLOAT21_SIZE]    = '\0';
   m_yuyl[FLOAT21_SIZE]    = '\0';
   m_yuzl[FLOAT21_SIZE]    = '\0';
   m_zuxl[FLOAT21_SIZE]    = '\0';
   m_zuyl[FLOAT21_SIZE]    = '\0';
   m_zuzl[FLOAT21_SIZE]    = '\0';
   m_apbase[APBASE_SIZE]   = '\0';
   m_nisap[NISAP_SIZE]     = '\0';
   m_nisapr[NISAPR_SIZE]   = '\0';
   m_nisapc[NISAPC_SIZE]   = '\0';
   m_ngsap[NBASIS_SIZE]    = '\0';
   m_urr[FLOAT21_SIZE]     = '\0';
   m_urc[FLOAT21_SIZE]     = '\0';
   m_ucc[FLOAT21_SIZE]     = '\0';
   m_uacsmc[UACSMC_SIZE]   = '\0';
   m_uncsr[UNCSR_SIZE]     = '\0';
   m_uncsr[UNCSR_SIZE]     = '\0';
   m_uncsc[UNCSC_SIZE]     = '\0';
   m_uacr[FLOAT21_SIZE]    = '\0';
   m_ualpcr[FLOAT21_SIZE]  = '\0';
   m_ubetcr[FLOAT21_SIZE]  = '\0';
   m_utcr[FLOAT21_SIZE]    = '\0';
   m_uacc[FLOAT21_SIZE]    = '\0';
   m_ualpcc[FLOAT21_SIZE]  = '\0';
   m_ubetcc[FLOAT21_SIZE]  = '\0';
   m_utcc[FLOAT21_SIZE]    = '\0';
}

std::ostream &ossimNitfRsmecbTag::print(std::ostream &out,
                                      const std::string &prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG: " << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL: " << getTagLength() << "\n"
       << pfx << std::setw(24) << "IID: " << m_iid << "\n"
       << pfx << std::setw(24) << "EDITION: " << m_edition << "\n"
       << pfx << std::setw(24) << "TID: " << m_tid << "\n "
       << pfx << std::setw(24) << "INCLIC: " << m_inclic << "\n"
       << pfx << std::setw(24) << "INCLUC: " << m_incluc << "\n"
       << pfx << std::setw(24) << "NPARO: " << m_nparo << "\n"
       << pfx << std::setw(24) << "IGN: " << m_ign << "\n"
       << pfx << std::setw(24) << "CVDATE:" << m_cvdate << "\n"
       << pfx << std::setw(24) << "NPAR: " << m_npar << "\n"
       << pfx << std::setw(24) << "APTYPE: " << m_aptype << "\n"
       << pfx << std::setw(24) << "LOCTYPE: " << m_loctype << "\n"
       << pfx << std::setw(24) << "NSFX: " << m_nsfx << "\n"
       << pfx << std::setw(24) << "NSFY: " << m_nsfy << "\n"
       << pfx << std::setw(24) << "NSFZ: " << m_nsfz << "\n"
       << pfx << std::setw(24) << "NOFFX: " << m_noffx << "\n"
       << pfx << std::setw(24) << "NOFFY: " << m_noffy << "\n"
       << pfx << std::setw(24) << "NOFFZ: " << m_noffz << "\n"
       << pfx << std::setw(24) << "XUOL: " << m_xuol << "\n"
       << pfx << std::setw(24) << "YUOL: " << m_yuol << "\n"
       << pfx << std::setw(24) << "ZUOL: " << m_zuol << "\n"
       << pfx << std::setw(24) << "XUXL: " << m_xuxl << "\n"
       << pfx << std::setw(24) << "XUYL: " << m_xuyl << "\n"
       << pfx << std::setw(24) << "XUZL: " << m_xuzl << "\n"
       << pfx << std::setw(24) << "YUXL: " << m_yuxl << "\n"
       << pfx << std::setw(24) << "YUYL: " << m_yuyl << "\n"
       << pfx << std::setw(24) << "YUZL: " << m_yuzl << "\n"
       << pfx << std::setw(24) << "ZUXL: " << m_zuxl << "\n"
       << pfx << std::setw(24) << "ZUYL: " << m_zuyl << "\n"
       << pfx << std::setw(24) << "ZUZL: " << m_zuzl << "\n"
       << pfx << std::setw(24) << "APBASE: " << m_apbase << "\n"
       << pfx << std::setw(24) << "NISAP: " << m_nisap << "\n"
       << pfx << std::setw(24) << "NISAPR: " << m_nisapr << "\n"
       << pfx << std::setw(24) << "NISAPC: " << m_nisapc << "\n"
       << pfx << std::setw(24) << "NGSAP: " << m_ngsap << "\n"
       << pfx << std::setw(24) << "URR: " << m_urr << "\n"
       << pfx << std::setw(24) << "URC: " << m_urc << "\n"
       << pfx << std::setw(24) << "UCC: " << m_ucc << "\n"
       << pfx << std::setw(24) << "UACSMC: " << m_uacsmc << "\n"
       << pfx << std::setw(24) << "UNCSR: " << m_uncsr << "\n"
       << pfx << std::setw(24) << "UNCSC: " << m_uncsc << "\n"
       << pfx << std::setw(24) << "UACR: " << m_uacr << "\n"
       << pfx << std::setw(24) << "UALPCR: " << m_ualpcr << "\n"
       << pfx << std::setw(24) << "UBETCR: " << m_ubetcr << "\n"
       << pfx << std::setw(24) << "UTCR: " << m_utcr << "\n"
       << pfx << std::setw(24) << "UACC: " << m_uacc << "\n"
       << pfx << std::setw(24) << "UALPCC: " << m_ualpcc << "\n"
       << pfx << std::setw(24) << "UBETCC: " << m_ubetcc << "\n"
       << pfx << std::setw(24) << "UTCC: " << m_utcc << "\n";

       //   m_imageSpaceAdjParamRowArray.clear();
       // m_imageSpaceAdjParamColArray.clear();
       //m_gsapidArray.clear();
       //m_aelArray.clear();
       //m_map.clear();
       //m_uncsrEntries.clear();
       //   m_uncscEntries.clear();

      return out;
}

bool ossimNitfRsmecbTag::getInclicFlag() const
{
   return ossimString(m_inclic).toBool();
}

bool ossimNitfRsmecbTag::getInclucFlag() const
{
   return ossimString(m_incluc).toBool();
}

ossimString ossimNitfRsmecbTag::getLocalCoordinateSystemType() const
{
   return ossimString(m_loctype).trim();
}

ossimString ossimNitfRsmecbTag::getAdjustableParameterType() const
{
   return ossimString(m_aptype);
}

ossim_int64 ossimNitfRsmecbTag::getNumberOfImageSpaceAdjParametersRow() const
{
   return ossimString(m_nisapr).toInt64();
}

ossim_int64 ossimNitfRsmecbTag::getNumberOfImageSpaceAdjParametersCol() const
{
   return ossimString(m_nisapr).toInt64();
}

ossim_int64 ossimNitfRsmecbTag::getNumberOfGroundSpaceAdjParameters() const
{
   return ossimString(m_ngsap).toInt64();
}

bool ossimNitfRsmecbTag::getAdjustableParameterBasisOption() const
{
   return ossimString(m_apbase).toBool();
}

ossim_int64 ossimNitfRsmecbTag::getNumberOfBasisAdjustableParameters() const
{
   return ossimString(m_nbasis).toInt64();
}

ossim_int64 ossimNitfRsmecbTag::getNumberOfActiveAdjustableParameters()const
{
   return ossimString(m_npar).toInt64();
}
ossim_int64 ossimNitfRsmecbTag::getNumberOfIndependentSubgroups() const
{
   return ossimString(m_ign).toInt64();
}
bool ossimNitfRsmecbTag::getUnmodeledCSMCorrelationOptionFlag() const
{
   return ossimString(m_uacsmc).toBool();
}

ossim_int64 ossimNitfRsmecbTag::getUncsr() const
{
   return ossimString(m_uncsr).toInt64();
}

ossim_int64 ossimNitfRsmecbTag::getUncsc() const
{
   return ossimString(m_uncsc).toInt64();
}

void ossimNitfRsmecbTag::setProperty(ossimRefPtr<ossimProperty> property)
{
    ossimNitfRegisteredTag::setProperty(property);
}

ossimRefPtr<ossimProperty> ossimNitfRsmecbTag::getProperty(const ossimString &name) const
{
   ossimRefPtr<ossimProperty> result = ossimNitfRegisteredTag::getProperty(name);

   if(!result)
   {
      if(name == IID_PROPERTY)
      {
         result = new ossimStringProperty(IID_PROPERTY,
                                           ossimString(m_iid).trim());
      }
      else if(name == EDITION_PROPERTY)
      {
         result = new ossimStringProperty(EDITION_PROPERTY,
                                           ossimString(m_edition).trim());
      }
      else if (name == INCLIC_PROPERTY)
      {
         result = new ossimBooleanProperty(INCLIC_PROPERTY, 
                                           ossimString(m_inclic).toBool());
      }
      else if (name == INCLUC_PROPERTY)
      {
         result = new ossimBooleanProperty(INCLUC_PROPERTY,
                                           ossimString(m_incluc).toBool());
      }
   }

   return result;
}

void ossimNitfRsmecbTag::getPropertyNames(std::vector<ossimString> &propertyNames) const
{
   ossimNitfRegisteredTag::getPropertyNames(propertyNames);

   propertyNames.push_back(IID_PROPERTY);
   propertyNames.push_back(EDITION_PROPERTY);
   propertyNames.push_back(INCLIC_PROPERTY);
   propertyNames.push_back(INCLUC_PROPERTY);
   propertyNames.push_back(NPAR_PROPERTY);
   propertyNames.push_back(NPARO_PROPERTY);
   propertyNames.push_back(IGN_PROPERTY);
   propertyNames.push_back(CVDATE_PROPERTY);
   propertyNames.push_back(APTYP_PROPERTY);
   propertyNames.push_back(LOCTYP_PROPERTY);
   propertyNames.push_back(NSFX_PROPERTY);
   propertyNames.push_back(NSFY_PROPERTY);
   propertyNames.push_back(NSFZ_PROPERTY);
   propertyNames.push_back(NOFFX_PROPERTY);
   propertyNames.push_back(NOFFY_PROPERTY);
   propertyNames.push_back(NOFFZ_PROPERTY);
   propertyNames.push_back(XUOL_PROPERTY);
   propertyNames.push_back(YUOL_PROPERTY);
   propertyNames.push_back(ZUOL_PROPERTY);
   propertyNames.push_back(XUXL_PROPERTY);
   propertyNames.push_back(XUYL_PROPERTY);
   propertyNames.push_back(XUZL_PROPERTY);
   propertyNames.push_back(YUXL_PROPERTY);
   propertyNames.push_back(YUYL_PROPERTY);
   propertyNames.push_back(YUZL_PROPERTY);
   propertyNames.push_back(ZUXL_PROPERTY);
   propertyNames.push_back(ZUYL_PROPERTY);
   propertyNames.push_back(ZUZL_PROPERTY);
   propertyNames.push_back(APBASE_PROPERTY);
   propertyNames.push_back(NISAP_PROPERTY);
   propertyNames.push_back(NISAPR_PROPERTY);
   propertyNames.push_back(NISAPC_PROPERTY);
   propertyNames.push_back(NGSAP_PROPERTY);
   propertyNames.push_back(NBASIS_PROPERTY);
   propertyNames.push_back(URR_PROPERTY);
   propertyNames.push_back(URC_PROPERTY);
   propertyNames.push_back(UCC_PROPERTY);
   propertyNames.push_back(UACSMC_PROPERTY);
   propertyNames.push_back(UNCSR_PROPERTY);
   propertyNames.push_back(UNCSC_PROPERTY);
   propertyNames.push_back(UACR_PROPERTY);
   propertyNames.push_back(UALPCR_PROPERTY);
   propertyNames.push_back(UBETCR_PROPERTY);
   propertyNames.push_back(UTCR_PROPERTY);
   propertyNames.push_back(UACC_PROPERTY);
   propertyNames.push_back(UALPCC_PROPERTY);
   propertyNames.push_back(UBETCC_PROPERTY);
   propertyNames.push_back(UTCC_PROPERTY);
}

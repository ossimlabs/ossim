//---
// License: MIT
//
// Author:  David Burken
//
// Description: Container class definition for JBD Segment Security Metadata.
//
// See: ISO/IEC JOINT BIIF PROFILE (JBP)
// Table 5.10-1 and 5.10-2 of JBP-2024.1 version.
//---
// $Id$

#include <ossim/support_data/ossimNitfSegmentSecurityMetadataV2_1.h>
#include <ossim/support_data/ossimNitfCommon.h>

#include <cstring>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>


ossimNitfSegmentSecurityMetadataV2_1::ossimNitfSegmentSecurityMetadataV2_1()
   : m_filePartType()
{
   clearFields();
}

ossimNitfSegmentSecurityMetadataV2_1::ossimNitfSegmentSecurityMetadataV2_1(
   const ossimNitfSegmentSecurityMetadataV2_1& obj)
   : m_filePartType(obj.m_filePartType)
{
   clearFields();
   
   strncpy(m_sclas, obj.m_sclas, SCLAS_SIZE);
   strncpy(m_sclsy, obj.m_sclsy, SCLSY_SIZE);
   strncpy(m_scode, obj.m_scode, SCODE_SIZE);
   strncpy(m_sctlh, obj.m_sctlh, SCTLH_SIZE);
   strncpy(m_srel, obj.m_srel, SREL_SIZE);
   strncpy(m_sdctp, obj.m_sdctp, SDCTP_SIZE);
   strncpy(m_sdcdt, obj.m_sdcdt, SDCDT_SIZE);
   strncpy(m_sdcxm, obj.m_sdcxm, SDCXM_SIZE);
   strncpy(m_sdg, obj.m_sdg, SDG_SIZE);
   strncpy(m_sdgdt, obj.m_sdgdt, SDGDT_SIZE);
   strncpy(m_scltx, obj.m_scltx, SCLTX_SIZE);
   strncpy(m_scatp, obj.m_scatp, SCATP_SIZE);
   strncpy(m_scaut, obj.m_scaut, SCAUT_SIZE);
   strncpy(m_scrsn, obj.m_scrsn, SCRSN_SIZE);
   strncpy(m_ssrdt, obj.m_ssrdt, SSRDT_SIZE);
   strncpy(m_sctln, obj.m_sctln, SCTLN_SIZE);
}

const ossimNitfSegmentSecurityMetadataV2_1& ossimNitfSegmentSecurityMetadataV2_1::operator=(
   const ossimNitfSegmentSecurityMetadataV2_1& rhs)
{
   if (this != &rhs)
   {
      m_filePartType = rhs.m_filePartType;
      strncpy(m_sclas, rhs.m_sclas, SCLAS_SIZE);
      strncpy(m_sclsy, rhs.m_sclsy, SCLSY_SIZE);
      strncpy(m_scode, rhs.m_scode, SCODE_SIZE);
      strncpy(m_sctlh, rhs.m_sctlh, SCTLH_SIZE);
      strncpy(m_srel, rhs.m_srel, SREL_SIZE);
      strncpy(m_sdctp, rhs.m_sdctp, SDCTP_SIZE);
      strncpy(m_sdcdt, rhs.m_sdcdt, SDCDT_SIZE);
      strncpy(m_sdcxm, rhs.m_sdcxm, SDCXM_SIZE);
      strncpy(m_sdg, rhs.m_sdg, SDG_SIZE);
      strncpy(m_sdgdt, rhs.m_sdgdt, SDGDT_SIZE);
      strncpy(m_scltx, rhs.m_scltx, SCLTX_SIZE);
      strncpy(m_scatp, rhs.m_scatp, SCATP_SIZE);
      strncpy(m_scaut, rhs.m_scaut, SCAUT_SIZE);
      strncpy(m_scrsn, rhs.m_scrsn, SCRSN_SIZE);
      strncpy(m_ssrdt, rhs.m_ssrdt, SSRDT_SIZE);
      strncpy(m_sctln, rhs.m_sctln, SCTLN_SIZE);
   }
   return *this;
}

ossim_uint32 ossimNitfSegmentSecurityMetadataV2_1::getLength() const
{
   return 167;
}

void ossimNitfSegmentSecurityMetadataV2_1::setFilePartType(const std::string& type)
{
   m_filePartType = type;
}

void ossimNitfSegmentSecurityMetadataV2_1::set_sclas(const char* sclas)
{
   ossimNitfCommon::setField(m_sclas, ossimString(sclas), SCLAS_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_sclsy(const char* sclsy)
{
   ossimNitfCommon::setField(m_sclsy, ossimString(sclsy), SCLSY_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_scode(const char* scode)
{
   ossimNitfCommon::setField(m_scode, ossimString(scode), SCODE_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_sctlh(const char* sctlh)
{
   ossimNitfCommon::setField(m_sctlh, ossimString(sctlh), SCTLH_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_srel(const char* srel)
{
   ossimNitfCommon::setField(m_srel, ossimString(srel), SREL_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_sdctp(const char* sdctp)
{
   ossimNitfCommon::setField(m_sdctp, ossimString(sdctp), SDCTP_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_sdcdt(const char* sdcdt)
{
   ossimNitfCommon::setField(m_sdcdt, ossimString(sdcdt), SDCDT_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_sdcxm(const char* sdcxm)
{
   ossimNitfCommon::setField(m_sdcxm, ossimString(sdcxm), SDCXM_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_sdg(const char* sdg)
{
   ossimNitfCommon::setField(m_sdg, ossimString(sdg), SDG_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_sdgdt(const char* sdgdt)
{
   ossimNitfCommon::setField(m_sdgdt, ossimString(sdgdt), SDGDT_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_scltx(const char* scltx)
{
   ossimNitfCommon::setField(m_scltx, ossimString(scltx), SCLTX_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_scatp(const char* scatp)
{
   ossimNitfCommon::setField(m_scatp, ossimString(scatp), SCATP_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_scaut(const char* scaut)
{
   ossimNitfCommon::setField(m_scaut, ossimString(scaut), SCAUT_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_scrsn(const char* scrsn)
{
   ossimNitfCommon::setField(m_scrsn, ossimString(scrsn), SCRSN_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_ssrdt(const char* ssrdt)
{
   ossimNitfCommon::setField(m_ssrdt, ossimString(ssrdt), SSRDT_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::set_sctln(const char* sctln)
{
   ossimNitfCommon::setField(m_sctln, ossimString(sctln), SCTLN_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::parseStream(std::istream& in)
{
   clearFields();
   in.read(m_sclas, SCLAS_SIZE);
   in.read(m_sclsy, SCLSY_SIZE);
   in.read(m_scode, SCODE_SIZE);
   in.read(m_sctlh, SCTLH_SIZE);
   in.read(m_srel,  SREL_SIZE);
   in.read(m_sdctp, SDCTP_SIZE);
   in.read(m_sdcdt, SDCDT_SIZE);
   in.read(m_sdcxm, SDCXM_SIZE);
   in.read(m_sdg,   SDG_SIZE);
   in.read(m_sdgdt, SDGDT_SIZE);
   in.read(m_scltx, SCLTX_SIZE);
   in.read(m_scatp, SCATP_SIZE);
   in.read(m_scaut, SCAUT_SIZE);
   in.read(m_scrsn, SCRSN_SIZE);
   in.read(m_ssrdt, SSRDT_SIZE);
   in.read(m_sctln, SCTLN_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::writeStream(std::ostream& out)
{
   out.write(m_sclas, SCLAS_SIZE);
   out.write(m_sclsy, SCLSY_SIZE);
   out.write(m_scode, SCODE_SIZE);
   out.write(m_sctlh, SCTLH_SIZE);
   out.write(m_srel,  SREL_SIZE);
   out.write(m_sdctp, SDCTP_SIZE);
   out.write(m_sdcdt, SDCDT_SIZE);
   out.write(m_sdcxm, SDCXM_SIZE);
   out.write(m_sdg,   SDG_SIZE);
   out.write(m_sdgdt, SDGDT_SIZE);
   out.write(m_scltx, SCLTX_SIZE);
   out.write(m_scatp, SCATP_SIZE);
   out.write(m_scaut, SCAUT_SIZE);
   out.write(m_scrsn, SCRSN_SIZE);
   out.write(m_ssrdt, SSRDT_SIZE);
   out.write(m_sctln, SCTLN_SIZE);
}

void ossimNitfSegmentSecurityMetadataV2_1::clearFields()
{
   memset(m_sclas, ' ', SCLAS_SIZE);
   memset(m_sclsy, ' ', SCLSY_SIZE);
   memset(m_scode, ' ', SCODE_SIZE);
   memset(m_sctlh, ' ', SCTLH_SIZE);
   memset(m_srel, ' ', SREL_SIZE);
   memset(m_sdctp, ' ', SDCTP_SIZE);
   memset(m_sdcdt, ' ', SDCDT_SIZE);
   memset(m_sdcxm, ' ', SDCXM_SIZE);
   memset(m_sdg, ' ', SDG_SIZE);
   memset(m_sdgdt, ' ', SDGDT_SIZE);
   memset(m_scltx, ' ', SCLTX_SIZE);
   memset(m_scatp, ' ', SCATP_SIZE);
   memset(m_scaut, ' ', SCAUT_SIZE);
   memset(m_scrsn, ' ', SCRSN_SIZE);
   memset(m_ssrdt, ' ', SSRDT_SIZE);
   memset(m_sctln, ' ', SCTLN_SIZE);
   
   m_sclas[SCLAS_SIZE] = '\0';
   m_sclsy[SCLSY_SIZE] = '\0';
   m_scode[SCODE_SIZE] = '\0';
   m_sctlh[SCTLH_SIZE] = '\0';
   m_srel[SREL_SIZE]   = '\0';
   m_sdctp[SDCTP_SIZE] = '\0';
   m_sdcdt[SDCDT_SIZE] = '\0';
   m_sdcxm[SDCXM_SIZE] = '\0';
   m_sdg[SDG_SIZE]     = '\0';
   m_sdgdt[SDGDT_SIZE] = '\0';
   m_scltx[SCLTX_SIZE] = '\0';
   m_scatp[SCATP_SIZE] = '\0';
   m_scaut[SCAUT_SIZE] = '\0';
   m_scrsn[SCRSN_SIZE] = '\0';
   m_ssrdt[SSRDT_SIZE] = '\0';
   m_sctln[SCTLN_SIZE] = '\0';
}

std::ostream& ossimNitfSegmentSecurityMetadataV2_1::print(
   std::ostream& out, const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += m_filePartType;
   int w = (int)((pfx.size()<29)?29-pfx.size():24);

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(w) << "SCLAS:" << m_sclas << "\n"
       << pfx << std::setw(w) << "SCLSY:" << m_sclsy << "\n"
       << pfx << std::setw(w) << "SCODE:" << m_scode << "\n"
       << pfx << std::setw(w) << "SCTLH:" << m_sctlh << "\n"
       << pfx << std::setw(w) << "SREL:"  << m_srel  << "\n"
       << pfx << std::setw(w) << "SDCTP:" << m_sdctp << "\n"
       << pfx << std::setw(w) << "SDCDT:" << m_sdcdt << "\n"
       << pfx << std::setw(w) << "SDCXM:" << m_sdcxm << "\n"
       << pfx << std::setw(w) << "SDG:"   << m_sdg   << "\n"
       << pfx << std::setw(w) << "SDGDT:" << m_sdgdt << "\n"
       << pfx << std::setw(w) << "SCLTX:" << m_scltx << "\n"
       << pfx << std::setw(w) << "SCATP:" << m_scatp << "\n"
       << pfx << std::setw(w) << "SCAUT:" << m_scaut << "\n"
       << pfx << std::setw(w) << "SCRSN:" << m_scrsn << "\n"
       << pfx << std::setw(w) << "SSRDT:" << m_ssrdt << "\n"
       << pfx << std::setw(w) << "SCTLN:" << m_sctln << std::endl;
   return out;
}

std::ostream& operator<<(std::ostream& out, const ossimNitfSegmentSecurityMetadataV2_1& obj)
{
   return obj.print(out, std::string(""));
}

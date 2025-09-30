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

#include <ossim/support_data/ossimNitfSegmentSecurityMetadataV1.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>

#include <cstring>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>


ossimNitfSegmentSecurityMetadataV1::ossimNitfSegmentSecurityMetadataV1()
   : m_segmentPrefix()
{
   clearFields();
}

ossimNitfSegmentSecurityMetadataV1::ossimNitfSegmentSecurityMetadataV1(
   const ossimNitfSegmentSecurityMetadataV1& obj)
   : m_segmentPrefix(obj.m_segmentPrefix)
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

const ossimNitfSegmentSecurityMetadataV1& ossimNitfSegmentSecurityMetadataV1::operator=(
   const ossimNitfSegmentSecurityMetadataV1& rhs)
{
   if (this != &rhs)
   {
      m_segmentPrefix = rhs.m_segmentPrefix;
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

void ossimNitfSegmentSecurityMetadataV1::setSegmentPrefix(const std::string& prefix)
{
   m_segmentPrefix = prefix;
}

void ossimNitfSegmentSecurityMetadataV1::set_sclas(const char* sclas)
{
   strncpy(m_sclas, sclas, SCLAS_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_sclsy(const char* sclsy)
{
   strncpy( m_sclsy, sclsy, SCLSY_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_scode(const char* scode)
{
   strncpy(m_scode, scode, SCODE_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_sctlh(const char* sctlh)
{
   strncpy(m_sctlh, sctlh, SCTLH_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_srel(const char* srel)
{
   strncpy(m_srel, srel, SREL_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_sdctp(const char* sdctp)
{
   strncpy(m_sdctp , sdctp, SDCTP_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_sdcdt(const char* sdcdt)
{
   strncpy(m_sdcdt, sdcdt, SDCDT_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_sdcxm(const char* sdcxm)
{
   strncpy(m_sdcxm, sdcxm, SDCXM_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_sdg(const char* sdg)
{
   strncpy(m_sdg, sdg, SDG_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_sdgdt(const char* sdgdt)
{
   strncpy(m_sdgdt, sdgdt, SDGDT_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_scltx(const char* scltx)
{
   strncpy(m_scltx, scltx, SCLTX_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_scatp(const char* scatp)
{
   strncpy(m_scatp, scatp, SCATP_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_scaut(const char* scaut)
{
   strncpy(m_scaut, scaut, SCAUT_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_scrsn(const char* scrsn)
{
   strncpy(m_scrsn, scrsn, SCRSN_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_ssrdt(const char* ssrdt)
{
   strncpy(m_ssrdt, ssrdt, SSRDT_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::set_sctln(const char* sctln)
{
   strncpy(m_sctln, sctln, SCTLN_SIZE);
}

void ossimNitfSegmentSecurityMetadataV1::parseStream(std::istream& in)
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

void ossimNitfSegmentSecurityMetadataV1::writeStream(std::ostream& out)
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

void ossimNitfSegmentSecurityMetadataV1::clearFields()
{
   memset(m_sclas, ' ', SCLAS_SIZE);
   m_sclas[SCLAS_SIZE] = '\0';
   memset(m_sclsy, ' ', SCLSY_SIZE);
   m_sclsy[SCLSY_SIZE] = '\0';
   memset(m_scode, ' ', SCODE_SIZE);
   m_scode[SCODE_SIZE] = '\0';
   memset(m_sctlh, ' ', SCTLH_SIZE);
   m_sctlh[SCTLH_SIZE] = '\0';
   memset(m_srel, ' ', SREL_SIZE);
   m_srel[SREL_SIZE] = '\0';
   memset(m_sdctp, ' ', SDCTP_SIZE);
   m_sdctp[SDCTP_SIZE] = '\0';
   memset(m_sdcdt, ' ', SDCDT_SIZE);
   m_sdcdt[SDCDT_SIZE] = '\0';
   memset(m_sdcxm, ' ', SDCXM_SIZE);
   m_sdcxm[SDCXM_SIZE] = '\0';
   memset(m_sdg, ' ', SDG_SIZE);
   m_sdg[SDG_SIZE] = '\0';
   memset(m_sdgdt, ' ', SDGDT_SIZE);
   m_sdgdt[SDGDT_SIZE] = '\0';
   memset(m_scltx, ' ', SCLTX_SIZE);
   m_scltx[SCLTX_SIZE] = '\0';
   memset(m_scatp, ' ', SCATP_SIZE);
   m_scatp[SCATP_SIZE] = '\0';
   memset(m_scaut, ' ', SCAUT_SIZE);
   m_scaut[SCAUT_SIZE] = '\0';
   memset(m_scrsn, ' ', SCRSN_SIZE);
   m_scrsn[SCRSN_SIZE] = '\0';
   memset(m_ssrdt, ' ', SSRDT_SIZE);
   m_ssrdt[SSRDT_SIZE] = '\0';
   memset(m_sctln, ' ', SCTLN_SIZE);
   m_sctln[SCTLN_SIZE] = '\0';
}

std::ostream& ossimNitfSegmentSecurityMetadataV1::print(std::ostream& out,
                                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += m_segmentPrefix;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(24) << "SCLAS:" << m_sclas  << "\n"
       << pfx << std::setw(24) << "SCLSY:" << m_sclsy << "\n"
       << pfx << std::setw(24) << "SCODE:" << m_scode << "\n"
       << pfx << std::setw(24) << "SCTLH:" << m_sctlh << "\n"
       << pfx << std::setw(24) << "SREL:"  << m_srel  << "\n"
       << pfx << std::setw(24) << "SDCTP:" << m_sdctp << "\n"
       << pfx << std::setw(24) << "SDCDT:" << m_sdcdt << "\n"
       << pfx << std::setw(24) << "SDCXM:" << m_sdcxm << "\n"
       << pfx << std::setw(24) << "SDG:"   << m_sdg   << "\n"
       << pfx << std::setw(24) << "SDGDT:" << m_sdgdt << "\n"
       << pfx << std::setw(24) << "SCLTX:" << m_scltx << "\n"
       << pfx << std::setw(24) << "SCATP:" << m_scatp << "\n"
       << pfx << std::setw(24) << "SCAUT:" << m_scaut << "\n"
       << pfx << std::setw(24) << "SCRSN:" << m_scrsn << "\n"
       << pfx << std::setw(24) << "SSRDT:" << m_ssrdt << "\n"
       << pfx << std::setw(24) << "SCTLN:" << m_sctln << std::endl;
   return out;
}

std::ostream& operator<<(std::ostream& out, const ossimNitfSegmentSecurityMetadataV1& obj)
{
   return obj.print(out, std::string(""));
}

//---
// File: ossimNitfRsmpiaTag.cpp
//---


#include <ossim/support_data/ossimNitfRsmpiaTag.h>
#include <iomanip>
#include <iostream>

RTTI_DEF1(ossimNitfRsmpiaTag, "ossimNitfRsmpiaTag", ossimNitfRegisteredTag);

ossimNitfRsmpiaTag::ossimNitfRsmpiaTag()
   :
   ossimNitfRegisteredTag(std::string("RSMPIA"), CEL_SIZE),
   m_iid(),
   m_edition(),
   m_r0(),
   m_rx(),
   m_ry(),
   m_rz(),
   m_rxx(),
   m_rxy(),
   m_rxz(),
   m_ryy(),
   m_ryz(),
   m_rzz(),
   m_c0(),
   m_cx(),
   m_cy(),
   m_cz(),
   m_cxx(),
   m_cxy(),
   m_cxz(),
   m_cyy(),
   m_cyz(),
   m_czz(),
   m_rnis(),
   m_cnis(),
   m_tnis(),
   m_rssiz(),
   m_cssiz()   
{
   clearFields();
}

void ossimNitfRsmpiaTag::parseStream(std::istream& in)
{
   in.read(m_iid, IID_SIZE);
   in.read(m_edition, EDITION_SIZE);
   in.read(m_r0, FLOAT21_SIZE);
   in.read(m_rx, FLOAT21_SIZE);
   in.read(m_ry, FLOAT21_SIZE);
   in.read(m_rz, FLOAT21_SIZE);
   in.read(m_rxx, FLOAT21_SIZE);
   in.read(m_rxy, FLOAT21_SIZE);
   in.read(m_rxz, FLOAT21_SIZE);
   in.read(m_ryy, FLOAT21_SIZE);
   in.read(m_ryz, FLOAT21_SIZE);
   in.read(m_rzz, FLOAT21_SIZE);
   in.read(m_c0, FLOAT21_SIZE);
   in.read(m_cx, FLOAT21_SIZE);
   in.read(m_cy, FLOAT21_SIZE);
   in.read(m_cz, FLOAT21_SIZE);
   in.read(m_cxx, FLOAT21_SIZE);
   in.read(m_cxy, FLOAT21_SIZE);
   in.read(m_cxz, FLOAT21_SIZE);
   in.read(m_cyy, FLOAT21_SIZE);
   in.read(m_cyz, FLOAT21_SIZE);
   in.read(m_czz, FLOAT21_SIZE);
   in.read(m_rnis, NIS_SIZE);
   in.read(m_cnis, NIS_SIZE);
   in.read(m_tnis, NIS_SIZE);
   in.read(m_rssiz, FLOAT21_SIZE);
   in.read(m_cssiz, FLOAT21_SIZE);
}

void ossimNitfRsmpiaTag::writeStream(std::ostream& out)
{
   out.write(m_iid, IID_SIZE);
   out.write(m_edition, EDITION_SIZE);
   out.write(m_r0, FLOAT21_SIZE);
   out.write(m_rx, FLOAT21_SIZE);
   out.write(m_ry, FLOAT21_SIZE);
   out.write(m_rz, FLOAT21_SIZE);
   out.write(m_rxx, FLOAT21_SIZE);
   out.write(m_rxy, FLOAT21_SIZE);
   out.write(m_rxz, FLOAT21_SIZE);
   out.write(m_ryy, FLOAT21_SIZE);
   out.write(m_ryz, FLOAT21_SIZE);
   out.write(m_rzz, FLOAT21_SIZE);
   out.write(m_c0, FLOAT21_SIZE);
   out.write(m_cx, FLOAT21_SIZE);
   out.write(m_cy, FLOAT21_SIZE);
   out.write(m_cz, FLOAT21_SIZE);
   out.write(m_cxx, FLOAT21_SIZE);
   out.write(m_cxy, FLOAT21_SIZE);
   out.write(m_cxz, FLOAT21_SIZE);
   out.write(m_cyy, FLOAT21_SIZE);
   out.write(m_cyz, FLOAT21_SIZE);
   out.write(m_czz, FLOAT21_SIZE);
   out.write(m_rnis, NIS_SIZE);
   out.write(m_cnis, NIS_SIZE);
   out.write(m_tnis, NIS_SIZE);
   out.write(m_rssiz, FLOAT21_SIZE);
   out.write(m_cssiz, FLOAT21_SIZE);
}

std::ostream& ossimNitfRsmpiaTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   out << setiosflags(ios::left)
       << pfx << std::setw(24) << "CETAG:"      << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"        << getTagLength() << "\n"
       << pfx << std::setw(24) << "IID:" << m_iid << "\n"
       << pfx << std::setw(24) << "EDITION:" << m_edition << "\n"
       << pfx << std::setw(24) << "R0:" << m_r0 << "\n"
       << pfx << std::setw(24) << "RX:" << m_rx << "\n"
       << pfx << std::setw(24) << "RY:" << m_ry << "\n"
       << pfx << std::setw(24) << "RZ:" << m_rz << "\n"
       << pfx << std::setw(24) << "RXX:" << m_rxx << "\n"
       << pfx << std::setw(24) << "RXY:" << m_rxy << "\n"
       << pfx << std::setw(24) << "RXZ:" << m_rxz << "\n"
       << pfx << std::setw(24) << "RYY:" << m_ryy << "\n"
       << pfx << std::setw(24) << "RYZ:" << m_ryz << "\n"
       << pfx << std::setw(24) << "RZZ:" << m_rzz << "\n"
       << pfx << std::setw(24) << "C0:" << m_c0 << "\n"
       << pfx << std::setw(24) << "CX:" << m_cx << "\n"
       << pfx << std::setw(24) << "CY:" << m_cy << "\n"
       << pfx << std::setw(24) << "CZ:" << m_cz << "\n"
       << pfx << std::setw(24) << "CXX:" << m_cxx << "\n"
       << pfx << std::setw(24) << "CXY:" << m_cxy << "\n"
       << pfx << std::setw(24) << "CXZ:" << m_cxz << "\n"
       << pfx << std::setw(24) << "CYY:" << m_cyy << "\n"
       << pfx << std::setw(24) << "CYZ:" << m_cyz << "\n"
       << pfx << std::setw(24) << "CZZ:" << m_czz << "\n"
       << pfx << std::setw(24) << "RNIS:" << m_rnis << "\n"
       << pfx << std::setw(24) << "CNIS:" << m_cnis << "\n"
       << pfx << std::setw(24) << "TNIS:" << m_tnis << "\n"
       << pfx << std::setw(24) << "RSSIZ:" << m_rssiz << "\n"
       << pfx << std::setw(24) << "CSSIZ:" << m_cssiz << "\n";

   return out;
}

void ossimNitfRsmpiaTag::clearFields()
{
   memset(m_iid,' ', IID_SIZE);
   memset(m_edition, ' ', EDITION_SIZE);
   memset(m_r0, ' ', FLOAT21_SIZE);
   memset(m_rx, ' ', FLOAT21_SIZE);
   memset(m_ry, ' ', FLOAT21_SIZE);
   memset(m_rz, ' ', FLOAT21_SIZE);
   memset(m_rxx, ' ', FLOAT21_SIZE);
   memset(m_rxy, ' ', FLOAT21_SIZE);
   memset(m_rxz, ' ', FLOAT21_SIZE);
   memset(m_ryy, ' ', FLOAT21_SIZE);
   memset(m_ryz, ' ', FLOAT21_SIZE);
   memset(m_rzz, ' ', FLOAT21_SIZE);
   memset(m_c0, ' ', FLOAT21_SIZE);
   memset(m_cx, ' ', FLOAT21_SIZE);
   memset(m_cy, ' ', FLOAT21_SIZE);
   memset(m_cz, ' ', FLOAT21_SIZE);
   memset(m_cxx, ' ', FLOAT21_SIZE);
   memset(m_cxy, ' ', FLOAT21_SIZE);
   memset(m_cxz, ' ', FLOAT21_SIZE);
   memset(m_cyy, ' ', FLOAT21_SIZE);
   memset(m_cyz, ' ', FLOAT21_SIZE);
   memset(m_czz, ' ', FLOAT21_SIZE);
   memset(m_rnis, ' ', NIS_SIZE);
   memset(m_cnis, ' ', NIS_SIZE);
   memset(m_tnis, ' ', NIS_SIZE);
   memset(m_rssiz, ' ', FLOAT21_SIZE);
   memset(m_cssiz, ' ', FLOAT21_SIZE);

   m_iid[IID_SIZE] = '\0';
   m_edition[EDITION_SIZE] = '\0';
   m_r0[FLOAT21_SIZE] = '\0';
   m_rx[FLOAT21_SIZE] = '\0';
   m_ry[FLOAT21_SIZE] = '\0';
   m_rz[FLOAT21_SIZE] = '\0';
   m_rxx[FLOAT21_SIZE] = '\0';
   m_rxy[FLOAT21_SIZE] = '\0';
   m_rxz[FLOAT21_SIZE] = '\0';
   m_ryy[FLOAT21_SIZE] = '\0';
   m_ryz[FLOAT21_SIZE] = '\0';
   m_rzz[FLOAT21_SIZE] = '\0';
   m_c0[FLOAT21_SIZE] = '\0';
   m_cx[FLOAT21_SIZE] = '\0';
   m_cy[FLOAT21_SIZE] = '\0';
   m_cz[FLOAT21_SIZE] = '\0';
   m_cxx[FLOAT21_SIZE] = '\0';
   m_cxy[FLOAT21_SIZE] = '\0';
   m_cxz[FLOAT21_SIZE] = '\0';
   m_cyy[FLOAT21_SIZE] = '\0';
   m_cyz[FLOAT21_SIZE] = '\0';
   m_czz[FLOAT21_SIZE] = '\0';
   m_rnis[NIS_SIZE] = '\0';
   m_cnis[NIS_SIZE] = '\0';
   m_tnis[NIS_SIZE] = '\0';
   m_rssiz[FLOAT21_SIZE] = '\0';
   m_cssiz[FLOAT21_SIZE] = '\0';
}

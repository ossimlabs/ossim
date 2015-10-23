//---
// File: ossimNitfRsmidaTag.cpp
//---

#include <ossim/support_data/ossimNitfRsmidaTag.h>
#include <iomanip>
#include <iostream>

RTTI_DEF1(ossimNitfRsmidaTag, "ossimNitfRsmidaTag", ossimNitfRegisteredTag);

ossimNitfRsmidaTag::ossimNitfRsmidaTag()
   :
   ossimNitfRegisteredTag(std::string("RSMIDA"), CEL_SIZE),
   m_iid(),
   m_edition(),
   m_isid(),
   m_sid(),
   m_stid(),
   m_year(),
   m_month(),
   m_day(),
   m_hour(),
   m_minute(),
   m_second(),
   m_nrg(),
   m_ncg(),
   m_trg(),
   m_tcg(),
   m_grndd(),

   m_xuor(),
   m_yuor(),
   m_zuor(),
   m_xuxr(),
   m_xuyr(),
   m_xuzr(),
   m_yuxr(),
   m_yuyr(),
   m_yuzr(),
   m_zuxr(),
   m_zuyr(),
   m_zuzr(),

   m_v1x(),
   m_v1y(),
   m_v1z(),
   m_v2x(),
   m_v2y(),
   m_v2z(),
   m_v3x(),
   m_v3y(),
   m_v3z(),
   m_v4x(),
   m_v4y(),
   m_v4z(),
   m_v5x(),
   m_v5y(),
   m_v5z(),
   m_v6x(),
   m_v6y(),
   m_v6z(),
   m_v7x(),
   m_v7y(),
   m_v7z(),
   m_v8x(),
   m_v8y(),
   m_v8z(),

   m_grpx(),
   m_grpy(),
   m_grpz(),

   m_fullr(),
   m_fullc(),

   m_minr(),
   m_maxr(),
   m_minc(),
   m_maxc(),

   m_ie0(),
   m_ier(),
   m_iec(),
   m_ierr(),
   m_ierc(),
   m_iecc(),
   m_ia0(),
   m_iar(),
   m_iac(),
   m_iarr(),
   m_iarc(),
   m_iacc(),

   m_spx(),
   m_svx(),
   m_sax(),
   m_spy(),
   m_svy(),
   m_say(),
   m_spz(),
   m_svz(),
   m_saz()
{
   clearFields();
}

void ossimNitfRsmidaTag::parseStream(std::istream& in)
{
   in.read(m_iid, IID_SIZE);
   in.read(m_edition, EDITION_SIZE);
   in.read(m_isid, ISID_SIZE);
   in.read(m_sid, SID_SIZE);
   in.read(m_stid, STID_SIZE);
   in.read(m_year, YEAR_SIZE);
   in.read(m_month, MONTH_SIZE);
   in.read(m_day, DAY_SIZE);
   in.read(m_hour, HOUR_SIZE);
   in.read(m_minute, MINUTE_SIZE);
   in.read(m_second, SECOND_SIZE);
   in.read(m_nrg, NRG_SIZE);
   in.read(m_ncg, NCG_SIZE);
   in.read(m_trg, FLOAT21_SIZE);
   in.read(m_tcg, FLOAT21_SIZE);
   in.read(m_grndd, GRNDD_SIZE);
   in.read(m_xuor, FLOAT21_SIZE);
   in.read(m_yuor, FLOAT21_SIZE);
   in.read(m_zuor, FLOAT21_SIZE);
   in.read(m_xuxr, FLOAT21_SIZE);
   in.read(m_xuyr, FLOAT21_SIZE);
   in.read(m_xuzr, FLOAT21_SIZE);
   in.read(m_yuxr, FLOAT21_SIZE);
   in.read(m_yuyr, FLOAT21_SIZE);
   in.read(m_yuzr, FLOAT21_SIZE);
   in.read(m_zuxr, FLOAT21_SIZE);
   in.read(m_zuyr, FLOAT21_SIZE);
   in.read(m_zuzr, FLOAT21_SIZE);
   in.read(m_v1x, FLOAT21_SIZE);
   in.read(m_v1y, FLOAT21_SIZE);
   in.read(m_v1z, FLOAT21_SIZE);
   in.read(m_v2x, FLOAT21_SIZE);
   in.read(m_v2y, FLOAT21_SIZE);
   in.read(m_v2z, FLOAT21_SIZE);
   in.read(m_v3x, FLOAT21_SIZE);
   in.read(m_v3y, FLOAT21_SIZE);
   in.read(m_v3z, FLOAT21_SIZE);
   in.read(m_v4x, FLOAT21_SIZE);
   in.read(m_v4y, FLOAT21_SIZE);
   in.read(m_v4z, FLOAT21_SIZE);
   in.read(m_v5x, FLOAT21_SIZE);
   in.read(m_v5y, FLOAT21_SIZE);
   in.read(m_v5z, FLOAT21_SIZE);
   in.read(m_v6x, FLOAT21_SIZE);
   in.read(m_v6y, FLOAT21_SIZE);
   in.read(m_v6z, FLOAT21_SIZE);
   in.read(m_v7x, FLOAT21_SIZE);
   in.read(m_v7y, FLOAT21_SIZE);
   in.read(m_v7z, FLOAT21_SIZE);
   in.read(m_v8x, FLOAT21_SIZE);
   in.read(m_v8y, FLOAT21_SIZE);
   in.read(m_v8z, FLOAT21_SIZE);
   in.read(m_grpx, FLOAT21_SIZE);
   in.read(m_grpy, FLOAT21_SIZE);
   in.read(m_grpz, FLOAT21_SIZE);
   in.read(m_fullr, FULL_SIZE);
   in.read(m_fullc, FULL_SIZE);
   in.read(m_minr, MIN_SIZE);
   in.read(m_maxr, MAX_SIZE);
   in.read(m_minc, MIN_SIZE);
   in.read(m_maxc, MAX_SIZE);
   in.read(m_ie0, FLOAT21_SIZE);
   in.read(m_ier, FLOAT21_SIZE);
   in.read(m_iec, FLOAT21_SIZE);
   in.read(m_ierr, FLOAT21_SIZE);
   in.read(m_ierc, FLOAT21_SIZE);
   in.read(m_iecc, FLOAT21_SIZE);
   in.read(m_ia0, FLOAT21_SIZE);
   in.read(m_iar, FLOAT21_SIZE);
   in.read(m_iac, FLOAT21_SIZE);
   in.read(m_iarr, FLOAT21_SIZE);
   in.read(m_iarc, FLOAT21_SIZE);
   in.read(m_iacc, FLOAT21_SIZE);
   in.read(m_spx, FLOAT21_SIZE);
   in.read(m_svx, FLOAT21_SIZE);
   in.read(m_sax, FLOAT21_SIZE);
   in.read(m_spy, FLOAT21_SIZE);
   in.read(m_svy, FLOAT21_SIZE);
   in.read(m_say, FLOAT21_SIZE);
   in.read(m_spz, FLOAT21_SIZE);
   in.read(m_svz, FLOAT21_SIZE);
   in.read(m_saz, FLOAT21_SIZE);
}

void ossimNitfRsmidaTag::writeStream(std::ostream& out)
{
   out.write(m_iid, IID_SIZE);
   out.write(m_edition, EDITION_SIZE);
   out.write(m_isid, ISID_SIZE);
   out.write(m_sid, SID_SIZE);
   out.write(m_stid, STID_SIZE);
   out.write(m_year, YEAR_SIZE);
   out.write(m_month, MONTH_SIZE);
   out.write(m_day, DAY_SIZE);
   out.write(m_hour, HOUR_SIZE);
   out.write(m_minute, MINUTE_SIZE);
   out.write(m_second, SECOND_SIZE);
   out.write(m_nrg, NRG_SIZE);
   out.write(m_ncg, NCG_SIZE);
   out.write(m_trg, FLOAT21_SIZE);
   out.write(m_tcg, FLOAT21_SIZE);
   out.write(m_grndd, GRNDD_SIZE);
   out.write(m_xuor, FLOAT21_SIZE);
   out.write(m_yuor, FLOAT21_SIZE);
   out.write(m_zuor, FLOAT21_SIZE);
   out.write(m_xuxr, FLOAT21_SIZE);
   out.write(m_xuyr, FLOAT21_SIZE);
   out.write(m_xuzr, FLOAT21_SIZE);
   out.write(m_yuxr, FLOAT21_SIZE);
   out.write(m_yuyr, FLOAT21_SIZE);
   out.write(m_yuzr, FLOAT21_SIZE);
   out.write(m_zuxr, FLOAT21_SIZE);
   out.write(m_zuyr, FLOAT21_SIZE);
   out.write(m_zuzr, FLOAT21_SIZE);
   out.write(m_v1x, FLOAT21_SIZE);
   out.write(m_v1y, FLOAT21_SIZE);
   out.write(m_v1z, FLOAT21_SIZE);
   out.write(m_v2x, FLOAT21_SIZE);
   out.write(m_v2y, FLOAT21_SIZE);
   out.write(m_v2z, FLOAT21_SIZE);
   out.write(m_v3x, FLOAT21_SIZE);
   out.write(m_v3y, FLOAT21_SIZE);
   out.write(m_v3z, FLOAT21_SIZE);
   out.write(m_v4x, FLOAT21_SIZE);
   out.write(m_v4y, FLOAT21_SIZE);
   out.write(m_v4z, FLOAT21_SIZE);
   out.write(m_v5x, FLOAT21_SIZE);
   out.write(m_v5y, FLOAT21_SIZE);
   out.write(m_v5z, FLOAT21_SIZE);
   out.write(m_v6x, FLOAT21_SIZE);
   out.write(m_v6y, FLOAT21_SIZE);
   out.write(m_v6z, FLOAT21_SIZE);
   out.write(m_v7x, FLOAT21_SIZE);
   out.write(m_v7y, FLOAT21_SIZE);
   out.write(m_v7z, FLOAT21_SIZE);
   out.write(m_v8x, FLOAT21_SIZE);
   out.write(m_v8y, FLOAT21_SIZE);
   out.write(m_v8z, FLOAT21_SIZE);
   out.write(m_grpx, FLOAT21_SIZE);
   out.write(m_grpy, FLOAT21_SIZE);
   out.write(m_grpz, FLOAT21_SIZE);
   out.write(m_fullr, FULL_SIZE);
   out.write(m_fullc, FULL_SIZE);
   out.write(m_minr, MIN_SIZE);
   out.write(m_maxr, MAX_SIZE);
   out.write(m_minc, MIN_SIZE);
   out.write(m_maxc, MAX_SIZE);
   out.write(m_ie0, FLOAT21_SIZE);
   out.write(m_ier, FLOAT21_SIZE);
   out.write(m_iec, FLOAT21_SIZE);
   out.write(m_ierr, FLOAT21_SIZE);
   out.write(m_ierc, FLOAT21_SIZE);
   out.write(m_iecc, FLOAT21_SIZE);
   out.write(m_ia0, FLOAT21_SIZE);
   out.write(m_iar, FLOAT21_SIZE);
   out.write(m_iac, FLOAT21_SIZE);
   out.write(m_iarr, FLOAT21_SIZE);
   out.write(m_iarc, FLOAT21_SIZE);
   out.write(m_iacc, FLOAT21_SIZE);
   out.write(m_spx, FLOAT21_SIZE);
   out.write(m_svx, FLOAT21_SIZE);
   out.write(m_sax, FLOAT21_SIZE);
   out.write(m_spy, FLOAT21_SIZE);
   out.write(m_svy, FLOAT21_SIZE);
   out.write(m_say, FLOAT21_SIZE);
   out.write(m_spz, FLOAT21_SIZE);
   out.write(m_svz, FLOAT21_SIZE);
   out.write(m_saz, FLOAT21_SIZE);
}

std::ostream& ossimNitfRsmidaTag::print(std::ostream& out,
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
       << pfx << std::setw(24) << "ISID:" << m_isid << "\n"
       << pfx << std::setw(24) << "SID:" << m_sid << "\n"
       << pfx << std::setw(24) << "STID:" << m_stid << "\n"
       << pfx << std::setw(24) << "YEAR:" << m_year << "\n"
       << pfx << std::setw(24) << "MONTH:" << m_month << "\n"
       << pfx << std::setw(24) << "DAY:" << m_day << "\n"
       << pfx << std::setw(24) << "HOUR:" << m_hour << "\n"
       << pfx << std::setw(24) << "MINUTE:" << m_minute << "\n"
       << pfx << std::setw(24) << "SECOND:" << m_second << "\n"
       << pfx << std::setw(24) << "NRG:" << m_nrg << "\n"
       << pfx << std::setw(24) << "NCG:" << m_ncg << "\n"
       << pfx << std::setw(24) << "TRG:" << m_trg << "\n"
       << pfx << std::setw(24) << "TCG:" << m_tcg << "\n"
       << pfx << std::setw(24) << "GRNDD:" << m_grndd << "\n"
       << pfx << std::setw(24) << "XUOR:" << m_xuor << "\n"
       << pfx << std::setw(24) << "YUOR:" << m_yuor << "\n"
       << pfx << std::setw(24) << "ZUOR:" << m_zuor << "\n"
       << pfx << std::setw(24) << "XUXR:" << m_xuxr << "\n"
       << pfx << std::setw(24) << "XUYR:" << m_xuyr << "\n"
       << pfx << std::setw(24) << "XUZR:" << m_xuzr << "\n"
       << pfx << std::setw(24) << "YUXR:" << m_yuxr << "\n"
       << pfx << std::setw(24) << "YUYR:" << m_yuyr << "\n"
       << pfx << std::setw(24) << "YUZR:" << m_yuzr << "\n"
       << pfx << std::setw(24) << "ZUXR:" << m_zuxr << "\n"
       << pfx << std::setw(24) << "ZUYR:" << m_zuyr << "\n"
       << pfx << std::setw(24) << "ZUZR:" << m_zuzr << "\n"
       << pfx << std::setw(24) << "V1X:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V1Y:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V1Z:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V2X:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V2Y:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V2Z:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V3X:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V3Y:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V3Z:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V4X:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V4Y:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V4Z:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V5X:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V5Y:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V5Z:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V6X:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V6Y:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V6Z:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V7X:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V7Y:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V7Z:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V8X:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V8Y:" << m_v1x << "\n"
       << pfx << std::setw(24) << "V8Z:" << m_v1x << "\n"
       << pfx << std::setw(24) << "GRPX:" << m_grpx << "\n"
       << pfx << std::setw(24) << "GRPY:" << m_grpy << "\n"
       << pfx << std::setw(24) << "GRPZ:" << m_grpz << "\n"
       << pfx << std::setw(24) << "FULLR:" << m_fullr << "\n"
       << pfx << std::setw(24) << "FULLC:" << m_fullc << "\n"
       << pfx << std::setw(24) << "MINR:" << m_minr << "\n"
       << pfx << std::setw(24) << "MAXR:" << m_maxr << "\n"
       << pfx << std::setw(24) << "MINC:" << m_minc << "\n"
       << pfx << std::setw(24) << "MAXC:" << m_maxc << "\n"
       << pfx << std::setw(24) << "IE0:" << m_ie0 << "\n"
       << pfx << std::setw(24) << "IER:" << m_ier << "\n"
       << pfx << std::setw(24) << "IEC:" << m_iec << "\n"
       << pfx << std::setw(24) << "IERR:" << m_ierr << "\n"
       << pfx << std::setw(24) << "IERC:" << m_ierc << "\n"
       << pfx << std::setw(24) << "IECC:" << m_iecc << "\n"
       << pfx << std::setw(24) << "IA0:" << m_ia0 << "\n"
       << pfx << std::setw(24) << "IAR:" << m_iar << "\n"
       << pfx << std::setw(24) << "IAC:" << m_iac << "\n"
       << pfx << std::setw(24) << "IARR:" << m_iarr << "\n"
       << pfx << std::setw(24) << "IARC:" << m_iarc << "\n"
       << pfx << std::setw(24) << "IACC:" << m_iacc << "\n"
       << pfx << std::setw(24) << "SPX:" << m_spx << "\n"
       << pfx << std::setw(24) << "SVX:" << m_svx << "\n"
       << pfx << std::setw(24) << "SAX:" << m_sax << "\n"
       << pfx << std::setw(24) << "SPY:" << m_spy << "\n"
       << pfx << std::setw(24) << "SVY:" << m_svy << "\n"
       << pfx << std::setw(24) << "SAY:" << m_say << "\n"
       << pfx << std::setw(24) << "SPZ:" << m_spz << "\n"
       << pfx << std::setw(24) << "SVZ:" << m_svz << "\n"
       << pfx << std::setw(24) << "SAZ:" << m_saz << "\n";

   return out;
}

void ossimNitfRsmidaTag::clearFields()
{
   memset(m_iid,' ', IID_SIZE);
   memset(m_edition, ' ', EDITION_SIZE);
   memset(m_isid, ' ', ISID_SIZE);
   memset(m_sid, ' ', SID_SIZE);
   memset(m_stid, ' ', STID_SIZE);
   memset(m_year, ' ', YEAR_SIZE);
   memset(m_month, ' ', MONTH_SIZE);
   memset(m_day, ' ', DAY_SIZE);
   memset(m_hour,' ', HOUR_SIZE);
   memset(m_minute, ' ', MINUTE_SIZE);
   memset(m_second, ' ', SECOND_SIZE);
   memset(m_nrg, ' ', NRG_SIZE);
   memset(m_ncg, ' ', NCG_SIZE);
   memset(m_trg, ' ', FLOAT21_SIZE);
   memset(m_tcg, ' ', FLOAT21_SIZE);
   memset(m_grndd, ' ', GRNDD_SIZE);
   memset(m_xuor, ' ', FLOAT21_SIZE);
   memset(m_yuor, ' ', FLOAT21_SIZE);
   memset(m_zuor, ' ', FLOAT21_SIZE);
   memset(m_xuxr, ' ', FLOAT21_SIZE);
   memset(m_xuyr, ' ', FLOAT21_SIZE);
   memset(m_xuzr, ' ', FLOAT21_SIZE);
   memset(m_yuxr, ' ', FLOAT21_SIZE);
   memset(m_yuyr, ' ', FLOAT21_SIZE);
   memset(m_yuzr, ' ', FLOAT21_SIZE);
   memset(m_zuxr, ' ', FLOAT21_SIZE);
   memset(m_zuyr, ' ', FLOAT21_SIZE);
   memset(m_zuzr, ' ', FLOAT21_SIZE);
   memset(m_v1x, ' ', FLOAT21_SIZE);
   memset(m_v1y, ' ', FLOAT21_SIZE);
   memset(m_v1z, ' ', FLOAT21_SIZE);
   memset(m_v2x, ' ', FLOAT21_SIZE);
   memset(m_v2y, ' ', FLOAT21_SIZE);
   memset(m_v2z, ' ', FLOAT21_SIZE);
   memset(m_v3x, ' ', FLOAT21_SIZE);
   memset(m_v3y, ' ', FLOAT21_SIZE);
   memset(m_v3z, ' ', FLOAT21_SIZE);
   memset(m_v4x, ' ', FLOAT21_SIZE);
   memset(m_v4y, ' ', FLOAT21_SIZE);
   memset(m_v4z, ' ', FLOAT21_SIZE);
   memset(m_v5x, ' ', FLOAT21_SIZE);
   memset(m_v5y, ' ', FLOAT21_SIZE);
   memset(m_v5z, ' ', FLOAT21_SIZE);
   memset(m_v6x, ' ', FLOAT21_SIZE);
   memset(m_v6y, ' ', FLOAT21_SIZE);
   memset(m_v6z, ' ', FLOAT21_SIZE);
   memset(m_v7x, ' ', FLOAT21_SIZE);
   memset(m_v7y, ' ', FLOAT21_SIZE);
   memset(m_v7z, ' ', FLOAT21_SIZE);
   memset(m_v8x, ' ', FLOAT21_SIZE);
   memset(m_v8y, ' ', FLOAT21_SIZE);
   memset(m_v8z, ' ', FLOAT21_SIZE);
   memset(m_grpx, ' ', FLOAT21_SIZE);
   memset(m_grpy, ' ', FLOAT21_SIZE);
   memset(m_grpz, ' ', FLOAT21_SIZE);
   memset(m_fullr, ' ', FULL_SIZE);
   memset(m_fullc, ' ', FULL_SIZE);
   memset(m_minr, ' ', MIN_SIZE);
   memset(m_maxr, ' ', MAX_SIZE);
   memset(m_minc, ' ', MIN_SIZE);
   memset(m_maxc, ' ', MAX_SIZE);
   memset(m_ie0, ' ', FLOAT21_SIZE);
   memset(m_ier, ' ', FLOAT21_SIZE);
   memset(m_iec, ' ', FLOAT21_SIZE);
   memset(m_ierr, ' ', FLOAT21_SIZE);
   memset(m_ierc, ' ', FLOAT21_SIZE);
   memset(m_iecc, ' ', FLOAT21_SIZE);
   memset(m_ia0, ' ', FLOAT21_SIZE);
   memset(m_iar, ' ', FLOAT21_SIZE);
   memset(m_iac, ' ', FLOAT21_SIZE);
   memset(m_iarr, ' ', FLOAT21_SIZE);
   memset(m_iarc, ' ', FLOAT21_SIZE);
   memset(m_iacc, ' ', FLOAT21_SIZE);
   memset(m_spx, ' ', FLOAT21_SIZE);
   memset(m_svx, ' ', FLOAT21_SIZE);
   memset(m_sax, ' ', FLOAT21_SIZE);
   memset(m_spy, ' ', FLOAT21_SIZE);
   memset(m_svy, ' ', FLOAT21_SIZE);
   memset(m_say, ' ', FLOAT21_SIZE);
   memset(m_spz, ' ', FLOAT21_SIZE);
   memset(m_svz, ' ', FLOAT21_SIZE);
   memset(m_saz, ' ', FLOAT21_SIZE);

   m_iid[IID_SIZE] = '\0';
   m_edition[EDITION_SIZE] = '\0';
   m_isid[ISID_SIZE] = '\0';
   m_sid[SID_SIZE] = '\0';
   m_stid[STID_SIZE] = '\0';
   m_year[YEAR_SIZE] = '\0';
   m_month[MONTH_SIZE] = '\0';
   m_day[DAY_SIZE] = '\0';
   m_hour[HOUR_SIZE] = '\0';
   m_minute[MINUTE_SIZE] = '\0';
   m_second[SECOND_SIZE] = '\0';
   m_nrg[NRG_SIZE] = '\0';
   m_ncg[NCG_SIZE] = '\0';
   m_trg[FLOAT21_SIZE] = '\0';
   m_tcg[FLOAT21_SIZE] = '\0';
   m_grndd[GRNDD_SIZE] = '\0';
   m_xuor[FLOAT21_SIZE] = '\0';
   m_yuor[FLOAT21_SIZE] = '\0';
   m_zuor[FLOAT21_SIZE] = '\0';
   m_xuxr[FLOAT21_SIZE] = '\0';
   m_xuyr[FLOAT21_SIZE] = '\0';
   m_xuzr[FLOAT21_SIZE] = '\0';
   m_yuxr[FLOAT21_SIZE] = '\0';
   m_yuyr[FLOAT21_SIZE] = '\0';
   m_yuzr[FLOAT21_SIZE] = '\0';
   m_zuxr[FLOAT21_SIZE] = '\0';
   m_zuyr[FLOAT21_SIZE] = '\0';
   m_zuzr[FLOAT21_SIZE] = '\0';
   m_v1x[FLOAT21_SIZE] = '\0';
   m_v1y[FLOAT21_SIZE] = '\0';
   m_v1z[FLOAT21_SIZE] = '\0';
   m_v2x[FLOAT21_SIZE] = '\0';
   m_v2y[FLOAT21_SIZE] = '\0';
   m_v2z[FLOAT21_SIZE] = '\0';
   m_v3x[FLOAT21_SIZE] = '\0';
   m_v3y[FLOAT21_SIZE] = '\0';
   m_v3z[FLOAT21_SIZE] = '\0';
   m_v4x[FLOAT21_SIZE] = '\0';
   m_v4y[FLOAT21_SIZE] = '\0';
   m_v4z[FLOAT21_SIZE] = '\0';
   m_v5x[FLOAT21_SIZE] = '\0';
   m_v5y[FLOAT21_SIZE] = '\0';
   m_v5z[FLOAT21_SIZE] = '\0';
   m_v6x[FLOAT21_SIZE] = '\0';
   m_v6y[FLOAT21_SIZE] = '\0';
   m_v6z[FLOAT21_SIZE] = '\0';
   m_v7x[FLOAT21_SIZE] = '\0';
   m_v7y[FLOAT21_SIZE] = '\0';
   m_v7z[FLOAT21_SIZE] = '\0';
   m_v8x[FLOAT21_SIZE] = '\0';
   m_v8y[FLOAT21_SIZE] = '\0';
   m_v8z[FLOAT21_SIZE] = '\0';
   m_grpx[FLOAT21_SIZE] = '\0';
   m_grpy[FLOAT21_SIZE] = '\0';
   m_grpz[FLOAT21_SIZE] = '\0';
   m_fullr[FULL_SIZE] = '\0';
   m_fullc[FULL_SIZE] = '\0';
   m_minr[MIN_SIZE] = '\0';
   m_maxr[MAX_SIZE] = '\0';
   m_minc[MIN_SIZE] = '\0';
   m_maxc[MAX_SIZE] = '\0';
   m_ie0[FLOAT21_SIZE] = '\0';
   m_ier[FLOAT21_SIZE] = '\0';
   m_iec[FLOAT21_SIZE] = '\0';
   m_ierr[FLOAT21_SIZE] = '\0';
   m_ierc[FLOAT21_SIZE] = '\0';
   m_iecc[FLOAT21_SIZE] = '\0';
   m_ia0[FLOAT21_SIZE] = '\0';
   m_iar[FLOAT21_SIZE] = '\0';
   m_iac[FLOAT21_SIZE] = '\0';
   m_iarr[FLOAT21_SIZE] = '\0';
   m_iarc[FLOAT21_SIZE] = '\0';
   m_iacc[FLOAT21_SIZE] = '\0';
   m_spx[FLOAT21_SIZE] = '\0';
   m_svx[FLOAT21_SIZE] = '\0';
   m_sax[FLOAT21_SIZE] = '\0';
   m_spy[FLOAT21_SIZE] = '\0';
   m_svy[FLOAT21_SIZE] = '\0';
   m_say[FLOAT21_SIZE] = '\0';
   m_spz[FLOAT21_SIZE] = '\0';
   m_svz[FLOAT21_SIZE] = '\0';
   m_saz[FLOAT21_SIZE] = '\0';
}

ossimString ossimNitfRsmidaTag::getIid() const
{
   return ossimString(m_iid);
}

ossimString ossimNitfRsmidaTag::getEdition() const
{
   return ossimString(m_edition);
}

ossimString ossimNitfRsmidaTag::getIsid() const
{
   return ossimString(m_isid);
}

ossimString ossimNitfRsmidaTag::getSid() const
{
   return ossimString(m_sid);
}

ossimString ossimNitfRsmidaTag::getStid() const
{
   return ossimString(m_stid);
}

ossimString ossimNitfRsmidaTag::getYear() const
{
   return ossimString(m_year);
}

ossimString ossimNitfRsmidaTag::getMonth() const
{
   return ossimString(m_month);
}

ossimString ossimNitfRsmidaTag::getDay() const
{
   return ossimString(m_day);
}

ossimString ossimNitfRsmidaTag::getHour() const
{
   return ossimString(m_hour);
}

ossimString ossimNitfRsmidaTag::getMinute() const
{
   return ossimString(m_minute);
}

ossimString ossimNitfRsmidaTag::getSecond() const
{
   return ossimString(m_second);
}

ossimString ossimNitfRsmidaTag::getNrg() const
{
   return ossimString(m_nrg);
}

ossimString ossimNitfRsmidaTag::getNcg() const
{
   return ossimString(m_ncg);
}

ossimString ossimNitfRsmidaTag::getTrg() const
{
   return ossimString(m_trg);
}

ossimString ossimNitfRsmidaTag::getTcg() const
{
   return ossimString(m_tcg);
}

ossimString ossimNitfRsmidaTag::getGrndd() const
{
   return ossimString(m_grndd);
}
   
ossimString ossimNitfRsmidaTag::getXuor() const
{
   return ossimString(m_xuor);
}

ossimString ossimNitfRsmidaTag::getYuor() const
{
   return ossimString(m_yuor);
}

ossimString ossimNitfRsmidaTag::getZuor() const
{
   return ossimString(m_zuor);
}

ossimString ossimNitfRsmidaTag::getXuxr() const
{
   return ossimString(m_xuxr);
}

ossimString ossimNitfRsmidaTag::getXuyr() const
{
   return ossimString(m_xuyr);
}

ossimString ossimNitfRsmidaTag::getXuzr() const
{
   return ossimString(m_xuzr);
}

ossimString ossimNitfRsmidaTag::getYuxr() const
{
   return ossimString(m_yuxr);
}

ossimString ossimNitfRsmidaTag::getYuyr() const
{
   return ossimString(m_yuyr);
}

ossimString ossimNitfRsmidaTag::getYuzr() const
{
   return ossimString(m_yuzr);
}

ossimString ossimNitfRsmidaTag::getZuxr() const
{
   return ossimString(m_zuxr);
}

ossimString ossimNitfRsmidaTag::getZuyr() const
{
   return ossimString(m_zuyr);
}

ossimString ossimNitfRsmidaTag::getZuzr() const
{
   return ossimString(m_zuzr);
}
   
ossimString ossimNitfRsmidaTag::getV1x() const
{
   return ossimString(m_v1x);
}

ossimString ossimNitfRsmidaTag::getV1y() const
{
   return ossimString(m_v1y);
}

ossimString ossimNitfRsmidaTag::getV1z() const
{
   return ossimString(m_v1z);
}

ossimString ossimNitfRsmidaTag::getV2x() const
{
   return ossimString(m_v2x);
}

ossimString ossimNitfRsmidaTag::getV2y() const
{
   return ossimString(m_v2y);
}

ossimString ossimNitfRsmidaTag::getV2z() const
{
   return ossimString(m_v2z);
}

ossimString ossimNitfRsmidaTag::getV3x() const
{
   return ossimString(m_v3x);
}

ossimString ossimNitfRsmidaTag::getV3y() const
{
   return ossimString(m_v3y);
}

ossimString ossimNitfRsmidaTag::getV3z() const
{
   return ossimString(m_v3z);
}

ossimString ossimNitfRsmidaTag::getV4x() const
{
   return ossimString(m_v4x);
}

ossimString ossimNitfRsmidaTag::getV4y() const
{
   return ossimString(m_v4y);
}

ossimString ossimNitfRsmidaTag::getV4z() const
{
   return ossimString(m_v4z);
}

ossimString ossimNitfRsmidaTag::getV5x() const
{
   return ossimString(m_v5x);
}

ossimString ossimNitfRsmidaTag::getV5y() const
{
   return ossimString(m_v5y);
}

ossimString ossimNitfRsmidaTag::getV5z() const
{
   return ossimString(m_v5z);
}

ossimString ossimNitfRsmidaTag::getV6x() const
{
   return ossimString(m_v6x);
}

ossimString ossimNitfRsmidaTag::getV6y() const
{
   return ossimString(m_v6y);
}

ossimString ossimNitfRsmidaTag::getV6z() const
{
   return ossimString(m_v6z);
}

ossimString ossimNitfRsmidaTag::getV7x() const
{
   return ossimString(m_v7x);
}

ossimString ossimNitfRsmidaTag::getV7y() const
{
   return ossimString(m_v7y);
}

ossimString ossimNitfRsmidaTag::getV7z() const
{
   return ossimString(m_v7z);
}

ossimString ossimNitfRsmidaTag::getV8x() const
{
   return ossimString(m_v8x);
}

ossimString ossimNitfRsmidaTag::getV8y() const
{
   return ossimString(m_v8y);
}

ossimString ossimNitfRsmidaTag::getV8z() const
{
   return ossimString(m_v8z);
}
   
ossimString ossimNitfRsmidaTag::getGrpx() const
{
   return ossimString(m_grpx);
}

ossimString ossimNitfRsmidaTag::getGrpy() const
{
   return ossimString(m_grpy);
}

ossimString ossimNitfRsmidaTag::getGrpz() const
{
   return ossimString(m_grpz);
}

ossimString ossimNitfRsmidaTag::getFullr() const
{
   return ossimString(m_fullr);
}

ossimString ossimNitfRsmidaTag::getFullc() const
{
   return ossimString(m_fullc);
}

ossimString ossimNitfRsmidaTag::getMinr() const
{
   return ossimString(m_minr);
}

ossimString ossimNitfRsmidaTag::getMaxr() const
{
   return ossimString(m_maxr);
}

ossimString ossimNitfRsmidaTag::getMinc() const
{
   return ossimString(m_minc);
}

ossimString ossimNitfRsmidaTag::getMaxc() const
{
   return ossimString(m_maxc);
}

ossimString ossimNitfRsmidaTag::getIe0() const
{
   return ossimString(m_ie0);
}

ossimString ossimNitfRsmidaTag::getIer() const
{
   return ossimString(m_ier);
}

ossimString ossimNitfRsmidaTag::getIec() const
{
   return ossimString(m_iec);
}

ossimString ossimNitfRsmidaTag::getIerr() const
{
   return ossimString(m_ierr);
}

ossimString ossimNitfRsmidaTag::getIerc() const
{
   return ossimString(m_ierc);
}

ossimString ossimNitfRsmidaTag::getIecc() const
{
   return ossimString(m_iecc);
}

ossimString ossimNitfRsmidaTag::getIa0() const
{
   return ossimString(m_ia0);
}

ossimString ossimNitfRsmidaTag::getIar() const
{
   return ossimString(m_iar);
}

ossimString ossimNitfRsmidaTag::getIac() const
{
   return ossimString(m_iac);
}

ossimString ossimNitfRsmidaTag::getIarr() const
{
   return ossimString(m_iarr);
}

ossimString ossimNitfRsmidaTag::getIarc() const
{
   return ossimString(m_iarc);
}

ossimString ossimNitfRsmidaTag::getIacc() const
{
   return ossimString(m_iacc);
}

ossimString ossimNitfRsmidaTag::getSpx() const
{
   return ossimString(m_spx);
}

ossimString ossimNitfRsmidaTag::getSvx() const
{
   return ossimString(m_svx);
}

ossimString ossimNitfRsmidaTag::getSax() const
{
   return ossimString(m_sax);
}

ossimString ossimNitfRsmidaTag::getSpy() const
{
   return ossimString(m_spy);
}

ossimString ossimNitfRsmidaTag::getSvy() const
{
   return ossimString(m_svy);
}

ossimString ossimNitfRsmidaTag::getSay() const
{
   return ossimString(m_say);
}

ossimString ossimNitfRsmidaTag::getSpz() const
{
   return ossimString(m_spz);
}

ossimString ossimNitfRsmidaTag::getSvz() const
{
   return ossimString(m_svz);
}

ossimString ossimNitfRsmidaTag::getSaz() const
{
   return ossimString(m_saz);
}


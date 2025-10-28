//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSEPHB des class definition.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.6 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfCsephbDes.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>
#include <ossim/support_data/ossimNitfCommon.h>

static ossimTrace traceDebug("ossimNitfCsephbdes:debug");

const std::string ossimNitfCsephbDes::CETAG_KW = "CSEPHB";

static const int FOREVER = 1;


ossimNitfCsephbDes::ossimNitfCsephbDes()
   : ossimNitfGenericDes("CSEPHB")
{
   // traceDebug.setTraceFlag(true);

   initializeFieldDefinitions();
   initializeFields();
   setDesDataLength(computeDesLength());
}

ossimNitfCsephbDes::ossimNitfCsephbDes(ossim_uint32 tagLength)
   : ossimNitfGenericDes("CSEPHB", tagLength)
{
   initializeFieldDefinitions();
   initializeFields();
}
void ossimNitfCsephbDes::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
      {
         {"DE", 2, 0},
         {"DESID", 25, 0},
         {"DESVER", 2, 1},
         {"DESCLAS to DESCTLN", 167, 0, 0, "0"},
         {"DESSHL", 4, 1},
         {"UUID", 36, 0, 0, " "},
         {"NUMAIS", 3, 0},
         {"NUMAIS n", LOOP_START},
            {"AISDLVLn", 3, 1},
         {"NUMAIS n", LOOP_END},
         {"NUM_ASSOC_ELEM", 3, 1},
         {"NUM_ASSOC_ELEM n", LOOP_START},
            {"ASSOC_ELEM_UUIDn", 36, 0},
         {"NUM_ASSOC_ELEM n", LOOP_END},
         {"RESERVEDSUBH_LEN", 4, 1},
         {"RESERVEDSUBH", 4, 1},
         {"QUAL_FLAG_EPH", 1, 1},
         {"INTERP_TYPE_EPH", 1, 1},
         {"INTERP_TYPE_EPH 2 =", IF_STATEMENT_START},
            {"INTERP_ORDER_EPH", 1, 1},
         {"INTERP_TYPE_EPH 2 =", IF_STATEMENT_END},
         {"EPHEM_FLAG", 1, 1},
         {"ECI_ECF_EPHEM", 1, 1},
         {"ECI_ECF_EPHEM 0 = DESVER 1 > &", IF_STATEMENT_START},
            {"TA_POLE", 19, 3, 11},
            {"A_POLE", 11, 4, 8},
            {"B_POLE", 11, 4, 8},
            {"CJ1_POLE", 11, 4, 8},
            {"CJ2_POLE", 11, 4, 8},
            {"DJ1_POLE", 11, 4, 8},
            {"DJ2_POLE", 11, 4, 8},
            {"PJ1_POLE", 10, 3, 6},
            {"PJ2_POLE", 10, 3, 6},
            {"E_POLE", 11, 4, 8},
            {"F_POLE", 11, 4, 8},
            {"GK1_POLE", 11, 4, 8},
            {"GK2_POLE", 11, 4, 8},
            {"HK1_POLE", 11, 4, 8},
            {"HK2_POLE", 11, 4, 8},
            {"PK1_POLE", 10, 3, 6},
            {"PK2_POLE", 10, 3, 6},
            {"TB_UT", 19, 3, 11},
            {"I_UT", 12, 4, 9},
            {"J_UT", 12, 4, 9},
            {"KN1_UT", 12, 1},
            {"KN2_UT", 12, 1},
            {"KN3_UT", 12, 1},
            {"KN4_UT", 12, 1},
            {"LN1_UT", 12, 4, 9},
            {"LN2_UT", 12, 4, 9},
            {"LN3_UT", 12, 4, 9},
            {"LN4_UT", 12, 4, 9},
            {"PN1_UT", 10, 3, 6},
            {"PN2_UT", 10, 3, 6},
            {"PN3_UT", 10, 3, 6},
            {"PN4_UT", 10, 3, 6},
         {"ECI_ECF_EPHEM 0 = DESVER  1 > &", IF_STATEMENT_END},
         {"DT_EPHEM", 13, 3, 9},
         {"DATE_EPHEM", 8, 1},
         {"T0_EPHEM", 16, 1},
         {"NUM_EPHEM", 5, 1},
         {"NUM_EPHEM n", LOOP_START},
            {"EPHEM_Xn", 12, 4, 2},
            {"EPHEM_Yn", 12, 4, 2},
            {"EPHEM_Zn", 12, 4, 2},
         {"NUM_EPHEM n", LOOP_END},
         {"RESERVED_LEN", 9, 1},
         {"MASK_LEN", 2, 1},
         {"RESERVED_FIELD_MASK MASK_LEN", VARIABLE_LENGTH, 0},
         {"RESERVED_LEN_AREA1", 9, 1},
         {"ACCEL_PROVIDED", 1, 0},
         {"NUM_EPHEM n", LOOP_START},
            {"VEL_Xn", 12, 4, 2},
            {"VEL_Yn", 12, 4, 2},
            {"VEL_Zn", 12, 4, 2},
            {"ACCEL_PROVIDED 'Y' =", IF_STATEMENT_START},
               {"ACCEL_Xn", 12, 4, 2},
               {"ACCEL_Yn", 12, 4, 2},
               {"ACCEL_Zn", 12, 4, 2},
            {"ACCEL_PROVIDED 'Y' =", IF_STATEMENT_END},
         {"NUM_EPHEM n", LOOP_END}
      };
}

ossimString ossimNitfCsephbDes::getClassName() const
{
   return ossimString("ossimNitfCsephbdes");
}

// bool ossimNitfCsephbDes::loadState(const ossimKeywordlist& kwl, const char* prefix)
// {
//    static const char MODULE[] = "ossimNitfCsephDes::loadState(...)";
//    if (traceDebug())
//    {
//       ossimNotify(ossimNotifyLevel_DEBUG)
//          << MODULE << " entered...\n"
//          << "kwl:\n" << kwl << "\n"
//          << "prefix: " << (prefix?prefix:"null") << "\n";
//    }
//
//    bool status = true;
//    std::string pfx = prefix?prefix:"";
//    std::string key;
//    std::string k;
//    std::string value;
//    std::string os;
//    char buf[64];
//    std::string s;
//    ossim_uint32 count;
//    ossim_uint32 i;
//    ossim_uint32 subHdrBytes = 0;
//    ossim_uint32 desBytes = 0;
//
//    while(FOREVER) // Break on error or at end.
//    {
//       value = kwl.findKey(pfx, ossim::nitf::UUID_KW);
//       if (value.size())
//       {
//          ossimNitfCommon::setField(m_uuid,
//                                    ossimString(value),
//                                    UUID_SZ,
//                                    std::ios::left,
//                                    ' ');
//       }
//       value = kwl.findKey(pfx, "NUMAIS");
//       if (value.size())
//       {
//          ossimNitfCommon::setField(m_numais,
//                                    ossimString(value),
//                                    NUMAIS_SZ,
//                                    std::ios::right,
//                                    '0');
//       }
//       count = getNumberAis();
//       if (count > 0)
//       {
//          key = "AISDLVL";
//          m_aisdlvl.resize(count);
//          for(i = 0; i < count; ++i)
//          {
//             k = key + ossimString::toString(i).string();
//             value = kwl.findKey(pfx, k);
//             if (value.size())
//             {
//                ossimNitfCommon::setField(buf,
//                                          ossimString(value),
//                                          AISDLVL_SZ,
//                                          std::ios::right,
//                                          '0');
//                buf[AISDLVL_SZ] = '\0';
//                m_aisdlvl[i] = buf;
//             }
// #if 0 /* Not sure if this should be an error and break out? drb */
//             else // error... ??? )
//             {
//                ossimNotify(ossimNotifyLevel_WARN)
//                   << MODULE << " WARNING:\n" << "Missing key: " << k << std::endl;
//                status = false;
//                break;
//             }
// #endif
//          }
//
//       }
//
//       value = kwl.findKey(pfx, "NUM_ASSOC_ELEM");
//       if (value.size())
//       {
//          ossimNitfCommon::setField(m_num_assoc_elem,
//                                    ossimString(value),
//                                    NUM_ASSOC_ELEM_SZ,
//                                    std::ios::right,
//                                    '0');
//       }
//       count = getNumberAssocElem();
//       if (count > 0)
//       {
//          key = "ASSOC_ELEM_UUID";
//          m_assoc_elem_uuid.resize(count);
//          for(i = 0; i < count; ++i)
//          {
//             k = key + ossimString::toString(i).string();
//             value = kwl.findKey(pfx, k);
//             if (value.size())
//             {
//                ossimNitfCommon::setField(buf,
//                                          ossimString(value),
//                                          UUID_SZ,
//                                          std::ios::right,
//                                          '0');
//                buf[UUID_SZ] = '\0';
//                m_assoc_elem_uuid[i] = buf;
//             }
//          }
//
//          // m_reservedsubh_len not loaded
//       }
//       // End sub header:
//
//       // Start of DES data:
//       value = kwl.findKey(pfx, "QUAL_FLAG_ATT");
//       if (value.size())
//       {
//          m_qual_flag_att[0] = (ossimString(value).toBool()?'1':'0');
//       }
//       value = kwl.findKey(pfx, "INTERP_TYPE_ATT");
//       if (value.size())
//       {
//          ossimNitfCommon::setField(m_interp_type_att,
//                                    ossimString(value),
//                                    B1_SZ,
//                                    std::ios::right,
//                                    '0');
//       }
//       s = get_interp_type_att();
//       if (s == "2" || s == "3")
//       {
//          value = kwl.findKey(pfx, "INTERP_ORDER_ATT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_interp_order_att,
//                                       ossimString(value),
//                                       B1_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//       }
//       value = kwl.findKey(pfx, "ATT_TYPE");
//       if (value.size())
//       {
//          ossimNitfCommon::setField(m_att_type,
//                                    ossimString(value),
//                                    B1_SZ,
//                                    std::ios::right,
//                                    '0');
//       }
//       value = kwl.findKey(pfx, "ECI_ECF_ATT");
//       if (value.size())
//       {
//          ossimNitfCommon::setField(m_eci_ecf_att,
//                                    ossimString(value),
//                                    B1_SZ,
//                                    std::ios::right,
//                                    '0');
//       }
//
//       if (m_eci_ecf_att[0] == '0' &&  getDesVersionNumber() >= 2)
//       {
//          value = kwl.findKey(pfx, "TA_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_ta_pole,
//                                       ossimString(value),
//                                       B19_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "A_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_a_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "B_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_b_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "CJ1_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_cj1_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "CJ2_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_cj2_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "DJ1_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_dj1_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "DJ2_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_dj2_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "PJ1_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_pj1_pole,
//                                       ossimString(value),
//                                       B10_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "PJ2_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_pj2_pole,
//                                       ossimString(value),
//                                       B10_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "E_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_e_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "F_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_f_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "GK1_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_gk1_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "GK2_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_gk2_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "HK1_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_hk1_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "HK2_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_hk2_pole,
//                                       ossimString(value),
//                                       B11_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "PK1_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_pk1_pole,
//                                       ossimString(value),
//                                       B10_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "PK2_POLE");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_pk2_pole,
//                                       ossimString(value),
//                                       B10_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "TB_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_tb_ut,
//                                       ossimString(value),
//                                       B19_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "I_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_i_ut,
//                                       ossimString(value),
//                                       B12_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "J_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_j_ut,
//                                       ossimString(value),
//                                       B12_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "KN1_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_kn1_ut,
//                                       ossimString(value),
//                                       B12_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "KN2_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_kn2_ut,
//                                       ossimString(value),
//                                       B12_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "KN3_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_kn3_ut,
//                                       ossimString(value),
//                                       B12_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "KN4_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_kn4_ut,
//                                       ossimString(value),
//                                       B12_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "LN1_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_ln1_ut,
//                                       ossimString(value),
//                                       B12_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "LN2_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_ln2_ut,
//                                       ossimString(value),
//                                       B12_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "LN3_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_ln3_ut,
//                                       ossimString(value),
//                                       B12_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "LN4_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_ln4_ut,
//                                       ossimString(value),
//                                       B12_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "PN1_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_pn1_ut,
//                                       ossimString(value),
//                                       B10_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "PN2_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_pn2_ut,
//                                       ossimString(value),
//                                       B10_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "PN3_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_pn3_ut,
//                                       ossimString(value),
//                                       B10_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//          value = kwl.findKey(pfx, "PN4_UT");
//          if (value.size())
//          {
//             ossimNitfCommon::setField(m_pn4_ut,
//                                       ossimString(value),
//                                       B10_SZ,
//                                       std::ios::right,
//                                       '0');
//          }
//
//       } // matches: if (m_eci_ecf_att[0] == '0' &&  getDesVersionNumber() >= 2)
//
//       value = kwl.findKey(pfx, "DT_ATT");
//       if (value.size())
//       {
//          ossimNitfCommon::setField(m_dt_att,
//                                    ossimString(value),
//                                    B13_SZ,
//                                    std::ios::right,
//                                    '0');
//       }
//       value = kwl.findKey(pfx, "DATE_ATT");
//       if (value.size())
//       {
//          ossimNitfCommon::setField(m_date_att,
//                                    ossimString(value),
//                                    B8_SZ,
//                                    std::ios::right,
//                                    '0');
//       }
//       value = kwl.findKey(pfx, "T0_ATT");
//       if (value.size())
//       {
//          ossimNitfCommon::setField(m_t0_att,
//                                    ossimString(value),
//                                    B16_SZ,
//                                    std::ios::right,
//                                    '0');
//       }
//       value = kwl.findKey(pfx, "NUM_ATT");
//       if (value.size())
//       {
//          ossimNitfCommon::setField(m_num_att,
//                                    ossimString(value),
//                                    B5_SZ,
//                                    std::ios::right,
//                                    '0');
//       }
//       count = getNumberAtt();
//       if (count > 0)
//       {
//          m_q1.resize(count);
//          m_q2.resize(count);
//          m_q3.resize(count);
//          m_q4.resize(count);
//          for(i = 0; i < count; ++i)
//          {
//             key = "Q1_";
//             k = key + ossimString::toString(i).string();
//             value = kwl.findKey(pfx, k);
//             if (value.size())
//             {
//                ossimNitfCommon::setField(buf,
//                                          ossimString(value),
//                                          B18_SZ,
//                                          std::ios::right,
//                                          '0');
//                buf[B18_SZ] = '\0';
//                m_q1[i] = buf;
//             }
//             key = "Q2_";
//             k = key + ossimString::toString(i).string();
//             value = kwl.findKey(pfx, k);
//             if (value.size())
//             {
//                ossimNitfCommon::setField(buf,
//                                          ossimString(value),
//                                          B18_SZ,
//                                          std::ios::right,
//                                          '0');
//                buf[B18_SZ] = '\0';
//                m_q2[i] = buf;
//             }
//             key = "Q3_";
//             k = key + ossimString::toString(i).string();
//             value = kwl.findKey(pfx, k);
//             if (value.size())
//             {
//                ossimNitfCommon::setField(buf,
//                                          ossimString(value),
//                                          B18_SZ,
//                                          std::ios::right,
//                                          '0');
//                buf[B18_SZ] = '\0';
//                m_q3[i] = buf;
//             }
//             key = "Q4_";
//             k = key + ossimString::toString(i).string();
//             value = kwl.findKey(pfx, k);
//             if (value.size())
//             {
//                ossimNitfCommon::setField(buf,
//                                          ossimString(value),
//                                          B18_SZ,
//                                          std::ios::right,
//                                          '0');
//                buf[B18_SZ] = '\0';
//                m_q4[i] = buf;
//             }
//          }
//
//       } // matches: count = getNumberAtt(); if (count > 0){
//
//       // m_reserved_len not loaded
//
//       break; // Trailing break from forever loop.
//
//    } // Matches: while(FOREVER)
//
//    // Set the lengths in base ossimNitfRegisteredDes:
//    setDesSubHeaderLength(computeDesSubHeaderLength());
//    setDesDataLength(computeDesDataLength());
//
//    if (traceDebug())
//    {
//       ossimNotify(ossimNotifyLevel_DEBUG)
//          << MODULE << " exit status: " << (status?"true":"false") << "\n";
//    }
//
//    return status;
// }
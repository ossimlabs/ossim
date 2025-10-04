//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSATTB des class definition.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.6 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfCsattbDes.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>
#include <iomanip>
#include <iostream>


static ossimTrace traceDebug("ossimNitfCsattbDes:debug");

const std::string ossimNitfCsattbDes::DESID = "CSATTB";

ossimNitfCsattbDes::ossimNitfCsattbDes()
   : ossimNitfRegisteredDes(),
     m_uuid(),
     m_numais(),
     m_aisdlvl(),
     m_num_assoc_elem(),
     m_assoc_elem_uuid(),
     m_reservedsubh_len(), // End DES sub header:
     
     m_qual_flag_att(), // Strart DES data:
     m_interp_type_att(),
     m_interp_order_att(),
     m_att_type(),
     m_ecf_eci_att(),
     m_ta_pole(),
     m_a_pole(),
     m_b_pole(),
     m_cj1_pole(),
     m_cj2_pole(),
     m_dj1_pole(),
     m_dj2_pole(),
     m_pj1_pole(),   
     m_pj2_pole(),
     m_e_pole(),
     m_f_pole(),
     m_gk1_pole(),   
     m_gk2_pole(),
     m_hk1_pole(),
     m_hk2_pole(),   
     m_pk1_pole(),
     m_pk2_pole(),   
     m_tb_ut(),
     m_i_ut(),
     m_j_ut(),
     m_kn1_ut(),
     m_kn2_ut(),
     m_kn3_ut(),
     m_kn4_ut(),
     m_ln1_ut(),
     m_ln2_ut(),
     m_ln3_ut(),
     m_ln4_ut(),
     m_pn1_ut(),
     m_pn2_ut(),
     m_pn3_ut(),
     m_pn4_ut(),
     m_dt_att(),
     m_date_att(),
     m_t0_att(),
     m_num_att(),
     m_q1(),
     m_q2(),
     m_q3(),
     m_q4(),
     m_reserved_len()
{
   // traceDebug.setTraceFlag(true);
   
   clearFields();
   setDesName(DESID);
}

ossimNitfCsattbDes::ossimNitfCsattbDes(ossim_uint32 tagLength)
   : ossimNitfRegisteredDes(DESID, tagLength),
     m_uuid(),
     m_numais(),
     m_aisdlvl(),
     m_num_assoc_elem(),
     m_assoc_elem_uuid(),
     m_reservedsubh_len(), // End DES sub header:
     
     m_qual_flag_att(), // Strart DES data:
     m_interp_type_att(),
     m_interp_order_att(),
     m_att_type(),
     m_ecf_eci_att(),
     m_ta_pole(),
     m_a_pole(),
     m_b_pole(),
     m_cj1_pole(),
     m_cj2_pole(),
     m_dj1_pole(),
     m_dj2_pole(),
     m_pj1_pole(),   
     m_pj2_pole(),
     m_e_pole(),
     m_f_pole(),
     m_gk1_pole(),   
     m_gk2_pole(),
     m_hk1_pole(),
     m_hk2_pole(),   
     m_pk1_pole(),
     m_pk2_pole(),   
     m_tb_ut(),
     m_i_ut(),
     m_j_ut(),
     m_kn1_ut(),
     m_kn2_ut(),
     m_kn3_ut(),
     m_kn4_ut(),
     m_ln1_ut(),
     m_ln2_ut(),
     m_ln3_ut(),
     m_ln4_ut(),
     m_pn1_ut(),
     m_pn2_ut(),
     m_pn3_ut(),
     m_pn4_ut(),
     m_dt_att(),
     m_date_att(),
     m_t0_att(),
     m_num_att(),
     m_q1(),
     m_q2(),
     m_q3(),
     m_q4(),
     m_reserved_len()
{
   clearFields();
}

ossimString ossimNitfCsattbDes::getClassName() const
{
   return ossimString("ossimNitfCsattbDes");
}

void ossimNitfCsattbDes::parseStream(std::istream& in)
{
}

void ossimNitfCsattbDes::writeStream(std::ostream& out)
{
}

std::string ossimNitfCsattbDes::get_uuid() const
{
   return std::string(m_uuid);
}

std::string ossimNitfCsattbDes::get_numais() const
{
   return std::string(m_numais);
}

ossim_uint32 ossimNitfCsattbDes::getNumberAis() const
{
   return ossimString(m_numais).toUInt32();
}

bool ossimNitfCsattbDes::get_aisdlv(ossim_uint32 index, std::string& aisdlvl) const
{
   bool status = true;
   if (index < getNumberAis() && index < m_aisdlvl.size())
   {
      aisdlvl = m_aisdlvl[index];
   }
   else
   {
      status = false;
   }
   return status;
}

std::string ossimNitfCsattbDes::get_num_assoc_elem() const
{
   return std::string(m_num_assoc_elem);
}

ossim_uint32 ossimNitfCsattbDes::getNumberAssocElem() const
{
   return ossimString(m_num_assoc_elem).toUInt32();
}

bool ossimNitfCsattbDes::get_assoc_elem_uuid(ossim_uint32 index, std::string& uuid) const
{
   bool status = true;
   if (index < getNumberAssocElem() && index < m_assoc_elem_uuid.size())
   {
      uuid = m_assoc_elem_uuid[index];
   }
   else
   {
      status = false;
   }
   return status;
}

std::string ossimNitfCsattbDes::get_reservedsubh_len() const
{
   m_reservedsubh_len;
}

ossim_uint32 ossimNitfCsattbDes::getReserveSubHdrLength() const
{
   return ossimString(m_reservedsubh_len).toUInt32();
}

ossim_uint32 ossimNitfCsattbDes::computeDesSubHeaderLength() const
{
   ossim_uint32 length = 0; // tmp drb 36 + 3
   return length;
}

void ossimNitfCsattbDes::setDesSubHeaderLength(ossim_uint32 length)
{
   
}

void ossimNitfCsattbDes::clearFields()
{
   memset(m_uuid, ' ', UUID_SZ);
   memset(m_numais, ' ', NUMAIS_SZ);
   memset(m_num_assoc_elem, ' ', NUM_ASSOC_ELEM_SZ);
   memset(m_reservedsubh_len, '0', RESERVERDSHUBH_LEN_SZ);
   memset(m_qual_flag_att, ' ', B1_SZ);
   memset(m_interp_type_att, ' ', B1_SZ);
   memset(m_interp_order_att, ' ', B1_SZ);
   memset(m_att_type, ' ', B1_SZ);
   memset(m_ecf_eci_att, ' ', B1_SZ);
   memset(m_ta_pole, ' ', B19_SZ);
   memset(m_a_pole, ' ', B11_SZ);
   memset(m_b_pole, ' ', B11_SZ);
   memset(m_cj1_pole, ' ', B11_SZ);
   memset(m_cj2_pole, ' ', B11_SZ);
   memset(m_dj1_pole, ' ', B11_SZ);
   memset(m_dj2_pole, ' ', B11_SZ);
   memset(m_pj1_pole, ' ', B10_SZ);   
   memset(m_pj2_pole, ' ', B10_SZ);
   memset(m_e_pole, ' ', B11_SZ);
   memset(m_f_pole, ' ', B11_SZ);
   memset(m_gk1_pole, ' ', B11_SZ);   
   memset(m_gk2_pole, ' ', B11_SZ);
   memset(m_hk1_pole, ' ', B11_SZ);
   memset(m_hk2_pole, ' ', B11_SZ);   
   memset(m_pk1_pole, ' ', B10_SZ);
   memset(m_pk2_pole, ' ', B10_SZ);   
   memset(m_tb_ut, ' ', B19_SZ);  
   memset(m_i_ut, ' ', B12_SZ);
   memset(m_j_ut, ' ', B12_SZ);
   memset(m_kn1_ut, ' ', B12_SZ);
   memset(m_kn2_ut, ' ', B12_SZ);
   memset(m_kn3_ut, ' ', B12_SZ);
   memset(m_kn4_ut, ' ', B12_SZ);
   memset(m_ln1_ut, ' ', B12_SZ);
   memset(m_ln2_ut, ' ', B12_SZ);
   memset(m_ln3_ut, ' ', B12_SZ);
   memset(m_ln4_ut, ' ', B12_SZ);
   memset(m_pn1_ut, ' ', B10_SZ);
   memset(m_pn2_ut, ' ', B10_SZ);
   memset(m_pn3_ut, ' ', B10_SZ);
   memset(m_pn4_ut, ' ', B10_SZ);
   memset(m_dt_att, ' ', B13_SZ);
   memset(m_date_att, ' ', B8_SZ);
   memset(m_t0_att, ' ', B16_SZ);
   memset(m_num_att, ' ', B5_SZ);
   memset(m_reserved_len, '0', B9_SZ);
  
   m_uuid[UUID_SZ] = '\0';
   m_numais[NUMAIS_SZ] = '\0';
   m_num_assoc_elem[NUM_ASSOC_ELEM_SZ] = '\0';
   m_reservedsubh_len[RESERVERDSHUBH_LEN_SZ] = '\0';
   m_qual_flag_att[B1_SZ] = '\0';
   m_interp_type_att[B1_SZ] = '\0';
   m_interp_order_att[B1_SZ] = '\0';
   m_att_type[B1_SZ] = '\0';
   m_ecf_eci_att[B1_SZ] = '\0';
   m_ta_pole[B19_SZ] = '\0';
   m_a_pole[B11_SZ] = '\0';
   m_b_pole[B11_SZ] = '\0';
   m_cj1_pole[B11_SZ] = '\0';
   m_cj2_pole[B11_SZ] = '\0';
   m_dj1_pole[B11_SZ] = '\0';
   m_dj2_pole[B11_SZ] = '\0';
   m_pj1_pole[B10_SZ] = '\0';   
   m_pj2_pole[B10_SZ] = '\0';
   m_e_pole[B11_SZ] = '\0';
   m_f_pole[B11_SZ] = '\0';
   m_gk1_pole[B11_SZ] = '\0';   
   m_gk2_pole[B11_SZ] = '\0';
   m_hk1_pole[B11_SZ] = '\0';
   m_hk2_pole[B11_SZ] = '\0';   
   m_pk1_pole[B10_SZ] = '\0';
   m_pk2_pole[B10_SZ] = '\0';   
   m_tb_ut[B19_SZ] = '\0';  
   m_i_ut[B12_SZ] = '\0';
   m_j_ut[B12_SZ] = '\0';
   m_kn1_ut[B12_SZ] = '\0';
   m_kn2_ut[B12_SZ] = '\0';
   m_kn3_ut[B12_SZ] = '\0';
   m_kn4_ut[B12_SZ] = '\0';
   m_ln1_ut[B12_SZ] = '\0';
   m_ln2_ut[B12_SZ] = '\0';
   m_ln3_ut[B12_SZ] = '\0';
   m_ln4_ut[B12_SZ] = '\0';
   m_pn1_ut[B10_SZ] = '\0';
   m_pn2_ut[B10_SZ] = '\0';
   m_pn3_ut[B10_SZ] = '\0';
   m_pn4_ut[B10_SZ] = '\0';
   m_dt_att[B13_SZ] = '\0';
   m_date_att[B8_SZ] = '\0';
   m_t0_att[B16_SZ] = '\0';
   m_num_att[B5_SZ] = '\0';
   m_reserved_len[B9_SZ] = '\0';

   m_aisdlvl.clear();
   m_assoc_elem_uuid.clear();
   m_q1.clear();
   m_q2.clear();
   m_q3.clear();
   m_q4.clear();
}

std::ostream& ossimNitfCsattbDes::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += DESID;
   pfx += ".";
   int w = (int)((pfx.size()<29)?29-pfx.size():24);
   ossim_uint32 i;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(w) << "UUID:" << m_uuid << "\n"
       << pfx << std::setw(w) << "NUMAIS:" << m_numais << "\n";
   
#if 0
   out << pfx << std::setw(w) << "DESSHL:" << m_desshl << "\n"
       << pfx << std::setw(w) << "UUID:" << m_uuid << "\n"
       << pfx << std::setw(w) << "numais:" << m_numais << "\n";
   for(i = 0; i < 
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
       << pfx << std::setw(w) << ":" << m_ << "\n"
#endif

   return out;
}


#if 0 /* ossimNitfGenericDes version */
ossimNitfCsattbDes::ossimNitfCsattbDes()
   : ossimNitfGenericDes("CSEXRB")
{
   // traceDebug.setTraceFlag(true);

   initializeFieldDefinitions();
   initializeFields();
   setDesLength(computeDesLength());
}

ossimNitfCsattbDes::ossimNitfCsattbDes(ossim_uint32 tagLength)
   : ossimNitfGenericDes("CSEXRB", tagLength)
{
   initializeFieldDefinitions();
   initializeFields();
}
void ossimNitfCsattbDes::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
      {
         //{"DE", 2, 0},
         //{"DESID", 25, 0},
         {"DESVER", 2, 1},
         {"DESCLAS to DESCTLN", 167, 0},
         {"DESSHL", 4, 1},
         {"UUID", 36, 0},
         {"NUMAIS", 3, 0},
         {"NUMAIS n", LOOP_START},
            {"AISDLVL", 3, 1},
         {"NUMAIS n", LOOP_END},
         {"NUM_ASSOC_ELEM", 3, 1},
         {"NUM_ASSOC_ELEM n", LOOP_START},
            {"ASSOC_ELEM_UUIDn", 36, 0},
         {"NUM_ASSOC_ELEM n", LOOP_END},
         {"RESERVEDSUBH_LEN", 4, 1},
         {"RESERVEDSUBH RESERVEDSUBH_LEN", VARIABLE_LENGTH, 0},
         {"QUAL_FLAG_ATT", 1, 1},
         {"INTERP_TYPE_ATT", 1, 1},
         {"INTERP_TYPE_ATT 2 = INTERP_TYPE_ATT 3 = |", IF_STATEMENT_START},
            {"INTERP_ORDER_ATT", 1, 1},
         {"INTERP_TYPE_ATT 2 = INTERP_TYPE_ATT 3 = |", IF_STATEMENT_END},
         {"ATT_TYPE", 1, 1},
         {"ECI_ECF_ATT", 1, 1},
         {"ECI_ECF_ATT 0 = DESVER 1 > &", IF_STATEMENT_START},
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
            {"KN1_UT", 12, 4, 9},
            {"KN2_UT", 12, 4, 9},
            {"KN3_UT", 12, 4, 9},
            {"KN4_UT", 12, 4, 9},
            {"LN1_UT", 12, 4, 9},
            {"LN2_UT", 12, 4, 9},
            {"LN3_UT", 12, 4, 9},
            {"LN4_UT", 12, 4, 9},
            {"PN1_UT", 10, 3, 6},
            {"PN2_UT", 10, 3, 6},
            {"PN3_UT", 10, 3, 6},
            {"PN4_UT", 10, 3, 6},
         {"ECI_ECF_ATT 0 = DESVER 1 > &", IF_STATEMENT_END},
         {"DT_ATT", 13, 3, 9},
         {"DATE_ATT", 8, 1},
         {"T0_ATT", 16, 1},
         {"NUM_ATT", 5, 1},
         {"NUM_ATT n", LOOP_START},
            {"Q1n", 18, 4, 15},
            {"Q2n", 18, 4, 15},
            {"Q3n", 18, 4, 15},
            {"Q4n", 18, 4, 15},
         {"NUM_ATT n", LOOP_END},
         {"RESERVED_LEN", 9, 1},
         {"RESERVED RESERVED_LEN", VARIABLE_LENGTH, 0}
      };
}
#endif


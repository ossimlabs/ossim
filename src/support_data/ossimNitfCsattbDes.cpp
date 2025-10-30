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
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>
#include <iomanip>
#include <iostream>

static ossimTrace traceDebug("ossimNitfCsattbDes:debug");
static const int FOREVER = 1;
const std::string ossimNitfCsattbDes::DESID = "CSATTB";

ossimNitfCsattbDes::ossimNitfCsattbDes()
   : ossimNitfRegisteredDes(DESID, 0),
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
     m_eci_ecf_att(),
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
   set_desid(DESID);

   // Set the lengths in base ossimNitfRegisteredDes:
   setDesSubHeaderLength(computeDesSubHeaderLength());
   setDesDataLength(computeDesDataLength());
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
     m_eci_ecf_att(),
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

   // Set the lengths in base ossimNitfRegisteredDes:
   setDesSubHeaderLength(computeDesSubHeaderLength());
   setDesDataLength(computeDesDataLength());
}

ossimString ossimNitfCsattbDes::getClassName() const
{
   return ossimString("ossimNitfCsattbDes");
}

void ossimNitfCsattbDes::parseStream(std::istream& in)
{
   static const char MODULE[] = "ossimNitfCsattbDes::parseStream(...)";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " DEBUG entered...\n";
   }
   
   char buf[64];
   std::string s;
   ossim_uint32 count;
   ossim_uint32 i;
   ossim_uint32 subHdrBytes = 0;
   ossim_uint32 desBytes = 0; 
   
   in.read(m_uuid, UUID_SZ);
   subHdrBytes += UUID_SZ;

   in.read(m_numais, NUMAIS_SZ);
   subHdrBytes += NUMAIS_SZ;
   count = getNumberAis();
   if (count > 0)
   {
      m_aisdlvl.resize(count);
      for(i = 0; i < count; ++i)
      {
         in.read(buf, AISDLVL_SZ);
         subHdrBytes += AISDLVL_SZ;
         buf[AISDLVL_SZ] = '\0';
         m_aisdlvl[i] = buf;
      }
   }

   in.read(m_num_assoc_elem, NUM_ASSOC_ELEM_SZ);
   subHdrBytes += NUM_ASSOC_ELEM_SZ;
   count = getNumberAssocElem();
   if (count > 0)
   {
      m_assoc_elem_uuid.resize(count);
      for(i = 0; i < count; ++i)
      {
         in.read(buf, UUID_SZ);
         subHdrBytes += UUID_SZ;
         buf[UUID_SZ] = '\0';
         m_assoc_elem_uuid[i] = buf;
      }
   }

   in.read(m_reservedsubh_len, RESERVERDSHUBH_LEN_SZ);
   subHdrBytes += RESERVERDSHUBH_LEN_SZ;
   // End sub header:

   // Start of DES data:
   in.read(m_qual_flag_att, B1_SZ);
   desBytes += B1_SZ;
   in.read(m_interp_type_att, B1_SZ);
   desBytes += B1_SZ;
   s = get_interp_type_att();
   if (s == "2" || s == "3")
   {
      in.read(m_interp_order_att, B1_SZ);
      desBytes += B1_SZ;
   }

   in.read(m_att_type, B1_SZ);
   desBytes += B1_SZ;
   in.read(m_eci_ecf_att, B1_SZ);
   desBytes += B1_SZ;

   if (m_eci_ecf_att[0] == '0' &&  getDesVersionNumber() >= 2)
   {
      in.read(m_ta_pole, B19_SZ);
      desBytes += B19_SZ;
      in.read(m_a_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_b_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_cj1_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_cj2_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_dj1_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_dj2_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_pj1_pole, B10_SZ);
      desBytes += B10_SZ;
      in.read(m_pj2_pole, B10_SZ);
      desBytes += B10_SZ;
      in.read(m_e_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_f_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_gk1_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_gk2_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_hk1_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_hk2_pole, B11_SZ);
      desBytes += B11_SZ;
      in.read(m_pk1_pole, B10_SZ);
      desBytes += B10_SZ;
      in.read(m_pk2_pole, B10_SZ);
      desBytes += B10_SZ;
      in.read(m_tb_ut, B19_SZ);
      desBytes += B19_SZ;
      in.read(m_i_ut, B12_SZ);
      desBytes += B12_SZ;
      in.read(m_j_ut, B12_SZ);
      desBytes += B12_SZ;
      in.read(m_kn1_ut, B12_SZ);
      desBytes += B12_SZ;
      in.read(m_kn2_ut, B12_SZ);
      desBytes += B12_SZ;
      in.read(m_kn3_ut, B12_SZ);
      desBytes += B12_SZ;
      in.read(m_kn4_ut, B12_SZ);
      desBytes += B12_SZ;
      in.read(m_ln1_ut, B12_SZ);
      desBytes += B12_SZ;
      in.read(m_ln2_ut, B12_SZ);
      desBytes += B12_SZ;
      in.read(m_ln3_ut, B12_SZ);
      desBytes += B12_SZ;
      in.read(m_ln4_ut, B12_SZ);
      desBytes += B12_SZ;
      in.read(m_pn1_ut, B10_SZ);
      desBytes += B10_SZ;
      in.read(m_pn2_ut, B10_SZ);
      desBytes += B10_SZ;
      in.read(m_pn3_ut, B10_SZ);
      desBytes += B10_SZ;
      in.read(m_pn4_ut, B10_SZ);
      desBytes += B10_SZ;
   }

   in.read(m_dt_att, B13_SZ);
   desBytes += B13_SZ;
   in.read(m_date_att, B8_SZ);
   desBytes += B8_SZ;
   in.read(m_t0_att, B16_SZ);
   desBytes += B16_SZ;
   in.read(m_num_att, B5_SZ);
   desBytes += B5_SZ;
   count = getNumberAtt();
   if (count > 0)
   {
      m_q1.resize(count);
      m_q2.resize(count);
      m_q3.resize(count);
      m_q4.resize(count);
      for(i = 0; i < count; ++i)
      {
         in.read(buf, B18_SZ);
         desBytes += B18_SZ;
         buf[B18_SZ] = '\0';
         m_q1[i] = buf;
         
         in.read(buf, B18_SZ);
         desBytes += B18_SZ;
         buf[B18_SZ] = '\0';
         m_q2[i] = buf;

         in.read(buf, B18_SZ);
         desBytes += B18_SZ;
         buf[B18_SZ] = '\0';
         m_q3[i] = buf;

         in.read(buf, B18_SZ);
         desBytes += B18_SZ;
         buf[B18_SZ] = '\0';
         m_q4[i] = buf;
      }
   }
   
   in.read(m_reserved_len, B9_SZ);
   desBytes += B9_SZ;

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "desshl field:              " << getDesSubHeaderLength()
         << "\nactual bytes read:       " << subHdrBytes
         << "\ndes length field:        " << getDesDataLength()
         << "\nactual bytes read:       " << desBytes << "\n"
         << "\ncomputed sub hdr bytes:  " << computeDesSubHeaderLength()
         << "\ncomputed des data bytes: " << computeDesDataLength()
         << MODULE << " DEBUG exited...\n";
   }
}

void ossimNitfCsattbDes::writeStream(std::ostream& out)
{
   static const char MODULE[] = "ossimNitfCsattbDes::writeStream(...)";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " DEBUG entered...\n";
   }
   
   char buf[64];
   std::string s;
   ossim_uint32 count;
   ossim_uint32 i;
   ossim_uint32 subHdrBytes = 0;
   ossim_uint32 desBytes = 0; 
   
   out.write(m_uuid, UUID_SZ);
   subHdrBytes += UUID_SZ;

   out.write(m_numais, NUMAIS_SZ);
   subHdrBytes += NUMAIS_SZ;
   count = getNumberAis();
   if ( count > 0 && m_aisdlvl.size() == count)
   {
      for(i = 0; i < count; ++i)
      {
         ossimNitfCommon::setField(buf,
                                   ossimString(m_aisdlvl[i]),
                                   AISDLVL_SZ,
                                   std::ios::right,
                                   '0');
         out.write(buf, AISDLVL_SZ);
         subHdrBytes += AISDLVL_SZ;
      }
   }

   out.write(m_num_assoc_elem, NUM_ASSOC_ELEM_SZ);
   subHdrBytes += NUM_ASSOC_ELEM_SZ;
   count = getNumberAssocElem();
   if ( count > 0 && m_assoc_elem_uuid.size() == count)
   {
      for(i = 0; i < count; ++i)
      {
         ossimNitfCommon::setField(buf,
                                   ossimString(m_assoc_elem_uuid[i]),
                                   UUID_SZ,
                                   std::ios::left,
                                   ' ');
         
         out.write(buf, UUID_SZ);
         subHdrBytes += UUID_SZ;
      }
   }

   out.write(m_reservedsubh_len, RESERVERDSHUBH_LEN_SZ);
   subHdrBytes += RESERVERDSHUBH_LEN_SZ;
   // End sub header:

   // Start of DES data:
   out.write(m_qual_flag_att, B1_SZ);
   desBytes += B1_SZ;
   out.write(m_interp_type_att, B1_SZ);
   desBytes += B1_SZ;
   s = get_interp_type_att();
   if (s == "2" || s == "3")
   {
      out.write(m_interp_order_att, B1_SZ);
      desBytes += B1_SZ;
   }

   out.write(m_att_type, B1_SZ);
   desBytes += B1_SZ;
   out.write(m_eci_ecf_att, B1_SZ);
   desBytes += B1_SZ;

   if (m_eci_ecf_att[0] == '0' &&  getDesVersionNumber() >= 2)
   {
      out.write(m_ta_pole, B19_SZ);
      desBytes += B19_SZ;
      out.write(m_a_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_b_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_cj1_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_cj2_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_dj1_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_dj2_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_pj1_pole, B10_SZ);
      desBytes += B10_SZ;
      out.write(m_pj2_pole, B10_SZ);
      desBytes += B10_SZ;
      out.write(m_e_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_f_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_gk1_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_gk2_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_hk1_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_hk2_pole, B11_SZ);
      desBytes += B11_SZ;
      out.write(m_pk1_pole, B10_SZ);
      desBytes += B10_SZ;
      out.write(m_pk2_pole, B10_SZ);
      desBytes += B10_SZ;
      out.write(m_tb_ut, B19_SZ);
      desBytes += B19_SZ;
      out.write(m_i_ut, B12_SZ);
      desBytes += B12_SZ;
      out.write(m_j_ut, B12_SZ);
      desBytes += B12_SZ;
      out.write(m_kn1_ut, B12_SZ);
      desBytes += B12_SZ;
      out.write(m_kn2_ut, B12_SZ);
      desBytes += B12_SZ;
      out.write(m_kn3_ut, B12_SZ);
      desBytes += B12_SZ;
      out.write(m_kn4_ut, B12_SZ);
      desBytes += B12_SZ;
      out.write(m_ln1_ut, B12_SZ);
      desBytes += B12_SZ;
      out.write(m_ln2_ut, B12_SZ);
      desBytes += B12_SZ;
      out.write(m_ln3_ut, B12_SZ);
      desBytes += B12_SZ;
      out.write(m_ln4_ut, B12_SZ);
      desBytes += B12_SZ;
      out.write(m_pn1_ut, B10_SZ);
      desBytes += B10_SZ;
      out.write(m_pn2_ut, B10_SZ);
      desBytes += B10_SZ;
      out.write(m_pn3_ut, B10_SZ);
      desBytes += B10_SZ;
      out.write(m_pn4_ut, B10_SZ);
      desBytes += B10_SZ;
   }

   out.write(m_dt_att, B13_SZ);
   desBytes += B13_SZ;
   out.write(m_date_att, B8_SZ);
   desBytes += B8_SZ;
   out.write(m_t0_att, B16_SZ);
   desBytes += B16_SZ;
   out.write(m_num_att, B5_SZ);
   desBytes += B5_SZ;
   count = getNumberAtt();
   if (count > 0 && m_q1.size() == count)
   {
      for(i = 0; i < count; ++i)
      {
         ossimNitfCommon::setField(buf,
                                   ossimString(m_q1[i]),
                                   B18_SZ,
                                   std::ios::left,
                                   '0');
         out.write(buf, B18_SZ);
         desBytes += B18_SZ;

         ossimNitfCommon::setField(buf,
                                   ossimString(m_q2[i]),
                                   B18_SZ,
                                   std::ios::left,
                                   '0');
         out.write(buf, B18_SZ);
         desBytes += B18_SZ;

         ossimNitfCommon::setField(buf,
                                   ossimString(m_q3[i]),
                                   B18_SZ,
                                   std::ios::left,
                                   '0');
         out.write(buf, B18_SZ);
         desBytes += B18_SZ;

         ossimNitfCommon::setField(buf,
                                   ossimString(m_q4[i]),
                                   B18_SZ,
                                   std::ios::left,
                                   '0');
         out.write(buf, B18_SZ);
         desBytes += B18_SZ;
      }
   }

   out.write(m_reserved_len, B9_SZ);
   desBytes += B9_SZ;

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "desshl field:              " << getDesSubHeaderLength()
         << "\nactual bytes written:    " << subHdrBytes
         << "\ndes length field:        " << getDesDataLength()
         << "\nactual bytes writen:     " << desBytes << "\n"
         << "\ncomputed sub hdr bytes:  " << computeDesSubHeaderLength()
         << "\ncomputed des data bytes: " << computeDesDataLength()
         << MODULE << " DEBUG exited...\n";
   }
}

bool ossimNitfCsattbDes::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   static const char MODULE[] = "ossimNitfCsattbDes::loadState(...)";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " entered...\n"
         << "kwl:\n" << kwl << "\n"
         << "prefix: " << (prefix?prefix:"null") << "\n";
   }

   bool status = true;
   std::string pfx = prefix?prefix:"";
   std::string key;
   std::string k;
   std::string value;
   std::string os;
   char buf[64];
   std::string s;
   ossim_uint32 count;
   ossim_uint32 i;
   ossim_uint32 subHdrBytes = 0;
   ossim_uint32 desBytes = 0; 
 
   while(FOREVER) // Break on error or at end.
   {
      value = kwl.findKey(pfx, ossim::nitf::UUID_KW);
      if (value.size())
      {
         ossimNitfCommon::setField(m_uuid,
                                   ossimString(value),
                                   UUID_SZ,
                                   std::ios::left,
                                   ' ');
      }
      value = kwl.findKey(pfx, "NUMAIS");
      if (value.size())
      {
         ossimNitfCommon::setField(m_numais,
                                   ossimString(value),
                                   NUMAIS_SZ,
                                   std::ios::right,
                                   '0');
      }
      count = getNumberAis();
      if (count > 0)
      {
         key = "AISDLVL";
         m_aisdlvl.resize(count);
         for(i = 0; i < count; ++i)
         {
            k = key + ossimString::toString(i).string();
            value = kwl.findKey(pfx, k);
            if (value.size())
            {
               ossimNitfCommon::setField(buf,
                                         ossimString(value),
                                         AISDLVL_SZ,
                                         std::ios::right,
                                         '0');
               buf[AISDLVL_SZ] = '\0';
               m_aisdlvl[i] = buf;
            }
#if 0 /* Not sure if this should be an error and break out? drb */
            else // error... ??? )
            {
               ossimNotify(ossimNotifyLevel_WARN)
                  << MODULE << " WARNING:\n" << "Missing key: " << k << std::endl;
               status = false;
               break;
            }
#endif         
         }
         
      }

      value = kwl.findKey(pfx, "NUM_ASSOC_ELEM");
      if (value.size())
      {
         ossimNitfCommon::setField(m_num_assoc_elem,
                                   ossimString(value),
                                   NUM_ASSOC_ELEM_SZ,
                                   std::ios::right,
                                   '0');
      }
      count = getNumberAssocElem();
      if (count > 0)
      {
         key = "ASSOC_ELEM_UUID";
         m_assoc_elem_uuid.resize(count);
         for(i = 0; i < count; ++i)
         {
            k = key + ossimString::toString(i).string();
            value = kwl.findKey(pfx, k);
            if (value.size())
            {
               ossimNitfCommon::setField(buf,
                                         ossimString(value),
                                         UUID_SZ,
                                         std::ios::right,
                                         '0');
               buf[UUID_SZ] = '\0';
               m_assoc_elem_uuid[i] = buf;
            }
         }

         // m_reservedsubh_len not loaded
      }
      // End sub header:

      // Start of DES data:
      value = kwl.findKey(pfx, "QUAL_FLAG_ATT");
      if (value.size())
      {
         m_qual_flag_att[0] = (ossimString(value).toBool()?'1':'0');
      }

      value = kwl.findKey(pfx, "INTERP_TYPE_ATT");
      if (value.size())
      {
         ossimNitfCommon::setField(m_interp_type_att,
                                   ossimString(value),
                                   B1_SZ,
                                   std::ios::right,
                                   '0');
      }
      s = get_interp_type_att();
      if (s == "2" || s == "3")
      {
         value = kwl.findKey(pfx, "INTERP_ORDER_ATT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_interp_order_att,
                                      ossimString(value),
                                      B1_SZ,
                                      std::ios::right,
                                      '0');
         }
      }
      value = kwl.findKey(pfx, "ATT_TYPE");
      if (value.size())
      {
         ossimNitfCommon::setField(m_att_type,
                                   ossimString(value),
                                   B1_SZ,
                                   std::ios::right,
                                   '0');
      }
      value = kwl.findKey(pfx, "ECI_ECF_ATT");
      if (value.size())
      {
         ossimNitfCommon::setField(m_eci_ecf_att,
                                   ossimString(value),
                                   B1_SZ,
                                   std::ios::right,
                                   '0');
      }

      if (m_eci_ecf_att[0] == '0' &&  getDesVersionNumber() >= 2)
      {
         value = kwl.findKey(pfx, "TA_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_ta_pole,
                                      ossimString(value),
                                      B19_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "A_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_a_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "B_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_b_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "CJ1_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_cj1_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "CJ2_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_cj2_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "DJ1_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_dj1_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "DJ2_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_dj2_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "PJ1_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_pj1_pole,
                                      ossimString(value),
                                      B10_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "PJ2_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_pj2_pole,
                                      ossimString(value),
                                      B10_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "E_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_e_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "F_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_f_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "GK1_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_gk1_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "GK2_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_gk2_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "HK1_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_hk1_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "HK2_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_hk2_pole,
                                      ossimString(value),
                                      B11_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "PK1_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_pk1_pole,
                                      ossimString(value),
                                      B10_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "PK2_POLE");
         if (value.size())
         {
            ossimNitfCommon::setField(m_pk2_pole,
                                      ossimString(value),
                                      B10_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "TB_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_tb_ut,
                                      ossimString(value),
                                      B19_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "I_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_i_ut,
                                      ossimString(value),
                                      B12_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "J_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_j_ut,
                                      ossimString(value),
                                      B12_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "KN1_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_kn1_ut,
                                      ossimString(value),
                                      B12_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "KN2_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_kn2_ut,
                                      ossimString(value),
                                      B12_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "KN3_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_kn3_ut,
                                      ossimString(value),
                                      B12_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "KN4_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_kn4_ut,
                                      ossimString(value),
                                      B12_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "LN1_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_ln1_ut,
                                      ossimString(value),
                                      B12_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "LN2_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_ln2_ut,
                                      ossimString(value),
                                      B12_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "LN3_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_ln3_ut,
                                      ossimString(value),
                                      B12_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "LN4_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_ln4_ut,
                                      ossimString(value),
                                      B12_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "PN1_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_pn1_ut,
                                      ossimString(value),
                                      B10_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "PN2_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_pn2_ut,
                                      ossimString(value),
                                      B10_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "PN3_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_pn3_ut,
                                      ossimString(value),
                                      B10_SZ,
                                      std::ios::right,
                                      '0');
         }
         value = kwl.findKey(pfx, "PN4_UT");
         if (value.size())
         {
            ossimNitfCommon::setField(m_pn4_ut,
                                      ossimString(value),
                                      B10_SZ,
                                      std::ios::right,
                                      '0');
         }

      } // matches: if (m_eci_ecf_att[0] == '0' &&  getDesVersionNumber() >= 2)

      value = kwl.findKey(pfx, "DT_ATT");
      if (value.size())
      {
         ossimNitfCommon::setField(m_dt_att,
                                   ossimString(value),
                                   B13_SZ,
                                   std::ios::right,
                                   '0');
      }
      value = kwl.findKey(pfx, "DATE_ATT");
      if (value.size())
      {
         ossimNitfCommon::setField(m_date_att,
                                   ossimString(value),
                                   B8_SZ,
                                   std::ios::right,
                                   '0');
      }
      value = kwl.findKey(pfx, "T0_ATT");
      if (value.size())
      {
         ossimNitfCommon::setField(m_t0_att,
                                   ossimString(value),
                                   B16_SZ,
                                   std::ios::right,
                                   '0');
      }
      value = kwl.findKey(pfx, "NUM_ATT");
      if (value.size())
      {
         ossimNitfCommon::setField(m_num_att,
                                   ossimString(value),
                                   B5_SZ,
                                   std::ios::right,
                                   '0');
      }
      count = getNumberAtt();
      if (count > 0)
      {
         m_q1.resize(count);
         m_q2.resize(count);
         m_q3.resize(count);
         m_q4.resize(count);
         for(i = 0; i < count; ++i)
         {
            key = "Q1_";
            k = key + ossimString::toString(i + 1).string();
            value = kwl.findKey(pfx, k);

            if (value.size())
            {
               ossimNitfCommon::setField(buf,
                                         ossimString(value),
                                         B18_SZ,
                                         std::ios::right,
                                         '0');
               buf[B18_SZ] = '\0';
               m_q1[i] = buf;
            }
            key = "Q2_";
            k = key + ossimString::toString(i + 1).string();
            value = kwl.findKey(pfx, k);
            if (value.size())
            {
               ossimNitfCommon::setField(buf,
                                         ossimString(value),
                                         B18_SZ,
                                         std::ios::right,
                                         '0');
               buf[B18_SZ] = '\0';
               m_q2[i] = buf;
            }
            key = "Q3_";
            k = key + ossimString::toString(i + 1).string();
            value = kwl.findKey(pfx, k);
            if (value.size())
            {
               ossimNitfCommon::setField(buf,
                                         ossimString(value),
                                         B18_SZ,
                                         std::ios::right,
                                         '0');
               buf[B18_SZ] = '\0';
               m_q3[i] = buf;
            }
            key = "Q4_";
            k = key + ossimString::toString(i + 1).string();
            value = kwl.findKey(pfx, k);
            if (value.size())
            {
               ossimNitfCommon::setField(buf,
                                         ossimString(value),
                                         B18_SZ,
                                         std::ios::right,
                                         '0');
               buf[B18_SZ] = '\0';
               m_q4[i] = buf;
            }
         }

      } // matches: count = getNumberAtt(); if (count > 0){

      // m_reserved_len not loaded
      
      break; // Trailing break from forever loop.
      
   } // Matches: while(FOREVER)

   // Set the lengths in base ossimNitfRegisteredDes:
   setDesSubHeaderLength(computeDesSubHeaderLength());
   setDesDataLength(computeDesDataLength());

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exit status: " << (status?"true":"false") << "\n";
   }   

   return status;
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
   ossim_uint32 result = 0;
   std::string s = m_numais;
   if (s != "ALL" && s != "999")
   {
      result = ossimString(s).toUInt32();
   }
   return result;
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
   return std::string(m_reservedsubh_len);
}

ossim_uint32 ossimNitfCsattbDes::getReserveSubHdrLength() const
{
   return ossimString(m_reservedsubh_len).toUInt32();
}

std::string ossimNitfCsattbDes::get_interp_type_att() const
{
   return std::string(m_interp_type_att);
}

std::string ossimNitfCsattbDes::get_num_att() const
{
   return std::string(m_num_att);
}

ossim_uint32 ossimNitfCsattbDes::getNumberAtt() const
{
   return ossimString(m_num_att).toUInt32();
}

ossim_uint32 ossimNitfCsattbDes::computeDesSubHeaderLength() const
{
   return 36 + 3 + getNumberAis() * 3 + 3 + getNumberAssocElem() * 36 + 4;
}

ossim_uint32 ossimNitfCsattbDes::computeDesDataLength() const
{
   ossim_uint32 length = 2;
   std::string s = get_interp_type_att();
   if (s == "2" || s == "3")
   {
      ++length;
   }
   length += 2;
   if (m_eci_ecf_att[0] == '0' &&  getDesVersionNumber() >= 2)
   {
      length += 370; // 19+66+20+66+20+19+120+40
   }
   length += 42; // 13+8+16+5
   if (getNumberAtt())
   {
      length += getNumberAtt() * 72; // 18*4
   }
   length += 9;
   return length;
}

void ossimNitfCsattbDes::clearFields()
{
   memset(m_uuid, ' ', UUID_SZ);
   // memset(m_numais, ' ', NUMAIS_SZ);
   strcpy(m_numais, "ALL");
   memset(m_num_assoc_elem, ' ', NUM_ASSOC_ELEM_SZ);
   memset(m_reservedsubh_len, '0', RESERVERDSHUBH_LEN_SZ);
   memset(m_qual_flag_att, '0', B1_SZ);
   memset(m_interp_type_att, '0', B1_SZ);
   memset(m_interp_order_att, ' ', B1_SZ);
   memset(m_att_type, ' ', B1_SZ);
   memset(m_eci_ecf_att, ' ', B1_SZ);
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
   memset(m_num_att, '0', B5_SZ);
   memset(m_reserved_len, '0', B9_SZ);
  
   m_uuid[UUID_SZ] = '\0';
   m_numais[NUMAIS_SZ] = '\0';
   m_num_assoc_elem[NUM_ASSOC_ELEM_SZ] = '\0';
   m_reservedsubh_len[RESERVERDSHUBH_LEN_SZ] = '\0';
   m_qual_flag_att[B1_SZ] = '\0';
   m_interp_type_att[B1_SZ] = '\0';
   m_interp_order_att[B1_SZ] = '\0';
   m_att_type[B1_SZ] = '\0';
   m_eci_ecf_att[B1_SZ] = '\0';
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
   int w = 24;
   ossim_uint32 i;
   ossim_uint32 count;
   std::string s;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(w) << "UUID:" << m_uuid << "\n"
       << pfx << std::setw(w) << "NUMAIS:" << m_numais << "\n";
   count = getNumberAis();
   if (count > 0 && m_aisdlvl.size() == count)
   {
      for(i = 0; i < count; ++i)
      {
         s = "AISDLV" + ossimString::toString(i).string() + ":";
         out << pfx << std::setw(w) << s << m_aisdlvl[i] << "\n";
      }
   }

   out << pfx << std::setw(w) << "NUM_ASSOC_ELEM:" << m_num_assoc_elem << "\n";
   count = getNumberAssocElem();
   if (count > 0 && m_assoc_elem_uuid.size() == count)
   {
      for(i = 0; i < count; ++i)
      {
         s = "ASSOC_ELEM_UUID" + ossimString::toString(i).string() + ":";
         out << pfx << std::setw(w) << s << m_assoc_elem_uuid[i] << "\n";
      }
   }
   
   // skipping m_reservedsubh_len
   // End of sub header.

   // Beginning of DESDATA section:
   out << pfx << std::setw(w) << "QUAL_FLAG_ATT:" << m_qual_flag_att << "\n"
       << pfx << std::setw(w) << "INTERP_TYPE_ATT:" << m_interp_type_att << "\n";

   s = get_interp_type_att();
   if (s == "2" || s == "3")
   {
      out << pfx << std::setw(w) << "INTERP_ORDER_ATT:" << m_interp_order_att << "\n";
   }

   out << pfx << std::setw(w) << "ATT_TYPE:" << m_att_type << "\n"
       << pfx << std::setw(w) << "ECI_ECF_ATT:" << m_eci_ecf_att << "\n";

   if (m_eci_ecf_att[0] == '0' &&  getDesVersionNumber() >= 2)
   {
      out << pfx << std::setw(w) << "TA_POLE:" << m_ta_pole << "\n"
          << pfx << std::setw(w) << "A_POLE:" << m_a_pole << "\n"
          << pfx << std::setw(w) << "B_POLE:" << m_b_pole << "\n"
          << pfx << std::setw(w) << "CJ1_POLE:" << m_cj1_pole<< "\n"
          << pfx << std::setw(w) << "CJ2_POLE:" << m_cj2_pole << "\n"
          << pfx << std::setw(w) << "DJ1_POLE:" << m_dj1_pole << "\n"
          << pfx << std::setw(w) << "DJ2_POLE:" << m_dj2_pole << "\n"
          << pfx << std::setw(w) << "PJ1_POLE:" << m_pj1_pole << "\n"
          << pfx << std::setw(w) << "PJ2_POLE:" << m_pj2_pole << "\n"
          << pfx << std::setw(w) << "E_POLE:" << m_e_pole << "\n"
          << pfx << std::setw(w) << "F_POLE:" << m_f_pole << "\n"
          << pfx << std::setw(w) << "GK1_POLE:" << m_gk1_pole << "\n"
          << pfx << std::setw(w) << "GK2_POLE:" << m_gk2_pole << "\n"
          << pfx << std::setw(w) << "HK1_POLE:" << m_hk1_pole << "\n"
          << pfx << std::setw(w) << "HK2_POLE:" <<  m_hk2_pole<< "\n"
          << pfx << std::setw(w) << "PK1_POLE:" << m_pk1_pole << "\n"
          << pfx << std::setw(w) << "PK2_POLE:" << m_pk2_pole << "\n"
          << pfx << std::setw(w) << "TB_UT:" << m_tb_ut << "\n"
          << pfx << std::setw(w) << "I_UT:" << m_i_ut << "\n"
          << pfx << std::setw(w) << "J_UT:" << m_j_ut << "\n"
          << pfx << std::setw(w) << "KN1_UT:" << m_kn1_ut << "\n"
          << pfx << std::setw(w) << "KN2_UT:" << m_kn2_ut << "\n"
          << pfx << std::setw(w) << "KN3_UT:" << m_kn3_ut << "\n"
          << pfx << std::setw(w) << "KN4_UT:" << m_kn4_ut << "\n"
          << pfx << std::setw(w) << "LN1_UT:" << m_ln1_ut << "\n"
          << pfx << std::setw(w) << "LN2_UT:" << m_ln2_ut << "\n"
          << pfx << std::setw(w) << "LN3_UT:" << m_ln3_ut << "\n"
          << pfx << std::setw(w) << "LN4_UT:" << m_ln4_ut << "\n"
          << pfx << std::setw(w) << "PN1_UT:" << m_pn1_ut << "\n"
          << pfx << std::setw(w) << "PN2_UT:" << m_pn2_ut << "\n"
          << pfx << std::setw(w) << "PN3_UT:" << m_pn3_ut << "\n"
          << pfx << std::setw(w) << "PN4_UT:" << m_pn4_ut << "\n";
   }

   out << pfx << std::setw(w) << "DT_ATT:" << m_dt_att << "\n"
       << pfx << std::setw(w) << "DATE_ATT:" << m_date_att << "\n"
       << pfx << std::setw(w) << "T0_ATT:" << m_t0_att << "\n"
       << pfx << std::setw(w) << "NUM_ATT:" << m_num_att << "\n";

   count = getNumberAtt();
   if (count > 0 && m_q1.size() == count)
   {
      for(i = 0; i < count; ++i)
      {
         s = "Q1[" + ossimString::toString(i + 1).string() + "]:";
         out << pfx << std::setw(w) << s << m_q1[i] << "\n";
         s = "Q2[" + ossimString::toString(i + 1).string() + "]:";
         out << pfx << std::setw(w) << s << m_q2[i] << "\n";
         s = "Q3[" + ossimString::toString(i + 1).string() + "]:";
         out << pfx << std::setw(w) << s << m_q3[i] << "\n";
         s = "Q4[" + ossimString::toString(i + 1).string() + "]:";
         out << pfx << std::setw(w) << s << m_q4[i] << "\n";
      }
   }
   
   // skipping m_reserved_len
   // End of DES data.

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


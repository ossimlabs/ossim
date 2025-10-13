//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSATTB des class declaration.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.6 for more info.
//
//----------------------------------------------------------------------------

#ifndef ossimNitfCsattbDes_HEADER
#define ossimNitfCsattbDes_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredDes.h>
#include <string>
#include <vector>

class ossimKeywordlist;

class OSSIM_DLL ossimNitfCsattbDes : public ossimNitfRegisteredDes
{
public:

   enum // byte size enums:
   {
      UUID_SZ = 36,
      NUMAIS_SZ = 3,
      AISDLVL_SZ = 3,
      NUM_ASSOC_ELEM_SZ = 3,
      RESERVERDSHUBH_LEN_SZ = 4,

      B1_SZ = 1,
      B5_SZ = 5,
      B8_SZ = 8,
      B9_SZ = 9,
      B10_SZ = 10,
      B11_SZ = 11,
      B12_SZ = 12,
      B13_SZ = 13,
      B16_SZ = 16,
      B18_SZ = 18,      
      B19_SZ = 19
   };
   
   ossimNitfCsattbDes();
   
   ossimNitfCsattbDes(ossim_uint32 tagLength);
   
   virtual ossimString getClassName() const;

   static const std::string DESID;

   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   /*!
    * Method to the load (recreate) the state of the object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
 
   // DES sub header:
   std::string get_uuid() const;   

   /**
    * @brief Gets the number of associated image segments field.
    * 3 byte field. 001 to 998, ALL or 999. Where 999 is the same as
    * ALL.
    * @return field as a string.
    */ 
   std::string get_numais() const;

   /**
    * @brief Number of associated image segments.
    * @return 0 if(m_numais==ALL or 999); else, field as an int.
    */
   ossim_uint32 getNumberAis() const;
   
   bool get_aisdlv(ossim_uint32 index, std::string& aisdlvl) const;
   
   std::string get_num_assoc_elem() const;
   ossim_uint32 getNumberAssocElem() const;
   
   bool get_assoc_elem_uuid(ossim_uint32 index, std::string& uuid) const;
   
   std::string get_reservedsubh_len() const;
   ossim_uint32 getReserveSubHdrLength() const;

   std::string get_interp_type_att() const;

   std::string get_num_att() const;
   ossim_uint32 getNumberAtt() const;

   /**
    * Clears all string fields within the record to some default nothingness.
    */
   void clearFields();

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const;
private:

   ossim_uint32 computeDesSubHeaderLength() const;
   ossim_uint32 computeDesDataLength() const;

   // DES user defined sub header:
   char m_uuid[UUID_SZ+1];

   char m_numais[NUMAIS_SZ+1];
   std::vector<std::string>  m_aisdlvl; // 3 bytes
   
   char m_num_assoc_elem[NUM_ASSOC_ELEM_SZ+1];
   std::vector<std::string> m_assoc_elem_uuid;

   char m_reservedsubh_len[RESERVERDSHUBH_LEN_SZ+1];

   // DES Data:
   char m_qual_flag_att[B1_SZ+1];
   char m_interp_type_att[B1_SZ+1];
   char m_interp_order_att[B1_SZ+1];
   char m_att_type[B1_SZ+1];
   char m_eci_ecf_att[B1_SZ+1];
   char m_ta_pole[B19_SZ+1];
   char m_a_pole[B11_SZ+1];
   char m_b_pole[B11_SZ+1];
   char m_cj1_pole[B11_SZ+1];
   char m_cj2_pole[B11_SZ+1];
   char m_dj1_pole[B11_SZ+1];
   char m_dj2_pole[B11_SZ+1];
   char m_pj1_pole[B10_SZ+1];   
   char m_pj2_pole[B10_SZ+1];
   char m_e_pole[B11_SZ+1];
   char m_f_pole[B11_SZ+1];
   char m_gk1_pole[B11_SZ+1];   
   char m_gk2_pole[B11_SZ+1];
   char m_hk1_pole[B11_SZ+1];
   char m_hk2_pole[B11_SZ+1];   
   char m_pk1_pole[B10_SZ+1];
   char m_pk2_pole[B10_SZ+1];   
   char m_tb_ut[B19_SZ+1];  
   char m_i_ut[B12_SZ+1];
   char m_j_ut[B12_SZ+1];
   char m_kn1_ut[B12_SZ+1];
   char m_kn2_ut[B12_SZ+1];
   char m_kn3_ut[B12_SZ+1];
   char m_kn4_ut[B12_SZ+1];
   char m_ln1_ut[B12_SZ+1];
   char m_ln2_ut[B12_SZ+1];
   char m_ln3_ut[B12_SZ+1];
   char m_ln4_ut[B12_SZ+1];
   char m_pn1_ut[B10_SZ+1];
   char m_pn2_ut[B10_SZ+1];
   char m_pn3_ut[B10_SZ+1];
   char m_pn4_ut[B10_SZ+1];
   char m_dt_att[B13_SZ+1];
   char m_date_att[B8_SZ+1];
   char m_t0_att[B16_SZ+1];
   char m_num_att[B5_SZ+1];
   std::vector<std::string> m_q1;
   std::vector<std::string> m_q2;
   std::vector<std::string> m_q3;
   std::vector<std::string> m_q4;
   
   char m_reserved_len[B9_SZ+1];
};

#endif /* #ifndef ossimNitfCsattbDes_HEADER */

#if 0 /* ossimNitfGenericDes version */
#include <ossim/support_data/ossimNitfGenericDes.h>

class OSSIM_DLL ossimNitfCsattbDes : public ossimNitfGenericDes
{
public:
   
   ossimNitfCsattbDes();
   
   ossimNitfCsattbDes(ossim_uint32 tagLength);
   
   virtual ossimString getClassName() const;

   static const std::string CETAG_KW;

private:

   void initializeFieldDefinitions();

};
#endif

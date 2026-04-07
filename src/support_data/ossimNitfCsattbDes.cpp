#include <ossim/support_data/ossimNitfCsattbDes.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>

static ossimTrace traceDebug("ossimNitfCsattbdes:debug");

const std::string ossimNitfCsattbDes::DESID = "CSATTB";

static const int FOREVER = 1;


ossimNitfCsattbDes::ossimNitfCsattbDes()
   : ossimNitfGenericDes("CSATTB")
{
   // traceDebug.setTraceFlag(true);

   initializeFieldDefinitions();
   initializeFields();
   setDesSubHeaderLength(getDesSubHeaderLength());
   setDesDataLength(getDesDataLength());
}

ossimNitfCsattbDes::ossimNitfCsattbDes(ossim_uint32 tagLength)
   : ossimNitfGenericDes("CSATTB", tagLength)
{
   initializeFieldDefinitions();
   initializeFields();
   setDesSubHeaderLength(getDesSubHeaderLength());
}

void ossimNitfCsattbDes::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
      {
         {ossim::nitf::UUID_KW, 36, 0},
         {ossim::nitf::NUMAIS_KW, 3, 0},
         {ossim::nitf::NUMAIS_KW, LOOP_START},
            {ossim::nitf::AISDLVL_KW, 3, 1},
         {ossim::nitf::NUMAIS_KW, LOOP_END},
         {ossim::nitf::NUM_ASSOC_ELEM_KW, 3, 1},
         {ossim::nitf::NUM_ASSOC_ELEM_KW, LOOP_START},
            {ossim::nitf::ASSOC_ELEM_UUID_KW, 36, 0},
         {ossim::nitf::NUM_ASSOC_ELEM_KW, LOOP_END},
         {ossim::nitf::RESERVEDSUBH_LEN_KW, 4, 1},
         {ossim::nitf::RESERVEDSUBH_KW + " " + ossim::nitf::RESERVEDSUBH_LEN_KW, VARIABLE_LENGTH, 0},
         {ossim::nitf::QUAL_FLAG_ATT_KW, 1, 1},
         {ossim::nitf::INTERP_TYPE_ATT_KW, 1, 1},
         {ossim::nitf::INTERP_TYPE_ATT_KW + " 1 >", IF_STATEMENT_START},
            {ossim::nitf::INTERP_ORDER_ATT_KW, 1, 1},
         {ossim::nitf::INTERP_TYPE_ATT_KW + " 1 >", IF_STATEMENT_END},
         {ossim::nitf::ATT_TYPE_KW, 1, 1},
         {ossim::nitf::ECI_ECF_ATT_KW, 1, 1},
         /*{"ECI_ECF_ATT 0 = DESVER 1 > &", IF_STATEMENT_START}, Assuming desver 1
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
         {"ECI_ECF_ATT 0 = DESVER 1 > &", IF_STATEMENT_END},*/
         {ossim::nitf::DT_ATT_KW, 13, 3, 9},
         {ossim::nitf::DATE_ATT_KW, 8, 1},
         {ossim::nitf::T0_ATT_KW, 16, U_DOUBLE, 9},
         {ossim::nitf::NUM_ATT_KW, 5, 1},
         {ossim::nitf::NUM_ATT_KW, LOOP_START},
            {ossim::nitf::Q1_KW, 18, 4, 15},
            {ossim::nitf::Q2_KW, 18, 4, 15},
            {ossim::nitf::Q3_KW, 18, 4, 15},
            {ossim::nitf::Q4_KW, 18, 4, 15},
         {ossim::nitf::NUM_ATT_KW, LOOP_END},
         {ossim::nitf::RESERVED_LEN_KW, 9, 1},
         {ossim::nitf::RESERVED_KW + " " + ossim::nitf::RESERVED_LEN_KW, VARIABLE_LENGTH, 0}
      };
}

ossimString ossimNitfCsattbDes::getClassName() const
{
   return ossimString("ossimNitfCsattbdes");
}
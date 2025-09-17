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

static ossimTrace traceDebug("ossimNitfCsattbdes:debug");

const std::string ossimNitfCsattbDes::CETAG_KW = "CSEXRB";

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

ossimString ossimNitfCsattbDes::getClassName() const
{
   return ossimString("ossimNitfCsattbdes");
}
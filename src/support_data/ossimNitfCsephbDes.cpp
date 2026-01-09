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

static ossimTrace traceDebug("ossimNitfCsephbdes:debug");

const std::string ossimNitfCsephbDes::DESID = "CSEPHB";

static const int FOREVER = 1;


ossimNitfCsephbDes::ossimNitfCsephbDes()
   : ossimNitfGenericDes("CSEPHB")
{
   // traceDebug.setTraceFlag(true);

   initializeFieldDefinitions();
   initializeFields();
   setDesSubHeaderLength(getDesSubHeaderLength());
   setDesDataLength(getDesDataLength());
}

ossimNitfCsephbDes::ossimNitfCsephbDes(ossim_uint32 tagLength)
   : ossimNitfGenericDes("CSEPHB", tagLength)
{
   initializeFieldDefinitions();
   initializeFields();
   setDesSubHeaderLength(getDesSubHeaderLength());
}
void ossimNitfCsephbDes::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
      {
         /*{"DE", 2, 0},
         {"DESID", 25, 0},
         {"DESVER", 2, 1},
         {"DESCLAS to DESCTLN", 167, 0, 0, "0"},
         {"DESSHL", 4, 1},*/
         {"UUID", 36, 0, 0, " "},
         {"NUMAIS", 3, 0, 0, "ALL"},
         {"NUMAIS n", LOOP_START},
            {"AISDLVL", 3, 1},
         {"NUMAIS n", LOOP_END},
         {"NUM_ASSOC_ELEM", 3, 1, 0, "1"},
         {"NUM_ASSOC_ELEM n", LOOP_START},
            {"ASSOC_ELEM_UUID", 36, 0},
         {"NUM_ASSOC_ELEM n", LOOP_END},
         {"RESERVEDSUBH_LEN", 4, 1},
         {"RESERVEDSUBH RESERVEDSUBH_LEN", VARIABLE_LENGTH, 1},
         {"QUAL_FLAG_EPH", 1, 1, 0, "1"},
         {"INTERP_TYPE_EPH", 1, 1},
         {"INTERP_TYPE_EPH 2 =", IF_STATEMENT_START},
            {"INTERP_ORDER_EPH", 1, 1},
         {"INTERP_TYPE_EPH 2 =", IF_STATEMENT_END},
         {"EPHEM_FLAG", 1, 1},
         {"ECI_ECF_EPHEM", 1, 1, 0, "1"},
         /*{"ECI_ECF_EPHEM 0 =", IF_STATEMENT_START},  //Assumes desver 1
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
         {"ECI_ECF_EPHEM 0 =", IF_STATEMENT_END},*/
         {"DT_EPHEM", 13, 3, 9},
         {"DATE_EPHEM", 8, 1},
         {"T0_EPHEM", 16, 0},
         {"NUM_EPHEM", 5, 1},
         {"NUM_EPHEM n", LOOP_START},
            {"EPHEM_X", 12, 4, 2},
            {"EPHEM_Y", 12, 4, 2},
            {"EPHEM_Z", 12, 4, 2},
         {"NUM_EPHEM n", LOOP_END},
         {"RESERVED_LEN", 9, 1},
         {"RESERVED_LEN 0 = !", IF_STATEMENT_START},
            {"MASK_LEN", 2, 1},
            {"RESERVED_FIELD_MASK MASK_LEN", VARIABLE_LENGTH, 0},
            {"RESERVED_LEN_AREA1", 9, 1},
            {"ACCEL_PROVIDED", 1, 0},
            {"NUM_EPHEM n", LOOP_START},
               {"VEL_X", 12, 4, 2},
               {"VEL_Y", 12, 4, 2},
               {"VEL_Z", 12, 4, 2},
               {"^ACCEL_PROVIDED 'Y' =", IF_STATEMENT_START},
                  {"ACCEL_X", 12, 4, 2},
                  {"ACCEL_Y", 12, 4, 2},
                  {"ACCEL_Z", 12, 4, 2},
               {"^ACCEL_PROVIDED 'Y' =", IF_STATEMENT_END},
            {"NUM_EPHEM n", LOOP_END},
         {"RESERVED_LEN 0 = !", IF_STATEMENT_END}
      };
}

ossimString ossimNitfCsephbDes::getClassName() const
{
   return ossimString("ossimNitfCsephbdes");
}

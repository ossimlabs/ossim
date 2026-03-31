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
         {ossim::nitf::UUID_KW, 36, 0, 0, " "},
         {ossim::nitf::NUMAIS_KW, 3, 0, 0, "ALL"},
         {ossim::nitf::NUMAIS_KW, LOOP_START},
         {ossim::nitf::AISDLVL_KW, 3, 1},
         {ossim::nitf::NUMAIS_KW, LOOP_END},
         {ossim::nitf::NUM_ASSOC_ELEM_KW, 3, 1, 0, "1"},
         {ossim::nitf::NUM_ASSOC_ELEM_KW, LOOP_START},
         {ossim::nitf::ASSOC_ELEM_UUID_KW, 36, 0},
         {ossim::nitf::NUM_ASSOC_ELEM_KW, LOOP_END},
         {ossim::nitf::RESERVEDSUBH_LEN_KW, 4, 1},
         {ossim::nitf::RESERVEDSUBH_KW + " " + ossim::nitf::RESERVEDSUBH_LEN_KW, VARIABLE_LENGTH, 1},
         {ossim::nitf::QUAL_FLAG_EPH_KW, 1, 1, 0, "1"},
         {ossim::nitf::INTERP_TYPE_EPH_KW, 1, 1},
         {ossim::nitf::INTERP_TYPE_EPH_KW + " = 2", IF_STATEMENT_START},
         {ossim::nitf::INTERP_ORDER_EPH_KW, 1, 1},
         {ossim::nitf::INTERP_TYPE_EPH_KW + " = 2", IF_STATEMENT_END},
         {ossim::nitf::EPHEM_FLAG_KW, 1, 1},
         {ossim::nitf::ECI_ECF_EPHEM_KW, 1, 1, 0, "1"},         /*{"ECI_ECF_EPHEM 0 =", IF_STATEMENT_START},  //Assumes desver 1
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
         {ossim::nitf::DT_EPHEM_KW, 13, 3, 9},
         {ossim::nitf::DATE_EPHEM_KW, 8, 1},
         {ossim::nitf::T0_EPHEM_KW, 16, U_DOUBLE, 9},
         {ossim::nitf::NUM_EPHEM_KW, 5, 1},
         {ossim::nitf::NUM_EPHEM_KW, LOOP_START},
            {ossim::nitf::EPHEM_X_KW, 12, 4, 2},
            {ossim::nitf::EPHEM_Y_KW, 12, 4, 2},
            {ossim::nitf::EPHEM_Z_KW, 12, 4, 2},
         {ossim::nitf::NUM_EPHEM_KW, LOOP_END},
         {ossim::nitf::RESERVED_LEN_KW, 9, 1},
         {"! " + ossim::nitf::RESERVED_LEN_KW + " = 0", IF_STATEMENT_START},
            {ossim::nitf::MASK_LEN_KW, 2, 1, 0, "01"},
            {ossim::nitf::RESERVED_FIELD_MASK_KW + " " + ossim::nitf::MASK_LEN_KW, VARIABLE_LENGTH, U_INT, 0, "1"},
            {ossim::nitf::RESERVED_LEN_AREA1_KW, 9, 1},
            {ossim::nitf::ACCEL_PROVIDED_KW, 1, 0, 0, "N"},
            {ossim::nitf::NUM_EPHEM_KW, LOOP_START},
               {ossim::nitf::VEL_X_KW, 12, 4, 2},
               {ossim::nitf::VEL_Y_KW, 12, 4, 2},
               {ossim::nitf::VEL_Z_KW, 12, 4, 2},
               {"^" + ossim::nitf::ACCEL_PROVIDED_KW + " = 'Y'", IF_STATEMENT_START},
                  {ossim::nitf::ACCEL_X_KW, 12, 4, 2},
                  {ossim::nitf::ACCEL_Y_KW, 12, 4, 2},
                  {ossim::nitf::ACCEL_Z_KW, 12, 4, 2},
               {"^" + ossim::nitf::ACCEL_PROVIDED_KW + " = 'Y'", IF_STATEMENT_END},
            {ossim::nitf::NUM_EPHEM_KW, LOOP_END},
         {"! " + ossim::nitf::RESERVED_LEN_KW + " = 0", IF_STATEMENT_END}
      };
}

ossimString ossimNitfCsephbDes::getClassName() const
{
   return ossimString("ossimNitfCsephbdes");
}

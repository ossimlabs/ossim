//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: BCHIPA tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document Vol1-AppAK-MATESA table 6.5 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfBchipaTag.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>
#include <utility> /* make_pair */

static ossimTrace traceDebug("ossimNitfBchipaTag:debug");

static const int FOREVER = 1;

const std::string ossimNitfBchipaTag::CETAG_KW = "BCHIPA";

ossimNitfBchipaTag::ossimNitfBchipaTag()
   : ossimNitfGenericTag("BCHIPA")
{
   initializeFieldDefinitions();
   initializeFields();
   setTagLength(computeTagLength());
}

ossimNitfBchipaTag::ossimNitfBchipaTag(ossim_uint32 tagLength)
   : ossimNitfGenericTag("BCHIPA", tagLength)
{
   initializeFieldDefinitions();

}
void ossimNitfBchipaTag::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
   {
      {"SDE_UUID", 36, ASCII},
      {"NUM_INSTS", 5, U_INT},
      {"INSTANCE", 5, U_INT},
      {"INCLUDE_A", 1, ASCII, 0, "Y"},
      {"INCLUDE_A 'Y' =", IF_STATEMENT_START},
         {"TOT_ORIG_BANDS", 5, U_INT},
         {"TOT_CURR_BANDS", 5, U_INT},
         {"NUM_BWP_IS", 3, U_INT, 0, "1"},
         {"NUM_BWP_IS n", LOOP_START},
            {"BWP_IS", 3, U_INT},
         {"NUM_BWP_IS n", LOOP_END},
         {"NUM_RLVNT_SDE", 3, U_INT, 0, "1"},
         {"NUM_RLVNT_SDE n", LOOP_START},
            {"SDE_NAME", 32, ASCII},
            {"SDE_STATUS", 1, ASCII, 0, "C"},
         {"NUM_RLVNT_SDE n", LOOP_END},
      {"INCLUDE_A 'Y' =", IF_STATEMENT_END},
      {"INCLUDE_B", 1, ASCII, 0, "Y"},
      {"INCLUDE_B 'Y' =", IF_STATEMENT_START},
         {"NUM_ORIGINAL_BANDS", 5, U_INT, 0, "1"},
         {"NUM_ORIGINAL_BANDS n", LOOP_START},
            {"ORIG_BAND_NUMBER", 5, U_INT},
            {"IREPBAND_ORIG", 2, ASCII},
            {"ISUBCAT_ORIG", 8, ASCII},
            {"IFC_ORIG", 1, ASCII, 0, "N"},
            {"IMFLT_ORIG", 3, ASCII},
            {"NLUTS_ORIG", 1, U_INT},
            {"NLUTS_ORIG 0 = !", IF_STATEMENT_START},
               {"NELUT_ORIG", 5, U_INT, 0, "1"},
               {"NLUTS_ORIG m", LOOP_START},
                  {"^NELUT_ORIG p", LOOP_START},
                     {"LUTD_ORIG", 1, ASCII},
                  {"NELUT_ORIG p", LOOP_END},
               {"NLUTS_ORIG m", LOOP_END},
            {"NLUTS_ORIG 0 = !", IF_STATEMENT_END},
         {"NUM_ORIGINAL_BANDS n", LOOP_END},
      {"INCLUDE_B 'Y' =", IF_STATEMENT_END},
      {"INCLUDE_C", 1, ASCII, 0, "Y"},
      {"INCLUDE_C 'Y' =", IF_STATEMENT_START},
         {"NUM_CURR_BANDS", 5, U_INT, 0, "1"},
         {"NUM_CURR_BANDS n", LOOP_START},
            {"CURR_BAND_NUMBER", 5, U_INT},
            {"SEMANTIC_SIZE", 4, U_INT},
            {"SEMANTIC_MEANING SEMANTIC_SIZE", VARIABLE_LENGTH, ASCII},
            {"NUM_ORIG_BANDS", 5, U_INT, 0, "1"},
            {"MAPPING_TYPE", 15, ASCII},
            {"NUM_ORIG_BANDS 0 = !", IF_STATEMENT_START},
               {"NUM_ORIG_BANDS m", LOOP_START},
                  {"ORIG_BND_NUM", 5, U_INT},
                  {"^MAPPING_TYPE 'WEIGHTED' =", IF_STATEMENT_START},
                     {"WEIGHT", 21, SCIENTIFIC},
                  {"^MAPPING_TYPE 'WEIGHTED' =", IF_STATEMENT_END},
               {"NUM_ORIG_BANDS m", LOOP_END},
               {"MAPPING_TYPE 'FORMULAIC' =", IF_STATEMENT_START},
                  {"FORMULA_SIZE", 3, U_INT},
                  {"FORMULA FORMULA_SIZE", VARIABLE_LENGTH, ASCII},
               {"MAPPING_TYPE 'FORMULAIC' =", IF_STATEMENT_END},
            {"NUM_ORIG_BANDS 0 = !", IF_STATEMENT_END},
         {"NUM_CURR_BANDS n", LOOP_END},
      {"INCLUDE_C 'Y' =", IF_STATEMENT_END}
   };
}

ossimString ossimNitfBchipaTag::getClassName() const
{
   return ossimString("ossimNitfBchipaTag");
}

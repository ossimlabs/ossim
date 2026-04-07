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
      {ossim::nitf::SDE_UUID_KW, 36, ASCII},
      {ossim::nitf::NUM_INSTS_KW, 5, U_INT},
      {ossim::nitf::INSTANCE_KW, 5, U_INT},
      {ossim::nitf::INCLUDE_A_KW, 1, ASCII, 0, "Y"},
      {ossim::nitf::INCLUDE_A_KW + " = 'Y'", IF_STATEMENT_START},
         {ossim::nitf::TOT_ORIG_BANDS_KW, 5, U_INT},
         {ossim::nitf::TOT_CURR_BANDS_KW, 5, U_INT},
         {ossim::nitf::NUM_BWP_IS_KW, 3, U_INT, 0, "1"},
         {ossim::nitf::NUM_BWP_IS_KW, LOOP_START},
            {ossim::nitf::BWP_IS_KW, 3, U_INT},
         {ossim::nitf::NUM_BWP_IS_KW, LOOP_END},
         {ossim::nitf::NUM_RLVNT_SDE_KW, 3, U_INT, 0, "1"},
         {ossim::nitf::NUM_RLVNT_SDE_KW, LOOP_START},
            {ossim::nitf::SDE_NAME_KW, 32, ASCII},
            {ossim::nitf::SDE_STATUS_KW, 1, ASCII, 0, "C"},
         {ossim::nitf::NUM_RLVNT_SDE_KW, LOOP_END},
      {ossim::nitf::INCLUDE_A_KW + " = 'Y'", IF_STATEMENT_END},
      {ossim::nitf::INCLUDE_B_KW, 1, ASCII, 0, "Y"},
      {ossim::nitf::INCLUDE_B_KW + " = 'Y'", IF_STATEMENT_START},
         {ossim::nitf::NUM_ORIGINAL_BANDS_KW, 5, U_INT, 0, "1"},
         {ossim::nitf::NUM_ORIGINAL_BANDS_KW, LOOP_START},
            {ossim::nitf::ORIG_BAND_NUMBER_KW, 5, U_INT},
            {ossim::nitf::IREPBAND_ORIG_KW, 2, ASCII},
            {ossim::nitf::ISUBCAT_ORIG_KW, 8, ASCII},
            {ossim::nitf::IFC_ORIG_KW, 1, ASCII, 0, "N"},
            {ossim::nitf::IMFLT_ORIG_KW, 3, ASCII},
            {ossim::nitf::NLUTS_ORIG_KW, 1, U_INT},
            {ossim::nitf::NLUTS_ORIG_KW + " != 0", IF_STATEMENT_START},
               {ossim::nitf::NELUT_ORIG_KW, 5, U_INT, 0, "1"},
               {ossim::nitf::NLUTS_ORIG_KW, LOOP_START},
                  {"^" + ossim::nitf::NELUT_ORIG_KW, LOOP_START},
                     {ossim::nitf::LUTD_ORIG_KW, 1, ASCII},
                  {"^" + ossim::nitf::NELUT_ORIG_KW, LOOP_END},
               {ossim::nitf::NLUTS_ORIG_KW, LOOP_END},
            {ossim::nitf::NLUTS_ORIG_KW + " != 0", IF_STATEMENT_END},
         {ossim::nitf::NUM_ORIGINAL_BANDS_KW, LOOP_END},
      {ossim::nitf::INCLUDE_B_KW + " = 'Y'", IF_STATEMENT_END},
      {ossim::nitf::INCLUDE_C_KW, 1, ASCII, 0, "Y"},
      {ossim::nitf::INCLUDE_C_KW + " = 'Y'", IF_STATEMENT_START},
         {ossim::nitf::NUM_CURR_BANDS_KW, 5, U_INT, 0, "1"},
         {ossim::nitf::NUM_CURR_BANDS_KW, LOOP_START},
            {ossim::nitf::CURR_BAND_NUMBER_KW, 5, U_INT},
            {ossim::nitf::SEMANTIC_SIZE_KW, 4, U_INT},
            {ossim::nitf::SEMANTIC_MEANING_KW + " " + ossim::nitf::SEMANTIC_SIZE_KW, VARIABLE_LENGTH, ASCII},
            {ossim::nitf::NUM_ORIG_BANDS_KW, 5, U_INT, 0, "1"},
            {ossim::nitf::MAPPING_TYPE_KW, 15, ASCII},
            {ossim::nitf::NUM_ORIG_BANDS_KW + " != 0", IF_STATEMENT_START},
               {ossim::nitf::NUM_ORIG_BANDS_KW, LOOP_START},
                  {ossim::nitf::ORIG_BND_NUM_KW, 5, U_INT},
                  {"^" + ossim::nitf::MAPPING_TYPE_KW + " = 'WEIGHTED'", IF_STATEMENT_START},
                     {ossim::nitf::WEIGHT_KW, 21, SCIENTIFIC},
                  {"^" + ossim::nitf::MAPPING_TYPE_KW + " = 'WEIGHTED'", IF_STATEMENT_END},
               {ossim::nitf::NUM_ORIG_BANDS_KW, LOOP_END},
               {ossim::nitf::MAPPING_TYPE_KW + " = 'FORMULAIC'", IF_STATEMENT_START},
                  {ossim::nitf::FORMULA_SIZE_KW, 3, U_INT},
                  {ossim::nitf::FORMULA_KW + " " + ossim::nitf::FORMULA_SIZE_KW, VARIABLE_LENGTH, ASCII},
               {ossim::nitf::MAPPING_TYPE_KW + " = 'FORMULAIC'", IF_STATEMENT_END},
            {ossim::nitf::NUM_ORIG_BANDS_KW + " != 0", IF_STATEMENT_END},
         {ossim::nitf::NUM_CURR_BANDS_KW, LOOP_END},
      {ossim::nitf::INCLUDE_C_KW + " = 'Y'", IF_STATEMENT_END}
   };
}

ossimString ossimNitfBchipaTag::getClassName() const
{
   return ossimString("ossimNitfBchipaTag");
}

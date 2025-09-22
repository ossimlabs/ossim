//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: BANDSB tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.1 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfBandsbTag.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>
#include <utility> /* make_pair */

static ossimTrace traceDebug("ossimNitfBandsbTag:debug");

static const int FOREVER = 1;

const std::string ossimNitfBandsbTag::CETAG_KW = "BANDSB";

ossimNitfBandsbTag::ossimNitfBandsbTag()
   : ossimNitfGenericTag("BANDSB")
{
   // traceDebug.setTraceFlag(true);
   
   initializeFieldDefinitions();
   setTagLength(computeTagLength());
   initializeFields();
}

ossimNitfBandsbTag::ossimNitfBandsbTag(ossim_uint32 tagLength)
   : ossimNitfGenericTag("BANDSB", tagLength)
{
   initializeFieldDefinitions();
   initializeFields();
}
void ossimNitfBandsbTag::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
   {
      {"COUNT", 5, 1},
      {"RADIOMETRICQUANTITY", 24, 0},
      {"RADIOMETRICQUANTITY UNIT", 1, 0},
      {"SCALE FACTOR", 4, 0},
      {"ADDITIVEFACTOR", 4, 0},
      {"ROW_GSD", 7, 3, 3},
      {"ROW_GSD_UNIT", 1, 0},
      {"COL_GSD", 7, 3, 3},
      {"COL_GSD_UNIT", 1, 0},
      {"SPT_RESP_ROW", 7, 3, 3},
      {"SPT_RESP_UNIT_ROW", 1, 0},
      {"SPT_RESP_COL", 7, 3, 3},
      {"SPT_RESP_UNIT_COL", 1, 0},
      {"DATA_FLD_1", 48, 0},
      {"EXISTENCE_MASK", 4, 0},
      {"RADIOMETRICADJUSTMENTSURFACE", 24, 0},
      {"ATMOSPHERICADJUSTMENTALTITUDE", 4, 0},
      {"DIAMETER", 7, 3, 2},
      {"DATA_FLD_2", 32, 0},
      {"WAVE_LENGTH_UNIT", 1, 0},
      {"EXISTENCE_MASK 0 = !", IF_STATEMENT_START},
         {"COUNT n", LOOP_START},
            {"BANDID", 50, 0},
            {"BAD_BAND", 1, 1},
            {"NIIRS", 3, 3, 1},
            {"FOCAL_LEN", 5, 1},
            {"CWAVE", 7, 3, 5},
            {"FWHM", 7, 3, 5},
            {"FWHM_UNC", 7, 3, 5},
            {"NOM_WAVE", 7, 3, 5},
            {"NOM_WAVE_UNC", 7, 3, 5},
            {"LBOUND", 7, 3, 5},
            {"UBOUND", 7, 3, 5},
            {"SCALE FACTOR", 4, 0},
            {"ADDITIVEFACTOR", 4, 0},
            {"START_TIME", 16, 1},
            {"INT_TIME", 6, 3, 5},
            {"CALDRK", 6, 3, 5},
            {"CALIBRATIONSENSITIVITY", 5, 3, 4},
            {"ROW_GSD", 7, 3, 2},
            {"ROW_GSD_UNC", 7, 3, 3},
            {"ROW_GSD_UNIT", 1, 0},
            {"COL_GSD", 7, 3, 2},
            {"COL_GSD_UNC", 7, 3, 2},
            {"COL_GSD_UNIT", 1, 0},
            {"BKNOISE", 5, 3, 4},
            {"SCNNOISE", 5, 3, 4},
            {"SPT_RESP_FUNCTION_ROW", 7, 3, 3},
            {"SPT_RESP_UNCROW", 7, 3, 3},
            {"SPT_RESP_UNIT_ROW", 1, 0},
            {"SPT_RESP_FUNCTION_COL", 7, 3, 3},
            {"SPT_RESP_UNCCOL", 7, 3, 3},
            {"SPT_RESP_UNIT_COL", 1, 0},
            {"DATA_FLD_3", 16, 0},
            {"DATA_FLD_4", 24, 0},
            {"DATA_FLD_5", 32, 0},
            {"DATA_FLD_6", 48, 0},
         {"COUNT n", LOOP_END},
      {"EXISTENCE_MASK 0 = !", IF_STATEMENT_END},
      {"NUM_AUX_B", 2, 1},
      {"NUM_AUX_C", 2, 1},
      {"NUM_AUX_B m", LOOP_START},
         {"BAPF", 1, 0},
         {"UBAP", 7, 0},
         {"COUNT ", LOOP_START},
            {"BAPFm I =", IF_STATEMENT_START},
               {"APN", 10, 1},
               {"APR", 4, 0},
               {"APA", 20, 0},
            {"BAPFm I =", IF_STATEMENT_END},
         {"End of the number of bands loop", LOOP_END},
      {"NUM_AUX_B m", LOOP_END},
      {"NUM_AUX_C k", LOOP_START},
         {"CAPF", 1, 0},
         {"UCAP", 7, 0},
         {"CAPFk I =", IF_STATEMENT_START},
            {"APN", 10, 0},
         {"CAPFk I =", IF_STATEMENT_END},
         {"CAPFk R =", IF_STATEMENT_START},
            {"APR", 4, 0},
         {"CAPFk R =", IF_STATEMENT_END},
         {"CAPFk A =", IF_STATEMENT_START},
            {"APA", 20, 0},
         {"CAPFk A =", IF_STATEMENT_END},
      {"NUM_AUX_C k", LOOP_END},
   };
}

ossimString ossimNitfBandsbTag::getClassName() const
{
   return ossimString("ossimNitfBandsbTag");
}
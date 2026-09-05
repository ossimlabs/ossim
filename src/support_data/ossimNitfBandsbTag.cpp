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
   initializeFields();
   setTagLength(computeTagLength());
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
      {ossim::nitf::COUNT_KW, 5, 1},
      {ossim::nitf::RADIOMETRICQUANTITY_KW, 24, 0, 0, "UNCALIBRATED"},
      {ossim::nitf::RADIOMETRICQUANTITY_UNIT_KW, 1, 0, 0, "U"},
      {ossim::nitf::SCALE_FACTOR_KW, 4, 0, 0, "1"},
      {ossim::nitf::ADDITIVEFACTOR_KW, 4, 0, 0, "0"},
      {ossim::nitf::ROW_GSD_KW, 7, 3, 3},
      {ossim::nitf::ROW_GSD_UNIT_KW, 1, 0, 0, "M"},
      {ossim::nitf::COL_GSD_KW, 7, 3, 3},
      {ossim::nitf::COL_GSD_UNIT_KW, 1, 0, 0, "M"},
      {ossim::nitf::SPT_RESP_ROW_KW, 7, 3, 3},
      {ossim::nitf::SPT_RESP_UNIT_ROW_KW, 1, 0},
      {ossim::nitf::SPT_RESP_COL_KW, 7, 3, 3},
      {ossim::nitf::SPT_RESP_UNIT_COL_KW, 1, 0},
      {ossim::nitf::DATA_FLD_1_KW, 48, 0},
      {ossim::nitf::EXISTENCE_MASK_KW, 4, 0, 0, ossimString(std::string{char(159), char(199), char(16), char(0)})},//10011111 11000111 00010000 00000000
      {ossim::nitf::RADIOMETRICADJUSTMENTSURFACE_KW, 24, 0, 0, "APERTURE"},
      {ossim::nitf::ATMOSPHERICADJUSTMENTALTITUDE_KW, 4, 0, 0, "NaN"},
      //{ossim::nitf::DIAMETER_KW, 7, 3, 2},
      //{ossim::nitf::DATA_FLD_2_KW, 32, 0},
      {ossim::nitf::WAVE_LENGTH_UNIT_KW, 1, 0},
      {ossim::nitf::COUNT_KW, LOOP_START},
         {ossim::nitf::BANDID_KW, 50, 0},
         {ossim::nitf::BAD_BAND_KW, 1, 1},
         {ossim::nitf::NIIRS_KW, 3, 3, 1},
         {ossim::nitf::FOCAL_LEN_KW, 5, 1},
         {ossim::nitf::CWAVE_KW, 7, 3, 5},
         {ossim::nitf::FWHM_KW, 7, 3, 5},
         {ossim::nitf::FWHM_UNC_KW, 7, 3, 5},
         //{ossim::nitf::NOM_WAVE_KW, 7, 3, 5},
         //{ossim::nitf::NOM_WAVE_UNC_KW, 7, 3, 5},
         //{ossim::nitf::LBOUND_KW, 7, 3, 5},
         //{ossim::nitf::UBOUND_KW, 7, 3, 5},
         {ossim::nitf::SCALE_FACTOR_KW, 4, 0, 0, "1"},
         {ossim::nitf::ADDITIVEFACTOR_KW, 4, 0, 0, "0"},
         // START_TIMEn is BCS-N, format YYMMDDhhmmss.sss (STDI-0002 Vol 1 App X) -
         // it contains a decimal point, so it cannot be parsed as an unsigned
         // integer (U_INT saturates to UINT32_MAX). Treat it as ASCII so the
         // already-formatted 16-character string passes through verbatim.
         {ossim::nitf::START_TIME_KW, 16, 0},
         {ossim::nitf::INT_TIME_KW, 6, 3, 5},
         //{ossim::nitf::CALDRK_KW, 6, 3, 5},
         //{ossim::nitf::CALIBRATIONSENSITIVITY_KW, 5, 3, 4},
         //{ossim::nitf::ROW_GSD_KW, 7, 3, 2},
         //{ossim::nitf::ROW_GSD_UNC_KW, 7, 3, 3},
         //{ossim::nitf::ROW_GSD_UNIT_KW, 1, 0},
         //{ossim::nitf::COL_GSD_KW, 7, 3, 2},
         //{ossim::nitf::COL_GSD_UNC_KW, 7, 3, 2},
         //{ossim::nitf::COL_GSD_UNIT_KW, 1, 0},
         {ossim::nitf::BKNOISE_KW, 5, 3, 4},
         {ossim::nitf::SCNNOISE_KW, 5, 3, 4},
         //{ossim::nitf::SPT_RESP_FUNCTION_ROW_KW, 7, 3, 3},
         //{ossim::nitf::SPT_RESP_UNCROW_KW, 7, 3, 3},
         //{ossim::nitf::SPT_RESP_UNIT_ROW_KW, 1, 0},
         //{ossim::nitf::SPT_RESP_FUNCTION_COL_KW, 7, 3, 3},
         //{ossim::nitf::SPT_RESP_UNCCOL_KW, 7, 3, 3},
         //{ossim::nitf::SPT_RESP_UNIT_COL_KW, 1, 0},
         //{ossim::nitf::DATA_FLD_3_KW, 16, 0},
         //{ossim::nitf::DATA_FLD_4_KW, 24, 0},
         //{ossim::nitf::DATA_FLD_5_KW, 32, 0},
         //{ossim::nitf::DATA_FLD_6_KW, 48, 0},
      {ossim::nitf::COUNT_KW, LOOP_END}
      /*, {ossim::nitf::NUM_AUX_B_KW, 2, 1},
      {ossim::nitf::NUM_AUX_C_KW, 2, 1},
      {ossim::nitf::NUM_AUX_B_KW, LOOP_START},
         {ossim::nitf::BAPF_KW, 1, 0},
         {ossim::nitf::UBAP_KW, 7, 0},
         {ossim::nitf::COUNT_KW, LOOP_START},
            {ossim::nitf::BAPF_KW + "m I =", IF_STATEMENT_START},
               {ossim::nitf::APN_KW, 10, 1},
               {ossim::nitf::APR_KW, 4, 0},
               {ossim::nitf::APA_KW, 20, 0},
            {ossim::nitf::BAPF_KW + "m I =", IF_STATEMENT_END},
         {"End of the number of bands loop", LOOP_END},
      {ossim::nitf::NUM_AUX_B_KW, LOOP_END},
      {ossim::nitf::NUM_AUX_C_KW, LOOP_START},
         {ossim::nitf::CAPF_KW, 1, 0},
         {ossim::nitf::UCAP_KW, 7, 0},
         {ossim::nitf::CAPF_KW + "k I =", IF_STATEMENT_START},
            {ossim::nitf::APN_KW, 10, 0},
         {ossim::nitf::CAPF_KW + "k I =", IF_STATEMENT_END},
         {ossim::nitf::CAPF_KW + "k R =", IF_STATEMENT_START},
            {ossim::nitf::APR_KW, 4, 0},
         {ossim::nitf::CAPF_KW + "k R =", IF_STATEMENT_END},
         {ossim::nitf::CAPF_KW + "k A =", IF_STATEMENT_START},
            {ossim::nitf::APA_KW, 20, 0},
         {ossim::nitf::CAPF_KW + "k A =", IF_STATEMENT_END},
      {ossim::nitf::NUM_AUX_C_KW, LOOP_END},*/
   };
}

ossimString ossimNitfBandsbTag::getClassName() const
{
   return ossimString("ossimNitfBandsbTag");
}
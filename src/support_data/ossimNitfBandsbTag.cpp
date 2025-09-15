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
// See document App AH - GLAS-GFM table AH.6.6 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfBandsbTag.h>

ossimNitfBandsbTag::ossimNitfBandsbTag(ossim_uint32 tagLength)
   : ossimNitfGenericTag("BANDSB", tagLength)
{
   FIELD_DEFINITIONS =
   {
      {"COUNT", 5, 1, "0"},
      {"RADIOMETRICQUANTITY", 24, 0, " "},
      {"RADIOMETRICQUANTITY UNIT", 1, 0, " "},
      {"SCALE FACTOR", 4, 0, "0"},
      {"ADDITIVEFACTOR", 4, 0, "0"},
      {"ROW_GSD", 7, 3, 3, "0"},
      {"ROW_GSD_UNIT", 1, 0, " "},
      {"COL_GSD", 7, 3, 3, "0"},
      {"COL_GSD_UNIT", 1, 0, " "},
      {"SPT_RESP_ROW", 7, 3, 3, "0"},
      {"SPT_RESP_UNIT_ROW", 1, 0, " "},
      {"SPT_RESP_COL", 7, 3, 3, "0"},
      {"SPT_RESP_UNIT_COL", 1, 0, " "},
      {"DATA_FLD_1", 48, 0, " "},
      {"EXISTENCE_MASK", 4, 0, "0"},
      {"RADIOMETRICADJUSTMENTSURFACE", 24, 0, " "},
      {"ATMOSPHERICADJUSTMENTALTITUDE", 4, 0, "0"},
      {"DIAMETER", 7, 3, 2, "0"},
      {"DATA_FLD_2", 32, 0, "0"},
      {"WAVE_LENGTH_UNIT", 1, 0, " "},
      {"EXISTENCE_MASK 0 = !", IF_STATEMENT_START},
         {"COUNT n", LOOP_START},
            {"BANDID", 50, 0, " "},
            {"BAD_BAND", 1, 1, "0"},
            {"NIIRS", 3, 3, 1, "0"},
            {"FOCAL_LEN", 5, 1, "0"},
            {"CWAVE", 7, 3, 5, "0"},
            {"FWHM", 7, 3, 5, "0"},
            {"FWHM_UNC", 7, 3, 5, "0"},
            {"NOM_WAVE", 7, 3, 5, "0"},
            {"NOM_WAVE_UNC", 7, 3, 5, "0"},
            {"LBOUND", 7, 3, 5, "0"},
            {"UBOUND", 7, 3, 5, "0"},
            {"SCALE FACTOR", 4, 0, "0"},
            {"ADDITIVEFACTOR", 4, 0, "0"},
            {"START_TIME", 16, 1, "0"},
            {"INT_TIME", 6, 3, 5, "0"},
            {"CALDRK", 6, 3, 5, "0"},
            {"CALIBRATIONSENSITIVITY", 5, 3, 4, "0"},
            {"ROW_GSD", 7, 3, 2, "0"},
            {"ROW_GSD_UNC", 7, 3, 3, "0"},
            {"ROW_GSD_UNIT", 1, 0, " "},
            {"COL_GSD", 7, 3, 2, "0"},
            {"COL_GSD_UNC", 7, 3, 2, "0"},
            {"COL_GSD_UNIT", 1, 0, " "},
            {"BKNOISE", 5, 3, 4, "0"},
            {"SCNNOISE", 5, 3, 4, "0"},
            {"SPT_RESP_FUNCTION_ROW", 7, 3, 3, "0"},
            {"SPT_RESP_UNCROW", 7, 3, 3, "0"},
            {"SPT_RESP_UNIT_ROW", 1, 0, " "},
            {"SPT_RESP_FUNCTION_COL", 7, 3, 3, "0"},
            {"SPT_RESP_UNCCOL", 7, 3, 3, "0"},
            {"SPT_RESP_UNIT_COL", 1, 0, " "},
            {"DATA_FLD_3", 16, 0, "0"},
            {"DATA_FLD_4", 24, 0, "0"},
            {"DATA_FLD_5", 32, 0, "0"},
            {"DATA_FLD_6", 48, 0, "0"},
         {"COUNT n", LOOP_END},
      {"EXISTENCE_MASK 0 = !", IF_STATEMENT_END},
      {"NUM_AUX_B", 2, 1, "0"},
      {"NUM_AUX_C", 2, 1, "0"},
      {"NUM_AUX_B m", LOOP_START},
         {"BAPF", 1, 0, " "},
         {"UBAP", 7, 0, " "},
         {"COUNT ", LOOP_START},
            {"BAPFm I =", IF_STATEMENT_START},
               {"APN", 10, 1, "0"},
               {"APR", 4, 0, "0"},
               {"APA", 20, 0, " "},
            {"BAPFm I =", IF_STATEMENT_END},
         {"End of the number of bands loop", LOOP_END},
      {"NUM_AUX_B m", LOOP_END},
      {"NUM_AUX_C k", LOOP_START},
         {"CAPF", 1, 0, " "},
         {"UCAP", 7, 0, " "},
         {"CAPFk I =", IF_STATEMENT_START},
            {"APN", 10, 0, " "},
         {"CAPFk I =", IF_STATEMENT_END},
         {"CAPFk R =", IF_STATEMENT_START},
            {"APR", 4, 0, "0"},
         {"CAPFk R =", IF_STATEMENT_END},
         {"CAPFk A =", IF_STATEMENT_START},
            {"APA", 20, 0, " "},
         {"CAPFk A =", IF_STATEMENT_END},
      {"NUM_AUX_C k", LOOP_END},
   };
}
//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSSFAB des class definition.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.6 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfCssfabDes.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>

static ossimTrace traceDebug("ossimNitfCssfabdes:debug");

const std::string ossimNitfCssfabDes::CETAG_KW = "CSSFAB";

ossimNitfCssfabDes::ossimNitfCssfabDes()
   : ossimNitfGenericDes("CSSFAB")
{
   // traceDebug.setTraceFlag(true);

   initializeFieldDefinitions();
   initializeFields();
   setDesSubHeaderLength(getDesSubHeaderLength());
   setDesDataLength(getDesDataLength());
}

ossimNitfCssfabDes::ossimNitfCssfabDes(ossim_uint32 tagLength)
   : ossimNitfGenericDes("CSSFAB", tagLength)
{
   initializeFieldDefinitions();
   initializeFields();
   setDesSubHeaderLength(getDesSubHeaderLength());
}
void ossimNitfCssfabDes::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
      {
   {"SENSOR_TYPE", 1, ASCII, 0, "S"},
   {"BAND_TYPE", 1},
   {"BAND_WAVELENGTH", 11, U_DOUBLE, 8},
   {"N_BANDS", 5, U_INT},
   {"N_BANDS i", LOOP_START},
      {"BAND_INDEX", 5, U_INT},
      {"IREPBAND", 2},
      {"ISUBCAT", 6},
   {"N_BANDS i", LOOP_END},
   {"NUM_FL_PTS", 3, INT, 0, "1"},
   {"FL_INTERP", 1, INT, 0, "1"},
   {"FOC_LENGTH_DATE", 8},
   {"NUM_FL_PTS n", LOOP_START},
      {"FOC_LENGTH_TIME", 15, U_DOUBLE, 9},
      {"FOC_LENGTH", 11, U_DOUBLE, 8},
   {"NUM_FL_PTS n", LOOP_END},
   {"PPOFF_X", 10, DOUBLE, 6},
   {"PPOFF_Y", 10, DOUBLE, 6},
   {"PPOFF_Z", 10, DOUBLE, 7},
   {"ANGOFF_X", 10, DOUBLE, 7, "0"},
   {"ANGOFF_Y", 10, DOUBLE, 7, "0"},
   {"ANGOFF_Z", 10, DOUBLE, 7, "-1.5707963"},
   {"SENSOR_TYPE 'S' =", IF_STATEMENT_START},
      {"SMPL_NUM_FIRST", 12, DOUBLE, 5},
      {"DELTA_SMPL_PAIRS", 11, U_DOUBLE, 5},
      {"NUM_FA_PAIRS", 3, U_INT},
      {"NUM_FA_PAIRS n", LOOP_START},
         {"START_FALIGN_X", 11, DOUBLE, 7},
         {"START_FALIGN_Y", 11, DOUBLE, 7},
         {"END_FALIGN_X", 11, DOUBLE, 7},
         {"END_FALIGN_Y", 11, DOUBLE, 7},
      {"NUM_FA_PAIRS n", LOOP_END},
   {"SENSOR_TYPE 'S' =", IF_STATEMENT_END},
   {"SENSOR_TYPE 'F' =", IF_STATEMENT_START},
      {"NUM_SETS_FA_DATA", 1, U_INT},
      {"FIELD_ANGLE_TYPE", 1, U_INT},
      {"FA_INTERP", 1, U_INT},
      {"FIELD_ANGLE_TYPE 0 =", IF_STATEMENT_START},
         {"NUM_SETS_FA_DATA n", LOOP_START},
            {"FL_CAL", 11, U_DOUBLE, 8},
            {"NUM_FIR_LINE", 12, DOUBLE, 5},
            {"DELTA_LINE", 11, U_DOUBLE, 5},
            {"NUM_FA_BLOCKS_LINE", 3, U_INT},
            {"NUM_FIR_SAMP", 12, DOUBLE, 5},
            {"DELTA_SAMP", 11, DOUBLE, 5},
            {"NUM_FA_BLOCKS_SAMP", 3, U_INT},
            {"NUM_FA_BLOCKS_LINE i", LOOP_START},
               {"NUM_FA_BLOCKS_SAMP j", LOOP_START},
                  {"FA_X1", 11, DOUBLE, 7},
                  {"FA_Y1", 11, DOUBLE, 7},
                  {"FA_X2", 11, DOUBLE, 7},
                  {"FA_Y2", 11, DOUBLE, 7},
                  {"FA_X3", 11, DOUBLE, 7},
                  {"FA_Y3", 11, DOUBLE, 7},
                  {"FA_X4", 11, DOUBLE, 7},
                  {"FA_Y4", 11, DOUBLE, 7},
               {"NUM_FA_BLOCKS_SAMP j", LOOP_END},
            {"NUM_FA_BLOCKS_LINE i", LOOP_END},
         {"NUM_SETS_FA_DATA n", LOOP_END},
      {"FIELD_ANGLE_TYPE == 0", IF_STATEMENT_END},
      {"FIELD_ANGLE_TYPE 1 =", IF_STATEMENT_START},
         {"NUM_FP_ARRAYS_LINE", 3, U_INT},
         {"NUM_FP_ARRAYS_SAMP", 3, U_INT},
         {"NUM_FP_ARRAYS_LINE n", LOOP_START},
            {"NUM_FP_ARRAYS_SAMP j", LOOP_START},
            {"LS_FID_TRANS_T0", 21, SCIENTIFIC},
            {"LS_FID_TRANS_T1", 21, SCIENTIFIC},
            {"LS_FID_TRANS_T2", 21, SCIENTIFIC},
            {"LS_FID_TRANS_T3", 21, SCIENTIFIC},
            {"LS_FID_TRANS_T4", 21, SCIENTIFIC},
            {"LS_FID_TRANS_T5", 21, SCIENTIFIC},
            {"LS_FID_TRANS_T6", 21, SCIENTIFIC},
            {"LS_FID_TRANS_T7", 21, SCIENTIFIC},
            {"NUM_FP_ARRAYS_LINE n", LOOP_END},
         {"NUM_FP_ARRAYS_SAMP j", LOOP_END},
         {"NUM_SETS_FA_DATA n", LOOP_START},
            {"FL_CAL_IOP", 11, U_DOUBLE, 8},
            {"PPO_X0", 21, SCIENTIFIC},
            {"PPO_Y0", 21, SCIENTIFIC},
            {"RLD_K0", 21, SCIENTIFIC},
            {"RLD_K1", 21, SCIENTIFIC},
            {"RLD_K2", 21, SCIENTIFIC},
            {"RLD_K3", 21, SCIENTIFIC},
            {"DCD_P1", 21, SCIENTIFIC},
            {"DCD_P2", 21, SCIENTIFIC},
            {"DCD_P3", 21, SCIENTIFIC},
            {"AD_A1", 21, SCIENTIFIC},
            {"AD_A2", 21, SCIENTIFIC},
            {"RADIUS_OF_VALIDITY", 21, SCIENTIFIC},
         {"NUM_SETS_FA_DATA n", LOOP_END},
      {"FIELD_ANGLE_TYPE == 1", IF_STATEMENT_END},
      {"TELESCOPE_OPTICS_FLAG", 1},
      {"TELESCOPE_OPTICS_FLAG 1 =", IF_STATEMENT_START},
         {"NUM_TELE_SETS_FA_DATA", 1},
         {"N_FRAMES", 4, U_INT},
         {"N_FRAMES n", LOOP_START},
            {"TELE_TRANS_T0", 21, SCIENTIFIC},
            {"TELE_TRANS_T1", 21, SCIENTIFIC},
            {"TELE_TRANS_T2", 21, SCIENTIFIC},
            {"TELE_TRANS_T3", 21, SCIENTIFIC},
            {"TELE_TRANS_T4", 21, SCIENTIFIC},
            {"TELE_TRANS_T5", 21, SCIENTIFIC},
            {"TELE_TRANS_T6", 21, SCIENTIFIC},
            {"TELE_TRANS_T7", 21, SCIENTIFIC},
         {"N_FRAMES n", LOOP_END},
         {"NUM_TELE_SETS_FA_DATA n", LOOP_START},
            {"FL_CAL_IOP_TELE", 11, U_DOUBLE, 8},
            {"PPO_X0_TELE", 21, SCIENTIFIC},
            {"PPO_Y0_TELE", 21, SCIENTIFIC},
            {"RLD_K0_TELE", 21, SCIENTIFIC},
            {"RLD_K1_TELE", 21, SCIENTIFIC},
            {"RLD_K2_TELE", 21, SCIENTIFIC},
            {"RLD_K3_TELE", 21, SCIENTIFIC},
            {"DCD_P1_TELE", 21, SCIENTIFIC},
            {"DCD_P2_TELE", 21, SCIENTIFIC},
            {"DCD_P3_TELE", 21, SCIENTIFIC},
            {"AD_A1_TELE", 21, SCIENTIFIC},
            {"AD_A2_TELE", 21, SCIENTIFIC},
            {"RADIUS_OF_VALIDITY_TELE", 21, SCIENTIFIC},
         {"NUM_TELE_SETS_FA_DATA n", LOOP_END},
      {"TELESCOPE_OPTICS_FLAG == 1", IF_STATEMENT_END},
      {"TELESCOPE_OPTICS_FLAG 2 =", IF_STATEMENT_START},
         {"NUM_TELE_SETS_FA_DATA", 1},
         {"N_FRAME_TIMES", 4, U_INT},
         {"N_VARYING_IO", 2, U_INT},
         {"N_VARYING_IO n", LOOP_START},
            {"TIME_VARYING_IO_PARM_ID", 2, U_INT}, //
         {"N_VARYING_IO n", LOOP_END},
         {"TELE_DATE", 8},
         {"N_FRAME_TIMES n", LOOP_START},
            {"TELE_TIME", 15, U_DOUBLE, 9}, //
            {"TELE_TRANS_T0", 21, SCIENTIFIC},
            {"TELE_TRANS_T1", 21, SCIENTIFIC},
            {"TELE_TRANS_T2", 21, SCIENTIFIC},
            {"TELE_TRANS_T3", 21, SCIENTIFIC},
            {"TELE_TRANS_T4", 21, SCIENTIFIC},
            {"TELE_TRANS_T5", 21, SCIENTIFIC},
            {"TELE_TRANS_T6", 21, SCIENTIFIC},
            {"TELE_TRANS_T7", 21, SCIENTIFIC},
            {"N_VARYING_IO m", LOOP_START},
               {"TIME_VARYING_IO_M", 21, SCIENTIFIC},
            {"N_VARYING_IO m", LOOP_END},
         {"N_FRAME_TIMES n", LOOP_END},
         {"NUM_TELE_SETS_FA_DATA", LOOP_START},
            {"FL_CAL_IOP_TELE", 11, U_DOUBLE, 8},
            {"PPO_X0_TELE", 21, SCIENTIFIC},
            {"PPO_Y0_TELE", 21, SCIENTIFIC},
            {"RLD_K0_TELE", 21, SCIENTIFIC},
            {"RLD_K1_TELE", 21, SCIENTIFIC},
            {"RLD_K2_TELE", 21, SCIENTIFIC},
            {"RLD_K3_TELE", 21, SCIENTIFIC},
            {"DCD_P1_TELE", 21, SCIENTIFIC},
            {"DCD_P2_TELE", 21, SCIENTIFIC},
            {"DCD_P3_TELE", 21, SCIENTIFIC},
            {"AD_A1_TELE", 21, SCIENTIFIC},
            {"AD_A2_TELE", 21, SCIENTIFIC},
            {"RADIUS_OF_VALIDITY_TELE", 21, SCIENTIFIC},
         {"NUM_TELE_SETS_FA_DATA", LOOP_END},
      {"TELESCOPE_OPTICS_FLAG == 2", IF_STATEMENT_END},
   {"SENSOR_TYPE == F", IF_STATEMENT_END},
   {"RESERVED_LEN", 9, U_INT}
   };
}

ossimString ossimNitfCssfabDes::getClassName() const
{
   return ossimString("ossimNitfCssfabdes");
}

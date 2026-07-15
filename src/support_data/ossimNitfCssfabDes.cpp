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
      {ossim::nitf::UUID_KW, 36},
      {ossim::nitf::NUMAIS_KW, 3, U_INT},
      {ossim::nitf::NUMAIS_KW, LOOP_START},
         {ossim::nitf::AISDLVL_KW, 3, U_INT},
      {ossim::nitf::NUMAIS_KW, LOOP_END},
      {ossim::nitf::NUM_ASSOC_ELEM_KW, 3, U_INT},
      {ossim::nitf::NUM_ASSOC_ELEM_KW, LOOP_START},
         {ossim::nitf::ASSOC_ELEM_UUID_KW, 36},
      {ossim::nitf::NUM_ASSOC_ELEM_KW, LOOP_END},
      {ossim::nitf::RESERVEDSUBH_LEN_KW, 4},
      {ossim::nitf::RESERVEDSUBH_KW + " " + ossim::nitf::RESERVEDSUBH_LEN_KW, VARIABLE_LENGTH},
      {ossim::nitf::SENSOR_TYPE_KW, 1, ASCII, 0, "S"},
      {ossim::nitf::BAND_TYPE_KW, 1},
      {ossim::nitf::BAND_WAVELENGTH_KW, 11, U_DOUBLE, 8},
      {ossim::nitf::N_BANDS_KW, 5, U_INT},
      {ossim::nitf::N_BANDS_KW, LOOP_START},
         {ossim::nitf::BAND_INDEX_KW, 5, U_INT},
         {ossim::nitf::IREPBAND_KW, 2},
         {ossim::nitf::ISUBCAT_KW, 6},
      {ossim::nitf::N_BANDS_KW, LOOP_END},
      {ossim::nitf::NUM_FL_PTS_KW, 3, INT, 0, "1"},
      {ossim::nitf::FL_INTERP_KW, 1, INT, 0, "1"},
      {ossim::nitf::FOC_LENGTH_DATE_KW, 8},
      {ossim::nitf::NUM_FL_PTS_KW, LOOP_START},
         {ossim::nitf::FOC_LENGTH_TIME_KW, 15, U_DOUBLE, 9},
         {ossim::nitf::FOC_LENGTH_KW, 11, U_DOUBLE, 8},
      {ossim::nitf::NUM_FL_PTS_KW, LOOP_END},
      {ossim::nitf::PPOFF_X_KW, 10, DOUBLE, 6},
      {ossim::nitf::PPOFF_Y_KW, 10, DOUBLE, 6},
      {ossim::nitf::PPOFF_Z_KW, 10, DOUBLE, 6},
      {ossim::nitf::ANGOFF_X_KW, 10, DOUBLE, 7, "0"},
      {ossim::nitf::ANGOFF_Y_KW, 10, DOUBLE, 7, "0"},
      {ossim::nitf::ANGOFF_Z_KW, 10, DOUBLE, 7, "0"},
      {ossim::nitf::SENSOR_TYPE_KW + " 'S' =", IF_STATEMENT_START},
         {ossim::nitf::SMPL_NUM_FIRST_KW, 12, DOUBLE, 5},
         {ossim::nitf::DELTA_SMPL_PAIRS_KW, 11, U_DOUBLE, 5},
         {ossim::nitf::NUM_FA_PAIRS_KW, 3, U_INT},
         {ossim::nitf::NUM_FA_PAIRS_KW, LOOP_START},
            {ossim::nitf::START_FALIGN_X_KW, 11, DOUBLE, 7},
            {ossim::nitf::START_FALIGN_Y_KW, 11, DOUBLE, 7},
            {ossim::nitf::END_FALIGN_X_KW, 11, DOUBLE, 7},
            {ossim::nitf::END_FALIGN_Y_KW, 11, DOUBLE, 7},
         {ossim::nitf::NUM_FA_PAIRS_KW, LOOP_END},
      {ossim::nitf::SENSOR_TYPE_KW + " 'S' =", IF_STATEMENT_END},
      {ossim::nitf::SENSOR_TYPE_KW + " 'F' =", IF_STATEMENT_START},
         {ossim::nitf::NUM_SETS_FA_DATA_KW, 1, U_INT},
         {ossim::nitf::FIELD_ANGLE_TYPE_KW, 1, U_INT},
         {ossim::nitf::FA_INTERP_KW, 1, U_INT},
         {ossim::nitf::FIELD_ANGLE_TYPE_KW + " 0 =", IF_STATEMENT_START},
            {ossim::nitf::NUM_SETS_FA_DATA_KW, LOOP_START},
               {ossim::nitf::FL_CAL_KW, 11, U_DOUBLE, 8},
               {ossim::nitf::NUM_FIR_LINE_KW, 12, DOUBLE, 5},
               {ossim::nitf::DELTA_LINE_KW, 11, U_DOUBLE, 5},
               {ossim::nitf::NUM_FA_BLOCKS_LINE_KW, 3, U_INT},
               {ossim::nitf::NUM_FIR_SAMP_KW, 12, DOUBLE, 5},
               {ossim::nitf::DELTA_SAMP_KW, 11, DOUBLE, 5},
               {ossim::nitf::NUM_FA_BLOCKS_SAMP_KW, 3, U_INT},
               {ossim::nitf::NUM_FA_BLOCKS_LINE_KW, LOOP_START},
                  {ossim::nitf::NUM_FA_BLOCKS_SAMP_KW, LOOP_START},
                     {ossim::nitf::FA_X1_KW, 11, DOUBLE, 7},
                     {ossim::nitf::FA_Y1_KW, 11, DOUBLE, 7},
                     {ossim::nitf::FA_X2_KW, 11, DOUBLE, 7},
                     {ossim::nitf::FA_Y2_KW, 11, DOUBLE, 7},
                     {ossim::nitf::FA_X3_KW, 11, DOUBLE, 7},
                     {ossim::nitf::FA_Y3_KW, 11, DOUBLE, 7},
                     {ossim::nitf::FA_X4_KW, 11, DOUBLE, 7},
                     {ossim::nitf::FA_Y4_KW, 11, DOUBLE, 7},
                  {ossim::nitf::NUM_FA_BLOCKS_SAMP_KW, LOOP_END},
               {ossim::nitf::NUM_FA_BLOCKS_LINE_KW, LOOP_END},
            {ossim::nitf::NUM_SETS_FA_DATA_KW, LOOP_END},
         {ossim::nitf::FIELD_ANGLE_TYPE_KW + " == 0", IF_STATEMENT_END},
         {ossim::nitf::FIELD_ANGLE_TYPE_KW + " 1 =", IF_STATEMENT_START},
            {ossim::nitf::NUM_FP_ARRAYS_LINE_KW, 3, U_INT},
            {ossim::nitf::NUM_FP_ARRAYS_SAMP_KW, 3, U_INT},
            {ossim::nitf::NUM_FP_ARRAYS_LINE_KW, LOOP_START},
               {ossim::nitf::NUM_FP_ARRAYS_SAMP_KW, LOOP_START},
               {ossim::nitf::LS_FID_TRANS_T0_KW, 21, SCIENTIFIC},
               {ossim::nitf::LS_FID_TRANS_T1_KW, 21, SCIENTIFIC},
               {ossim::nitf::LS_FID_TRANS_T2_KW, 21, SCIENTIFIC},
               {ossim::nitf::LS_FID_TRANS_T3_KW, 21, SCIENTIFIC},
               {ossim::nitf::LS_FID_TRANS_T4_KW, 21, SCIENTIFIC},
               {ossim::nitf::LS_FID_TRANS_T5_KW, 21, SCIENTIFIC},
               {ossim::nitf::LS_FID_TRANS_T6_KW, 21, SCIENTIFIC},
               {ossim::nitf::LS_FID_TRANS_T7_KW, 21, SCIENTIFIC},
               {ossim::nitf::NUM_FP_ARRAYS_LINE_KW, LOOP_END},
            {ossim::nitf::NUM_FP_ARRAYS_SAMP_KW, LOOP_END},
            {ossim::nitf::NUM_SETS_FA_DATA_KW, LOOP_START},
               {ossim::nitf::FL_CAL_IOP_KW, 11, U_DOUBLE, 8},
               {ossim::nitf::PPO_X0_KW, 21, SCIENTIFIC},
               {ossim::nitf::PPO_Y0_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K0_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K1_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K2_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K3_KW, 21, SCIENTIFIC},
               {ossim::nitf::DCD_P1_KW, 21, SCIENTIFIC},
               {ossim::nitf::DCD_P2_KW, 21, SCIENTIFIC},
               {ossim::nitf::DCD_P3_KW, 21, SCIENTIFIC},
               {ossim::nitf::AD_A1_KW, 21, SCIENTIFIC},
               {ossim::nitf::AD_A2_KW, 21, SCIENTIFIC},
               {ossim::nitf::RADIUS_OF_VALIDITY_KW, 21, SCIENTIFIC},
            {ossim::nitf::NUM_SETS_FA_DATA_KW, LOOP_END},
         {ossim::nitf::FIELD_ANGLE_TYPE_KW + " == 1", IF_STATEMENT_END},
         {ossim::nitf::TELESCOPE_OPTICS_FLAG_KW, 1},
         {ossim::nitf::TELESCOPE_OPTICS_FLAG_KW + " 1 =", IF_STATEMENT_START},
            {ossim::nitf::NUM_TELE_SETS_FA_DATA_KW, 1},
            {ossim::nitf::N_FRAMES_KW, 4, U_INT},
            {ossim::nitf::N_FRAMES_KW, LOOP_START},
               {ossim::nitf::TELE_TRANS_T0_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T1_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T2_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T3_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T4_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T5_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T6_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T7_KW, 21, SCIENTIFIC},
            {ossim::nitf::N_FRAMES_KW, LOOP_END},
            {ossim::nitf::NUM_TELE_SETS_FA_DATA_KW, LOOP_START},
               {ossim::nitf::FL_CAL_IOP_TELE_KW, 11, U_DOUBLE, 8},
               {ossim::nitf::PPO_X0_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::PPO_Y0_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K0_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K1_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K2_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K3_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::DCD_P1_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::DCD_P2_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::DCD_P3_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::AD_A1_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::AD_A2_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::RADIUS_OF_VALIDITY_TELE_KW, 21, SCIENTIFIC},
            {ossim::nitf::NUM_TELE_SETS_FA_DATA_KW, LOOP_END},
         {ossim::nitf::TELESCOPE_OPTICS_FLAG_KW + " == 1", IF_STATEMENT_END},
         {ossim::nitf::TELESCOPE_OPTICS_FLAG_KW + " 2 =", IF_STATEMENT_START},
            {ossim::nitf::NUM_TELE_SETS_FA_DATA_KW, 1},
            {ossim::nitf::N_FRAME_TIMES_KW, 4, U_INT},
            {ossim::nitf::N_VARYING_IO_KW, 2, U_INT},
            {ossim::nitf::N_VARYING_IO_KW, LOOP_START},
               {ossim::nitf::TIME_VARYING_IO_PARM_ID_KW, 2, U_INT},
            {ossim::nitf::N_VARYING_IO_KW, LOOP_END},
            {ossim::nitf::TELE_DATE_KW, 8},
            {ossim::nitf::N_FRAME_TIMES_KW, LOOP_START},
               {ossim::nitf::TELE_TIME_KW, 15, U_DOUBLE, 9},
               {ossim::nitf::TELE_TRANS_T0_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T1_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T2_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T3_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T4_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T5_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T6_KW, 21, SCIENTIFIC},
               {ossim::nitf::TELE_TRANS_T7_KW, 21, SCIENTIFIC},
               {ossim::nitf::N_VARYING_IO_KW, LOOP_START},
                  {ossim::nitf::TIME_VARYING_IO_M_KW, 21, SCIENTIFIC},
               {ossim::nitf::N_VARYING_IO_KW, LOOP_END},
            {ossim::nitf::N_FRAME_TIMES_KW, LOOP_END},
            {ossim::nitf::NUM_TELE_SETS_FA_DATA_KW, LOOP_START},
               {ossim::nitf::FL_CAL_IOP_TELE_KW, 11, U_DOUBLE, 8},
               {ossim::nitf::PPO_X0_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::PPO_Y0_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K0_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K1_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K2_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::RLD_K3_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::DCD_P1_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::DCD_P2_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::DCD_P3_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::AD_A1_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::AD_A2_TELE_KW, 21, SCIENTIFIC},
               {ossim::nitf::RADIUS_OF_VALIDITY_TELE_KW, 21, SCIENTIFIC},
            {ossim::nitf::NUM_TELE_SETS_FA_DATA_KW, LOOP_END},
         {ossim::nitf::TELESCOPE_OPTICS_FLAG_KW + " == 2", IF_STATEMENT_END},
      {ossim::nitf::SENSOR_TYPE_KW + " == F", IF_STATEMENT_END},
      {ossim::nitf::RESERVED_LEN_KW, 9, U_INT}
   };
}

ossimString ossimNitfCssfabDes::getClassName() const
{
   return ossimString("ossimNitfCssfabdes");
}

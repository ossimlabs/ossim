//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSEXRB tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.1 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfCsexrbTag.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>
#include <utility> /* make_pair */

static ossimTrace traceDebug("ossimNitfCsexrbTag:debug");

static const int FOREVER = 1;

const std::string ossimNitfCsexrbTag::CETAG_KW = "CSEXRB";

ossimNitfCsexrbTag::ossimNitfCsexrbTag()
   : ossimNitfGenericTag("CSEXRB")
{
   // traceDebug.setTraceFlag(true);
   
   initializeFieldDefinitions();
   // initializeFields();
   initializeFields();
   setTagLength(computeTagLength());
}

ossimNitfCsexrbTag::ossimNitfCsexrbTag(ossim_uint32 tagLength)
   : ossimNitfGenericTag("CSEXRB", tagLength)
{
   initializeFieldDefinitions();

}
void ossimNitfCsexrbTag::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
   {
      {ossim::nitf::IMAGE_UUID_KW, 36},
      {ossim::nitf::NUM_ASSOC_DES_KW, 3, U_INT},
      {ossim::nitf::NUM_ASSOC_DES_KW, LOOP_START},
         {ossim::nitf::ASSOC_DES_UUID_KW, 36},
      {ossim::nitf::NUM_ASSOC_DES_KW, LOOP_END},
      {ossim::nitf::PLATFORM_ID_KW, 6},
      {ossim::nitf::PAYLOAD_ID_KW, 6},
      {ossim::nitf::SENSOR_ID_KW, 6},
      {ossim::nitf::SENSOR_TYPE_KW, 1, 0, 0, "S"},
      {ossim::nitf::GROUND_REF_POINT_X_KW, 12, U_DOUBLE, 3, " "},
      {ossim::nitf::GROUND_REF_POINT_Y_KW, 12, U_DOUBLE, 3, " "},
      {ossim::nitf::GROUND_REF_POINT_Z_KW, 12, U_DOUBLE, 3, " "},
      {ossim::nitf::SENSOR_TYPE_KW + " = 'S'", IF_STATEMENT_START},
         {ossim::nitf::DAY_FIRST_LINE_IMAGE_KW, 8, U_INT},
         {ossim::nitf::TIME_FIRST_LINE_IMAGE_KW, 15, U_DOUBLE, 9},
         {ossim::nitf::TIME_IMAGE_DURATION_KW, 16, DOUBLE, 9},
      {ossim::nitf::SENSOR_TYPE_KW + " = 'S'", IF_STATEMENT_END},
      {ossim::nitf::SENSOR_TYPE_KW + " = 'F'", IF_STATEMENT_START},
         {ossim::nitf::TIME_STAMP_LOC_KW, 1},
         {ossim::nitf::TIME_STAMP_LOC_KW + " = 0", IF_STATEMENT_START, U_INT},
            {ossim::nitf::REFRENCE_FRAME_NUM_KW, 9, U_INT, 0, " "},
            {ossim::nitf::BASE_TIMESTAMP_KW, 24},
            {ossim::nitf::DT_MULTIPLIER_KW, 8, U_INT},
            {ossim::nitf::DT_SIZE_KW, 1, U_INT},
            {ossim::nitf::NUMBER_FRAMES_KW, 4, U_INT},
            {ossim::nitf::NUMBER_DT_KW, 4, U_INT},
            {ossim::nitf::NUMBER_DT_KW, LOOP_START},
               {ossim::nitf::DT_KW + " " + ossim::nitf::DT_SIZE_KW, VARIABLE_LENGTH, U_INT},
            {ossim::nitf::NUMBER_DT_KW, LOOP_END},
         {ossim::nitf::TIME_STAMP_LOC_KW + " = 0", IF_STATEMENT_END},
      {ossim::nitf::SENSOR_TYPE_KW + " = 'F'", IF_STATEMENT_END},
      {ossim::nitf::MAX_GSD_KW, 12, U_DOUBLE, 1, " "},
      {ossim::nitf::ALONG_SCAN_GSD_KW, 12, U_DOUBLE, 1, " "},
      {ossim::nitf::CROSS_SCAN_GSD_KW, 12, U_DOUBLE, 1, " "},
      {ossim::nitf::GEO_MEAN_GSD_KW, 12, U_DOUBLE, 1, " "},
      {ossim::nitf::A_S_VERT_GSD_KW, 12, U_DOUBLE, 1, " "},
      {ossim::nitf::C_S_VERT_GSD_KW, 12, U_DOUBLE, 1, " "},
      {ossim::nitf::GEO_MEAN_VERT_GSD_KW, 12, U_DOUBLE, 1, " "},
      {ossim::nitf::GSD_BETA_ANGLE_KW, 5, U_DOUBLE, 1, " "},
      {ossim::nitf::DYNAMIC_RANGE_KW, 5, U_INT, 0, " "},
      {ossim::nitf::NUM_LINES_KW, 7, U_INT},
      {ossim::nitf::NUM_SAMPLES_KW, 5, U_INT},
      {ossim::nitf::ANGLE_TO_NORTH_KW, 7, U_DOUBLE, 3, " "},
      {ossim::nitf::OBLIQUITY_ANGLE_KW, 6, U_DOUBLE, 3, " "},
      {ossim::nitf::AZ_OF_OBLIQUITY_KW, 7, U_DOUBLE, 3, " "},
      {ossim::nitf::ATM_REFR_FLAG_KW, 1},
      {ossim::nitf::VEL_ABER_FLAG_KW, 1},
      {ossim::nitf::GRD_COVER_KW, 1, 0, 0, "9"},
      {ossim::nitf::SNOW_DEPTH_CATEGORY_KW, 1, 0, 0, "9"},
      {ossim::nitf::SUN_AZIMUTH_KW, 7, U_DOUBLE, 3, " "},
      {ossim::nitf::SUN_ELEVATION_KW, 7, DOUBLE, 3, " "},
      {ossim::nitf::PREDICTED_NIIRS_KW, 3, U_DOUBLE, 1, " "},
      {ossim::nitf::CIRCL_ERR_KW, 5, U_DOUBLE, 1, " "},
      {ossim::nitf::LINEAR_ERR_KW, 5, U_DOUBLE, 1, " "},
      {ossim::nitf::CLOUD_COVER_KW, 3, U_INT, 0, " "},
      {ossim::nitf::SENSOR_TYPE_KW + " = 'F'", IF_STATEMENT_START},
         {ossim::nitf::ROLLING_SHUTTER_FLAG_KW, 1},
      {ossim::nitf::SENSOR_TYPE_KW + " = 'F'", IF_STATEMENT_END},
      {ossim::nitf::UE_TIME_FLAG_KW, 1},
      {ossim::nitf::RESERVED_LEN_KW, 5, U_INT},
      {"! ( " + ossim::nitf::RESERVED_LEN_KW + " = 0 )", IF_STATEMENT_START},
         {ossim::nitf::MASK_LEN_KW, 2, U_INT},
         {ossim::nitf::RESERVED_FIELD_MASK_KW + " " + ossim::nitf::MASK_LEN_KW, VARIABLE_LENGTH, U_INT},
         {ossim::nitf::RESERVED_FIELD_MASK_KW + ":0 = 1", IF_STATEMENT_START},
            {ossim::nitf::RESERVED_LEN_AREA1_KW, 5, U_INT},
            {ossim::nitf::NUM_IMG_OPS_KW, 2, U_INT},
            {ossim::nitf::TGT_ID_LEN_KW, 2, U_INT},
            {ossim::nitf::TGT_ID_KW + " " + ossim::nitf::TGT_ID_LEN_KW, VARIABLE_LENGTH},
            {ossim::nitf::TGT_NAME_LEN_KW, 2, U_INT},
            {ossim::nitf::TGT_NAME_KW + " " + ossim::nitf::TGT_NAME_LEN_KW, VARIABLE_LENGTH},
            {ossim::nitf::TGT_TYPE_LEN_KW, 2, U_INT},
            {ossim::nitf::TGT_TYPE_KW + " " + ossim::nitf::TGT_TYPE_LEN_KW, VARIABLE_LENGTH},
            {ossim::nitf::TGT_LAT_KW, 9, DOUBLE, 5, " "},
            {ossim::nitf::TGT_LON_KW, 10, DOUBLE, 5, " "},
            {ossim::nitf::TGT_HT_KW, 8, DOUBLE, 1, " "},
            {ossim::nitf::TGT_DATE_TIME_KW, 14},
            {ossim::nitf::TGT_AZ_KW, 7, U_DOUBLE, 3, " "},
            {ossim::nitf::TGT_ELEV_ANG_KW, 7, DOUBLE, 3, " "},
            {ossim::nitf::TGT_BIDEC_ANG_KW, 7, U_DOUBLE, 3, " "},
            {ossim::nitf::COLL_REQ_ID_LEN_KW, 3, U_INT},
            {ossim::nitf::COLL_REQ_ID_KW + " " + ossim::nitf::COLL_REQ_ID_LEN_KW, VARIABLE_LENGTH},
            {ossim::nitf::COLLECT_STRAT_LEN_KW, 2, U_INT},
            {ossim::nitf::COLLECT_STRAT_KW + " " + ossim::nitf::COLLECT_STRAT_LEN_KW, VARIABLE_LENGTH},
            {ossim::nitf::COLLECT_TYPE_LEN_KW, 2, U_INT},
            {ossim::nitf::COLLECT_TYPE_KW + " " + ossim::nitf::COLLECT_TYPE_LEN_KW, VARIABLE_LENGTH},
            {ossim::nitf::COLL_CODE_LEN_KW, 2, U_INT},
            {ossim::nitf::COLL_CODE_KW + " " + ossim::nitf::COLL_CODE_LEN_KW, VARIABLE_LENGTH},
            {ossim::nitf::NUM_COLLECT_CRITERIA_KW, 2, U_INT},
            {ossim::nitf::NUM_COLLECT_CRITERIA_KW, LOOP_START},
               {ossim::nitf::COLLECT_CRITERIA_NAME_LEN_KW, 2, U_INT},
               {ossim::nitf::COLLECT_CRITERIA_NAME_KW + " " + ossim::nitf::COLLECT_CRITERIA_NAME_LEN_KW, VARIABLE_LENGTH},
               {ossim::nitf::COLLECT_CRITERIA_UNIT_LEN_KW, 2, U_INT},
               {ossim::nitf::COLLECT_CRITERIA_UNIT_KW + " " + ossim::nitf::COLLECT_CRITERIA_UNIT_LEN_KW, VARIABLE_LENGTH},
               {ossim::nitf::COLLECT_CRITERIA_VALUE_LEN_KW, 2, U_INT},
               {ossim::nitf::COLLECT_CRITERIA_VALUE_KW + " " + ossim::nitf::COLLECT_CRITERIA_VALUE_LEN_KW, VARIABLE_LENGTH},
            {ossim::nitf::NUM_COLLECT_CRITERIA_KW, LOOP_END},
            {ossim::nitf::NUM_IMG_OPS_DATA_KW, 2, U_INT},
            {ossim::nitf::NUM_IMG_OPS_DATA_KW, LOOP_START},
               {ossim::nitf::CM_ID_LEN_KW, 2, U_INT},
               {ossim::nitf::CM_ID_KW + " " + ossim::nitf::CM_ID_LEN_KW, VARIABLE_LENGTH},
               {ossim::nitf::SENSOR_CONFIG_LEN_KW, 2, U_INT},
               {ossim::nitf::SENSOR_CONFIG_KW + " " + ossim::nitf::SENSOR_CONFIG_LEN_KW, VARIABLE_LENGTH},
               {ossim::nitf::IMG_OP_ID_LEN_KW, 2, U_INT},
               {ossim::nitf::IMG_OP_ID_KW + " " + ossim::nitf::IMG_OP_ID_LEN_KW, VARIABLE_LENGTH},
               {ossim::nitf::NUM_EXP_KW, 2, U_INT},
               {ossim::nitf::INDEX_SIZE_KW, 1},
               {ossim::nitf::NUM_INDICES_KW, 2, U_INT},
               {ossim::nitf::NUM_INDICES_KW, LOOP_START},
                  {ossim::nitf::INDEX_IN_IMG_OP_ID_KW + " " + ossim::nitf::INDEX_SIZE_KW, VARIABLE_LENGTH, U_INT},
               {ossim::nitf::NUM_INDICES_KW, LOOP_END},
               {ossim::nitf::NUM_QUALITY_METRICS_KW, 2, U_INT},
               {ossim::nitf::NUM_QUALITY_METRICS_KW, LOOP_START},
                  {ossim::nitf::QUALITY_METRIC_NAME_LEN_KW, 2, U_INT},
                  {ossim::nitf::QUALITY_METRIC_NAME_KW + " " + ossim::nitf::QUALITY_METRIC_NAME_LEN_KW, VARIABLE_LENGTH},
                  {ossim::nitf::QUALITY_METRIC_UNIT_LEN_KW, 2, U_INT},
                  {ossim::nitf::QUALITY_METRIC_UNIT_KW + " " + ossim::nitf::QUALITY_METRIC_NAME_LEN_KW, VARIABLE_LENGTH},
                  {ossim::nitf::QUALITY_METRIC_TYPE_KW, 1},
                  {ossim::nitf::QUALITY_METRIC_VALUE_LEN_KW, 2, U_INT},
                  {ossim::nitf::QUALITY_METRIC_VALUE_KW + " " + ossim::nitf::QUALITY_METRIC_NAME_LEN_KW, VARIABLE_LENGTH, U_INT},
               {ossim::nitf::NUM_QUALITY_METRICS_KW, LOOP_END},
            {ossim::nitf::NUM_IMG_OPS_DATA_KW, LOOP_END},
         {ossim::nitf::RESERVED_FIELD_MASK_KW + ":0 = 1", IF_STATEMENT_END},
      {"! ( " + ossim::nitf::RESERVED_LEN_KW + " = 0 )", IF_STATEMENT_END}
   };
}

ossimString ossimNitfCsexrbTag::getClassName() const
{
   return ossimString("ossimNitfCsexrbTag");
}

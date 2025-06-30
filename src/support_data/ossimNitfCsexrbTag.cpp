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

ossimNitfCsexrbTag::ossimNitfCsexrbTag(ossim_uint32 tagLength)
   : ossimNitfGenericTag("CSEXRB", tagLength)
{
}

const ossim_int32 ossimNitfGenericTag::NUM_DEFINITIONS = 123;
//-1: variable length, -2: if start, -3: if end, -4: loop start, -5 loop end
ossimNitfGenericTag::definition ossimNitfGenericTag::FIELD_DEFINITIONS[NUM_DEFINITIONS] =
{
   {"IMAGE_UUID", 36},
   {"NUM_ASSOC_DES", 3},
   {"NUM_ASSOC_DES i", -4},
      {"ASSOC_DES_UUID", 36},
   {"NUM_ASSOC_DES", -5},
   {"PLATFORM_ID", 6},
   {"PAYLOAD_ID", 6},
   {"SENSOR_ID", 6},
   {"SENSOR_TYPE", 1},
   {"GROUND_REF_POINT_X", 12},
   {"GROUND_REF_POINT_Y", 12},
   {"GROUND_REF_POINT_Z", 12},
   {"SENSOR_TYPE == S", -2},
      {"DAY_FIRST_LINE_IMAGE", 8},
      {"TIME_FIRST_LINE_IMAGE", 15},
      {"TIME_IMAGE_DURATION", 16},
   {"SENSOR_TYPE == S", -3},
   {"SENSOR_TYPE == F", -2},
      {"TIME_STAMP_LOC", 1},
      {"TIME_STAMP_LOC == 0", -2},
         {"REFRENCE_FRAME_NUM", 9},
         {"BASE_TIMESTAMP", 24},
         {"DT_MULTIPLIER", 8},
         {"DT_SIZE", 1},
         {"NUMBER_FRAMES", 4},
         {"NUMBER_DT", 4},
         {"NUMBER_DT n", -4},
            {"DT DT_SIZE", -1},
         {"NUMBER_DT", -5},
      {"TIME_STAMP_LOC == 0", -3},
   {"SENSOR_TYPE F", -3},
   {"MAX_GSD", 12},
   {"ALONG_SCAN_GSD", 12},
   {"CROSS_SCAN_GSD", 12},
   {"GEO_MEAN_GSD", 12},
   {"A_S_VERT_GSD", 12},
   {"C_S_VERT_GSD", 12},
   {"GEO_MEAN_VERT_GSD", 12},
   {"GSD_BETA_ANGLE", 5},
   {"DYNAMIC_RANGE", 5},
   {"NUM_LINES", 7},
   {"NUM_SAMPLES", 5},
   {"ANGLE_TO_NORTH", 7},
   {"OBLIQUITY_ANGLE", 6},
   {"AZ_OF_OBLIQUITY", 7},
   {"ATM_REFR_FLAG", 1},
   {"VEL_ABER_FLAG", 1},
   {"GRD_COVER", 1},
   {"SNOW_DEPTH_CATEGORY", 1},
   {"SUN_AZIMUTH", 7},
   {"SUN_ELEVATION", 7},
   {"PREDICTED_NIIRS", 3},
   {"CIRCL_ERR", 5},
   {"LINEAR_ERR", 5},
   {"CLOUD_COVER", 3},
   {"SENSOR_TYPE == F", -2},
      {"ROLLING_SHUTTER_FLAG", 1},
   {"SENSOR_TYPE F", -3},
   {"UE_TIME_FLAG", 1},
   {"RESERVED_LEN", 5, '0'},
   {"RESERVED_LEN != 00000", -2},
      {"MASK_LEN", 2, '0'},
      {"RESERVED_FIELD_MASK MASK_LEN", -1, '0'},
      {"RESERVED_FIELD_MASK:0 == 1", -2},
         {"RESERVED_LEN_AREA1", 5},
         {"NUM_IMG_OPS", 2},
         {"TGT_ID_LEN", 2},
         {"TGT_ID TGT_ID_LEN", -1},
         {"TGT_NAME_LEN", 2},
         {"TGT_NAME TGT_NAME_LEN", -1},
         {"TGT_TYPE_LEN", 2},
         {"TGT_TYPE TGT_TYPE_LEN", -1},
         {"TGT_LAT", 9},
         {"TGT_LON", 10},
         {"TGT_HT", 8},
         {"TGT_DATE_TIME", 14},
         {"TGT_AZ", 7},
         {"TGT_ELEV_ANG", 7},
         {"TGT_BIDEC_ANG", 7},
         {"COLL_REQ_ID_LEN", 3},
         {"COLL_REQ_ID COLL_REQ_ID_LEN", -1},
         {"COLLECT_STRAT_LEN", 2},
         {"COLLECT_STRAT COLLECT_STRAT_LEN", -1},
         {"COLLECT_TYPE_LEN", 2},
         {"COLLECT_TYPE COLLECT_TYPE_LEN", -1},
         {"COLL_CODE_LEN", 2},
         {"COLL_CODE COLL_CODE_LEN", -1},
         {"NUM_COLLECT_CRITERIA", 2},
         {"NUM_COLLECT_CRITERIA n", -4},
            {"COLLECT_CRITERIA_NAME_LEN", 2},
            {"COLLECT_CRITERIA_NAME COLLECT_CRITERIA_NAME_LEN", -1},
            {"COLLECT_CRITERIA_UNIT_LEN", 2},
            {"COLLECT_CRITERIA_UNIT COLLECT_CRITERIA_UNIT_LEN", -1},
            {"COLLECT_CRITERIA_VALUE_LEN", 2},
            {"COLLECT_CRITERIA_VALUE COLLECT_CRITERIA_VALUE_LEN", -1},
         {"NUM_COLLECT_CRITERIA", -5},
         {"NUM_IMG_OPS_DATA", 2},
         {"NUM_IMG_OPS_DATA n", -4},
            {"CM_ID_LEN", 2},
            {"CM_ID CM_ID_LEN", -1},
            {"SENSOR_CONFIG_LEN", 2},
            {"SENSOR_CONFIG SENSOR_CONFIG_LEN", -1},
            {"IMG_OP_ID_LEN", 2},
            {"IMG_OP_ID IMG_OP_ID_LEN", -1},
            {"NUM_EXP", 2},
            {"INDEX_SIZE", 1},
            {"NUM_INDICES", 2},
            {"NUM_INDICES m", -4},
               {"INDEX_IN_IMG_OP_ID INDEX_SIZE", -1},
            {"NUM_INDICES", -5},
            {"NUM_QUALITY_METRICS", 2},
            {"NUM_QUALITY_METRICS m", -4},
               {"QUALITY_METRIC_NAME_LEN", 2},
               {"QUALITY_METRIC_NAME QUALITY_METRIC_NAME_LEN", -1},
               {"QUALITY_METRIC_UNIT_LEN", 2},
               {"QUALITY_METRIC_UNIT QUALITY_METRIC_NAME_LEN", -1},
               {"QUALITY_METRIC_TYPE", 1},
               {"QUALITY_METRIC_VALUE_LEN", 2},
               {"QUALITY_METRIC_VALUE QUALITY_METRIC_NAME_LEN", -1},
            {"NUM_QUALITY_METRICS", -5},
         {"NUM_IMG_OPS_DATA", -5},
      {"RESERVED_FIELD_MASK:0 == 1", -3},
   {"RESERVED_LEN != 00000", -3}
};


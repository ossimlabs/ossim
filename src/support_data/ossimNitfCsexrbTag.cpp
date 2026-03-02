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
   {"IMAGE_UUID", 36},
   {"NUM_ASSOC_DES", 3, U_INT},
   {"NUM_ASSOC_DES i", LOOP_START},
      {"ASSOC_DES_UUID", 36},
   {"NUM_ASSOC_DES", LOOP_END},
   {"PLATFORM_ID", 6},
   {"PAYLOAD_ID", 6},
   {"SENSOR_ID", 6},
   {"SENSOR_TYPE", 1, 0, 0, "S"},
   {"GROUND_REF_POINT_X", 12, U_DOUBLE, 3, " "},
   {"GROUND_REF_POINT_Y", 12, U_DOUBLE, 3, " "},
   {"GROUND_REF_POINT_Z", 12, U_DOUBLE, 3, " "},
   {"SENSOR_TYPE 'S' =", IF_STATEMENT_START},
      {"DAY_FIRST_LINE_IMAGE", 8, U_INT},
      {"TIME_FIRST_LINE_IMAGE", 15, U_DOUBLE, 9},
      {"TIME_IMAGE_DURATION", 16, DOUBLE, 9},
   {"SENSOR_TYPE 'S' =", IF_STATEMENT_END},
   {"SENSOR_TYPE 'F' =", IF_STATEMENT_START},
      {"TIME_STAMP_LOC", 1},
      {"TIME_STAMP_LOC 0 =", IF_STATEMENT_START, U_INT},
         {"REFRENCE_FRAME_NUM", 9, U_INT, 0, " "},
         {"BASE_TIMESTAMP", 24},
         {"DT_MULTIPLIER", 8, U_INT},
         {"DT_SIZE", 1, U_INT},
         {"NUMBER_FRAMES", 4, U_INT},
         {"NUMBER_DT", 4, U_INT},
         {"NUMBER_DT n", LOOP_START},
            {"DT DT_SIZE", VARIABLE_LENGTH, U_INT},
         {"NUMBER_DT", LOOP_END},
      {"TIME_STAMP_LOC 0 =", IF_STATEMENT_END},
   {"SENSOR_TYPE 'F' =", IF_STATEMENT_END},
   {"MAX_GSD", 12, U_DOUBLE, 1, " "},
   {"ALONG_SCAN_GSD", 12, U_DOUBLE, 1, " "},
   {"CROSS_SCAN_GSD", 12, U_DOUBLE, 1, " "},
   {"GEO_MEAN_GSD", 12, U_DOUBLE, 1, " "},
   {"A_S_VERT_GSD", 12, U_DOUBLE, 1, " "},
   {"C_S_VERT_GSD", 12, U_DOUBLE, 1, " "},
   {"GEO_MEAN_VERT_GSD", 12, U_DOUBLE, 1, " "},
   {"GSD_BETA_ANGLE", 5, U_DOUBLE, 1, " "},
   {"DYNAMIC_RANGE", 5, U_INT, 0, " "},
   {"NUM_LINES", 7, U_INT},
   {"NUM_SAMPLES", 5, U_INT},
   {"ANGLE_TO_NORTH", 7, U_DOUBLE, 3, " "},
   {"OBLIQUITY_ANGLE", 6, U_DOUBLE, 3, " "},
   {"AZ_OF_OBLIQUITY", 7, U_DOUBLE, 3, " "},
   {"ATM_REFR_FLAG", 1, U_INT},
   {"VEL_ABER_FLAG", 1, U_INT},
   {"GRD_COVER", 1, 0, 0, "9"},
   {"SNOW_DEPTH_CATEGORY", 1, 0, 0, "9"},
   {"SUN_AZIMUTH", 7, U_DOUBLE, 3, " "},
   {"SUN_ELEVATION", 7, DOUBLE, 3, " "},
   {"PREDICTED_NIIRS", 3, U_DOUBLE, 1, " "},
   {"CIRCL_ERR", 5, U_DOUBLE, 1, " "},
   {"LINEAR_ERR", 5, U_DOUBLE, 1, " "},
   {"CLOUD_COVER", 3, U_INT, 0, " "},
   {"SENSOR_TYPE 'F' =", IF_STATEMENT_START},
      {"ROLLING_SHUTTER_FLAG", 1},
   {"SENSOR_TYPE 'F' =", IF_STATEMENT_END},
   {"UE_TIME_FLAG", 1},
   {"RESERVED_LEN", 5, U_INT},
   {"RESERVED_LEN 0 = !", IF_STATEMENT_START},
      {"MASK_LEN", 2, U_INT},
      {"RESERVED_FIELD_MASK MASK_LEN", VARIABLE_LENGTH, U_INT},
      {"RESERVED_FIELD_MASK:0 1 =", IF_STATEMENT_START},
         {"RESERVED_LEN_AREA1", 5, U_INT},
         {"NUM_IMG_OPS", 2, U_INT},
         {"TGT_ID_LEN", 2, U_INT},
         {"TGT_ID TGT_ID_LEN", VARIABLE_LENGTH},
         {"TGT_NAME_LEN", 2, U_INT},
         {"TGT_NAME TGT_NAME_LEN", VARIABLE_LENGTH},
         {"TGT_TYPE_LEN", 2, U_INT},
         {"TGT_TYPE TGT_TYPE_LEN", VARIABLE_LENGTH},
         {"TGT_LAT", 9, DOUBLE, 5, " "},
         {"TGT_LON", 10, DOUBLE, 5, " "},
         {"TGT_HT", 8, DOUBLE, 1, " "},
         {"TGT_DATE_TIME", 14},
         {"TGT_AZ", 7, U_DOUBLE, 3, " "},
         {"TGT_ELEV_ANG", 7, DOUBLE, 3, " "},
         {"TGT_BIDEC_ANG", 7, U_DOUBLE, 3, " "},
         {"COLL_REQ_ID_LEN", 3, U_INT},
         {"COLL_REQ_ID COLL_REQ_ID_LEN", VARIABLE_LENGTH},
         {"COLLECT_STRAT_LEN", 2, U_INT},
         {"COLLECT_STRAT COLLECT_STRAT_LEN", VARIABLE_LENGTH},
         {"COLLECT_TYPE_LEN", 2, U_INT},
         {"COLLECT_TYPE COLLECT_TYPE_LEN", VARIABLE_LENGTH},
         {"COLL_CODE_LEN", 2, U_INT},
         {"COLL_CODE COLL_CODE_LEN", VARIABLE_LENGTH},
         {"NUM_COLLECT_CRITERIA", 2, U_INT},
         {"NUM_COLLECT_CRITERIA n", LOOP_START},
            {"COLLECT_CRITERIA_NAME_LEN", 2, U_INT},
            {"COLLECT_CRITERIA_NAME COLLECT_CRITERIA_NAME_LEN", VARIABLE_LENGTH},
            {"COLLECT_CRITERIA_UNIT_LEN", 2, U_INT},
            {"COLLECT_CRITERIA_UNIT COLLECT_CRITERIA_UNIT_LEN", VARIABLE_LENGTH},
            {"COLLECT_CRITERIA_VALUE_LEN", 2, U_INT},
            {"COLLECT_CRITERIA_VALUE COLLECT_CRITERIA_VALUE_LEN", VARIABLE_LENGTH},
         {"NUM_COLLECT_CRITERIA", LOOP_END},
         {"NUM_IMG_OPS_DATA", 2, U_INT},
         {"NUM_IMG_OPS_DATA n", LOOP_START},
            {"CM_ID_LEN", 2, U_INT},
            {"CM_ID CM_ID_LEN", VARIABLE_LENGTH},
            {"SENSOR_CONFIG_LEN", 2, U_INT},
            {"SENSOR_CONFIG SENSOR_CONFIG_LEN", VARIABLE_LENGTH},
            {"IMG_OP_ID_LEN", 2, U_INT},
            {"IMG_OP_ID IMG_OP_ID_LEN", VARIABLE_LENGTH},
            {"NUM_EXP", 2, U_INT},
            {"INDEX_SIZE", 1},
            {"NUM_INDICES", 2, U_INT},
            {"NUM_INDICES m", LOOP_START},
               {"INDEX_IN_IMG_OP_ID INDEX_SIZE", VARIABLE_LENGTH, U_INT},
            {"NUM_INDICES", LOOP_END},
            {"NUM_QUALITY_METRICS", 2, U_INT},
            {"NUM_QUALITY_METRICS m", LOOP_START},
               {"QUALITY_METRIC_NAME_LEN", 2, U_INT},
               {"QUALITY_METRIC_NAME QUALITY_METRIC_NAME_LEN", VARIABLE_LENGTH},
               {"QUALITY_METRIC_UNIT_LEN", 2, U_INT},
               {"QUALITY_METRIC_UNIT QUALITY_METRIC_NAME_LEN", VARIABLE_LENGTH},
               {"QUALITY_METRIC_TYPE", 1},
               {"QUALITY_METRIC_VALUE_LEN", 2, U_INT},
               {"QUALITY_METRIC_VALUE QUALITY_METRIC_NAME_LEN", VARIABLE_LENGTH, U_INT},
            {"NUM_QUALITY_METRICS", LOOP_END},
         {"NUM_IMG_OPS_DATA", LOOP_END},
      {"RESERVED_FIELD_MASK:0 1 =", IF_STATEMENT_END},
   {"RESERVED_LEN 0 = !", IF_STATEMENT_END}
   };
}

ossimString ossimNitfCsexrbTag::getClassName() const
{
   return ossimString("ossimNitfCsexrbTag");
}

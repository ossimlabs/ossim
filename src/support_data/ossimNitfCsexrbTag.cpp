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
   initializeDefaults();
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
   {"GROUND_REF_POINT_X", 12, U_DOUBLE, 3},
   {"GROUND_REF_POINT_Y", 12, U_DOUBLE, 3},
   {"GROUND_REF_POINT_Z", 12, U_DOUBLE, 3},
   {"SENSOR_TYPE 'S' =", IF_STATEMENT_START},
      {"DAY_FIRST_LINE_IMAGE", 8, U_INT},
      {"TIME_FIRST_LINE_IMAGE", 15, U_DOUBLE, 9},
      {"TIME_IMAGE_DURATION", 16, DOUBLE, 9},
   {"SENSOR_TYPE 'S' =", IF_STATEMENT_END},
   {"SENSOR_TYPE 'F' =", IF_STATEMENT_START},
      {"TIME_STAMP_LOC", 1},
      {"TIME_STAMP_LOC 0 =", IF_STATEMENT_START, U_INT},
         {"REFRENCE_FRAME_NUM", 9, U_INT},
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
   {"MAX_GSD", 12, U_DOUBLE, 1},
   {"ALONG_SCAN_GSD", 12, U_DOUBLE, 1},
   {"CROSS_SCAN_GSD", 12, U_DOUBLE, 1},
   {"GEO_MEAN_GSD", 12, U_DOUBLE, 1},
   {"A_S_VERT_GSD", 12, U_DOUBLE, 1},
   {"C_S_VERT_GSD", 12, U_DOUBLE, 1},
   {"GEO_MEAN_VERT_GSD", 12, U_DOUBLE, 1},
   {"GSD_BETA_ANGLE", 5, U_DOUBLE, 1},
   {"DYNAMIC_RANGE", 5, U_INT},
   {"NUM_LINES", 7, U_INT},
   {"NUM_SAMPLES", 5, U_INT},
   {"ANGLE_TO_NORTH", 7, U_DOUBLE, 3},
   {"OBLIQUITY_ANGLE", 6, U_DOUBLE, 3},
   {"AZ_OF_OBLIQUITY", 7, U_DOUBLE, 3},
   {"ATM_REFR_FLAG", 1},
   {"VEL_ABER_FLAG", 1},
   {"GRD_COVER", 1, 0, 0, "9"},
   {"SNOW_DEPTH_CATEGORY", 1, 0, 0, "9"},
   {"SUN_AZIMUTH", 7, U_DOUBLE, 3},
   {"SUN_ELEVATION", 7, DOUBLE, 3},
   {"PREDICTED_NIIRS", 3, U_DOUBLE, 1},
   {"CIRCL_ERR", 5, U_DOUBLE, 1},
   {"LINEAR_ERR", 5, U_DOUBLE, 1},
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
         {"TGT_LAT", 9, DOUBLE, 5},
         {"TGT_LON", 10, DOUBLE, 5},
         {"TGT_HT", 8, DOUBLE, 1},
         {"TGT_DATE_TIME", 14},
         {"TGT_AZ", 7, U_DOUBLE, 3},
         {"TGT_ELEV_ANG", 7, DOUBLE, 3},
         {"TGT_BIDEC_ANG", 7, U_DOUBLE, 3},
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

//---
// See: Vol-2-APP M-GLAS-GFM
// Table M.6-1: Common Sensor Exploitation Reference Data (CSEXRB) TRE
//---
void ossimNitfCsexrbTag::initializeDefaults()
{
   clearFields();

   ossimString key;
   ossimString val;

   // 36 BCS-A R
   val = "00000000-0000-0000-0000-000000000000";
   m_fields_map.insert(std::make_pair(ossimString(ossim::nitf::IMAGE_UUID_KW), val));

   // 3 BCS-N 000 to 999 R
   key = ossim::nitf::NUM_ASSOC_DES_KW;
   val = "000";
   m_fields_map.insert(std::make_pair(key, val));

   //  ASSOC_DES_UUIDi 36 BCS-A C

   // 6 BCS-A R
   key = ossim::nitf::PLATFORM_ID_KW;
   val.string().resize(6);
   val.string().replace(0,6,6,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 6 BCS-A R
   key = ossim::nitf::PAYLOAD_ID_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 6 BCS-A R
   key = ossim::nitf::SENSOR_ID_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 1 BCS-A "F", "S" or BCS space if field is N/A <R>
   key = ossim::nitf::SENSOR_TYPE_KW;
   val = " ";
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A -99999999.99 to +99999999.99 or BCS spaces meters <R>
   key = ossim::nitf::GROUND_REF_POINT_X_KW;
   val.string().resize(12);
   val.string().replace(0,12,12,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A -99999999.99 to +99999999.99 or BCS spaces meters <R>
   key = ossim::nitf::GROUND_REF_POINT_Y_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A -99999999.99 to +99999999.99 or BCS spaces meters <R>
   key = ossim::nitf::GROUND_REF_POINT_Z_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // If (SENSOR_TYPE = S) conditional.

   //---
   // Field: DAY_FIRST_LINE_IMAGE 8 BCS-N CCYYMMDD UTC C
   // Day of First Line of the Synthetic Array Image.
   //---

   //---
   // Field: TIME_FIRST_LINE_IMAGE
   // 15 BCS-N  00000.000000000 to 86399.999999999 seconds UTC C
   //---

   //---
   // Field: TIME_IMAGE_DURATION
   // 16 BCS-N -86399.999999999 to 86399.999999999 seconds UTC C
   //---

   // END If (SENSOR_TYPE = S) conditional.

   // If (SENSOR_TYPE = F)

   // Field: TIME_STAMP_LOC 1 BCS-N 0 or 1 C

   //---
   // Field: REFERENCE_FRAME_NUM 9 BCS-A 000000001 to 999999999 or BCS spaces <C>
   //---

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = ossim::nitf::MAX_GSD_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = ossim::nitf::ALONG_SCAN_GSD_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = ossim::nitf::CROSS_SCAN_GSD_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = ossim::nitf::GEO_MEAN_GSD_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = ossim::nitf::A_S_VERT_GSD_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = ossim::nitf::C_S_VERT_GSD_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = ossim::nitf::GEO_MEAN_VERT_GSD_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-A 000.0 to 180.0 or BCS spaces degrees <R>
   key = ossim::nitf::GSD_BETA_ANGLE_KW;
   val.string().resize(5);
   val.string().replace(0,5,5,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-A 00000 to 99999 or BCS spaces dn <R>
   key = ossim::nitf::DYNAMIC_RANGE_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 7 BCS-N 0000000 to 9999999 R
   key = ossim::nitf::NUM_LINES_KW;
   val.string().resize(7);
   val.string().replace(0,7,7,'0');
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-N 00000 to 99999 R
   key = ossim::nitf::NUM_SAMPLES_KW;
   val.string().resize(5);
   val.string().replace(0,5,5,'0');
   m_fields_map.insert(std::make_pair(key, val));

   // 7 BCS-A 000.000 to 359.999 or BCS spaces degrees <R>
   key = ossim::nitf::ANGLE_TO_NORTH_KW;
   val.string().resize(7);
   val.string().replace(0,7,7,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 6 BCS-A 00.000 to 90.000 or BCS spaces degrees <R>
   key = ossim::nitf::OBLIQUITY_ANGLE_KW;
   val.string().resize(6);
   val.string().replace(0,6,6,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 7 BCS-A 000.000 to 359.999 or BCS spaces degrees <R>
   key = ossim::nitf::AZ_OF_OBLIQUITY_KW;
   val.string().resize(7);
   val.string().replace(0,7,7,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 1 BCS-N  0 = Do not apply correction 1 = Apply correction R
   key = ossim::nitf::ATM_REFR_FLAG_KW;
   val.string().resize(1);
   val = "0";
   m_fields_map.insert(std::make_pair(key, val));

   // 1 BCS-N  0 = Do not apply correction 1 = Apply correction R
   key = ossim::nitf::VEL_ABER_FLAG_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 1 BCS-N  1 = Snow 0 = No Snow 9 - Not Available R
   key = ossim::nitf::GRD_COVER_KW;
   val = "9";
   m_fields_map.insert(std::make_pair(key, val));

   //---
   // 1 BCS-N 0 = inches
   // 1 = 1 to 8 inches of ice and/or snow
   // 2 = 9 to 17 inches
   // 3 = greater than 17 inches
   // 9 = Not Available
   //---
   key = ossim::nitf::SNOW_DEPTH_CATEGORY_KW;
   val = "9";
   m_fields_map.insert(std::make_pair(key, val));

   // 7 BCS-A -90.000 to +90.000 or BCS spaces degrees <R>
   key = ossim::nitf::SUN_AZIMUTH_KW;
   val.string().resize(7);
   val.string().replace(0,7,7,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 7 BCS-A -90.000 to +90.000 or BCS spaces degrees <R>
   key = ossim::nitf::SUN_ELEVATION_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 3 BCS-A 0.0 to 9.0 or BCS spaces NIIRS <R>
   key = ossim::nitf::PREDICTED_NIIRS_KW;
   val.string().resize(3);
   val.string().replace(0,3,3,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-A 000.0 to 999.9 or BCS spaces feet <R>
   key = ossim::nitf::CIRCL_ERR_KW;
   val.string().resize(5);
   val.string().replace(0,5,5,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-A 000.0 to 999.9 or BCS spaces feet <R>
   key = ossim::nitf::LINEAR_ERR_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 3 BCS-A 000 to 100, 999 or BCS spaces percent <R>
   key = ossim::nitf::CLOUD_COVER_KW;
   val.string().resize(3);
   val.string().replace(0,3,3,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 1 BCS-N 0 = no 1 = yes or BCS space <R>
   key = ossim::nitf::UE_TIME_FLAG_KW;
   val.string().resize(1);
   val = " ";
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-A 00000 to 00063 Max or BCS spaces bytes R
   key = ossim::nitf::RESERVED_LEN_KW;
   val.string().resize(5);
   val.string().replace(0,5,5,'0');
   m_fields_map.insert(std::make_pair(key, val));
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimNitfCsexrbTag::initializeDefaults() DEBUG\n"
         << "Default map:\n";
      printMap(ossimNotify(ossimNotifyLevel_DEBUG)); 
   }
   
} // End: void ossimNitfCsexrbTag::initializeDefaults()

bool ossimNitfCsexrbTag::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   static const char MODULE[] = "ossimNitfCsexrbTag::loadState(...)";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " entered...\n"
         << "kwl:\n" << kwl << "\n"
         << "prefix: " << (prefix?prefix:"null") << "\n";
   }

   bool status = true;
   std::string pfx = prefix?prefix:"";
   std::string value;
   std::string os;
   ossim_uint32 fieldSize = 0;

   while(FOREVER) // Break on error or at end.
   {
      fieldSize = 36;
      value = kwl.findKey( pfx, ossim::nitf::IMAGE_UUID_KW );
      // Size must be exact. Currently no format check, only size.
      if ( value.size() == fieldSize )
      {
         m_fields_map.insert_or_assign(ossimString(ossim::nitf::IMAGE_UUID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING: Incorrect length of " << value.size()
            << " for " << ossim::nitf::IMAGE_UUID_KW << " field!" << std::endl;
         status = false;
         break;
      }

      // NUM_ASSOC_DES

      // ASSOC_DES_UUID

      // PLATFORM_ID
      fieldSize = 6;
      value = kwl.findKey( pfx, ossim::nitf::PLATFORM_ID_KW );
      if ( value.size() <= fieldSize )
      {
         if ( value.size() < fieldSize ) // Currently no format check, only size.
         {
            os = value;
            value.resize(fieldSize);
            ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
         }
         m_fields_map.insert_or_assign(ossimString(ossim::nitf::PLATFORM_ID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << ossim::nitf::PLATFORM_ID_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }

      // PAYLOAD_ID
      value = kwl.findKey( pfx, ossim::nitf::PAYLOAD_ID_KW );
      if ( value.size() <= fieldSize )
      {
         if ( value.size() < fieldSize ) // Currently no format check, only size.
         {
            os = value;
            value.resize(fieldSize);
            ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
         }
         m_fields_map.insert_or_assign(ossimString(ossim::nitf::PAYLOAD_ID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << ossim::nitf::PAYLOAD_ID_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }

      // SENSOR_ID
      value = kwl.findKey( pfx, ossim::nitf::SENSOR_ID_KW );
      if ( value.size() <= fieldSize )
      {
         if ( value.size() < fieldSize ) // Currently no format check, only size.
         {
            os = value;
            value.resize(fieldSize);
            ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
         }
         m_fields_map.insert_or_assign(ossimString(ossim::nitf::SENSOR_ID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << ossim::nitf::SENSOR_ID_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }

      // ossim::nitf::SENSOR_TYPE
      fieldSize = 1;
      value = kwl.findKey( pfx, ossim::nitf::SENSOR_TYPE_KW );
      if ( value.size() == fieldSize )
      {
         m_fields_map.insert_or_assign(ossimString(ossim::nitf::SENSOR_TYPE_KW), ossimString(value));

         if ( value == "S" ) // If scan we need these three fields.
         {
            // DAY_FIRST_LINE_IMAGE:
            fieldSize = 8;
            value = kwl.findKey( pfx, ossim::nitf::DAY_FIRST_LINE_IMAGE_KW );
            if ( value.size() < fieldSize ) // Currently no format check, only size.
            {
               os = value;
               value.resize(fieldSize);
               ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
            }
            m_fields_map.insert_or_assign(
               ossimString(ossim::nitf::DAY_FIRST_LINE_IMAGE_KW), ossimString(value));

            // TIME_FIRST_LINE_IMAGE:
            fieldSize = 15;
            value = kwl.findKey( pfx, ossim::nitf::TIME_FIRST_LINE_IMAGE_KW );
            if ( value.size() < fieldSize ) // Currently no format check, only size.
            {
               os = value;
               value.resize(fieldSize);
               ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
            }
            m_fields_map.insert_or_assign(
               ossimString(ossim::nitf::TIME_FIRST_LINE_IMAGE_KW ), ossimString(value));

            // TIME_IMAGE_DURATION:
            fieldSize = 16;
            value = kwl.findKey( pfx, ossim::nitf::TIME_IMAGE_DURATION_KW );
            if ( value.size() < fieldSize ) // Currently no format check, only size.
            {
               os = value;
               value.resize(fieldSize);
               ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
            }
            m_fields_map.insert_or_assign(
               ossimString(ossim::nitf::TIME_IMAGE_DURATION_KW), ossimString(value));
         }
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << ossim::nitf::SENSOR_TYPE_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }


      break; // Trailing break from forever loop.

   } // Matches: while(FOREVER)

   // Recompute and set tag length.
   setTagLength(computeTagLength());

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimNitfCsexrbTag::loadState(...) DEBUG:\nresult:\n";
      print(ossimNotify(ossimNotifyLevel_DEBUG), pfx);
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "\n" << MODULE << " exit status" << (status?"true\n":"false\n");
   }

   return status;
}

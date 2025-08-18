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
#include <utility> /* make_pair */

static ossimTrace traceDebug("ossimNitfCsexrbTag:debug");

static const int FOREVER = 1;

const std::string ossimNitfCsexrbTag::CETAG_KW = "CSEXRB";
const std::string ossimNitfCsexrbTag::IMAGE_UUID_KW = "IMAGE_UUID";
const std::string ossimNitfCsexrbTag::NUM_ASSOC_DES_KW = "NUM_ASSOC_DES";
const std::string ossimNitfCsexrbTag::ASSOC_DES_UUID_KW = "ASSOC_DES_UUID";
const std::string ossimNitfCsexrbTag::PLATFORM_ID_KW = "PLATFORM_ID";
const std::string ossimNitfCsexrbTag::PAYLOAD_ID_KW = "PAYLOAD_ID";
const std::string ossimNitfCsexrbTag::SENSOR_ID_KW = "SENSOR_ID";
const std::string ossimNitfCsexrbTag::SENSOR_TYPE_KW = "SENSOR_TYPE";
const std::string ossimNitfCsexrbTag::DAY_FIRST_LINE_IMAGE_KW = "DAY_FIRST_LINE_IMAGE";
const std::string ossimNitfCsexrbTag::TIME_FIRST_LINE_IMAGE_KW = "TIME_FIRST_LINE_IMAGE";
const std::string ossimNitfCsexrbTag::TIME_IMAGE_DURATION_KW = "TIME_IMAGE_DURATION";   
const std::string ossimNitfCsexrbTag::GROUND_REF_POINT_X_KW = "GROUND_REF_POINT_X";
const std::string ossimNitfCsexrbTag::GROUND_REF_POINT_Y_KW = "GROUND_REF_POINT_Y";
const std::string ossimNitfCsexrbTag::GROUND_REF_POINT_Z_KW = "GROUND_REF_POINT_Z";
const std::string ossimNitfCsexrbTag::MAX_GSD_KW = "MAX_GSD";
const std::string ossimNitfCsexrbTag::ALONG_SCAN_GSD_KW = "ALONG_SCAN_GSD";
const std::string ossimNitfCsexrbTag::CROSS_SCAN_GSD_KW = "CROSS_SCAN_GSD";
const std::string ossimNitfCsexrbTag::GEO_MEAN_GSD_KW = "GEO_MEAN_GSD";
const std::string ossimNitfCsexrbTag::A_S_VERT_GSD_KW = "A_S_VERT_GSD";
const std::string ossimNitfCsexrbTag::C_S_VERT_GSD_KW = "C_S_VERT_GSD";
const std::string ossimNitfCsexrbTag::GEO_MEAN_VERT_GSD_KW = "GEO_MEAN_VERT_GSD";
const std::string ossimNitfCsexrbTag::GSD_BETA_ANGLE_KW = "GSD_BETA_ANGLE";
const std::string ossimNitfCsexrbTag::DYNAMIC_RANGE_KW = "DYNAMIC_RANGE";
const std::string ossimNitfCsexrbTag::NUM_LINES_KW = "NUM_LINES";
const std::string ossimNitfCsexrbTag::NUM_SAMPLES_KW = "NUM_SAMPLES";
const std::string ossimNitfCsexrbTag::ANGLE_TO_NORTH_KW = "ANGLE_TO_NORTH";
const std::string ossimNitfCsexrbTag::OBLIQUITY_ANGLE_KW = "OBLIQUITY_ANGLE";
const std::string ossimNitfCsexrbTag::AZ_OF_OBLIQUITY_KW = "AZ_OF_OBLIQUITY";
const std::string ossimNitfCsexrbTag::ATM_REFR_FLAG_KW = "ATM_REFR_FLAG";
const std::string ossimNitfCsexrbTag::VEL_ABER_FLAG_KW = "VEL_ABER_FLAG";
const std::string ossimNitfCsexrbTag::GRD_COVER_KW = "GRD_COVER";
const std::string ossimNitfCsexrbTag::SNOW_DEPTH_CATEGORY_KW = "SNOW_DEPTH_CATEGORY";
const std::string ossimNitfCsexrbTag::SUN_AZIMUTH_KW = "SUN_AZIMUTH";
const std::string ossimNitfCsexrbTag::SUN_ELEVATION_KW = "SUN_ELEVATION";
const std::string ossimNitfCsexrbTag::PREDICTED_NIIRS_KW = "PREDICTED_NIIRS";
const std::string ossimNitfCsexrbTag::CIRCL_ERR_KW = "CIRCL_ERR";
const std::string ossimNitfCsexrbTag::LINEAR_ERR_KW = "LINEAR_ERR";
const std::string ossimNitfCsexrbTag::CLOUD_COVER_KW = "CLOUD_COVER";


ossimNitfCsexrbTag::ossimNitfCsexrbTag()
   : ossimNitfGenericTag(CETAG_KW, 0)
{
   // Uncomment to hard code on trace for class.
   // traceDebug.setTraceFlag(true);
   
   initializeFieldDefinitions();
   initializeDefaults();
   setTagLength(computeTagLength());

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimNitfCsexrbTag::ossimNitfCsexrbTag() DEBUG:\n"
         << "Computed default tag length: " << getTagLength() << "\n";
   }
}

ossimNitfCsexrbTag::ossimNitfCsexrbTag(ossim_uint32 tagLength)
   : ossimNitfGenericTag(CETAG_KW, tagLength)
{
   initializeFieldDefinitions();
}

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
      value = kwl.findKey( pfx, IMAGE_UUID_KW );
      // Size must be exact. Currently no format check, only size.
      if ( value.size() == fieldSize ) 
      {
         m_fields_map.insert_or_assign(ossimString(IMAGE_UUID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING: Incorrect length of " << value.size()
            << " for " << IMAGE_UUID_KW << " field!" << std::endl;
         status = false;
         break;
      }

      // NUM_ASSOC_DES

      // ASSOC_DES_UUID

      // PLATFORM_ID
      fieldSize = 6;
      value = kwl.findKey( pfx, PLATFORM_ID_KW );
      if ( value.size() <= fieldSize )
      {
         if ( value.size() < fieldSize ) // Currently no format check, only size.
         {
            os = value;
            value.resize(fieldSize);
            ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
         }
         m_fields_map.insert_or_assign(ossimString(PLATFORM_ID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << PLATFORM_ID_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }      

      // PAYLOAD_ID
      value = kwl.findKey( pfx, PAYLOAD_ID_KW );
      if ( value.size() <= fieldSize )
      {
         if ( value.size() < fieldSize ) // Currently no format check, only size.
         {
            os = value;
            value.resize(fieldSize);
            ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
         }
         m_fields_map.insert_or_assign(ossimString(PAYLOAD_ID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << PAYLOAD_ID_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }     

      // SENSOR_ID
      value = kwl.findKey( pfx, SENSOR_ID_KW );
      if ( value.size() <= fieldSize )
      {
         if ( value.size() < fieldSize ) // Currently no format check, only size.
         {
            os = value;
            value.resize(fieldSize);
            ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
         }
         m_fields_map.insert_or_assign(ossimString(SENSOR_ID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << SENSOR_ID_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }

      // SENSOR_TYPE
      fieldSize = 1;
      value = kwl.findKey( pfx, SENSOR_TYPE_KW );
      if ( value.size() == fieldSize )
      {
         m_fields_map.insert_or_assign(ossimString(SENSOR_TYPE_KW), ossimString(value));

         if ( value == "S" ) // If scan we need these three fields.
         {
            // DAY_FIRST_LINE_IMAGE:
            fieldSize = 8;
            value = kwl.findKey( pfx, DAY_FIRST_LINE_IMAGE_KW );
            if ( value.size() < fieldSize ) // Currently no format check, only size.
            {
               os = value;
               value.resize(fieldSize);
               ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
            }
            m_fields_map.insert_or_assign(
               ossimString(DAY_FIRST_LINE_IMAGE_KW), ossimString(value));

            // TIME_FIRST_LINE_IMAGE:
            fieldSize = 15;
            value = kwl.findKey( pfx, TIME_FIRST_LINE_IMAGE_KW );
            if ( value.size() < fieldSize ) // Currently no format check, only size.
            {
               os = value;
               value.resize(fieldSize);
               ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
            }
            m_fields_map.insert_or_assign(
               ossimString(TIME_FIRST_LINE_IMAGE_KW ), ossimString(value));
            
            // TIME_IMAGE_DURATION:
            fieldSize = 16;
            value = kwl.findKey( pfx, TIME_IMAGE_DURATION_KW );
            if ( value.size() < fieldSize ) // Currently no format check, only size.
            {
               os = value;
               value.resize(fieldSize);
               ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
            }
            m_fields_map.insert_or_assign(
               ossimString(TIME_IMAGE_DURATION_KW), ossimString(value));
         }
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << SENSOR_TYPE_KW
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
         << MODULE << " exit status" << (status?"true\n":"false\n");
   }
   
   return status;
}

void ossimNitfCsexrbTag::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
   {
   {IMAGE_UUID_KW, 36},
   {NUM_ASSOC_DES_KW, 3, {1}},
   {(NUM_ASSOC_DES_KW + " i"), LOOP_START}, // hack...
      {ASSOC_DES_UUID_KW, 36},
   {NUM_ASSOC_DES_KW, LOOP_END},
   {PLATFORM_ID_KW, 6},
   {PAYLOAD_ID_KW, 6},
   {SENSOR_ID_KW, 6},
   {SENSOR_TYPE_KW, 1},
   {GROUND_REF_POINT_X_KW, 12, {3, 3}},
   {GROUND_REF_POINT_Y_KW, 12, {3, 3}},
   {GROUND_REF_POINT_Z_KW, 12, {3, 3}},
   {(SENSOR_TYPE_KW+" 'S' ="), IF_STATEMENT_START},
      {DAY_FIRST_LINE_IMAGE_KW, 8, {1}},
      {TIME_FIRST_LINE_IMAGE_KW, 15, {3, 9}},
      {TIME_IMAGE_DURATION_KW, 16, {4, 9}},
   {(SENSOR_TYPE_KW+" 'S' ="), IF_STATEMENT_END},
   {"SENSOR_TYPE 'F' =", IF_STATEMENT_START},
      {"TIME_STAMP_LOC", 1},
      {"TIME_STAMP_LOC 0 =", IF_STATEMENT_START, {1}},
         {"REFRENCE_FRAME_NUM", 9, {1}},
         {"BASE_TIMESTAMP", 24},
         {"DT_MULTIPLIER", 8, {1}},
         {"DT_SIZE", 1, {1}},
         {"NUMBER_FRAMES", 4, {1}},
         {"NUMBER_DT", 4, {1}},
         {"NUMBER_DT n", LOOP_START},
            {"DT DT_SIZE", VARIABLE_LENGTH, {1}},
         {"NUMBER_DT", LOOP_END},
      {"TIME_STAMP_LOC 0 =", IF_STATEMENT_END},
   {"SENSOR_TYPE 'F' =", IF_STATEMENT_END},
   {MAX_GSD_KW, 12, {3, 1}},
   {ALONG_SCAN_GSD_KW, 12, {3, 1}},
   {CROSS_SCAN_GSD_KW, 12, {3, 1}},
   {GEO_MEAN_GSD_KW, 12, {3, 1}},
   {A_S_VERT_GSD_KW, 12, {3, 1}},
   {C_S_VERT_GSD_KW, 12, {3, 1}},
   {GEO_MEAN_VERT_GSD_KW, 12, {3, 1}},
   {GSD_BETA_ANGLE_KW, 5, {3, 1}},
   {DYNAMIC_RANGE_KW, 5, {1}},
   {NUM_LINES_KW, 7, {1}},
   {NUM_SAMPLES_KW, 5, {1}},
   {ANGLE_TO_NORTH_KW, 7, {3, 3}},
   {OBLIQUITY_ANGLE_KW, 6, {3, 3}},
   {AZ_OF_OBLIQUITY_KW, 7, {3, 3}},
   {ATM_REFR_FLAG_KW, 1},
   {VEL_ABER_FLAG_KW, 1},
   {GRD_COVER_KW, 1},
   {SNOW_DEPTH_CATEGORY_KW, 1},
   {SUN_AZIMUTH_KW, 7, {3, 3}},
   {SUN_ELEVATION_KW, 7, {4, 3}},
   {PREDICTED_NIIRS_KW, 3, {3, 1}},
   {CIRCL_ERR_KW, 5, {3, 1}},
   {LINEAR_ERR_KW, 5, {3, 1}},
   {CLOUD_COVER_KW, 3, {1}},
   {"SENSOR_TYPE 'F' =", IF_STATEMENT_START},
      {"ROLLING_SHUTTER_FLAG", 1},
   {"SENSOR_TYPE 'F' =", IF_STATEMENT_END},
   {"UE_TIME_FLAG", 1},
   {"RESERVED_LEN", 5, {1}},
   {"RESERVED_LEN 0 = !", IF_STATEMENT_START},
      {"MASK_LEN", 2, {1}},
      {"RESERVED_FIELD_MASK MASK_LEN", VARIABLE_LENGTH, {1}},
      {"RESERVED_FIELD_MASK:0 1 =", IF_STATEMENT_START},
         {"RESERVED_LEN_AREA1", 5, {1}},
         {"NUM_IMG_OPS", 2, {1}},
         {"TGT_ID_LEN", 2, {1}},
         {"TGT_ID TGT_ID_LEN", VARIABLE_LENGTH},
         {"TGT_NAME_LEN", 2, {1}},
         {"TGT_NAME TGT_NAME_LEN", VARIABLE_LENGTH},
         {"TGT_TYPE_LEN", 2, {1}},
         {"TGT_TYPE TGT_TYPE_LEN", VARIABLE_LENGTH},
         {"TGT_LAT", 9, {4, 5}},
         {"TGT_LON", 10, {4, 5}},
         {"TGT_HT", 8, {4, 1}},
         {"TGT_DATE_TIME", 14},
         {"TGT_AZ", 7, {3, 3}},
         {"TGT_ELEV_ANG", 7, {4, 3}},
         {"TGT_BIDEC_ANG", 7, {3, 3}},
         {"COLL_REQ_ID_LEN", 3, {1}},
         {"COLL_REQ_ID COLL_REQ_ID_LEN", VARIABLE_LENGTH},
         {"COLLECT_STRAT_LEN", 2, {1}},
         {"COLLECT_STRAT COLLECT_STRAT_LEN", VARIABLE_LENGTH},
         {"COLLECT_TYPE_LEN", 2, {1}},
         {"COLLECT_TYPE COLLECT_TYPE_LEN", VARIABLE_LENGTH},
         {"COLL_CODE_LEN", 2, {1}},
         {"COLL_CODE COLL_CODE_LEN", VARIABLE_LENGTH},
         {"NUM_COLLECT_CRITERIA", 2, {1}},
         {"NUM_COLLECT_CRITERIA n", LOOP_START},
            {"COLLECT_CRITERIA_NAME_LEN", 2, {1}},
            {"COLLECT_CRITERIA_NAME COLLECT_CRITERIA_NAME_LEN", VARIABLE_LENGTH},
            {"COLLECT_CRITERIA_UNIT_LEN", 2, {1}},
            {"COLLECT_CRITERIA_UNIT COLLECT_CRITERIA_UNIT_LEN", VARIABLE_LENGTH},
            {"COLLECT_CRITERIA_VALUE_LEN", 2, {1}},
            {"COLLECT_CRITERIA_VALUE COLLECT_CRITERIA_VALUE_LEN", VARIABLE_LENGTH},
         {"NUM_COLLECT_CRITERIA", LOOP_END},
         {"NUM_IMG_OPS_DATA", 2, {1}},
         {"NUM_IMG_OPS_DATA n", LOOP_START},
            {"CM_ID_LEN", 2, {1}},
            {"CM_ID CM_ID_LEN", VARIABLE_LENGTH},
            {"SENSOR_CONFIG_LEN", 2, {1}},
            {"SENSOR_CONFIG SENSOR_CONFIG_LEN", VARIABLE_LENGTH},
            {"IMG_OP_ID_LEN", 2, {1}},
            {"IMG_OP_ID IMG_OP_ID_LEN", VARIABLE_LENGTH},
            {"NUM_EXP", 2, {1}},
            {"INDEX_SIZE", 1},
            {"NUM_INDICES", 2, {1}},
            {"NUM_INDICES m", LOOP_START},
               {"INDEX_IN_IMG_OP_ID INDEX_SIZE", VARIABLE_LENGTH, {1}},
            {"NUM_INDICES", LOOP_END},
            {"NUM_QUALITY_METRICS", 2, {1}},
            {"NUM_QUALITY_METRICS m", LOOP_START},
               {"QUALITY_METRIC_NAME_LEN", 2, {1}},
               {"QUALITY_METRIC_NAME QUALITY_METRIC_NAME_LEN", VARIABLE_LENGTH},
               {"QUALITY_METRIC_UNIT_LEN", 2, {1}},
               {"QUALITY_METRIC_UNIT QUALITY_METRIC_NAME_LEN", VARIABLE_LENGTH},
               {"QUALITY_METRIC_TYPE", 1},
               {"QUALITY_METRIC_VALUE_LEN", 2, {1}},
               {"QUALITY_METRIC_VALUE QUALITY_METRIC_NAME_LEN", VARIABLE_LENGTH, {1}},
            {"NUM_QUALITY_METRICS", LOOP_END},
         {"NUM_IMG_OPS_DATA", LOOP_END},
      {"RESERVED_FIELD_MASK:0 1 =", IF_STATEMENT_END},
   {"RESERVED_LEN 0 = !", IF_STATEMENT_END}
   };

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimNitfCsexrbTag::initializeFieldDefinitions() DEBUG\n"
         << "Field definitions:\n";
      printFieldDefs(ossimNotify(ossimNotifyLevel_DEBUG)); 
   }
   
} // End: ossimNitfCsexrbTag::initializeFieldDefinitions()

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
   m_fields_map.insert(std::make_pair(ossimString(IMAGE_UUID_KW), val));

   // 3 BCS-N 000 to 999 R
   key = NUM_ASSOC_DES_KW;
   val = "000";
   m_fields_map.insert(std::make_pair(key, val));

   //  ASSOC_DES_UUIDi 36 BCS-A C

   // 6 BCS-A R
   key = PLATFORM_ID_KW;
   val.string().resize(6);
   val.string().replace(0,6,6,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 6 BCS-A R
   key = PAYLOAD_ID_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 6 BCS-A R
   key = SENSOR_ID_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 1 BCS-A "F", "S" or BCS space if field is N/A <R>
   key = SENSOR_TYPE_KW;
   val = " ";
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A -99999999.99 to +99999999.99 or BCS spaces meters <R>
   key = GROUND_REF_POINT_X_KW;
   val.string().resize(12);
   val.string().replace(0,12,12,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A -99999999.99 to +99999999.99 or BCS spaces meters <R>
   key = GROUND_REF_POINT_Y_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A -99999999.99 to +99999999.99 or BCS spaces meters <R>
   key = GROUND_REF_POINT_Z_KW;
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
   key = MAX_GSD_KW;
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = "ALONG_SCAN_GSD";
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = "CROSS_SCAN_GSD";
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = "GEO_MEAN_GSD";
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = "A_S_VERT_GSD";
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = "C_S_VERT_GSD";
   m_fields_map.insert(std::make_pair(key, val));

   // 12 BCS-A 0000000000.0 to 9999999999.9 or BCS spaces inches <R>   
   key = "GEO_MEAN_VERT_GSD";
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-A 000.0 to 180.0 or BCS spaces degrees <R>
   key = "GSD_BETA_ANGLE";
   val.string().resize(5);
   val.string().replace(0,5,5,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-A 00000 to 99999 or BCS spaces dn <R>
   key = "DYNAMIC_RANGE";
   m_fields_map.insert(std::make_pair(key, val));

   // 7 BCS-N 0000000 to 9999999 R
   key = "NUM_LINES";
   val.string().resize(7);
   val.string().replace(0,7,7,'0');
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-N 00000 to 99999 R
   key = "NUM_SAMPLES";
   val.string().resize(5);
   val.string().replace(0,5,5,'0');
   m_fields_map.insert(std::make_pair(key, val));

   // 7 BCS-A 000.000 to 359.999 or BCS spaces degrees <R>
   key = "ANGLE_TO_NORTH";
   val.string().resize(7);
   val.string().replace(0,7,7,'0');
   m_fields_map.insert(std::make_pair(key, val));

   // 6 BCS-A 00.000 to 90.000 or BCS spaces degrees <R>
   key = "OBLIQUITY_ANGLE";
   val.string().resize(6);
   val.string().replace(0,6,6,'0');
   m_fields_map.insert(std::make_pair(key, val));

   // 7 BCS-A 000.000 to 359.999 or BCS spaces degrees <R>
   key = "AZ_OF_OBLIQUITY";
   val.string().resize(7);
   val.string().replace(0,7,7,'0');
   m_fields_map.insert(std::make_pair(key, val));

   // 1 BCS-N  0 = Do not apply correction 1 = Apply correction R
   key = "ATM_REFR_FLAG";
   val.string().resize(1);
   val = "0";
   m_fields_map.insert(std::make_pair(key, val));

   // 1 BCS-N  0 = Do not apply correction 1 = Apply correction R
   key = "VEL_ABER_FLAG";
   m_fields_map.insert(std::make_pair(key, val));

   // 1 BCS-N  1 = Snow 0 = No Snow 9 - Not Available R
   key = "GRD_COVER";
   val = "9";
   m_fields_map.insert(std::make_pair(key, val));

   //---
   // 1 BCS-N 0 = inches
   // 1 = 1 to 8 inches of ice and/or snow
   // 2 = 9 to 17 inches
   // 3 = greater than 17 inches
   // 9 = Not Available
   //---
   key = "SNOW_DEPTH_CATEGORY";
   val = "9";
   m_fields_map.insert(std::make_pair(key, val));

   // 7 BCS-A -90.000 to +90.000 or BCS spaces degrees <R>
   key = "SUN_AZIMUTH";
   val.string().resize(7);
   val.string().replace(0,7,7,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 7 BCS-A -90.000 to +90.000 or BCS spaces degrees <R>
   key = "SUN_ELEVATION";
   m_fields_map.insert(std::make_pair(key, val));

   // 3 BCS-A 0.0 to 9.0 or BCS spaces NIIRS <R>
   key = "PREDICTED_NIIRS";
   val.string().resize(3);
   val.string().replace(0,3,3,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-A 000.0 to 999.9 or BCS spaces feet <R>
   key = "CIRCL_ERR";
   val.string().resize(5);
   val.string().replace(0,5,5,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-A 000.0 to 999.9 or BCS spaces feet <R>
   key = "LINEAR_ERR";
   m_fields_map.insert(std::make_pair(key, val));

   // 3 BCS-A 000 to 100, 999 or BCS spaces percent <R>
   key = "CLOUD_COVER";
   val.string().resize(3);
   val.string().replace(0,3,3,' ');
   m_fields_map.insert(std::make_pair(key, val));

   // 1 BCS-N 0 = no 1 = yes or BCS space <R>
   key = "UE_TIME_FLAG";
   val.string().resize(1);
   val = " ";
   m_fields_map.insert(std::make_pair(key, val));

   // 5 BCS-A 00000 to 00063 Max or BCS spaces bytes R
   key = "RESERVED_LEN";
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




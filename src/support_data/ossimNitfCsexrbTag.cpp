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
// See document STDI-0002-NCDRD Table M.6.1 for more info.
//
//----------------------------------------------------------------------------
// $Id
#include <ossim/support_data/ossimNitfCsexrbTag.h>

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <utility>

static const ossim_int32 NUM_DEFINITIONS = 122;
   //-1: variable length, -2: if start, -3: if end, -4: loop start, -5 loop end
static std::pair<ossimString, ossim_int32> FIELD_DEFINITIONS[NUM_DEFINITIONS] = {
      //{"CETAG", 6},
      //{"CEL", 5},
      {"IMAGE_UUID", 36},
      {"NUM_ASSOC_DES", 3},
      {"NUM_ASSOC_DES", -4},
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
            {"NUMBER_DT", -4},
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
      {"RESERVED_LEN", 5},
      {"RESERVED_LEN != 00000", -2},
      {"MASK_LEN", 2},
      {"RESERVED_FIELD_MASK", -1},
      {"RESERVED_FIELD_MASK:0 == 1", -2},
         {"RESERVED_LEN_AREA1", 5},
         {"NUM_IMG_OPS", 2},
         {"TGT_ID_LEN", 2},
         {"TGT_ID", -1},
         {"TGT_NAME_LEN", 2},
         {"TGT_NAME", -1},
         {"TGT_TYPE_LEN", 2},
         {"TGT_TYPE", -1},
         {"TGT_LAT", 9},
         {"TGT_LON", 10},
         {"TGT_HT", 8},
         {"TGT_DATE_TIME", 14},
         {"TGT_AZ", 7},
         {"TGT_ELEV_ANG", 7},
         {"TGT_BIDEC_ANG", 7},
         {"COLL_REQ_ID_LEN", 3},
         {"COLL_REQ_ID", -1},
         {"COLLECT_STRAT_LEN", 2},
         {"COLLECT_STRAT", -1},
         {"COLLECT_TYPE_LEN", 2},
         {"COLLECT_TYPE", -1},
         {"COLL_CODE_LEN", 2},
         {"COLL_CODE", -1},
         {"NUM_COLLECT_CRITERIA", 2},
         {"NUM_COLLECT_CRITERIA", -4},
            {"COLLECT_CRITERIA_NAME_LEN", 2},
            {"COLLECT_CRITERIA_NAME", -1},
            {"COLLECT_CRITERIA_UNIT_LEN", 2},
            {"COLLECT_CRITERIA_UNIT", -1},
            {"COLLECT_CRITERIA_VALUE_LEN", 2},
            {"COLLECT_CRITERIA_VALUE", -1},
         {"NUM_COLLECT_CRITERIA", -5},
         {"NUM_IMG_OPS_DATA", 2},
         {"NUM_IMG_OPS_DATA", -4},
            {"CM_ID_LEN", 2},
            {"CM_ID", -1},
            {"SENSOR_CONFIG_LEN", 2},
            {"SENSOR_CONFIG", -1},
            {"IMG_OP_ID_LEN", 2},
            {"IMG_OP_ID", -1},
            {"NUM_EXP", 2},
            {"INDEX_SIZE", 1},
            {"NUM_INDICES", 2},
            {"NUM_INDICES", -4},//Confusing
               {"INDEX_IN_IMG_OP_ID", -1},
            {"NUM_INDICES", -5},
            {"NUM_QUALITY_METRICS", 2},
            {"NUM_QUALITY_METRICS", -4},
               {"QUALITY_METRIC_NAME_LEN", 2},
               {"QUALITY_METRIC_NAME", -1},
               {"QUALITY_METRIC_UNIT_LEN", 2},
               {"QUALITY_METRIC_UNIT", -1},
               {"QUALITY_METRIC_TYPE", 1},
               {"QUALITY_METRIC_VALUE_LEN", 2},
               {"QUALITY_METRIC_VALUE", -1},
            {"NUM_QUALITY_METRICS", -5},
         {"NUM_IMG_OPS_DATA", -5},
      {"RESERVED_FIELD_MASK[0] == 1", -3},
      //{"RESERVED_LEN_AREA", 5},
      //{"RESERVED_AREA", 0}
   };

ossimNitfCsexrbTag::ossimNitfCsexrbTag()
   : ossimNitfRegisteredTag()//std::string("CSEXRB"), 443
{
   clearFields();
   setTagName("CSEXRB");
   m_fields_map = {};
   m_fields_vector = {};
}

static ossimString formatSuffix(std::vector<std::vector<ossim_int32>> suffixIn)
{
   ossimString result = "";
   char separator = 'n';
    for(std::vector<ossim_int32> set: suffixIn)
    {
       result += separator + std::to_string(set[0]);
       separator ++;
    }
    return result;
}
void ossimNitfCsexrbTag::parseStream(std::istream& in)
{
   std::cout << "Parse\n";
   clearFields();

    m_fields_map.clear();
    m_fields_vector.clear();

   //Curent itteration, Total iteration, i value
   std::vector<std::vector<ossim_int32>> suffix;
   std::vector<ossimString> spaceSubStrings, colonSubStrings;
   ossim_int32 length, i = 0;
   ossimString name, prevName;
   char value[256];
   bool condition;

   std::streampos start = in.tellg();
   while(i < NUM_DEFINITIONS)
   {
      spaceSubStrings.clear();
      colonSubStrings.clear();
      std::cout << i << "\n";
      std::cout << FIELD_DEFINITIONS[i].second << "\n";
      switch(FIELD_DEFINITIONS[i].second)
      {
         //variable length
         case -1:
            FIELD_DEFINITIONS[i].first.split(spaceSubStrings, ' ');
            name = spaceSubStrings[0] + formatSuffix(suffix);
            if(spaceSubStrings.size() == 1)
            {
               //value = new char[m_fields_vector.back().second.toInt() + 1];
               in.read(value, m_fields_vector.back().second.toInt());
               value[m_fields_vector.back().second.toInt()] = '\0';
            }
            else
            {
               length = m_fields_map[spaceSubStrings[1] + formatSuffix(suffix)].toInt();
               //value = new char[length + 1];
               in.read(value, length);
               value[length] = '\0';
            }
            std::cout << name << ", " << value << "\n";
            m_fields_map.insert(std::pair<ossimString, ossimString>(name, value));
            m_fields_vector.push_back(std::pair<ossimString, ossimString>(name, value));
            i++;
            //delete[] value;
            //value = 0;
            break;
         //if start
         case -2:
            FIELD_DEFINITIONS[i].first.split(spaceSubStrings, ' ');
            spaceSubStrings[0].split(colonSubStrings, ':');
            name = m_fields_map[colonSubStrings[0] + formatSuffix(suffix)];
            if(colonSubStrings.size() > 1)
               name = name.at(colonSubStrings[1].toInt());
            std::cout << FIELD_DEFINITIONS[i].first << ": "
                 << name << ", "
                 << spaceSubStrings[1] << ", "
                 << spaceSubStrings[2] << "\n";

            if(spaceSubStrings[1] == "==")
               condition = (name == spaceSubStrings[2]);
            else if(spaceSubStrings[1] == "!=")
               condition = (name != spaceSubStrings[2]);
            else
               condition = false;
            std::cout << condition << "\n";
            if(!condition)
               while(FIELD_DEFINITIONS[i].second != -3)
                  i++;
            i++;
            break;
         //if end
         case -3:
            i++;
            break;
         //loop start
         case -4:
            FIELD_DEFINITIONS[i].first.split(spaceSubStrings, ' ');
            length = m_fields_map[spaceSubStrings[0] + formatSuffix(suffix)].toInt();
            if(length > 0){
               suffix.push_back({1, length, i + 1});
               std::cout << 0 << ", "
               << length << ", "
               << i+1 << "\n";
            }

            else
               while(FIELD_DEFINITIONS[i].second != -5)
                  i++;
            i++;
            break;
         //loop end
         case -5:
            std::cout << suffix.back()[0] << ", "
                      << suffix.back()[1] << ", "
                      << suffix.back()[2] << "\n";
            suffix.back()[0] ++;
            if(suffix.back()[0] <= suffix.back()[1])
            {
               i = suffix.back()[2];
            }
            else
            {
               suffix.pop_back();
               i++;
            }
            break;
         //length provided
         default:
            name = FIELD_DEFINITIONS[i].first + formatSuffix(suffix);
            //value = new char[FIELD_DEFINITIONS[i].second + 1];
            in.read(value, FIELD_DEFINITIONS[i].second);
            value[FIELD_DEFINITIONS[i].second] = '\0';
            std::cout << name << ", " << value << "\n";
            m_fields_map.insert(std::pair<ossimString, ossimString>(name, value));
            m_fields_vector.push_back(std::pair<ossimString, ossimString>(name, value));
            //delete[] value;
            //value = 0;
            i++;
            break;
      };
   }
   std::streampos stop = in.tellg();
   std::cout << "ossimNitfCsexrbTag parseStream bytes read: " << (stop-start) << std::endl;

}

void ossimNitfCsexrbTag::writeStream(std::ostream& out)
{
   std::cout << "Write\n";
   clearFields();

   for(std::pair field : m_fields_vector)
   {
      out.write(field.second, field.second.length());
   }
}

std::ostream& ossimNitfCsexrbTag::print(std::ostream& out, const std::string& prefix) const
{
   std::cout << "Print\n";
   std::string pfx = prefix;
   pfx += "CSEXRB";
   pfx += ".";

   out << std::setiosflags(std::ios::left)
          << pfx << std::setw(24) << "CETAG:"
          << getTagName() << "\n"
          << pfx << std::setw(24) << "CEL:"
          << getTagLength() << "\n";

   for(std::pair field : m_fields_vector)
   {
      out << std::setiosflags(std::ios::left)
          << pfx << std::setw(24) << field.first << ":"
          << field.second << "\n";
   }

   return out;
}

void ossimNitfCsexrbTag::clearFields()
{
   m_fields_map.clear();
   m_fields_vector.clear();
}

ossimString ossimNitfCsexrbTag::get(ossimString fieldName)
{
   return m_fields_map[fieldName];
}

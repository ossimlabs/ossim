//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: MATESA tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document Vol1-AppAK-MATESA table 6.5 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfMatesaTag.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>
#include <utility> /* make_pair */

static ossimTrace traceDebug("ossimNitfMatesaTag:debug");

static const int FOREVER = 1;

const std::string ossimNitfMatesaTag::CETAG_KW = "MATESA";

ossimNitfMatesaTag::ossimNitfMatesaTag()
   : ossimNitfGenericTag("MATESA")
{
   initializeFieldDefinitions();
   initializeFields();
   setTagLength(computeTagLength());
}

ossimNitfMatesaTag::ossimNitfMatesaTag(ossim_uint32 tagLength)
   : ossimNitfGenericTag("MATESA", tagLength)
{
   initializeFieldDefinitions();

}
void ossimNitfMatesaTag::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
   {
      {"CUR_SOURCE", 42, ASCII},
      {"CUR_MATE_TYPE", 16, ASCII},
      {"CUR_FILE_ID_LEN", 4, U_INT},
      {"CUR_FILE_ID CUR_FILE_ID_LEN", VARIABLE_LENGTH, ASCII},
      {"NUM_GROUPS", 4, U_INT, 0, "1"},
      {"NUM_GROUPS n", LOOP_START},
         {"RELATIONSHIP", 24, ASCII},
         {"NUM_MATES", 4, U_INT, 0, "1"},
         {"NUM_MATES n", LOOP_START},
            {"SOURCE", 42, ASCII},
            {"MATE_TYPE", 16, ASCII, 0, "SIBLING"},
            {"MATE_ID_LEN", 4, U_INT},
            {"MATE_ID MATE_ID_LEN", VARIABLE_LENGTH, ASCII},
         {"NUM_MATES n", LOOP_END},
      {"NUM_GROUPS n", LOOP_END}
   };
}

ossimString ossimNitfMatesaTag::getClassName() const
{
   return ossimString("ossimNitfMatesaTag");
}

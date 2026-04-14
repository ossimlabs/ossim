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
      {ossim::nitf::CUR_SOURCE_KW, 42, ASCII},
      {ossim::nitf::CUR_MATE_TYPE_KW, 16, ASCII},
      {ossim::nitf::CUR_FILE_ID_LEN_KW, 4, U_INT},
      {ossim::nitf::CUR_FILE_ID_KW + " " + ossim::nitf::CUR_FILE_ID_LEN_KW, VARIABLE_LENGTH, ASCII},
      {ossim::nitf::NUM_GROUPS_KW, 4, U_INT, 0, "1"},
      {ossim::nitf::NUM_GROUPS_KW, LOOP_START},
         {ossim::nitf::RELATIONSHIP_KW, 24, ASCII},
         {ossim::nitf::NUM_MATES_KW, 4, U_INT, 0, "1"},
         {ossim::nitf::NUM_MATES_KW, LOOP_START},
            {ossim::nitf::SOURCE_KW, 42, ASCII},
            {ossim::nitf::MATE_TYPE_KW, 16, ASCII, 0, "SIBLING"},
            {ossim::nitf::MATE_ID_LEN_KW, 4, U_INT, 0, "1"},
            {ossim::nitf::MATE_ID_KW + " " + ossim::nitf::MATE_ID_LEN_KW, VARIABLE_LENGTH, ASCII},
         {ossim::nitf::NUM_MATES_KW, LOOP_END},
      {ossim::nitf::NUM_GROUPS_KW, LOOP_END}
   };
}

ossimString ossimNitfMatesaTag::getClassName() const
{
   return ossimString("ossimNitfMatesaTag");
}

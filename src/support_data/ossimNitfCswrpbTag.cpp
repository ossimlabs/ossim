//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSWRPB tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document Vol1-AppAK-MATESA table 6.5 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfCswrpbTag.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>
#include <utility> /* make_pair */

static ossimTrace traceDebug("ossimNitfCswrpbTag:debug");

static const int FOREVER = 1;

const std::string ossimNitfCswrpbTag::CETAG_KW = "CSWRPB";

ossimNitfCswrpbTag::ossimNitfCswrpbTag()
   : ossimNitfGenericTag("CSWRPB")
{
   initializeFieldDefinitions();
   initializeFields();
   setTagLength(computeTagLength());
}

ossimNitfCswrpbTag::ossimNitfCswrpbTag(ossim_uint32 tagLength)
   : ossimNitfGenericTag("CSWRPB", tagLength)
{
   initializeFieldDefinitions();

}
void ossimNitfCswrpbTag::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
   {
      {"NUM_SETS_WARP_DATA", 1, U_INT, 0, "1"},
      {"SENSOR_TYPE", 1, ASCII, 0, "S"},
      {"SENSOR_TYPE 'F' =", IF_STATEMENT_START},
         {"WRP_INTERP", 1, U_INT},
      {"SENSOR_TYPE 'F' =", IF_STATEMENT_END},
      {"NUM_SETS_WARP_DATA n", LOOP_START},
         {"^SENSOR_TYPE 'F' =", IF_STATEMENT_START},
            {"FL_WARP", 11, U_DOUBLE, 8},
         {"^SENSOR_TYPE 'F' =", IF_STATEMENT_END},
         {"OFFSET_LINE", 7, U_INT, 0, "1"},
         {"OFFSET_SAMP", 7, U_INT, 0, "1"},
         {"SCALE_LINE", 7, U_INT, 0, "1"},
         {"SCALE_SAMP", 7, U_INT, 0, "1"},
         {"OFFSET_LINE_UNWRP", 7, U_INT, 0, "1"},
         {"OFFSET_SAMP_UNWRP", 7, U_INT, 0, "1"},
         {"SCALE_LINE_UNWRP", 7, U_INT, 0, "1"},
         {"SCALE_SAMP_UNWRP", 7, U_INT, 0, "1"},
         {"LINE_POLY_ORDER_M1", 1, U_INT},
         {"LINE_POLY_ORDER_M2", 1, U_INT},
         {"SAMP_POLY_ORDER_N1", 1, U_INT},
         {"SAMP_POLY_ORDER_N2", 1, U_INT},
         {"LINE_POLY_ORDER_M2 j", LOOP_START},
            {"LINE_POLY_ORDER_M1 i", LOOP_START},
               {"A", 21, ASCII},
            {"LINE_POLY_ORDER_M1 i", LOOP_END},
         {"LINE_POLY_ORDER_M2 j", LOOP_END},
         {"SAMP_POLY_ORDER_N2 j", LOOP_START},
            {"SAMP_POLY_ORDER_N1 i", LOOP_START},
               {"B", 21, ASCII},
            {"SAMP_POLY_ORDER_N1 i", LOOP_END},
         {"SAMP_POLY_ORDER_N2 j", LOOP_END},
      {"NUM_SETS_WARP_DATA n", LOOP_END},
      {"RESERVED_LEN", 5, U_INT},
      {"RESERVED RESERVED_LEN", VARIABLE_LENGTH, ASCII}
   };
}

ossimString ossimNitfCswrpbTag::getClassName() const
{
   return ossimString("ossimNitfCswrpbTag");
}

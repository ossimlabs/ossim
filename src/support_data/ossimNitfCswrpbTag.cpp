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
      {ossim::nitf::NUM_SETS_WARP_DATA_KW, 1, U_INT, 0, "1"},
      {ossim::nitf::SENSOR_TYPE_KW, 1, ASCII, 0, "S"},
      {ossim::nitf::SENSOR_TYPE_KW + " = 'F'", IF_STATEMENT_START},
         {ossim::nitf::WRP_INTERP_KW, 1, U_INT},
      {ossim::nitf::SENSOR_TYPE_KW + " = 'F'", IF_STATEMENT_END},
      {ossim::nitf::NUM_SETS_WARP_DATA_KW, LOOP_START},
         {"^" + ossim::nitf::SENSOR_TYPE_KW + " = 'F'", IF_STATEMENT_START},
            {ossim::nitf::FL_WARP_KW, 11, U_DOUBLE, 8},
         {"^" + ossim::nitf::SENSOR_TYPE_KW + " = 'F'", IF_STATEMENT_END},
         {ossim::nitf::OFFSET_LINE_KW, 7, U_INT, 0, "1"},
         {ossim::nitf::OFFSET_SAMP_KW, 7, U_INT, 0, "1"},
         {ossim::nitf::SCALE_LINE_KW, 7, U_INT, 0, "1"},
         {ossim::nitf::SCALE_SAMP_KW, 7, U_INT, 0, "1"},
         {ossim::nitf::OFFSET_LINE_UNWRP_KW, 7, U_INT, 0, "1"},
         {ossim::nitf::OFFSET_SAMP_UNWRP_KW, 7, U_INT, 0, "1"},
         {ossim::nitf::SCALE_LINE_UNWRP_KW, 7, U_INT, 0, "1"},
         {ossim::nitf::SCALE_SAMP_UNWRP_KW, 7, U_INT, 0, "1"},
         {ossim::nitf::LINE_POLY_ORDER_M1_KW, 1, U_INT},
         {ossim::nitf::LINE_POLY_ORDER_M2_KW, 1, U_INT},
         {ossim::nitf::SAMP_POLY_ORDER_N1_KW, 1, U_INT},
         {ossim::nitf::SAMP_POLY_ORDER_N2_KW, 1, U_INT},
         {ossim::nitf::LINE_POLY_ORDER_M2_KW + " + 1", LOOP_START},
            {"^" + ossim::nitf::LINE_POLY_ORDER_M1_KW + " + 1", LOOP_START},
               {ossim::nitf::A_KW, 21, SCIENTIFIC},
            {"^" + ossim::nitf::LINE_POLY_ORDER_M1_KW + " + 1", LOOP_END},
         {ossim::nitf::LINE_POLY_ORDER_M2_KW + " + 1", LOOP_END},
         {ossim::nitf::SAMP_POLY_ORDER_N2_KW + " + 1", LOOP_START},
            {"^" + ossim::nitf::SAMP_POLY_ORDER_N1_KW + " + 1", LOOP_START},
               {ossim::nitf::B_KW, 21, SCIENTIFIC},
            {"^" + ossim::nitf::SAMP_POLY_ORDER_N1_KW + " + 1", LOOP_END},
         {ossim::nitf::SAMP_POLY_ORDER_N2_KW + " + 1", LOOP_END},
      {ossim::nitf::NUM_SETS_WARP_DATA_KW, LOOP_END},
      {ossim::nitf::RESERVED_LEN_KW, 5, U_INT},
      {ossim::nitf::RESERVED_KW + " " + ossim::nitf::RESERVED_LEN_KW, VARIABLE_LENGTH, ASCII}
   };
}

ossimString ossimNitfCswrpbTag::getClassName() const
{
   return ossimString("ossimNitfCswrpbTag");
}

//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSRLSB tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document Vol1-AppAK-MATESA table 6.5 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfCsrlsbTag.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>
#include <utility> /* make_pair */

static ossimTrace traceDebug("ossimNitfCsrlsbTag:debug");

static const int FOREVER = 1;

const std::string ossimNitfCsrlsbTag::CETAG_KW = "CSRLSB";

ossimNitfCsrlsbTag::ossimNitfCsrlsbTag()
   : ossimNitfGenericTag("CSRLSB")
{
   initializeFieldDefinitions();
   initializeFields();
   setTagLength(computeTagLength());
}

ossimNitfCsrlsbTag::ossimNitfCsrlsbTag(ossim_uint32 tagLength)
   : ossimNitfGenericTag("CSRLSB", tagLength)
{
   initializeFieldDefinitions();

}
void ossimNitfCsrlsbTag::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
   {
      {ossim::nitf::N_RS_ROW_BLOCKS_KW, 2, U_INT, 0, "1"},
      {ossim::nitf::M_RS_COLUMN_BLOCKS_KW, 2, U_INT, 0, "1"},
      {ossim::nitf::N_RS_ROW_BLOCKS_KW, LOOP_START},
         {"^" + ossim::nitf::M_RS_COLUMN_BLOCKS_KW, LOOP_START},
            {ossim::nitf::RS_DT_1_KW, 12, SCIENTIFIC},
            {ossim::nitf::RS_DT_2_KW, 12, SCIENTIFIC},
            {ossim::nitf::RS_DT_3_KW, 12, SCIENTIFIC},
            {ossim::nitf::RS_DT_4_KW, 12, SCIENTIFIC},
         {"^" + ossim::nitf::M_RS_COLUMN_BLOCKS_KW, LOOP_END},
      {ossim::nitf::N_RS_ROW_BLOCKS_KW, LOOP_END}
   };
}

ossimString ossimNitfCsrlsbTag::getClassName() const
{
   return ossimString("ossimNitfCsrlsbTag");
}

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
      {"N_RS_ROW_BLOCKS", 2, U_INT, 0, "1"},
      {"M_RS_COLUMN_BLOCKS", 2, U_INT, 0, "1"},
      {"N_RS_ROW_BLOCKS n", LOOP_START},
         {"^M_RS_COLUMN_BLOCKS m", LOOP_START},
            {"RS_DT_1", 12, SCIENTIFIC},
            {"RS_DT_2", 12, SCIENTIFIC},
            {"RS_DT_3", 12, SCIENTIFIC},
            {"RS_DT_4", 12, SCIENTIFIC},
         {"^M_RS_COLUMN_BLOCKS m", LOOP_END},
      {"N_RS_ROW_BLOCKS n", LOOP_END}
   };
}

ossimString ossimNitfCsrlsbTag::getClassName() const
{
   return ossimString("ossimNitfCsrlsbTag");
}

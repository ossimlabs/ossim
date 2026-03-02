//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: PIXQLA tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document Vol1-AppAK-MATESA table 6.5 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfPixqlaTag.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>
#include <utility> /* make_pair */

static ossimTrace traceDebug("ossimNitfPixqlaTag:debug");

static const int FOREVER = 1;

const std::string ossimNitfPixqlaTag::CETAG_KW = "PIXQLA";

ossimNitfPixqlaTag::ossimNitfPixqlaTag()
   : ossimNitfGenericTag("PIXQLA")
{
   initializeFieldDefinitions();
   initializeFields();
   setTagLength(computeTagLength());
}

ossimNitfPixqlaTag::ossimNitfPixqlaTag(ossim_uint32 tagLength)
   : ossimNitfGenericTag("PIXQLA", tagLength)
{
   initializeFieldDefinitions();

}
void ossimNitfPixqlaTag::initializeFieldDefinitions()
{
   FIELD_DEFINITIONS =
   {
      {"NUMAIS", 3, ASCII, 0, "ALL"},
      {"NUMAIS n", LOOP_START},
         {"AISDLVL", 3, U_INT},
      {"NUMAIS n", LOOP_END},
      {"NPIXQUAL", 4, U_INT, 0, "9"},
      {"PQ_BIT_VALUE", 1, U_INT, 0, "1"},
      {"NPIXQUAL n", LOOP_START},
         {"PQ_CONDITION", 40, ASCII, 0, "Fill"},
      {"NPIXQUAL n", LOOP_END}
   };
}

ossimString ossimNitfPixqlaTag::getClassName() const
{
   return ossimString("ossimNitfPixqlaTag");
}

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
      {ossim::nitf::NUMAIS_KW, 3, ASCII, 0, "ALL"},
      {ossim::nitf::NUMAIS_KW, LOOP_START},
         {ossim::nitf::AISDLVL_KW, 3, U_INT},
      {ossim::nitf::NUMAIS_KW, LOOP_END},
      {ossim::nitf::NPIXQUAL_KW, 4, U_INT, 0, "9"},
      {ossim::nitf::PQ_BIT_VALUE_KW, 1, U_INT, 0, "1"},
      {ossim::nitf::NPIXQUAL_KW, LOOP_START},
         {ossim::nitf::PQ_CONDITION_KW, 40, ASCII, 0, "Fill"},
      {ossim::nitf::NPIXQUAL_KW, LOOP_END}
   };
}

ossimString ossimNitfPixqlaTag::getClassName() const
{
   return ossimString("ossimNitfPixqlaTag");
}

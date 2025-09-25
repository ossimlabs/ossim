//---
//
// License: MIT
//
// Author:  David Burken
//
// Description: CSDIDA tag class definition.
//
// Dataset Indentification TRE.
//
// See document STDI-0006-NCDRD Table 3.3-14 for more info.
// 
//---
// $Id$

#include <ossim/support_data/ossimNitfCsdidaTag.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>

RTTI_DEF1(ossimNitfCsdidaTag, "ossimNitfCsdidaTag", ossimNitfRegisteredTag);

static ossimTrace traceDebug("ossimNitfCsdidaTag:debug");
static const int FOREVER = 1;
const std::string ossimNitfCsdidaTag::CETAG_KW = "CSDIDA";

ossimNitfCsdidaTag::ossimNitfCsdidaTag()
   : ossimNitfRegisteredTag(CETAG_KW, 70)
{
   clearFields();
}

ossimNitfCsdidaTag::~ossimNitfCsdidaTag()
{
}

void ossimNitfCsdidaTag::parseStream(std::istream& in)
{
   clearFields();

   in.read(theDay, 2);
   in.read(theMonth, 3);
   in.read(theYear, 4);
   in.read(thePlatformCode, 2);
   in.read(theVehicleId, 2);
   in.read(thePass, 2);
   in.read(theOperation, 3);
   in.read(theSensorId, 2);
   in.read(theProductId, 2);
   in.read(theReservedField1, 4);
   in.read(theTime, 14);
   in.read(theProcessTime, 14);
   in.read(theReservedField2, 2);
   in.read(theReservedField3, 2);
   in.read(theReservedField4, 1);
   in.read(theReservedField5, 1);
   in.read(theSoftwareVersionNumber, 10);
}

void ossimNitfCsdidaTag::writeStream(std::ostream& out)
{
   out.write(theDay, 2);
   out.write(theMonth, 3);
   out.write(theYear, 4);
   out.write(thePlatformCode, 2);
   out.write(theVehicleId, 2);
   out.write(thePass, 2);
   out.write(theOperation, 3);
   out.write(theSensorId, 2);
   out.write(theProductId, 2);
   out.write(theReservedField1, 4);
   out.write(theTime, 14);
   out.write(theProcessTime, 14);
   out.write(theReservedField2, 2);
   out.write(theReservedField3, 2);
   out.write(theReservedField4, 1);
   out.write(theReservedField5, 1);
   out.write(theSoftwareVersionNumber, 10);
}

void ossimNitfCsdidaTag::clearFields()
{
   // BCS-N's to '0's, BCS-A's to ' '(spaces)
   
   memset(theDay, '0', 2);
   memset(theMonth, ' ', 3);
   memset(theYear, '0', 4);
   memset(thePlatformCode, ' ', 2);
   memset(theVehicleId, '0', 2);
   memset(thePass, '0', 2);
   memset(theOperation, '0', 3);
   memset(theSensorId, ' ', 2);
   memset(theProductId, ' ', 2);
   memcpy(theReservedField1, "00  ", 4);
   memset(theTime, '0', 14);
   memset(theProcessTime, '0', 14);
   memcpy(theReservedField2, "00", 2);
   memcpy(theReservedField3, "01", 2);
   memcpy(theReservedField4, "N", 1);
   memcpy(theReservedField5, "N", 1);
   memset(theSoftwareVersionNumber, ' ', 10);

   theDay[2] = '\0';
   theMonth[3] = '\0';
   theYear[4] = '\0';
   thePlatformCode[2] = '\0';
   theVehicleId[2] = '\0';
   thePass[2] = '\0';
   theOperation[3] = '\0';
   theSensorId[2] = '\0';
   theProductId[2] = '\0';
   theReservedField1[4] = '\0';
   theTime[14] = '\0';
   theProcessTime[14] = '\0';
   theReservedField2[2] = '\0';
   theReservedField3[2] = '\0';
   theReservedField4[0] = '\0';
   theReservedField5[0] = '\0';
   theSoftwareVersionNumber[10] = '\0';
}

bool ossimNitfCsdidaTag::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   static const char MODULE[] = "ossimNitfCsdidaTag::loadState(...)";
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

   while(FOREVER) // Break on error or at end.
   {
      value = kwl.findKey( pfx, ossim::nitf::DAY_KW );
      if ( value.size() == 2 )
      {
         strcpy(theDay, value.c_str());
      }
      value = kwl.findKey( pfx, ossim::nitf::MONTH_KW );
      if ( value.size() == 3 )
      {
         strcpy(theMonth, value.c_str());
      }
      value = kwl.findKey( pfx, ossim::nitf::YEAR_KW );
      if ( value.size() == 4 )
      {
         strcpy(theYear, value.c_str());
      }
      value = kwl.findKey( pfx, ossim::nitf::PLATFORM_CODE_KW );
      if ( value.size() == 2 )
      {
         strcpy(thePlatformCode, value.c_str());
      }
      value = kwl.findKey( pfx, ossim::nitf::VEHICLE_ID_KW );
      if ( value.size() == 2 )
      {
         strcpy(theVehicleId, value.c_str());
      }
      value = kwl.findKey( pfx, ossim::nitf::PASS_KW );
      if ( value.size() == 2 )
      {
         strcpy(thePass, value.c_str());
      }
      value = kwl.findKey( pfx, ossim::nitf::OPERATION_KW );
      if ( value.size() == 3 )
      {
         strcpy(theOperation, value.c_str());
      }
      value = kwl.findKey( pfx, ossim::nitf::SENSOR_ID_KW );
      if ( value.size() == 2 )
      {
         strcpy(theSensorId, value.c_str());
      }
      value = kwl.findKey( pfx, ossim::nitf::PRODUCT_ID_KW );
      if ( value.size() == 2 )
      {
         strcpy(theProductId, value.c_str());
      }
      value = kwl.findKey( pfx, ossim::nitf::TIME_KW );
      if ( value.size() == 14 )
      {
         strcpy(theTime, value.c_str());
      }
      value = kwl.findKey( pfx, ossim::nitf::PROCESS_TIME_KW );
      if ( value.size() == 14 )
      {
         strcpy(theProcessTime, value.c_str());
      }
      ossim_uint32 fieldSize = 10;
      value = kwl.findKey( pfx, ossim::nitf::SOFTWARE_VERSION_NUMBER_KW );
      if ( value.size() <= fieldSize )
      {
         ossimNitfCommon::setField(theSoftwareVersionNumber, ossimString(value),
                                   fieldSize, std::ios::left, ' ');
         // strcpy(theSoftwareVersionNumber, value.c_str());
      }

      break; // Trailing break from forever loop.
      
   } // Matches: while(FOREVER)
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exit status: " << (status?"true":"false") << "\n";
   }
   
   return status;
}

std::ostream& ossimNitfCsdidaTag::print(
   std::ostream& out, const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:"
       << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"   << getTagLength() << "\n"
       << pfx << std::setw(24) << "DAY:" << theDay << "\n"
       << pfx << std::setw(24) << "MONTH:" << theMonth << "\n"
       << pfx << std::setw(24) << "YEAR:" << theYear << "\n"
       << pfx << std::setw(24) << "PLATFORM_CODE:" << thePlatformCode << "\n"
       << pfx << std::setw(24) << "VEHICLE_ID:" << theVehicleId << "\n"
       << pfx << std::setw(24) << "PASS:" << thePass << "\n"
       << pfx << std::setw(24) << "OPERATION:" << theOperation << "\n"
       << pfx << std::setw(24) << "SENSOR_ID:" << theSensorId << "\n"
       << pfx << std::setw(24) << "PRODUCT_ID:" << theProductId << "\n"
       << pfx << std::setw(24) << "TIME:" << theTime << "\n"
       << pfx << std::setw(24) << "PROCESS_TIME:" << theProcessTime << "\n"
       << pfx << std::setw(24) << "SOFTWARE_VERSION_NUMBER:"
       << theSoftwareVersionNumber << "\n";
   
   return out;
}

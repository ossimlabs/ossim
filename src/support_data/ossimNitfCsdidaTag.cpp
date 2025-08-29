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
// $Id

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>

#include <ossim/support_data/ossimNitfCsdidaTag.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>

RTTI_DEF1(ossimNitfCsdidaTag, "ossimNitfCsdidaTag", ossimNitfRegisteredTag);

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

   bool status = true;
#if 0   /* tmp drb */
   std::string pfx = prefix?prefix:"";
   std::string value;
   std::string os;
   ossim_uint32 fieldSize = 0;

   while(FOREVER) // Break on error or at end.
   {
      fieldSize = 36;
      value = kwl.findKey( pfx, IMAGE_UUID_KW );
      // Size must be exact. Currently no format check, only size.
      if ( value.size() == fieldSize ) 
      {
         m_fields_map.insert_or_assign(ossimString(IMAGE_UUID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING: Incorrect length of " << value.size()
            << " for " << IMAGE_UUID_KW << " field!" << std::endl;
         status = false;
         break;
      }

      // NUM_ASSOC_DES

      // ASSOC_DES_UUID

      // PLATFORM_ID
      fieldSize = 6;
      value = kwl.findKey( pfx, PLATFORM_ID_KW );
      if ( value.size() <= fieldSize )
      {
         if ( value.size() < fieldSize ) // Currently no format check, only size.
         {
            os = value;
            value.resize(fieldSize);
            ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
         }
         m_fields_map.insert_or_assign(ossimString(PLATFORM_ID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << PLATFORM_ID_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }      

      // PAYLOAD_ID
      value = kwl.findKey( pfx, PAYLOAD_ID_KW );
      if ( value.size() <= fieldSize )
      {
         if ( value.size() < fieldSize ) // Currently no format check, only size.
         {
            os = value;
            value.resize(fieldSize);
            ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
         }
         m_fields_map.insert_or_assign(ossimString(PAYLOAD_ID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << PAYLOAD_ID_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }     

      // SENSOR_ID
      value = kwl.findKey( pfx, SENSOR_ID_KW );
      if ( value.size() <= fieldSize )
      {
         if ( value.size() < fieldSize ) // Currently no format check, only size.
         {
            os = value;
            value.resize(fieldSize);
            ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
         }
         m_fields_map.insert_or_assign(ossimString(SENSOR_ID_KW), ossimString(value));
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << SENSOR_ID_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }

      // SENSOR_TYPE
      fieldSize = 1;
      value = kwl.findKey( pfx, SENSOR_TYPE_KW );
      if ( value.size() == fieldSize )
      {
         m_fields_map.insert_or_assign(ossimString(SENSOR_TYPE_KW), ossimString(value));

         if ( value == "S" ) // If scan we need these three fields.
         {
            // DAY_FIRST_LINE_IMAGE:
            fieldSize = 8;
            value = kwl.findKey( pfx, DAY_FIRST_LINE_IMAGE_KW );
            if ( value.size() < fieldSize ) // Currently no format check, only size.
            {
               os = value;
               value.resize(fieldSize);
               ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
            }
            m_fields_map.insert_or_assign(
               ossimString(DAY_FIRST_LINE_IMAGE_KW), ossimString(value));

            // TIME_FIRST_LINE_IMAGE:
            fieldSize = 15;
            value = kwl.findKey( pfx, TIME_FIRST_LINE_IMAGE_KW );
            if ( value.size() < fieldSize ) // Currently no format check, only size.
            {
               os = value;
               value.resize(fieldSize);
               ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
            }
            m_fields_map.insert_or_assign(
               ossimString(TIME_FIRST_LINE_IMAGE_KW ), ossimString(value));
            
            // TIME_IMAGE_DURATION:
            fieldSize = 16;
            value = kwl.findKey( pfx, TIME_IMAGE_DURATION_KW );
            if ( value.size() < fieldSize ) // Currently no format check, only size.
            {
               os = value;
               value.resize(fieldSize);
               ossimNitfCommon::setField(value.data(), os, fieldSize, std::ios::left, ' ');
            }
            m_fields_map.insert_or_assign(
               ossimString(TIME_IMAGE_DURATION_KW), ossimString(value));
         }
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n" << SENSOR_TYPE_KW
            << " key value has incorrect length of "
            << value.size() << " for field!" << " value: " << value << std::endl;
         status = false;
         break;
      }      

      break; // Trailing break from forever loop.
      
   } // Matches: while(FOREVER)

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exit status" << (status?"true\n":"false\n");
   }
#endif   
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

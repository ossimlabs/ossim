//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: Generic tag class definition.
//
//----------------------------------------------------------------------------
#include <ossim/support_data/ossimNitfGenericTag.h>
#include <ossim/support_data/ossimNitfCommon.h>

#include <istream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <utility>

#include "base/ossimException.h"

ossimNitfGenericTag::ossimNitfGenericTag(ossimString tag, ossim_uint32 tagLength)
   : ossimNitfRegisteredTag(tag, tagLength)
{
}

static ossimString formatSuffix(std::vector<std::vector<ossim_int32> > suffixIn)
{
   ossimString result = "";
   for (std::vector<ossim_int32> set: suffixIn)
   {
      char separator = set[3];
      result += separator + std::to_string(set[0]);
   }
   return result;
}

void ossimNitfGenericTag::readDefinitions(int actionFunction, std::istream &in, std::ostream &out, ossimString args)
{
   char fieldContentsBuffer[256];
   std::vector<std::pair<ossimString, ossim_int32>> result;
   std::vector<std::vector<ossim_int32>> suffix;
   std::vector<ossimString> spaceSubStrings, colonSubStrings;
   ossim_int32 fieldLength, i = 0;
   ossimString generatedFieldName;
   bool ifCondition;

   while (i < NUM_DEFINITIONS)
   {
      spaceSubStrings.clear();
      colonSubStrings.clear();
      switch (FIELD_DEFINITIONS[i].size)
      {
         case IF_STATEMENT_START:
            FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
            spaceSubStrings[0].split(colonSubStrings, ':');
            generatedFieldName = m_fields_map[colonSubStrings[0] + formatSuffix(suffix)];
            if (colonSubStrings.size() > 1 && generatedFieldName.length() > colonSubStrings[1].toInt())
               generatedFieldName = generatedFieldName.at(colonSubStrings[1].toInt());
            if (spaceSubStrings[1] == "==")
               ifCondition = (generatedFieldName == spaceSubStrings[2]);
            else if (spaceSubStrings[1] == "!=")
               ifCondition = (generatedFieldName != spaceSubStrings[2]);
            else
               ifCondition = false;
            if (!ifCondition)
               while (FIELD_DEFINITIONS[i].size != IF_STATEMENT_END)
                  i++;
            i++;
            break;
         case IF_STATEMENT_END:
            i++;
            break;
         case LOOP_START:
            FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
            fieldLength = m_fields_map[spaceSubStrings[0] + formatSuffix(suffix)].toInt();
            if (fieldLength > 0)
               suffix.push_back({1, fieldLength, i + 1, spaceSubStrings[1].at(0)});
            else
            {
               int loopCount = 1;
               while (loopCount > 0)
               {
                  i++;
                  if (FIELD_DEFINITIONS[i].size == LOOP_END)
                     loopCount--;
                  else if (FIELD_DEFINITIONS[i].size == LOOP_START)
                     loopCount++;
               }
            }
            i++;
            break;
         case LOOP_END:
            suffix.back()[0]++;
            if (suffix.back()[0] <= suffix.back()[1])
            {
               i = suffix.back()[2];
            } else
            {
               suffix.pop_back();
               i++;
            }
            break;
         default:
            if (FIELD_DEFINITIONS[i].size == VARIABLE_LENGTH)
            {
               FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
               generatedFieldName = spaceSubStrings[0] + formatSuffix(suffix);
               fieldLength = m_fields_map[spaceSubStrings[1] + formatSuffix(suffix)].toInt();
            }
            else
            {
               generatedFieldName = FIELD_DEFINITIONS[i].field + formatSuffix(suffix);
               fieldLength = FIELD_DEFINITIONS[i].size;
            }
            switch (actionFunction)
            {
               case PARSE_STREAM:
                  in.read(fieldContentsBuffer, fieldLength);
                  fieldContentsBuffer[fieldLength] = '\0';
                  m_fields_map.insert(std::pair<ossimString, ossimString>(generatedFieldName, fieldContentsBuffer));
                  break;
            case SET_FIELD:
                  if (m_fields_map.count(generatedFieldName) == 0)
                     m_fields_map.insert(std::pair<ossimString, ossimString>(generatedFieldName, std::string(fieldLength, ' ')));
                  break;
               case WRITE_STREAM:
                  out.write(m_fields_map[generatedFieldName], fieldLength);
                  break;
               case PRINT:
                  out << std::setiosflags(std::ios::left)
                      << args << std::setw(24) << generatedFieldName << ":"
                      << m_fields_map[generatedFieldName] << "\n";
                  break;
               default:
                  break;
            }
            i++;
            break;
      }
   }
}

void ossimNitfGenericTag::parseStream(std::istream &in)
{
   clearFields();

   m_fields_map.clear();

   readDefinitions(PARSE_STREAM, in, std::cout);
}

void ossimNitfGenericTag::writeStream(std::ostream &out)
{
   readDefinitions(WRITE_STREAM, std::cin, out);
}

std::ostream &ossimNitfGenericTag::print(std::ostream &out, const std::string &prefix)
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";

   out << std::setiosflags(std::ios::left)
         << pfx << std::setw(24) << "CETAG:"
         << getTagName() << "\n"
         << pfx << std::setw(24) << "CEL:"
         << getTagLength() << "\n";

   readDefinitions(PRINT, std::cin, out, pfx);

   return out;
}

void ossimNitfGenericTag::clearFields()
{
   m_fields_map.clear();
}

ossimString ossimNitfGenericTag::get(ossimString fieldName)
{
   return m_fields_map[fieldName];
}

void ossimNitfGenericTag::setField(ossimString fieldName, ossimString fieldValue)
{
   //Formatting
   int definition = 0;
   for (int i=0; i < NUM_DEFINITIONS; i++)
   {
      if (FIELD_DEFINITIONS[i].size >= VARIABLE_LENGTH &&
         FIELD_DEFINITIONS[i].field.length() >= fieldName.length() &&
         fieldName == FIELD_DEFINITIONS[i].field.substr(0, fieldName.length()))
      {
         definition = i;
         break;
      }
   }
   int length = FIELD_DEFINITIONS[definition].size;
   if (length == VARIABLE_LENGTH)
   {
      std::vector<ossimString> spaceSubStrings;
      FIELD_DEFINITIONS[definition].field.split(spaceSubStrings, ' ');
      length = m_fields_map[spaceSubStrings[1]].toInt();
   }
   if (fieldValue.size() != length)
   {
      switch (FIELD_DEFINITIONS[definition].formatMethod[0])
      {
         case 1:
            fieldValue = ossimNitfCommon::convertToUIntString(fieldValue.toUInt32(),
               FIELD_DEFINITIONS[definition].size);
            break;
         case 2:
            fieldValue = ossimNitfCommon::convertToIntString(fieldValue.toInt32(),
               FIELD_DEFINITIONS[definition].size);
            break;
         case 3:
            fieldValue = ossimNitfCommon::convertToDoubleString(fieldValue.toFloat64(),
               FIELD_DEFINITIONS[definition].formatMethod[1],
               FIELD_DEFINITIONS[definition].size);
            break;
         case 4:
            if (fieldValue.toFloat64() > 0)
               fieldValue = "+" + ossimNitfCommon::convertToDoubleString(fieldValue.toFloat64(),
                                 FIELD_DEFINITIONS[definition].formatMethod[1],
                                    FIELD_DEFINITIONS[definition].size);
            else
               fieldValue = ossimNitfCommon::convertToDoubleString(fieldValue.toFloat64(),
                                 FIELD_DEFINITIONS[definition].formatMethod[1],
                                    FIELD_DEFINITIONS[definition].size);
         default:
            while (fieldValue.length() < length)
               fieldValue = fieldValue + ' ';
            break;
      }
   }
   m_fields_map[fieldName] = fieldValue;
   readDefinitions(SET_FIELD, std::cin, std::cout, fieldName);
}
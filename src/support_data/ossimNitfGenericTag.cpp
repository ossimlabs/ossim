//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Tagcription: Generic tag class definition.
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
#include <stack>

#include "base/ossimException.h"
#include "base/ossimTrace.h"

ossimNitfGenericTag::ossimNitfGenericTag(const std::string& tag, ossim_uint32 tagLength)
   : ossimNitfRegisteredTag(tag, tagLength)
{
}

ossimNitfGenericTag::definition::definition::definition(const ossimString& field, ossim_int32 size,
                       ossim_int8 dataFormat, ossim_int8 precision,
                       const ossimString& defaultValue)
   :
   field(field),
   size(size),
   dataFormat(dataFormat),
   precision(precision),
   defaultValue(defaultValue)
{

}

std::ostream& ossimNitfGenericTag::definition::print(std::ostream& out) const
{
   out << "field: " << field << "\n"
       << "size: " << size << "\n"
       << "dataFormat: " << dataFormat << "\n"
       << "precision: " << precision << "\n"
       << "defaultValue: " << defaultValue;
   return out;
}

std::ostream& operator<<(std::ostream& out, const ossimNitfGenericTag::definition& def)
{
   return def.print(out);
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

//Parse statements in reverse polish notation
int ossimNitfGenericTag::parseRPN(ossimString input, std::vector<std::vector<ossim_int32>> suffixIn) const
{
   std::vector<ossimString> splitInput = input.split(' ');
   std::stack<ossimString> stack;
   ossimString a, b;
   std::vector<ossimString> colonSubStrings;
   for(ossimString entry: splitInput)
   {
      switch(entry.at(0))
      {
         case '+':
            a = stack.top();
            stack.pop();
            b = stack.top();
            stack.pop();
            stack.push(a.toInt() + b.toInt());
            break;
         case '-':
            a = stack.top();
            stack.pop();
            b = stack.top();
            stack.pop();
            stack.push(a.toInt()- b.toInt());
            break;
         case '*':
            a = stack.top();
            stack.pop();
            b = stack.top();
            stack.pop();
            stack.push(a.toInt()* b.toInt());
            break;
         case '/':
            a = stack.top();
            stack.pop();
            b = stack.top();
            stack.pop();
            stack.push(a.toInt()/ b.toInt());
            break;
         case '&':
            a = stack.top();
            stack.pop();
            b = stack.top();
            stack.pop();
            stack.push(bool(a) && bool(b));
            break;
         case '|':
            a = stack.top();
            stack.pop();
            b = stack.top();
            stack.pop();
            stack.push(bool(a) || bool(b));
            break;
         case '=':
            a = stack.top();
            stack.pop();
            b = stack.top();
            stack.pop();
            if(a == b)
               stack.push("1");
            else
               stack.push("0");
            break;
         case '!':
            a = stack.top();
            stack.pop();
            stack.push(!bool(a));
            break;
         default:
            if(entry.toInt() != 0 || entry == "0")
               stack.push(entry.toInt());
            else
            {
               if(entry[0] == '\'')
                  stack.push(entry.substr(1, entry.length() - 2));
               else
               {
                  colonSubStrings = entry.split(':');
                  if(colonSubStrings.size() > 1)
                     stack.push(m_fields_map.at(colonSubStrings[0] + formatSuffix(suffixIn))[colonSubStrings[1].toInt()]);
                  else
                     stack.push(m_fields_map.at(entry + formatSuffix(suffixIn)));
               }
            }
            break;
      }
   }
   if(stack.size() > 1)
      std::cout << "EEEEEE" << std::endl;
   return stack.top().toInt();
}

void ossimNitfGenericTag::loopLogic(ossim_int32 &i, std::vector<std::vector<ossim_int32>> &suffix) const
{
    ossim_int32 fieldLength;
    bool ifCondition;
    switch (FIELD_DEFINITIONS[i].size)
    {
         case IF_STATEMENT_START:
            ifCondition = parseRPN(FIELD_DEFINITIONS[i].field, suffix);
            if (!ifCondition)
            {
               int loopCount = 1;
               while (loopCount > 0)
               {
                  i++;
                  if (FIELD_DEFINITIONS[i].size == IF_STATEMENT_END)
                     loopCount--;
                  else if (FIELD_DEFINITIONS[i].size == IF_STATEMENT_START)
                     loopCount++;
               }
            }
            i++;
            break;
         case IF_STATEMENT_END:
            i++;
            break;
         case LOOP_START:
            fieldLength = parseRPN(FIELD_DEFINITIONS[i].field.substr(0, FIELD_DEFINITIONS[i].field.length() - 2) , suffix);
            if (fieldLength > 0)
               suffix.push_back({1, fieldLength, i + 1, FIELD_DEFINITIONS[i].field.at(FIELD_DEFINITIONS[i].field.length() - 1)});
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
    }
}

ossimString ossimNitfGenericTag::formatField(int definition, const ossimString& fieldValue) const
{
   ossimString result = fieldValue;
   ossim_int8 format = FIELD_DEFINITIONS[definition].dataFormat;
   if (result.empty())
   {
      result = FIELD_DEFINITIONS[definition].defaultValue;
      if (result == " ")
         format = -1;
   }

   ossim_uint32 length = FIELD_DEFINITIONS[definition].size;
   if (length == VARIABLE_LENGTH)
   {
      std::vector<ossimString> spaceSubStrings;
      FIELD_DEFINITIONS[definition].field.split(spaceSubStrings, ' ');
      length = m_fields_map.at(spaceSubStrings[1]).toInt();
   }
   if (result.size() != length)
   {
      switch (format)
      {
         case 1:
            result = ossimNitfCommon::convertToUIntString(result.toUInt32(),
               length);
            break;
         case 2:
            result = ossimNitfCommon::convertToIntString(result.toInt32(),
               length);
            break;
         case 3:
            result = ossimNitfCommon::convertToDoubleString(result.toFloat64(),
               FIELD_DEFINITIONS[definition].precision,
               length);
            break;
         case 4:
            if (result.toFloat64() > 0)
               result = "+" + ossimNitfCommon::convertToDoubleString(result.toFloat64(),
                                 FIELD_DEFINITIONS[definition].precision,
                                    length);
            else
               result = ossimNitfCommon::convertToDoubleString(result.toFloat64(),
                                 FIELD_DEFINITIONS[definition].precision,
                                    length);
         case 5:
            result = ossimNitfCommon::convertToScientificString(result.toFloat64(), length);
         default:
            while (result.length() < length)
               result = result + ' ';
            break;
      }
   }
   return result;
}

void ossimNitfGenericTag::parseStream(std::istream &in)
{
   clearFields();

   m_fields_map.clear();

   //copy-pasted looping logic
   char fieldContentsBuffer[256];
   std::vector<std::vector<ossim_int32>> suffix;
   std::vector<ossimString> spaceSubStrings;
   ossim_int32 fieldLength, i = 0;
   ossimString generatedFieldName;
   while ((ossim_uint32)i < FIELD_DEFINITIONS.size())
   {
      spaceSubStrings.clear();
      if(FIELD_DEFINITIONS[i].size < -1)
      {
         loopLogic(i, suffix);
      }
      else
      {
         if (FIELD_DEFINITIONS[i].size == VARIABLE_LENGTH)
         {
            FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
            generatedFieldName = spaceSubStrings[0] + formatSuffix(suffix);
            fieldLength = m_fields_map.at(spaceSubStrings[1] + formatSuffix(suffix)).toInt();
         }
         else
         {
            generatedFieldName = FIELD_DEFINITIONS[i].field + formatSuffix(suffix);
            fieldLength = FIELD_DEFINITIONS[i].size;
         }
         //Unique actions for parseStream
         in.read(fieldContentsBuffer, fieldLength);
         fieldContentsBuffer[fieldLength] = '\0';
         m_fields_map.insert(std::pair<ossimString, ossimString>(generatedFieldName, fieldContentsBuffer));
         i++;
      }
   }
}

void ossimNitfGenericTag::writeStream(std::ostream &out)
{
   //Start of copy-pasted block
   std::vector<std::pair<ossimString, ossim_int32>> result;
   std::vector<std::vector<ossim_int32>> suffix;
   std::vector<ossimString> spaceSubStrings;
   ossim_int32 fieldLength, i = 0;
   ossimString generatedFieldName;

   while ((ossim_uint32)i < FIELD_DEFINITIONS.size())
   {
      spaceSubStrings.clear();
      if(FIELD_DEFINITIONS[i].size < -1)
      {
         loopLogic(i, suffix);
      }
      else
      {
         if (FIELD_DEFINITIONS[i].size == VARIABLE_LENGTH)
         {
            FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
            generatedFieldName = spaceSubStrings[0] + formatSuffix(suffix);
            fieldLength = m_fields_map.at(spaceSubStrings[1] + formatSuffix(suffix)).toInt();
         }
         else
         {
            generatedFieldName = FIELD_DEFINITIONS[i].field + formatSuffix(suffix);
            fieldLength = FIELD_DEFINITIONS[i].size;
         }
         out.write(m_fields_map.at(generatedFieldName), fieldLength);
         i++;
      }
   }
}

std::ostream &ossimNitfGenericTag::print(std::ostream &out, const std::string &prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";

   out << std::setiosflags(std::ios::left)
         << pfx << std::setw(24) << "CETAG:"
         << getTagName() << "\n"
         << pfx << std::setw(24) << "CEL:"
         << getTagLength() << "\n";

   std::vector<std::vector<ossim_int32>> suffix;
   std::vector<ossimString> spaceSubStrings;
   ossim_int32 i = 0;
   ossimString generatedFieldName;

   while ((ossim_uint32)i < FIELD_DEFINITIONS.size())
   {
      if(FIELD_DEFINITIONS[i].size < -1)
      {
         loopLogic(i, suffix);
      }
      else
      {
         if (FIELD_DEFINITIONS[i].size == VARIABLE_LENGTH)
         {
            FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
            generatedFieldName = spaceSubStrings[0] + formatSuffix(suffix);
         }
         else
         {
            generatedFieldName = FIELD_DEFINITIONS[i].field + formatSuffix(suffix);
         }
         //Unique print actions
         out << std::setiosflags(std::ios::left)
             << pfx << std::setw(24) << generatedFieldName << ":"
             << m_fields_map.at(generatedFieldName) << "\n";
         i++;
      }
   }
   return out;
}

void ossimNitfGenericTag::clearFields()
{
   m_fields_map.clear();
}

ossimString ossimNitfGenericTag::get(const ossimString& fieldName)
{
   return m_fields_map.at(fieldName);
}

void ossimNitfGenericTag::setField(const ossimString& fieldName, const ossimString& fieldValue)
{
   int definition = -1;
   for (int i=0; i < FIELD_DEFINITIONS.size(); i++)
   {
      if (FIELD_DEFINITIONS[i].size >= VARIABLE_LENGTH &&
         FIELD_DEFINITIONS[i].field.length() >= fieldName.length() &&
         fieldName == FIELD_DEFINITIONS[i].field.substr(0, fieldName.length()))
      {
         definition = i;
         break;
      }
   }
   if (definition > -1)
      m_fields_map.insert_or_assign(fieldName, formatField(definition, fieldValue));
   initaliseFields();
}

void ossimNitfGenericTag::initaliseFields()
{
   std::vector<std::vector<ossim_int32>> suffix;
   std::vector<ossimString> spaceSubStrings;
   ossim_int32 fieldLength, i = 0;
   ossimString generatedFieldName;

   while ((ossim_uint32)i < FIELD_DEFINITIONS.size())
   {
      spaceSubStrings.clear();
      if(FIELD_DEFINITIONS[i].size < -1)
      {
         loopLogic(i, suffix);
      }
      else
      {
         if (FIELD_DEFINITIONS[i].size == VARIABLE_LENGTH)
         {
            FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
            generatedFieldName = spaceSubStrings[0] + formatSuffix(suffix);
         } else
         {
            generatedFieldName = FIELD_DEFINITIONS[i].field + formatSuffix(suffix);
         }
         //Unique setField actions
         if (m_fields_map.count(generatedFieldName) == 0)
            m_fields_map.insert(std::pair(generatedFieldName, formatField(i, "")));
         i++;
      }
   }
}

ossim_uint32 ossimNitfGenericTag::computeTagLength() const
{
   ossim_uint32 length = 0;
   for ( const auto& i : m_fields_map )
   {
      length += i.second.string().size();
   }
   return length;
}

std::ostream& ossimNitfGenericTag::printMap(std::ostream& out ) const
{
   for ( const auto& i : m_fields_map )
   {
      out << i.first << ": " << i.second << "\n";
   }
   out << std::endl;
   return out;
}

std::ostream& ossimNitfGenericTag::printFieldDefs(std::ostream& out ) const
{
   for ( const auto& def : FIELD_DEFINITIONS )
   {
      out << def;
   }
   out << std::endl;
   return out;
}
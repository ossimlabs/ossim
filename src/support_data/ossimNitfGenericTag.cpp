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
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>

#include <istream>
#include <iostream>
#include <iomanip>
#include <stack>
#include <utility>

static ossimTrace traceDebug("ossimNitfGenericTag:debug");

std::ostream& ossimNitfGenericTag::definition::print(std::ostream& out) const
{
   out << "field: " << field
       << "\nsize: " << size << "\n";
   for ( ossim_uint32 i = 0; i < formatMethod.size(); ++i)
   {
      out << "formatMethod[" << i << "]: " << (int)formatMethod[i] << "\n";
   }
   return out;
}

std::ostream& operator<<(std::ostream& out, const ossimNitfGenericTag::definition& def)
{
   return def.print(out);
}

ossimNitfGenericTag::ossimNitfGenericTag(ossimString tag, ossim_uint32 tagLength)
   : ossimNitfRegisteredTag(tag, tagLength)
{
   // Uncomment to hard code on trace for class.
   // traceDebug.setTraceFlag(true);
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
   // int colPosition;
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

void ossimNitfGenericTag::parseStream(std::istream &in)
{
   static const char MODULE[] = "ossimNitfGenericTag::parseStream";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   clearFields();

   //copy-pasted looping logic
   char fieldContentsBuffer[256];
   std::vector<std::pair<ossimString, ossim_int32>> result;
   std::vector<std::vector<ossim_int32>> suffix;
   std::vector<ossimString> spaceSubStrings, colonSubStrings;
   ossim_int32 fieldLength, i = 0;
   ossimString generatedFieldName;
   bool ifCondition;

   while ((ossim_uint32)i < FIELD_DEFINITIONS.size())
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "FIELD_DEFINITIONS[" << i << "]:\n" << FIELD_DEFINITIONS[i] << "\n";
      }
      
      spaceSubStrings.clear();
      colonSubStrings.clear();
      switch (FIELD_DEFINITIONS[i].size)
      {
         case IF_STATEMENT_START:
            ifCondition = bool(parseRPN(FIELD_DEFINITIONS[i].field, suffix));
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
         default:
            if ((ossim_int32)FIELD_DEFINITIONS[i].size == VARIABLE_LENGTH)
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
            break;
      }
   }

   // Recompute and set tag length. This is needed for conditional tags.
   setTagLength(computeTagLength());

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited..." << std::endl;
   }
}

void ossimNitfGenericTag::writeStream(std::ostream &out)
{
   static const char MODULE[] = "ossimNitfGenericTag::writeStream";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }
   
   //Start of copy-pasted block
   std::vector<std::pair<ossimString, ossim_int32>> result;
   std::vector<std::vector<ossim_int32>> suffix;
   std::vector<ossimString> spaceSubStrings, colonSubStrings;
   ossim_int32 fieldLength, i = 0;
   ossimString generatedFieldName;
   bool ifCondition;

   while ((ossim_uint32)i < FIELD_DEFINITIONS.size())
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "FIELD_DEFINITIONS[" << i << "]:\n" << FIELD_DEFINITIONS[i] << "\n";
      }
            
      spaceSubStrings.clear();
      colonSubStrings.clear();
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
         default:
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
            break;
      }
   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
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

   //Start of copy-pasted block
   std::vector<std::pair<ossimString, ossim_int32>> result;
   std::vector<std::vector<ossim_int32>> suffix;
   std::vector<ossimString> spaceSubStrings, colonSubStrings;
   ossim_int32 fieldLength, i = 0;
   ossimString generatedFieldName;
   bool ifCondition;

   while ((ossim_uint32)i < FIELD_DEFINITIONS.size())
   {
      spaceSubStrings.clear();
      colonSubStrings.clear();
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
         default:
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
            //Unique print actions
            out << std::setiosflags(std::ios::left)
                << pfx << std::setw(24) << generatedFieldName << ":"
                << m_fields_map.at(generatedFieldName) << "\n";
            i++;
            break;
      }
      }

   return out;
}

void ossimNitfGenericTag::clearFields()
{
   m_fields_map.clear();
}

ossimString ossimNitfGenericTag::get(ossimString fieldName)
{
   return m_fields_map.at(fieldName);
}

void ossimNitfGenericTag::setField(ossimString fieldName, ossimString fieldValue)
{
   //Formatting
   ossim_uint32 definition = 0;
   for (ossim_uint32 i=0; i < FIELD_DEFINITIONS.size(); i++)
   {
      if (FIELD_DEFINITIONS[i].size >= VARIABLE_LENGTH &&
         FIELD_DEFINITIONS[i].field.length() >= fieldName.length() &&
         fieldName == FIELD_DEFINITIONS[i].field.substr(0, fieldName.length()))
      {
         definition = i;
         break;
      }
   }
   ossim_uint32 length = FIELD_DEFINITIONS[definition].size;
   if ((ossim_int32)length == VARIABLE_LENGTH) // VARIABLE_LENGTH is signed -1
   {
      std::vector<ossimString> spaceSubStrings;
      FIELD_DEFINITIONS[definition].field.split(spaceSubStrings, ' ');
      length = m_fields_map.at(spaceSubStrings[1]).toInt();
   }
   if (fieldValue.size() != length)
   {
      switch (FIELD_DEFINITIONS[definition].formatMethod[0])
      {
         case 1:
            fieldValue = ossimNitfCommon::convertToUIntString(fieldValue.toUInt32(),
               length);
            break;
         case 2:
            fieldValue = ossimNitfCommon::convertToIntString(fieldValue.toInt32(),
               length);
            break;
         case 3:
            fieldValue = ossimNitfCommon::convertToDoubleString(fieldValue.toFloat64(),
               FIELD_DEFINITIONS[definition].formatMethod[1],
               length);
            break;
         case 4:
            if (fieldValue.toFloat64() > 0)
               fieldValue = "+" + ossimNitfCommon::convertToDoubleString(fieldValue.toFloat64(),
                                 FIELD_DEFINITIONS[definition].formatMethod[1],
                                    length);
            else
               fieldValue = ossimNitfCommon::convertToDoubleString(fieldValue.toFloat64(),
                                 FIELD_DEFINITIONS[definition].formatMethod[1],
                                    length);
         case 5:
            fieldValue = ossimNitfCommon::convertToScientificString(fieldValue.toFloat64(), length);
         default:
            while (fieldValue.length() < length)
               fieldValue = fieldValue + ' ';
            break;
      }
   }
   m_fields_map.at(fieldName) = fieldValue;

   //Start of copy-pasted code
   std::vector<std::pair<ossimString, ossim_int32>> result;
   std::vector<std::vector<ossim_int32>> suffix;
   std::vector<ossimString> spaceSubStrings, colonSubStrings;
   ossim_int32 fieldLength, i = 0;
   ossimString generatedFieldName;
   bool ifCondition;

   while ((ossim_uint32)i < FIELD_DEFINITIONS.size())
   {
      spaceSubStrings.clear();
      colonSubStrings.clear();
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
         default:
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
            //Unique setField actions
            if (m_fields_map.count(generatedFieldName) == 0)
               m_fields_map.insert(std::pair<ossimString, ossimString>(generatedFieldName, std::string(fieldLength, '0')));
            i++;
            break;
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

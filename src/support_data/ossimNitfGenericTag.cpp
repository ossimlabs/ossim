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
// $Id
#include <ossim/support_data/ossimNitfGenericTag.h>

#include <istream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <utility>

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

std::vector<std::pair<ossimString, ossim_int32>> ossimNitfGenericTag::readDefinitions(int start)
{
   std::cout << "Definitions";
   std::vector<std::pair<ossimString, ossim_int32>> result;
   std::vector<std::vector<ossim_int32> > suffix;
   std::vector<ossimString> spaceSubStrings, colonSubStrings;
   ossim_int32 length, i = start;
   ossimString name, prevName;
   bool condition;

   while (i < NUM_DEFINITIONS)
   {
      spaceSubStrings.clear();
      colonSubStrings.clear();
      switch (FIELD_DEFINITIONS[i].size)
      {
         //variable length
         case -1:
            FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
            name = spaceSubStrings[0] + formatSuffix(suffix);
            if (spaceSubStrings.size() == 1)
            {
               length = m_fields_vector.back().second.toInt();
            } else
            {
               length = m_fields_map[spaceSubStrings[1] + formatSuffix(suffix)].toInt();
            }
            result.push_back(std::make_pair(name, length));
            i++;
            break;
         //if start
         case -2:
            FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
            spaceSubStrings[0].split(colonSubStrings, ':');
            name = m_fields_map[colonSubStrings[0] + formatSuffix(suffix)];
            if (colonSubStrings.size() > 1)
               name = name.at(colonSubStrings[1].toInt());

            if (spaceSubStrings[1] == "==")
               condition = (name == spaceSubStrings[2]);
            else if (spaceSubStrings[1] == "!=")
               condition = (name != spaceSubStrings[2]);
            else
               condition = false;
            if (!condition)
               while (FIELD_DEFINITIONS[i].size != -3)
                  i++;
            i++;
            break;
         //if end
         case -3:
            i++;
            break;
         //loop start
         case -4:
            FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
            length = m_fields_map[spaceSubStrings[0] + formatSuffix(suffix)].toInt();
            if (length > 0)
               suffix.push_back({1, length, i + 1, spaceSubStrings[1].at(0)});
            else
               while (FIELD_DEFINITIONS[i].size != -5)
                  i++;
            i++;
            break;
         //loop end
         case -5:
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
         //length provided
         default:
            result.push_back(std::make_pair(name, length));
            i++;
            break;
      };
   }
}

void ossimNitfGenericTag::parseStream(std::istream &in)
{
   clearFields();

   m_fields_map.clear();
   m_fields_vector.clear();

   char value[256];

   std::vector<std::pair<ossimString, ossim_int32>> actionItems = readDefinitions(0);
   for (std::pair item: actionItems)
   {
      in.read(value, item.second);
      value[item.second] = '\0';
      m_fields_map.insert(std::pair<ossimString, ossimString>(item.first, value));
      m_fields_vector.push_back(std::pair<ossimString, ossimString>(item.first, value));
   }
}

void ossimNitfGenericTag::writeStream(std::ostream &out)
{
   for (std::pair field: m_fields_vector)
   {
      out.write(field.second, field.second.size());
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

   for (std::pair field: m_fields_vector)
   {
      out << std::setiosflags(std::ios::left)
            << pfx << std::setw(24) << field.first << ":"
            << field.second << "\n";
   }

   return out;
}

void ossimNitfGenericTag::clearFields()
{
   m_fields_map.clear();
   m_fields_vector.clear();
}

ossimString ossimNitfGenericTag::get(ossimString name)
{
   return m_fields_map[name];
}

void ossimNitfGenericTag::setField(ossimString name, ossimString value)
{
   //Formatting
   int definition = 0;
   for (int i=0; i < NUM_DEFINITIONS; i++)
      if (name == FIELD_DEFINITIONS[i].field.substr(name.length()))
         definition = i;
   switch (FIELD_DEFINITIONS[definition].specs)
   {
      case ' ':
         while (value.length() < FIELD_DEFINITIONS[definition].size)
            value = value + ' ';
         break;
      case '0':
         while (value.length() < FIELD_DEFINITIONS[definition].size)
            value = '0' + value;
         break;
      default:
         break;
   };
}
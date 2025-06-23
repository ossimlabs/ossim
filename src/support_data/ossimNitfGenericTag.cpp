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

ossimNitfGenericTag::ossimNitfGenericTag(ossimString tag)
   : ossimNitfRegisteredTag(tag, 0)
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

void ossimNitfGenericTag::parseStream(std::istream &in)
{
   clearFields();

   m_fields_map.clear();
   m_fields_vector.clear();

   //Current iteration, Total iteration, i value
   std::vector<std::vector<ossim_int32> > suffix;
   std::vector<ossimString> spaceSubStrings, colonSubStrings;
   ossim_int32 length, i = 0;
   ossimString name, prevName;
   char value[256];
   bool condition;

   while (i < NUM_DEFINITIONS)
   {
      spaceSubStrings.clear();
      colonSubStrings.clear();
      switch (FIELD_DEFINITIONS[i].second)
      {
         //variable length
         case -1:
            FIELD_DEFINITIONS[i].first.split(spaceSubStrings, ' ');
            name = spaceSubStrings[0] + formatSuffix(suffix);
            if (spaceSubStrings.size() == 1)
            {
               in.read(value, m_fields_vector.back().second.toInt());
               value[m_fields_vector.back().second.toInt()] = '\0';
            } else
            {
               length = m_fields_map[spaceSubStrings[1] + formatSuffix(suffix)].toInt();
               in.read(value, length);
               value[length] = '\0';
            }
            m_fields_map.insert(std::pair<ossimString, ossimString>(name, value));
            m_fields_vector.push_back(std::pair<ossimString, ossimString>(name, value));
            i++;
            break;
         //if start
         case -2:
            FIELD_DEFINITIONS[i].first.split(spaceSubStrings, ' ');
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
               while (FIELD_DEFINITIONS[i].second != -3)
                  i++;
            i++;
            break;
         //if end
         case -3:
            i++;
            break;
         //loop start
         case -4:
            FIELD_DEFINITIONS[i].first.split(spaceSubStrings, ' ');
            length = m_fields_map[spaceSubStrings[0] + formatSuffix(suffix)].toInt();
            if (length > 0)
               suffix.push_back({1, length, i + 1, spaceSubStrings[1].at(0)});
            else
               while (FIELD_DEFINITIONS[i].second != -5)
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
            name = FIELD_DEFINITIONS[i].first + formatSuffix(suffix);
            in.read(value, FIELD_DEFINITIONS[i].second);
            value[FIELD_DEFINITIONS[i].second] = '\0';
            m_fields_map.insert(std::pair<ossimString, ossimString>(name, value));
            m_fields_vector.push_back(std::pair<ossimString, ossimString>(name, value));
            i++;
            break;
      };
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

ossimString ossimNitfGenericTag::get(ossimString fieldName)
{
   return m_fields_map[fieldName];
}

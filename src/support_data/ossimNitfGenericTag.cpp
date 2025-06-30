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
   char value[256];
   std::vector<std::pair<ossimString, ossim_int32>> result;
   std::vector<std::vector<ossim_int32> > suffix;
   std::vector<ossimString> spaceSubStrings, colonSubStrings;
   ossim_int32 length, i = 0;
   ossimString name;
   bool condition;

   while (i < NUM_DEFINITIONS)
   {
      if (i==118)
         std::cout <<"break";
      spaceSubStrings.clear();
      colonSubStrings.clear();
      switch (FIELD_DEFINITIONS[i].size)
      {
         //if start
         case -2:
            FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
            spaceSubStrings[0].split(colonSubStrings, ':');
            name = m_fields_map[colonSubStrings[0] + formatSuffix(suffix)];
            if (colonSubStrings.size() > 1 && name.length() > colonSubStrings[1].toInt())
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
            {
               int loopCount = 1;
               while (loopCount > 0)
               {
                  i++;
                  if (FIELD_DEFINITIONS[i].size == -5)
                     loopCount--;
                  else if (FIELD_DEFINITIONS[i].size == -4)
                     loopCount++;
               }
            }
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
            if (FIELD_DEFINITIONS[i].size == -1)
            {
               FIELD_DEFINITIONS[i].field.split(spaceSubStrings, ' ');
               name = spaceSubStrings[0] + formatSuffix(suffix);
               length = m_fields_map[spaceSubStrings[1] + formatSuffix(suffix)].toInt();
            }
            else
            {
               name = FIELD_DEFINITIONS[i].field + formatSuffix(suffix);
               length = FIELD_DEFINITIONS[i].size;
            }
            switch (actionFunction)
            {
               case 0:
                  in.read(value, length);
                  value[length] = '\0';
                  m_fields_map.insert(std::pair<ossimString, ossimString>(name, value));
                  //m_fields_vector.push_back(std::pair<ossimString, ossimString>(name, value));
                  break;
               case 1:
                  if (m_fields_map.count(name) == 0)
                     m_fields_map.insert(std::pair<ossimString, ossimString>(name, defaultFormatting("", i, length)));
                  break;
               case 2:
                  out.write(m_fields_map[name], length);
                  break;
               case 3:
                  out << std::setiosflags(std::ios::left)
                      << args << std::setw(24) << name << ":"
                      << m_fields_map[name] << "\n";
                  break;
            }
            i++;
            break;
      };
   }
}

void ossimNitfGenericTag::parseStream(std::istream &in)
{
   clearFields();

   m_fields_map.clear();

   readDefinitions(0, in, std::cout);
}

void ossimNitfGenericTag::writeStream(std::ostream &out)
{
   readDefinitions(2, std::cin, out);
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

   /*for (std::pair field: m_fields_vector)
   {
      out << std::setiosflags(std::ios::left)
            << pfx << std::setw(24) << field.first << ":"
            << field.second << "\n";
   }*/
   readDefinitions(3, std::cin, out, pfx);

   return out;
}

void ossimNitfGenericTag::clearFields()
{
   m_fields_map.clear();
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
   {
      if (FIELD_DEFINITIONS[i].size >= -1 &&
         FIELD_DEFINITIONS[i].field.length() >= name.length() &&
         name == FIELD_DEFINITIONS[i].field.substr(0, name.length()))
      {
         definition = i;
         break;
      }
   }
   value = defaultFormatting(value, definition, name);
   m_fields_map[name] = value;
   readDefinitions(1, std::cin, std::cout, name);
}

ossimString ossimNitfGenericTag::defaultFormatting(ossimString value, int definition, ossimString name)
{
   int length;
      if (FIELD_DEFINITIONS[definition].size == -1)
      {
         length = m_fields_map[name].toInt();
      }
      else
      {
         length = FIELD_DEFINITIONS[definition].size;
      }
   return defaultFormatting(value, definition, length);
}

ossimString ossimNitfGenericTag::defaultFormatting(ossimString value, int definition, int length)
{
   switch (FIELD_DEFINITIONS[definition].specs)
   {
      case ' ':
         while (value.length() < length)
            value = value + ' ';
         break;
      case '0':
         while (value.length() < length)
            value = '0' + value;
         break;
      default:
         throw ossimException("This should be unreachable");
   };
   return value;
}
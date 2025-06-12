//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSEXRB tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document STDI-0002-NCDRD Table M.6.1 for more info.
//
//----------------------------------------------------------------------------
// $Id
#include <ossim/support_data/ossimNitfCsexrbTag.h>

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>


ossimNitfCsexrbTag::ossimNitfCsexrbTag()
   : ossimNitfRegisteredTag(std::string("CSEXRB"), 353)
{
   clearFields();
   m_total_length = 0;
}

ossimString formatPrefix(std::vector<std::vector<ossim_int32>> prefixIn)
{
   ossimString result = "";
   char separator = 'n';
    for(std::vector<ossim_int32> set: prefixIn)
    {
       result += separator + ossimString(set[0]);
       separator ++;
    }
    return result;
}
void ossimNitfCsexrbTag::parseStream(std::istream& in)
{
   clearFields();

   //Curent itteration, Total iteration, i value
   std::vector<std::vector<ossim_int32>> prefix;
   std::vector<ossimString> spaceSubStrings, bracketSubStrings;
   ossim_int32 length, i = 0;
   ossimString name, prevName;
   char *value;
   bool condition;


   while(i < NUM_DEFINITIONS)
   {
      switch(FIELD_DEFINITIONS[i].second)
      {

         //variable length
         case -1:
            FIELD_DEFINITIONS[i].first.split(spaceSubStrings, ' ');
            name = spaceSubStrings[0] + formatPrefix(prefix);
            if(spaceSubStrings.size() == 1)
            {
               in.read(value, m_fields_vector.back().second.toInt());
               m_total_length += m_fields_vector.back().second.toInt();
            }
            else
            {
               length = m_fields_map[spaceSubStrings[1] + formatPrefix(prefix)].toInt();
               in.read(value, length);
               m_total_length += length;
            }
            m_fields_map.insert(std::pair<ossimString, ossimString>(name, value));
            m_fields_vector.push_back(std::pair<ossimString, ossimString>(name, value));
            i++;
            break;
         //if start
         case -2:
            FIELD_DEFINITIONS[i].first.split(spaceSubStrings, ' ');
            spaceSubStrings[0].split(bracketSubStrings, ':');
            name = bracketSubStrings[0];
            if(bracketSubStrings.size() > 1)
               condition = name.at(bracketSubStrings[1].toInt());
            if(spaceSubStrings[1] == "==")
               condition = (name == spaceSubStrings[2]);
            else if(spaceSubStrings[1] == "!=")
               condition = (name != spaceSubStrings[2]);
            if(condition)
               while(FIELD_DEFINITIONS[i].second != -3)
                  i++;
            i++;
            break;
         //if end
         case -3:
            break;
         //loop start
         case -4:
            FIELD_DEFINITIONS[i].first.split(spaceSubStrings, ' ');
            length = m_fields_map[spaceSubStrings[0] + formatPrefix(prefix)].toInt();
            if(length > 0)
               prefix.push_back({0, m_fields_map[spaceSubStrings[0]].toInt(), i + 1});
            else
               while(FIELD_DEFINITIONS[i].second != -5)
                  i++;
            i++;
            break;
         //loop end
         case -5:
            prefix.back()[0] ++;
            if(prefix.back()[0] < prefix.back()[1])
            {
               i = prefix.back()[3];
            }
            else
            {
               prefix.pop_back();
            }
            break;
         //length provided
         default:
            name = FIELD_DEFINITIONS[i].first + formatPrefix(prefix);
            in.read(value, FIELD_DEFINITIONS[i].second);
            m_total_length += FIELD_DEFINITIONS[i].second;
            m_fields_map.insert(std::pair<ossimString, ossimString>(name, value));
            m_fields_vector.push_back(std::pair<ossimString, ossimString>(name, value));
            i++;
            break;
      };
   }
}

void ossimNitfCsexrbTag::writeStream(std::ostream& out)
{
   clearFields();

   for(std::pair field : m_fields_vector)
   {
      out.write(field.second, field.second.length());
   }
}

std::ostream& ossimNitfCsexrbTag::print(std::ostream& out, const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += "CSEXRB";
   pfx += ".";

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:CSEXRB" << "\n"
       << pfx << std::setw(24) << "CEL:" << m_total_length << "\n";

   for(std::pair field : m_fields_vector)
   {
      out << std::setiosflags(std::ios::left)
          << pfx << std::setw(24) << field.first << field.second << "\n";
   }

   return out;
}

void ossimNitfCsexrbTag::clearFields()
{
   m_fields_map.clear();
   m_fields_vector.clear();
   m_total_length = 0;
}

ossimString ossimNitfCsexrbTag::get(ossimString fieldName)
{
   return m_fields_map[fieldName];
}

//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Scott Bortman
//
// Description: CSEPHA tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document STDI-0006-NCDRD Table  3.4-1 for more info.
// 
//----------------------------------------------------------------------------
// $Id

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>

#include <ossim/support_data/ossimNitfCsephaTag.h>

using namespace std;


RTTI_DEF1(ossimNitfCsephaTag, "ossimNitfCsephaTag", ossimNitfRegisteredTag);

ossimNitfCsephaTag::ossimNitfCsephaTag()
   : ossimNitfRegisteredTag(std::string("CSEPHA"), 0)
{
   clearFields();
}

ossimNitfCsephaTag::~ossimNitfCsephaTag()
{
}

void ossimNitfCsephaTag::parseStream(std::istream& in)
{
   clearFields();
   in.read(theEphemFlag, 12);
   in.read(theDtEphem, 5);
   in.read(theDateEphem, 8);
   in.read(theT0Ephem, 13);
   in.read(theNumEphem, 3);

   for ( int i = 0, n = atoi(theNumEphem); i < n; i++) {
      char ephem[13];

      in.read(ephem, 12);
      ephem[12] = '\0';
      theEphemX.push_back(ephem);

      in.read(ephem, 12);
      ephem[12] = '\0';
      theEphemY.push_back(ephem);

      in.read(ephem, 12);
      ephem[12] = '\0';
      theEphemZ.push_back(ephem);
   }
}

void ossimNitfCsephaTag::writeStream(std::ostream& out)
{
   out.write(theEphemFlag, 12);
   out.write(theDtEphem, 5);
   out.write(theDateEphem, 8);
   out.write(theT0Ephem, 13);
   out.write(theNumEphem, 3);

   for ( int i = 0, n = atoi(theNumEphem); i < n; i++) {
      out.write(theEphemX[i], 12);
      out.write(theEphemY[i], 12);
      out.write(theEphemZ[i], 12);
   } 
}

void ossimNitfCsephaTag::clearFields()
{
   //---
   // No attempt made to set to defaults.
   // BCS-N's to '0's, BCS-A's to ' '(spaces)
   //---

   memset(theEphemFlag, ' ',  12);
   memset(theDtEphem, 0, 5);
   memset(theDateEphem, 0, 8);
   memset(theT0Ephem, 0, 13);
   memset(theNumEphem, 0, 3);

   theEphemFlag[12] = '\0';
   theDtEphem[5] = '\0';
   theDateEphem[8] = '\0';
   theT0Ephem[13] = '\0';
   theNumEphem[3] = '\0';
}

std::ostream& ossimNitfCsephaTag::print(
   std::ostream& out, const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:"
       << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"
       << getTagLength() << "\n"
       << pfx << std::setw(24) << "EPHEM_FLAG:"
       << theEphemFlag << "\n"
       << pfx << std::setw(24) << "DT_EPHEM:"
       << theDtEphem << "\n"
       << pfx << std::setw(24) << "DATE_EPHEM:"
       << theDateEphem << "\n"
       << pfx << std::setw(24) << "T0_EPHEM:"
       << theT0Ephem << "\n"
       << pfx << std::setw(24) << "NUM_EPHEM:"
       << theNumEphem << "\n"
       << pfx << std::setw(24) << "EPHEM_X:"
       << vec2str(theEphemX) << "\n"
       << pfx << std::setw(24) << "EPHEM_Y:"
       << vec2str(theEphemY) << "\n"
       << pfx << std::setw(24) << "EPHEM_Z:"
       << vec2str(theEphemZ) << "\n"
       << "\n";
   
   return out;
}

ossimString ossimNitfCsephaTag::getEphemFlag() const
{
   return ossimString(theEphemFlag);
}
   
ossimString ossimNitfCsephaTag::getDtEphem() const
{
   return ossimString(theDtEphem);
}
   
ossimString ossimNitfCsephaTag::getDateEphem() const
{
   return ossimString(theDateEphem);
}

ossimString ossimNitfCsephaTag::getT0Ephem() const
{
   return ossimString(theT0Ephem);
}

ossimString ossimNitfCsephaTag::getNumEphem() const
{
   return ossimString(theNumEphem);
}

   
std::vector<ossimString> ossimNitfCsephaTag::getEphemX() const
{
   return theEphemX;
}
   
std::vector<ossimString> ossimNitfCsephaTag::getEphemY() const
{
   return theEphemY;
}
   
std::vector<ossimString> ossimNitfCsephaTag::getEphemZ() const
{
   return theEphemZ;
}

ossimString ossimNitfCsephaTag::vec2str(std::vector<ossimString> vec) const {
   ossimString buffer = "";

   if (!vec.empty())
   {
      for ( int i = 0, n = vec.size(); i < n; i++ ) {
         if ( i > 0 ) {
            buffer += " ";
         }

         buffer += vec[i];   
      }
   }

   return buffer;
}
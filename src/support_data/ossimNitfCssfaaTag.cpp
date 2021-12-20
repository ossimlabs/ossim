//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Scott Bortman
//
// Description: CSSFAA tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document STDI-0006-NCDRD Table 3.7-1 for more info.
// 
//----------------------------------------------------------------------------
// $Id

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>

#include <ossim/support_data/ossimNitfCssfaaTag.h>

using namespace std;


RTTI_DEF1(ossimNitfCssfaaTag, "ossimNitfCssfaaTag", ossimNitfRegisteredTag);

ossimNitfCssfaaTag::ossimNitfCssfaaTag()
   : ossimNitfRegisteredTag(std::string("CSSFAA"), 0)
{
   clearFields();
}

ossimNitfCssfaaTag::~ossimNitfCssfaaTag()
{
}

void ossimNitfCssfaaTag::parseStream(std::istream& in)
{
   clearFields();

   in.read(theNumBands, 1);

   for ( int i = 0, n = atoi(theNumBands); i < n; i++) {
      char bandType[2];
      char bandId[7];
      char focLength[12];
      char numDap[9];
      char numFir[9];
      char delta[8];
      char oppOffX[8];
      char oppOffY[8];
      char oppOffZ[8];
      char startX[12];
      char startY[12];
      char finishX[12];
      char finishY[12];

      in.read(bandType, 1);
      bandType[1] = '\0';
      theBandType.push_back(bandType);

      in.read(bandId, 6);
      bandId[6] = '\0';
      theBandId.push_back(bandId);

      in.read(focLength, 11);
      focLength[11] = '\0';
      theFocLength.push_back(focLength);

      in.read(numDap, 8);
      numDap[8] = '\0';
      theNumDap.push_back(numDap);

      in.read(numFir, 8);
      numFir[8] = '\0';
      theNumFir.push_back(numFir);

      in.read(delta, 7);
      delta[7] = '\0';
      theDelta.push_back(delta);

      in.read(oppOffX, 7);
      oppOffX[7] = '\0';
      theOppOffX.push_back(oppOffX);

      in.read(oppOffY, 7);
      oppOffY[7] = '\0';
      theOppOffY.push_back(oppOffY);

      in.read(oppOffZ, 7);
      oppOffZ[7] = '\0';
      theOppOffZ.push_back(oppOffZ);

      in.read(startX, 11);
      startX[11] = '\0';
      theStartX.push_back(startX);

      in.read(startY, 11);
      startY[11] = '\0';
      theStartY.push_back(startY);

      in.read(finishX, 11);
      finishX[11] = '\0';
      theFinishX.push_back(finishX);

      in.read(finishY, 11);
      finishY[11] = '\0';
      theFinishY.push_back(finishY);
   }
}

void ossimNitfCssfaaTag::writeStream(std::ostream& out)
{
   out.write(theNumBands, 1);

   for ( int i = 0, n = atoi(theNumBands); i < n; i++) {

      out.write(theBandType[i], 1);
      out.write(theBandId[i], 6);
      out.write(theFocLength[i], 11);
      out.write(theNumDap[i], 8);
      out.write(theNumFir[i], 8);
      out.write(theDelta[i], 7);
      out.write(theOppOffX[i], 7);
      out.write(theOppOffY[i], 7);
      out.write(theOppOffZ[i], 7);
      out.write(theStartX[i], 11);
      out.write(theStartY[i], 11);
      out.write(theFinishX[i], 11);
      out.write(theFinishY[i], 11);
   } 
}

void ossimNitfCssfaaTag::clearFields()
{
   //---
   // No attempt made to set to defaults.
   // BCS-N's to '0's, BCS-A's to ' '(spaces)
   //---

   memset(theNumBands, ' ',  1);
   theNumBands[1] = '\0';
}

std::ostream& ossimNitfCssfaaTag::print(
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
       << pfx << std::setw(24) << "NUM_BANDS:"
       << theNumBands << "\n"
       << pfx << std::setw(24) << "BAND_TYPE:"
       << vec2str(theBandType) << "\n"
       << pfx << std::setw(24) << "BAND_ID:"
       << vec2str(theBandId) << "\n"
       << pfx << std::setw(24) << "FOC_LENGTH:"
       << vec2str(theFocLength) << "\n"
       << pfx << std::setw(24) << "NUM_DAP:"
       << vec2str(theNumDap) << "\n"
       << pfx << std::setw(24) << "NUM_FIR:"
       << vec2str(theNumFir) << "\n"
       << pfx << std::setw(24) << "DELTA:"
       << vec2str(theDelta) << "\n"
       << pfx << std::setw(24) << "OPPOFF_X:"
       << vec2str(theOppOffX) << "\n"
       << pfx << std::setw(24) << "OPPOFF_Y:"
       << vec2str(theOppOffY) << "\n"
       << pfx << std::setw(24) << "OPPOFF_Z:"
       << vec2str(theOppOffZ) << "\n"
       << pfx << std::setw(24) << "START_X:"
       << vec2str(theStartX) << "\n"
       << pfx << std::setw(24) << "START_Y:"
       << vec2str(theStartY) << "\n"
       << pfx << std::setw(24) << "FINISH_X:"
       << vec2str(theFinishX) << "\n"
       << pfx << std::setw(24) << "FINISH_Y:"
       << vec2str(theFinishY) << "\n"
       << "\n";
   
   return out;
}

ossimString ossimNitfCssfaaTag::getNumBands() const
{
   return ossimString(theNumBands);
}
   
std::vector<ossimString> ossimNitfCssfaaTag::getBandType() const
{
   return theBandType;
}
   
std::vector<ossimString> ossimNitfCssfaaTag::getBandId() const
{
   return theBandId;
}

std::vector<ossimString> ossimNitfCssfaaTag::getFocLength() const
{
   return theFocLength;
}

std::vector<ossimString> ossimNitfCssfaaTag::getNumDap() const
{
   return theNumDap;
}

std::vector<ossimString> ossimNitfCssfaaTag::getNumFir() const
{
   return theNumFir;
}

std::vector<ossimString> ossimNitfCssfaaTag::getDelta() const
{
   return theDelta;
}

std::vector<ossimString> ossimNitfCssfaaTag::getOppOffX() const
{
   return theOppOffX;
}

std::vector<ossimString> ossimNitfCssfaaTag::getOppOffY() const
{
   return theOppOffY;
}

std::vector<ossimString> ossimNitfCssfaaTag::getOppOffZ() const
{
   return theOppOffZ;
}

std::vector<ossimString> ossimNitfCssfaaTag::getStartX() const
{
   return theStartX;
}

std::vector<ossimString> ossimNitfCssfaaTag::getStartY() const
{
   return theStartY;
}

std::vector<ossimString> ossimNitfCssfaaTag::getFinishX() const
{
   return theFinishX;
}

std::vector<ossimString> ossimNitfCssfaaTag::getFinishY() const
{
   return theFinishY;
}

ossimString ossimNitfCssfaaTag::vec2str(std::vector<ossimString> vec) const {
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

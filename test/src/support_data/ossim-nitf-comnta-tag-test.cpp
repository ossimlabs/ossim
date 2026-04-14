//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Regression test for variable-length COMNTA TRE parsing.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfComntaTag.h>
#include <ossim/support_data/ossimNitfTagInformation.h>

#include <iostream>
#include <sstream>
#include <string>

int main(int /*argc*/, char* /*argv*/[])
{
   const std::string comment(808, 'A');
   std::ostringstream encoded;
   encoded << "COMNTA";
   encoded << "00808";
   encoded << comment;

   std::istringstream in(encoded.str());
   ossimNitfTagInformation info;
   info.parseStream(in);

   if (info.getTagName() != "COMNTA")
   {
      std::cerr << "Unexpected tag name: " << info.getTagName() << "\n";
      return 1;
   }

   if (info.getTagLength() != comment.size())
   {
      std::cerr << "Unexpected CEL value: " << info.getTagLength() << "\n";
      return 1;
   }

   ossimRefPtr<ossimNitfRegisteredTag> tagData = info.getTagData();
   if (!tagData.valid())
   {
      std::cerr << "Missing COMNTA tag data\n";
      return 1;
   }

   ossimNitfComntaTag* comnta = dynamic_cast<ossimNitfComntaTag*>(tagData.get());
   if (!comnta)
   {
      std::cerr << "Parsed tag was not COMNTA\n";
      return 1;
   }

   if (comnta->getTagLength() != comment.size())
   {
      std::cerr << "Unexpected parsed tag length: " << comnta->getTagLength() << "\n";
      return 1;
   }

   if (comnta->getComment().string() != comment)
   {
      std::cerr << "Parsed comment payload mismatch\n";
      return 1;
   }

   return 0;
}

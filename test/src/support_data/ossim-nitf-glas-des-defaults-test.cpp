//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Regression test for the user-defined subheader defaults of the
// GLAS/GFM data extension segments (CSATTB, CSCSDB, CSEPHB, CSSFAB).
//
// STDI-0002 Vol 2 App M gives every GLAS/GFM DES the same subheader:
//
//     UUID (36)  NUMAIS (3, "ALL" or 001-998)  [AISDLVLn (3), only if numeric]
//     NUM_ASSOC_ELEM (3)  [ASSOC_ELEM_UUIDn (36) x NUM_ASSOC_ELEM]
//     RESERVEDSUBH_LEN (4, "0000" when nothing is reserved)
//
// The four classes disagreed, so a freshly constructed DES came out with:
//
//     CSATTB   NUMAIS blank
//     CSSFAB   NUMAIS "000" (not a legal value), RESERVEDSUBH_LEN blank
//     CSCSDB   NUM_ASSOC_ELEM "001" with a blank ASSOC_ELEM_UUID1
//     CSEPHB   NUM_ASSOC_ELEM "001" with a blank ASSOC_ELEM_UUID1
//
// A library cannot know an element UUID, so the only self-consistent default is
// to associate with every image segment (NUMAIS = ALL) and declare no
// associated elements (NUM_ASSOC_ELEM = 000); a caller that has a UUID sets it.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfCsattbDes.h>
#include <ossim/support_data/ossimNitfCscsdbDes.h>
#include <ossim/support_data/ossimNitfCsephbDes.h>
#include <ossim/support_data/ossimNitfCssfabDes.h>
#include <ossim/support_data/ossimNitfGenericDes.h>

#include <iostream>
#include <sstream>
#include <string>

namespace
{
   int failures = 0;

   void expect(const std::string& got, const std::string& want, const std::string& what)
   {
      if (got != want)
      {
         std::cerr << "FAIL: " << what << ": got '" << got << "' want '" << want << "'\n";
         ++failures;
      }
   }

   void check(ossimNitfGenericDes& des, const std::string& name)
   {
      expect(des.get("NUMAIS").string(), "ALL", name + " NUMAIS");
      expect(des.get("NUM_ASSOC_ELEM").string(), "000", name + " NUM_ASSOC_ELEM");
      expect(des.get("RESERVEDSUBH_LEN").string(), "0000", name + " RESERVEDSUBH_LEN");

      // And as written: UUID (36) "ALL" "000" "0000" is the whole subheader.
      std::ostringstream out;
      des.writeStream(out);
      const std::string bytes = out.str();
      expect(bytes.size() >= 46 ? bytes.substr(36, 10) : std::string("<short: ") +
                std::to_string(bytes.size()) + ">",
             "ALL0000000", name + " subheader bytes after the UUID");
      expect(std::to_string(des.getDesSubHeaderLength()), "46", name + " DESSHL");
   }
}

int main(int /*argc*/, char* /*argv*/[])
{
   ossimNitfCsattbDes csattb;
   ossimNitfCscsdbDes cscsdb;
   ossimNitfCsephbDes csephb;
   ossimNitfCssfabDes cssfab;

   check(csattb, "CSATTB");
   check(cscsdb, "CSCSDB");
   check(csephb, "CSEPHB");
   check(cssfab, "CSSFAB");

   if (failures)
   {
      std::cerr << failures << " failure(s)\n";
      return 1;
   }
   std::cout << "ossim-nitf-glas-des-defaults-test: PASS\n";
   return 0;
}

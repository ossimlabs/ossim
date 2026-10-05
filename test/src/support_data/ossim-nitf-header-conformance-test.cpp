//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Regression test for two NITF file/image header fields the
// writers got wrong.
//
// 1. CLEVEL.  The Kakadu NITF writer set it from the IMAGE data length rather
//    than the file length, using a "gigabyte" of 1000 MiB, and never looked at
//    the image's rows and columns.  JBP Table G-1 (still imagery) marks a file
//    no lower than the highest feature it exceeds:
//
//        CLEVEL    image rows or columns    file size (bytes)
//        03        <= 2048                  <= 52,428,799
//        05        <= 8192                  <= 1,073,741,823
//        06        <= 65536                 <= 2,147,483,647
//        07        <= 99,999,999            <= 10,737,418,239
//        09        beyond CLEVEL 07
//
//    A 92,398 row, 1.9 GB image was written as CLEVEL 06 and needs 07.
//
// 2. IXSOFL.  ossimNitfImageHeaderV2_1::writeStream() set the extended
//    subheader overflow field to "001" whenever the subheader held ANY TRE, so
//    it pointed at DES number 1 whether or not that was a TRE_OVERFLOW.  It is
//    "000" unless TREs really overflowed into a DES.
//
//----------------------------------------------------------------------------

#include <ossim/imaging/ossimNitfWriterBase.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_1.h>
#include <ossim/support_data/ossimNitfTagInformation.h>

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

   std::string clevel(ossim_uint64 width, ossim_uint64 height, ossim_uint64 bytes)
   {
      return ossimNitfWriterBase::getComplexityLevel(width, height, bytes).string();
   }
}

int main(int /*argc*/, char* /*argv*/[])
{
   // 1. CLEVEL against JBP Table G-1.  The first rows are delivered products.
   expect(clevel(42500, 92398, 1915066497ULL), "07", "92398 rows, 1.9 GB");
   expect(clevel(42500, 68464, 1172621812ULL), "07", "68464 rows, 1.17 GB");
   expect(clevel(29847, 30628, 338462939ULL),  "06", "30628 rows, 338 MB");
   expect(clevel(10651, 17119, 244833240ULL),  "06", "17119 rows, 245 MB");
   expect(clevel(7458, 7911, 73962606ULL),     "05", "7911 rows, 74 MB");
   expect(clevel(512, 512, 164413ULL),         "03", "512 x 512 chip");

   // The size limits themselves.
   expect(clevel(2048, 2048, 1000),    "03", "2048 is still CLEVEL 03");
   expect(clevel(2049, 100, 1000),     "05", "2049 needs CLEVEL 05");
   expect(clevel(100, 8192, 1000),     "05", "8192 is still CLEVEL 05");
   expect(clevel(100, 8193, 1000),     "06", "8193 needs CLEVEL 06");
   expect(clevel(65536, 100, 1000),    "06", "65536 is still CLEVEL 06");
   expect(clevel(65537, 100, 1000),    "07", "65537 needs CLEVEL 07");

   // File size alone, with a small image.
   expect(clevel(1000, 1000, 52428799ULL),    "03", "50 MiB - 1 byte");
   expect(clevel(1000, 1000, 52428800ULL),    "05", "50 MiB");
   expect(clevel(1000, 1000, 1073741823ULL),  "05", "1 GiB - 1 byte");
   expect(clevel(1000, 1000, 1073741824ULL),  "06", "1 GiB");
   expect(clevel(1000, 1000, 2147483647ULL),  "06", "2 GiB - 1 byte");
   expect(clevel(1000, 1000, 2147483648ULL),  "07", "2 GiB");
   expect(clevel(1000, 1000, 10737418239ULL), "07", "10 GiB - 1 byte");
   expect(clevel(1000, 1000, 10737418240ULL), "09", "10 GiB");

   // A "gigabyte" is 2^30.  The old code used 1000 MiB, so a 1,050,000,000 byte
   // file (under 1 GiB) was marked 06.
   expect(clevel(1000, 1000, 1050000000ULL), "05", "1,050,000,000 bytes is under 1 GiB");

   // 2. IXSOFL is "000" when nothing overflowed.  The three bytes just before
   //    the first TRE's tag name ARE the field: IXSHDL(5) IXSOFL(3) TREs...
   const std::string comment(40, 'A');
   std::istringstream tre("COMNTA00040" + comment);
   ossimNitfTagInformation info;
   info.parseStream(tre);

   ossimNitfImageHeaderV2_1 header;
   header.addTag(info);

   std::ostringstream out;
   header.writeStream(out);
   const std::string bytes = out.str();

   const std::string::size_type at = bytes.find("COMNTA");
   if (at == std::string::npos || at < 8)
   {
      std::cerr << "FAIL: the written image header does not contain the COMNTA tag\n";
      ++failures;
   }
   else
   {
      expect(bytes.substr(at - 3, 3), "000", "IXSOFL with a TRE that did not overflow");
      expect(bytes.substr(at - 8, 5), "00054", "IXSHDL is IXSOFL (3) + COMNTA (6 + 5 + 40)");
   }

   if (failures)
   {
      std::cerr << failures << " failure(s)\n";
      return 1;
   }
   std::cout << "ossim-nitf-header-conformance-test: PASS\n";
   return 0;
}

//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Regression test for setFileHeaderProperty() /
// setImageHeaderProperty() reaching the written NITF.
//
// Both setters push onto vectors whose only consumers are
// addFileHeaderProperties() / addImageHeaderProperties().  Between
// a694e18f (2025-06-13) and this test, NOTHING called either method -- the
// image-header call site was disabled with "#if 0 /* Moved to
// addImageHeaderProperties */" and the new methods were never wired to a
// caller, and the file-header half never had a call site at all.  So the
// public API silently discarded everything set through it: FSCLAS came out a
// space no matter what the caller asked for.
//
// This writes a small NITF with FSCLAS/ISCLAS set through that API and reads
// the bytes back out of the file.  It fails on the unfixed code.
//
//----------------------------------------------------------------------------

#include <ossim/base/ossimFilename.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimMemoryImageSource.h>
#include <ossim/imaging/ossimNitfWriter.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
   int failures = 0;

   void check(bool ok, const std::string& what)
   {
      std::cout << (ok ? "  ok   " : "  FAIL ") << what << "\n";
      if (!ok)
      {
         ++failures;
      }
   }

   // FSCLAS is a single byte at offset 119 of a NITF 2.1 file header:
   // FHDR(4) FVER(5) CLEVEL(2) STYPE(4) OSTAID(10) FDT(14) FTITLE(80) = 119.
   const std::streamoff FSCLAS_OFFSET = 119;

   std::string byteAt(const ossimFilename& file, std::streamoff offset)
   {
      std::ifstream in(file.c_str(), std::ios::binary);
      if (!in)
      {
         return std::string();
      }
      in.seekg(offset);
      char c = '\0';
      in.get(c);
      return in ? std::string(1, c) : std::string();
   }
}

int main(int argc, char* argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   const ossimFilename output("ossim-nitf-writer-property-test.ntf");
   output.remove();

   // A small, uniform source image is enough; this test is about the header.
   ossimIrect rect(0, 0, 63, 63);
   ossimRefPtr<ossimImageData> tile =
      ossimImageDataFactory::instance()->create(0, OSSIM_UINT8, 1, 64, 64);
   tile->initialize();
   tile->fill(128.0);
   tile->setImageRectangle(rect);

   ossimRefPtr<ossimMemoryImageSource> source = new ossimMemoryImageSource();
   source->setImage(tile);

   ossimRefPtr<ossimNitfWriter> writer = new ossimNitfWriter();
   writer->connectMyInputTo(0, source.get());
   writer->setFilename(output);

   // THE API UNDER TEST.
   writer->setFileHeaderProperty(
      new ossimStringProperty(ossimString("FSCLAS"), ossimString("U")));
   writer->setImageHeaderProperty(
      new ossimStringProperty(ossimString("ISCLAS"), ossimString("U")));

   const bool wrote = writer->execute();
   check(wrote, "the writer produced a file");
   if (!wrote || !output.exists())
   {
      std::cout << "\nFAILED\n";
      return 1;
   }

   const std::string fsclas = byteAt(output, FSCLAS_OFFSET);
   check(fsclas == "U",
         "FSCLAS set through setFileHeaderProperty() reaches the file: got '" +
         fsclas + "', want 'U'");

   output.remove();
   std::cout << "\n" << (failures ? "FAILED" : "PASSED") << "\n";
   return failures ? 1 : 0;
}

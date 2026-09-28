//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Regression test for NITF per-band representation (IREPBANDnn)
// and subcategory (ISUBCATnn).
//
// The NITF writers used to write the band INDEX ("00", "01", ...) as
// IREPBANDnn, which is not a legal value, so a reader looking for the colour
// bands (ossimNitfTileSource::getRgbBandList(), GDAL ColorInterp) found none
// and displayed bands 1,2,3 as R,G,B. A caller could not correct it either:
// ossimNitfImageHeaderV2_X::setProperty() matched "IREPBAND001" as the
// image-level IREP and overwrote it.
//
//----------------------------------------------------------------------------

#include <ossim/base/ossimStringProperty.h>
#include <ossim/support_data/ossimNitfImageBandV2_1.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_1.h>

#include <iostream>
#include <string>

namespace
{
   int failures = 0;

   void expect(const ossimString& got, const std::string& want, const std::string& what)
   {
      if (got.string() != want)
      {
         std::cerr << "FAIL: " << what << ": got '" << got << "' want '" << want << "'\n";
         ++failures;
      }
   }

   ossimString rep(const ossimNitfImageHeaderV2_1& h, ossim_uint32 i)
   {
      return h.getBandInformation(i)->getBandRepresentation();
   }

   ossimString subcat(const ossimNitfImageHeaderV2_1& h, ossim_uint32 i)
   {
      const ossimNitfImageBandV2_0* b =
         dynamic_cast<const ossimNitfImageBandV2_0*>(h.getBandInformation(i).get());
      return b ? b->getBandSignificance() : ossimString("<none>");
   }
}

int main(int /*argc*/, char* /*argv*/[])
{
   typedef ossimNitfImageHeaderV2_X H;

   // 1. Writer defaults: legal values, never the index.
   expect(H::defaultBandRepresentation("RGB", 0), "R", "RGB band 0");
   expect(H::defaultBandRepresentation("RGB", 1), "G", "RGB band 1");
   expect(H::defaultBandRepresentation("RGB", 2), "B", "RGB band 2");
   expect(H::defaultBandRepresentation("MONO", 0), "M", "MONO band 0");
   expect(H::defaultBandRepresentation("MULTI", 0), "  ", "MULTI band 0 is blank");
   expect(H::defaultBandRepresentation("MULTI", 3), "  ", "MULTI band 3 is blank");
   expect(H::defaultBandRepresentation("rgb ", 2), "B", "IREP is trimmed and case-insensitive");

   // 2. Per-band caller overrides, the way a writer applies them after its
   //    band loop (ossimNitfWriterBase::addImageHeaderProperties).
   ossimNitfImageHeaderV2_1 hdr;
   hdr.setNumberOfBands(4);
   ossimNitfImageBandV2_1 blank;
   for (ossim_uint32 i = 0; i < 4; ++i)
   {
      blank.setBandRepresentation(H::defaultBandRepresentation("MULTI", i));
      hdr.setBandInfo(i, blank);
   }
   hdr.setRepresentation("MULTI");

   const char* const reps[] = { "B", "G", "R", "N" };
   const char* const nms[]  = { "480", "545", "660", "835" };
   for (int i = 0; i < 4; ++i)
   {
      char k[32];
      std::snprintf(k, sizeof(k), "IREPBAND%03d", i + 1);
      hdr.setProperty(new ossimStringProperty(k, reps[i]));
      std::snprintf(k, sizeof(k), "ISUBCAT%03d", i + 1);
      hdr.setProperty(new ossimStringProperty(k, nms[i]));
   }
   expect(rep(hdr, 0), "B ", "IREPBAND001 set");
   expect(rep(hdr, 3), "N ", "IREPBAND004 set");
   expect(subcat(hdr, 0), "   480", "ISUBCAT001 set (right-justified)");
   expect(subcat(hdr, 3), "   835", "ISUBCAT004 set");
   expect(hdr.getRepresentation(), "MULTI   ",
          "IREPBAND properties do not overwrite the image-level IREP");

   // Prefixed and unpadded names resolve to the same band.
   hdr.setProperty(new ossimStringProperty("nitf.image0.IREPBAND2", "R"));
   expect(rep(hdr, 1), "R ", "prefixed, unpadded IREPBAND2");

   // Out-of-range and malformed names change nothing and do not touch IREP.
   hdr.setProperty(new ossimStringProperty("IREPBAND009", "X"));
   hdr.setProperty(new ossimStringProperty("IREPBAND000", "X"));
   hdr.setProperty(new ossimStringProperty("IREPBANDXYZ", "X"));
   expect(hdr.getRepresentation(), "MULTI   ", "bad per-band names never reach IREP");

   // A plain IREP property still sets the image-level field.
   hdr.setProperty(new ossimStringProperty("IREP", "MONO"));
   expect(hdr.getRepresentation(), "MONO    ", "IREP property still works");

   if (failures)
   {
      std::cerr << failures << " failure(s)\n";
      return 1;
   }
   std::cout << "ossim-nitf-irepband-test: PASS\n";
   return 0;
}

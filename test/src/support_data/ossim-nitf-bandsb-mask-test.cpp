//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Regression test for BANDSB honoring its EXISTENCE_MASK.
//
// STDI-0002 Vol 1 App X Table X.6-1 makes every BANDSB field below the mask conditional on a bit of
// EXISTENCE_MASK (b24 CWAVE, b23 FWHM, b22 FWHM_UNC, b21 NOM_WAVE, b19 LBOUND and UBOUND, ...), and WAVE_LENGTH_UNIT present
// when any wavelength bit is set. The tag used to be a fixed layout (CWAVE, FWHM and FWHM_UNC always, nothing else), so a file
// with another mask was read by position: NOM_WAVE came out labeled CWAVE, LBOUND as FWHM, UBOUND as FWHM_UNC, and a layout of
// another length drew "CEL field length does not match fixed tag length". The tag now follows the mask in the data.
//
// Each case builds the data of a 2-band BANDSB for one mask, reads it, checks that the values land in the fields the mask names
// (band 2 as well, which only works if band 1 consumed exactly the right bytes), and writes it back expecting the same bytes.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfBandsbTag.h>

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

   std::string pad(const std::string& s, std::size_t n) { return (s + std::string(n, ' ')).substr(0, n); }

   // The fixed part of a BANDSB TRE for a 2-band cube: everything up to and including EXISTENCE_MASK.
   std::string header(std::uint32_t mask)
   {
      std::string h = "00002";                                         // COUNT
      h += pad("UNCALIBRATED", 24) + "D";                             // RADIOMETRIC_QUANTITY, _UNIT
      h += std::string{char(0x3F), char(0x80), char(0), char(0)};      // SCALE_FACTOR 1.0 (IEEE binary32)
      h += std::string(4, '\0');                                       // ADDITIVE_FACTOR 0.0
      h += "000.138M000.138M000.000 000.000 ";                         // ROW_GSD, COL_GSD, SPT_RESP_ROW, SPT_RESP_COL with units
      h += std::string(48, ' ');                                       // DATA_FLD_1
      h += std::string{char(mask >> 24), char(mask >> 16), char(mask >> 8), char(mask)};
      return h;
   }

   struct Band { std::string id, bad, a, b, c; };

   // Two bands of BANDID, BAD_BAND and up to three 7-character fields (a, b, c: whichever the mask switches on).
   std::string bands(const std::vector<Band>& bs, bool threeFields)
   {
      std::string out;
      for (const Band& b : bs)
         out += pad(b.id, 50) + b.bad + (threeFields ? b.a + b.b + b.c : "");
      return out;
   }

   // Read `data` as a BANDSB, check what `check` expects, then write it back and compare byte for byte.
   template <class F>
   void roundTrip(const std::string& data, const std::string& name, F check)
   {
      ossimNitfBandsbTag tag(static_cast<ossim_uint32>(data.size()));
      std::istringstream in(data);
      tag.parseStream(in);
      check(tag);
      std::ostringstream out;
      tag.writeStream(out);
      expect(std::to_string(out.str().size()), std::to_string(data.size()), name + " written length");
      expect(out.str() == data ? "same" : "different", "same", name + " written bytes");
   }
}

int main(int /*argc*/, char* /*argv*/[])
{
   const std::vector<Band> two = {{"BLUE", "1", "0.48254", "0.45106", "0.51026"},
                                  {"NIR1", "1", "0.82454", "0.77093", "0.89104"}};

   // 1. The layout the converter wrote until now: b28 BANDID, b27 BAD_BAND, b24 CWAVE, b23 FWHM, b22 FWHM_UNC.
   roundTrip(header(0x19C00000) + "U" + bands(two, true), "legacy mask", [](ossimNitfBandsbTag& t) {
      expect(t.get("WAVE_LENGTH_UNIT").string(), "U", "legacy WAVE_LENGTH_UNIT");
      expect(t.get("BANDID2").string().substr(0, 4), "NIR1", "legacy BANDID2");
      expect(t.get("CWAVE1").string(), "0.48254", "legacy CWAVE1");
      expect(t.get("FWHM2").string(), "0.77093", "legacy FWHM2");
      expect(t.get("FWHM_UNC2").string(), "0.89104", "legacy FWHM_UNC2");
   });

   // 2. The same three 7-character slots under another mask: b21 NOM_WAVE and b19 LBOUND/UBOUND instead of b24..b22.
   roundTrip(header(0x18280000) + "U" + bands(two, true), "NOM_WAVE/LBOUND/UBOUND mask", [](ossimNitfBandsbTag& t) {
      expect(t.get("NOM_WAVE1").string(), "0.48254", "NOM_WAVE1");
      expect(t.get("LBOUND1").string(), "0.45106", "LBOUND1");
      expect(t.get("UBOUND1").string(), "0.51026", "UBOUND1");
      expect(t.get("NOM_WAVE2").string(), "0.82454", "NOM_WAVE2");
      expect(t.get("UBOUND2").string(), "0.89104", "UBOUND2");
      expect(t.get("CWAVE1").string(), "", "CWAVE1 is not in this layout");
      expect(t.get("FWHM1").string(), "", "FWHM1 is not in this layout");
   });

   // 3. No wavelength bit at all: WAVE_LENGTH_UNIT is absent (GDAL's condition), so BANDID starts right after the mask.
   roundTrip(header(0x18000000) + bands(two, false), "no-wavelength mask", [](ossimNitfBandsbTag& t) {
      expect(t.get("BANDID1").string().substr(0, 4), "BLUE", "BANDID1 without WAVE_LENGTH_UNIT");
      expect(t.get("BAD_BAND2").string(), "1", "BAD_BAND2 without WAVE_LENGTH_UNIT");
      expect(t.get("WAVE_LENGTH_UNIT").string(), "", "WAVE_LENGTH_UNIT absent");
   });

   // 4. A tag nobody touches keeps its default mask, so existing writers produce what they always did.
   ossimNitfBandsbTag fresh;
   expect(fresh.get("EXISTENCE_MASK").string(), std::string{char(0x9F), char(0xC7), char(0x10), char(0x00)}, "default EXISTENCE_MASK");

   if (failures)
   {
      std::cerr << failures << " check(s) failed\n";
      return 1;
   }
   std::cout << "ossim-nitf-bandsb-mask-test: all checks passed\n";
   return 0;
}

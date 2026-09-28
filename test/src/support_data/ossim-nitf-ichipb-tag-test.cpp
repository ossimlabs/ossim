//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Regression test for reading an ICHIPB tag back as a transform.
//
// ICHIPB stores its grid points "pixel is area" (STDI-0002 Vol 1 App B: the
// centre of a pixel is at .5), and ossimNitfIchipbTag::initialize() writes
// them that way from OSSIM's "pixel is point" rectangles. newTransform() must
// undo that before fitting, or the chip-to-full-image transform it returns is
// off by 0.5 * (1 - scale) pixels whenever the chip is not 1:1 -- the
// half-pixel only cancels for a pure translation.
//
//----------------------------------------------------------------------------

#include <ossim/base/ossim2dTo2dTransform.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/support_data/ossimNitfIchipbTag.h>

#include <cmath>
#include <iostream>

namespace
{
   int failures = 0;

   void checkScale(double scale)
   {
      // A W x H chip whose pixel (0,0) is full-image pixel (1000, 2000), with
      // each chip pixel covering `scale` full-image pixels.
      const double W = 100.0, H = 50.0;
      const ossimDpt ul(1000.0, 2000.0);
      const ossimDrect opRect(0.0, 0.0, W - 1.0, H - 1.0);
      const ossimDrect fiRect(ul.x, ul.y,
                              ul.x + scale * (W - 1.0), ul.y + scale * (H - 1.0));

      ossimNitfIchipbTag tag;
      if (!tag.initialize(opRect, fiRect))
      {
         std::cerr << "scale " << scale << ": initialize() refused the rects\n";
         ++failures;
         return;
      }

      ossimRefPtr<ossim2dTo2dTransform> xform = tag.newTransform();
      const ossimDpt pts[] = { ossimDpt(0.0, 0.0), ossimDpt(10.0, 20.0),
                               ossimDpt(W - 1.0, H - 1.0), ossimDpt(37.5, 12.25) };
      for (const ossimDpt& p : pts)
      {
         ossimDpt got;
         xform->forward(p, got);
         const ossimDpt want(ul.x + scale * p.x, ul.y + scale * p.y);
         if (std::fabs(got.x - want.x) > 1e-6 || std::fabs(got.y - want.y) > 1e-6)
         {
            std::cerr << "scale " << scale << ": chip " << p << " -> " << got
                      << ", want " << want << "\n";
            ++failures;
         }
      }
   }
}

int main(int /*argc*/, char* /*argv*/[])
{
   checkScale(1.0);   // 1:1 -- passed before the fix too (the shift cancels)
   checkScale(2.0);   // reduced resolution
   checkScale(0.5);   // enlarged
   if (failures)
   {
      std::cerr << failures << " failure(s)\n";
      return 1;
   }
   std::cout << "ossim-nitf-ichipb-tag-test: PASS\n";
   return 0;
}

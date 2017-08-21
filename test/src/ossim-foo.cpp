//---
//
// License: MIT
//
// File: ossim-foo.cpp
//
// Description: Contains application definition "ossim-foo" app.
//
// NOTE:  This is supplied for simple quick test. DO NOT checkin your test to
//        the svn repository.  Simply edit ossim-foo.cpp and run your test.
//        After completion you can do a "git checkout -- ossimfoo.cpp" if
//        you want to keep your working repository up to snuff.
//
// $Id$
//---

// ossim includes:  These are here just to save time/typing...
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimCsvFile.h>
#include <ossim/base/ossimDate.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimObjectFactory.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimUrl.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/base/ossimEcefPoint.h>
#include <ossim/base/ossimEcefVector.h>
#include <ossim/base/ossim2dBilinearTransform.h>

#include <ossim/imaging/ossimNitfTileSource.h>
#include <ossim/imaging/ossimBrightnessContrastSource.h>
#include <ossim/imaging/ossimBumpShadeTileSource.h>
#include <ossim/imaging/ossimFilterResampler.h>
#include <ossim/imaging/ossimFusionCombiner.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageMosaic.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/imaging/ossimImageToPlaneNormalFilter.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/imaging/ossimRectangleCutFilter.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimSFIMFusion.h>
#include <ossim/imaging/ossimTwoColorView.h>
#include <ossim/imaging/ossimImageSourceFactoryRegistry.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>

#include <ossim/init/ossimInit.h>

#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimUtmProjection.h>

#include <ossim/support_data/ossimSrcRecord.h>
#include <ossim/support_data/ossimWkt.h>

// Put your includes here:

// System includes:
#include <cmath>
#include <memory>
#include <sstream>
#include <iostream>
using namespace std;

/*
void getOverlappingPatchCenters(std::vector<ossimGpt>& patchCenters,
                           ossimRefPtr<ossimImageGeometry> geom1,
                           ossimRefPtr<ossimImageGeometry> geom2,
                           ossim_uint32 size=256)
{
   ossimDrect rect1;
   ossimDrect rect2;
   geom1->getBoundingRect(rect1);
   geom2->getBoundingRect(rect2);
   ossim_uint32 halfSize = size>>1;
   ossimDpt halfPoint(halfSize, halfSize);
   ossimIpt originTop    = rect1.ul() + halfPoint;
   ossimIpt originBottom = rect1.lr() - halfPoint;
   ossimIpt iteratePoint = originTop;
   ossimIpt destTop    = rect2.ul() + halfPoint;
   ossimIpt destBottom = rect2.lr() - halfPoint;
   ossimDrect originRect(originTop, originBottom);
   ossimDrect testRect(destTop, destBottom);

   if(originRect.completely_within(rect1))
   {
      for(ossim_uint32 y = originTop.y; y < originBottom.y;y+=size)
      {
         for(ossim_uint32 x = originTop.x; x < originBottom.x;x+=size)
         {
            ossimDpt destImagePt;
            ossimGpt gpt;
            geom1->localToWorld(ossimDpt(x,y), gpt);
            geom2->worldToLocal(gpt, destImagePt);
            ossimDpt ul = destImagePt - halfPoint;
            ossimDpt lr = ul + ossimDpt(size-1, size-1);
            ossimDrect patchRect(ul, lr);
            if(patchRect.completely_within(testRect))
            {
               patchCenters.push_back(gpt);   
            }
         }
      }
   }
   else
   {
      std::cout << "Patch size is too big for it exceeds the bounds of the image\n";
   }

}
*/
int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   try
   {
      ossimKeywordlist kwl;
      ossimRefPtr<ossimImageHandler> ih = ossimImageHandlerRegistry::instance()->open(ossimFilename(argv[1]));

      ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
      if(geom.valid())
      {  
         ossimDrect rect;
         ossimDpt dpt;
         geom->getBoundingRect(rect);
         dpt = rect.midPoint();
         std::cout << rect << "\n";

         std::cout << "UP IS UP " << geom->upIsUpAngle(dpt) << "\n";
      
         ossimGpt gpt;
         geom->localToWorld(dpt, gpt);
         ossimEcefPoint ecefPoint(gpt);
         
         ossimGpt gpt2 = gpt;
         gpt2.height(gpt2.height()+10);

         ossimEcefPoint ecefPoint2(gpt2);

         std::cout << "ecef: " << ecefPoint << "\n";
         std::cout << "ecef2: " << ecefPoint2 << "\n";
         ossimEcefVector v(ecefPoint2-ecefPoint);
         std::cout << "ecef v: " << v << "\n";
         std::cout << "LENGTH: " << v.normalize() << "\n";
         std::cout << "NORM: " << v << "\n";

      }
   

      /*
      if(argc > 2)
      {
         ossim_uint32 patchSize = 512;
         ossimRefPtr<ossimImageHandler> ih1 = ossimImageHandlerRegistry::instance()->open(ossimFilename(argv[1]));
         ossimRefPtr<ossimImageHandler> ih2 = ossimImageHandlerRegistry::instance()->open(ossimFilename(argv[2]));
         if(argc > 3)
         {
            patchSize = ossimString(argv[3]).toUInt32();
         }
         if(ih1.valid()&&ih2.valid())
         {
            std::vector<ossimGpt> points;
            ossimPolyArea2d poly1;
            ossimPolyArea2d poly2;
            ossimRefPtr<ossimImageGeometry> geom1 = ih1->getImageGeometry();
            ossimRefPtr<ossimImageGeometry> geom2 = ih2->getImageGeometry();

            getOverlappingPatchCenters(points, geom1, geom2, patchSize);

            ossim_uint32 idx = 0;
            for(idx=0;idx < points.size(); ++idx)
            {

               std::cout << points[idx].latd() <<  " " << points[idx].lond() << "\n";
            }
         }
      }
      else
      {
         std::cout << "Arguments must be <image 1> <image 2> <tile_size>\n";
         std::cout << "  where tile_size is optional\n";
      }
*/
      // Put your code here.
   }
   catch(const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      returnCode = 1;
   }
   catch( ... )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossim-foo caught unhandled exception!" << std::endl;
      returnCode = 1;
   }
   
   return returnCode;
}

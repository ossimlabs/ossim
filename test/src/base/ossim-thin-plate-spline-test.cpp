#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/base/ossimCommon.h>  // ossim contants...
#include <ossim/base/ossimRefPtr.h>  // ossim contants...
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimDpt.h>
#include <iostream>
#include <iterator>

#include <ossim/base/ossimThinPlateSpline.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);
   ossimThinPlateSpline spline(2);
   ossimThinPlateSpline splineinv(2);
   ossimRefPtr<ossimImageHandler> ih = ossimImageHandlerRegistry::instance()->open(ossimFilename(argv[1]));
   
   if(argc <2)
   {
      std::cout << "Please pass an image to fit a polynomial" << std::endl;
   }
   if(!ih.valid())
   {
      std::cout << "The input image is not a valid handler: " << argv[1] << std::endl;
      return 0;
   }
   ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
   double pt[3];
   double ptinv[3];
   
   ossimDrect rect(ih->getBoundingRect());
   ossim_int32 x = 0;
   ossim_int32 y = 0;
   ossimGpt gpt;
   pt[0] = -180;
   pt[1] = 90;
   for(y = 0; y <  10; ++y)
   {
      for(x = 0; x < 10; ++x)
      {
         double localx = x*.1 * rect.width();
         double localy = y*.1 * rect.height();
         geom->localToWorld(ossimDpt(localx, localy), gpt);
         pt[0] = gpt.latd();
         pt[1] = gpt.lond();
         ptinv[0] = localx;
         ptinv[1] = localy;
         
         spline.addPoint(localx,localy,pt);
         splineinv.addPoint(gpt.latd(),gpt.lond(),ptinv);
      }
   }
   
   spline.solve();
   splineinv.solve();
   for(y = -10; y <  10; ++y)
   {
      for(x = -10; x < 10; ++x)
      {
         double localx = x*.1 * rect.width();
         double localy = y*.1 * rect.height();
         spline.getPoint(localx, localy, pt);
         geom->localToWorld(ossimDpt(localx, localy), gpt);
         
         std::cout << "WORLD POINT = " << gpt << std::endl;
         std::cout << "LOCAL POINT = " << localx << ", " << localy << std::endl;
         std::cout << "fit point   = " << pt[0] << ", " << pt[1] << std::endl;
         std::cout << "model point = " << gpt.latd() << ", " << gpt.lond() << std::endl;
         splineinv.getPoint(gpt.latd(), gpt.lond(), pt);
         std::cout << "INVERSE = " << pt[0] << ", " << pt[1] << std::endl; 
      }
   }
   return 0;
}

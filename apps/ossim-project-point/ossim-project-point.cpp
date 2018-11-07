//*******************************************************************
// OSSIM
//
// License:  See top level LICENSE.txt file.
//
// Simple point projector from image to map and visa versa
//
//*******************************************************************
#include <iostream>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimUtmProjection.h>
#include <ossim/base/ossimGrect.h>

using namespace std;

static void usage()
{
   cout  << "\nSimple point projector that does image->ground->map->ground->image. \n"
         << "An output map projection (UTM or geographic) is created at the nominal \n"
         << "GSD of the image. Usage:\n"
         << "\n"
         << "  ossim-project [options] <image_file> [<x> <y>]\n"
         << "\n"
         << "Options:\n"
         << "  -h         Displays this. \n"
         << "  -g         Use geographic projection. Default is UTM. \n"
         << "\n"
         << "If no coordinates are provided, the image center is used.\n"
         << endl;
   exit(0);
}

int main(int argc, char *argv[])
{
	ossimInit::instance()->initialize(argc, argv);

   vector<ossimString> cmdline;
   for (uint32_t i=1; i<argc; ++i)
      cmdline.emplace_back(argv[i]);

   bool doInverse = false;
   bool doGeographic = false;
   ossimFilename fname;
   double x = ossim::nan();
   double y = ossim::nan();
   for (auto &s : cmdline)
   {
      if (s == "-h")
         usage();

      if (s == "-g")
         doGeographic = true;
      else if (fname.empty())
         fname = s;
      else if (ossim::isnan(x))
         x = s.toDouble();
      else if (ossim::isnan(y))
         y = s.toDouble();
      else
      {
         cout << "\nError parsing command line!" << endl;
         exit(1);
      }
   }
   if (fname.empty())
      usage();

   ossimRefPtr<ossimImageHandler> handler = ossimImageHandlerRegistry::instance()->open(fname);
   if (!handler)
   {
      cout << "\nNull image handler returned for input file <"<<fname<<">!" << endl;
      exit(1);
   }
   ossimRefPtr<ossimImageGeometry> geom = handler->getImageGeometry();
   if (!handler)
   {
      cout << "\nNull geometry returned from handler!" << endl;
      exit(1);
   }

   // Fetch UL image and transform to ground:
   ossimDpt imgUL (0,0);
   ossimGpt gndUL;

   geom->localToWorld(imgUL, gndUL);
   ossimDpt gsd (geom->getMetersPerPixel());

   cout<<"\nFile: "<<fname<<endl;
   cout<<"  Image_UL_corner: "<<gndUL<<endl;
   cout<<"        Image_GSD: "<<gsd<<endl;

   ossimRefPtr<ossimMapProjection> proj;
   if (doGeographic)
      proj = new ossimEquDistCylProjection(ossimEllipsoid(), gndUL);
   else
      proj = new ossimUtmProjection(ossimEllipsoid(), gndUL);

   // Init map projection with UL tiepoint:
   proj->setUlTiePoints(gndUL);
   proj->setMetersPerPixel(gsd);

   ossimDpt testImgPt(x, y);
   ossimGpt testGndPt(x, y);
   if (testImgPt.hasNans())
   {
      ossimIrect imgRect;
      geom->getBoundingRect(imgRect);
      testImgPt = imgRect.midPoint();
   }

   ossimDpt testMapPt, testImgPt2;

   geom->localToWorld(testImgPt, testGndPt);
   cout<<"Using_image_point: "<<testImgPt<<endl;
   cout<<"  Image-to-Ground: "<<testGndPt<<endl;

   testMapPt = proj->worldToLineSample(testGndPt);
   cout<<"    Ground-to-Map: "<<testMapPt<<endl;

   testGndPt = proj->lineSampleToWorld(testMapPt);
   cout<<"    Map-to-Ground: "<<testGndPt<<endl;

   geom->worldToLocal(testGndPt, testImgPt2);
   cout<<"  Ground-to-Image: "<<testImgPt2<<endl;

   cout<<"       Difference: "<<testImgPt2-testImgPt<<endl;
   cout <<endl;
   exit(0);
}

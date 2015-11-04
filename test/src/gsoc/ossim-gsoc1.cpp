//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-foo.cpp
//
// Author:  David Burken
//
// Description: Contains application definition "ossim-foo" app.
//
// NOTE:  This is supplied for simple quick test. DO NOT checkin your test to
//        the svn repository.  Simply edit ossim-foo.cpp and run your test.
//        After completion you can do a "svn revert foo.cpp" if you want to
//        keep your working repository up to snuff.
//
// $Id: ossim-gsoc1.cpp 23420 2015-07-12 11:58:34Z dburken $
//----------------------------------------------------------------------------

// ossim includes:  These are here just to save time/typing...
#include <ossim/init/ossimInit.h>
#include <ossim/point_cloud/ossimPointCloudImageHandler.h>
#include <ossim/point_cloud/ossimGenericPointCloudHandler.h>
#include <vector>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/imaging/ossimTiffWriter.h>

using namespace std;

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);
   
   // TODO: Read your ECEF points from your input file here:
   vector<ossimEcefPoint> ecef_points; // Need to fill this vector array

   ossimEcefPoint Pt1( 4347158.78960514, 848070.80152498, 4573989.32558993);
   ossimEcefPoint Pt2( 4347158.78960514, 853070.80152498, 4573927.96772204);
   ossimEcefPoint Pt3( 4347158.78960514, 858070.80152498, 4573866.60985415);
   ossimEcefPoint Pt4( 4352158.78960514, 848070.80152498, 4569848.07121407);
   ossimEcefPoint Pt5( 4352158.78960514, 853070.80152498, 4569786.71334618);
   ossimEcefPoint Pt6( 4352158.78960514, 858070.80152498, 4569725.35547829);
   ossimEcefPoint Pt7( 4357158.78960514, 848070.80152498, 4565706.81683821);
   ossimEcefPoint Pt8( 4357158.78960514, 853070.80152498, 4565645.45897032);
   ossimEcefPoint Pt9( 4357158.78960514, 858070.80152498, 4565584.10110243);

   ecef_points.push_back(Pt1);
   ecef_points.push_back(Pt2);
   ecef_points.push_back(Pt3);
   ecef_points.push_back(Pt4);
   ecef_points.push_back(Pt5);
   ecef_points.push_back(Pt6);
   ecef_points.push_back(Pt7);
   ecef_points.push_back(Pt8);
   ecef_points.push_back(Pt9);

   ossimRefPtr<ossimGenericPointCloudHandler> pc_handler =
      new ossimGenericPointCloudHandler(ecef_points);

   ossimRefPtr<ossimPointCloudImageHandler> ih =  new ossimPointCloudImageHandler;
   ih->setCurrentEntry((ossim_uint32)ossimPointCloudImageHandler::HIGHEST);
   ih->setPointCloudHandler(pc_handler.get());

   // TODO: This sets the resolution of the output file. There is a default value computed but you
   // may either adjust it or set it manually here:
   ossimDpt gsd;
   ih->getGSD(gsd, 0);
   ossimString gsdstr = ossimString::toString((gsd.x + gsd.y)/2.0); // I use 1/6th of default
   ossimRefPtr<ossimProperty> gsd_prop =
         new ossimStringProperty(ossimKeywordNames::METERS_PER_PIXEL_KW, gsdstr);
   ih->setProperty(gsd_prop);

   // Set up the writer:
   ossimRefPtr<ossimTiffWriter> tif_writer =  new ossimTiffWriter();
   tif_writer->setGeotiffFlag(true);

   ossimFilename outfile ("output.tif");
   tif_writer->setFilename(outfile);
   if (tif_writer.valid())
   {
      tif_writer->connectMyInputTo(0, ih.get());
      tif_writer->execute();
   }

   cout << "Output written to <"<<outfile<<">"<<endl;
   tif_writer->close();
   tif_writer = 0;
   ih = 0;
   pc_handler = 0;
   return 0;
}

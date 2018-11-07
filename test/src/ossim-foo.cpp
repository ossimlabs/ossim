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

// System includes:
#include <cmath>
#include <memory>
#include <sstream>
#include <iostream>

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
#include <ossim/base/ossimTimer.h>
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
#include <ossim/support_data/ImageHandlerState.h>

#include <ossim/init/ossimInit.h>

#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimUtmProjection.h>

#include <ossim/support_data/ossimSrcRecord.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/support_data/ossimWkt.h>

#include <ossim/base/Barrier.h>
#include <ossim/base/Block.h>
#include <ossim/base/Thread.h>
#include <ossim/support_data/TiffHandlerState.h>
#include <ossim/support_data/ImageHandlerStateRegistry.h>
#include <ossim/imaging/ossimNitfCodecFactory.h>
#include <ossim/projection/ossimNitfRpcModel.h>
#include <ossim/projection/ossimQuickbirdRpcModel.h>
#include <ossim/imaging/ossimNitfCodecFactory.h>
#include <ossim/imaging/ossimTiffTileSource.h>

// Put your includes here:

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   char osl2Line[1024];
   char of15Line[1024];
   ossimString osl2Str, of15Str, fname;
   vector<ossimString> osl2Split;
   vector<ossimString> of15Split;
   double v1, v2, delta1, delta2;
   try
   {

      ifstream osl2 (argv[1]);
      ifstream of15 (argv[2]);

      while (!osl2.eof() && !of15.eof())
      {
         osl2Split.clear();
         of15Split.clear();
         osl2.getline(osl2Line, 1024);
         of15.getline(of15Line, 1024);
         osl2Str = osl2Line;
         of15Str = of15Line;
         osl2Split = osl2Str.split(" ");
         of15Split = of15Str.split(" ");

         if (osl2Split.size() < 2)
            continue;
         if (of15Split.size() < 2)
         {
            cout << "Out of sync! (001)"<<endl;
            break;
         }

         if (osl2Split[0].contains("File:"))
         {
            if (!of15Split[0].contains("File:"))
            {
               cout << "Out of sync! (002)"<<endl;
               break;
            }
            fname = osl2Split[1];
            if (of15Split[1] != fname)
            {
               cout <<fname<< "Filename mismatch! ("<<of15Split[1]<<")"<<endl;
               break;
            }

            for (int i=0; i<8; ++i)
            {
               osl2Split.clear();
               of15Split.clear();
               osl2.getline(osl2Line, 1024);
               of15.getline(of15Line, 1024);
               osl2Str = osl2Line;
               of15Str = of15Line;
               osl2Str.trim();
               of15Str.trim();
               osl2Split = osl2Str.split(":(,");
               of15Split = of15Str.split(":(,");

               if ((osl2Split.size() < 4) || (of15Split.size() < 4))
               {
                  cout << fname<<"  Bad line! (003)"<<endl;
                  break;
               }
               osl2Split[2].trim(",");
               of15Split[2].trim(",");
               osl2Split[3].trim(",");
               of15Split[3].trim(",");
               if (((osl2Split[2] == "nan") && (of15Split[2] == "nan")) ||
                   ((osl2Split[3] == "nan") && (of15Split[3] == "nan")))
               {
                  continue;
               }
               else if ((osl2Split[2] == "nan") || (of15Split[2] == "nan") ||
                        (osl2Split[3] == "nan") || (of15Split[3] == "nan"))
               {
                  cout << fname << "  NAN mismatch!" << endl;
                  continue;
               }

               v1 = osl2Split[2].toDouble();
               v2 = of15Split[2].toDouble();
               delta1 = fabs(v1 - v2);

               v1 = osl2Split[3].toDouble();
               v2 = of15Split[3].toDouble();
               delta2 = fabs(v1 - v2);

               if ((delta1 > 0.0000001) || (delta2 > 0.0000001))
               {
                  cout << "##### Large difference detected for file " << fname << " #####"<<endl;
                  break;
               }
            }
         }
      }
      ossimRefPtr<ossimImageHandler> handler = ossimImageHandlerRegistry::instance()->open(fname);
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

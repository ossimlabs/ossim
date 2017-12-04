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
#include <ossim/support_data/ossimWkt.h>

#include <ossim/base/Barrier.h>
#include <ossim/base/Block.h>
#include <ossim/base/Thread.h>
#include <ossim/support_data/TiffHandlerState.h>
#include <ossim/support_data/ImageHandlerStateRegistry.h>

// Put your includes here:
#include<ossim/projection/ossimNitfRpcModel.h>
#include<ossim/projection/ossimQuickbirdRpcModel.h>

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   ossimFilename fname (argv[1]);

   try
   {
      ossimImageHandlerRegistry* handlerReg = ossimImageHandlerRegistry::instance();
      ossimProjectionFactoryRegistry* projReg = ossimProjectionFactoryRegistry::instance();

      // Open the image and fetch the geometry via the plugin:
      ossimRefPtr<ossimImageHandler> pluginHandler = handlerReg->open(fname, true, false);
      ossimRefPtr<ossimImageGeometry> pluginGeom = pluginHandler->getImageGeometry();
      cout<<"pluginGeom's projection: "<<pluginGeom->getProjection()->getClassName()<<endl;

      // NITF:
      ossimRefPtr<ossimNitfRpcModel> nitfProj = new ossimNitfRpcModel(fname);
      ossimRefPtr<ossimImageGeometry> nitfGeom = new ossimImageGeometry(0, nitfProj.get());
      cout<<"NITF projection: "<<nitfProj->getClassName()<<endl;

      // QB:
      ossimRefPtr<ossimQuickbirdRpcModel> dgqbProj = new ossimQuickbirdRpcModel();
      dgqbProj->parseFile(fname);
      ossimRefPtr<ossimImageGeometry> dgqbGeom = new ossimImageGeometry(0, dgqbProj.get());
      cout<<"DG/QB projection: "<<dgqbProj->getClassName()<<endl;

      // Establish even image point distribution:
      ossimIrect imageRect;
      pluginGeom->getBoundingRect(imageRect);
      int dx = (imageRect.width()-1)/2;
      int dy = (imageRect.height()-1)/2;
      ossimGpt pluginGpt, nitfGpt, dgqbGpt;
      ossimDpt ip0;
      double nitfDist, dgqbDist;

      // Compute residual error between models:
      for (int Y=0; Y<3; Y++)
      {
         ip0.y = dy*Y;
         for (int X=0; X<3; X++)
         {
            ip0.x = dx*X;
            pluginGeom->localToWorld(ip0, 0, pluginGpt);

            nitfGeom->localToWorld(ip0, 0, nitfGpt);
            nitfDist = pluginGpt.distanceTo(nitfGpt);

            dgqbGeom->localToWorld(ip0, 0, dgqbGpt);
            dgqbDist = pluginGpt.distanceTo(dgqbGpt);

            cout  <<"\nGround residuals for image point "<<ip0<<":"
                  <<"\n  NITF: "<<nitfDist<<" m"
                  <<"\n  DGQB: "<<dgqbDist<<" m"<<endl;
         }
      }
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

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

// System includes:
#include <cmath>
#include <memory>
#include <sstream>
#include <iostream>

class Info
{
public:
  ossimRefPtr<ossimImageData> m_data;
  std::shared_ptr<ossim::ImageHandlerState> m_state;
};

std::shared_ptr<Info> readFile(const ossimFilename& filename)
{
  ossimTimer::Timer_t start;
  ossimTimer::Timer_t stop;
  ossimRefPtr<ossimImageData> data;

  start = ossimTimer::instance()->tick();
  ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(filename);
  std::shared_ptr<Info> info = std::make_shared<Info>();
  stop = ossimTimer::instance()->tick();
  std::cout << "Open file from filename delta: " << ossimTimer::instance()->delta_s(start, stop) << "\n";
  info->m_state=h->getState();
  start = ossimTimer::instance()->tick();
  info->m_data = h->getTile(ossimIrect(0,0,1023,1023));
  stop = ossimTimer::instance()->tick();
  std::cout << "Get Tile from Handle delta: " << ossimTimer::instance()->delta_s(start, stop) << "\n";

  return info;
}

void readFile(std::shared_ptr<Info> info)
{
  ossimTimer::Timer_t start;
  ossimTimer::Timer_t stop;
  ossimRefPtr<ossimImageData> data;

  start = ossimTimer::instance()->tick();
  ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(info->m_state);
  stop = ossimTimer::instance()->tick();
  std::cout << "Open file from state delta: " << ossimTimer::instance()->delta_s(start, stop) << "\n";
  start = ossimTimer::instance()->tick();
  data = h->getTile(ossimIrect(0,0,1023,1023));

  stop = ossimTimer::instance()->tick();
  std::cout << "Get Tile from state delta: " << ossimTimer::instance()->delta_s(start, stop) << "\n";
  if(data->isEqualTo(*info->m_data, true))
  {
    std::cout << "DATA IS EQUAL!!!\n";
  }
}

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   try
   {
      ossimKeywordlist kwl;
      ossimFilename filename(argv[1]);

// timing test
#if 0 

      for(int x =0 ; x < 10; ++x)
      {
        std::shared_ptr<Info> info = readFile(filename);
#if 0
        int i;
        std::cout << "Hit any key to continue....";
        cin  >> i;
        std::cout << "\n";
#endif
        readFile(info);
        std::cout << "******************\n";

      }
#endif
      ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(filename);

      // std::shared_ptr<ossim::ImageHandlerState> state = readFile(filename);
      // int i;
      // std::cout << "Hit any key to continue....";
      // cin  >> i;
      // std::cout << "\n";
      // readFile(state);

//      std::shared_ptr<ossim::ImageHandlerState> state = h->getState();
      std::shared_ptr<ossim::TiffHandlerState> state = std::make_shared<ossim::TiffHandlerState>();

      state->loadDefaults(filename);

      state->save(kwl);
      std::cout << kwl << "\n";

// timings

      // return 0;

      // ossimRefPtr<ossimImageGeometry> geom = h->getImageGeometry();
      //std::cout << "GETTING GEOMETRY FROM IMAGE HANDLER\n";
      // if(geom)
      // {
      //    geom->saveState(kwl);

      //    std::cout << kwl << "\n";
      //    kwl.clear();
      // }
      //std::cout << "END GETTING GEOMETRY FROM IMAGE HANDLER\n";
//       if(h)
//       {
//          state = h->getState();

//          ossimRefPtr<ossimImageHandler> fromState = ossimImageHandlerRegistry::instance()->open(state);
//          if(fromState)
//          {
//             fromState->saveState(kwl);
//             std::cout << fromState->getBoundingRect() << "\n";
//          }

//          state->save(kwl);
//          ossimKeywordlist loadedKwl;

//          state = ossim::ImageHandlerStateRegistry::instance()->createState(kwl);

//          state->save(loadedKwl);
//          //std:cout << loadedKwl << "\n";

//          ossimRefPtr<ossimProjection> proj = ossimProjectionFactoryRegistry::instance()->createProjection(filename, 0);
//          if(proj)
//          {
//             kwl.clear();
//             proj->saveState(kwl);
// //            std::cout << kwl << "\n";
//          }
// //         std::cout << kwl <<"\n";
//       }
      // std::shared_ptr<ossim::TiffHandlerState> tiffstate = std::make_shared<ossim::TiffHandlerState>();
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

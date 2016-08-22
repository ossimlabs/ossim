/*****************************************************************************
*                                                                            *
*                                 O S S I M                                  *
*            Open Source, Geospatial Image Processing Project                *
*          License: MIT, see LICENSE at the top-level directory              *
*                                                                            *
*****************************************************************************/

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimException.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
   int returnCode = 0;
   ossimInit::instance()->initialize(argc, argv);

   try
   {
      ossimRefPtr<ossimImageHandler> handler =
            ossimImageHandlerRegistry::instance()->open(ossimFilename(argv[1]));
      if (!handler.valid())
      {
         throw ossimException("The handler factory failed to create an hdf5 image handler object. Check that hdf5 is enabled "
            "via the BUILD_OSSIM_HDF5_SUPPORT environment variable before running cmake, and that "
            "the hdf5 library is discoverable.");
      }

      cout<<"\n Successfully created an instance of "<<handler->getClassName()<<endl;
      ossimRefPtr<ossimImageGeometry> geom = handler->getImageGeometry();
      if (!geom.valid())
         throw ossimException("Could not create geometry object.");

      geom->print(cout);
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
   
   cout<<endl;
   return returnCode;
}

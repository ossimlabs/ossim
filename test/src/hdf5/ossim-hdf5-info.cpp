/*****************************************************************************
*                                                                            *
*                                 O S S I M                                  *
*            Open Source, Geospatial Image Processing Project                *
*          License: MIT, see LICENSE at the top-level directory              *
*                                                                            *
*****************************************************************************/

#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/support_data/ossimInfoFactoryRegistry.h>
#include <ossim/support_data/ossimInfoBase.h>
#include <iostream>
#include <memory>

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   try
   {
      std::shared_ptr<ossimInfoBase> info = ossimInfoFactoryRegistry::instance()->create(argv[1]);
      if (info)
      {
         std::cout<<std::endl;
         ossimKeywordlist kwl;
         info->getKeywordlist(kwl);
         kwl.print(std::cout);
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "The info factory failed to create an hdf5 info object. Check that hdf5 is enabled "
            "via the BUILD_OSSIM_HDF5_SUPPORT environment variable before running cmake, and that "
            "the hdf5 library is discoverable." << std::endl;
         returnCode = 1;
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
   
   std::cout<<std::endl;
   return returnCode;
}

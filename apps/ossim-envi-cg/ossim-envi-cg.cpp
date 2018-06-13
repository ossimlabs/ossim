//---
//
// License: MIT
//
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
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/Thread.h>
#include <ossim/projection/ossimEnviCgModel.h>
#include <ossim/imaging/ossimImageGeometry.h>

// Put your includes here:

int main(int argc, char *argv[])
{
   int returnCode = 0;
   if (argc < 2)
   {
      cout<<"\nUsage: "<<argv[0]<<" <filename.geoc>\n"<<endl;
      return 1;
   }
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   ossimFilename fname (argv[1]);

   try
   {
      ossimRefPtr<ossimEnviCgModel> model = new ossimEnviCgModel;
      if (model->loadEnviGeocFile(fname))
      {
         ossimFilename geomFname(fname);
         geomFname.setExtension("geom");
         model->saveCoarseGrid(geomFname);
         ossimKeywordlist kwl;
         ossimImageGeometry geom;
         geom.setProjection(model.get());
         geom.saveState(kwl);
         kwl.write(geomFname);
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

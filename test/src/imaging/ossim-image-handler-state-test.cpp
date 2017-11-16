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
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/base/State.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/init/ossimInit.h>
#include <ossim/support_data/ImageHandlerStateRegistry.h>
// Put your includes here:

// System includes:
#include <cmath>
#include <memory>
#include <sstream>
#include <iostream>

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);
   ap.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   ap.getApplicationUsage()->addCommandLineOption("--max-time-to-open-state","specify the time in seconds.  Example: 1.234");
   ap.getApplicationUsage()->addCommandLineOption("--min-speedup","specify the minumum speedup for state open versus normal open.  For min-speedup = 2.5 means that we must be at least 2.5 times faster with the state");
   ap.getApplicationUsage()->addCommandLineOption("--geom-test","Will enable geometry tests.  Will load the file the with and without the state and make sure the geometry remains the same");
   std::string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);

   try
   {
      ossim_float64 maxTimeForStateOpen = -1;
      ossim_float64 minSpeedup = 0.0;
      bool geomTestFlag = false;
   //   argumentParser.getApplicationUsage()->addCommandLineOption("--random-seed", "value to use as the seed for the random elevation post generator");
      if (ap.argc() == 1 || ap.read("-h") ||
          ap.read("--help"))
      {
         ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
         exit(0);
      }
      if(ap.read("--max-time-to-open-state", sp1))     
      {
         maxTimeForStateOpen = ossimString(ts1).toFloat64();
      }
      if(ap.read("--min-speedup", sp1))     
      {
         minSpeedup = ossimString(ts1).toFloat64();
      }
      if(ap.read("--geom-test"))
      {
         geomTestFlag = true;
      }
      if(ap.argc() == 2)
      {

         ossimTimer::Timer_t t1;
         ossimTimer::Timer_t t2;         
         std::shared_ptr<ossim::ImageHandlerState> state;
         ossim_float64 deltaNoState = 0.0;

         //std::cout << "maxTimeForStateOpen: " << maxTimeForStateOpen << "\n";
         t1 = ossimTimer::instance()->tick();
         ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(ossimFilename(ap[1]));
         t2 = ossimTimer::instance()->tick();
         deltaNoState = ossimTimer::instance()->delta_s(t1,t2);

         std::cout << "open-image-delta: " << deltaNoState << "\n"; 

         if(h)
         {
            state = h->getState();

            t1 = ossimTimer::instance()->tick();
            ossimRefPtr<ossimImageHandler> hState  = ossimImageHandlerRegistry::instance()->open(state);
            t2 = ossimTimer::instance()->tick();
            
            if(hState)
            {
               ossim_float64 deltaState = ossimTimer::instance()->delta_s(t1,t2);
               std::cout << "open-image-state-delta: " << deltaState << "\n"; 
               if(maxTimeForStateOpen > 0.0)
               {
                  std::cout << "max-time-to-open-state: " << maxTimeForStateOpen << "\n";
                  if(deltaState>maxTimeForStateOpen)
                  {
                     throw ossimException("Took too long for state open!");
                  }  
               }
               ossim_float64 r = deltaNoState/deltaState;

               if(minSpeedup > 0.0)
               {
                  std::cout << "speedup: " << r << "\n";
                  if(r < minSpeedup)
                  {
                     throw ossimException("Took too long for state open!");
                  }
               }

               if(geomTestFlag)
               {
                  t1 = ossimTimer::instance()->tick();
                  ossimRefPtr<ossimImageGeometry> geomNoState = h->getImageGeometry();
                  t2 = ossimTimer::instance()->tick();
                  std::cout << "geometry-no-state-delta: " << ossimTimer::instance()->delta_s(t1,t2) << "\n";
                  t1 = ossimTimer::instance()->tick();
                  ossimRefPtr<ossimImageGeometry> geomState   = hState->getImageGeometry();
                  t2 = ossimTimer::instance()->tick();
                  std::cout << "geometry-state-delta: " << ossimTimer::instance()->delta_s(t1,t2) << "\n";

                  if(geomNoState&&geomState)
                  {
                     ossimKeywordlist kwl1,kwl2;

                     geomNoState->saveState(kwl1);
                     geomState->saveState(kwl2);

                     if(kwl1 != kwl2)
                     {
                        std::cout << "GEOMETRY from No State Load: \n" << kwl1 << "\n";
                        std::cout << "GEOMETRY from State Load: \n"    << kwl2 << "\n";
                        throw ossimException("Geometries are not equal");
                     }
                  }
                  else
                  {
                     throw ossimException("Unable to create geometry for geom test");
                  }
               }
            }
            else
            {
               throw ossimException(ossimString("Unable to open image from state")+ ap[1]);
            }
         }
         else
         {
            throw ossimException(ossimString("Unable to open file") + ap[1]);
         }
      }
      else
      {
         throw ossimException("Image not specified");
      }
    // add code
   }
   catch(const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      returnCode = 1;
   }
   catch( ... )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << argv[0] << " caught unhandled exception!" << std::endl;
      returnCode = 1;
   }
   
   return returnCode;
}

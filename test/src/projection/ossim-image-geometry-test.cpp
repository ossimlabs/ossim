//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:
//
// Test app for image geometries.  Prints out projection keyword
// list.  Performs lineSampleToWorld, worldToLineSample, lineSampleToWorld
// round trip dumping points and delta on image points.  Dumps elevation cells
// used.
// 
//----------------------------------------------------------------------------
// $Id: ossim-image-geometry-test.cpp 22201 2013-03-20 00:53:33Z dburken $

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <ctime>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>

// Points in one direction.  If 5, will get 5X5 or 25.
static const ossim_int32 POINTS = 5; 

static std::string getDate()
{
// Get the build date in the format of (yyyymmdd).
   char s[9];
   s[8] = '\0';
   time_t t;
   time(&t);
   tm* lt = localtime(&t);
   strftime(s, 9, "%Y%m%d", lt);
   std::string date = s;
   return date;
}

static void usage()
{
   cout << "ossimGeometryTest <imagefile>"
        << "\nTest image geometry." << endl;
}

static const ossim_uint32 W = 14;

static void print(const std::string& prefix,
                  const ossimDpt& ipt,
                  const ossimGpt& wpt,
                  const ossimDpt& rpt)
{
   ossimDpt delta = ipt - rpt;
   ossim_uint32 w =
      (prefix.size() < W) ? (W - prefix.size()) : prefix.size();
                                                     
   cout << std::setfill(' ') << setiosflags(ios::left) 
        << prefix << std::setw(w) << "ipt:" << ipt.toString().c_str() << "\n"
        << prefix << std::setw(w) << "wpt:" << wpt.toString().c_str() << "\n"
        << prefix << std::setw(w) << "rpt:" << rpt.toString().c_str() << "\n"
        << prefix << std::setw(w) << "delta:" << delta.toString() << "\n\n";
}


int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   if (argc != 2)
   {
      usage();
      return 0;
   }

   ossimFilename imageFile = argv[argc - 1];

   if ( !imageFile.exists() )
   {
      usage();
      return 1;
   }

   // Open up an image handler.
   ossimRefPtr<ossimImageHandler> ih
      = (ossimImageHandler*)ossimImageHandlerRegistry::instance()->open(
         ossimFilename(argv[argc - 1]));
   if ( !ih )
   {
      cerr << "Could not open: " << imageFile << endl;
      return 1;
   }

   cout << "image_file:  " << imageFile << "\n";

   // cout << "date_yyyymmdd: " << getDate().c_str() << "\n"; 
   
   // Get the entry list:
   std::vector<ossim_uint32> entryList;
   ih->getEntryList(entryList);
   
   std::vector<ossim_uint32>::const_iterator entry = entryList.begin();
   while ( entry != entryList.end() )
   {
      cout << "entry_index:  " << (*entry) << "\n";
      ih->setCurrentEntry( (*entry) );

      ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();

      if ( geom.valid() )
      {
#if 1    /* Comment out to disable elevation for map projections. */
         ossimRefPtr<ossimProjection> proj = geom->getProjection();
         if (proj.valid() )
         {
            ossimRefPtr<ossimMapProjection> mp = PTR_CAST(ossimMapProjection, proj.get());
            if (mp.valid())
            {
               cout << "setting elev lookup flag..." << endl;
               mp->setElevationLookupFlag(true);
            }
         }
#endif
         ossimKeywordlist kwl;
         geom->saveState(kwl, 0);
         cout << kwl << "\n";

         ossimDrect rect = ih->getImageRectangle(0);

         cout << "image_rect: " << rect << "\n\n";

         // Do forward, inverse, forward on corners and center.

         std::string prefix;
         ossimDpt ipt;  // image point
         ossimDpt rpt;  // round trip image point
         ossimGpt wpt;  // world point

         // Upper left:
         prefix = "ul.";
         ipt = rect.ul();
         geom->localToWorld(ipt, wpt);
         geom->worldToLocal(wpt, rpt);
         print(prefix, ipt, wpt, rpt);

         // Upper right:
         prefix = "ur.";
         ipt = rect.ur();
         geom->localToWorld(ipt, wpt);
         geom->worldToLocal(wpt, rpt);
         print(prefix, ipt, wpt, rpt);

         // Lower right:
         prefix = "lr.";
         ipt = rect.lr();
         geom->localToWorld(ipt, wpt);
         geom->worldToLocal(wpt, rpt);
         print(prefix, ipt, wpt, rpt);

         // Lower left:
         prefix = "ll.";
         ipt = rect.ll();
         geom->localToWorld(ipt, wpt);
         geom->worldToLocal(wpt, rpt);
         print(prefix, ipt, wpt, rpt);

         // center:
         prefix = "center.";
         ipt = rect.midPoint();
         geom->localToWorld(ipt, wpt);
         geom->worldToLocal(wpt, rpt);
         print(prefix, ipt, wpt, rpt);

         geom->setTargetRrds(2);
         cout << "geometry target rrds set to: " << geom->getTargetRrds() << "\n\n";
         
         // Upper left:
         prefix = "ul-r2.";
         geom->rnToRn(rect.ul(), 0, 2, ipt);
         geom->localToWorld(ipt, wpt);
         geom->worldToLocal(wpt, rpt);
         print(prefix, ipt, wpt, rpt);

         // Upper right:
         prefix = "ur-r2.";
         geom->rnToRn(rect.ur(), 0, 2, ipt);
         geom->localToWorld(ipt, wpt);
         geom->worldToLocal(wpt, rpt);
         print(prefix, ipt, wpt, rpt);

         // Lower right:
         prefix = "lr-r2.";
         geom->rnToRn(rect.lr(), 0, 2, ipt);
         geom->localToWorld(ipt, wpt);
         geom->worldToLocal(wpt, rpt);
         print(prefix, ipt, wpt, rpt);

         // Lower left:
         prefix = "ll-r2.";
         ipt = rect.ll();
         geom->rnToRn(rect.ll(), 0, 2, ipt);
         geom->localToWorld(ipt, wpt);
         geom->worldToLocal(wpt, rpt);
         print(prefix, ipt, wpt, rpt);

         // center:
         prefix = "center-r2.";
         ipt = rect.midPoint();
         geom->rnToRn(ipt, 0, 2, ipt);
         geom->localToWorld(ipt, wpt);
         geom->worldToLocal(wpt, rpt);
         print(prefix, ipt, wpt, rpt);

         // Test rnToWorld and world to rn:
         cout << "testing rnToWorld and worldToRn:\n\n";
         prefix = "center-r3.";
         ipt = rect.midPoint();
         geom->rnToRn(ipt, 0, 3, ipt);
         geom->rnToWorld(ipt, 3, wpt);
         geom->worldToRn(wpt, 3, rpt);
         print(prefix, ipt, wpt, rpt);

         geom->setTargetRrds(0);
         cout << "geometry target rrds set to: " << geom->getTargetRrds() << "\n\n";

         prefix = "center-r3.";
         ipt = rect.midPoint();
         geom->rnToRn(ipt, 0, 3, ipt);
         geom->rnToWorld(ipt, 3, wpt);
         geom->worldToRn(wpt, 3, rpt);
         print(prefix, ipt, wpt, rpt);
         

         ossim_int32 tenthW  = rect.width()  / POINTS;
         ossim_int32 tenthH  = rect.height() / POINTS;
         ossim_int32 offW    = tenthW / 2;
         ossim_int32 offH    = tenthH / 2;
         ossimIpt    ul      = rect.ul();
         ossimIpt    lr      = rect.lr();
         ossim_int32 ptIndex = 0;

         // Line loop:
         for (ossim_int32 y = ul.y+offH; y < lr.y; y += tenthH)
         {
            ipt.y = y;
            
            // Sample loop:
            for (ossim_int32 x = ul.x+offW; x < lr.x; x += tenthW)
            {
               ipt.x = x;

               std::ostringstream s;
               s << std::setfill('0') << setiosflags(ios::right) 
                 << "pt" << std::setw(3) << ptIndex << ".";
               prefix = s.str();
               geom->localToWorld(ipt, wpt);
               geom->worldToLocal(wpt, rpt);
               print(s.str(), ipt, wpt, rpt);

               ++ptIndex;
            }
         }

         // Test the decimationFactors method.
         ossim_uint32 level;
         std::vector<ossimDpt> decimations;
         geom->decimationFactors(decimations);
         
         for (level = 0; level < decimations.size(); ++level)
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "decimation[" << level << "]: " << decimations[level]
               << std::endl;
         }
         
         bool isAffectedByElevation = geom->isAffectedByElevation();
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "\nImage projection " << ( isAffectedByElevation ? "is" : "isn't")
            << " affected by elevation." << std::endl;
         
         // Test up is code:
         if ( isAffectedByElevation )
         {
            ossim_float64 upIsUpAngle = geom->upIsUpAngle();
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "\nUp is up rotation angle: " << upIsUpAngle << "\n" << std::endl;
         }
         
#if 0 /* commented out as one time test is good */
         
         // Test the decimationFactor method.
         ossimDpt decimation;
         geom->decimationFactors(decimations);
         ossim_float64 r0_lines = ih->getNumberOfLines(0);
         ossim_float64 r0_samps = ih->getNumberOfSamples(0);
         
         for (level = 0; level < geom->getNumberOfDecimations(); ++level)
         {
            geom->decimationFactor(level, decimation);
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "decimation[" << level << "]:          " << decimation
               << "\nlines[" << level << "]:             " << ih->getNumberOfLines(level)
               << "\nsamples[" << level << "]:           " << ih->getNumberOfSamples(level)
               << "\ncomputed lines[" << level << "]:    " << (r0_lines * decimation.y)
               << "\ncomputed samples[" << level << "]:  " << (r0_samps * decimation.x)
               << std::endl;
         }
#endif

         // Print out transform, if any:
         if ( geom->getTransform() != 0 )
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "Image geometry 2d To 2d transform:\n"
               << *(geom->getTransform()) << "\n";
         }
         else
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "Image geometry has no 2d To 2d transform.\n";
         }
            
         
      }
      
      ++entry; // Go to next entry.
      
   } // matches: while ( entry != entryList.end() )

#if 0
   // Dump the elevation cells used:
   std::vector<ossimFilename> cells;
   ossimElevManager::instance()->getOpenCellList(cells);

   for (ossim_uint32 cellIndex = 0; cellIndex < cells.size(); ++cellIndex)
   {
      std::ostringstream s;
      s << std::setfill('0')
        << "cell" << std::setw(3) << cellIndex << ": ";

      cout << std::setfill(' ') << setiosflags(ios::left) << std::setw(W)
           << s.str().c_str() << cells[cellIndex] << "\n";
   }

   cout << endl;
#endif
   return 0;
}


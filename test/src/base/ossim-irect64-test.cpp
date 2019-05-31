//---
// ossim file: ossim-irect64-test.cpp
//
// Description: Contains application definition "ossim-irect64-test" app.
//
// License: MIT
//---
// $Id$

#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimIpt64.h>
#include <ossim/base/ossimIrect64.h>


// Put your includes here:

// System includes:
#include <cmath>
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   // ossimArgumentParser ap(&argc, argv);
   // ossimInit::instance()->addOptions(ap);
   // ossimInit::instance()->initialize(ap);
   
   try
   {
      std::string s0 = "(16, 18, 20, 20, LH)"; // endpt = (35, 37)
      ossimIrect64 r0;
      r0.toRect(s0);
      ossimIpt64 endPt0;
      r0.end( endPt0 );

      cout << "\ns0: " << s0
           << "\nr0: " << r0
           << "\nr0 end pt: " << endPt0 << endl;

      ossimIrect64 r1( ossimIpt64(32, 32), ossimIpt64(20, 20)); // endpt = (51,51)
      ossimIpt64 endPt1;
      r1.end( endPt1 );
     
      cout << "\nr1: " << r1
           << "\nr1 end pt: " << endPt1 << endl;
      
      cout << "\nr0 intersects r1: " << (r0.intersects(r1)?"true":"false") << endl;

      cout << "\nr1 clipped to r0: " << r1.clipToRect(r0) << endl;

      cout << "\nr1 combined with r0: " << r1.combine(r0) << endl;

      ossimIrect64 r2(65, 65, 18, 18);
      ossimIpt64 endPt2;
      r2.end( endPt2 );
      cout << "\nr2: " << r2
           << "\nr2 end pt: " << endPt2 << endl;

      cout << "\nr2 clipped to r0: " << r2.clipToRect(r0) << endl;

      cout << "\nr2 combined with r0: " << r2.combine(r0) << endl;
     
      ossimIrect64 r3( 123, OSSIM_INT64_NAN, 456, OSSIM_INT64_NAN );
      ossimString s3 = r3.toString();
      ossimIrect64 r4;
      r4.toRect(s3);
      cout << "\nr3: " << r3
           << "\ns3: " << s3
           << "\nr4: " << r4
           << endl;

      ossimIrect64 r5(82, 82, 10, 10);
      cout << "\nr5: " << r5
           << "\nr5.clipped to r2" << r5.clipToRect(r2)
           << "\nr5.clipped to r1" << r5.clipToRect(r1) << endl;

      ossimIrect64 r6(1024, 1024, 256, 256);
      ossimIrect64 r7(0, 0, 2024, 2048);
      ossimIrect64 r8(1000, 1000, 256, 256);
      cout << "\nr6: " << r6
           << "\nr7: " << r7
           << "\nr8: " << r8
           << "\nr6 completely within r7: " << r6.completely_within( r7 )
           << "\nr6 completely within r8: " << r6.completely_within( r8 )
           << endl;

      ossimIrect64 r9(553, -553, 950, 950);
      ossimIpt64 tileWidthHeight(64,64);
      cout << "\nr9: " << r9 << " end: " << r9.end();
      r9.stretchToTileBoundary(tileWidthHeight);
      cout << "\nr9.stretchToTileBoundary(64,64): " << r9 << " end: "
           << r9.end() << endl;
   }
   catch(const ossimException& e)
   {
      // ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      returnCode = 1;
   }
   catch( ... )
   {
      // ossimNotify(ossimNotifyLevel_WARN)
      // << "ossim-pt caught unhandled exception!" << std::endl;
      returnCode = 1;
   }
   
   return returnCode;
}

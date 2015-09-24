//----------------------------------------------------------------------------
//
// File ossim-fgdc-txt-doc-test.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Test application for ossimFgdcTxtDoc class.
// 
//----------------------------------------------------------------------------
// $Id: ossim-info-test.cpp 19751 2011-06-13 15:13:07Z dburken $

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/support_data/ossimFgdcTxtDoc.h>

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);

   // Initialize ossim stuff, factories, plugin, etc.
   ossimInit::instance()->initialize(ap);

   if ( ap.argc() == 2 )
   {
      try
      {
         // Test the ossimFgdcTxtDoc
         ossimRefPtr<ossimFgdcTxtDoc> fgdcDoc = new ossimFgdcTxtDoc();
         ossimFilename file(argv[1]);
         if ( fgdcDoc->open( file ) )
         {
            cout << "opened: " << file << endl;
            ossimRefPtr<ossimProjection> proj;
            fgdcDoc->getProjection(proj);
            if ( proj.valid() )
            {
               proj->print(cout);
            }

            std::string units;
            fgdcDoc->getAltitudeDistanceUnits(units);
            cout << "units: " << units << endl;
         }
         
      }
      catch (const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         return 1;
      }
      
   }  // End: if ( ( ap.argc() == 2 ) ...
   else
   {
      cout << argv[0] << " <FGDC-text-doc.txt>\nTest ossimFgdcTxtDoc class..." << endl;
   }
   
   return 0;
   
} // End of main...

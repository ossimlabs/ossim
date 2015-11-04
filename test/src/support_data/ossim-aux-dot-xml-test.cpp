//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-aux-dot-xml-test.cpp
//
// Author:  David Burken
//
// Description: Test application for ossimAuxXmlSupportData class.
//
//----------------------------------------------------------------------------
// $Id$

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/init/ossimInit.h>
#include <ossim/support_data/ossimAuxXmlSupportData.h>

#include <iostream>
#include <sstream>
using namespace std;

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   int status = 0;
   
   if (argc == 2)
   {
      try
      {   
         ossimFilename f = ossimFilename(argv[1]);
         ossimAuxXmlSupportData sd;
         ossimRefPtr<ossimProjection> proj = sd.getProjection( f );
         if ( proj.valid() )
         {
            proj->print( cout );
         }
         else
         {
            cerr << "Could not get projection from: " << f << endl;
            status = 1;
         }
      }
      catch (const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
         status = 1;
      }
   }
   else
   {
      cout << argv[0] << " <input-aux.xml>"
           << "\nAttempts to get/print a projection from aux dot xml file."
           << endl;
   }
      
   return status;
}

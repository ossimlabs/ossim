//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Test app for ossimEnviHeader class.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/support_data/ossimEnviHeader.h>
#include <iostream>
using namespace std;


int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   if (argc != 2)
   {
      cout << argv[0] << " <envi_header_file>"
           << "\nOpen, parse and print ossimEnviHeader object." << endl;
      return 0;
   }

   cout << "ossimEnviHeader test:\n";

   ossimEnviHeader hdr;
   if ( hdr.open( ossimFilename( argv[1] ) ) )
   {
      ossimString s = "Hello world...";
      hdr.setDescription( s );
      
      cout << hdr << endl;
   }
   else
   {
      cout << "Could not open: " << argv[1] << endl;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// XML functions test.
// 
//----------------------------------------------------------------------------
// $Id: ossim-xml-test.cpp 23285 2015-04-28 19:45:07Z dburken $

#include <iostream>
using namespace std;

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimXmlDocument.h>
#include <ossim/init/ossimInit.h>

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   if (argc != 2)
   {
      cout << "usage: " << argv[0] << " <xml_file>" << endl;
      return 0;
   }
   
   ossimFilename f = argv[1];

   cout << "file: " << f << endl;

   ossimXmlDocument* xdoc = new ossimXmlDocument();
   if ( xdoc->openFile(f) )
   {
      cout << "opened..." << endl;
   }
   else
   {
      cout << "not opened..." << endl;
   }
   
   delete xdoc;

   return 0;
}

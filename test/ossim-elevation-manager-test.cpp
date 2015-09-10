//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Test code for ossimElevManger.
//
// $Id: ossim-elevation-manager-test.cpp 22751 2014-04-25 17:54:56Z dburken $
//----------------------------------------------------------------------------
#include <iostream>
using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/elevation/ossimElevManager.h>

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

#if 1
   if (argc != 2)
   {
      cout << "usage: " << argv[0] << " <elevationDirectoryToOpen>" << endl;
      return 0;
   }
   
   ossimFilename dir = argv[1];

   cout << "dir: " << dir << endl;
   
   if (ossimElevManager::instance()->loadElevationPath(dir))
   {
      cout << "ossimElevManager::openDirectory worked for: " << dir << endl;
   }
   else
   {
      cout << "ossimElevManager::openDirectory failed for: " << dir << endl;
   }
#endif
#if 1
   std::vector<std::string> cells;
   ossimElevManager::instance()->getCellsForBounds( 24.0, -83.0, 28.0, -80.0, cells );

   std::vector<std::string>::const_iterator i = cells.begin();
   while ( i != cells.end() )
   {
      cout << "cell: " << (*i) << endl;
      ++i;
   }
#endif
   
   return 0;
}

//----------------------------------------------------------------------------
//
// File: ossim-directory-test.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:
//
// Test application for ossimDirectory class.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimDirectory.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <iostream>
using namespace std;

static void usage()
{
   cout << "ossim-directory-test <directory>"
        << "\nDumps files in directory..." << endl;
}

int main(int argc, char *argv[])
{
   int result = 0;
   
   ossimInit::instance()->initialize(argc, argv);
   
   if (argc == 2)
   {
      try // Exceptions can be thrown so 
      {
         ossimFilename directory = argv[1];
         
         cout << "directory: " << directory << "\n";
         directory = directory.expand();

         ossimDirectory d;
         if ( d.open(directory) )
         {
            ossimFilename f;
            if ( d.getFirst(f) )
            {
               do
               {
                  if ( f.size() )
                  {
                     cout << "file: " << f << endl;
                  }
                  else
                  {
                     cout << "file is empty!" << endl;
                  }
               } while ( d.getNext(f) );
            }
            else
            {
               cout << "ossimDirectory::getFirst() returned false!" << endl;
            }
         }
      }
      catch( const ossimException& e )
      {
         cout << e.what() << endl;
         result = 1;
      }
   }
   else
   {
      usage();
   }
   
   return result;
}

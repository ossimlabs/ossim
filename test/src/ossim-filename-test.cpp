//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// ossimFilename class test app.
// 
//----------------------------------------------------------------------------
// $Id: ossim-filename-test.cpp 20097 2011-09-14 17:03:00Z dburken $

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimFilename.h>

#include <iostream>
#include <string>
using namespace std;

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   ossimFilename directoryToDelete;

   if ( argc == 2 )
   {
      ossimString osArg1 = argv[1];
      osArg1.downcase();
      if ( (osArg1 == "-h") || (osArg1 == "--help") )
      {
         cout << "\nUsage: ossim-filename-test <optional_directory_to_delete>\n"
              << "Runs various test on ossimFilename class.\n"
              << "If optional_directory_to_delete is set calls ossimFilename::remove on it.\n"
              << "Options: -h --help Gives this usage.\n" << endl;
         return 0;
      }
      else
      {
         directoryToDelete = argv[1];
      }
   }
         
   ossimFilename a  = "c:\\foo\\you";
   ossimFilename a2 = "\\foo\\you";
   ossimFilename b  = "/foo/you";
   ossimFilename c  = "foo/you";
   ossimFilename d1 = "~/docs";
   ossimFilename d2 = "./foo";
   ossimFilename d3 = "../../foo";
   ossimFilename d4 = ".";
   ossimFilename d5 = "ab";
   ossimFilename d6 = "a";

   // Environment var WORK must be set.
   ossimFilename d7 = "$(WORK)/ossim";
   ossimFilename d8 = "~/tmp/$(WORK)/ossim";
   
  ossimFilename e = "/foo/you.xxx";
   ossimString   ext1 = ".tif";
   ossimString   ext2 = "jpg";
   ossimString   ext3;


   cout << "\na:              " << a
        << "\na.expand():     " << a.expand()
        << "\na.isRelative(): " << a.isRelative() << "\n"

        << "\na2:              " << a2
        << "\na2.expand():     " << a2.expand()
        << "\na2.isRelative(): " << a2.isRelative() << "\n"      

        << "\nb:              " << b
        << "\nb.expand():     " << b.expand()
        << "\nb.isRelative(): " << b.isRelative() << "\n"
      
        << "\nc:              " << c
        << "\nc.expand():     " << c.expand()
        << "\nc.isRelative(): " << c.isRelative() << "\n"

        << "\nd1:              " << d1
        << "\nd1.expand():     " << d1.expand()
        << "\nd1.isRelative(): " << d1.isRelative() << "\n"
      
        << "\nd2:              " << d2
        << "\nd2.expand():     " << d2.expand()
        << "\nd2.isRelative(): " << d2.isRelative() << "\n"

        << "\nd3:              " << d3
        << "\nd3.expand():     " << d3.expand()
        << "\nd3.isRelative(): " << d3.isRelative() << "\n"

        << "\nd4:              " << d4
        << "\nd4.expand():     " << d4.expand()
        << "\nd4.isRelative(): " << d4.isRelative() << "\n"

        << "\nd5:              " << d5
        << "\nd5.expand():     " << d5.expand()
        << "\nd5.isRelative(): " << d5.isRelative() << "\n"
      
        << "\nd6:              " << d6
        << "\nd6.expand():     " << d6.expand()
        << "\nd6.isRelative(): " << d6.isRelative() << "\n";

   cout << "\nNOTE: d7 and d8 output require variable WORK be set in your "
        << "environment.\n"
      
        << "\nd7:              " << d7
        << "\nd7.expand():     " << d7.expand()
        << "\nd7.isRelative(): " << d7.isRelative() << "\n"

        << "\nd8:              " << d8
        << "\nd8.expand():     " << d8.expand()
        << "\nd8.isRelative(): " << d8.isRelative() << "\n"
      
        << "\next1:        " << ext1
        << "\next2:        " << ext2
        << "\next3:        " << ext3 << "\n";
   
   cout << "e:                    " << e << "\n";
   cout << "e.setExtension(ext1): " << e.setExtension(ext1) << "\n";
   cout << "e.setExtension(ext2): " << e.setExtension(ext2) << "\n";
   cout << "e.setExtension(ext3): " << e.setExtension(ext3) << "\n";

   // Test dir cat.
   a = "/foo/";
   b = "you";
   c = a.dirCat(b);
   cout << "\na:           " << a
        << "\nb:           " << b
        << "\nc = a.dirCat(b): " << c << "\n";

   a = "/foo";
   b = "you";
   c = a.dirCat(b);
   cout << "\na:           " << a
        << "\nb:           " << b
        << "\nc = a.dirCat(b): " << c << "\n";

   a = "/foo";
   b = "./you";
   c = a.dirCat(b);
   cout << "\na:           " << a
        << "\nb:           " << b
        << "\nc = a.dirCat(b): " << c << "\n";

   a = "/foo";
   b = "/you";
   c = a.dirCat(b);
   cout << "\na:           " << a
        << "\nb:           " << b
        << "\nc = a.dirCat(b): " << c << "\n";

   a = "/foo";
   b = "";
   c = a.dirCat(b);
   cout << "\na:           " << a
        << "\nb:           " << b
        << "\nc = a.dirCat(b): " << c << "\n";

   a = "";
   b = "you";
   c = a.dirCat(b);
   cout << "\na:           " << a
        << "\nb:           " << b
        << "\nc = a.dirCat(b): " << c << "\n";

   a = "/data/images/t1.tif";
   cout << "\na: " << a
        << "\na.fileNoExtension(): " << a.fileNoExtension()
        << "\na.noExtension(): " << a.noExtension() << "\n";

   // Test a file with two dots.
   a = "/data/ascii_dems/dtm_v1.5_5mp_3141.asc";
   cout << "\n// Test a file with two dots."
        << "\na: " << a
        << "\na.fileNoExtension(): " << a.fileNoExtension()
        << "\na.noExtension(): " << a.noExtension() << "\n";

   std::string s = "abc";
   ossimFilename f = s; // Used to not compile.
   cout << "ossimFilename f = std::string(abc): " << f << "\n";

   if ( directoryToDelete.size() )
   {
      cout << "ossimFilename::remove on " << directoryToDelete << " was ";
      if ( directoryToDelete.remove() )
      {
         cout << "successful...\n";
      }
      else
      {
         cout << "unsuccessful...\n";
      }
   }

   cout << endl;
   
   return 0;
}

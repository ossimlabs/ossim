//----------------------------------------------------------------------------
//
// File ossim-fgdc-txt-doc-test.cpp
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: Test for the generic nitf tags
//
//----------------------------------------------------------------------------
// $Id:

#include <ossim/support_data/ossimNitfCsexrbTag.h>
#include <ossim/support_data/ossimNitfGenericTag.h>

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/init/ossimInit.h>

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
   cout << "Hello World! Courtesy of OSSIM." << endl;
   ossimNitfCsexrbTag csexrb;
   string fname = getenv("OSSIM_DATA");
   fname += "/19SEP01060448-P1BS-200007943201_01_P004.NTF";
   cout << fname << endl;
   ifstream file(fname);

   std::string buffer;

   if (!file.is_open())
   {
      std::cerr << "Could not open the file: " << std::endl;
      return 1;
   }

   char ch;
   while (file.get(ch))
   {
      std::cout << ch; // Print each character
      buffer += ch;

      // Keep buffer the same length as the target
      if (buffer.size() > 11)
         buffer.erase(0, 1); // Remove the first character

      if (buffer == "CSEXRB00443")
      {
         std::cout << "\nTarget string found. Stopping read." << std::endl;
         break;
      }
   }

   csexrb.parseStream(file);
   csexrb.print(cout, "");
   csexrb.writeStream(cout);

   return 0;
}

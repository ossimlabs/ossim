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
#include <ossim/support_data/ossimNitfCssfabDes.h>
#include <ossim/support_data/ossimNitfCsattbDes.h>

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
   ossimNitfCsexrbTag csexrb = ossimNitfCsexrbTag();
   ossimNitfCssfabDes cssfab = ossimNitfCssfabDes();
   ossimNitfCsattbDes csattb = ossimNitfCsattbDes();
   string fname = getenv("OSSIM_DATA");
   //19SEP01060448-P1BS-200007943201_01_P004.NTF
   //24MAR05002840-P1BS-200004901937_01_P001_B.NTF
   fname += "/24MAR05002840-P1BS-200004901937_01_P001_B.NTF";
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
      //std::cout << ch; // Print each character
      buffer += ch;

      // Keep buffer the same length as the target
      if (buffer.size() > 6)
         buffer.erase(0, 1); // Remove the first character

      if (buffer == "CSEXRB")
      {
         file.seekg (5, ios::cur);
         std::cout << "\nTarget string found. Stopping read." << std::endl;
         break;
      }
   }

   csexrb.parseStream(file);
   csexrb.writeStream(cout);
   csexrb.print(cout, "");

   while (file.get(ch))
   {
      //std::cout << ch; // Print each character
      buffer += ch;

      // Keep buffer the same length as the target
      if (buffer.size() > 6)
         buffer.erase(0, 1); // Remove the first character

      if (buffer == "CSATTB")
      {
         file.seekg (19, ios::cur);
         std::cout << "\nTarget string found. Stopping read." << std::endl;
         break;
      }
   }

   csattb.parseStream(file);
   csattb.writeStream(cout);
   csattb.print(cout, "");

   while (file.get(ch))
   {
      //std::cout << ch; // Print each character
      buffer += ch;

      // Keep buffer the same length as the target
      if (buffer.size() > 6)
         buffer.erase(0, 1); // Remove the first character

      if (buffer == "CSSFAB")
      {
         file.seekg (19, ios::cur);
         std::cout << "\nTarget string found. Stopping read." << std::endl;
         break;
      }
   }

   cssfab.parseStream(file);
   cssfab.writeStream(cout);
   cssfab.print(cout, "");

   return 0;
}

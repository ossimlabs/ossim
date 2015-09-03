//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Ken Melero
// 
// Description: Changes the value of a keyword within a keyword list to
//              value passed in by user.
//
//********************************************************************
// $Id: chgkwval.cpp 13083 2008-06-25 19:45:41Z dburken $

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <iostream>

int main(int argc, char* argv[])
{
   enum
   {
      OK    = 0,
      ERROR = 1
   };
   
   int status = OK;
   
   if ( argc == 4 )
   {
      ossimFilename kwlFile = argv[1];
      ossimKeywordlist kwl;
      
      if ( kwlFile.exists() )
      {
         // Open existing keyword list.
         if ( kwl.addFile( kwlFile ) == false )
         {
            status = ERROR;
            std::cerr
               << "Error opening keyword list:  " << kwlFile << std::endl;
         }
      }

      if ( status == OK )
      {
         std::string key = argv[2];
         std::string value = argv[3];
         
         // Add or change the value of keyword in the keyword list.
         kwl.addPair( key, value, true );

         // Create or re-write the file.
         kwl.write(kwlFile); 
   
         std::cout
            << "Added or changed key: \"" << key << "\" with value: \"" << value
            << "\" in " << kwlFile << std::endl;
      }
   }
   else // Usage:
   {
      std::cout
         << "Usage: chgkwval <keywordlist_file> <keyword> <value>\n"
         << "Changes the keyword value within the keyword list to "
         << "value specified.\n"
         << "Example:   chgval band1.spec area.size \"pixels 7689 8031\"\n"
         << "Changes the keyword value of area.size to pixels 7689 8031\n\n"
         << "Note 1:  Use no colon \":\" after the keyword.\n"
         << "Note 2:  If the keyword is present in the keyword list it will\n"
         << "         be overwritten.\n"
         << "Note 3:  If the keyword is not in the keyword list it will"
         << " be added.\n"
         << "Note 4:  Enclose value in double quotes if it is more than one"
         << " string.\n"
         << "Note 5:  Will create file if doesn't exists.\n"
         << std::endl;
   }
   
   return status;
}

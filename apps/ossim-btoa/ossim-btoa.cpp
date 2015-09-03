//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Ken Melero
//         Originally written by Oscar Kramer.
//         
// Description:  This app displays a binary file in a nice
//               formatted view.  Very helpful for finding offsets of
//               binary headers.
//
//********************************************************************
// $Id: ossim-btoa.cpp 22209 2013-03-29 13:38:38Z dburken $

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimEbcdicToAscii.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

// Regular print with offset, hex, and ascii section.
static void print( FILE* fptr, bool isEbcdic )
{
   int           offset = 0;
   unsigned char bin_data[16];
   char          asc_data[17];
   int           hex_data[16];
   int           nbytes;
   int           i;

   asc_data[16] = '\0';
   
   ossimEbcdicToAscii* e2a = 0;
   if ( isEbcdic )
   {
      e2a = new ossimEbcdicToAscii;
   }

   while (!feof(fptr))
   {
      nbytes = (int)fread(bin_data, 16, 1, fptr);
      for(i=0; i<16; i++)
      {
         hex_data[i] = (int) bin_data[i];

         if (e2a) 
         {
            bin_data[i] = e2a->ebcdicToAscii(bin_data[i]);
         }
         
         if ((bin_data[i] >= 0x20) && (bin_data[i] < 0x7F)) // 0x7F is a DEL
         {
            asc_data[i] = bin_data[i];
         }
         else
         {
            asc_data[i] = '.';
         }
      }

      printf("%06d  %02X %02X %02X %02X %02X %02X %02X %02X  "
             "%02X %02X %02X %02X %02X %02X %02X %02X  %16s\n", offset,
             hex_data[0],  hex_data[1],  hex_data[2],  hex_data[3], 
             hex_data[4],  hex_data[5],  hex_data[6],  hex_data[7], 
             hex_data[8],  hex_data[9],  hex_data[10], hex_data[11], 
             hex_data[12], hex_data[13], hex_data[14], hex_data[15],
             asc_data);

      offset += 16;
   }

   if (e2a)
   {
      delete e2a;
      e2a = 0;
   }
   
} // End: print(...)

// Wide print offset and ascii section only.
static void printWide( FILE* fptr, bool isEbcdic )
{
   const int WIDTH = 64;
   
   int           offset = 0;
   unsigned char bin_data[WIDTH];
   char          asc_data[WIDTH+1];
   int           nbytes;
   int           i = 0;

   asc_data[WIDTH] = '\0';
   
   ossimEbcdicToAscii* e2a = 0;
   if ( isEbcdic )
   {
      e2a = new ossimEbcdicToAscii;
   }
   
   while (!feof(fptr))
   {
      nbytes = (int)fread(bin_data, WIDTH, 1, fptr);
      for( i = 0; i < WIDTH; ++i )
      {
         if (e2a) 
         {
            bin_data[i] = e2a->ebcdicToAscii(bin_data[i]);
         }
         
         if ((bin_data[i] >= 0x20) && (bin_data[i] < 0x7F)) // 0x7F is a DEL
         {
            asc_data[i] = bin_data[i];
         }
         else
         {
            asc_data[i] = '.';
         }
      }

      printf("%06d  %64s\n", offset, asc_data);

      offset += WIDTH;
   }

   if (e2a)
   {
      delete e2a;
      e2a = 0;
   }
   
} // End printWide( ... )

// Wide print ascii section only.
static void printAscii( FILE* fptr, bool isEbcdic )
{
   const int WIDTH = 80;
   
   int           offset = 0;
   unsigned char bin_data[WIDTH];
   char          asc_data[WIDTH+1];
   int           nbytes;
   int           i = 0;

   asc_data[WIDTH] = '\0';
   
   ossimEbcdicToAscii* e2a = 0;
   if ( isEbcdic )
   {
      e2a = new ossimEbcdicToAscii;
   }
   
   while (!feof(fptr))
   {
      nbytes = (int)fread(bin_data, WIDTH, 1, fptr);
      for( i = 0; i < WIDTH; ++i )
      {
         if (e2a) 
         {
            bin_data[i] = e2a->ebcdicToAscii(bin_data[i]);
         }
         
         if ((bin_data[i] >= 0x20) && (bin_data[i] < 0x7F)) // 0x7F is a DEL
         {
            asc_data[i] = bin_data[i];
         }
         else
         {
            asc_data[i] = '.';
         }
      }

      printf( "%80s\n", asc_data );

      offset += WIDTH;
   }

   if (e2a)
   {
      delete e2a;
      e2a = 0;
   }
   
} // printAscii

int main(int argc, char** argv)
{
   // Parse the args:
   ossimArgumentParser argumentParser(&argc, argv);

   enum PrintMode
   {
      ALL   = 0, // offsets hex ascii - default
      WIDE  = 1, // offsets ascii     - "-w" option
      ASCII = 2  // ascii             - "-a" option 
   };

   // Check for options:
   bool isEbcdic = false;
   PrintMode mode = ALL;

   if ( argumentParser.read("-e") )
   {
      isEbcdic = true;
   }
   if ( argumentParser.read("-w") )
   {
      mode = WIDE;
   }
   if ( argumentParser.read("-a") )
   {
      if ( mode != ALL )
      {
         std::cerr << "Cannot use -a and -w options." << std::endl;
         return -1;
      }
      mode = ASCII;
   }

   if ( argumentParser.read("-h") || argumentParser.read("--help") ||
        (argumentParser.argc() != 2) )
   {
      std::cout << "\nBinary to ascii usage: btoa [options] <binary_file>\n"
                << "Options:\n"
                << "-e Indicates text characters are ebcdic and should be"
                << " converted to ascii.\n"
                << "-h or --help usage\n"
                << "-a Ascii mode, prints 64 characters wide with no offset or hex data.\n"
                << "-w Wide mode, prints offset and 64 characters wide with no hex data.\n"
                << "Notes:\n"
                << "  1) Non-ascii characters are printed as dot in ascii output.\n"
                << "  2) Default mode prints offsets, hex and ascii data."
                << std::endl;
      return 0;
   }
   
   FILE* fptr = fopen(argv[argumentParser.argc()-1], "rb");
   if (fptr == 0)
   {
      std::cout << "\nError opening file " << argv[argumentParser.argc()-1] 
                << std::endl << std::endl;
      return -1;
   }

   if ( mode == WIDE )
   {
      printWide( fptr, isEbcdic );
   }
   else if ( mode == ASCII )
   {
      printAscii( fptr, isEbcdic );
   }
   else // default
   {
      print( fptr, isEbcdic );
   }

   fclose (fptr);
             
   return 0;
   
} // End: main( ... )

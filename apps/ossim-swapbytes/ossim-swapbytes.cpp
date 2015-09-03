//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
// Contributor: David A. Horner (http://dave.thehorners.com)
//
// Description:  Application to swapbytes.
// Reads input pixels from input_file, performs a byte swap, and outputs result
// to output_file.
//
// NOTE:
// This application assumes data is of "short" (two bytes per pixel) type.
//
//*******************************************************************
//  $Id: swapbytes.cpp 10285 2007-01-16 17:54:33Z dburken $


#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

#include <ossim/ossimConfig.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimEndian.h>


int main(int argc, char** argv)
{
   enum
   {
      OK,
      ERROR
   };
   ossimString tempString;
   ossimArgumentParser argumentParser(&argc, argv);
   ossimArgumentParser::ossimParameter stringParam(tempString);
   
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" Swaps bytes of input_file and outputs result to output_file.\
   \n\nNotes:\n\
   - Data type of a short (two bytes per pixel) is the default.\n\
   - Supported bytes_per_pixel:  2, 4, and 8");
   
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" [-h][-b <bytes_per_pixel>] input_file> <output_file>");
   
   argumentParser.getApplicationUsage()->addCommandLineOption("-b", "bytes_per_pixel");
   
   argumentParser.getApplicationUsage()->addCommandLineOption("-h", "Prints the usage");
   
   argumentParser.reportRemainingOptionsAsUnrecognized();
   
   if (argumentParser.errors())
   {
      argumentParser.writeErrorMessages(std::cout);
      exit(1);
   }

   if(argumentParser.read("-h") || argumentParser.read("--help"))
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      exit(0);
   }
   //***
   // Extract optional arguments.
   //***
   ossim_uint32 bpp = 2; // bytes per pixel
   if(argumentParser.read("-b", stringParam))
   {
      bpp = atoi(tempString);
   }

   // // Two required args:  input_file output_file
   if(argumentParser.argc() < 3)
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      exit(ERROR);
   }

   if (bpp != 2 && bpp != 4 && bpp != 8)
   {
      cerr << "swapbytes:main ERROR:"
           << "\nUnsupported bytes per pixel:  " << bpp
           << "\nSupported values are:  2, 4, and 8"
           << endl;
      exit(ERROR);
   }

   // Input stream to source file.
   ifstream is(argv[1],  ios::in | ios::binary);
   if (!is)
   {
      cerr << "swapbytes" << " ERROR:\n"
           << "Cannot open:  " << argv[1] << endl;
      return false;
   }

   // Output stream to new file.
   ofstream os(argv[2], ios::out | ios::binary);
   if (!os)
   {
      cerr << "swapbytes" << " ERROR:\n"
           << "Cannot open:  " << argv[2] << endl;
      return false;
   }

   //***
   // Get the file size of the source.
   //***
   struct stat stat_buf;
   stat(argv[1], &stat_buf);
   ossim_uint32 inputFileByteSize = stat_buf.st_size;

   ossim_uint32 totalPixelsToSwap = inputFileByteSize / bpp;
   double totalPixels = totalPixelsToSwap; // for percent complete

   cout << "\ninputFile:          " << argv[1]
        << "\noutputFile:         " << argv[2]
        << "\ninputFileByteSize:  " << inputFileByteSize
        << "\npixelsToSwap:       " << totalPixelsToSwap
        << "\nbytes per pixel:    " << bpp << endl;

   //***
   // The byte size should be evenly divisible by bytes per pixel (bpp);
   // if not, consider this an error condition and get out.
   //***
   if (inputFileByteSize % bpp)
   {
      cerr << "Error:  Byte size of file should be evenly divisible by "
           << bpp
           << ".\nExiting..." << endl;
      is.close();
      os.close();
      exit(ERROR);
   }

   const ossim_uint32 DEFAULT_SAMPLES = 512;
   
   unsigned char* lineBuff = new unsigned char[DEFAULT_SAMPLES*bpp];
   ossim_uint32 pixelsSwaped = 0;
   ossimEndian oe;
   
   while (pixelsSwaped < totalPixelsToSwap)
   {
      ossim_uint32 pixelsLeftToSwap = totalPixelsToSwap - pixelsSwaped;

      ossim_uint32 itemsToSwap = (DEFAULT_SAMPLES < pixelsLeftToSwap) ?
                           DEFAULT_SAMPLES : pixelsLeftToSwap;

      ossim_uint32 bytesToSwap = itemsToSwap * bpp;

      // read the input line.
      is.read((char*)lineBuff, bytesToSwap);
      if (!is) 
      {
         cerr << "\nERROR:  Reading image line." << endl;
         is.close();
         os.close();
         delete [] lineBuff;
         exit(ERROR);
      }

      pixelsSwaped += itemsToSwap;

      // Swap the bytes.
      switch (bpp)
      {
      case 2:
         oe.swapTwoBytes(lineBuff, itemsToSwap);
         break;
      case 4:
         oe.swapFourBytes(lineBuff, itemsToSwap);
         break;
      case 8:
         oe.swapEightBytes(lineBuff, itemsToSwap);
         break;
      }

      os.write((char*)lineBuff, bytesToSwap);
      if (!os) 
      {
         cerr << "\nERROR:  Writing image line." << endl;
         is.close();
         os.close();
         delete [] lineBuff;
         exit(ERROR);
      }      

      double pixSwapped = pixelsSwaped;
      cout << setiosflags(ios::fixed)
           << setprecision(0) << setw(3)
           << pixSwapped / totalPixels * 100 << "% \r"
           << flush;
   }

   cout << "\r100%\nFinished..." << endl;

   // Clean up...
   is.close();
   os.close();
   delete [] lineBuff;
       
   return OK;
}

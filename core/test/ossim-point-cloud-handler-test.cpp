//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// Description: Unit test application for WKT Projection Factory.
//
// $Id$
//----------------------------------------------------------------------------

#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;

#include <ossim/init/ossimInit.h>
#include <ossim/point_cloud/ossimPointCloudHandlerRegistry.h>
#include <ossim/point_cloud/ossimPointCloudHandler.h>
#include <ossim/point_cloud/ossimPointRecord.h>


int main(int argc, char* argv[])
{
   cout << "ossim-point-cloud-handler Test:" << endl;
   ossimInit::instance()->initialize(argc, argv);

   if (argc < 2)
   {
      cout  << "Missing input point-cloud data file name.\n"
            << "Usage: " << argv[0] << " <filename.las> [<output.dat>]" << endl;
      return -1;
   }
   ossimFilename fname (argv[1]);

   bool genDatFile = false;
   ofstream outStream;
   if (argc > 2)
   {
      outStream.open(argv[2]);
      genDatFile = true;
   }

   cout << "  Testing open() via factory registry... "; cout.flush();
   ossimPointCloudHandlerRegistry* factory = ossimPointCloudHandlerRegistry::instance();
   ossimPointCloudHandler* handler = factory->open(fname);
   assert(handler);
   cout << "  Passed.\n";

   cout << "  Testing getPoints()... "<<endl;
   const ossim_uint32 BLOCK_SIZE = 1024; // Read the points from the file in 1024-point blocks:
   ossim_uint32 offset=0;
   int blockNum = 0;
   ossimGpt position;
   ossimPointBlock points;
   while (1)
   {
      // Fetch block of points:
      cout << "  ... Fetching block at offset = " << offset<<endl;

      handler->getFileBlock(offset, points, BLOCK_SIZE);
      ossim_uint32 size_read = points.size();
      if (points.empty() )
         break;
      ossimPointRecord* p = points[0];

      // Sum intensity channel as "checksum" value:
      double checksum = 0;
      for (ossim_uint32 i=0; i< points.size(); ++i)
      {
         if (genDatFile)
         {
            ossim_uint32 id = points[i]->getPointId();
            position = points[i]->getPosition();
            outStream << position << endl;
         }
         checksum += points[i]->getField(ossimPointRecord::Intensity);
      }
      cout << "  Intensity checksum for block "<<blockNum <<" = " << checksum <<endl;
      ++blockNum;
   }
   cout << "  Passed."<<endl;

   if (genDatFile)
      outStream.close();

   delete handler;
   return 0;
}



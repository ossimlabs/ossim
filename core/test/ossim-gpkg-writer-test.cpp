//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// File: ossim-gpkg-writer-test.cpp
//
// Description: Test class for ossimGpkgWriter.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimGpkgWriterInterface.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimU8ImageData.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>

#include <iomanip>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
   int status = 0;
   
   ossimInit::instance()->initialize(argc, argv);

   if (argc == 2 )
   {
      ossimFilename outputFile = argv[1];
      
      ossimString writerClass = "ossimGpkgWriter";
      ossimRefPtr<ossimImageFileWriter> writer =
         ossimImageWriterFactoryRegistry::instance()->createWriter( writerClass );

      if ( writer.valid() )
      {
         ossimGpkgWriterInterface* gpkgWriter =
            dynamic_cast<ossimGpkgWriterInterface*>( writer.get() );

         if ( gpkgWriter )
         {
            ossimKeywordlist kwl;
            std::string key;
            std::string value;
            
            key = "zoom_levels";
            // value = "(0,1,2,3,4,5,6,7,8,9,10,11)";
            value = "(2,3,4,5)";
            kwl.addPair( key, value );
            
            key = "epsg";
            value = "3857";
            kwl.addPair( key, value );
            
            key = "filename";
            value = outputFile.string();
            kwl.addPair( key, value );

            if ( gpkgWriter->openFile( kwl ) )
            {
               gpkgWriter->beginTileProcessing();
               
               // Write some tiles:
               ossimRefPtr<ossimImageData> id = new ossimU8ImageData(0, 3, 256, 256);
               id->initialize();
               id->createTestTile();

               // Valid tile:
               if ( gpkgWriter->writeTile( id, 2, 0, 0 ) ) // tile, level, row, col
               {
                  cout << "writeTile id, 2, 0, 0 success..." << endl;
               }
               else
               {
                  cout << "writeTile id, 2, 0, 0 failed..." << endl;
               }
               
               if ( gpkgWriter->writeTile( id, 2, 0, 1 ) ) // tile, level, row, col
               {
                  cout << "writeTile id, 2, 0, 1 success..." << endl;
               }
               else
               {
                  cout << "writeTile id, 2, 0, 1 failed..." << endl;
               }
               
               if ( gpkgWriter->writeTile( id, 2, 1, 0 ) ) // tile, level, row, col
               {
                  cout << "writeTile id, 2, 1, 0 success..." << endl;
               }
               else
               {
                  cout << "writeTile id, 2, 1, 0 failed..." << endl;
               }

               if ( gpkgWriter->writeTile( id, 2, 1, 1 ) ) // tile, level, row, col
               {
                  cout << "writeTile id, 2, 1, 1 success..." << endl;
               }
               else
               {
                  cout << "writeTile id, 2, 1, 1 failed..." << endl;
               }

               if ( gpkgWriter->writeTile( id, 3, 0, 0 ) ) // tile, level, row, col
               {
                  cout << "writeTile id, 3, 0, 0 success..." << endl;
               }
               else
               {
                  cout << "writeTile id, 3, 0, 0 failed..." << endl;
               }
               
               if ( gpkgWriter->writeTile( id, 4, 0, 0 ) ) // tile, level, row, col
               {
                  cout << "writeTile id, 4, 0, 0 success..." << endl;
               }
               else
               {
                  cout << "writeTile id, 4, 0, 0 failed..." << endl;
               }

               if ( gpkgWriter->writeTile( id, 5, 0, 0 ) ) // tile, level, row, col
               {
                  cout << "writeTile id, 5, 0, 0 success..." << endl;
               }
               else
               {
                  cout << "writeTile id, 5, 0, 0 failed..." << endl;
               }
               // Invalid level
               if ( gpkgWriter->writeTile( id, 20, 0, 0 ) ) // tile, level, row, col
               {
                  cout << "writeTile id, 20, 0, 0 success..." << endl;
               }
               else
               {
                  cout << "writeTile id, 20, 0, 0 failed..." << endl;
               }

               // Invalid row
               if ( gpkgWriter->writeTile( id, 4, 10000, 0 ) ) // tile, level, row, col
               {
                  cout << "writeTile id, 4, 10000, 0success..." << endl;
               }
               else
               {
                  cout << "writeTile id, 4, 10000, 0 failed..." << endl;
               }

               // Invalid col
               if ( gpkgWriter->writeTile( id, 4, 0, 10000 ) ) // tile, level, row, col
               {
                  cout << "writeTile id, 4, 0, 10000 success..." << endl;
               }
               else
               {
                  cout << "writeTile id, 4, 0, 10000 failed..." << endl;
               }

               gpkgWriter->finalizeTileProcessing();

               // Close this writer and test append mode:
               writer->close();
               gpkgWriter = 0;
               writer = 0; // ref ptr

               cout << "Testing append mode..." << endl;
               
               writer = ossimImageWriterFactoryRegistry::instance()->createWriter( writerClass );
               if ( writer.valid() )
               {
                  ossimGpkgWriterInterface* gpkgWriter =
                     dynamic_cast<ossimGpkgWriterInterface*>( writer.get() );
                  
                  if ( gpkgWriter )
                  {
                     // Add append to options:
                     key = "append";
                     value = "true";
                     kwl.addPair( key, value );

                     if ( gpkgWriter->openFile( kwl ) )
                     {
                        gpkgWriter->beginTileProcessing();
                        
                        if ( gpkgWriter->writeTile( id, 3, 0, 1 ) ) // tile, level, row, col
                        {
                           cout << "writeTile id, 3, 0, 1 success..." << endl;
                        }
                        else
                        {
                           cout << "writeTile id, 3, 0, 1 failed..." << endl;
                        }
                     }

                     gpkgWriter->finalizeTileProcessing();
                     
                     writer->close();
                     gpkgWriter = 0;
                  }
                  writer = 0;
               }
            }
         }
         else
         {
            cerr << "Could not cast!" << endl;
         }
         
         writer = 0;
      }
      else // Matches: 
      {
         cerr << "ossimImageWriterFactoryRegistry::instance()->createWriter( ossimGpkgWriter ) failed!" << endl;
      }
   }
   else // Matches: if (argc == 2 ){ ... }
   {
      cout << argv[0] << " <output_file>"
           << "\n\nGeoPackage writer test code.\n" << endl;
   }

   return status;
}

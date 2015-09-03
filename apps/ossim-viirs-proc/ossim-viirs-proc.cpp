//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// File: ossim-viirs-proc
//
// Description: Application to extract VIIRS Radiance layer, shifting
// min/max linearly to 16 bit space with coarse grid geometry file.
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimMetadataInterface.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/imaging/ossimCastTileSourceFilter.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimShiftFilter.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimCoarseGridModel.h>
#include <ossim/projection/ossimProjection.h>

#include <iomanip>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
   int status = 0;

   // Timer for elapsed time:
   ossimTimer::instance()->setStartTick();
   
   ossimInit::instance()->initialize(argc, argv);

   if (argc == 2 )
   {
      ossimFilename inputFile = argv[1];
      if ( inputFile.exists() )
      {
         ossimFilename outputFile = inputFile.expand().noExtension();
         outputFile.string() += "_1.tif";
         
         if ( inputFile != outputFile )
         {
            std::string command =
               "ossim-cmm --null -9999.0 --reader-prop layer=/All_Data/VIIRS-DNB-SDR_All/Radiance ";
            command += inputFile.string();
            cout << "Executing command: " << command << endl;

            status = system( command.c_str() );

            if ( status == 0 )
            {
               ossimRefPtr<ossimSingleImageChain> sic = new ossimSingleImageChain();
               if ( sic->open( inputFile, false ) )
               {
                  cout << "Opened: " << inputFile << endl;

                  // Set the entry to the Radiance layer.
                  ossimRefPtr<ossimProperty> readerProp = new  ossimStringProperty(
                     ossimString("layer"),
                     ossimString("/All_Data/VIIRS-DNB-SDR_All/Radiance") );

                  sic->getImageHandler()->setProperty( readerProp );

                  // Add filter to shift/stretch data uint16 range.
                  ossimRefPtr<ossimShiftFilter> sf = new ossimShiftFilter();
                  sf->setNullPixelValue( 0.0 );
                  sf->setMinPixelValue( 1.0 );
                  sf->setMaxPixelValue( 65535.0 );
                  sic->addFirst( sf.get() );

                  // Cast it to uint16:
                  ossimRefPtr<ossimCastTileSourceFilter> cf =
                     new ossimCastTileSourceFilter(0, OSSIM_UINT16 );
                  sic->addFirst( cf.get() );

                  // Put a cache in front of resampler.
                  sic->addCache();
                  
                  // sic->addResampler();
                  
                  sic->initialize();
                  
                  // Write image:
                  ossimRefPtr<ossimImageFileWriter> writer = new ossimTiffWriter();
                  if ( writer->open( outputFile ) )
                  {
                     // Turn on overviews and histograms:
                     writer->setWriteOverviewFlag( true );
                     writer->setWriteHistogramFlag( true );

                     // Add a listener to get percent complete.
                     ossimStdOutProgress prog(0, true);
                     writer->addListener(&prog);
                     
                     writer->connectMyInputTo(0, sic.get());

                     ossimIrect rect = writer->getAreaOfInterest();
                     
                     writer->execute();

                     cout << "Wrote file: " << outputFile << endl;
                     
                     ossimRefPtr<ossimImageHandler> ih = sic->getImageHandler();

                     if ( ih.valid() )
                     {
                        //---
                        // If there is a coarse grid model write it out so the
                        // oqutput file will pick it up.
                        //---
                        ossimRefPtr<ossimImageGeometry> geom = sic->getImageHandler()->
                           getImageGeometry();
                        if ( geom.valid() )
                        {
                           ossimRefPtr<ossimProjection> proj = geom->getProjection();
                           if ( proj.valid() )
                           {
                              ossimFilename geomFile = outputFile.noExtension();
                              geomFile.string() += ".geom";

                              ossimRefPtr<ossimCoarseGridModel> cg =
                                 dynamic_cast<ossimCoarseGridModel*>( proj.get() );
                              if ( cg.valid() )
                              {
                                 // this saves geom file as well
                                 cg->saveCoarseGrid( geomFile );
                                 cout << "Wrote file: " << geomFile << endl;
                              }
                              else
                              {
                                 // Save the state to keyword list.
                                 ossimKeywordlist geomKwl;
                                 geom->saveState(geomKwl);

                                 // Write to file:
                                 geomKwl.write( geomFile.c_str() );
                                 cout << "Wrote file: " << geomFile << endl;
                              }
                           }
                        }

                        //---
                        // If the image handler is a metadata interface pass the metadata
                        // to the dot.omd file and rewrite it.
                        //---
                        ossimMetadataInterface* mdi =
                           dynamic_cast<ossimMetadataInterface*>( ih.get() );
                        if ( mdi )
                        {
                           ossimFilename omdFile = outputFile.noExtension();
                           omdFile += ".omd";
                           ossimRefPtr<ossimKeywordlist> kwl = new ossimKeywordlist();
                           if ( omdFile.exists() )
                           {
                              kwl->addFile( omdFile );
                           }
                           
                           std::string prefix = "";
                           mdi->addMetadata( kwl.get(), prefix );
                           kwl->write( omdFile.c_str() );
                           cout << "Wrote file: " << omdFile << endl;
                        }
                     }
                  }
                  else
                  {
                     cerr << "Could not open: " << outputFile << endl;
                  }
               }
            }
         } 
         else // Matches: 
         {
            cerr << "Input file is same as output file!  Returning..." << endl;
            status = -1;
         }
      }
      else // Match: if ( inputFile.exists() ){ ... }
      {
         cerr << "Input file does not exists!" << endl;
         status = -1;
      }
   }
   else // Matches: if (argc == 2 ){ ... }
   {
      cout << argv[0] << " <image_file>"
           << "\n\nExtracts VIIRS Radiance layer from hdf5 file, shifting min/max "
           << "linearly to 16 bit space with coarse grid geometry file.\n" << endl;
   }

   cout << "Elapsed time in seconds: "
        << std::setiosflags(ios::fixed)
        << std::setprecision(3)
        << ossimTimer::instance()->time_s() << "\n";

   return status;
}

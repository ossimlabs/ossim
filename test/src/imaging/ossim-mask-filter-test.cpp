//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// File: ossim-mask-filter-test.cpp
//
// Author:  David Burken
//
// Description: Test application for ossimMaskFilter.
// 
//----------------------------------------------------------------------------
// $Id: ossim-mask-filter-test.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <iostream>
using namespace std;

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/base/ossimViewInterface.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimMaskFilter.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/init/ossimInit.h>

int main(int argc, char* argv[])
{
   ossimTimer::instance()->setStartTick();
   
   ossimInit::instance()->initialize(argc, argv);

   if (argc != 6)
   {
      cout << argv[0] << " <mask_type> <resample_flag> <image_file> <shape_file> <output_file>"
           << "\nCombines image and shape files with mask filter and writes "
           << "result to output file.\n"
           << "valid mask_types:\n"
           << "1 = OSSIM_MASK_TYPE_SELECT\n"
           << "2 = OSSIM_MASK_TYPE_INVERT\n"
           << "3 = OSSIM_MASK_TYPE_WEIGHTED\n"
           << "4 = OSSIM_MASK_TYPE_BINARY\n"
           << "5 = OSSIM_MASK_TYPE_BINARY_INVERSE\n"
           << "resample_flag if true the output will be rendered to a geographic projection.\n"
           << "output_file is a mask of image and shape.\n"
           << endl;
      return 0;
   }


   int i = atoi(argv[1]);
   if ( (i < 1) || (i > 5) )
   {
      cerr << "mask type out of range!" << endl;
      return 1;
   }
   ossimMaskFilter::ossimFileSelectionMaskType mask_type =
      static_cast<ossimMaskFilter::ossimFileSelectionMaskType>(i);
   i = atoi(argv[2]);
   bool resample_flag = (i?true:false);
   ossimFilename inputImgName = ossimFilename(argv[3]);
   ossimFilename inputShpName = ossimFilename(argv[4]);
   ossimFilename outputFile  = ossimFilename(argv[5]);

   cout << "ossim-mask-filter-test:"
        << "\nmask_type:     " << mask_type
        << "\nresample_flag: " << resample_flag
        << "\ninputImgName:  " << inputImgName
        << "\ninputShpName:  " << inputShpName
        << "\noutputFile:    " << outputFile
        << endl;

   //---
   // Test masking shape and image:
   //---
   ossimRefPtr<ossimImageSource> inputImg = 0;
   if ( !resample_flag )
   {
      inputImg = ossimImageHandlerRegistry::instance()->open(inputImgName);
   }
   else
   {
      ossimRefPtr<ossimSingleImageChain> sic = new ossimSingleImageChain();
      if ( sic->open( inputImgName ) )
      {
         // Render the image chain.
         sic->createRenderedChain();
         inputImg = sic.get();
      }
      else
      {
         sic = 0;
      }
   }
   if ( inputImg.valid() == false )
   {
      cout << "Could not open: " << inputImgName  << endl;
      return 1;
   }
         
   ossimRefPtr<ossimImageHandler> inputShp =
      ossimImageHandlerRegistry::instance()->open(inputShpName);
   if ( inputShp.valid() )
   {
      if ( inputShp->getClassName() == "ossimOgrGdalTileSource" )
      {
         ossimViewInterface* shpView = PTR_CAST(ossimViewInterface, inputShp.get());
         if (shpView)
         {
            //---
            // Test masking image handler and shape file.
            //---
            
            // Set the shape reader's view to that of the image's.
            shpView->setView(inputImg->getImageGeometry().get());
            
            // Turn fill on...
            ossimRefPtr<ossimProperty> fillProp =
               new ossimStringProperty(ossimString("fill_flag"),
                                       ossimString("1"));
            inputShp->setProperty(fillProp);
            
            
            ossimRefPtr<ossimMaskFilter> maskFlt = new ossimMaskFilter();
            maskFlt->setMaskType(mask_type);
            maskFlt->connectMyInputTo( 0, inputImg.get() );
            // maskFlt->connectMyInputTo(1, inputShp.get());
            maskFlt->setMaskSource( inputShp.get() );
            maskFlt->initialize();
            
            ossimRefPtr<ossimImageFileWriter> writer =
               ossimImageWriterFactoryRegistry::instance()->
               createWriterFromExtension(outputFile.ext());
            
            if ( writer->open( outputFile ) )
            {
               // Add a listener to get percent complete.
               ossimStdOutProgress prog(0, true);
               writer->addListener(&prog);
               
               writer->connectMyInputTo(0, maskFlt.get());
               
               // Set the cut rect to the input image.
               writer->setAreaOfInterest(inputImg->getBoundingRect());
               
               writer->execute();
               
               cout << "Wrote file: " << outputFile
                    << "\nElapsed time(seconds): "
                    << ossimTimer::instance()->time_s() << "\n";
            }
            else
            {
               cout << "Could not open: " << argv[3] << endl;
            }
         }
         else
         {
            cerr << "Could not get view interface..." << endl;
         }
      }
      else
      {
         cerr << "2nd argument must be a shape file." << endl;
      }
   }
   else
   {
      cerr << "Could not open: " << inputShpName << endl;
   }

   return 0;
}

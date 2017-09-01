//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-mda-test.cpp
//
// Author:  Oscar Kramer
//
// Tests MDA format reading and writing. Given an original MDA file, a chain is created
// with an MDA reader on the left end and an MDA writer on the right with a simple sequencer 
// in between. An output file is written to disk then read back in with a new handler instance.
// The test is successful only if both handlers are equal (minus the filename).
// 
// $Id$
//----------------------------------------------------------------------------

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/init/ossimInit.h>

// Put your includes here:
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimImageSourceSequencer.h>
#include <ossim/base/ossimKeywordNames.h>

#include <iostream>
#include <sstream>
using namespace std;

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   try
   {
      if (argc < 2)
         ossimException ("Need an input original image filename. Cannot continue.");

      ossimFilename orig_fname (argv[1]);
      if (!orig_fname.isReadable())
         ossimException ("Filename specified is not readable. Cannot continue.");

      // Open the original input file using factory (includes plugins)
      ossimRefPtr<ossimImageHandler> handler1 = ossimImageHandlerRegistry::instance()->open(orig_fname);
      if (!handler1.valid())
         ossimException ("Could not open input file. Cannot continue.");

      handler1->print(cout); //###

      // Fetch image parameters needed for write:
      ossimIrect image_rect (handler1->getBoundingRect());
      // int num_rlevels = handler1->getNumberOfDecimationLevels();

      // Establish a sequencer to loop through all tiles:
      ossimRefPtr<ossimImageSourceSequencer> sequencer = new ossimImageSourceSequencer;
      sequencer->connectMyInputTo(handler1.get());

      // Establish an output filename based on the input name:
      ossimFilename output_fname (orig_fname.noExtension());
      output_fname += "-RWTestOut";
      output_fname.setExtension(orig_fname.ext());

      // Establish the associated writer object using factory scheme. This should pick up the 
      // corresponding writer but no guaranties:
      ossimRefPtr<ossimImageFileWriter> writer = 
         ossimImageWriterFactoryRegistry::instance()->createWriterFromExtension(output_fname.ext());
      if (!writer.valid())
         ossimException ("Could not create a writer object for this image file extension. Cannot continue.");

      // Connect the writer to the chain and write the file:
      writer->setOutputName(output_fname);
      writer->connectMyInputTo(sequencer.get());
      bool write_successful = writer->execute();
      if (!write_successful)
         ossimException ("Error encountered writing output image. Cannot continue.");

      // Use the new output file to initialize a new handler to compare against the original handler
      ossimRefPtr<ossimImageHandler> handler2 = 
         ossimImageHandlerRegistry::instance()->open(output_fname);
      if (!handler2.valid())
         ossimException ("Could not open test output file for comparison. Test failed.");

      // Save the states to KWL -- this is what will be compared. Need to remove the filename
      // keyword however since that differs:
      ossimKeywordlist kwl1;
      handler1->saveState(kwl1);
      kwl1.removeKeysThatMatch(ossimKeywordNames::FILENAME_KW);
      
      ossimKeywordlist kwl2;
      handler2->saveState(kwl2);
      kwl2.removeKeysThatMatch(ossimKeywordNames::FILENAME_KW);

      if (kwl1 == kwl2)
      {
         cout << "OSSIM successfully read and wrote image format associated with file <"
            << orig_fname << ">. Test PASSED." << endl;
      }
      else
      {
         ossimException ("Differences found between reader and writer. Test FAILED.");
      }
   }
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      return 1;
   }
   
   return 0;
}

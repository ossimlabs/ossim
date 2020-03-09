//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Test code for generic ossim test.
//
// $Id: ossim-test.cpp 19751 2011-06-13 15:13:07Z dburken $
//----------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
using namespace std;

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/init/ossimInit.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>


int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   ossimFilename inFile (argv[1]);
   ossimFilename outFile (argv[2]);

   auto handler = ossimImageHandlerRegistry::instance()->open(inFile);
   if (!handler)
      throw(runtime_error("Unable to create handler."));

   // Create writer:
   auto writer = ossimImageWriterFactoryRegistry::instance()->createWriter(outFile);
   if (!writer)
      throw(runtime_error("Unable to create writer."));
   writer->connectMyInputTo(handler);

   writer->setWriteExternalGeometryFlag(false);
   //writer->setTileSize(ossimIpt(egressor->getTileWidth(), egressor->getTileHeight()));
   //writer->setOutputImageType("tiff_tiled");
   //writer->setPixelType(OSSIM_PIXEL_IS_POINT);
   writer->initialize();
   if (writer->getErrorStatus() != ossimErrorCodes::OSSIM_OK)
      throw(runtime_error("Unable to initialize writer for execution"));

   // OSSIM writer sequences through the output tiles and writes the output image complete
   // with metadata:
   if (!writer->execute())
      throw(runtime_error("Error encountered writing TIFF."));
   writer->close();
   cout << "\nWrote " << outFile << endl;

   return 0;
}

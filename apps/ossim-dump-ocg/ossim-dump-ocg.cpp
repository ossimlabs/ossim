//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Oscar Kramer
//
// Description:
//
// Contains application "dumpocg" for outputing an OSSIM coarse grid data
// file in human-friendly format.
//
//*******************************************************************
//  $Id: dumpocg.cpp 10253 2007-01-14 17:52:22Z dburken $

#include <cstdlib> /* for exit */

#include <ossim/base/ossimDblGrid.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimCoarseGridModel.h>

int main(int argc, char** argv)
{
   ossimInit::instance()->initialize(argc, argv);

   if (argc < 2)
   {
      cout << "USAGE: "<<argv[0]<<" <OCG geom file>" << endl;
      exit(-1);
   }
   
   ossimFilename geom_file(argv[1]);
   ossimCoarseGridModel model (geom_file);
   if (model.getErrorStatus() != ossimErrorCodes::OSSIM_OK)
   {
      cout << "Error encountered instantiating the OCG object given geometry "
           << "file name: " << geom_file << ". Check the name and try again."
           << endl;
      exit(-1);
   }

   model.print(std::cout);
   
   exit(0);
}

//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Test application for ossimPiecewiseRemapper class.
// 
//----------------------------------------------------------------------------
// $Id: ossim-single-image-chain-test.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimTimer.h>
#include <ossim/imaging/ossimLinearStretchRemapper.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/init/ossimInit.h>
#include <iomanip>
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
   ossimTimer::instance()->setStartTick();
   
   ossimInit::instance()->initialize(argc, argv);

   ossimTimer::Timer_t t1 = ossimTimer::instance()->tick();
   
   cout << "elapsed time after initialize(ms): "
        << std::setiosflags(ios::fixed)
        << std::setprecision(3)
        << ossimTimer::instance()->time_s() << "\n";

   if (argc < 5)
   {
      cout <<"\n"<< argv[0] << " <min> <max> <infile> <outfile>"
           << "\nPerforms a linear stretch on <infile> and writes the result to <outfile>. The input"
           " image is stretched between <min> and <max> provided.\n"<< endl;
      return 0;
   }

   ossimFilename infile = argv[3];
   ossimFilename outfile = argv[4];
   double min = atof(argv[1]);
   double max = atof(argv[2]);

   ossimRefPtr<ossimSingleImageChain> sic1 = new ossimSingleImageChain();
   if ( !sic1->open( infile ) )
   {
      cout<<"Error opening input file <"<<infile<<">"<<endl;
      return 1;
   }

   cout << "Opened: " << infile << endl;

   ossimRefPtr<ossimLinearStretchRemapper> remapper = new ossimLinearStretchRemapper();
   remapper->setMinPixelValue(min, 0);
   remapper->setMaxPixelValue(max, 0);
   sic1->addFirst( remapper.get() );
   sic1->initialize();

   // Write image:
   ossimRefPtr<ossimImageFileWriter> writer = new ossimTiffWriter();
   if ( !writer->open( outfile ) )
   {
      cout<<"Error opening output file <"<<outfile<<">"<<endl;
      return 1;
   }

   cout << "Outputting file: " << outfile << endl;

   // Add a listener to get percent complete.
   ossimStdOutProgress prog(0, true);
   writer->addListener(&prog);

   writer->connectMyInputTo(0, sic1.get());
   writer->execute();
   ossimTimer::Timer_t t2 = ossimTimer::instance()->tick();
   cout << "elapsed time after write(ms): "
         << std::setiosflags(ios::fixed)
   << std::setprecision(3)
   << ossimTimer::instance()->time_s() << "\n";

   cout << "write time minus initialize: "
         << std::setiosflags(ios::fixed)
         << std::setprecision(3)
         << ossimTimer::instance()->delta_s(t1, t2) << "\n";

   return 0;
}


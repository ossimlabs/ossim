//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// Description:
//
// Command line application "ossim-histo-compare" to evaluate differences between histograms.
//
//----------------------------------------------------------------------------
// $Id: ossim-histo-compare.cpp 19751 2011-06-13 15:13:07Z dburken $

#include <ossim/ossimConfig.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimMultiResLevelHistogram.h>
#include <cstdio>
#include <cstdlib> /* for exit */

//*************************************************************************************************
// FINALIZE -- Convenient location for placing debug breakpoint for catching program exit.
//*************************************************************************************************
void finalize(int code)
{
   exit (code);
}

//*************************************************************************************************
// MAIN
//*************************************************************************************************
int main(int argc, char* argv[])
{
   ossimString tempString;
   double tempDouble, tempDouble2;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser::ossimParameter doubleParam(tempDouble);
   ossimArgumentParser::ossimParameter doubleParam2(tempDouble2);

   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+
      " compares two ossim histogram files and presents statistics about differences. The return "
      "value is 0 if histograms are within difference tolerances and 1 if not.");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+
      " [options] <path_to/image1.his> <path_to/image2.his>");
   argumentParser.getApplicationUsage()->addCommandLineOption("--tolerance | -t <max_diff_percent>", 
      "Tolerance (0.0 - 100.0) as percent of difference to total bin population. If any bin's "
      "count difference is found to exceed <max_diff_percent>, the comparison will fail.");
   argumentParser.getApplicationUsage()->addCommandLineOption("--max-sigma | -s <sigma>",
      "Tolerance in standard deviation from 0 mean difference. If the mean RSS of all"
      "differences is greater that <sigma>, the comparison will fail.");

   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help", 
      "Shows help");

   if(argumentParser.read("-h") || argumentParser.read("--help"))
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      finalize(0);
   }

   if ( argumentParser.read("--version") || argumentParser.read("-v"))
   {
      ossimNotify(ossimNotifyLevel_NOTICE)<< argumentParser.getApplicationName().c_str() << " " 
         << ossimInit::instance()->instance()->version().c_str()<< std::endl;
      finalize(0);
   }

   // Fetch command line options:
   double tolerance = .5; // % default (= 0.005)
   if (argumentParser.read("--tolerance", doubleParam) || argumentParser.read("-t", doubleParam))
      tolerance = tempDouble/100.0;

   double max_sigma = 0.05;  
   if (argumentParser.read("--max-sigma", doubleParam) || argumentParser.read("-s", doubleParam))
      max_sigma = tempDouble;

   // Handle bad command line:
   argumentParser.reportRemainingOptionsAsUnrecognized();
   if (argumentParser.errors())
   {
      argumentParser.writeErrorMessages(std::cout);
      finalize(1);
   }
   if(argumentParser.argc() < 3)
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      finalize(1);
   }

   // Establish the two histogram objects from files:
   ossimFilename histofile1(argv[1]);
   ossimRefPtr<ossimMultiResLevelHistogram> mrl_histo1 = new ossimMultiResLevelHistogram;
   bool success = mrl_histo1->importHistogram(histofile1);
   if (!success)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<< argumentParser.getApplicationName().c_str() 
         << ": Error encountered establishing histogram from <"<<histofile1<<">. Failed."<<endl;
      finalize(1);
   }

   ossimFilename histofile2(argv[2]);
   ossimRefPtr<ossimMultiResLevelHistogram> mrl_histo2 = new ossimMultiResLevelHistogram;
   success = mrl_histo2->importHistogram(histofile2);
   if (!success)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<< argumentParser.getApplicationName().c_str() 
         << ": Error encountered establishing histogram from <"<<histofile2<<">. FAILED."<<endl;
      finalize(1);
   }

   // Check that number of R-levels and bands match:
   ossim_uint32 nlevels1 = mrl_histo1->getNumberOfResLevels();
   ossim_uint32 nlevels2 = mrl_histo2->getNumberOfResLevels();
   if (nlevels1 != nlevels2)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<< argumentParser.getApplicationName().c_str()
         << ": Number of resolution levels does not match. FAILED."<<endl;
      finalize(1);
   }
   ossim_uint32 nbands1  = mrl_histo1->getNumberOfBands();
   ossim_uint32 nbands2  = mrl_histo2->getNumberOfBands();
   if (nbands1 != nbands2)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<< argumentParser.getApplicationName().c_str() 
         << ": Number of bands does not match. FAILED."<<endl;
      finalize(1);
   }

   // Loop over each R-level and band:
   bool failed = false;
   for (ossim_uint32 rlevel=0; rlevel<nlevels1; ++rlevel)
   {
      for (ossim_uint32 band=0; band<nbands1; ++band)
      {
         ossimNotify(ossimNotifyLevel_FATAL)<< "\n  Comparing R-level: "<<rlevel<<",  Band: "<<band
            <<endl;

         // Establish the histogram objects for this R-level and band:
         const ossimRefPtr<ossimHistogram> histo1 = mrl_histo1->getHistogram(band, rlevel);
         const ossimRefPtr<ossimHistogram> histo2 = mrl_histo2->getHistogram(band, rlevel);

         float min_val = histo1->GetMinVal();
         float max_val = histo1->GetMaxVal();
         float del_val = histo1->GetBucketSize();
         std::vector<double> norm_diffs;
         ossim_uint32 diff, count1, count2;
         double norm_diff;

         // Loop over all bins to collect all differences:
         for (float val=min_val; val<=max_val; val+=del_val)
         {
            count1 = (ossim_uint32) histo1->GetCount(val);
            count2 = (ossim_uint32) histo2->GetCount(val);
            diff = abs((int)(count1 - count2));
 
            if (count1 != 0)
               norm_diff = (double) diff / count1;
            else if (count2 != 0)
               norm_diff = (double) diff / count2;
            else
               norm_diff = 0.0;

            norm_diffs.push_back(100.0 * norm_diff);
         }

         // Check trivial case:
         if (norm_diffs.size() == 0)
         {
            ossimNotify(ossimNotifyLevel_FATAL)<< argumentParser.getApplicationName().c_str() 
               << ": No samples found!. FAILED."<< endl;
            finalize(1);
         }

         // Compute statistics on differences:
         double accum=0;
         std::vector<double>::iterator i = norm_diffs.begin();
         while (i != norm_diffs.end())
         {
            accum += *i;
            ++i;
         }
         double mean = accum / (double) norm_diffs.size();
         accum=0;
         i = norm_diffs.begin();
         while (i != norm_diffs.end())
         {
            accum += (*i - mean) * (*i - mean);
            ++i;
         }
         double sigma = sqrt(accum) / (double) norm_diffs.size();

         // Output mean and sigma for this histo comparison:
         ossimNotify(ossimNotifyLevel_FATAL)<<"    Mean normalized difference: "<<mean<<" +- "<<sigma<<endl;

         // Do tolerance test:
         if (mean > tolerance)
         {
            ossimNotify(ossimNotifyLevel_FATAL)<<"    Found mean difference of "<<mean
               <<"%. FAILED tolerance test."<<endl;
            failed = true;
         }
         if (sigma > max_sigma)
         {
            ossimNotify(ossimNotifyLevel_FATAL)<<"    Computed sigma of "<<sigma
               <<". FAILED sigma test."<<endl;
            failed = true;
         }
      }
   }

   // Return with pass/fail code:
   if (!failed)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<< "\n  PASSED"<<endl;
      finalize(0);
   }
   ossimNotify(ossimNotifyLevel_FATAL)<< "\n  FAILED"<<endl;
   finalize(1);
}


//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Description: Functional test for ossimRangeDomeTileSource class.
//
// $Id: ossim-range-dome-test.cpp 23468 2015-08-19 01:51:42Z dburken $
//----------------------------------------------------------------------------

#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/imaging/ossimRangeDomeTileSource.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <iostream>
#include <cstdlib> /* for exit */

using namespace std;

const char* data = "OSSIM_RANGE_DOMES 1.0\n"
      "# ID, Lat, Lon, Hgt, Radius, Classification [, Az_start, Az_end] [, \"Description\"]\n"
      "1, 25.8607, -80.1343, 0, 20, 1\n"
      "2, 25.8601, -80.1336, 0, 40, 3, 135, 45, NATO friendly dome\n"
      "3, 25.8605, -80.1343, 0, 40, 102, 135, 225, 50mm battery threat dome\n"
      "4, 25.8601, -80.1335, 0, 50, 103, 45, 135\n"
      "5, 25.8605, -80.1336, 0, 50, 104, 340, 20\n";

const char* lut =
      "type: ossimIndexToRgbLutFilter \n"
      "mode: literal \n"
      "entry0.index: 1 \n"
      "entry0.color: 48 48 255\n"
      "entry1.index: 2 \n"
      "entry1.color: 32 32 255 \n"
      "entry2.index: 3 \n"
      "entry2.color: 16 16 255 \n"
      "entry3.index: 4 \n"
      "entry3.color: 1 1 255 \n"
      "entry0.index: 101 \n"
      "entry0.color: 255 32 32 \n"
      "entry4.index: 102 \n"
      "entry4.color: 255 128 1 \n"
      "entry5.index: 103 \n"
      "entry5.color: 255 64 1 \n"
      "entry6.index: 104 \n"
      "entry6.color: 255 1 1 \n";

const char* TEMP_CSV_FILE = "rdtltest-default.csv";
const char* TEMP_LUT_FILE = "rdtltest-default.lut";
const char* DEFAULT_RESULTS_FILE = "rdtltest-RESULT.tif";

class Executive
{
public:
   Executive(ossimArgumentParser& ap);
   ~Executive();
   bool run();

private:
   ossimFilename m_csvFile;
   ossimFilename m_lutFile;
   ossimFilename m_resultsFile;
   double m_gsd;
   bool m_saveFiles;
};

Executive::Executive(ossimArgumentParser& ap)
:  m_gsd (1.0),
   m_saveFiles (false)
{
   // Set the general usage:
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " [options]";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->addCommandLineOption(
         "--domes <filename>",
         "Specifies the input range-dome specifications CSV file name.");
   au->addCommandLineOption(
         "--gsd <meters>",
         "Specifies output GSD in meters. Defaults to 1 meter. ");
   au->addCommandLineOption(
         "--lut <filename>",
         "Causes single-band, output range-dome raster to be remapped to the RGB LUT specified in "
         "<filename>. Implies \"--use-lut\" option.");
   au->addCommandLineOption(
         "--out <filename>",
         "Specifies output filename. Defaults to <rdtltest-OUT.tif>.");
   au->addCommandLineOption(
         "--save",
         "Prevents temporary domes CSV file and LUT file from being deleted after test so they "
         "can be used as examples for custom inputs.");
   au->addCommandLineOption(
         "--use-lut",
         "Causes single-band, output range-dome raster to be remapped to default "
         "range-dome RGB LUT.");

   if (ap.read("-h") || ap.read("--help"))
   {
      ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      return;
   }

   std::string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);
   m_csvFile = TEMP_CSV_FILE;
   m_resultsFile = DEFAULT_RESULTS_FILE;
   m_lutFile.clear();

   if (ap.read("--gsd", sp1))
      m_gsd = ossimString(ts1).toDouble();

   if (ap.read("--domes", sp1))
      m_csvFile = ts1;

   if (ap.read("--lut", sp1))
      m_lutFile = ts1;

   if (ap.read("--out", sp1))
      m_resultsFile = ts1;

   if (ap.read("--save"))
      m_saveFiles = true;

   if (ap.read("--use-lut"))
      m_lutFile = TEMP_LUT_FILE;
}

Executive::~Executive()
{
   if (!m_saveFiles)
   {
      if (m_csvFile == TEMP_CSV_FILE)
         m_csvFile.remove();

      if (m_lutFile == TEMP_LUT_FILE)
         m_lutFile.remove();
   }
}

bool Executive::run()
{
   if (m_resultsFile.empty())
      return false;

   // Output temp datafile if none provided:
   if (m_csvFile == TEMP_CSV_FILE)
   {
      ofstream ofs (m_csvFile.string().c_str(), ios_base::trunc);
      if (ofs.fail())
      {
         ossimNotify(ossimNotifyLevel_FATAL)
               << "Could not write out temporary CSV file to CWD. Check permissions." << endl;
         return false;
      }
      ofs << data << endl;
      ofs.close();
   }

   // Use factory to open the RDTS:
   ossimRefPtr<ossimImageHandler> handler = ossimImageHandlerRegistry::instance()->open(m_csvFile);
   if (!handler.valid())
   {
      ossimNotify(ossimNotifyLevel_FATAL)
            << "Error encountered instantiating threat dome handler from CSV." << std::endl;
      return false;
   }

   // Set up the range-dome filter chain:
   ossimRangeDomeTileSource* rdts =  dynamic_cast<ossimRangeDomeTileSource*>(handler.get());
   if (!rdts)
   {
      ossimNotify(ossimNotifyLevel_FATAL)
            << "Error encountered casting handler to ossimRangeDomeTileSource" << std::endl;
      return false;
   }
   ossimConnectableObject* chain_head = rdts;
   rdts->setGSD(m_gsd);

   cout<<" numLines = "<< rdts->getNumberOfLines()<<endl;
   cout<<" numSamps = "<< rdts->getNumberOfSamples()<<endl;
   cout<<" numDomes = "<< rdts->getNumRangeDomes()<<endl;

   // Add LUT for visibility if requested:
   if (!m_lutFile.empty())
   {
      if (m_lutFile == TEMP_LUT_FILE)
      {
         // Need to output default temporary LUT:
         ofstream lutfs (m_lutFile.string().c_str(), ios_base::trunc);
         if (lutfs.fail())
         {
            cout << "Could not write out temporary LUT file to CWD. Check permissions." << endl;
            return false;
         }
         lutfs << lut << endl;
         lutfs.close();
      }

      ossimRefPtr<ossimIndexToRgbLutFilter> lutFilter = new ossimIndexToRgbLutFilter();
      lutFilter->connectMyInputTo(chain_head);
      lutFilter->setLut(m_lutFile);
      lutFilter->initialize();
      chain_head = lutFilter.get();
   }

   // Set up the writer for results file:
   ossimRefPtr<ossimTiffWriter> tiffWriter = new ossimTiffWriter();
   tiffWriter->connectMyInputTo(chain_head);
   tiffWriter->setFilename(m_resultsFile);
   if (!tiffWriter->execute())
   {
      cout << "Could not write results file <"<<m_resultsFile<<">." << endl;
      return false;
   }

   tiffWriter->close();
   tiffWriter = 0;
   handler = 0;

   cout<<"\nTest completed. Output written to <"<<m_resultsFile<<">.\n"<<endl;
   return true;
}

int main(int argc, char *argv[])
{
   ossimArgumentParser ap(&argc, argv);
   ap.getApplicationUsage()->setApplicationName(argv[0]);
   ossimInit::instance()->initialize(ap);

   Executive* executive = new Executive(ap);
   bool success = executive->run();
   delete executive;

   if (!success)
      exit(1);

   exit(0);
}



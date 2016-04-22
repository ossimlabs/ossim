//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimBandMergeUtil.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimException.h>
#include <ossim/imaging/ossimBandMergeSource.h>
#include <iostream>

using namespace std;

const char* ossimBandMergeUtil::DESCRIPTION  =
      "Merges multiple band files into a single RGB image.";

ossimBandMergeUtil::ossimBandMergeUtil()
{
}

ossimBandMergeUtil::~ossimBandMergeUtil()
{
}

void ossimBandMergeUtil::setUsage(ossimArgumentParser& ap)
{
   // Add options.
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " -r <red_band> -g <green_band> -b <blue_band> <output-rgb-image>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->setDescription(DESCRIPTION);

   // Base class has its own:
   ossimChipProcUtil::setUsage(ap);

   au->addCommandLineOption("--red", "<filename> Filename of red band. ");
   au->addCommandLineOption("--green", "<filename> Filename of green band. ");
   au->addCommandLineOption("--blue", "<filename> Filename of blue band. ");
}

bool ossimBandMergeUtil::initialize(ossimArgumentParser& ap)
{
   if (!ossimChipProcUtil::initialize(ap))
      return false;

   ostringstream xmsg;

   std::string ts1;
   ossimArgumentParser::ossimParameter sp1 (ts1);
   int band_count = 0;

   if ( ap.read("--red", sp1))
   {
      ostringstream key;
      key<<ossimKeywordNames::IMAGE_FILE_KW<<"0";
      m_kwl.addPair( key.str(), ts1 );
      ++band_count;
   }
   if ( ap.read("--green", sp1))
   {
      ostringstream key;
      key<<ossimKeywordNames::IMAGE_FILE_KW<<"1";
      m_kwl.addPair( key.str(), ts1 );
      ++band_count;
   }
   if ( ap.read("--blue", sp1))
   {
      ostringstream key;
      key<<ossimKeywordNames::IMAGE_FILE_KW<<"2";
      m_kwl.addPair( key.str(), ts1 );
      ++band_count;
   }
   if (band_count != 3)
   {
      ossimNotify(ossimNotifyLevel_NOTICE)<<"ossimBandMergeUtil:"<<__LINE__
            <<" Must supply three band filenames."<<endl;
      return false;
   }

   processRemainingArgs(ap);
   return true;
}

void ossimBandMergeUtil::initialize(const ossimKeywordlist& kwl)
{
   m_kwl.add( ossimKeywordNames::PROJECTION_KW, "input", false );
   ossimChipProcUtil::initialize(kwl);
}

void ossimBandMergeUtil::initProcessingChain()
{
   if (m_imgLayers.size() != 3)
   {
      ostringstream errMsg;
      errMsg<<"ERROR ossimBandMergeUtil:"<<__LINE__<<"  Expected 3 input layers but found "
            <<m_imgLayers.size() <<". Cannot continue. "<<ends;
      throw ossimException(errMsg.str());
   }

   ossimConnectableObject::ConnectableObjectList bandList;
   for (int band=0; band<3; band++)
      bandList.push_back(m_imgLayers[band].get());
   ossimRefPtr<ossimBandMergeSource> combiner = new ossimBandMergeSource(bandList);
   m_procChain->add(combiner.get());
}

void ossimBandMergeUtil::getKwlTemplate(ossimKeywordlist& kwl)
{
   ossimChipProcUtil::getKwlTemplate(kwl);

   ossimString keybase (ossimKeywordNames::IMAGE_FILE_KW );
   ossimString key = keybase + "0";
   kwl.add(key.chars(), "<red-band>", true);
   key = keybase + "1";
   kwl.add(key.chars(), "<green-band>", true);
   key = keybase + "2";
   kwl.add(key.chars(), "<blue-band>", true);
}


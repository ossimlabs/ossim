//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimRtti.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/elevation/ossimImageElevationDatabase.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimImageMosaic.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimEquationCombiner.h>
#include <ossim/imaging/ossimBandLutFilter.h>
#include <ossim/imaging/ossimEdgeFilter.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/util/ossimShorelineUtil.h>
#include <fstream>

static const string COLOR_CODING_KW = "color_coding";
static const string THRESHOLD_KW = "threshold";
static const string TOLERANCE_KW = "tolerance";
static const string ALGORITHM_KW = "algorithm";
static const string SKIP_EDGE_DETECT_KW = "skip_edge_detect";

const char* ossimShorelineUtil::DESCRIPTION =
      "Computes bitmap of water versus land areas in an input image.";

using namespace std;

ossimShorelineUtil::ossimShorelineUtil()
:    m_waterValue (255),
     m_marginalValue (128),
     m_landValue (64),
     m_sensor ("ls8"),
     m_threshold (0.51),
     m_tolerance(0.01),
     m_algorithm(NDWI),
     m_skipThreshold(false),
     m_skipEdgeDetect(false)
{
}

ossimShorelineUtil::~ossimShorelineUtil()
{
}

void ossimShorelineUtil::setUsage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimChipProcUtil::setUsage(ap);

   // Set the general usage:
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " [options] <output-image>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->addCommandLineOption("--algorithm <name>",
         "Specifies detection algorithm to apply. Supported names are \"ndwi\" (requires 2 input "
         "bands: 3 and 5|6), \"awei\" (requires 4 input bands: 3, 6, 5, and 7).");
   au->addCommandLineOption("--color-coding <water> <marginal> <land>",
         "Specifies the pixel values (0-255) for the output product corresponding to water, marginal, "
         "and land zones, respectively. Defaults to 255, 128, and 64, respectively.");
   au->addCommandLineOption("--no-edge",
         "Directs the processing to skip edge detection step.");
   au->addCommandLineOption("--sensor <string>",
         "Sensor used to compute Modified Normalized Difference Water Index. Currently only "
         "\"ls8\" supported.");
   au->addCommandLineOption("--threshold <0.0-1.0>",
         "Normalized threshold for converting the image to bitmap. Defaults to 0.5. Alternatively "
         "can be set to 'X' to skip thresholding operation.");
   au->addCommandLineOption("--tolerance <float>",
         "tolerance +- deviation from threshold for marginal classifications. Defaults to 0.05.");
}

bool ossimShorelineUtil::initialize(ossimArgumentParser& ap)
{
   if (!ossimChipProcUtil::initialize(ap))
      return false;

   string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);
   string ts2;
   ossimArgumentParser::ossimParameter sp2(ts2);
   string ts3;
   ossimArgumentParser::ossimParameter sp3(ts3);

   if ( ap.read("--algorithm", sp1))
      m_kwl.addPair(ALGORITHM_KW, ts1);

   if (ap.read("--color-coding", sp1, sp2, sp3))
   {
      ostringstream value;
      value<<ts1<<" "<<ts2<<" "<<ts3;
      m_kwl.addPair( COLOR_CODING_KW, value.str() );
   }

   if ( ap.read("--no-edge"))
      m_kwl.addPair(SKIP_EDGE_DETECT_KW, string("true"));

   if ( ap.read("--sensor", sp1))
      m_kwl.addPair(ossimKeywordNames::SENSOR_ID_KW, ts1);

   if ( ap.read("--threshold", sp1))
      m_kwl.addPair(THRESHOLD_KW, ts1);

   if ( ap.read("--tolerance", sp1))
      m_kwl.addPair(TOLERANCE_KW, ts1);

   processRemainingArgs(ap);
   return true;
}

void ossimShorelineUtil::initialize(const ossimKeywordlist& kwl)
{
   ossimString value;
   ostringstream xmsg;

   // Don't copy KWL if member KWL passed in:
   if (&kwl != &m_kwl)
   {
      // Start with clean options keyword list.
      m_kwl.clear();
      m_kwl.addList( kwl, true );
   }

   value = m_kwl.findKey(ALGORITHM_KW);
   if (!value.empty())
   {
      if (value=="ndwi")
         m_algorithm = NDWI;
      else if (value=="awei")
         m_algorithm = AWEI;
      else
      {
         xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Bad value encountered for keyword <"
               <<ALGORITHM_KW<<">.";
         throw(xmsg.str());
      }
   }

   value = m_kwl.findKey(COLOR_CODING_KW);
   if (!value.empty())
   {
      vector<ossimString> values = value.split(" ");
      if (values.size() == 3)
      {
         m_waterValue = values[0].toUInt8();
         m_marginalValue = values[1].toUInt8();
         m_landValue = values[2].toUInt8();
      }
      else
      {
         xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Unexpected number of values encountered for keyword <"
               <<COLOR_CODING_KW<<">.";
         throw(xmsg.str());
      }
   }

   value = m_kwl.find(ossimKeywordNames::SENSOR_ID_KW);
   if (!value.empty())
      m_sensor = value;

   m_kwl.getBoolKeywordValue(m_skipEdgeDetect, SKIP_EDGE_DETECT_KW.c_str());

   value = m_kwl.findKey(THRESHOLD_KW);
   if (!value.empty())
   {
      if (value=="X")
         m_skipThreshold = true;
      else
         m_threshold = value.toDouble();
   }

   value = m_kwl.findKey(TOLERANCE_KW);
   if (!value.empty())
      m_tolerance = value.toDouble();

   ossimChipProcUtil::initialize(kwl);
}

void ossimShorelineUtil::initProcessingChain()
{
   ostringstream xmsg;

   if (m_aoiGroundRect.hasNans() || m_aoiViewRect.hasNans())
   {
      xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Encountered NaNs in AOI."<<ends;
      throw ossimException(xmsg.str());
   }

   if (m_sensor == "ls8")
      initLandsat8();
   else
   {
      xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Sensor <"<<m_sensor<<"> not supported"<<ends;
      throw ossimException(xmsg.str());
   }
}

void ossimShorelineUtil::initLandsat8()
{
   ostringstream xmsg;

   ossim_uint32 reqdNumInputs = 0;
   ossimString equationSpec;
   switch (m_algorithm)
   {
   case NDWI:
      reqdNumInputs = 2;
      equationSpec = "in[0]/(in[0]+in[1])";
      break;
   case AWEI:
      reqdNumInputs = 4;
      equationSpec = "4*(in[0]+in[1]) - 0.25*in[2] - 2.75*in[3]";
      break;
   default:
      break;
   }

   if (m_imgLayers.size() < reqdNumInputs)
   {
      xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Expected "<< reqdNumInputs << " input images"
            " but only found "<<m_imgLayers.size()<<"."<<ends;
      throw ossimException(xmsg.str());
   }

   // Set up equation combiner:
   ossimConnectableObject::ConnectableObjectList connectable_list;
   for (ossim_uint32 i=0; i<reqdNumInputs; ++i)
      connectable_list.push_back(m_imgLayers[i].get());
   ossimRefPtr<ossimEquationCombiner> eqFilter = new ossimEquationCombiner(connectable_list);
   eqFilter->setOutputScalarType(OSSIM_FLOAT);
   eqFilter->setEquation(equationSpec);
   m_procChain->add(eqFilter.get());

   if (!m_skipThreshold)
   {
      // Set up threshold filter:
      double del = FLT_EPSILON;
      ossimString landValue = ossimString::toString(m_landValue).chars();
      ossimString waterValue = ossimString::toString(m_waterValue).chars();
      ossimString marginalValue = ossimString::toString(m_marginalValue).chars();
      ossimString thresholdValueLo1 = ossimString::toString(m_threshold-m_tolerance).chars();
      ossimString thresholdValueLo2 = ossimString::toString(m_threshold-m_tolerance+del).chars();
      ossimString thresholdValueHi1 = ossimString::toString(m_threshold+m_tolerance).chars();
      ossimString thresholdValueHi2 = ossimString::toString(m_threshold+m_tolerance+del).chars();
      ossimKeywordlist remapper_kwl;
      remapper_kwl.add("type", "ossimBandLutFilter");
      remapper_kwl.add("enabled", "1");
      remapper_kwl.add("mode", "interpolated");
      remapper_kwl.add("scalar_type", "U8");
      remapper_kwl.add("entry0.in", "0.0");
      remapper_kwl.add("entry0.out", landValue.chars());
      remapper_kwl.add("entry1.in", thresholdValueLo1.chars());
      remapper_kwl.add("entry1.out", landValue.chars());
      remapper_kwl.add("entry2.in", thresholdValueLo2.chars());
      remapper_kwl.add("entry2.out", marginalValue.chars());
      remapper_kwl.add("entry3.in", thresholdValueHi1.chars());
      remapper_kwl.add("entry3.out", marginalValue.chars());
      remapper_kwl.add("entry4.in", thresholdValueHi2.chars());
      remapper_kwl.add("entry4.out", waterValue.chars());
      remapper_kwl.add("entry5.in", "1.0");
      remapper_kwl.add("entry5.out", waterValue.chars());
      ossimRefPtr<ossimBandLutFilter> remapper = new ossimBandLutFilter;
      remapper->loadState(remapper_kwl);
      m_procChain->add(remapper.get());
   }

   if (!m_skipEdgeDetect)
   {
      // Set up edge detector:
      ossimRefPtr<ossimEdgeFilter> edge_filter = new ossimEdgeFilter;
      edge_filter->setFilterType("roberts");
      m_procChain->add(edge_filter.get());
   }
}

ossimRefPtr<ossimImageData> ossimShorelineUtil::getChip(const ossimIrect& bounding_irect)
{
   ostringstream xmsg;
   if (!m_geom.valid())
      return 0;

   m_aoiViewRect = bounding_irect;
   m_geom->setImageSize( m_aoiViewRect.size() );
   m_geom->localToWorld(m_aoiViewRect, m_aoiGroundRect);

   return m_procChain->getTile( m_aoiViewRect, 0 );
}

/*
void ossimShorelineUtil::computeKMeans()
{
   int numbers, k, kvals[25], prevKvals[25], steps = 1, addition[25][100], count = 0, groups[25][100], min, groupnum, value, sum, ok = 1, nums[100];
   cout << "How many numbers you want to enter: ";
   cin >> numbers;

   cout << "Enter value of k: ";
   cin >> k;

   //get numbers
   for(int i = 0; i < numbers; i++)
   {
      cout << "Enter Number " << i+1 << ": ";
      cin >> nums[i];
   }

   // set values of C's
   for(int i = 0; i < 3; i++)
   {
      kvals[i] = nums[i];
   }
   //show values of user
   cout << "You have entered: ";
   for(int i = 0; i < numbers; i++)
   {
      cout << nums[i] << ", ";
   }

   //while(steps < 10)
   while(ok == 1)
   {
      cout << endl << "Itration Number: " << steps;
      //make <span class="IL_AD" id="IL_AD8">calculations</span> (C - bla bla bla)
      for(int i = 0; i < k; i++)
      {
         for(int j = 0; j < numbers; j++)
         {
            addition[i][j] = <span class="IL_AD" id="IL_AD12">abs</span>(kvals[i] - nums[j]);
         }
      }

      //make groups of number(C)
      for(int i = 0; i < numbers; i++)
      {
         min = 100000;
         for(int j = 0; j < k; j++)
         {
            if(addition[j][i] < min)
            {
               min = addition[j][i];
               value = nums[i];
               groupnum = j;
            }
         }
         groups[groupnum][i] = value;
      }

      //show results of calculations (C - bla bla bla)
      cout << endl << "Calculations" << endl;
      for(int i = 0; i < numbers; i++)
      {
         for(int j = 0; j < k; j++)
         {
            cout << addition[j][i] << "\t";
         }
         cout << endl;
      }
      // show groups and get new C's
      cout << endl << "Gruops" << endl;
      for(int i = 0; i < k; i++)
      {
         sum = 0;
         count = 0;
         cout << "Group " << i+1 << ": ";
         for(int j = 0; j < numbers; j++)
         {
            if(groups[i][j] != NULL)
            {
               cout << groups[i][j] << "\t";
               sum += groups[i][j];
               count++;
            }
         }
         prevKvals[i] = kvals[i];
         kvals[i] = sum/count;
         cout << "\t=\t" << kvals[i] << endl;
      }

      //make empty array of groups
      for(int i = 0; i < 25; i++)
      {
         for(int j = 0; j < 100; j++)
         {
            groups[i][j] = NULL;
         }
      }

      //check condition of termination
      ok = 0;
      for(int i = 0; i < k; i++)
      {
         if(prevKvals[i] != kvals[i])
         {
            ok = 1;
         }
      }

      if(ok != 1)
      {
         getch();
      }

      steps++;
   } // end while loop

   getch();
   return 0;
}
*/

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
#include <ossim/util/ossimShorelineUtil.h>
#include <fstream>

static const string COLOR_CODING_KW = "color_coding";
static const string THRESHOLD_KW = "threshold";
static const string TOLERANCE_KW = "tolerance";

const char* ossimShorelineUtil::DESCRIPTION =
      "Computes bitmap of water versus land areas in an input image.";

using namespace std;

ossimShorelineUtil::ossimShorelineUtil()
:    m_waterValue (255),
     m_marginalValue (128),
     m_landValue (64),
     m_sensor ("ls8"),
     m_threshold (0.5),
     m_tolerance(0.05)
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
   au->addCommandLineOption("--color-coding <water> <marginal> <land>",
         "Specifies the pixel values (0-255) for the output product corresponding to water, marginal, "
         "and land zones, respectively. Defaults to 255, 128, and 64, respectively.");
   au->addCommandLineOption("--sensor <string>",
         "Sensor used to compute Modified Normalized Difference Water Index. Currently only "
         "\"ls8\" supported.");
   au->addCommandLineOption("--threshold <0.0-1.0>",
         "Normalized threshold for converting the image to bitmap. Defaults to 0.5.");
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

   if (ap.read("--color-coding", sp1, sp2, sp3))
   {
      ostringstream value;
      value<<ts1<<" "<<ts2<<" "<<ts3;
      m_kwl.addPair( COLOR_CODING_KW, value.str() );
   }

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

   m_sensor = m_kwl.find(ossimKeywordNames::SENSOR_ID_KW);

   value = m_kwl.findKey(THRESHOLD_KW);
   if (!value.empty())
      m_threshold = value.toDouble();

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

   // Landsat 8 requires two inputs: Band 3 (green) and band 6 (SWIR-1):
   if (m_imgLayers.size() != 2)
   {
      xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Expected two input images representing "
            "bands 3 and 6, respectively, but only found "<<m_imgLayers.size()<<"."<<ends;
      throw ossimException(xmsg.str());
   }

   ossimConnectableObject::ConnectableObjectList connectable_list;
   connectable_list.push_back(m_imgLayers[0].get());
   connectable_list.push_back(m_imgLayers[1].get());
   ossimRefPtr<ossimEquationCombiner> eqFilter = new ossimEquationCombiner(connectable_list);
   eqFilter->setOutputScalarType(OSSIM_NORMALIZED_FLOAT);
   ossimString equationSpec ("in[0]/(in[0]+in[1])");
   eqFilter->setEquation(equationSpec);
   m_procChain->add(eqFilter.get());

   double del = OSSIM_DEFAULT_MIN_PIX_FLOAT;
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


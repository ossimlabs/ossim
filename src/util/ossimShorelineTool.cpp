//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/util/ossimShorelineTool.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimKMeansClustering.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimEquationCombiner.h>
#include <ossim/imaging/ossimBandLutFilter.h>
#include <ossim/imaging/ossimEdgeFilter.h>
#include <ossim/imaging/ossimImageGaussianFilter.h>
#include <ossim/imaging/ossimErosionFilter.h>
#include <ossim/imaging/ossimImageHistogramSource.h>
#include <ossim/ossimVersion.h>
#include <ossim/util/ossimToolRegistry.h>
#include <ossim/json/json.h>
#include <fstream>

using namespace std;

static const string COLOR_CODING_KW = "color_coding";
static const string SMOOTHING_KW = "smoothing";
static const string THRESHOLD_KW = "threshold";
static const string TMODE_KW = "tmode";
static const string TOLERANCE_KW = "tolerance";
static const string ALGORITHM_KW = "algorithm";
static const ossimFilename DUMMY_OUTPUT_FILENAME = "@@NEVER_USE_THIS@@";
static const ossimFilename TEMP_INDEX_FILENAME = "temp_shoreline_index.tif";
static const ossimFilename TEMP_THRESHOLD_FILENAME = "temp_shoreline_threshold.tif";
static const ossimFilename TEMP_MASK_FILENAME = "temp_shoreline_mask.tif";

const char* ossimShorelineTool::DESCRIPTION =
      "Computes bitmap of water versus land areas in an input image.";

ossimShorelineTool::ossimShorelineTool()
:    m_waterValue (0),
     m_marginalValue (128),
     m_landValue (255),
     m_sensor ("ls8"),
     m_threshold (-1.0), // Flags auto-thresholding
     m_tolerance(0.0),
     m_algorithm(NDWI),
     m_thresholdMode(SIGMA),
     m_smoothing(0),
     m_noVector(false)
{
}

ossimShorelineTool::~ossimShorelineTool()
{
}

void ossimShorelineTool::setUsage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimChipProcTool::setUsage(ap);

   // Set the general usage:
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " shoreline [options] [<output-vector-filename>]";
   au->setCommandLineUsage(usageString);
   au->setDescription("Computes vector shoreline from raster imagery. The vectors are output "
         "in GeoJSON format. If an output filename is specified, the JSON is written to it. "
         "Otherwise it is written to the console. The supported algorithms are (where A, B, etc are ordered input band files): \n"
         "   NDWI = (A - B) / (A + B) \n"
         "   AWEI = 4*(A + B) - 0.25*C - 2.75*D \n"
         "   PAN-THRESHOLD = A (no equation applied). The input single-band image is thresholded directly. \n");

   // Set the command line options:
   au->addCommandLineOption("--algorithm <name>",
         "Specifies detection algorithm to apply. Supported names are: "
         "\"ndwi\", \"aewi\", \"pan-threshold\"");
   au->addCommandLineOption("--color-coding <water> <marginal> <land>",
         "Specifies the pixel values (0-255) for the output product corresponding to water, marginal, "
         "and land zones. Defaults to 0, 128, and 255, respectively.");
   au->addCommandLineOption("--no-vector",
        "Outputs the raster, thresholded indexed image instead of a vector product. For engineering purposes.");
    au->addCommandLineOption("--sensor <string>",
           "Sensor used to compute Modified Normalized Difference Water Index. Currently only "
           "\"ls8\" supported (default).");
   au->addCommandLineOption("--smooth <sigma>",
         "Applies gaussian filter to index raster file. The filter sigma must be specified (0.2 is good). Sigma=0 "
         "indicates no smoothing.");
   au->addCommandLineOption("--threshold <0.0-1.0>",
         "Normalized threshold for converting the image to bitmap. Defaults to 0.55. If none specified, an optimized threshold"
         " is computed.");
   au->addCommandLineOption("--tmode <1|2|3>",
         "Mode to use for computing threshold from k-means statistics, where:\n"
         "   0 = no auto-thresholding,\n"
         "   1 = unweighted mean of means,\n"
         "   2 = sigma-weighted mean of means (default),\n"
         "   3 = variance-weighted mean of means."
         "If \"--threshold\" option is specified, this option is ignored.\n");
   au->addCommandLineOption("--tolerance <float>",
         "tolerance +- deviation from threshold for marginal classifications. Defaults to 0.01.");
}

bool ossimShorelineTool::initialize(ossimArgumentParser& ap)
{
   if (!ossimChipProcTool::initialize(ap))
      return false;
   if (m_helpRequested)
      return true;

   ossimString ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);
   ossimString ts2;
   ossimArgumentParser::ossimParameter sp2(ts2);
   ossimString ts3;
   ossimArgumentParser::ossimParameter sp3(ts3);

   if ( ap.read("--algorithm", sp1))
      m_kwl.addPair(ALGORITHM_KW, ts1);

   if (ap.read("--color-coding", sp1, sp2, sp3))
   {
      ostringstream value;
      value<<ts1<<" "<<ts2<<" "<<ts3;
      m_kwl.addPair( COLOR_CODING_KW, value.str() );
   }

   if ( ap.read("--no-vector"))
      m_noVector = true;

   if ( ap.read("--sensor", sp1))
      m_kwl.addPair(ossimKeywordNames::SENSOR_ID_KW, ts1);

   if ( ap.read("--smooth", sp1))
      m_kwl.addPair(SMOOTHING_KW, ts1);

   if ( ap.read("--threshold", sp1))
      m_kwl.addPair(THRESHOLD_KW, ts1);

   if ( ap.read("--tmode", sp1))
      m_kwl.addPair(TMODE_KW, ts1);

   if ( ap.read("--tolerance", sp1))
      m_kwl.addPair(TOLERANCE_KW, ts1);

   // Added for "beachfront" to allow passing properties (keyword:value pairs) through to the
   // GeoJSON vector output
   while (ap.read("--prop", sp1))
   {
      m_geoJsonProps.insert(pair<ossimString, ossimString>(ts1.before(":"), ts1.after(":")));
   }

   // Fake the base class into thinking there is a default output filename to avoid it complaining,
   // since this utility will stream vector output to console if no output file name provided:
   m_kwl.add( ossimKeywordNames::OUTPUT_FILE_KW, DUMMY_OUTPUT_FILENAME.c_str());

   processRemainingArgs(ap);
   return true;
}

void ossimShorelineTool::initialize(const ossimKeywordlist& kwl)
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
      else if (value=="pan-threshold")
         m_algorithm = PAN_THRESHOLD;
      else
      {
         xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Bad value encountered for keyword <"
               <<ALGORITHM_KW<<">.";
         throw ossimException(xmsg.str());
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
         throw ossimException(xmsg.str());
      }
   }

   value = m_kwl.find(ossimKeywordNames::SENSOR_ID_KW);
   if (!value.empty())
      m_sensor = value;

   value = m_kwl.findKey(SMOOTHING_KW);
   if (!value.empty())
      m_smoothing = value.toDouble();

   value = m_kwl.findKey(TMODE_KW);
   if (!value.empty())
   {
      int tmode = value.toInt();
      if ((tmode >= 0) && (tmode <= 4))
         m_thresholdMode = (ThresholdMode) tmode;
   }

   value = m_kwl.findKey(THRESHOLD_KW);
   if (!value.empty())
   {
      m_threshold = value.toDouble();
      m_thresholdMode = VALUE;
   }

   value = m_kwl.findKey(TOLERANCE_KW);
   if (!value.empty())
      m_tolerance = value.toDouble();

   // Output filename specifies the vector output, while base class interprets as raster, correct:
   m_vectorFilename = m_kwl.find(ossimKeywordNames::OUTPUT_FILE_KW);
   if (m_vectorFilename == DUMMY_OUTPUT_FILENAME)
   {
      m_vectorFilename = "";
      m_indexFilename = TEMP_INDEX_FILENAME;
      m_threshFilename = TEMP_THRESHOLD_FILENAME;
      m_maskFilename = TEMP_MASK_FILENAME;
   }
   else
   {
      m_indexFilename = m_vectorFilename.fileNoExtension() + "_index.tif";
      m_threshFilename = m_vectorFilename.fileNoExtension() + "_thresh.tif";
      m_maskFilename = m_vectorFilename.fileNoExtension() + "_mask.tif";
   }
   m_kwl.add(ossimKeywordNames::OUTPUT_FILE_KW, m_indexFilename.chars());

   // Unless an output projection was specifically requested, use the input:
   m_kwl.add(ossimKeywordNames::PROJECTION_KW, "identity", false);

   ossimChipProcTool::initialize(kwl);
}

void ossimShorelineTool::initProcessingChain()
{
   // The processing chain to produce the index image is assembled here. The thresholding part is
   // done during execute().
   ostringstream xmsg;

   if (m_aoiGroundRect.hasNans() || m_aoiViewRect.hasNans())
   {
      xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Encountered NaNs in AOI."<<ends;
      throw ossimException(xmsg.str());
   }

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
   case PAN_THRESHOLD:
      reqdNumInputs = 1;
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

   if ((m_algorithm==NDWI) || (m_algorithm==AWEI))
   {
      // Set up equation combiner:
      ossimConnectableObject::ConnectableObjectList connectable_list;
      for (ossim_uint32 i=0; i<reqdNumInputs; ++i)
         connectable_list.push_back(m_imgLayers[i].get());
      ossimRefPtr<ossimEquationCombiner> eqFilter = new ossimEquationCombiner(connectable_list);
      eqFilter->setOutputScalarType(OSSIM_NORMALIZED_DOUBLE);
      eqFilter->setEquation(equationSpec);
      m_procChain->add(eqFilter.get());
   }
   else
   {
      // Just add input connection for pan thresholding:
      m_procChain->add(m_imgLayers[0].get());
   }

   if (m_smoothing > 0)
   {
      // Set up gaussian filter:
      ossimRefPtr<ossimImageGaussianFilter> smoother = new ossimImageGaussianFilter;
      smoother->setGaussStd(m_smoothing);
      m_procChain->add(smoother.get());
   }

   return;
}

ossimRefPtr<ossimImageData> ossimShorelineTool::getChip(const ossimIrect& bounding_irect)
{
   // NOTE:
   // getChip only performs the index and thresholding (and possibly edge). For vector output,
   // execute must be called.

   ostringstream xmsg;
   if (!m_geom.valid())
      return 0;

   m_aoiViewRect = bounding_irect;
   m_geom->setImageSize( m_aoiViewRect.size() );
   m_geom->localToWorld(m_aoiViewRect, m_aoiGroundRect);

   return m_procChain->getTile( m_aoiViewRect, 0 );
}

bool ossimShorelineTool::execute()
{
   if (m_helpRequested)
      return true;

   bool status = true;

   // Base class handles the thresholded image generation. May throw exception. Output written to
   // m_productFilename. Note that if PAN mode selected, there is no intermediate index file:
   m_productFilename = m_indexFilename;
   if ((m_algorithm != PAN_THRESHOLD) && !ossimChipProcTool::execute())
      return false;

   if (m_thresholdMode != NONE)
      doThreshold();
   else
      return true;

   if (m_noVector)
      return status;

   // Now for vector product, need services of a plugin utility. Check if available:
   ossimRefPtr<ossimTool> potrace =
         ossimToolRegistry::instance()->createTool(string("potrace"));
   if (!potrace.valid())
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"ossimShorelineUtil:"<<__LINE__<<"  Need the "
            "ossim-potrace plugin to perform vectorization. Only the thresholded image is "
            "available at <"<<m_productFilename<<">."<<endl;
      return false;
   }

   // Need a mask image representing an eroded version of the input image:
   m_procChain = new ossimImageChain;
   m_procChain->add(m_imgLayers[0].get());
   if (m_smoothing > 0)
   {
      // Set up gaussian filter:
      ossimRefPtr<ossimImageGaussianFilter> smoother = new ossimImageGaussianFilter;
      smoother->setGaussStd(m_smoothing);
      m_procChain->add(smoother.get());
   }
   ossimRefPtr<ossimErosionFilter> eroder = new ossimErosionFilter;
   eroder->setWindowSize(10);
   m_procChain->add(eroder.get());
   m_procChain->initialize();
   m_productFilename = m_maskFilename;
   status = ossimChipProcTool::execute(); // generates mask
   if (m_productFilename.ext() != "JSON")
      m_productFilename = m_vectorFilename.setExtension("json");

   // Convey possible redirection of console out:
   potrace->setOutputStream(m_consoleStream);

   ossimKeywordlist potrace_kwl;
   potrace_kwl.add("image_file0", m_threshFilename.chars());
   potrace_kwl.add("image_file1", m_maskFilename.chars());
   potrace_kwl.add(ossimKeywordNames::OUTPUT_FILE_KW, m_productFilename.chars());
   potrace_kwl.add("mode", "linestring");
   potrace_kwl.add("alphamax", "1.0");
   potrace_kwl.add("turdsize", "4");

   potrace->initialize(potrace_kwl);

   status =  potrace->execute();

   if (status)
      status =  addPropsToJSON();

   if (status)
      ossimNotify(ossimNotifyLevel_INFO)<<"Wrote vector product to <"<<m_productFilename<<">"<<endl;
   else
      ossimNotify(ossimNotifyLevel_WARN)<<"Error encountered writing vector product to <"<<m_productFilename<<">"<<endl;

   return status;
}

void ossimShorelineTool::doThreshold()
{
   ostringstream xmsg;

   // Open the index file (unless doing pan thresholding) to use as the input:
   if (m_algorithm != PAN_THRESHOLD)
   {
      m_procChain = new ossimImageChain();
      ossimRefPtr<ossimImageHandler> indexImage =
            ossimImageHandlerRegistry::instance()->open(m_indexFilename);
      if (!indexImage.valid())
      {
         xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Error encountered reading index image at <"
               <<m_indexFilename<<">.";
         throw ossimException(xmsg.str());
      }
      m_procChain->add(indexImage.get());
   }

   // Some form of auto-thresholding was specified:
   if (m_thresholdMode != VALUE)
      autoComputeThreshold();

   // Set up threshold filter using a simple LUT remapper:
   double del = FLT_EPSILON;
   ossimString landValue = ossimString::toString(m_landValue).chars();
   ossimString waterValue = ossimString::toString(m_waterValue).chars();
   ossimString marginalValue = ossimString::toString(m_marginalValue).chars();
   ossimString thresholdValueLo1 = ossimString::toString(m_threshold-m_tolerance, 9).chars();
   ossimString thresholdValueLo2 = ossimString::toString(m_threshold-m_tolerance+del, 9).chars();
   ossimString thresholdValueHi1 = ossimString::toString(m_threshold+m_tolerance, 9).chars();
   ossimString thresholdValueHi2 = ossimString::toString(m_threshold+m_tolerance+del, 9).chars();
   ossimKeywordlist remapper_kwl;
   remapper_kwl.add("type", "ossimBandLutFilter");
   remapper_kwl.add("enabled", "1");
   remapper_kwl.add("mode", "interpolated");
   remapper_kwl.add("scalar_type", "U8");
   remapper_kwl.add("entry0.in", "0.0");
   remapper_kwl.add("entry0.out", landValue.chars());
   remapper_kwl.add("entry1.in", thresholdValueLo1.chars());
   remapper_kwl.add("entry1.out", landValue.chars());
   if (m_tolerance == 0)
   {
      remapper_kwl.add("entry2.in", thresholdValueLo2.chars());
      remapper_kwl.add("entry2.out", waterValue.chars());
      remapper_kwl.add("entry3.in", "1.0");
      remapper_kwl.add("entry3.out", waterValue.chars());
   }
   else
   {
      remapper_kwl.add("entry2.in", thresholdValueLo2.chars());
      remapper_kwl.add("entry2.out", marginalValue.chars());
      remapper_kwl.add("entry3.in", thresholdValueHi1.chars());
      remapper_kwl.add("entry3.out", marginalValue.chars());
      remapper_kwl.add("entry4.in", thresholdValueHi2.chars());
      remapper_kwl.add("entry4.out", waterValue.chars());
      remapper_kwl.add("entry5.in", "1.0");
      remapper_kwl.add("entry5.out", waterValue.chars());
   }

   ossimRefPtr<ossimBandLutFilter> remapper = new ossimBandLutFilter;
   remapper->loadState(remapper_kwl);
   m_procChain->add(remapper.get());

   m_productFilename = m_threshFilename;
   if (!ossimChipProcTool::execute())
   {
      xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Error encountered creating threshold image at <"
            <<m_threshFilename<<">.";
      throw ossimException(xmsg.str());
   }
}

void ossimShorelineTool::autoComputeThreshold()
{
   // Use the K-means classifier to determine the threshold point based on the histogram clustering
   // into two groups: land and water.

   // If an input histogram was provided, use it. Otherwise compute one:
   ossimImageHandler* handler = dynamic_cast<ossimImageHandler*>(m_procChain->getFirstSource());
   ostringstream xmsg;
   if (!handler)
   {
      xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  No input handler in procession chain.";
      throw ossimException(xmsg.str());
   }
   ossimRefPtr<ossimMultiResLevelHistogram> multi_histo = handler->getImageHistogram();
   if (!multi_histo.valid())
   {
      xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Could not establish an index histogram.";
      throw ossimException(xmsg.str());
   }

   ossimRefPtr<ossimHistogram> band_histo = multi_histo->getHistogram(0);
   if (!band_histo.valid())
   {
      xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  Null band histogram returned!";
      throw ossimException(xmsg.str());
   }

   ossimRefPtr<ossimKMeansClustering> classifier = new ossimKMeansClustering;
   //classifier->setVerbose(); // TODO: Remove verbose mode
   classifier->setNumClusters(2);
   classifier->setSamples(band_histo->GetVals(), band_histo->GetRes());
   classifier->setPopulations(band_histo->GetCounts(), band_histo->GetRes());
   if (!classifier->computeKmeans())
   {
      xmsg<<"ossimShorelineUtil:"<<__LINE__<<"  K-means clustering of histogram failed. Cannot "
            "auto-compute threshold point.";
      throw ossimException(xmsg.str());
   }

   double mean0 = classifier->getMean(0);
   double mean1 = classifier->getMean(1);
   double sigma0 = classifier->getSigma(0);
   double sigma1 = classifier->getSigma(1);
   switch (m_thresholdMode)
   {
   case MEAN:
      m_threshold = (mean0 + mean1)/2.0;
      break;
   case SIGMA:
      m_threshold = (sigma1*mean0 + sigma0*mean1)/(sigma0 + sigma1);
      break;
   case VARIANCE:
      m_threshold = (sigma1*sigma1*mean0 + sigma0*sigma0*mean1)/(sigma0*sigma0 + sigma1*sigma1);
      break;
   default:
      break;
   }

   cout.precision(8);
   cout<<"ossimShorelineUtil::autoComputeThreshold(): Using threshold = "<<m_threshold<<endl;
}

bool ossimShorelineTool::addPropsToJSON()
{
   // Read existing JSON file as output by potrace:
   Json::Value root;
   std::ifstream jsonFile (m_vectorFilename.chars(), std::ifstream::binary);
   if (jsonFile.fail() || jsonFile.eof())
      return false;
   jsonFile >> root;
   jsonFile.close();

   // Add OSSIM-specific properties:
   Json::Value properties(Json::objectValue);
   properties["ossim-version"] = OSSIM_VERSION;

   properties["commit"] = OSSIM_REVISION;

   properties["build_date"] = OSSIM_BUILD_DATE;

   switch (m_algorithm)
   {
   case NDWI:
      properties["algorithm"] = "NDWI";
      break;
   case AWEI:
      properties["algorithm"] = "AWEI";
      break;
   default:
      properties["algorithm"] = "UNKNOWN";
   }

   Json::Value  fnames(Json::arrayValue);
   for (ossim_uint32 f=0; f<m_imgLayers.size(); f++)
   {
      Json::Value fname (m_imgLayers[f]->getFilename().chars());
      fnames.append(fname);
   }
   properties["input_files"] = fnames;

   // Add additional properties provided in the command line:
   map<ossimString, ossimString>::iterator prop = m_geoJsonProps.begin();
   while (prop != m_geoJsonProps.end())
   {
      properties[prop->first.chars()] = prop->second.chars();
      ++prop;
   }

   Json::Value& features = root["features"];
   int n = features.size();
   for (int i=0; i<n; ++i)
   {
      Json::Value& feature = features[i];
      Json::Value& feature_props = feature["properties"];
      feature_props = properties;
   }

   // Output the updated JSON to the file:
   ofstream outFile;
   outFile.open (m_vectorFilename.chars(), ofstream::out | ofstream::trunc);
   if (outFile.fail())
      return false;

   outFile << root;
   outFile.close();

   return true;
}



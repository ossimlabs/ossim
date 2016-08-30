//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/hdf5/ossimHdf5Tool.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/hdf5/ossimHdf5ImageHandler.h>
#include <ossim/hdf5/ossimHdf5Info.h>
#include <iostream>

using namespace std;
using namespace H5;

const char* ossimHdf5Tool::DESCRIPTION  = "Utility for parsing and extracting HDF5 image data.";

static const string IMAGE_DATASET_KW = "image_dataset";
static const string GEOM_DATASET_KW = "geom_dataset";
static const string LIST_DATASETS_KW = "list_datasets";
static const string LIST_NDIMS_KW = "list_ndims";
static const string DUMP_INFO_KW = "dump_info";
static const string DUMP_KWL_KW = "dump_kwl";

ossimHdf5Tool::ossimHdf5Tool()
:  m_dumpInfo (false),
   m_dumpKwl (false),
   m_listDatasets (false),
   m_listNdimDatasets (false)
{
}

ossimHdf5Tool::~ossimHdf5Tool()
{
}

void ossimHdf5Tool::setUsage(ossimArgumentParser& ap)
{
   // Add options.
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " hdf5 [options] [--i <input-hdf5-file>] [-o <output-image>]";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->setDescription(DESCRIPTION);
   au->addCommandLineOption("--geom-dataset","<path_to_dataset>"
                            "Full HDF5-internal path to geometry dataset to use for image. ");
   au->addCommandLineOption("--image-dataset","<path_to_dataset>"
                            "Full HDF5-internal path to pixel dataset to extract. ");
   au->addCommandLineOption("--list-datasets",
                            "Lists all datasets with extents. ");
   au->addCommandLineOption("--list-ndims",
                            "Lists all datasets with extents that are of rank 2 or higher.");
   au->addCommandLineOption("--geom", "[<geomfilename>]\n"
                            "Dumps the geometry keyword-list to the specified file or the console if no filename specified.");
   au->addCommandLineOption("--info",
                            "Outputs human-readable dump of all objects in the file");
   au->addCommandLineOption("--kwl",
                            "Outputs keywordlist dump of all objects in the file");

   // Base class has its own:
   ossimChipProcTool::setUsage(ap);
}

bool ossimHdf5Tool::initialize(ossimArgumentParser& ap)
{
   if (!ossimChipProcTool::initialize(ap))
      return false;
   if (m_helpRequested)
      return true;

   std::string tempString1;
   ossimArgumentParser::ossimParameter stringParam1(tempString1);
   const string TRUE_STR ("true");

   if ( ap.read("--geom-dataset", stringParam1) || ap.read("--gdata", stringParam1))
      m_kwl.addPair(GEOM_DATASET_KW, tempString1);

   if ( ap.read("--geom", stringParam1))
      m_kwl.addPair(ossimKeywordNames::GEOM_FILE_KW, tempString1);

   if ( ap.read("--image-dataset", stringParam1) || ap.read("--idata", stringParam1))
      m_kwl.addPair(IMAGE_DATASET_KW, tempString1);

   if ( ap.read("--list-datasets"))
      m_kwl.addPair(LIST_DATASETS_KW, TRUE_STR);

   if ( ap.read("--list-ndims") )
      m_kwl.addPair(LIST_NDIMS_KW, TRUE_STR);

   if ( ap.read("--info") )
      m_kwl.addPair(DUMP_INFO_KW, TRUE_STR);

   if ( ap.read("--kwl") )
      m_kwl.addPair(DUMP_KWL_KW, TRUE_STR);

   ap.reportRemainingOptionsAsUnrecognized();

   if ( ap.argc() > 1 )
   {
      // If input file already specified, then remaining arg is output file:
      if (m_kwl.numberOf(ossimKeywordNames::IMAGE_FILE_KW))
         m_kwl.add( ossimKeywordNames::OUTPUT_FILE_KW, ap[1]);
      else
      {
         // No input specified, so remaining args contains the input filename:
         if (ap.argc() == 2)
            m_kwl.add( ossimKeywordNames::IMAGE_FILE_KW, ap[1]);
         else
         {
            m_kwl.add( ossimKeywordNames::IMAGE_FILE_KW, ap[1]);
            m_kwl.add( ossimKeywordNames::OUTPUT_FILE_KW, ap[2]);
         }
      }
   }

   initialize(m_kwl);
   return true;
}

void ossimHdf5Tool::initialize(const ossimKeywordlist& kwl)
{
   ostringstream xmsg;

   // Don't copy KWL if member KWL passed in:
   if (&kwl != &m_kwl)
   {
      // Start with clean options keyword list.
      m_kwl.clear();
      m_kwl.addList( kwl, true );
   }

   m_imageDataPath = m_kwl.findKey(IMAGE_DATASET_KW);
   m_geomDataPath = m_kwl.findKey(GEOM_DATASET_KW);
   m_geomFilename =  m_kwl.findKey(ossimKeywordNames::GEOM_FILE_KW);

   m_kwl.getBoolKeywordValue(m_listDatasets, LIST_DATASETS_KW.c_str());
   m_kwl.getBoolKeywordValue(m_listNdimDatasets, LIST_NDIMS_KW.c_str());
   m_kwl.getBoolKeywordValue(m_dumpInfo, DUMP_INFO_KW.c_str());
   m_kwl.getBoolKeywordValue(m_dumpKwl, DUMP_KWL_KW.c_str());

   ossimFilename hdfFile = m_kwl.find(ossimKeywordNames::IMAGE_FILE_KW);
   if (hdfFile.empty() )
   {
      xmsg<<"ERROR: ossimHdf5Tool:"<<__LINE__<<" -- No input filename was specified.";
      throw ossimException(xmsg.str());
   }

   m_hdf5 = new ossimHdf5;
   if (!m_hdf5->open(hdfFile))
   {
      xmsg<<"ERROR: ossimHdf5Tool:"<<__LINE__<<" -- Could not open <"<<hdfFile
            <<"> as an HDF5 file.";
      throw ossimException(xmsg.str());
   }

   if (m_listDatasets || m_listNdimDatasets || m_dumpInfo || m_dumpKwl)
      return;

   // Only if an output file is specified, do we need the services of ossimChipProcTool:
   m_productFilename = m_kwl.findKey( std::string(ossimKeywordNames::OUTPUT_FILE_KW) );
   if (!m_productFilename.empty())
   {
      ossimChipProcTool::initialize(kwl);
      if (m_imgLayers.empty())
      {
         xmsg<<"ERROR: ossimHdf5Tool:"<<__LINE__<<" -- No input chain available. Make sure an HDF5"
               " input file name is specified";
         throw ossimException(xmsg.str());
      }
   }
}

void ossimHdf5Tool::loadImageFiles()
{
   ostringstream errMsg;

   // Special handling needed for loading HDF5 file:
   ossimString value = m_kwl.find(ossimKeywordNames::IMAGE_FILE_KW);
   if (value.empty())
   {
      errMsg<<"ERROR: ossimHdf5Tool:"<<__LINE__<<" -- No input HDF5 file name specified.";
      throw ossimException(errMsg.str());
   }

   if (m_imageDataPath.empty())
   {
      errMsg<<"ERROR: ossimHdf5Tool:"<<__LINE__<<" -- No HDF5 dataset paths were specified."
            " Don't know what dataset to extract. Use \"--idata\" option to specify.";
      throw ossimException(errMsg.str());
   }

   ossimRefPtr<ossimHdf5ImageHandler> handler = new ossimHdf5ImageHandler;
   handler->addRenderable(m_imageDataPath);
   handler->setFilename(ossimFilename(value));
   if (!handler->open())
   {
      errMsg<<"ERROR: ossimHdf5Tool:"<<__LINE__<<" -- Could not open <"<<value<<"> as an HDF5 file.";
      throw ossimException(errMsg.str());
   }

   // Init chain with handler:
   ossimRefPtr<ossimSingleImageChain> chain = new ossimSingleImageChain;
   chain->addLast(handler.get());

   // Set up the remapper:
   if ( ( m_productScalarType != OSSIM_SCALAR_UNKNOWN) &&
         ( m_procChain->getOutputScalarType() != m_productScalarType ) )
   {
      ossimRefPtr<ossimScalarRemapper> remapper = new ossimScalarRemapper();
      remapper->setOutputScalarType(m_productScalarType);
      chain->add(remapper.get());
   }

   // Set up the renderer with cache:
   chain->addResampler();
   chain->addCache();

   // Add geo polygon cutter if specifried:
   ossimString param = m_kwl.findKey(string("clip_poly_lat_lon"));
   if (!param.empty())
   {
      std::vector<ossimGpt> points;
      ossim::toVector(points, param);
      if(points.size() >= 3)
      {
         ossimGeoPolygon polygon(points);
         chain->addGeoPolyCutterPolygon(polygon);
      }
   }

   chain->initialize();
   m_imgLayers.push_back(chain);
}

void ossimHdf5Tool::initProcessingChain()
{
   ossimRefPtr<ossimImageSource> input_mosaic = combineLayers(m_imgLayers);
   m_procChain->add(input_mosaic.get());
}

bool ossimHdf5Tool::execute()
{
   // Need to fix output to tiff

   ostringstream errMsg;
   if (!m_hdf5.valid())
      return false;

   if (m_listDatasets || m_listNdimDatasets || m_dumpInfo || m_dumpKwl)
   {
      ossimHdf5Info info (m_hdf5.get());
      if (m_dumpKwl)
      {
         ossimKeywordlist kwl;
         info.getKeywordlist(kwl);
         kwl.print(cout);
      }
      if (m_dumpInfo)
      {
         info.print(cout);
      }

      Group root;
      m_hdf5->getRoot(root);
      vector<DataSet> datasets;
      if (m_listDatasets)
         m_hdf5->getDatasets(root, datasets, true);
      else if (m_listNdimDatasets)
         m_hdf5->getNdimDatasets(root, datasets, true);
      for (int i=0; i<datasets.size(); ++i)
      {
         info.print(cout, datasets[i]);
      }
   }

   bool status = true;
   if (m_imgLayers.size())
      status = ossimChipProcTool::execute();

   return status;
}

//----------------------------------------------------------------------------
//
// File: ossimAutRegUtil.cpp
//
// License:  LGPL
// 
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
//
// Description: Utility class for autonomous registration operations.
//
//----------------------------------------------------------------------------
// $Id$

#include <ossim/util/ossimAutRegUtil.h>

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimDate.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimTieMeasurementGeneratorInterface.h>
#include <ossim/base/ossimAdjustmentExecutive.h>
#include <ossim/base/ossimPointObservation.h>
#include <ossim/base/ossimObservationSet.h>

#include <ossim/elevation/ossimElevManager.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/init/ossimInit.h>

#include <iostream>

static ossimTrace traceDebug ("ossimAutRegUtil:debug");
static ossimTrace traceLog("ossimAutRegUtil:log");
static ossimTrace traceOptions("ossimAutRegUtil:options");

static const std::string APPLICATION_NAME_KW            = "application_name";
static const std::string ADJ_DESC_LABEL_KW              = "adj_desc_label";
static const std::string FALSE_KW                       = "false";
static const std::string IMG_1_KW                       = "img_1";
static const std::string IMG_2_KW                       = "img_2";
static const std::string IMG_M_KW                       = "img_m"; // master
static const std::string IMG_S_KW                       = "img_s"; // slave
static const std::string OP_KW                          = "operation";
static const std::string SRC_FILE_KW                    = "src_file";
static const std::string TRUE_KW                        = "true";
static const std::string ROI_CENTER_LAT_KW              = "roi_center_lat";
static const std::string ROI_CENTER_LON_KW              = "roi_center_lon";
static const std::string ROI_RADIUS_KW                  = "roi_radius";  // meters
static const std::string ROI_HEIGHT_KW                  = "roi_height";  // pixels
static const std::string ROI_MAX_LAT_KW                 = "roi_max_lat";
static const std::string ROI_MAX_LON_KW                 = "roi_max_lon";
static const std::string ROI_MIN_LAT_KW                 = "roi_min_lat";
static const std::string ROI_MIN_LON_KW                 = "roi_min_lon";
static const std::string ROI_WIDTH_KW                   = "roi_width";   // pixels
static const std::string REPORT_FILE_KW                 = "report_file";
        
static const std::string FILE_KW                 = "file";
static const std::string IMG_KW                  = "image";
static const std::string READER_PROPERTY_KW      = "reader_property";

static const std::string OCV_CONFIG_FILE_KW             = "ocv_config_file";
static const std::string OCV_DETECTOR_NAME_KW           = "ocv_detector_name";
static const std::string OCV_DRAW_MATCHES_FLAG_KW       = "ocv_draw_matches_flag";
static const std::string OCV_EXTRACTOR_NAME_KW          = "ocv_extractor_name";
static const std::string OCV_MATCHER_NAME_KW            = "ocv_matcher_name";
static const std::string OCV_MAX_MATCHES_KW             = "ocv_max_matches";
static const std::string OCV_USE_CROSS_CHECK_KW         = "ocv_use_cross_check";
static const std::string OCV_USE_GRID_ADAPTED_DETECT_KW = "ocv_use_grid_adapted_detect";
static const std::string OCV_RESULTS_WINDOW_SHOW_KW     = "ocv_results_window_show";//????

static const std::string OAX_CONFIG_FILE_KW             = "oax_config_file";
static const std::string OAX_CONVERGENCE_CRITERIA_KW    = "oax_convergence_criteria";
static const std::string OAX_GROUND_SIGMA_KW            = "oax_ground_sigma";
static const std::string OAX_LAT_SIGMA_KW               = "oax_lat_sigma";
static const std::string OAX_LON_SIGMA_KW               = "oax_lon_sigma";
static const std::string OAX_HGT_SIGMA_KW               = "oax_hgt_sigma";
static const std::string OAX_CONTROL_SIGMA_KW           = "oax_control_sigma";
static const std::string OAX_MAX_ITERATIONS_KW          = "oax_max_iterations";


//*****************************************************************************
//  METHOD: ossimAutRegUtil::ossimAutRegUtil()
//  
//  Constructor.
//*****************************************************************************
ossimAutRegUtil::ossimAutRegUtil()
   : ossimReferenced(),
     m_tGen(0),
     m_tGenObj(0),
     m_roiRects(),
     m_showMatchWindow(false),
     m_adjExec(0),
     m_obsSet(0),
     m_LatLonHgtSigmas(),
     m_LatLonHgtControlSigmas(),
     m_rep(0),
     m_imgLayer(0),
     m_kwl(new ossimKeywordlist()),
     m_ocvKwl(0),
     m_oaxKwl(0),
     m_srcKwl(0),
     m_operation(OSSIM_AUTREG_OP_UNKNOWN)
{
   // traceDebug.setTraceFlag(true);
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "\nossimAutRegUtil::ossimAutRegUtil 1 DEBUG:" << std::endl;
   }

   // Default tie point ground sigmas
   m_LatLonHgtSigmas[0] = 50.0;
   m_LatLonHgtSigmas[1] = 50.0;
   m_LatLonHgtSigmas[2] = 50.0;

   // Default control point ground sigmas
   m_LatLonHgtControlSigmas[0] = 1.0;
   m_LatLonHgtControlSigmas[1] = 1.0;
   m_LatLonHgtControlSigmas[2] = 1.0;

   // ossimImageGeometry
   m_geom[0] = 0;
   m_geom[1] = 0;

   // Default control image flags
   m_controlImage[0] = false;
   m_controlImage[1] = false;


   m_kwl->setExpandEnvVarsFlag(true);

}

// Private/hidden
ossimAutRegUtil::ossimAutRegUtil( const ossimAutRegUtil& /* obj */ )
{
}

// Private/hidden
const ossimAutRegUtil& ossimAutRegUtil::operator=( const ossimAutRegUtil& /* rhs */)
{
   return *this;
}


//*****************************************************************************
//  DESTRUCTOR: ~ossimAutRegUtil()
//*****************************************************************************
ossimAutRegUtil::~ossimAutRegUtil()
{
   
   if (traceDebug())  ossimNotify(ossimNotifyLevel_DEBUG)
      << "DEBUG: ~ossimAutRegUtil(): entered..." << std::endl;

   clear();

   *m_rep << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
   *m_rep << endl;

   if (traceDebug())  ossimNotify(ossimNotifyLevel_DEBUG)
      << "DEBUG: ~ossimAutRegUtil(): returning..." << std::endl;
}


void ossimAutRegUtil::addArguments(ossimArgumentParser& ap)
{
   ossimString usageString = ap.getApplicationName();
   usageString += " [option]... [input-option]... -img_1 <input-file 1> -img_2 <input-file 2>\n\nAvailable traces:\n-T \"ossimAutRegUtil:debug\"   - General debug trace to standard out.\n-T \"ossimAutRegUtil:log\"     - Writes a log file to output-file.log.\n-T \"ossimAutRegUtil:options\" - Writes the options to output-file-options.kwl.";

   ossimApplicationUsage* au = ap.getApplicationUsage();
   
   au->setCommandLineUsage(usageString);
    
   au->setDescription(ap.getApplicationName()+": Utility application for autonomous image registration.");

   au->addCommandLineOption("-h or --help", "Display this help and exit.");

   au->addCommandLineOption("--quality", "<quality>\nNOT WORKING... Desired quality of tiepoint collection.\nRange: 1 to 5, Default = 3");
   
   au->addCommandLineOption("--roi-center-llwh","<latitude> <longitude> <width> <height>\nSpecify the center ROI in latitude longitude space with width and height in pixels.");

   au->addCommandLineOption("--roi-center-llr","<latitude> <longitude> <radius_in_meters>\nSpecify the center ROI in latitude longitude space with radius in meters.");

   au->addCommandLineOption("--ocv_config_file","<file_path>\nSpecify a keyword list OpenCV configuration file.");

   au->addCommandLineOption("--oax_config_file","<file_path>\nSpecify a keyword list oax configuration file.");

   
} // End: ossimAutRegUtil::addArguments


void ossimAutRegUtil::clear()
{
   // Must disconnect chains so that they destroy.
   std::vector< ossimRefPtr<ossimSingleImageChain> >::iterator i = m_imgLayer.begin();
   while ( i != m_imgLayer.end() )
   {
      (*i)->disconnect();
      (*i) = 0;
      ++i;
   }
   m_imgLayer.clear();

   // i = m_demLayer.begin();
   // while ( i != m_demLayer.end() )
   // {
   //    (*i)->disconnect();
   //    (*i) = 0;
   //    ++i;
   // }
   // m_demLayer.clear();
}

bool ossimAutRegUtil::initialize(ossimArgumentParser& ap)
{
   static const char MODULE[] = "ossimAutRegUtil::initialize(ossimArgumentParser&)";

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   clear();
   if( ap.read("-h") || ap.read("--help") || (ap.argc() == 1) )
   {
      usage(ap);
      
      return false; // Indicates process should be terminated to caller.
   }

   // Start with clean options keyword list.
   m_kwl->clear();

   std::string tempString1;
   ossimArgumentParser::ossimParameter stringParam1(tempString1);
   std::string tempString2;
   ossimArgumentParser::ossimParameter stringParam2(tempString2);
   std::string tempString3;
   ossimArgumentParser::ossimParameter stringParam3(tempString3);
   std::string tempString4;
   ossimArgumentParser::ossimParameter stringParam4(tempString4);

   double tempDouble1;
   ossimArgumentParser::ossimParameter doubleParam1(tempDouble1);
   double tempDouble2;
   ossimArgumentParser::ossimParameter doubleParam2(tempDouble2);

   ossimString key = "";
   
   // Extract arguments and stuff them in a keyword list.
   if( ap.read("--report_file", stringParam1) )
   {
      m_kwl->addPair( std::string(REPORT_FILE_KW), tempString1 );
   }

   // report file
   if( ap.read("--quality", stringParam1) )
   {
      m_kwl->addPair( std::string(ossimKeywordNames::QUALITY_KW), tempString1 );
   }

   // operation
   if( ap.read("--op", stringParam1) )
   {
      m_kwl->addPair( OP_KW, tempString1 );
   }

   // image 1
   if( ap.read("--img_1", stringParam1))
   {
      m_kwl->addPair( std::string(IMG_1_KW), tempString1 );
   }

   // image 2
   if( ap.read("--img_2", stringParam1))
   {
     m_kwl->addPair( std::string(IMG_2_KW), tempString1 );
   }

   // master image
   if( ap.read("--img_m", stringParam1))
   {
      m_kwl->addPair( std::string(IMG_M_KW), tempString1 );
   }

   // slave image
   if( ap.read("--img_s", stringParam1))
   {
     m_kwl->addPair( std::string(IMG_S_KW), tempString1 );
   }

   // OpenCV config
   if( ap.read("--ocv_config_file", stringParam1))
   {
     m_kwl->addPair( std::string(OCV_CONFIG_FILE_KW), tempString1 );
   }

   // oax config
   if( ap.read("--oax_config_file", stringParam1))
   {
     m_kwl->addPair( std::string(OAX_CONFIG_FILE_KW), tempString1 );
   }

   // llwh chipping parameters
   if( ap.read("--roi-center-llwh", stringParam1, stringParam2, stringParam3, stringParam4) )
   {
      m_kwl->addPair( ROI_CENTER_LAT_KW, tempString1 );
      m_kwl->addPair( ROI_CENTER_LON_KW, tempString2 );
      m_kwl->addPair( ROI_WIDTH_KW,      tempString3 );
      m_kwl->addPair( ROI_HEIGHT_KW,     tempString4 );
   }

   // llr chipping parameters
   if( ap.read("--roi-center-llr", stringParam1, stringParam2, stringParam3) )
   {
      m_kwl->addPair( ROI_CENTER_LAT_KW, tempString1 );
      m_kwl->addPair( ROI_CENTER_LON_KW, tempString2 );
      m_kwl->addPair( ROI_RADIUS_KW,     tempString3 );
   }

   // TODO Necessary???
   if( ap.read("--input-src", stringParam1) )
   {
      m_kwl->addPair( SRC_FILE_KW, tempString1 );
   }

   // TODO  For now, just an example of using ossimKeywordNames content
   if( ap.read("--quality", stringParam1) )
   {
      m_kwl->addPair( std::string(ossimKeywordNames::QUALITY_KW), tempString1 );
   }

   //---
   // Deprecated: "--options-keyword-list"
   //---
   if( ap.read("--options", stringParam1) )
   {
      ossimFilename optionsKwl = tempString1;
      if ( optionsKwl.exists() )
      {
         if ( m_kwl->addFile(optionsKwl) == false )
         {
            std::string errMsg = "ERROR could not open options keyword list file: ";
            errMsg += optionsKwl.string();
            throw ossimException(errMsg);
         }
      }
      else
      {
         std::string errMsg = "ERROR options keyword list file does not exists: ";
         errMsg += optionsKwl.string();
         throw ossimException(errMsg); 
      }
   }


   // End of arg parsing.


   ap.reportRemainingOptionsAsUnrecognized();
   if ( ap.errors() )
   {
      ap.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
      std::string errMsg = "Unknown option...";
      throw ossimException(errMsg);
   }


   if ( ap.argc() >= 2 )
   {
      // Output file is last arg:
      m_kwl->add( ossimKeywordNames::OUTPUT_FILE_KW, ap[ap.argc()-1]);
   }
   else
   {
      if ( !m_kwl->find(ossimKeywordNames::OUTPUT_FILE_KW) )
      {
         ap.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
         std::string errMsg = "Must supply an output file.";
         throw ossimException(errMsg);
      }
   }

   if ( ap.argc() > 2 ) // User passed inputs in front of output file.
   {
      int pos = 1; // ap.argv[0] is application name. 
      while ( pos < (ap.argc()-1) )
      {
         ossimFilename file = ap[pos];
         if ( traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "argv[" << pos << "]: " << file << "\n";
         }
         
         if ( isSrcFile(file) ) 
         {
            if ( m_kwl->find( SRC_FILE_KW.c_str() ) ) // --input-src used also
            {
               std::string errMsg = MODULE;
               errMsg += "ERROR Multiple src files passed in.  Please combine into one.";
               throw ossimException(errMsg);
            }
            
            m_kwl->addPair( SRC_FILE_KW, file.string() );
         }
         else // Add as an input image.
         {
            // key = IMG_KW;
            // key += ossimString::toString(imgIdx);
            // key += ".";
            // key += FILE_KW;
            // m_kwl->addPair(key.string(), file.string() );
            // ++imgIdx;
         }
         
         ++pos; // Go to next arg...
         
      } // End: while ( pos < (ap.argc()-1) )
       
   } // End: if ( ap.argc() > 2 )

   initialize();
   
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited..." << std::endl;
   }  
   return true;
   
} // End: void ossimAutRegUtil::initialize(ossimArgumentParser& ap)


void ossimAutRegUtil::initialize( const ossimKeywordlist& kwl )
{
   clear();

   // Start with clean options keyword list.
   m_kwl->clear();

   m_kwl->addList( kwl, true );

   initialize();
}



void ossimAutRegUtil::initialize()
{
   static const char MODULE[] = "ossimAutRegUtil::initialize()";

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }
   
   if ( traceOptions() )
   {
      ossimFilename optionsFile;
      getOutputFilename(optionsFile);
      optionsFile = optionsFile.noExtension();
      optionsFile += "-options.kwl";
      ossimString comment = " Can be use for --options argument.";
      m_kwl->write( optionsFile.c_str(), comment.c_str() );
   }

   // Determine the operation to do.
   std::string op = m_kwl->findKey( OP_KW );
   if ( op.size() )
   {
      ossimString s = op;
      s.downcase();
      
      if ( s == "coreg" )
      {
         m_operation = OSSIM_AUTREG_OP_COREG;
      }
      else if ( s == "masreg" )
      {
         m_operation = OSSIM_AUTREG_OP_MASREG;
      }
      else
      {
         std::string errMsg = "unknown operation: ";
         errMsg += s.string();
         throw ossimException(errMsg);
      }
   }
   else
   {
      std::string errMsg = "keyword not found: ";
      errMsg += OP_KW;
      errMsg += "\nUse --op option to specify operation.\n";
      throw ossimException(errMsg);  
   }

   //---
   // Report file
   //---
   ossimFilename repFile = m_kwl->findKey( REPORT_FILE_KW );

   if (repFile.size())
   {
      m_rep = new ofstream( repFile.c_str(), ios::out );
   }
   else
   {
      m_rep = &std::cout;
   }
   *m_rep << "\n ossimAutRegUtil Summary..."<<std::endl;
   ossimString ts;
   ossim::getFormattedTime("%a %m.%d.%y %H:%M:%S", false, ts);
   *m_rep << "\n" << ts;
   *m_rep << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
   *m_rep << endl;


   //---
   // Populate the m_srcKwl if --src option was set.
   // Note do this before creating chains.
   //---
   initializeSrcKwl();
   
   //---
   // Load OpenCV config if --ocv_config_file option was set.
   //---
   initializeOcvKwl();
   
   //---
   // Load oax config if --oax_config_file option was set.
   //---
   initializeOaxKwl();


   //---
   // Load input image pair
   //---
   if (!loadImages())
   {
      throw ossimException("loadImages() failed...");
   }

//=========================================================
// TODO...  Is there a possibility of ortho image input?
//   What about associated geometry/adjustbale parameters?
//=========================================================
   //--
   // Raw image input
   //--
   createIdentityProjection(0);
   createIdentityProjection(1);


   //---
   // Determine overlap ROIs
   //---
   if (!delineateROIs())
   {
      throw ossimException("delineateROIs() failed...");
   }


   if ( traceLog() )
   {
      ossimKeywordlist logKwl;
      // m_writer->saveStateOfAllInputs(logKwl);
      
      ossimFilename logFile;
      getOutputFilename(logFile);
      logFile.setExtension("log");

      logKwl.write( logFile.c_str() );
   }

   
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "options keyword list:\n"
         << *(m_kwl.get()) << "\n";
      
      if ( m_srcKwl.valid() )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "support record keyword list:\n"
            << *(m_srcKwl.get()) << "\n";
      }
   }

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Instantiate ossimTieMeasurementGenerator
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    m_tGenObj = ossimObjectFactoryRegistry::instance()->
      createObject(ossimString("ossimTieMeasurementGenerator"));
   if(m_tGenObj.valid())
   {
      m_tGen = dynamic_cast<ossimTieMeasurementGeneratorInterface*> (m_tGenObj.get());
   }
   else
   {
      throw ossimException("ossimTieMeasurementGenerator createObject failed...");
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }

} // End: void ossimAutRegUtil::initialize()



//*****************************************************************************
//  METHOD: ossimAutRegUtil::loadImages()
//  
//  Load image pair.
//  
//*****************************************************************************
bool ossimAutRegUtil::loadImages()
{
   static const char MODULE[] = "ossimAutRegUtil::loadImages";

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   bool isGoodLoad = false;
   ossimFilename inputFile;

   ossimString key;


   // Load 1st image
   if (m_operation == OSSIM_AUTREG_OP_COREG)
   {
      key = IMG_1_KW;
   }
   else  // Load master image
   {
      key = IMG_M_KW;
      m_controlImage[0] = true;
   }
   inputFile = m_kwl->findKey(key.string());
   if (addImgSource(inputFile, 0 ))
   {
      ossimImageHandler *ih1 = m_imgLayer[0]->getImageHandler().get();
      if (!ih1)
      {
         *m_rep << " open for image 1 failed..."<<std::endl;
      }
      else
      {
         int overviews     = ih1->getNumberOfDecimationLevels();
         ossimIrect bounds = ih1->getBoundingRect(0);
         // ossimScalarType scalarType = ih1->getOutputScalarType();
         ossimScalarTypeLut* lut = ossimScalarTypeLut::instance();
         *m_rep << "-----------------" << endl
                << "filename       = " << ih1->getFilename() << endl
                << "width          = " << bounds.width() << endl
                << "height         = " << bounds.height() << endl
                << "overview count = " << (overviews-1) << endl
                << "scalar type    = " << lut->getEntryString(ih1->getOutputScalarType()) << endl
                << "Handler used   = " << ih1->getClassName() << endl;


         // Load 2nd image
         if (m_operation == OSSIM_AUTREG_OP_COREG)
         {
            key = IMG_2_KW;
         }
         else  // Load slave image
         {
            key = IMG_S_KW;
            m_controlImage[1] = false;
         }
         inputFile = m_kwl->findKey(key.string());
         if (addImgSource(inputFile, 0 ))
         {
            ossimImageHandler *ih2 = m_imgLayer[1]->getImageHandler().get();
            if (!ih2)
            {
               *m_rep << " open for image 2 failed"<<std::endl;
            }
            else
            {
               int overviews     = ih2->getNumberOfDecimationLevels();
               ossimIrect bounds = ih2->getBoundingRect(0);
               // ossimScalarType scalarType = ih2->getOutputScalarType();
               ossimScalarTypeLut* lut = ossimScalarTypeLut::instance();
               *m_rep << "-----------------" << endl
                      << "filename       = " << ih2->getFilename() << endl
                      << "width          = " << bounds.width() << endl
                      << "height         = " << bounds.height() << endl
                      << "overview count = " << (overviews-1) << endl
                      << "scalar type    = " << lut->getEntryString(ih2->getOutputScalarType()) << endl
                      << "Handler used   = " << ih2->getClassName() << endl;
               *m_rep << "-----------------" << endl;
               isGoodLoad = true;
            }
         }
      }  
   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }   
   return isGoodLoad;
}


//*****************************************************************************
//  METHOD: ossimAutRegUtil::delineateROIs()
//  
//  Determine correlation region of interest.
//  
//*****************************************************************************
bool ossimAutRegUtil::delineateROIs()
{
   static const char MODULE[] = "ossimAutRegUtil::delineateROIs";

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   bool isGoodOp = false;

   for (int img=0; img<2; ++img)
   {
      ossimIrect rect;
      rect.makeNan();
   
      ossimImageSource* source = m_imgLayer[img].get();
      
      if ( source )
      {
   
         if ( m_geom[img].valid() )
         {
            if ( m_kwl->find( ROI_CENTER_LAT_KW.c_str() ) ) 
            {
               // "ROI Center" with: --roi-center-llwh or --roi-center-llr:
               ossimString latStr = m_kwl->findKey( ROI_CENTER_LAT_KW );
               ossimString lonStr = m_kwl->findKey( ROI_CENTER_LON_KW );
               if ( latStr.size() && lonStr.size() )
               {
                  ossimGpt centerGpt;
   
                  //---
                  // Want the height nan going into worldToLocal call so it gets picked
                  // up by the elevation manager.
                  //---
                  centerGpt.makeNan(); 
   
                  centerGpt.lat = latStr.toFloat64();
                  centerGpt.lon = lonStr.toFloat64();
   
                  if ( !centerGpt.isLatNan() && !centerGpt.isLonNan() )
                  {
                     // Ground "cut center" to view:
                     ossimDpt centerDpt(0.0, 0.0);
                     m_geom[img]->worldToLocal(centerGpt, centerDpt);
   
                     if ( !centerDpt.hasNans() )
                     {
                        // if ( isIdentity() && m_ivt.valid() ) // Chipping in image space.
                        if ( m_ivt[img].valid() ) // Chipping in image space.
                        {
                           // Transform image center point to view:
                           ossimDpt ipt = centerDpt;
                           m_ivt[img]->imageToView( ipt, centerDpt );
                        }
                        
                        // --roi-center-llwh:
                        ossimString widthStr  = m_kwl->findKey( ROI_WIDTH_KW );
                        ossimString heightStr = m_kwl->findKey( ROI_HEIGHT_KW );
                        if ( widthStr.size() && heightStr.size() )
                        {
                           ossim_int32 width  = widthStr.toInt32();
                           ossim_int32 height = heightStr.toInt32();
                           if ( width && height )
                           {
                              ossimIpt ul( ossim::round<int>(centerDpt.x - (width/2)),
                                           ossim::round<int>(centerDpt.y - (height/2)) );
                              ossimIpt lr( (ul.x + width - 1), ul.y + height - 1);
                              rect = ossimIrect(ul, lr);
                           }
                        }
                        else // --roi-center-llr: 
                        {
                           ossimString radiusStr = m_kwl->findKey( ROI_RADIUS_KW );
                           if ( radiusStr.size() )
                           {
                              ossim_float64 radius = radiusStr.toFloat64();
                              if ( radius )
                              {
                                 ossimDpt mpp;
                                 m_geom[0]->getMetersPerPixel( mpp );
   
                                 if ( !mpp.hasNans() )
                                 {
                                    ossim_float64 rx = radius/mpp.x;
                                    ossim_float64 ry = radius/mpp.y;
                                    
                                    ossimIpt ul( ossim::round<int>( centerDpt.x - rx ),
                                                 ossim::round<int>( centerDpt.y - ry ) );
                                    ossimIpt lr( ossim::round<int>( centerDpt.x + rx ),
                                                 ossim::round<int>( centerDpt.y + ry ) );
                                    rect = ossimIrect(ul, lr);
                                 }
                              }
                           }
                        }
                     }
                     
                  } // Matches: if ( !centerGpt.hasNans() )
                  
               } // Matches: if ( latStr && lonStr )
               
            } // Matches: if ( m_kwl->find( ROI_CENTER_LAT_KW ) )
   
         } // if ( igeom.valid() )
   
         else
         {
            // Should never happen...
            std::string errMsg = MODULE;
            if ( !source )
            {
               errMsg += " image source null!";
            }
            throw( ossimException(errMsg) );
         }
   
         // rect.print(*m_rep);
         m_roiRects.push_back(rect);
      } // if ( source )

   }

   isGoodOp = true;


   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }   
   return isGoodOp;
}


//*****************************************************************************
//  METHOD: ossimAutRegUtil::populateObsSet()
//  
//  Transfer tie points from generator to observation set.
//  
//*****************************************************************************
bool ossimAutRegUtil::populateObsSet()
{
   static const char MODULE[] = "ossimAutRegUtil::populateObsSet";

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   bool isGoodLoad = true;

   ossimString id;
   ossimGpt gp;


   for (int m=0; m<m_tGen->numMeasurements(); ++m)
   {
      id = ossimString::toString(m+1);

      // Initialize ground point
      gp.makeNan();
      ossimPointObservation *pt = new ossimPointObservation(gp, id, m_LatLonHgtSigmas);

      for (int img=0; img<2; ++img)
      {
         ossimDpt imgPt = m_tGen->pointIndexedAt(img,m);
         // If control, set ground coordinates and reset sigmas
         if (m_controlImage[img])
         {
            // Point drop to get control coordinates
            ossimGpt worldPt;
            m_geom[img]->localToWorld(imgPt, worldPt);
            if (worldPt.isHgtNan())
            {
               ossim_float64 hgt =
                  ossimElevManager::instance()->getHeightAboveEllipsoid(worldPt);
               worldPt.height(hgt);
            }
            pt->Gpt() = worldPt;

            // Set control sigmas
            pt->setGroundSigmas
               (m_LatLonHgtControlSigmas[0],m_LatLonHgtControlSigmas[1],m_LatLonHgtControlSigmas[2]);
         }
         else
         {
            // Get image filename
            ossimFilename filename = m_imgLayer[img]->getImageHandler()->getFilename();
            
            // Add measurement to point observation
            pt->addMeasurement(imgPt, filename);
         }
      }
      // Add point observation to set
      if (pt->numMeas() > 0)
      {
         m_obsSet->addObservation(pt);
      }
   }


   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }   
   return isGoodLoad;
}


//*****************************************************************************
//  METHOD: ossimAutRegUtil::configureTieMeasGenerator()
//  
//  Configure the tie measurement generator using m_ocvKwl
//  
//*****************************************************************************
void ossimAutRegUtil::configureTieMeasGenerator()
{
   static const char MODULE[] = "ossimAutRegUtil::configureTieMeasGenerator";

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   if ( m_ocvKwl.valid() )
   {

      // Draw OpenCV match window
      m_showMatchWindow = keyIsTrue(m_ocvKwl, OCV_DRAW_MATCHES_FLAG_KW);
      m_tGen->setShowCvWindow(m_showMatchWindow);
   
      // OpenCV detector
      if (m_ocvKwl->find(OCV_DETECTOR_NAME_KW.c_str()))
      {
         m_tGen->setFeatureDetector(m_ocvKwl->findKey(OCV_DETECTOR_NAME_KW));
      }
   
      // OpenCV descriptor/extractor
      if (m_ocvKwl->find(OCV_EXTRACTOR_NAME_KW.c_str()))
      {
         m_tGen->setDescriptorExtractor(m_ocvKwl->findKey(OCV_EXTRACTOR_NAME_KW));
      }
   
      // OpenCV matcher
      if (m_ocvKwl->find(OCV_MATCHER_NAME_KW.c_str()))
      {
         m_tGen->setDescriptorMatcher(m_ocvKwl->findKey(OCV_MATCHER_NAME_KW));
      }

      // OpenCV maximum number of matches
      if (m_ocvKwl->find(OCV_MAX_MATCHES_KW.c_str()))
      {
         ossimString maxMatches = m_ocvKwl->findKey(OCV_MAX_MATCHES_KW);
         if (maxMatches.size())
         {
            ossim_int32 max  = maxMatches.toInt32();
            m_tGen->setMaxMatches(max);
         }
      }
   
      // TODO: set crossCheck currently commented out in ossimTieMeasurementGenerator
      // m_tGen->setUseCrossCheck(keyIsTrue(m_ocvKwl, OCV_USE_CROSS_CHECK_KW));
   
      // Use OpenCV GridAdaptedFeatureDetector
      m_tGen->setUseGrid(keyIsTrue(m_ocvKwl, OCV_USE_GRID_ADAPTED_DETECT_KW));

   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }   
}


//*****************************************************************************
//  METHOD: ossimAutRegUtil::configureAdjustmentExecutive()
//  
//  Configure the adjustment executive using m_oaxKwl
//  
//*****************************************************************************
void ossimAutRegUtil::configureAdjustmentExecutive()
{
   static const char MODULE[] = "ossimAutRegUtil::configureAdjustmentExecutive";

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   if ( m_oaxKwl.valid() )
   {

      if (m_oaxKwl->find(OAX_MAX_ITERATIONS_KW.c_str()))
      {
         ossimString maxIter = m_oaxKwl->findKey(OAX_MAX_ITERATIONS_KW);
         if (maxIter.size())
         {
            ossim_int32 max  = maxIter.toInt32();
            m_adjExec->setMaxIter(max);
         }
      }

      if (m_oaxKwl->find(OAX_CONVERGENCE_CRITERIA_KW.c_str()))
      {
         ossimString conv = m_oaxKwl->findKey(OAX_CONVERGENCE_CRITERIA_KW);
         if (conv.size())
         {
            ossim_float64 convPerc = conv.toFloat64();
            m_adjExec->setConvCriteria(convPerc);
         }
      }

      if (m_oaxKwl->find(OAX_GROUND_SIGMA_KW.c_str()))
      {
         ossimString conv = m_oaxKwl->findKey(OAX_GROUND_SIGMA_KW);
         if (conv.size())
         {
            ossim_float64 gndSigma = conv.toFloat64();
            m_LatLonHgtSigmas[0] = gndSigma;
            m_LatLonHgtSigmas[1] = gndSigma;
            m_LatLonHgtSigmas[2] = gndSigma;
         }
      }

      if (m_oaxKwl->find(OAX_CONTROL_SIGMA_KW.c_str()))
      {
         ossimString conv = m_oaxKwl->findKey(OAX_CONTROL_SIGMA_KW);
         if (conv.size())
         {
            ossim_float64 gndSigma = conv.toFloat64();
            m_LatLonHgtControlSigmas[0] = gndSigma;
            m_LatLonHgtControlSigmas[1] = gndSigma;
            m_LatLonHgtControlSigmas[2] = gndSigma;
         }
      }

      if (m_oaxKwl->find(OAX_LAT_SIGMA_KW.c_str()))
      {
         ossimString conv = m_oaxKwl->findKey(OAX_LAT_SIGMA_KW);
         if (conv.size())
         {
            ossim_float64 gndLatSigma = conv.toFloat64();
            m_LatLonHgtSigmas[0] = gndLatSigma;
         }
      }

      if (m_oaxKwl->find(OAX_LON_SIGMA_KW.c_str()))
      {
         ossimString conv = m_oaxKwl->findKey(OAX_LON_SIGMA_KW);
         if (conv.size())
         {
            ossim_float64 gndLonSigma = conv.toFloat64();
            m_LatLonHgtSigmas[1] = gndLonSigma;
         }
      }

      if (m_oaxKwl->find(OAX_HGT_SIGMA_KW.c_str()))
      {
         ossimString conv = m_oaxKwl->findKey(OAX_HGT_SIGMA_KW);
         if (conv.size())
         {
            ossim_float64 gndHgtSigma = conv.toFloat64();
            m_LatLonHgtSigmas[2] = gndHgtSigma;
         }
      }

   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }   
}


//*****************************************************************************
//  METHOD: ossimAutRegUtil::execute()
//  
//  Execute registration operation.
//*****************************************************************************
void ossimAutRegUtil::execute()
{
   static const char MODULE[] = "ossimAutRegUtil::execute";

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }


   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Initialize the measurement generator
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   *m_rep<<"\n\n >> Tie point collection follows...\n"<<endl;
   m_tGen->init(*m_rep);

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Configure the measurement generator
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   configureTieMeasGenerator();


   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Load ossimImageSource vector
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   std::vector<ossimImageSource*> src;
   src.push_back(m_imgLayer[0].get());
   src.push_back(m_imgLayer[1].get());


   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Set correlation areas
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   int primeIndex = 0; //TODO
   m_tGen->setBox(m_roiRects, primeIndex, src);


   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Run the OpenCV detector/extractor/matcher to collect tiepoints
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   if (m_tGen->run())
   {

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Close match window if active
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      if (m_showMatchWindow)
      {
         bool waitKey = true;
         m_tGen->closeCvWindow(waitKey);
      }

      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      // Check for adjustable parameter interface before adjustment
      //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      ossimAdjustableParameterInterface* iface1 = m_geom[0]->getAdjustableParameterInterface();
      ossimAdjustableParameterInterface* iface2 = m_geom[1]->getAdjustableParameterInterface();
      
      if (iface1 && iface2)
      {

         //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         // Create observation set
         //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         m_obsSet = new ossimObservationSet;

         //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         // Create adjustment executive
         //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         m_adjExec = new ossimAdjustmentExecutive(*m_rep);

         //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         // Configure adjustment executive
         //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         configureAdjustmentExecutive();

         //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         // Load measurements into the observation set
         //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         if (populateObsSet())
         {
   
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // Perform adjustment
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            bool solutionOK = false;
         
            *m_rep<<"\n\n >> Adjustment summary follows...\n"<<endl;
            if (m_adjExec->initializeSolution(*m_obsSet))
            {
               solutionOK = m_adjExec->runSolution();
               m_adjExec->summarizeSolution();
               if (solutionOK)
               {
                  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                  // Write updated geometry files
                  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                  saveAdjustment();

                  if (traceDebug())
                  {
                     ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << "Solution complete...\n";
                  }   
               }
               else
               {
                  throw ossimException("m_adjExec->runSolution() failed...");
               } //end if m_adjExec->runSolution()
            }
            else
            {
               throw ossimException("m_adjExec->initializeSolution() failed...");
            } //end if m_adjExec->initializeSolution()
      
         } 
         else
         {
            throw ossimException("populateObsSet() failed...");
         } //end if (populateObsSet())

      }
      else
      {
         throw ossimException("ossimAdjustableParameterInterface missing...");
      } //end if (iface1 && iface2)

   }
   else
   {
      throw ossimException( "m_tGen->run() failed...");
   } //end if (m_tGen->run())
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }   
}


void ossimAutRegUtil::saveAdjustment()
{
   ossimString ts;
   ossim::getFormattedTime("ossim-autreg_%a_%m.%d.%y_%H:%M:%S", false, ts);
 
   ossimAdjustmentInfo* adjInfo = new ossimAdjustmentInfo;

   for (int img=0; img<2; ++img)
   {
      if (!m_controlImage[img])
      {
         // Get the adjustment info from the bundle adjustment via m_obsSet
         ossimFilename filename = m_imgLayer[img]->getImageHandler()->getFilename();

         ossimAdjustableParameterInterface* adjParIface;

         for (ossim_uint32 ii=0; ii<m_obsSet->numImages(); ++ii)
         {
            if (filename == m_obsSet->imageFile(ii))
               adjParIface = m_obsSet->getImageGeom(ii)->getAdjustableParameterInterface();
         }

         adjParIface->setAdjustmentDescription(ts);
         adjParIface->getAdjustment(*adjInfo);
   
         // Grab adjustable parameter interface and update it with the adjustment info
         ossimAdjustableParameterInterface* iface = m_geom[img]->getAdjustableParameterInterface();
         iface->setAdjustment(*adjInfo, true);

         if (iface)
         {
            ossimFilename fn = m_imgLayer[img]->getImageHandler()->createDefaultGeometryFilename();
// TODO ***********
//  dirty flag????
// fn += "_test";
// TODO ***********
            ossimKeywordlist kwl;
            iface->getBaseObject()->saveState(kwl);
            kwl.write(fn);
         }
      }
   }

   delete adjInfo;
}


void ossimAutRegUtil::getOutputFilename(ossimFilename& f) const
{
   f.string() = m_kwl->findKey( std::string(ossimKeywordNames::OUTPUT_FILE_KW) );
}


bool ossimAutRegUtil::isSrcFile(const ossimFilename& file) const
{
   bool result = false;
   ossimString ext = file.ext();
   ext.downcase();
   if ( ext == "src" )
   {
      result = true;
   }
   return result;
}


void ossimAutRegUtil::usage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimInit::instance()->addOptions(ap);
   
   // Set app name.
   std::string appName = ap.getApplicationName();
   ap.getApplicationUsage()->setApplicationName( ossimString( appName ) );

   // Add options.
   addArguments(ap);
   
   // Write usage.
   ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));

   // Keeping single line in tact for examples for cut and paste purposes.
   ossimNotify(ossimNotifyLevel_INFO)
      << "NOTES:\n"
      << "1) Note 1 content\n"
      << "   Note 1 place holder continued...\n"
      << "2) Note 2 content\n"
      << "   Note 2 place holder continued...\n"
      
      << "\nExample commands:\n"

      << "\n// Co-register image1 and image2 with 512x512 ROIs centered on point:\n"
      << appName << " --op coreg --img_1 image1.ntf --img_2 image2.ntf --roi-center-llwh 28.097255037664400 -80.630762190246145 512 512\n"

      << "\n// Register image2 to control image1 (master): (TBD... not working)\n"
      << appName << " --op masreg -img_m image1.ntf -img_s image2.ntf -cfg configFile.kwl\n"

      << "\n// Above command where all options are in a keyword list:\n"
      << appName << " --options autreg-options.kwl\n"
      << std::endl;
}


void  ossimAutRegUtil::initializeSrcKwl()
{
   static const char MODULE[] = "ossimAutRegUtil::initializeSrcKwl";
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " entered...\n";
   }

   std::string value = m_kwl->findKey(std::string(SRC_FILE_KW));
   if ( value.size() )
   {
      m_srcKwl = new ossimKeywordlist();
      m_srcKwl->setExpandEnvVarsFlag(true);
      if ( m_srcKwl->addFile( value.c_str() ) == false )
      {
         m_srcKwl = 0;
      }
   }
   else
   {
      m_srcKwl = 0; 
   }

   if ( traceDebug() )
   {
      if ( m_srcKwl.valid() )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "src keyword list:\n" << *(m_srcKwl.get()) << "\n";
      }
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exited...\n";
   }
}


void  ossimAutRegUtil::initializeOcvKwl()
{
   static const char MODULE[] = "ossimAutRegUtil::initializeOcvKwl";
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " entered...\n";
   }

   std::string value = m_kwl->findKey(std::string(OCV_CONFIG_FILE_KW));
   if ( value.size() )
   {
      m_ocvKwl = new ossimKeywordlist();
      m_ocvKwl->setExpandEnvVarsFlag(true);
      if ( m_ocvKwl->addFile( value.c_str() ) == false )
      {
         m_ocvKwl = 0;
      }
   }
   else
   {
      m_ocvKwl = 0; 
   }

   if ( traceDebug() )
   {
      if ( m_ocvKwl.valid() )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ocv keyword list:\n" << *(m_ocvKwl.get()) << "\n";
      }
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exited...\n";
   }
}


void  ossimAutRegUtil::initializeOaxKwl()
{
   static const char MODULE[] = "ossimAutRegUtil::initializeOaxKwl";
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " entered...\n";
   }

   std::string value = m_kwl->findKey(std::string(OAX_CONFIG_FILE_KW));
   if ( value.size() )
   {
      m_oaxKwl = new ossimKeywordlist();
      m_oaxKwl->setExpandEnvVarsFlag(true);
      if ( m_oaxKwl->addFile( value.c_str() ) == false )
      {
         m_oaxKwl = 0;
      }
   }
   else
   {
      m_oaxKwl = 0; 
   }

   if ( traceDebug() )
   {
      if ( m_oaxKwl.valid() )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "oax keyword list:\n" << *(m_oaxKwl.get()) << "\n";
      }
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exited...\n";
   }
}


bool ossimAutRegUtil::addImgSource(const ossimFilename& file, ossim_uint32 entryIndex)
{
   static const char MODULE[] = "ossimAutRegUtil::addImgSource";

   bool addOK = false;
   
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " entered...\nFile: " << file << "\n";
   }

   ossimRefPtr<ossimSingleImageChain> ic = createChain(file, entryIndex);
   if ( ic.valid() )
   {
      m_imgLayer.push_back(ic);
      addOK = true;
   }

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exiting...\n";
   }

   return addOK;
}


void ossimAutRegUtil::setReaderProps( ossimImageHandler* ih ) const
{
   if ( ih && m_kwl.valid() )
   {
      ossim_uint32 count = m_kwl->numberOf( READER_PROPERTY_KW.c_str() );
      for (ossim_uint32 i = 0; i < count; ++i)
      {
         ossimString key = READER_PROPERTY_KW;
         key += ossimString::toString(i);
         ossimString value = m_kwl->findKey( key.string() );
         if ( value.size() )
         {
            std::vector<ossimString> splitArray;
            value.split(splitArray, "=");
            if(splitArray.size() == 2)
            {
               ossimRefPtr<ossimProperty> prop =
                  new ossimStringProperty(splitArray[0], splitArray[1]);
               
               ih->setProperty( prop );
            }
         }
      }
   }
}


bool ossimAutRegUtil::setChainEntry(
   ossimRefPtr<ossimSingleImageChain>& chain, ossim_uint32 entryIndex ) const
{
   bool result = false;
   if ( chain.valid() )
   {
      ossimRefPtr<ossimImageHandler> ih = chain->getImageHandler();
      if ( ih.valid() )
      {
         result = ih->setCurrentEntry( entryIndex );
      }
   }
   return result;
}

ossimRefPtr<ossimSingleImageChain> ossimAutRegUtil::createChain(const ossimFilename& file,
                                                                 ossim_uint32 entryIndex) const
{
   static const char MODULE[] = "ossimAutRegUtil::createChain(const ossimFilename&";

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " entered..."
         << "\nfile: " << file
         << "\nentry: " << entryIndex
         << "\n";
   }   
   
   ossimRefPtr<ossimSingleImageChain> ic = 0;

   if ( file.size() )
   {
      if ( file.exists() )
      {
         ic = new ossimSingleImageChain;
         if ( ic->open( file ) )
         {
            // Set any reader props:
            setReaderProps( ic->getImageHandler().get() );
            
            // we can't guarantee the state of the image handler at this point so
            // let's make sure that the entry is always set to the requested location
            //  On Cib/Cadrg we were having problems.  Removed the compare for entry 0
            //
             if ( setChainEntry( ic, entryIndex ) == false )
             {
                std::ostringstream errMsg;
                errMsg << MODULE << " ERROR:\nEntry " << entryIndex << " out of range!"
                       << std::endl;
                throw ossimException( errMsg.str() );
             }

            
            // Always have resampler cache.
            ic->setAddResamplerCacheFlag(true);

            //---
            // Don't need a chain cache as we're doing a sequential write.  So the same tile
            // should never be visited more than once.
            //---
            ic->setAddChainCacheFlag(false);


            // // Brightness, contrast. Note in same filter.
            // if ( hasBrightnesContrastOperation() )
            // {
            //    ic->setBrightnessContrastFlag(true);
            // }

            // std::string sharpnessMode = getSharpenMode();
            // if ( sharpnessMode.size() )
            // {
            //    ic->setSharpenFlag(true);
            // }

            // Create the chain.
            ic->createRenderedChain();


            // // Histogram setup.
            // if ( hasHistogramOperation() )
            // {
            //    setupChainHistogram( ic );
            // }

            // // Brightness contrast setup:
            // if ( hasBrightnesContrastOperation() )
            // {
            //    // Assumption bright contrast filter in chain:
               
            //    ossim_float64 value = getBrightness();
            //    ic->getBrightnessContrast()->setBrightness( value );
               
            //    value = getContrast();
            //    ic->getBrightnessContrast()->setContrast( value );
            // }

            // // Sharpness:
            // if ( sharpnessMode.size() )
            // {
            //    if ( sharpnessMode == "light" )
            //    {
            //       ic->getSharpenFilter()->setWidthAndSigma( 3, 0.5 );
            //    }
            //    else if ( sharpnessMode == "heavy" )
            //    {
            //       ic->getSharpenFilter()->setWidthAndSigma( 5, 1.0 );
            //    }
            // }
         }
      }
   }

   if ( ic.valid() == false )
   {
      std::string errMsg = "Could not open: ";
      errMsg += file.string();
      throw ossimException(errMsg); 
   }

   if ( traceDebug() )
   {
      ossimKeywordlist kwl;
      ic->saveState(kwl, 0);

      ossimNotify(ossimNotifyLevel_DEBUG)
         << "chain:\n" << kwl << "\n"
         << MODULE << " exiting...\n";
   }   

   return ic;
}


void ossimAutRegUtil::createIdentityProjection(int idx)
{
   static const char MODULE[] = "ossimAutRegUtil::createIdentityProjection";
   
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   // Get the singe image chain.  Should be only one.
   ossimRefPtr<ossimSingleImageChain> sic = 0;
   if ( m_imgLayer.size() )
   {
      sic = m_imgLayer[idx];
   }

   if ( sic.valid() )
   {
      // Get the image handler.
      ossimRefPtr<ossimImageHandler>  ih = sic->getImageHandler();

      // Resampler:
      ossimRefPtr<ossimImageRenderer> resampler = sic->getImageRenderer();

      if ( ih.valid() )
      {
         //---
         // Get the geometry from the image handler.  Since we're in "identity"
         // mode use the inputs for the outputs.
         //---
         m_geom[idx] = ih->getImageGeometry();

         if ( m_geom[idx].valid() )
         {
            // Get the image projection.
            ossimRefPtr<ossimProjection> proj = m_geom[idx]->getProjection();
            if ( proj.valid() )
            {
               ossim_float64 rotation = 0.0;
               
               ossimDrect rect;
               m_geom[idx]->getBoundingRect(rect);
               ossimDpt midPt = rect.midPoint();
               
               if ( traceDebug() )
               {
                  ossimNotify(ossimNotifyLevel_DEBUG)
                     << MODULE
                     << "\nAffine transform parameters:"
                     << "\nrotation:  " << rotation
                     << "\nmid point: " << midPt << std::endl;
               }
               
               m_ivt[idx] = new ossimImageViewAffineTransform(-rotation,
                                                              1.0, 1.0, // image space scale x and y
                                                              1.0, 1.0, //scale x and y
                                                              0.0, 0.0, // translate x,y
                                                              midPt.x, midPt.y); // pivot point
               
               resampler->setImageViewTransform( m_ivt[idx].get() );
            }

         } // Matches: if ( m_geom.valid() )
         
      } // Matches: if ( ih.valid() )
      
   } // Matches: if ( sic.valid() 
   
} // End: createIdentityProjection()


bool ossimAutRegUtil::keyIsTrue(ossimRefPtr<ossimKeywordlist> kwl, const std::string& key ) const
{
   bool result = false;
   if ( kwl.valid() )
   {
      std::string value = kwl->findKey( key );
      if ( value.size() )
      {
         result = ossimString(value).toBool();
      }
   }
   return result;
}

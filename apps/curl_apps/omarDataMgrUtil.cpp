//----------------------------------------------------------------------------
// File: omarDataMgrUtil.cpp
// 
// License: See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: omarDataMgrUtil class definition
//
// Utility class for processing images recursively and calling OMAR
// services upon successful open.
// 
//----------------------------------------------------------------------------
// $Id$

#include "omarDataMgrUtil.h"
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimPreferences.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/util/ossimFileWalker.h>

#include <curl/curl.h>
#include <iomanip>
#include <string>
#include <vector>

static std::string CLEAN_KW                    = "clean";
static std::string DUMP_FILTERED_IMAGES_KW     = "dump_filtered_images";
static std::string FALSE_KW                    = "false";
static std::string FILE_KW                     = "file";
static std::string OVERRIDE_FILTERED_IMAGES_KW = "override_filtered_images";
static std::string SERVICE_KW                  = "service";
static std::string THREADS_KW                  = "threads";
static std::string TRUE_KW                     = "true";
static std::string URL_KW                      = "url";

// Static trace for debugging.  Use -T omarDataMgrUtil to turn on.
static ossimTrace traceDebug = ossimTrace("omarDataMgrUtil:debug");

omarDataMgrUtil::omarDataMgrUtil()
   :
   ossimReferenced(),
   ossimFileProcessorInterface(),
   m_kwl( new ossimKeywordlist() ),
   m_fileWalker(0),
   m_mutex(),
   m_imageUtil(0),
   m_errorStatus(0),
   m_filteredImages(0)
{
}

omarDataMgrUtil::~omarDataMgrUtil()
{
   if ( m_fileWalker )
   {
      delete m_fileWalker;
      m_fileWalker = 0;
   }
}

void omarDataMgrUtil::addArguments(ossimArgumentParser& ap)
{
   // Set the general usage:
   ossimApplicationUsage* au = ap.getApplicationUsage();
   ossimString usageString = ap.getApplicationName();
   usageString += " [options] <service> <file(s)-or-directory-to-walk>";
   au->setCommandLineUsage(usageString);

   // Set the command line options:
   au->addCommandLineOption("--clean", "Cleans/removes image file and associated files from file system if present.\n\"remove\" option only.\nLooks for associated files of image, e.g. .ovr, .his, .omd, .geom\nCAUTION: This command is irreversible once envoked!");
   
   au->addCommandLineOption("--dump-filtered-image-list", "Outputs list of filtered images.");

   au->addCommandLineOption("--override-filtered-images", "Allows processing of file that is in the filtered image list.");
   
   au->addCommandLineOption("--threads", "<threads> The number of threads to use. (default=1) Note a default can be set in your ossim preferences file by setting the key \"ossim_threads\".");
   
   au->addCommandLineOption("--preproc", "Enables the use of the preproc utility to build overviews and histograms");

   au->addCommandLineOption( "-u", "<url> URL, e.g. \"http://omar.osssim.org/omar\"\n(default= url \"http://localhost:8080/omar\"" );

   ossimRefPtr<ossimImageUtil> imageUtil = new ossimImageUtil();
   imageUtil->addOptions(au);
   imageUtil = 0;
   
} // void omarDataMgrUtil::addArguments(ossimArgumentParser& ap)

bool omarDataMgrUtil::initialize(ossimArgumentParser& ap)
{
   static const char M[] = "omarDataMgrUtil::initialize(ossimArgumentParser&)";
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   }

   bool result = true;

   if ( (ap.argc() == 1) || ap.read("-h") || ap.read("--help") )
   {
      usage(ap);

      // continue_after_init to false
      result = false;
   }
   else
   {
      //---
      // Start with clean options keyword list.
      //---
      m_kwl->clear();

      while ( 1 ) //  While forever loop...
      {
         // Used throughout below:
         std::string ts1;
         ossimArgumentParser::ossimParameter sp1(ts1);

         if( ap.read("--clean") )
         {
            addOption( CLEAN_KW, TRUE_KW );
            if ( ap.argc() < 3 )
            {
               break;
            }
         }

         if( ap.read("--dump-filtered-image-list") )
         {
            setDumpFilteredImageListFlag( true );
            if ( ap.argc() < 3 )
            {
               break;
            }
         }

         if( ap.read("--override-filtered-images") )
         {
            setOverrideFilteredImagesFlag( true );
            if ( ap.argc() < 3 )
            {
               break;
            }
         }

         if( ap.read("--threads", sp1) )
         {
            m_kwl->addPair( THREADS_KW, ts1 );
            if ( ap.argc() < 3 )
            {
               break;
            }
         }

         if( ap.read("-u", sp1) )
         {
            m_kwl->addPair( URL_KW, ts1 );
            if ( ap.argc() < 3 )
            {
               break;
            }
         }
         
         if( ap.read("--preproc"))
         {
            m_imageUtil = new ossimImageUtil();
            m_imageUtil->initialize(ap);
         }

         if ( ap.argc() < 3 )
         {
            break;
         }

         // End of arg parsing.
         ap.reportRemainingOptionsAsUnrecognized();
         if ( ap.errors() )
         {
            ap.writeErrorMessages(ossimNotify(ossimNotifyLevel_NOTICE));
            std::string errMsg = "Unknown option...";
            throw ossimException(errMsg);
         }

         break; // Break from while forever.
         
      } // End while (forever) loop.
      
      if( ap.argc() > 2 )
      {
         // First arg should be service, e.g. "add", "remove".
         std::string value = ap[1];
         m_kwl->addPair( SERVICE_KW, value );

         // Check for "clean" used with "add" and issue warning???
         if ( getCleanFlag() && ( value == "add" ) )
         {
            ossimNotify(ossimNotifyLevel_NOTICE)
               << "NOTICE: Using --clean with \"add\" service is illogical!\n";
         }

         // The remaining args should be files to process.
         for (ossim_int32 i = 1; i < (ap.argc()-1); ++i)
         {
            ossimString kw = "file";
            kw += ossimString::toString(i);
            std::string value = ap[i+1];
            m_kwl->addPair(kw.string(), value, true);
         }
      }
      else
      {
         if ( getDumpFilterImagesFlag() )
         {
            // Caller wants to see filtered image names:
            dumpFilteredImageList();
         }
         else
         {
            usage(ap);
            result = false;
         }
      }

   } // not usage

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "m_kwl:\n" << *(m_kwl.get()) << "\n"
         << M << " exit result = " << (result?"true":"false")
         << "\n";
   }
   
   return result;
}

ossim_int32 omarDataMgrUtil::execute()
{
   static const char M[] = "omarDataMgrUtil::execute()";
   
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   }

   if ( getDumpFilterImagesFlag() )
   {
      // Caller wants to see filtered image names:
      if ( m_filteredImages.empty() )
      {
         initializeDefaultFilterList();
      }
      
      dumpFilteredImageList();
   }
   
   // Get the number of "file*" keywords.
   ossim_uint32 fileCount = m_kwl->numberOf("file");

   if ( fileCount )
   {
      if ( !m_fileWalker )
      {
         m_fileWalker = new ossimFileWalker();
      }
      
      if ( !getOverrideFilteredImagesFlag() && m_filteredImages.empty() )
      {
         initializeDefaultFilterList();
      }
      
      m_fileWalker->initializeDefaultFilterList();

      //---
      // Passing getNumberOfThreads() to ossimFileWalker::setNumberOfThreads was
      // kicking up a coverity scan error:
      // Passing tainted variable "this->getNumberOfThreads()" to a tainted sink.
      // drb
      //---
      ossim_uint32 threads = getNumberOfThreads();
      m_fileWalker->setNumberOfThreads( threads );

      // Must set this so we can stop recursion on directory based images.
      m_fileWalker->setWaitOnDirFlag( true );

      // This links the file walker back to our "processFile" method.
      m_fileWalker->setFileProcessor( this );
      
      // Wrap in try catch block as excptions can be thrown under the hood.
      try
      {
         // Get the list of files passed to us:
         std::vector<ossimFilename> files;
         ossim_uint32 processedFiles = 0;;
         ossim_uint32 i = 0;
         while ( processedFiles < fileCount )
         {
            ossimString kw = FILE_KW;
            kw += ossimString::toString(i);
            std::string lookup = m_kwl->findKey( kw.string() );
            if ( lookup.size() )
            {
               files.push_back( ossimFilename(lookup) );
               ++processedFiles;
            }
            
            ++i;
            if ( i > (fileCount + 100) ) break;
         }

         // Process the files:
         m_fileWalker->walk( files ); 
      }
      catch (const ossimException& e)
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "Caught exception: " << e.what() << endl;
         setErrorStatus( ossimErrorCodes::OSSIM_ERROR );
      }
      
   } // if ( fileCount )

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << M << " exit status: " << m_errorStatus << std::endl;
   }
   
   // Zero is good, non zero is bad.
   return m_errorStatus; 
}

//---
// This method is called back by the ossimFileWalker::walk method for each file it finds that it
// deems can be processed.
//---
void omarDataMgrUtil::processFile(const ossimFilename& file)
{
   static const char M[] = "omarDataMgrUtil::processFile";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << M << " entered...\n" << "\nfile:    " << file << "\n";
   }
   
   if ( file.size() )
   {
      bool processFileFlag = true;
      if ( !getOverrideFilteredImagesFlag() )
      {
         processFileFlag = !isFiltered( file );
      }
      
      if ( processFileFlag )
      {
         omarDataMgrUtil::OmarDataMgrUtilService service = getService();

         if ( service == omarDataMgrUtil::ADD_RASTER )
         {
            callAddRasterService( file );
         }
         else if ( service == omarDataMgrUtil::REMOVE_RASTER )
         {
            callRemoveRasterService( file );
         }
         else
         {
            std::string s;
            getService( s );
            ossimNotify(ossimNotifyLevel_WARN)
               << "Unhandled service: " << s << "\n";
         }
      }
      else
      {
         ossimNotify(ossimNotifyLevel_NOTICE)
            << "Filtered file, not processing: " << file << std::endl;
      }
      
   } // Matches: if ( file.size() )
   
   
   if(traceDebug())
   {
      // Since ossimFileWalker is threaded output the file so we know which job exited.
      ossimNotify(ossimNotifyLevel_DEBUG) << M << "\nfile: " << file << "\nexited...\n";
   }
   
} // End: omarDataMgrUtil::processFile(const ossimFilename& file)

void omarDataMgrUtil::usage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimInit::instance()->addOptions(ap);
   
   // Set app name.
   ap.getApplicationUsage()->setApplicationName(ap.getApplicationName());

   // Add options.
   addArguments(ap);
   
   // Write usage.
   ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
   
   ossimNotify(ossimNotifyLevel_INFO)
      << "\nValid services: \"add\" and \"remove\"."
      << "\nExample usage:\n\n"

      << "Using default url \"http://localhost:8080/omar\":\n"
      << ap.getApplicationName()
      << " add 5V090205M0001912264B220000100072M_001508507.ntf\n\n"

      << "Build overviews, histogram, and add to database.\n"
      << ap.getApplicationName()
      << " --preproc --ot ossim_kakadu_nitf_j2k --ch add "
      << "5V090205M0001912264B220000100072M_001508507.ntf\n\n"

     << "Passing in url:\n"
      << ap.getApplicationName()
      << " -u http://your_url/omar add 5V090205M0001912264B220000100072M_001508507.ntf\n\n"

      << "Adding all images in a directory using 32 threads:\n"
      << ap.getApplicationName()
      << " --threads 32 add /data1/imagery/2015/09/28/0000\n\n"
      
      << "Removing file from database:\n"
      << ap.getApplicationName()
      << " remove 5V090205M0001912264B220000100072M_001508507.ntf\n\n"

      << "Removing file from database and file system:\n"
      << ap.getApplicationName()
      << " --clean remove 5V090205M0001912264B220000100072M_001508507.ntf\n\n"

      << "Removing all images in a directory using 32 threads:\n"
      << ap.getApplicationName()
      << " --clean --threads 32 remove /data1/imagery/2015/09/28/0000\n"

      << std::endl;
}

// Private method:
bool omarDataMgrUtil::isDirectoryBasedImage(const ossimImageHandler* ih) const
{
   bool result = false;
   if ( ih )
   {
      // Get the image handler name.
      ossimString imageHandlerName = ih->getClassName();
      if ( (imageHandlerName == "ossimAdrgTileSource") ||
           (imageHandlerName == "ossimCibCadrgTileSource") )  
      {
         result = true;
      }
   }
   return result;
}

void omarDataMgrUtil::setNumberOfThreads( ossim_uint32 threads )
{
   addOption( THREADS_KW, threads );
}

void omarDataMgrUtil::setNumberOfThreads( const std::string& threads )
{
   addOption( THREADS_KW, threads );
}

ossim_uint32 omarDataMgrUtil::getNumberOfThreads() const
{
   ossim_uint32 result = 1;
   std::string lookup = m_kwl->findKey( THREADS_KW );
   if ( lookup.size() )
   {
      result = ossimString(lookup).toUInt32();
   }
   else
   {
      result = ossim::getNumberOfThreads();
   }
   return result;
}

void omarDataMgrUtil::addOption( const std::string& key, ossim_uint32 value )
{
   addOption( key, ossimString::toString( value ).string() );
}

void omarDataMgrUtil::addOption(  const std::string& key, const std::string& value )
{
   m_mutex.lock();
   if ( m_kwl.valid() )
   {
      if ( key.size() && value.size() )
      {
         m_kwl->addPair( key, value );
      }
   }
   m_mutex.unlock();
}

void omarDataMgrUtil::setErrorStatus( ossim_int32 status )
{
   m_mutex.lock();
   m_errorStatus = status;
   m_mutex.unlock();
}

omarDataMgrUtil::OmarDataMgrUtilService omarDataMgrUtil::getService() const
{
   omarDataMgrUtil::OmarDataMgrUtilService service = omarDataMgrUtil::UNKNOWN;
   
   std::string value = m_kwl->findKey( SERVICE_KW );
   if ( value.size() )
   {
      ossimString s = value;
      s.downcase();

      if ( s == "add" )
      {
         service = omarDataMgrUtil::ADD_RASTER;
      }
      else if ( (s == "remove") || (s == "rm") || (s == "delete") || (s == "erase") )
      {
         service = omarDataMgrUtil::REMOVE_RASTER;
      }
   }

   return service;
}

void omarDataMgrUtil::getService(std::string& serviceString) const
{
   omarDataMgrUtil::OmarDataMgrUtilService service = getService();
   
   if ( service == omarDataMgrUtil::ADD_RASTER )
   {
      serviceString = "/dataManager/addRaster";
   }
   else if ( service == omarDataMgrUtil::REMOVE_RASTER )
   {
      serviceString = "/dataManager/removeRaster";
   }
   else
   {
      serviceString.clear();
   }
}

void omarDataMgrUtil::getUrl( std::string& url ) const
{
   url = m_kwl->findKey( URL_KW );
   if ( url.empty() )
   {
      url = "http://localhost:8080/omar"; // default url
   }
}

bool omarDataMgrUtil::callAddRasterService( const ossimFilename& file )
{
   static const char M[] = "omarDataMgrUtil::callAddRasterService";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << M << " entered...\n" << "\nfile:    " << file << "\n";
   }
   
   bool result = false;

   // Must be able to open:
   m_mutex.lock();
   ossimRefPtr<ossimImageHandler> ih =
      ossimImageHandlerRegistry::instance()->open(file, true, true);
   m_mutex.unlock();
   
   if ( ih.valid() && !ih->hasError() )
   {
      if ( isDirectoryBasedImage( ih.get() ) )
      {
         // Tell the walker not to recurse this directory.
         m_mutex.lock();
         m_fileWalker->setRecurseFlag(false);
         m_mutex.unlock();
      }
            
      // Must have geometry:
      ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
      if( geom.valid() )
      {
         // Geometry object must have a valid projection:
         ossimRefPtr<ossimProjection> proj = geom->getProjection();
         if ( proj.valid() )
         {
            ossimNotify(ossimNotifyLevel_NOTICE)
               << "Processing file: " << file << std::endl;

            if( m_imageUtil.valid() )
            {
               m_imageUtil->processFile(file);
            }
            
            std::string service;
            getService( service );
            
            std::string url;
            getUrl( url );
            
            if ( service.size() && url.size() )
            {
               CURL* curl = curl_easy_init();
               if ( curl )
               {
                  // Data for POST:
                  std::string data = std::string("filename=") + file.string();
                  curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data.c_str() );
                  
                  // Create the URL string:
                  std::string urlString = url + service;
                  curl_easy_setopt( curl, CURLOPT_URL, urlString.c_str() );
                  
                  ossimNotify(ossimNotifyLevel_INFO)
                     << "data: " << data 
                     << "\nurl: " << urlString.c_str()
                     << std::endl;
                  
                  // Tell libcurl to follow redirection
                  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                  
                  // Run it:
                  CURLcode res = curl_easy_perform(curl);
                  
                  // Check for errors:
                  if ( res == CURLE_OK )
                  {
                     // Response code of the http transaction:
                     long respcode; 
                     res = curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
                     if ( res == CURLE_OK )
                     {
                        if ( respcode == 200 ) //  OK 200 "The request was fulfilled."
                        {
                           result = true; // Set return status for caller.
                        }
                     }
                  }
                  else
                  {
                     ossimNotify(ossimNotifyLevel_WARN)
                        << "curl_easy_perform() failed: \n"
                        << curl_easy_strerror(res)
                        << std::endl;
                     setErrorStatus( (ossim_int32)res );
                  }
                  
                  ossimNotify(ossimNotifyLevel_WARN) << std::endl;
                  
                  // Always cleanup:
                  curl_easy_cleanup(curl);
               }
               
               // Cleanup curl:
               curl_global_cleanup();
            
            } // Matches: if ( url.size() )

         }
         else
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << M << "\nCould create projection for: " << file << std::endl;
         }
      }
      else
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << M << "\nCould create geometry for: " << file << std::endl;
      }  
   }
   else
   {
      ossimNotify(ossimNotifyLevel_WARN) << M << "\nCould not open: " << file << std::endl;
   }
   
   if(traceDebug())
   {
      // Since ossimFileWalker is threaded output the file so we know which job exited.
      ossimNotify(ossimNotifyLevel_DEBUG) << M << "\nfile: " << file << "\nexited...\n";
   }
   
   return result;
   
} // End: omarDataMgrUtil::callAddRasterService( file )

bool omarDataMgrUtil::callRemoveRasterService( const ossimFilename& file )
{
   static const char M[] = "omarDataMgrUtil::callRemoveRasterService";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << M << " entered...\n" << "\nfile:    " << file << "\n";
   }
   
   bool result = false;
   
   std::string service;
   getService( service );
   
   std::string url;
   getUrl( url );
   
   if ( service.size() && url.size() )
   {
      CURL* curl = curl_easy_init();
      if ( curl )
      {
         // Data for POST:
         std::string data = std::string("filename=") + file.string();
         curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data.c_str() );
         
         // Create the URL string:
         std::string urlString = url + service;
         curl_easy_setopt( curl, CURLOPT_URL, urlString.c_str() );
         
         ossimNotify(ossimNotifyLevel_INFO)
            << "data: " << data 
            << "\nurl: " << urlString.c_str()
            << std::endl;
         
         // Tell libcurl to follow redirection
         curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
         
         // Run it:
         CURLcode res = curl_easy_perform(curl);
         
         // Check for errors:
         if ( res == CURLE_OK )
         {
            // Response code of the http transaction:
            long respcode; 
            res = curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE, &respcode);
            if ( res == CURLE_OK )
            {
               if ( respcode == 200 ) //  OK 200 "The request was fulfilled."
               {
                  result = true; // Set return status for caller.

                  // Check for clean flag:
                  if ( getCleanFlag() )
                  {
                     clean( file );
                  }
               }
            }
         }
         else
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << "curl_easy_perform() failed: \n"
               << curl_easy_strerror(res)
               << std::endl;
            setErrorStatus( (ossim_int32)res );
         }
         
         ossimNotify(ossimNotifyLevel_WARN) << std::endl;
         
         // Always cleanup:
         curl_easy_cleanup(curl);
      }
      
      // Cleanup curl:
      curl_global_cleanup();
      
   } // Matches: if ( service.size() )
   
   if(traceDebug())
   {
      // Since ossimFileWalker is threaded output the file so we know which job exited.
      ossimNotify(ossimNotifyLevel_DEBUG) << M << "\nfile: " << file << "\nexited...\n";
   }
   
   return result;
      
} // End: omarDataMgrUtil::callRemoveRasterService( file, clean )


void omarDataMgrUtil::clean( const ossimFilename& file ) const
{
   // Remove files from disk:
   ossimFilename f = file;
   
   // Base image file:
   if ( f.exists() )
   {
      ossimNotify(ossimNotifyLevel_NOTICE) << "Removing file: " << f << "\n";
      ossimFilename::remove( f );
   }
   
   // Overview:
   ossimString e = "ovr";
   f.setExtension( e );
   if ( f.exists() )
   {
      ossimNotify(ossimNotifyLevel_NOTICE) << "Removing file: " << f << "\n";
      ossimFilename::remove( f );
   }

   // Histogram:
   e = "his";
   f.setExtension( e );
   if ( f.exists() )
   {
      ossimNotify(ossimNotifyLevel_NOTICE) << "Removing file: " << f << "\n";
      ossimFilename::remove( f );
   }
   
   // Omd file:
   e = "omd";
   f.setExtension( e );
   if ( f.exists() )
   {
      ossimNotify(ossimNotifyLevel_NOTICE) << "Removing file: " << f << "\n";
      ossimFilename::remove( f );
   }
}

bool omarDataMgrUtil::isFiltered(const ossimFilename& file) const
{
   bool result = false;
   if ( file.size() )
   {
      // Strip full path to base name.
      std::string baseName = file.file().string();
      if ( baseName.size() )
      {
         std::vector<std::string>::const_iterator i = m_filteredImages.begin();
         while ( i != m_filteredImages.end() )
         {
            if ( baseName == (*i) )
            {
               result = true;
               break;
            }
            ++i;
         }
      }
   }
#if 0 /* Please leave for debug. (drb) */
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimFileWalker::isFiltered file " << (result?"filtered: ":"not filtered: ")
         << file << "\n";
   }
#endif
   
   return result;
}

bool omarDataMgrUtil::keyIsTrue( const std::string& key ) const
{
   bool result = false;
   if ( m_kwl.valid() )
   {
      std::string value = m_kwl->findKey( key );
      if ( value.size() )
      {
         result = ossimString(value).toBool();
      }
   }
   return result;
}

const std::vector<std::string>& omarDataMgrUtil::getFilteredImages() const
{
   return m_filteredImages;
}

std::vector<std::string>& omarDataMgrUtil::getFilteredImages()
{
   return m_filteredImages;
}

void omarDataMgrUtil::initializeDefaultFilterList()
{
   m_mutex.lock();

   // Common images to filter out, put most common first.
   m_filteredImages.push_back(std::string("icon.jpg"));
   m_filteredImages.push_back(std::string("logo.jpg"));
   m_filteredImages.push_back(std::string("preview.jpg"));
   
   m_mutex.unlock();
}

void omarDataMgrUtil::dumpFilteredImageList() const
{
   ossimNotify(ossimNotifyLevel_NOTICE) << "Filtered image list:\n";

   std::vector<std::string>::const_iterator i = m_filteredImages.begin();
   while ( i != m_filteredImages.end() )
   {
      ossimNotify(ossimNotifyLevel_NOTICE) << (*i) << "\n";
      ++i;
   }

   ossimNotify(ossimNotifyLevel_NOTICE) << std::endl;
}

void omarDataMgrUtil::setDumpFilteredImageListFlag( bool flag )
{
   // Add this for hasRequiredOverview method.
   std::string key   = DUMP_FILTERED_IMAGES_KW;
   std::string value = ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );
}

bool omarDataMgrUtil::getDumpFilterImagesFlag() const
{
   return keyIsTrue( DUMP_FILTERED_IMAGES_KW );
}

bool omarDataMgrUtil::getCleanFlag() const
{
   return keyIsTrue( CLEAN_KW );
}

void omarDataMgrUtil::setOverrideFilteredImagesFlag( bool flag )
{
   std::string key   = OVERRIDE_FILTERED_IMAGES_KW;
   std::string value = ( flag ? TRUE_KW : FALSE_KW );
   addOption( key, value );
}

bool omarDataMgrUtil::getOverrideFilteredImagesFlag() const
{
   return keyIsTrue( OVERRIDE_FILTERED_IMAGES_KW );
}

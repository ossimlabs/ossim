//----------------------------------------------------------------------------
// File: omarDataMgrUtil.h
// 
// License: See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: omarDataMgrUtil
//
// See class descriptions below for more.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef omarDataMgrUtil_HEADER
#define omarDataMgrUtil_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimFileProcessorInterface.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/util/ossimImageUtil.h>

#include <OpenThreads/Mutex>
#include <ostream>
#include <vector>

class ossimArgumentParser;
class ossimFileWalker;
class ossimGpt;
class ossimImageHandler;
class ossimPropertyInterface;

/**
 * @brief omarDataMgrUtil class.
 *
 * Utility class for processing images recursively and calling OMAR
 * dataManager services upon successful open.
 */
class omarDataMgrUtil :
   public ossimReferenced, public ossimFileProcessorInterface
{
public:

   enum OmarDataMgrUtilService
   {
      UNKNOWN       = 0,
      ADD_RASTER    = 1,
      REMOVE_RASTER = 2
   };
   
   /** default constructor */
   omarDataMgrUtil();

   /** virtual destructor */
   virtual ~omarDataMgrUtil();

   /**
    * @brief Adds application arguments to the argument parser.
    * @param ap Parser to add to.
    */
   void addArguments(ossimArgumentParser& ap);

   /**
    * @brief Initial method.
    *
    * Typically called from application prior to execute.  This parses
    * all options and put in keyword list m_kwl.
    * 
    * @param ap Arg parser to initialize from.
    *
    * @return true, indicating process should continue with execute.
    */
   bool initialize(ossimArgumentParser& ap);

   /**
    * @brief Execute method.
    *
    * This launches file walking mechanism.
    *
    * @return int, 0 = good, non-zero something happened.  Because this can
    * process multiple files a non-zero return may indicate just one file
    * did not complete, e.g. building overviews.
    * 
    * @note Throws ossimException on error.
    */
   ossim_int32 execute();

   /**
    * @brief ProcessFile method.
    *
    * Satisfies pure virtual ossimFileProcessorInterface::processFile.
    *
    * This method is linked to the ossimFileWalker::walk method via a callback
    * mechanism.  It is called by the ossimFileWalk (caller).  In turn this
    * class (callee) calls ossimFileWalker::setRecurseFlag and
    * ossimFileWalker::setAbortFlag to control the waking process.
    * 
    * @param file to process.
    */
   virtual void processFile(const ossimFilename& file);
   
   /**
    * @brief Set number of threads to use.
    *
    * This is only used in execute method if a directory is given to
    * application to walk.
    *
    * @param threads Defaults to 1 if THREADS_KW is not found.
    */
   void setNumberOfThreads( ossim_uint32 threads );
   void setNumberOfThreads( const std::string& threads );

   /** @return The list of filtered out files. */
   const std::vector<std::string>& getFilteredImages() const;

   /**
    * @brief Non const method to allow access for
    * adding or deleting extensions from the list.
    *
    * The list is used by the private isFiltered method to avoid trying to
    * process unwanted files.
    */
   std::vector<std::string>& getFilteredImages();

private:

   /** @return service as enumerated type. */
   omarDataMgrUtil::OmarDataMgrUtilService getService() const;

   /** @brief Gets service string, e.g. /dataManager/addRaster. */
   void getService(std::string& serviceString) const;

   /** @brief Gets url string, e.g. http://localhost:8080/omar */
   void getUrl(std::string& url) const;

   /**
    * @brief Runs the curl command to call addRaster service.
    * @param file Image file.
    * @return true if http status code of 200(OK); else, false
    */
   bool callAddRasterService( const ossimFilename& file );

   /**
    * @brief Runs the curl command to call removeRaster service.
    * @param file Image file.
    * @return true if http status code of 200(OK); else, false
    */
   bool callRemoveRasterService( const ossimFilename& file );

   /**
    * @brief Removes files from disk.
    *
    * Triggered by the --clean option.
    * 
    * Given: 
    * base file = foo.ntf
    *
    * Currently removes:
    * foo.ntf
    * foo.ovr
    * foo.his
    * foo.omd
    *
    * There are no checks for multi-entry images or directory based image
    * files.
    */
   void clean( const ossimFilename& file ) const;

   /**
    * @brief Convenience method to check file to see is if file should be
    * processed.
    *
    * @param f File to check.
    * 
    * @return true if f is in filter list, false if not.
    */
   bool isFiltered(const ossimFilename& f) const;
   
   /**
    * @brief Initializes the filter list with a default set of filtered out
    * file names.
    */
   void initializeDefaultFilterList();

   /** @brief Dumps filtered image list to std out. */
   void dumpFilteredImageList() const;
   
   /** @brief Initializes arg parser and outputs usage. */
   void usage(ossimArgumentParser& ap);
   
   /**
    * @return Threads to use.  Defaults to 1 if THREADS_KW is not found.
    */
   ossim_uint32 getNumberOfThreads() const;

   /**
    * @brief Adds option to m_kwl with mutex lock.
    * @param key
    * @param value
    */
   void addOption( const std::string& key, ossim_uint32 value );
   void addOption( const std::string& key, const std::string& value );
   
   /**
    * @brief Sets the m_errorStatus for return on execute.
    */
   void setErrorStatus( ossim_int32 status );

   /**
    * @brief Sets the dump filteredImageList flag.
    * @param flag
    */
   void setDumpFilteredImageListFlag( bool flag );
   
   /**
    * @return true if DUMP_FILTERED_IMAGES_KW key is found and value is true; else,
    * false.
    * 
    * DUMP_FILTERED_IMAGES_KW = "dump_filtered_images"
    */
   bool getDumpFilterImagesFlag() const;

   /**
    * @return true if CLEAN_KW key is found and value is true; else,
    * false.
    * 
    * CLEAN_KW = "clean"
    */
   bool getCleanFlag() const;

   /**
    * @brief Sets the override filtered images flag.
    * @param flag
    */
   void setOverrideFilteredImagesFlag( bool flag );
   
   /**
    * @return true if DUMP_FILTERED_IMAGES_KW key is found and value is true; else,
    * false.
    * 
    * DUMP_FILTERED_IMAGES_KW = "dump_filtered_images"
    */
   bool getOverrideFilteredImagesFlag() const;

   /**
    * @return true if file is a directory based image and the stager should go
    * on to next directory; false if stager should continue with directory.
    */
   bool isDirectoryBasedImage(const ossimImageHandler* ih) const;

   /** @return true if key is set to true; false, if not. */
   bool keyIsTrue( const std::string& key ) const;
   
   /** Holds all options passed into intialize except writer props. */
   ossimRefPtr<ossimKeywordlist> m_kwl;

   ossimFileWalker*   m_fileWalker;
   OpenThreads::Mutex m_mutex;

   ossimRefPtr<ossimImageUtil> m_imageUtil;
   ossim_int32 m_errorStatus;

   /** Hold images we never want to process. */
   std::vector<std::string> m_filteredImages; 
};

#endif /* #ifndef omarDataMgrUtil_HEADER */

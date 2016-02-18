//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimUtility_HEADER
#define ossimUtility_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimListenerManager.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimKeywordlist.h>

/*!
 *  Base class for all OSSIM utility applications.
 */
class OSSIM_DLL ossimUtility : public ossimObject,
                               public ossimProcessInterface,
                               public ossimListenerManager
{
public:
   ossimUtility();
   ~ossimUtility();

   /**
    * Initializes the aurgument parser with expected parameters and options. It does not output
    * anything. To see the usage, the caller will need to do something like:
    *
    *   ap.getApplicationUsage()->write(<ostream>);
    *
    * This base class has some common arguments to add. The derived class should call this
    * implementation in addition to setting its own arguments.
    */
   virtual void setUsage(ossimArgumentParser& ap);

   /**
    * Initializes from command line arguments.
    * This base class has some common arguments to read. The derived class should call this
    * implementation in addition to setting its own arguments.
    * @note Throws ossimException on error.
    */
   virtual void initialize(ossimArgumentParser& ap);

   /**
    * Reads processing params from KWL and prepares for execute. Returns TRUE if successful.
    * @note Throws ossimException on error.
    */
   virtual void initialize(const ossimKeywordlist& /*kwl*/) { }

   /**
    * Writes product to output file. Always returns true since using exception on error.
    * @note Throws ossimException on error.
    */
   virtual bool execute() = 0;

   /**
    * Disconnects and clears the DEM and image layers. Leaves OSSIM initialized.
    */
   virtual void clear() {}

   /**
    * Kills current (asynchronous) process. Defaults to do nothing.
    */
   virtual void abort() {}

   /**
    * Assigns a template keywordlist to string for initializing derived classes.
    */
   virtual void getKwlTemplate(ossimKeywordlist& kwl);

   /**
    * Outputs a JSON representation of the Utility's API.
    */
   void getAPI(std::string& out) const;
   std::string getAPI() const;

   virtual ossimObject* getObject() { return this; }
   virtual const ossimObject* getObject() const  { return this; }
   virtual ossimListenerManager* getManager()  { return this; };
   virtual ossimString getClassName() const { return "ossimUtility"; }

   /**
    * @brief Gets build date.
    * @param s String to initialize.
    */
   void getBuildDate(std::string& s) const;

   /**
    * @brief Gets revision.
    * @param s String to initialize.
    */
   void getRevision(std::string& s) const;

   /**
    * @brief Gets version.
    * @param s String to initialize.
    */
   void getVersion(std::string& s) const;

   // NOTE: The ossimUtilityFactory::getCapabilities() needs to access a brief description of each
   // utility. For convenience, the ossimUtility-derived (final) classes should declare a public
   // static member to hold the description string. See ossimViewshedUtility for an example.
   // static const char* DESCRIPTION;

private:
   /**
    * Used for reading text files of template and JSON API from disk ONLY.
    */
   bool readTextFile(const ossimFilename& filename, std::string& contents) const;
};

#endif

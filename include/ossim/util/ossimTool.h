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
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/JsonInterface.h>
#include <iostream>

/*!
 *  Base class for all OSSIM tool applications. These are utilities providing high-level
 *  functionality via the OSSIM library.
 */
class OSSIM_DLL ossimTool : public ossimObject, public ossim::JsonInterface

{
public:
   ossimTool();
   virtual ~ossimTool();

   /**
    * Initializes from command line arguments.
    * This base class has some common arguments to read. The derived class should call this
    * implementation in addition to setting its own arguments.
    * @param ap Represents command line.
    * @return FALSE if --help option requested or no params provided, so that derived classes can
    * exit without error.
    * @note Throws ossimException on error.
    */
   virtual bool initialize(ossimArgumentParser& ap);

   /**
    * Reads processing params from KWL and prepares for execute. Returns TRUE if successful.
    * @param kwl Full keyword-list representing state
    * @note Throws ossimException on error.
    */
   virtual void initialize(const ossimKeywordlist& kwl);

   /**
    * Reads processing params from JSON object provided. If all good, the object is ready for
    * subsequent call to execute().
    * @note Throws ossimException on error.
    */
   virtual void loadJSON(const Json::Value& /* json_request */) {};

   /**
    * Fetch product as JSON object when applicable
    * Always returns true since using exception on error.
    * @param json Returns non-empty object if valid response available.
    */
   virtual void saveJSON(Json::Value& json) const { json.clear(); }

   /**
    * Writes product to output file if applicable. The product may also beAlways returns true since using exception on error.
    * @note Throws ossimException on error.
    */
   virtual bool execute() = 0;

   /**
    * Fetch product as string (typically JSON) when applicable, otherwise passes back empty string.
    * Always returns true since using exception on error.
    * @return Returns non-empty string if valid response available.
    */
   virtual const std::string& getResponse() const { return m_response; }

   /**
    * Disconnects and clears the DEM and image layers. Leaves OSSIM initialized.
    */
   virtual void clear();

   /**
    * Kills current (asynchronous) process. Defaults to do nothing.
    */
   virtual void abort();

   /**
    * Assigns a template keywordlist to string for initializing derived classes.
    */
   virtual void getKwlTemplate(ossimKeywordlist& kwl);

   /**
    * Outputs a JSON representation of the Utility's API.
    */
   void getAPI(std::string& out) const;
   std::string getAPI() const;

   virtual ossimString getClassName() const;

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

   // NOTE: The ossimToolFactory::getCapabilities() needs to access a brief description of each
   // utility. For convenience, the ossimTool-derived (final) classes should declare a public
   // static member to hold the description string. See ossimViewshedTool for an example.
   // static const char* DESCRIPTION;

   /** Overrides base class implementation to indicate this class supports getChip() calls.
    * Can be done with dynamic cast and pointer test, but not sure how that is supported in SWIG
    * (OLK 11/2015). */
   virtual bool isChipProcessor() const { return false; }

   /**
    * Redirects any console output to the supplied stream for logging or JNI application.
    */
   void setOutputStream(std::ostream* os) { m_consoleStream = os; }

   /**
    * Returns true when the initialization detects a "--help" option, so caller can avoid
    * subsequent execute() call. If execute() is called, it will return true after trivial execute.
    */
   bool helpRequested() const { return m_helpRequested; }

protected:
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

   ossimKeywordlist m_kwl;
   std::ostream* m_consoleStream;
   bool m_helpRequested;
   std::string m_response;

private:
   /**
    * Used for reading text files of template and JSON API from disk ONLY.
    */
   bool readTextFile(const ossimFilename& filename, std::string& contents) const;

};

#endif

//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
//*************************************************************************
// $Id$

#ifndef ossimUtility_HEADER
#define ossimUtility_HEADER

#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimListenerManager.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimKeywordlist.h>

/*!
 *  Base class for all OSSIM utility applications.
 */
class OSSIMDLLEXPORT ossimUtility : public ossimObject,
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
   virtual bool initialize(ossimArgumentParser& ap);

   /**
    * Reads processing params from KWL and prepares for execute. Returns TRUE if successful.
    * @note Throws ossimException on error.
    */
   virtual bool initialize(const ossimKeywordlist& kwl) { return true; }

   /**
    * Writes product to output file. Returns true if successful.
    * @note Throws ossimException on error.
    */
   virtual bool execute() = 0;

   /**
    * Disconnects and clears the DEM and image layers. Leaves OSSIM initialized.
    */
   virtual void clear() = 0;

   /**
    * Kills current (asynchronous) process. Defaults to do nothing.
    */
   virtual void abort() {}

   /**
    * Assigns a template keywordlist to string for initializing derived classes.
    */
   virtual void getKwlTemplate(ossimString& kwl);

   /**
    * Outputs a JSON representation of the Utility's API.
    */
   void getUtilityApi(ossimString& out) const;

   virtual ossimObject* getObject() { return this; }
   virtual const ossimObject* getObject() const  { return this; }
   virtual ossimListenerManager* getManager()  { return this; };
   virtual ossimString getClassName() const { return "ossimUtility"; }

private:
   bool readFile(const ossimFilename& filename, ossimString& contents) const;
};

#endif

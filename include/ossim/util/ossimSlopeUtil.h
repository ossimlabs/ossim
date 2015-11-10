//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimSlopeUtil_HEADER
#define ossimSlopeUtil_HEADER

#include <ossim/util/ossimUtility.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/imaging/ossimImageSource.h>

/*!
 *  Class for computing the slope on each elevation post and generatinga corresponding slope image.
 *  The output scalar type is a normalized float unless unsigned 8-bit is selected via the options.
 */
class OSSIMDLLEXPORT ossimSlopeUtil : public ossimUtility
{
public:
   ossimSlopeUtil();
   ~ossimSlopeUtil();

   /**
    * Initializes the aurgument parser with expected parameters and options. It does not output
    * anything. To see the usage, the caller will need to do something like:
    *
    *   ap.getApplicationUsage()->write(<ostream>);
    */
   virtual void setUsage(ossimArgumentParser& ap);

   /**
    * Initializes from command line arguments.
    * @note Throws ossimException on error.
    */
   virtual bool initialize(ossimArgumentParser& ap);

   /**
    * Reads processing params from KWL and prepares for execute. Returns TRUE if successful.
    * @note Throws ossimException on error.
    */
   virtual bool initialize(const ossimKeywordlist& kwl);

   /**
    * Writes product to output file. Returns true if successful.
    * @note Throws ossimException on error.
    */
   virtual bool execute();

   /**
    * Disconnects and clears the DEM and image layers. Leaves OSSIM initialized.
    */
   virtual void clear();

   /**
    * Kills current (asynchronous) process. Defaults to do nothing.
    */
   virtual void abort() {}

   virtual ossimString getClassName() const { return "ossimSlopeUtil"; }

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

protected:
   bool initializeChain();
   bool loadDemFile();
   bool loadElevDb();

   ossimFilename m_demFile;
   ossimFilename m_slopeFile;
   ossimFilename m_lutFile;
   ossimGpt m_centerGpt;
   double m_aoiRadius; // meters
   bool m_remapToByte;
   ossimRefPtr<ossimImageSource> m_procChain;
};

#endif

//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimSubImageTool_HEADER
#define ossimSubImageTool_HEADER

#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/util/ossimChipProcTool.h>

/*!
 *  Tool class for extracting a subimage rectangle from a full image. The original sensor model
 *  is replaced with the subimage RPC representation. This is necessary as OSSIM does not support
 *  sub-image offset in the sensor models yet.
 */
class OSSIMDLLEXPORT ossimSubImageTool : public ossimChipProcTool
{
public:
   ossimSubImageTool();
   ~ossimSubImageTool();

   /**
    * Initializes the aurgument parser with expected parameters and options. It does not output
    * anything. To see the usage, the caller will need to do something like:
    *
    *   ap.getApplicationUsage()->write(<ostream>);
    */
   virtual void setUsage(ossimArgumentParser& ap);

   /**
    * Initializes from command line arguments.
    * @return FALSE if --help option requested or no params provided, so that derived classes can
    * @note Throws ossimException on error.
    */
   virtual bool initialize(ossimArgumentParser& ap);

   /**
    * Reads processing params from KWL and prepares for execute. Returns TRUE if successful.
    * @note Throws ossimException on error.
    */
   virtual void initialize(const ossimKeywordlist& kwl);

   /**
    * Writes product to output file if applicable. The product may also beAlways returns true since using exception on error.
    * @note Throws ossimException on error.
    */
   virtual bool execute();

   virtual ossimString getClassName() const { return "ossimSubImageTool"; }

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

protected:
   virtual void initProcessingChain() {}
};

#endif

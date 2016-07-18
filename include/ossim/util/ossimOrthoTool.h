//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimOrthoUtil_HEADER
#define ossimOrthoUtil_HEADER

#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/util/ossimChipProcTool.h>

/*!
 *  Class for computing the slope on each elevation post and generatinga corresponding slope image.
 *  The output scalar type is a normalized float unless unsigned 8-bit is selected via the options.
 */
class OSSIMDLLEXPORT ossimOrthoTool : public ossimChipProcTool
{
public:
   ossimOrthoTool();
   ~ossimOrthoTool();

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

   virtual ossimString getClassName() const { return "ossimOrthoUtil"; }

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

protected:
   virtual void initProcessingChain();
};

#endif

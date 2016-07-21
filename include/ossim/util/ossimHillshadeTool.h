//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimHillshadeUtil_HEADER
#define ossimHillshadeUtil_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/util/ossimChipProcTool.h>

// Forward class declarations:
class ossimArgumentParser;
class ossimDpt;
class ossimFilename;
class ossimGeoPolygon;
class ossimGpt;
class ossimImageData;
class ossimImageFileWriter;
class ossimImageGeometry;
class ossimImageViewAffineTransform;
class ossimIrect;
class ossimKeywordlist;

class OSSIM_DLL ossimHillshadeTool : public ossimChipProcTool
{
public:
   /** default constructor */
   ossimHillshadeTool();

   /** virtual destructor */
   virtual ~ossimHillshadeTool();

   virtual void setUsage(ossimArgumentParser& ap);

   virtual bool initialize(ossimArgumentParser& ap);

   virtual ossimString getClassName() const { return "ossimHillshadeUtil"; }

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

protected:

   virtual void initProcessingChain();

   /** @brief Hidden from use copy constructor. */
   ossimHillshadeTool( const ossimHillshadeTool& obj );

   /** @brief Hidden from use assignment operator. */
   const ossimHillshadeTool& operator=( const ossimHillshadeTool& rhs );
};

#endif /* #ifndef ossimHillshadeUtil_HEADER */

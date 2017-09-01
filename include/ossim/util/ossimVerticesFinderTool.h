//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimVerticesFinderUtil_HEADER
#define ossimVerticesFinderUtil_HEADER

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/util/ossimTool.h>

/*!
 *  Class for determining the active image corner vertices inside larger null-filled image rectangle.
 */
class OSSIMDLLEXPORT ossimVerticesFinderTool : public ossimTool
{
public:
   ossimVerticesFinderTool();
   ~ossimVerticesFinderTool();

   virtual void setUsage(ossimArgumentParser& ap);

   virtual bool initialize(ossimArgumentParser& ap);

   virtual void initialize(const ossimKeywordlist& kwl);

   virtual bool execute();

   virtual ossimString getClassName() const { return "ossimVerticesFinderUtil"; }

   virtual void getKwlTemplate(ossimKeywordlist& kwl);

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

protected:
   ossimFilename m_inputFile;
   ossimFilename m_outputFile;
   ossim_uint32 m_entryIndex;
};

#endif

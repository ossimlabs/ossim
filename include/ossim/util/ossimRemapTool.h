//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimRemapTool_HEADER
#define ossimRemapTool_HEADER

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/util/ossimChipProcTool.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimSingleImageChain.h>

/*!
 *  Class for Performing histo ans scalar-based remapping.
 */
class OSSIMDLLEXPORT ossimRemapTool : public ossimChipProcTool
{
public:
   ossimRemapTool();
   ~ossimRemapTool();

   virtual bool initialize(ossimArgumentParser& ap);

   virtual ossimString getClassName() const { return "ossimRemapTool"; }

   virtual bool execute();

   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

protected:
   virtual void setUsage(ossimArgumentParser& ap);
   virtual void initProcessingChain();

   bool m_doHistoStretch;
   ossimFilename m_inputFilename;
   uint32_t m_entry;
};

#endif

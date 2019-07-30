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
 *  ossimTool class for performing histo ans scalar-based remapping.
 */
class OSSIMDLLEXPORT ossimRemapTool : public ossimChipProcTool
{
public:
   ossimRemapTool();

   /** This constructor packages up all the remap functionality into one "call". There is no need to
    * call execute() if this constructor is invoked. The remapped file will be accompanied by an
    * OSSIM geom file that echoes the original input image geometry.
    * @param inputFile  The image file to be remapped.
    * @param entry_index  For multi-entry input files, specifies the entry to be remapped
    * @param skipHistoStretch  Set to true to bypass histogram stretch
    * @param outputFile  If left blank, the output will be "<inputFile>-remap.<ext>"
    * @throws ossimException on error. Use ossimException::what() to get error description.
    */
   ossimRemapTool(const ossimFilename& inputFile,
                  int entryIndex=0,
                  bool doHistoStretch=true,
                  ossimFilename outputFile="");

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
   double m_gsd;
};

#endif

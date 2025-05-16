//---
//
// License: MIT
//
// Author: David Burken
//
// Description: Info object for Quickbird metadata with a collection of
// metadata sidecar files, i.e. dot TIL, IMD, ATT, GEO, EPH, and RPB.
// 
//---
// $Id$
#ifndef ossimQuickbirdMetaDataInfo_H
#define ossimQuickbirdMetaDataInfo_H 1

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimInfoBase.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>

/** @brief ossimQuickbirdMetaDataInfo info class. */
class OSSIM_DLL ossimQuickbirdMetaDataInfo : public ossimInfoBase
{
public:
   
   /** default constructor */
   ossimQuickbirdMetaDataInfo();

   /**
    * @brief open method.
    *
    * @param file File name to open.
    *
    * @return true on success false on error.
    */
   virtual bool open(const ossimFilename& file);
   
   /**
    * Print method.
    *
    * @param out Stream to print to.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;

protected:
   
   ossimFilename    m_file;
   ossimKeywordlist m_info;
};

#endif /* End of "#ifndef ossimQuickbirdMetaDataInfo_H" */

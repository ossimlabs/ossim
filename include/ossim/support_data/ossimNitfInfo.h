//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: NITF Info object.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimNitfInfo_HEADER
#define ossimNitfInfo_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/support_data/ossimInfoBase.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <iosfwd>

/**
 * @brief NITF info class.
 *
 * Encapsulates the dumpnitf functionality.
 */
class OSSIM_DLL ossimNitfInfo : public ossimInfoBase
{
public:
   
   /** default constructor */
   ossimNitfInfo();

   /** virtual destructor */
   virtual ~ossimNitfInfo();

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

   /**
    * @brief Method to dump info to a keyword list.
    * @param kwl The keyword list to initialize.
    * @param entryIndex Entry to print.  Not supported by all info objects
    * yet.
    * @return true on success, false on error.
    */
   virtual bool getKeywordlist( ossimKeywordlist& kwl,
                                ossim_uint32 entryIndex )const;
   
private:
   ossimRefPtr<ossimNitfFile> m_nitfFile;
};

#endif /* End of "#ifndef ossimNitfInfo_HEADER" */

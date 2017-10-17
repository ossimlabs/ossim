//----------------------------------------------------------------------------
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: DTED Info object.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimDtedInfo_HEADER
#define ossimDtedInfo_HEADER

#include <iosfwd>
#include <vector>
#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimInfoBase.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/support_data/ossimDtedVol.h>
#include <ossim/support_data/ossimDtedHdr.h>
#include <ossim/support_data/ossimDtedUhl.h>
#include <ossim/support_data/ossimDtedDsi.h>
#include <ossim/support_data/ossimDtedAcc.h>

class ossimNitfFile;
class ossimProperty;

/**
 * @brief DTED Info class.
 *
 * Encapsulates the dtedInfo functionality.
 */
class OSSIM_DLL ossimDtedInfo : public ossimInfoBase
{
public:
   
   /** default constructor */
   ossimDtedInfo();

   /** virtual destructor */
   virtual ~ossimDtedInfo();

   /**
    * @brief open method.
    *
    * @param str stream to test.
    * @param connectionString original string for the stream.
    *
    * @return true on success false on error.
    */
   virtual bool open(std::shared_ptr<ossim::istream>& str,
                     const std::string& connectionString);
   
   /**
    * Print method.
    *
    * @param out Stream to print to.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;

   /**
    * @brief Gets a property for name.
    * @param name Property name to get.
    * @return ossimRefPtr<ossimProperty> Note that this can be empty if
    * property for name was not found.
    */
   ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;
   
   /**
    * @brief Adds this class's properties to list.
    * @param propertyNames list to append to.
    */
   void getPropertyNames(std::vector<ossimString>& propertyNames)const;

private:
  mutable std::shared_ptr<ossim::istream> m_dtedFileStr;
  std::string  m_connectionString;
  ossimDtedVol m_vol;
  ossimDtedHdr m_hdr;
  ossimDtedUhl m_uhl;
  ossimDtedDsi m_dsi;
  ossimDtedAcc m_acc;

   //ossimFilename theFile;
};

#endif /* End of "#ifndef ossimDtedInfo_HEADER" */

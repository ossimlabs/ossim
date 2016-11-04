//---
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//---
// $Id$
#ifndef ossimNitfFile_HEADER
#define ossimNitfFile_HEADER 1

#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/support_data/ossimNitfFileHeader.h>

#include <iosfwd>
#include <vector>

class ossimNitfImageHeader;
class ossimNitfSymbolHeader;
class ossimNitfLabelHeader;
class ossimNitfTextHeader;
class ossimNitfDataExtensionSegment;
class ossimFilename;
class ossimNitfRegisteredTag;
class ossimNitfTagInformation;

class OSSIMDLLEXPORT ossimNitfFile : public ossimReferenced
{
public:
   friend OSSIMDLLEXPORT std::ostream& operator <<(std::ostream& out,
                                                   const ossimNitfFile &data);

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out Stream to output to.
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @param printOverviews If true overview, if present(e.g. rpf's) will be
    * printed.
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       const std::string& prefix=std::string(),
                       bool printOverviews=false) const;

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out Stream to output to.
    * @param entryIndex Entry to print.  Not supported by all info objects
    * @param prefix This will be prepended to key.
    * e.g. Where prefix = "nitf." and key is "file_name" key becomes:
    * "nitf.file_name:"
    * @param printOverviews If true overview, if present(e.g. rpf's) will be 
    * printed.
    * @return output stream.
    */
   std::ostream& print(std::ostream& out,
                       ossim_uint32 entryIndex, 
                       const std::string& prefix=std::string(),
                       bool printOverviews=false) const;
   
   ossimNitfFile();
   virtual ~ossimNitfFile();

   /*!
    *  Opens the nitf file and attempts to parse.
    *  Returns true on success, false on error.
    */
   bool parseFile(const ossimFilename &file);

   /*!
    * Will return the header.
    */
   const ossimNitfFileHeader* getHeader()const;
   ossimNitfFileHeader* getHeader();
   ossimIrect getImageRect()const;
   
   ossimNitfImageHeader*  getNewImageHeader(ossim_uint32 imageNumber)const;
   ossimNitfSymbolHeader* getNewSymbolHeader(ossim_uint32 symbolNumber)const;
   ossimNitfLabelHeader*  getNewLabelHeader(ossim_uint32 labelNumber)const;
   ossimNitfTextHeader*   getNewTextHeader(ossim_uint32 textNumber)const;
   ossimNitfDataExtensionSegment* getNewDataExtensionSegment(ossim_uint32 dataExtNumber)const;
   
   ossimString getVersion()const;

   /** @return The filename parsed by this object. */
   ossimFilename getFilename() const;
   
   virtual bool saveState(ossimKeywordlist& kwl, const ossimString& prefix)const;

protected:
   ossimNitfImageHeader* allocateImageHeader()const;
   
   ossimFilename                    theFilename;
   ossimRefPtr<ossimNitfFileHeader> theNitfFileHeader;
};

#endif

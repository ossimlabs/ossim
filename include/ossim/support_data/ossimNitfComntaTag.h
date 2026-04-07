//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: COMNTA tag class declaration.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.1 for more info.
//
//----------------------------------------------------------------------------

#ifndef ossimNitfComntaTag_HEADER
#define ossimNitfComntaTag_HEADER 1

#include <ossim/support_data/ossimNitfGenericTag.h>

class OSSIM_DLL ossimNitfComntaTag : public ossimNitfRegisteredTag
{
public:
   const ossimString CETAG_KW = "COMNTA";
   ossimNitfComntaTag();

   /**
    * Parse method.
    *
    * @param in Stream to parse.
    */
   virtual void parseStream(std::istream& in);

   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix);

   /**
    * Write method.
    *
    * @param out Stream to write to.
    */
   virtual void writeStream(std::ostream& out);

   /**
    * Clears all string fields within the record to some default nothingness.
    */
   virtual void clearFields();

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;

   ossimString getComment();
   void setComment(const ossimString& fieldValue);

   ossim_uint32 computeTagLength() const;

   
private:

   std::string comment;

};

#endif /* #ifndef ossimNitfComntaTag_HEADER */

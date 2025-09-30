//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSATTB des class declaration.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.6 for more info.
//
//----------------------------------------------------------------------------

#ifndef ossimNitfCsattbDes_HEADER
#define ossimNitfCsattbDes_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredDes.h>
#include <ossim/support_data/ossimNitfSegmentSecurityMetadataV1.h>

class OSSIM_DLL ossimNitfCsattbDes : public ossimNitfRegisteredDes
{
public:
   enum
   {
      DESVER_SIZE = 2
   };
   
   ossimNitfCsattbDes();
   
   ossimNitfCsattbDes(ossim_uint32 tagLength);
   
   virtual ossimString getClassName() const;

   static const std::string DESID;

   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   /**
    * @brief Sets segment security metadata.
    * @param obj
    */
   void setSegmentSecurityMetadata(
      const ossimNitfSegmentSecurityMetadataV1& obj);

   /**
    * @brief Gets segment security metadata.
    * @return const reference to segment security metadata.
    */
   const ossimNitfSegmentSecurityMetadataV1& getSegmentSecurityMetadata() const;

   /**
    * @brief Gets segment security metadata.
    * @return Reference to segment security metadata.
    */
   ossimNitfSegmentSecurityMetadataV1& getSegmentSecurityMetadata();

   /**
    * Clears all string fields within the record to some default nothingness.
    */
   void clearFields();

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const;
private:

   char m_desver[DESVER_SIZE+1];
   
   ossimNitfSegmentSecurityMetadataV1 m_segSecurityMetadata;

};


#endif

#if 0 /* ossimNitfGenericDes version */

#include <ossim/support_data/ossimNitfGenericDes.h>

class OSSIM_DLL ossimNitfCsattbDes : public ossimNitfGenericDes
{
public:
   
   ossimNitfCsattbDes();
   
   ossimNitfCsattbDes(ossim_uint32 tagLength);
   
   virtual ossimString getClassName() const;

   static const std::string CETAG_KW;

private:

   void initializeFieldDefinitions();

};
#endif

//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License: MIT
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id$

#ifndef ossimNitfDesInformation_HEADER
#define ossimNitfDesInformation_HEADER 1

#include <ossim/base/ossimObject.h>
#include <ossim/support_data/ossimNitfRegisteredDes.h>
#include <ossim/support_data/ossimNitfSegmentSecurityMetadataV2_1.h>

class ossimString;

class OSSIMDLLEXPORT ossimNitfDesInformation : public ossimObject
{

public:
   enum
   {
      DE_SIZE = 2,
      DESID_SIZE = 25,
      DESVER_SIZE = 2,
      DESOFLW_SIZE = 6,
      DESITEM_SIZE = 3,
      DESSHL_SIZE = 4
   };

   ossimNitfDesInformation(ossimRefPtr<ossimNitfRegisteredDes> desData = 0);
   virtual ~ossimNitfDesInformation();
   
   //virtual void parseStream(std::istream& in);
   virtual void parseStream(std::istream &in, ossim_uint64 dataLength=0);
   virtual void writeStream(std::ostream& out);

   void setDesName(const ossimString& desName);

   /** deprecated, use setDesDataLength */
   void setDesLength(ossim_uint32 desLength);

   /**
    * Should return the value of theDesLength which is the length of
    * the data in bytes.
    */
   // ossim_uint32 getDesLength()const;
   ossim_uint64 getDesOffset()const;
   ossim_uint64 getDesDataOffset()const;
   
   ossimString getDesId()const;
   bool isTreOverflow() const;
   ossimString getDesVer()const;

   virtual std::ostream& print(std::ostream& out, const std::string& prefix)const;
   void clearFields();
   
   ossimRefPtr<ossimNitfRegisteredDes> getDesData();
   const ossimRefPtr<ossimNitfRegisteredDes> getDesData()const;
   void setDesData(ossimRefPtr<ossimNitfRegisteredDes> desData);

   bool operator<(const ossimNitfDesInformation& rhs) const;

   /**
    * @brief Sets segment security metadata.
    * @param obj
    */
   void setSegmentSecurityMetadata(
      const ossimNitfSegmentSecurityMetadataV2_1& obj);

   /**
    * @brief Gets segment security metadata.
    * @return const reference to segment security metadata.
    */
   const ossimNitfSegmentSecurityMetadataV2_1& getSegmentSecurityMetadata() const;

   /**
    * @brief Gets segment security metadata.
    * @return Reference to segment security metadata.
    */
   ossimNitfSegmentSecurityMetadataV2_1& getSegmentSecurityMetadata();

   std::string get_desshl() const;
   void set_desshl(ossim_uint32 length);
   ossim_uint32 getDesSubHeaderLength() const;

   /**
    * @brief This is the des record size down to the desshl field or
    * header + user defined sub header.
    * @return 200+sub header length
    */
   ossim_uint32 getDesHeaderLength() const;

   /**
    * @brief Set the m_desDataSize work variable.
    * @param length
    */
   void setDesDataLength(ossim_uint32 length);

   /**
    * @brief This is the length of the des data portion.
    * @return length
    */
   ossim_uint32 getDesDataLength() const;

   /**
    * @brief This is getDesDataLength() + getDesHeaderLength()
    * @return length
    */
   ossim_uint32 getDesTotalLength() const;

private:



   char           m_de[DE_SIZE+1];
   char           m_desid[DESID_SIZE+1];
   char 	  m_desver[DESVER_SIZE+1];

   ossimNitfSegmentSecurityMetadataV2_1 m_segSecurityMetadata;
   
   char           m_desoflw[DESOFLW_SIZE+1];
   char           m_desitem[DESITEM_SIZE+1];
   char           m_desshl[DESSHL_SIZE+1];

   /**
    * This will hold the offset to the start of the above information
    * This is just a work variable.
    */
   ossim_uint64 m_desOffset;

   /**
    * This will hold the start to the data.  This is just the
    * position in the file just past the des length field.
    * This is just a work variable
    */
   ossim_uint64 m_desDataOffset;
   ossim_uint64 m_desDataSize;

   /**
    * Used to hold the des data.
    */
   ossimRefPtr<ossimNitfRegisteredDes> m_desData;
};

#endif

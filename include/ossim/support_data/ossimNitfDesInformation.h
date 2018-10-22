//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfDesInformation.h 22418 2013-09-26 15:01:12Z gpotts $
#ifndef ossimNitfDesInformation_HEADER
#define ossimNitfDesInformation_HEADER

#include <ossim/base/ossimObject.h>
#include <ossim/support_data/ossimNitfRegisteredDes.h>

class ossimString;

class OSSIMDLLEXPORT ossimNitfDesInformation : public ossimObject
{

public:
   enum{
      DE_SIZE = 2,
      DESID_SIZE = 25,
      DESVER_SIZE = 2,
      DECLAS_SIZE = 1,
      DESCLSY_SIZE = 2,
      DESCODE_SIZE = 11,
      DESCTLH_SIZE = 2,
      DESREL_SIZE = 20,
      DESDCTP_SIZE = 2,
      DESDCDT_SIZE = 8,
      DESDCXM_SIZE = 4,
      DESDG_SIZE = 1,
      DESDGDT_SIZE = 8,
      DESCLTX_SIZE = 43,
      DESCATP_SIZE = 1,
      DESCAUT_SIZE = 40,
      DESCRSN_SIZE = 1,
      DESSRDT_SIZE = 8,
      DESCTLN_SIZE = 15,
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
   void setDesLength(ossim_uint32 desLength);

   /**
    * Length of the 5 byte des length the 6 byte des name and
    * the data length.
    * So we have Data length + 11 bytes.
    */
   ossim_uint32 getTotalDesLength()const;

   /**
    * Should return the value of theDesLength which is the length of
    * the data in bytes.
    */
   ossim_uint32 getDesLength()const;
   ossim_uint64 getDesOffset()const;
   ossim_uint64 getDesDataOffset()const;
   
   ossimString   getDesId()const;
   virtual std::ostream& print(std::ostream& out, const std::string& prefix)const;
   void clearFields();
   
   ossimRefPtr<ossimNitfRegisteredDes> getDesData();
   const ossimRefPtr<ossimNitfRegisteredDes> getDesData()const;
   void setDesData(ossimRefPtr<ossimNitfRegisteredDes> desData);

   bool operator<(const ossimNitfDesInformation& rhs) const
   {
      return getTotalDesLength() < rhs.getTotalDesLength();
   }
   
private:

   char           m_de[DE_SIZE+1];
   char           m_desid[DESID_SIZE+1];
   char 	         m_desver[DESVER_SIZE+1];
   char           m_declas[DECLAS_SIZE+1];
   char           m_desclsy[DESCLSY_SIZE+1];
   char           m_descode[DESCODE_SIZE+1];
   char           m_desctlh[DESCTLH_SIZE+1];
   char           m_desrel[DESREL_SIZE+1];
   char           m_desdctp[DESDCTP_SIZE+1];
   char           m_desdcdt[DESDCDT_SIZE+1];
   char           m_desdcxm[DESDCXM_SIZE+1];
   char           m_desdg[DESDG_SIZE+1];
   char           m_desdgdt[DESDGDT_SIZE+1];
   char           m_descltx[DESCLTX_SIZE+1];
   char           m_descatp[DESCATP_SIZE+1];
   char           m_descaut[DESCAUT_SIZE+1];
   char           m_descrsn[DESCRSN_SIZE+1];
   char           m_dessrdt[DESSRDT_SIZE+1];
   char           m_desctln[DESCTLN_SIZE+1];
   char           m_desoflw[DESOFLW_SIZE+1];
   char           m_desitem[DESITEM_SIZE+1];
   char           m_desshl[DESSHL_SIZE+1];

   /**
    * This will hold the offset to the start of the above information
    * This is just a work variable.
    */
   ossim_uint64 m_desOffset;
   ossim_uint64 m_desLength;


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

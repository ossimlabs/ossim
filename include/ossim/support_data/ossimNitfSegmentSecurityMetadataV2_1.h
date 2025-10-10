//---
// License: MIT
//
// Author:  David Burken
//
// Description: Container class declaration for JBD Segment Security Metadata.
//
// See: ISO/IEC JOINT BIIF PROFILE (JBP)
// Table 5.10-1 and 5.10-2 of JBP-2024.1 version.
//---
// $Id$

#ifndef ossimNitfSegmentSecurityMetadataV2_1_H
#define ossimNitfSegmentSecurityMetadataV2_1_H 1

#include <ossim/base/ossimConstants.h>
#include <iosfwd>
#include <string>

/**
 * @class ossimNitfSegmentSecurityMetadataV2_1
 * Segment Security Metadata:
 */
class OSSIM_DLL ossimNitfSegmentSecurityMetadataV2_1
{
public:
   
   enum
   {
      SCLAS_SIZE = 1,
      SCLSY_SIZE = 2,
      SCODE_SIZE = 11,
      SCTLH_SIZE = 2,
      SREL_SIZE  = 20,
      SDCTP_SIZE = 2,
      SDCDT_SIZE = 8,
      SDCXM_SIZE = 4,
      SDG_SIZE   = 1,
      SDGDT_SIZE = 8,
      SCLTX_SIZE = 43,
      SCATP_SIZE = 1,
      SCAUT_SIZE = 40,
      SCRSN_SIZE = 1,
      SSRDT_SIZE = 8,
      SCTLN_SIZE = 15
      //           -----
      //            167
   };

   /** @brief Default constructor. */
   ossimNitfSegmentSecurityMetadataV2_1();

   /**
    * @brief Copy Constructor.
    * @param obj
    */
   ossimNitfSegmentSecurityMetadataV2_1(const ossimNitfSegmentSecurityMetadataV2_1& obj);

   /**
    * @brief assignment operator
    * @param rhs
    */
   const ossimNitfSegmentSecurityMetadataV2_1& operator=(
      const ossimNitfSegmentSecurityMetadataV2_1& rhs);

   /**
    * @brief gets length
    * @return length in bytes
    */
   ossim_uint32 getLength() const;

   /**
    * @brief Sets the file part type.
    *
    * This is the prefix tacked onto the front of key for print / loadState
    * outputs, where prefix is "DE" and key is "SCLAS" and value is "U" would
    * result in: "DESCLAS: U".
    * @param prefix Per spec this should one of:
    * F  = File Header
    * I  = Image Segment
    * S  = Graphic Segment
    * T  = Text Segment
    * DE = Data Extension Segment
    * RE = Reserved Extension Segment
    */
   void setFilePartType(const std::string& type);

   void set_sclas(const char* sclas);
   void set_sclsy(const char* sclsy);
   void set_scode(const char* scode);
   void set_sctlh(const char* sctlh);
   void set_srel(const char* srel);
   void set_sdctp(const char* sdctp);
   void set_sdcdt(const char* sdcdt);
   void set_sdcxm(const char* sdcxm);
   void set_sdg(const char* sdg);
   void set_sdgdt(const char* sdgdt);
   void set_scltx(const char* scltx);
   void set_scatp(const char* scatp);
   void set_scaut(const char* scaut);
   void set_scrsn(const char* scrsn);
   void set_ssrdt(const char* ssrdt);
   void set_sctln(const char* sctln);

   /**
    * @brief Parse method.
    * @param in Stream to parse.
    */
   void parseStream(std::istream& in);
   
   /**
    * @brief Write method.
    * @param out Stream to write to.
    */
   void writeStream(std::ostream& out);
   
   /**
    * @brief Clears all string fields within the record to some default
    * nothingness.
    */
   void clearFields();

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   std::ostream& print(std::ostream& out, const std::string& prefix) const;

   friend OSSIM_DLL std::ostream& operator<<(
      std::ostream& out, const ossimNitfSegmentSecurityMetadataV2_1& obj);

private:

   // F, I, S, T, DE or RE
   std::string m_filePartType;
   
   char m_sclas[SCLAS_SIZE+1];
   char m_sclsy[SCLSY_SIZE+1];
   char m_scode[SCODE_SIZE+1];
   char m_sctlh[SCTLH_SIZE+1];
   char m_srel[SREL_SIZE+1];
   char m_sdctp[SDCTP_SIZE+1];
   char m_sdcdt[SDCDT_SIZE+1];
   char m_sdcxm[SDCXM_SIZE+1];
   char m_sdg[SDG_SIZE+1];
   char m_sdgdt[SDGDT_SIZE+1];
   char m_scltx[SCLTX_SIZE+1];
   char m_scatp[SCATP_SIZE+1];
   char m_scaut[SCAUT_SIZE+1];
   char m_scrsn[SCRSN_SIZE+1];
   char m_ssrdt[SSRDT_SIZE+1];
   char m_sctln[SCTLN_SIZE+1];
};

#endif /* End of "#ifndef ossimNitfSegmentSecurityMetadataV2_1_H" */

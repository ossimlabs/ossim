//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: ENGRDA - Engineering Data tag class declaration.
//
// See document STDI-0002 (version 4), Appendix N for more info.
// 
//----------------------------------------------------------------------------
// $Id

#ifndef ossimNitfEngrdaTag_HEADER
#define ossimNitfEngrdaTag_HEADER 1

#include <string>
#include <vector>

#include <ossim/support_data/ossimNitfRegisteredTag.h>

class OSSIM_DLL ossimNitfEngrdaTag : public ossimNitfRegisteredTag
{
public:

   enum 
   {
      RESRC_SIZE   = 20,
      RECNT_SIZE   = 3,
      ENGLN_SIZE   = 2,
      ENGMTXC_SIZE = 4,
      ENGMTXR_SIZE = 4,
      ENGTYP_SIZE  = 1,
      ENGDTS_SIZE  = 1,
      ENGDATU_SIZE = 2,
      ENGDATC_SIZE = 8,
      TMP_BUF_SIZE = 128
   };
   
   /** @brief default constructor */
   ossimNitfEngrdaTag();

   /** @brief destructor */
   virtual ~ossimNitfEngrdaTag();

   /** @brief Method to parse data from stream. */
   virtual void parseStream(std::istream& in);

   /** @brief Method to write data to stream. */
   virtual void writeStream(std::ostream& out);

   /** @brief Method to clear all fields including null terminating. */
   virtual void clearFields();

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   
protected:

   /** @brief Container class for an Engineering Data Element Record. */
   class ossimEngDataElement
   {
   public:
      
      /**
       * FIELD: ENGLN
       *
       * TYPE: BCS-N
       * 
       * 2 byte field
       *
       * Engineering Data Label Length 01 - 99
       */
      char m_engLn[ENGLN_SIZE+1];
      
      /**
       * FIELD: ENGLBL
       *
       * TYPE: BCS-A
       * 
       * Engineering Label - variable length
       */
      std::string m_engLbl;

      /**
       * FIELD: ENGMTXC
       *
       * TYPE: BCS-N
       * 
       * 4 byte field
       *
       * Engineering Matrix Data Column Count 0001 - 9999
       */
      char m_engMtxC[ENGMTXC_SIZE+1];

      /**
       * FIELD: ENGMTXR
       *
       * TYPE: BCS-N
       * 
       * 4 byte field
       *
       * Engineering Matrix Data Row Count 0001 - 9999
       */
      char m_engMtxR[ENGMTXR_SIZE+1];

      /**
       * FIELD: ENGTYP
       *
       * TYPE: BCS
       * 
       * 1 byte field
       *
       * Value Type of Engineering Data Element
       */
      char m_engTyp;

      /**
       * FIELD: ENGDTS
       *
       * TYPE: BCS-N
       * 
       * 1 byte field
       *
       * Engineering Data Element Size
       */
      char m_engDts;

      /**
       * FIELD: ENGDATU
       *
       * TYPE: BCS-A
       * 
       * 2 byte field
       *
       * Engineering Data Units
       */
      char m_engDatU[ENGDATU_SIZE+1];

      /**
       * FIELD: ENGDATC
       *
       * TYPE: BCS-N
       * 
       * 8 byte field
       *
       * Engineering Data Count
       */
      char m_engDatC[ENGDATC_SIZE+1];

      /** ENGDATA - data */
      std::vector<ossim_uint8> m_engDat;
   };

   /**
    * @brief Method to print out a ossimEngDataElement object.
    * @param out Stream to print to.
    * @param element The element to print.
    * @param elIndex The index of the element array index.
    * @param prefix  The prefix to add to key.
    */
   std::ostream& printData(std::ostream& out,
                           const ossimEngDataElement& element,
                           ossim_uint32 elIndex,
                           const std::string& prefix) const;

   /**
    * @brief Method to convert value to a string, padding with zero's,
    * left justified.
    * @param v The value to convert.
    * @param w The width of the string field.
    * @param s The string to stuff.
    */
   template <class T> void getValueAsString(T v,
                                            ossim_uint16 w,
                                            std::string& s) const;

   /** Type R = Required Type <R> = BCS spaces allowed for entire field */
   
   /**
    * FIELD: RESRC
    *
    * TYPE: BCS-A
    * 
    * 20 byte field
    *
    * Unique Source System Name.
    */
   char m_reSrc[RESRC_SIZE+1];
   
   /**
    * FIELD: RECNT
    *
    * TYPE: BCS-N
    * 
    * 3 byte field
    *
    * Record Entry Count 001 - 999
    */
   char m_reCnt[RECNT_SIZE+1];

   std::vector<ossimEngDataElement> m_data;
   
TYPE_DATA   
};

#endif /* matches #ifndef ossimNitfEngrdaTag_HEADER */

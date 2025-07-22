//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: Generic tag class declaration.
//
//----------------------------------------------------------------------------

#ifndef ossimNitfGenericTag_HEADER
#define ossimNitfGenericTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <ossim/base/ossimString.h>
#include <map>
#include <vector>

/**
 * @class ossimNitfGenericTag
 */
class OSSIM_DLL ossimNitfGenericTag : public ossimNitfRegisteredTag
{
public:
   ossimNitfGenericTag(ossimString tag, ossim_uint32 tagLength=0);

   virtual void parseStream(std::istream &in);

   virtual void writeStream(std::ostream &out);

   virtual void clearFields();

   virtual std::ostream &print(std::ostream &out,
                               const std::string &prefix) const;

   ossimString get(ossimString fieldName);
   void setField(ossimString fieldName, ossimString fieldValue);
   
protected:
   
   /**
    * @brief Computes tag length from m_fields_map size.
    * @return Tag length in bytes.
    */
   ossim_uint32 computeTagLength() const;

   /**
    * @brief Prints out m_fields_map for debug.
    */
   std::ostream& printMap(std::ostream& out ) const;

   /**
    * @brief Prints out field definitions for debug.
    */
   std::ostream& printFieldDefs(std::ostream& out ) const;

   class definition
   {
   public:
      std::ostream& print(std::ostream& out) const;
      
      ossimString field;

      /** Size in bytes or if negative relative to specialFields enum. */
      ossim_int32 size;
      
      std::vector<ossim_int8> formatMethod;
   };

   friend std::ostream& operator<<(std::ostream& out,
                                   const ossimNitfGenericTag::definition& def);

   std::vector<definition> FIELD_DEFINITIONS;

   //Parses field value from reverse polish noatation for loop and if conditions
   int parseRPN(ossimString input, std::vector<std::vector<ossim_int32>> suffixIn) const;
   std::map<ossimString, ossimString> m_fields_map;
   enum specialFields
   {
      VARIABLE_LENGTH = -1,
      IF_STATEMENT_START = -2,
      IF_STATEMENT_END = -3,
      LOOP_START = -4,
      LOOP_END = -5
   };
};



#endif

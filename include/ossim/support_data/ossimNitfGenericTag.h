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
#include <map>
#include <vector>

/**
 * @class ossimNitfGenericTag
 */
class OSSIM_DLL ossimNitfGenericTag : public ossimNitfRegisteredTag
{
public:
   ossimNitfGenericTag(const std::string& tag, ossim_uint32 tagLength=0);

   virtual void parseStream(std::istream &in);

   virtual void writeStream(std::ostream &out);

   virtual void clearFields();

   virtual std::ostream &print(std::ostream &out,
                               const std::string &prefix) const;

   ossimString get(const ossimString& fieldName);
   void setField(const ossimString& fieldName, const ossimString& fieldValue);

   struct definition
   {
      definition(const ossimString& field, ossim_int32 size = 0,
                       ossim_int8 dataFormat = 0, ossim_int8 precision = 0,
                       const ossimString& defaultValue = "");
      std::ostream& print(std::ostream& out) const;
      ossimString field;
      ossim_int32 size;
      ossim_int8 dataFormat;
      ossim_int8 precision;
      ossimString defaultValue;
   };
   std::vector<definition> FIELD_DEFINITIONS;

   ossim_uint32 computeTagLength() const;

   std::ostream& printMap(std::ostream& out ) const;

   std::ostream& printFieldDefs(std::ostream& out ) const;

protected:
   ossimString formatField(int definition, const ossimString& fieldValue) const;
   void loopLogic(ossim_int32 &i, std::vector<std::vector<ossim_int32>> &suffix) const;

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
   enum dataFormats
   {
      ASCII = 0,
      U_INT = 1,
      INT = 2,
      U_DOUBLE = 3,
      DOUBLE = 4,
      SCIENTIFIC = 5

   };
};

#endif

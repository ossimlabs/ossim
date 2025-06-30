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

#include <ossim/support_data/ossimNitfRegisteredTag.h>

#include <vector>
#include <map>
#ifndef ossimNitfGenericTag_HEADER
#define ossimNitfGenericTag_HEADER 1

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
                               const std::string &prefix);

   ossimString get(ossimString fieldName);
   void setField(ossimString fieldName, ossimString fieldValue);

   struct definition
   {
      ossimString field;
      ossim_int32 size;
      char specs = ' ';
   };
   static const ossim_int32 NUM_DEFINITIONS;
   static definition FIELD_DEFINITIONS[];

protected:
   std::map<ossimString, ossimString> m_fields_map;
   void readDefinitions(int actionFunction, std::istream &in, std::ostream &out, ossimString args = "");
   ossimString defaultFormatting(ossimString fieldValue, int definitionIndex, ossimString fieldName);
   ossimString defaultFormatting(ossimString fieldValue, int definitionIndex, int length);
   enum specialFields
   {
      VARIABLE_LENGTH = -1,
      IF_STATEMENT_START = -2,
      IF_STATEMENT_END = -3,
      LOOP_START = -4,
      LOOP_END = -5
   };
   enum actionFunctions
   {
      PARSE_STREAM = 0,
      SET_FIELD = 1,
      WRITE_STREAM = 2,
      PRINT = 3
   };
};

#endif

//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: Generic des class declaration.
//
//----------------------------------------------------------------------------

#ifndef ossimNitfGenericDes_HEADER
#define ossimNitfGenericDes_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredDes.h>
#include <map>
#include <vector>

/**
 * @class ossimNitfGenericDes
 */
class OSSIM_DLL ossimNitfGenericDes : public ossimNitfRegisteredDes
{
public:
   ossimNitfGenericDes(const std::string& des, ossim_uint32 desLength=0);

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
                       ossim_int8 dataFormat = 0,
                       ossim_int8 precision = 0,
                       const ossimString& defaultValue = "");
      std::ostream& print(std::ostream& out) const;
      ossimString field;
      ossim_int32 size;
      ossim_int8 dataFormat;
      ossim_int8 precision;
      ossimString defaultValue;
   };
   std::vector<definition> FIELD_DEFINITIONS;

   ossim_uint32 getDesSubHeaderLength() const;
   ossim_uint32 getDesDataLength() const;

   std::ostream& printMap(std::ostream& out ) const;

   std::ostream& printFieldDefs(std::ostream& out ) const;

   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix);

   int solveEquation(const ossimString& equation,std::vector<std::vector<ossim_int32>> suffixIn) const;

protected:
   ossimString formatField(int definition, const ossimString& fieldValue) const;
   void loopLogic(ossim_int32 &i, std::vector<std::vector<ossim_int32>> &suffix) const;
   virtual void initializeFields();

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
      SCIENTIFIC = 5,
      // 4-byte big-endian IEEE 754-2008 binary32, written as raw bytes rather
      // than as text. STDI-0002 marks such fields "IEEE 754-2008" in the C-Set
      // column. Before this existed they fell to the ASCII default and were
      // space-padded, so "1" shipped as 0x31202020 -- which reads back as
      // 2.33e-09, not 1.0. No DES field table needs it today; it is here so
      // the two twins stay in step and a future one cannot repeat the bug.
      IEEE_FLOAT = 6
   };
};

#endif

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
// $Id

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
   void setField(ossimString name, ossimString value);

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
   ossimString defaultFormatting(ossimString value, int definition, ossimString name);
   ossimString defaultFormatting(ossimString value, int definition, int length);
};

#endif

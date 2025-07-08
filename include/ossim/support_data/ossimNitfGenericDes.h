//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: Generic DES class declaration.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfRegisteredDes.h>

#include <vector>
#include <map>

#ifndef OSSIMNITFGENERICDES_H
#define OSSIMNITFGENERICDES_H

/**
 * @class ossimNitfGenericDes
 */
class OSSIM_DLL ossimNitfGenericDes : public ossimNitfRegisteredDes
{
public:
   ossimNitfGenericDes(ossimString tag, ossim_uint32 tagLength=0);

   virtual void parseStream(std::istream &in);

   virtual void writeStream(std::ostream &out);

   virtual void clearFields();

   virtual std::ostream &print(std::ostream &out,
                               const std::string &prefix) const;

   ossimString get(ossimString fieldName);
   void setField(ossimString fieldName, ossimString fieldValue);

   struct definition
   {
      ossimString field;
      ossim_int32 size;
      std::vector<ossim_int8> formatMethod = {0};
   };
   std::vector<definition> FIELD_DEFINITIONS;

protected:
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

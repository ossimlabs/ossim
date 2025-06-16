//---
// File: ossimNitfCsexrbTag.h
//---

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <vector>
#include <map>
#ifndef ossimNitfCsexrbTag_HEADER
#define ossimNitfCsexrbTag_HEADER 1

/**
 * @class ossimNitfCsexrbTag
 */
class OSSIM_DLL ossimNitfCsexrbTag : public ossimNitfRegisteredTag
{
public:
   ossimNitfCsexrbTag();

   virtual void parseStream(std::istream& in);

   virtual void writeStream(std::ostream& out);

   virtual void clearFields();

   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const;

   ossimString get(ossimString fieldName);

protected:

   std::map<ossimString, ossimString> m_fields_map;
   std::vector<std::pair<ossimString, ossimString>> m_fields_vector;
   ossim_uint32 m_total_length;

};

#endif

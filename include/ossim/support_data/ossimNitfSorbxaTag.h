//
// Created by Ryan Feldbush on 9/22/25.
//

#ifndef OSSIMNITFSORBXA_H
#define OSSIMNITFSORBXA_H

#include <base/ossimXmlNode.h>
#include <ossim/support_data/ossimNitfRegisteredTag.h>

#include <base/ossimXmlDocument.h>

class OSSIM_DLL ossimNitfSorbxaTag : public ossimNitfRegisteredTag
{
public:
   ossimNitfSorbxaTag(ossim_uint32 tagLength=0);

   void parseStream(std::istream &in);

   void writeStream(std::ostream &out);

   void clearFields();

   std::ostream &print(std::ostream &out,
                               const std::string &prefix) const;

   ossimString get(const ossimString& fieldName, int i = 0);
   void setField(const ossimString& fieldName, const ossimString& fieldValue, int i = 0);

   ossim_uint32 computeTagLength() const;

   bool loadState(const ossimKeywordlist& kwl, const char* prefix);

   bool r_loadState(ossimRefPtr<ossimXmlNode> fieldParent, ossimRefPtr<ossimXmlNode> valueParent, ossimKeywordlist& kwl);
private:
   void initializeFields();

   ossimString r_get(ossimRefPtr<ossimXmlNode> valueParent, ossimString fieldName, int& i);
   void r_set(ossimRefPtr<ossimXmlNode> valueParent, const ossimString& fieldName, const ossimString& fieldValue, int &i);

   ossimXmlDocument m_doc;
   ossimXmlDocument m_fieldsDoc;
};
#endif //OSSIMNITFSORBXA_H

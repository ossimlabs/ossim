//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: Generic tag class declaration for NITF tags in an xml format,
//    uses template files fed to the constructor to initialise and check formatting.
//
//----------------------------------------------------------------------------

#ifndef OSSIMNITFXML_H
#define OSSIMNITFXML_H

#include <ossim/base/ossimXmlNode.h>
#include <ossim/support_data/ossimNitfRegisteredTag.h>

#include <ossim/base/ossimXmlDocument.h>

class OSSIM_DLL ossimNitfXmlTag : public ossimNitfRegisteredTag
{
public:
   ossimNitfXmlTag(ossimString tagName);

   void parseStream(std::istream &in);

   void writeStream(std::ostream &out);

   void clearFields();

   std::ostream &print(std::ostream &out,
                       const std::string &prefix) const;

   ossimString get(const ossimString& fieldName, int i = 0);
   void setField(const ossimString& fieldName, const ossimString& fieldValue, int i = 0);

   ossim_uint32 computeTagLength() const;

   bool loadState(const ossimKeywordlist& kwl, const char* prefix);


private:
   void initializeFields();

   //Recursive functions that are called by the functions sharing their name
   void r_initializeFields(ossimRefPtr<ossimXmlNode> fieldParent, ossimRefPtr<ossimXmlNode> valueParent);
   bool r_loadState(ossimRefPtr<ossimXmlNode> fieldParent, ossimRefPtr<ossimXmlNode> valueParent, ossimKeywordlist& kwl);
   ossimString r_get(ossimRefPtr<ossimXmlNode> valueParent, ossimString fieldName, int& i);
   void r_set(ossimRefPtr<ossimXmlNode> valueParent, const ossimString& fieldName, const ossimString& fieldValue, int &i);
   std::ostream &r_print(ossimRefPtr<ossimXmlNode> valueParent, std::ostream &out, const std::string &prefix) const;


   ossimXmlDocument m_doc;
   ossimXmlDocument m_fieldsDoc;
};
#endif //OSSIMNITFXML_H

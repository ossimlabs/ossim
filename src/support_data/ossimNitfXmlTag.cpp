//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: Generic tag function definitions for NITF tags in an xml format,
//    uses template files fed to the constructor to initialise and check formatting.
//
//----------------------------------------------------------------------------

#include <support_data/ossimNitfXmlTag.h>
#include <sstream>

ossimNitfXmlTag::ossimNitfXmlTag(ossimString formatPath, ossimString tagName)
   : ossimNitfRegisteredTag(tagName, 0)
{
   m_fieldsDoc.openFile(formatPath);
   initializeFields();
   this->m_tagLength = computeTagLength();
}

//Recursive component of the initializeFields method
void ossimNitfXmlTag::r_initializeFields(ossimRefPtr<ossimXmlNode> fieldParent, ossimRefPtr<ossimXmlNode> valueParent)
{
   std::vector<ossimRefPtr<ossimXmlNode>> children = fieldParent->getChildNodes();
   for (auto& fieldChild: children)
   {
      if (fieldChild->findAttribute("mult") && fieldChild->getAttributeValue("mult") == "1")
      {
         auto valueChild = valueParent->addChildNode(fieldChild->getTag());
         valueChild->setText(fieldChild->getText());
         r_initializeFields(fieldChild, valueChild);
      }
   }
}

void ossimNitfXmlTag::initializeFields()
{
   ossimRefPtr<ossimXmlNode> node = new ossimXmlNode();
   node->setTag(m_fieldsDoc.getRoot()->getTag());
   m_doc.initRoot(node);
   r_initializeFields(m_fieldsDoc.getRoot(), m_doc.getRoot());
}

void ossimNitfXmlTag::parseStream(std::istream &in)
{
   std::ostringstream buffer;
   std::string line;
   std::string delimiter = "</" + m_fieldsDoc.getRoot()->getTag() + ">";
   while (std::getline(in, line)) {
      if (line.find(delimiter) != std::string::npos) {
         buffer << line.substr(0, line.find(delimiter));
         break;
      }
      buffer << line << '\n';
   }
   m_doc.read(in);
}

void ossimNitfXmlTag::writeStream(std::ostream &out)
{
   out << m_doc;
}

void ossimNitfXmlTag::clearFields()
{
   m_doc.getRoot()->clear();
   initializeFields();
}

std::ostream &ossimNitfXmlTag::print(std::ostream &out,
                            const std::string &prefix) const
{
   out << m_doc;
   return out;
}

//Recursive component of the get method
ossimString ossimNitfXmlTag::r_get(ossimRefPtr<ossimXmlNode> valueParent, ossimString fieldName, int& i)
{
   std::vector<ossimRefPtr<ossimXmlNode>> children = valueParent->getChildNodes();
   for (auto& child: children)
   {
      if (child->getTag() == fieldName)
      {
         if (i == 0)
            return child->getText();
         else
            i--;
      }
      return r_get(child, fieldName, i);
   }
   return "";
}

ossimString ossimNitfXmlTag::get(const ossimString& fieldName, int i)
{
   return r_get(m_doc.getRoot(), fieldName, i);
}

//Recursive component of the set method
void ossimNitfXmlTag::r_set(ossimRefPtr<ossimXmlNode> valueParent, const ossimString& fieldName, const ossimString& fieldValue, int &i)
{
   std::vector<ossimRefPtr<ossimXmlNode>> children = valueParent->getChildNodes();
   for (auto& child: children)
   {
      if (child->getTag() == fieldName)
      {
         if (i == 0)
            child->setText(fieldValue);
         i--;
      }
      r_set(child, fieldName, fieldValue, i);
   }
}

void ossimNitfXmlTag::setField(const ossimString& fieldName, const ossimString& fieldValue, int i)
{
   r_set(m_doc.getRoot(), fieldName, fieldValue, i);
}

ossim_uint32 ossimNitfXmlTag::computeTagLength() const
{
   std::stringstream result;
   result << m_doc;
   return result.str().size();
}

//Recursive component of the loadState method
bool ossimNitfXmlTag::r_loadState(ossimRefPtr<ossimXmlNode> fieldParent, ossimRefPtr<ossimXmlNode> valueParent, ossimKeywordlist& kwl)
{

   std::vector<ossimRefPtr<ossimXmlNode>> children = fieldParent->getChildNodes();
   std::vector<ossimRefPtr<ossimXmlNode>> valueChildren = valueParent->getChildNodes();
   bool needChildren = false;
   for (auto& fieldChild: children)
   {
      ossimRefPtr<ossimXmlNode> valueChild;
      for (auto searchChild : valueChildren)
      {
         if (searchChild->getTag() == fieldChild->getTag())
         {
            valueChild = searchChild;
            break;
         }
      }
      if (!valueChild)
         valueChild = valueParent->addChildNode(fieldChild->getTag());
      for (const auto& keyword: kwl.getMap())
      {
         if (!fieldChild->getAttributeValue("attributes").empty() &&
            fieldChild->getAttributeValue("attributes").find(keyword.first) != std::string::npos)
            valueChild->addAttribute(keyword.first, keyword.second);
         if (fieldChild->getTag().length() <= keyword.first.length() &&
            fieldChild->getTag() == keyword.first.substr(0, fieldChild->getTag().length()))
         {
            valueChild->setText(keyword.second);
            kwl.remove(keyword.first.c_str());
            break;
         }
      }
      bool needChild = !valueChild->getText().empty() | r_loadState(fieldChild, valueChild, kwl);
      if (!needChild && !fieldChild->findAttribute("mult"))
         valueParent->removeChild(valueChild);
      needChildren = needChildren | needChild;
   }
   return needChildren;
}

bool ossimNitfXmlTag::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimKeywordlist localKWL = kwl;
   return r_loadState(m_fieldsDoc.getRoot(), m_doc.getRoot(), localKWL);
}
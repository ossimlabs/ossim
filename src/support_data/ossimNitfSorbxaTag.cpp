//
// Created by Ryan Feldbush on 9/22/25.
//

#include <support_data/ossimNitfSorbxaTag.h>
#include <sstream>

ossimNitfSorbxaTag::ossimNitfSorbxaTag(ossim_uint32 tagLength)
   : ossimNitfRegisteredTag("SORBXA", tagLength)
{
   ossimString ossimInstallPrefix = getenv("OSSIM_HOME");
   m_fieldsDoc.openFile(ossimInstallPrefix + "/share/ossim/util/nitfSorbxa.xml");
   initializeFields();
}

void r_initializeFields(ossimRefPtr<ossimXmlNode> fieldParent, ossimRefPtr<ossimXmlNode> valueParent)
{
   std::vector<ossimRefPtr<ossimXmlNode>> children = fieldParent->getChildNodes();
   for (auto& fieldChild: children)
   {
      if (fieldChild->findAttribute("mult")->getValue()[0] == '1')
      {
         auto valueChild = valueParent->addChildNode(fieldChild->getTag());
         valueChild->setText(fieldChild->getText());
         r_initializeFields(fieldChild, valueChild);
      }
   }
}

void ossimNitfSorbxaTag::initializeFields()
{
   ossimRefPtr<ossimXmlNode> node = new ossimXmlNode();
   node->setTag("spaceObjectOrbitGeometry");
   m_doc.initRoot(node);
   r_initializeFields(m_fieldsDoc.getRoot(), m_doc.getRoot());
}

void ossimNitfSorbxaTag::parseStream(std::istream &in)
{
   m_doc.read(in);
}

void ossimNitfSorbxaTag::writeStream(std::ostream &out)
{
   out << m_doc;
}

void ossimNitfSorbxaTag::clearFields()
{
   m_doc.getRoot()->clear();
   initializeFields();
}

std::ostream &ossimNitfSorbxaTag::print(std::ostream &out,
                            const std::string &prefix) const
{
   out << m_doc;
   return out;
}

ossimString ossimNitfSorbxaTag::r_get(ossimRefPtr<ossimXmlNode> valueParent, ossimString fieldName, int& i)
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

ossimString ossimNitfSorbxaTag::get(const ossimString& fieldName, int i)
{

   return r_get(m_doc.getRoot(), fieldName, i);
}

void ossimNitfSorbxaTag::r_set(ossimRefPtr<ossimXmlNode> valueParent, const ossimString& fieldName, const ossimString& fieldValue, int &i)
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

void ossimNitfSorbxaTag::setField(const ossimString& fieldName, const ossimString& fieldValue, int i)
{
   r_set(m_doc.getRoot(), fieldName, fieldValue, i);
}

ossim_uint32 ossimNitfSorbxaTag::computeTagLength() const
{
   std::stringstream result;
   result << m_doc;
   return result.str().size();
}

bool ossimNitfSorbxaTag::r_loadState(ossimRefPtr<ossimXmlNode> fieldParent, ossimRefPtr<ossimXmlNode> valueParent, ossimKeywordlist& kwl)
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
         if (fieldChild->getTag().length() <= keyword.first.length() &&
            fieldChild->getTag() == keyword.first.substr(0, fieldChild->getTag().length()))
         {
            valueChild->setText(keyword.second);
            kwl.remove(keyword.first.c_str());
            break;
         }
      }
      bool needChild = !valueChild->getText().empty() | r_loadState(fieldChild, valueChild, kwl);
      if (!needChild && fieldChild->findAttribute("mult")->getValue()[0] == '0')
         valueParent->removeChild(valueChild);
      needChildren = needChildren | needChild;
   }
   return needChildren;
}

bool ossimNitfSorbxaTag::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimKeywordlist localKWL = kwl;
   return r_loadState(m_fieldsDoc.getRoot(), m_doc.getRoot(), localKWL);
}
#include <ossim/support_data/ossimNitfSicdXmlDes.h>
#include <ossim/base/ossimPreferences.h>
#include <iomanip>
#include <iostream>
#include <sstream>

ossimNitfSicdXmlDes::ossimNitfSicdXmlDes()
   : ossimNitfRegisteredDes(std::string("SICD_XML"), 0),
     m_xmlString()
{
   clearFields();
}
void ossimNitfSicdXmlDes::parseStream(std::istream& in)
{
  clearFields();

  if(getSizeInBytes())
  {
    char* c = new char[getSizeInBytes()+1];
    in.read(c, getSizeInBytes());
    c[getSizeInBytes()] = '\0';
    m_xmlString.string().resize(getSizeInBytes());
    m_xmlString = c;
    m_xmlDocument = new ossimXmlDocument;;
    std::istringstream xmlStringStream(m_xmlString.string());
    m_xmlDocument->read(xmlStringStream);
    delete [] c;
    c = 0;
  }
}

void ossimNitfSicdXmlDes::writeStream(std::ostream& out)
{
}

std::ostream& ossimNitfSicdXmlDes::print(std::ostream& out,
                                        const std::string& prefix) const
{
   bool typeinfo = ossimString(ossimPreferences::instance()->findPreference("kwl_type_info")).toBool();

   std::string pfx = prefix;
   pfx += getDesName() + ".";
   
   out << setiosflags(ios::left)
        // Quick hack to print on one line, this will leave extra spaces in CDATA sections
       << pfx << std::setw(24) << "XML: \"\"\"" << m_xmlString << "\"\"\"\n";
   return out;
}

void ossimNitfSicdXmlDes::clearFields()
{
}

bool ossimNitfSicdXmlDes::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   return true;
}

bool ossimNitfSicdXmlDes::loadValueFromXml(const ossimRefPtr<ossimXmlDocument> xml, const ossimString& xpath, ossimString& target) const
{
  bool result = false;
  if (xml->getErrorStatus()) return result;
  std::vector<ossimRefPtr<ossimXmlNode> > xml_nodes;
  xml->findNodes(xpath, xml_nodes);
  if (xml_nodes.size())
  { 
    target = xml_nodes[0]->getText();
    result = true;
  }

  return result;
}


#include <ossim/support_data/ossimNitfXmlDataContentDes.h>
#include <ossim/base/ossimPreferences.h>
#include <iomanip>
#include <iostream>
#include <sstream>

//<CHILDCLASSCPP>

ossimNitfXmlDataContentDes::ossimNitfXmlDataContentDes()
   : ossimNitfRegisteredDes(std::string("XML_DATA_CONTENT"), 0),
     m_xmlString()
{
   clearFields();
}

void ossimNitfXmlDataContentDes::parseStream(std::istream& in)
{
   in.read(m_descrc, DESCRC_SIZE);
   in.read(m_desshft, DESSHFT_SIZE);
   in.read(m_desshdt, DESSHDT_SIZE);
   in.read(m_desshrp, DESSHRP_SIZE);
   in.read(m_desshsi, DESSHSI_SIZE);
   in.read(m_desshsv, DESSHSV_SIZE);
   in.read(m_desshsd, DESSHSD_SIZE);
   in.read(m_desshtn, DESSHTN_SIZE);

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

void ossimNitfXmlDataContentDes::writeStream(std::ostream& out)
{
   out.write(m_desshl, DESSHL_SIZE);
   out.write(m_descrc, DESCRC_SIZE);
   out.write(m_desshft, DESSHFT_SIZE);
   out.write(m_desshdt, DESSHDT_SIZE);
   out.write(m_desshrp, DESSHRP_SIZE);
   out.write(m_desshsi, DESSHSI_SIZE);
   out.write(m_desshsv, DESSHSV_SIZE);
   out.write(m_desshsd, DESSHSD_SIZE);
   out.write(m_desshtn, DESSHTN_SIZE);
}

std::ostream& ossimNitfXmlDataContentDes::print(std::ostream& out,
                                        const std::string& prefix) const
{
   bool typeinfo = ossimString(ossimPreferences::instance()->findPreference("kwl_type_info")).toBool();

   std::string pfx = prefix;
   pfx += getDesName() + ".";
   
   out << setiosflags(ios::left)
       << pfx << std::setw(24) << "DESCRC:" << ((typeinfo) ? "(string)" : "") << m_descrc << "\n"
       << pfx << std::setw(24) << "DESSHFT:" << ((typeinfo) ? "(string)" : "") << m_desshft << "\n"
       << pfx << std::setw(24) << "DESSHDT:" << ((typeinfo) ? "(string)" : "") << m_desshdt << "\n"
       << pfx << std::setw(24) << "DESSHRP:" << ((typeinfo) ? "(string)" : "") << m_desshrp << "\n"
       << pfx << std::setw(24) << "DESSHSI:" << ((typeinfo) ? "(string)" : "") << m_desshsi << "\n"
       << pfx << std::setw(24) << "DESSHSV:" << ((typeinfo) ? "(string)" : "") << m_desshsv << "\n"
       << pfx << std::setw(24) << "DESSHSD:" << ((typeinfo) ? "(string)" : "") << m_desshsd << "\n"
       << pfx << std::setw(24) << "DESSHTN:" << ((typeinfo) ? "(string)" : "") << m_desshtn << "\n"
       // Quick hack to print on one line, this will leave extra spaces in CDATA sections
       << pfx << std::setw(24) << "XML:" << m_xmlString.substitute(ossimString("\n"), " ", true) << "\n"
   ;
   return out;
}

void ossimNitfXmlDataContentDes::clearFields()
{
   memset(m_descrc, ' ', DESCRC_SIZE);
   m_descrc[DESCRC_SIZE] = '\0';
   memset(m_desshft, ' ', DESSHFT_SIZE);
   m_desshft[DESSHFT_SIZE] = '\0';
   memset(m_desshdt, ' ', DESSHDT_SIZE);
   m_desshdt[DESSHDT_SIZE] = '\0';
   memset(m_desshrp, ' ', DESSHRP_SIZE);
   m_desshrp[DESSHRP_SIZE] = '\0';
   memset(m_desshsi, ' ', DESSHSI_SIZE);
   m_desshsi[DESSHSI_SIZE] = '\0';
   memset(m_desshsv, ' ', DESSHSV_SIZE);
   m_desshsv[DESSHSV_SIZE] = '\0';
   memset(m_desshsd, ' ', DESSHSD_SIZE);
   m_desshsd[DESSHSD_SIZE] = '\0';
   memset(m_desshtn, ' ', DESSHTN_SIZE);
   m_desshtn[DESSHTN_SIZE] = '\0';
}

bool ossimNitfXmlDataContentDes::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   const char* lookup;
   lookup = kwl.find(prefix, "DESCRC");
   {
      strcpy(m_descrc, lookup);
   }
   lookup = kwl.find(prefix, "DESSHFT");
   {
      strcpy(m_desshft, lookup);
   }
   lookup = kwl.find(prefix, "DESSHDT");
   {
      strcpy(m_desshdt, lookup);
   }
   lookup = kwl.find(prefix, "DESSHRP");
   {
      strcpy(m_desshrp, lookup);
   }
   lookup = kwl.find(prefix, "DESSHSI");
   {
      strcpy(m_desshsi, lookup);
   }
   lookup = kwl.find(prefix, "DESSHSV");
   {
      strcpy(m_desshsv, lookup);
   }
   lookup = kwl.find(prefix, "DESSHSD");
   {
      strcpy(m_desshsd, lookup);
   }
   lookup = kwl.find(prefix, "DESSHTN");
   {
      strcpy(m_desshtn, lookup);
   }

   return true;
}

bool ossimNitfXmlDataContentDes::loadValueFromXml(const ossimRefPtr<ossimXmlDocument> xml, const ossimString& xpath, ossimString& target) const
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

ossimString ossimNitfXmlDataContentDes::getDescrc() const
{
   return ossimString(m_descrc);
}

ossimString ossimNitfXmlDataContentDes::getDesshft() const
{
   return ossimString(m_desshft);
}

ossimString ossimNitfXmlDataContentDes::getDesshdt() const
{
   return ossimString(m_desshdt);
}

ossimString ossimNitfXmlDataContentDes::getDesshrp() const
{
   return ossimString(m_desshrp);
}

ossimString ossimNitfXmlDataContentDes::getDesshsi() const
{
   return ossimString(m_desshsi);
}
 
ossimString ossimNitfXmlDataContentDes::getDesshsv() const
{
   return ossimString(m_desshsv);
}

ossimString ossimNitfXmlDataContentDes::getDesshsd() const
{
   return ossimString(m_desshsd);
}

ossimString ossimNitfXmlDataContentDes::getDesshtn() const
{
   return ossimString(m_desshtn);
}

#include <ossim/support_data/ossimNitfSicdXmlDataContentDes.h>
#include <ossim/base/ossimPreferences.h>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

//<CHILDCLASSCPP>

ossimNitfSicdXmlDataContentDes::ossimNitfSicdXmlDataContentDes()
   : ossimNitfRegisteredDes(std::string("XML_DATA_CONTENT"), 0)
{
   clearFields();
}

void ossimNitfSicdXmlDataContentDes::parseStream(std::istream& in)
{
  clearFields();

  if (m_desName.contains("XML_DATA_CONTENT"))
  {

	  in.read(m_descrc, DESCRC_SIZE);
	  in.read(m_desshft, DESSHFT_SIZE);
	  in.read(m_desshdt, DESSHDT_SIZE);
	  in.read(m_desshrp, DESSHRP_SIZE);
	  in.read(m_desshsi, DESSHSI_SIZE);
	  in.read(m_desshsv, DESSHSV_SIZE);
	  in.read(m_desshsd, DESSHSD_SIZE);
	  in.read(m_desshtn, DESSHTN_SIZE);
	  in.read(m_desshlpg, DESSHLPG_SIZE);
	  in.read(m_desshlpt, DESSHLPT_SIZE);
	  in.read(m_desshli, DESSHLI_SIZE);
	  in.read(m_desshlin, DESSHLIN_SIZE);
	  in.read(m_desshabs, DESSHABS_SIZE);
  }

  if(getSizeInBytes())
  {
	  m_desData.resize(getSizeInBytes());
	  char* bufStart = &m_desData.front();
	  in.read(bufStart, getSizeInBytes());
  }
}

void ossimNitfSicdXmlDataContentDes::writeStream(std::ostream& out)
{
	if (m_desName.contains("XML_DATA_CONTENT"))
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
		out.write(m_desshlpg, DESSHLPG_SIZE);
		out.write(m_desshlpt, DESSHLPT_SIZE);
		out.write(m_desshli, DESSHLI_SIZE);
		out.write(m_desshlin, DESSHLIN_SIZE);
		out.write(m_desshabs, DESSHABS_SIZE);
	}
	if (!m_desData.empty())
	{
		out.write(&m_desData.front(), m_desData.size());
	}
}

std::ostream& ossimNitfSicdXmlDataContentDes::print(std::ostream& out,
                                        const std::string& prefix) const
{
   bool typeinfo = ossimString(ossimPreferences::instance()->findPreference("kwl_type_info")).toBool();
   std::string pfx = prefix;
   // pfx += getDesName() + ".";
   ossimString desData;
	getDesDataAsString(desData);
	out << setiosflags(ios::left)
		 << pfx << std::setw(24) << "DESCRC:" << ((typeinfo) ? "(string)" : "") << m_descrc << "\n"
		 << pfx << std::setw(24) << "DESSHFT:" << ((typeinfo) ? "(string)" : "") << m_desshft << "\n"
		 << pfx << std::setw(24) << "DESSHDT:" << ((typeinfo) ? "(string)" : "") << m_desshdt << "\n"
		 << pfx << std::setw(24) << "DESSHRP:" << ((typeinfo) ? "(string)" : "") << m_desshrp << "\n"
		 << pfx << std::setw(24) << "DESSHSI:" << ((typeinfo) ? "(string)" : "") << m_desshsi << "\n"
		 << pfx << std::setw(24) << "DESSHSV:" << ((typeinfo) ? "(string)" : "") << m_desshsv << "\n"
		 << pfx << std::setw(24) << "DESSHSD:" << ((typeinfo) ? "(string)" : "") << m_desshsd << "\n"
		 << pfx << std::setw(24) << "DESSHTN:" << ((typeinfo) ? "(string)" : "") << m_desshtn << "\n"
		 << pfx << std::setw(24) << "DESSHLPG:" << ((typeinfo) ? "(string)" : "") << m_desshlpg << "\n"
		 << pfx << std::setw(24) << "DESSHLPT:" << ((typeinfo) ? "(string)" : "") << m_desshlpt << "\n"
		 << pfx << std::setw(24) << "DESSHLI:" << ((typeinfo) ? "(string)" : "") << m_desshli << "\n"
		 << pfx << std::setw(24) << "DESSHABS:" << ((typeinfo) ? "(string)" : "") << m_desshabs << "\n"
		 << pfx << std::setw(24) << "DESDATA:" << ((typeinfo) ? "(string)" : "") << "\"\"\"" << desData << "\"\"\"\n"
		;
// might be better to leave it as the raw XML under the DESDATA value
#if 0
	if (desData)
	{
		ossimXmlDocument doc;
		if(doc.readString(desData))
		{
			ossimKeywordlist kwl;
			doc.toKwl(kwl, pfx);
			out << kwl << "\n";
		}
	}
#endif
		return out;
}

void ossimNitfSicdXmlDataContentDes::clearFields()
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
   memset(m_desshlpg, ' ', DESSHLPG_SIZE);
   m_desshlpg[DESSHLPG_SIZE] = '\0';
   memset(m_desshlpt, ' ', DESSHLPT_SIZE);
   m_desshlpt[DESSHLPT_SIZE] = '\0';
   memset(m_desshli, ' ', DESSHLI_SIZE);
   m_desshli[DESSHLI_SIZE] = '\0';
   memset(m_desshlin, ' ', DESSHLIN_SIZE);
   m_desshlin[DESSHLIN_SIZE] = '\0';
   memset(m_desshabs, ' ', DESSHABS_SIZE);
   m_desshabs[DESSHABS_SIZE] = '\0';
   m_desData.clear();
}

bool ossimNitfSicdXmlDataContentDes::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimString lookup;
	clearFields();
   lookup = kwl.find(prefix, "DESCRC");
	if (lookup)
	{
		strcpy(m_descrc, lookup);
   }
   lookup = kwl.find(prefix, "DESSHFT");
	if (lookup)
	{
		strcpy(m_desshft, lookup);
   }
   lookup = kwl.find(prefix, "DESSHDT");
	if (lookup)
	{
		strcpy(m_desshdt, lookup);
   }
   lookup = kwl.find(prefix, "DESSHRP");
	if (lookup)
	{
		strcpy(m_desshrp, lookup);
   }
   lookup = kwl.find(prefix, "DESSHSI");
	if (lookup)
	{
		strcpy(m_desshsi, lookup);
   }
   lookup = kwl.find(prefix, "DESSHSV");
	if (lookup)
	{
		strcpy(m_desshsv, lookup);
   }
   lookup = kwl.find(prefix, "DESSHSD");
	if (lookup)
	{
		strcpy(m_desshsd, lookup);
   }
	lookup = kwl.find(prefix, "DESSHTN");
	if (lookup)
	{
		strcpy(m_desshtn, lookup);
	}
	lookup = kwl.find(prefix, "DESDATA");
	if(lookup)
	{
		ossimString value = lookup;
		m_desData.insert(m_desData.begin(),
		                 value.begin(), 
		                 value.end());
	}

	return true;
}

bool ossimNitfSicdXmlDataContentDes::saveState(ossimKeywordlist &kwl, const char *prefix)const
{
	ossimString copyPrefix = prefix;
	ossimString desData;
	ossimNitfRegisteredDes::saveState(kwl, prefix);
	kwl.add(copyPrefix + "DESCRC", m_descrc);
	kwl.add(copyPrefix + "DESSHFT", m_desshft);
	kwl.add(copyPrefix + "DESSHDT", m_desshdt);
	kwl.add(copyPrefix + "DESSHRP", m_desshrp);
	kwl.add(copyPrefix + "DESSHSI", m_desshsi);
	kwl.add(copyPrefix + "DESSHSV", m_desshsv);
	kwl.add(copyPrefix + "DESSHSD", m_desshsd);
	kwl.add(copyPrefix + "DESSHTN", m_desshtn);
	getDesDataAsString(desData);
	kwl.add(copyPrefix + "DESDATA", desData);

	return true;
}

// bool ossimNitfSicdXmlDataContentDes::loadValueFromXml(const ossimRefPtr<ossimXmlDocument> xml, const ossimString &xpath, ossimString &target) const
// {
//   bool result = false;
//   if (xml->getErrorStatus()) return result;
//   std::vector<ossimRefPtr<ossimXmlNode> > xml_nodes;
//   xml->findNodes(xpath, xml_nodes);
//   if (xml_nodes.size())
//   { 
//     target = xml_nodes[0]->getText();
//     result = true;
//   }

//   return result;
// }

ossimString ossimNitfSicdXmlDataContentDes::getDescrc() const
{
   return ossimString(m_descrc);
}

ossimString ossimNitfSicdXmlDataContentDes::getDesshft() const
{
   return ossimString(m_desshft);
}

ossimString ossimNitfSicdXmlDataContentDes::getDesshdt() const
{
   return ossimString(m_desshdt);
}

ossimString ossimNitfSicdXmlDataContentDes::getDesshrp() const
{
   return ossimString(m_desshrp);
}

ossimString ossimNitfSicdXmlDataContentDes::getDesshsi() const
{
   return ossimString(m_desshsi);
}
 
ossimString ossimNitfSicdXmlDataContentDes::getDesshsv() const
{
   return ossimString(m_desshsv);
}

ossimString ossimNitfSicdXmlDataContentDes::getDesshsd() const
{
   return ossimString(m_desshsd);
}

ossimString ossimNitfSicdXmlDataContentDes::getDesshtn() const
{
   return ossimString(m_desshtn);
}

void ossimNitfSicdXmlDataContentDes::getDesDataAsString(ossimString& result) const
{
	if(!m_desData.empty())
	{
		const char* bufStart = &m_desData.front();
		result = ossimString(bufStart, bufStart + m_desData.size());
	}
	else
	{
		result = "";
	}
}

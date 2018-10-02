#ifndef ossimNitfSicdXmlDes_HEADER
#define ossimNitfSicdXmlDes_HEADER

#include <ossim/support_data/ossimNitfRegisteredDes.h>
#include <ossim/base/ossimXmlDocument.h>
#include <ossim/base/ossimXmlNode.h>

class OSSIM_DLL ossimNitfSicdXmlDes : public ossimNitfRegisteredDes
{
public:
   ossimNitfSicdXmlDes();
   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   virtual void clearFields();
   bool loadState(const ossimKeywordlist& kwl, const char* prefix);
   bool loadValueFromXml(const ossimRefPtr<ossimXmlDocument>, const ossimString& xpath, ossimString& target) const;

protected:
   ossimString m_xmlString;
   ossimRefPtr<ossimXmlDocument> m_xmlDocument;

};

#endif
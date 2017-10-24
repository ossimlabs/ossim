#include <ossim/support_data/ossimNitfRegisteredDes.h>
#include <ossim/base/ossimXmlDocument.h>
#include <ossim/base/ossimXmlNode.h>

class OSSIM_DLL ossimNitfXmlDataContentDes : public ossimNitfRegisteredDes
{
public:
   enum
   {
      DESSHL_SIZE = 4,
      DESCRC_SIZE = 5,
      DESSHFT_SIZE = 8,
      DESSHDT_SIZE = 20,
      DESSHRP_SIZE = 40,
      DESSHSI_SIZE = 60,
      DESSHSV_SIZE = 10,
      DESSHSD_SIZE = 20,
      DESSHTN_SIZE = 120,
      DESSHLPG_SIZE = 125,
      DESSHLPT_SIZE = 25,
      DESSHLI_SIZE = 20,
      DESSHLIN_SIZE = 120,
      DESSHABS_SIZE = 200
   };
   ossimNitfXmlDataContentDes();

   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   virtual void clearFields();
   bool loadState(const ossimKeywordlist& kwl, const char* prefix);
   bool loadValueFromXml(const ossimRefPtr<ossimXmlDocument>, const ossimString& xpath, ossimString& target) const;

   ossimString getDesshl() const;
   ossim_uint32 getDesshlAsUint32() const;

   ossimString getDescrc() const;

   ossimString getDesshft() const;

   ossimString getDesshdt() const;

   ossimString getDesshrp() const;

   ossimString getDesshsi() const;

   ossimString getDesshsv() const;

   ossimString getDesshsd() const;

   ossimString getDesshtn() const;


protected:
   char m_desshl[DESSHL_SIZE+1];
   char m_descrc[DESCRC_SIZE+1];
   char m_desshft[DESSHFT_SIZE+1];
   char m_desshdt[DESSHDT_SIZE+1];
   char m_desshrp[DESSHRP_SIZE+1];
   char m_desshsi[DESSHSI_SIZE+1];
   char m_desshsv[DESSHSV_SIZE+1];
   char m_desshsd[DESSHSD_SIZE+1];
   char m_desshtn[DESSHTN_SIZE+1];
   char m_desshlpg[DESSHLPG_SIZE+1];
   char m_desshlpt[DESSHLPT_SIZE+1];
   char m_desshli[DESSHLI_SIZE+1];
   char m_desshlin[DESSHLIN_SIZE+1];
   char m_desshabs[DESSHABS_SIZE+1];
   
   ossimString m_xmlString;
   ossimRefPtr<ossimXmlDocument> m_xmlDocument;
};

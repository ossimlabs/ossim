//*******************************************************************
//
// LICENSE: MIT
//
// Author:
// Description: Nitf support class
// 
//********************************************************************
// $Id$

#include <ossim/support_data/ossimNitfDesInformation.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfDesFactoryRegistry.h>
#include <ossim/base/ossimPreferences.h>
#include <sstream>
#include <iomanip>
#include <cstring> // for memset

ossimNitfDesInformation::ossimNitfDesInformation(ossimRefPtr<ossimNitfRegisteredDes> desData)
{
   clearFields();
   m_segSecurityMetadata.setSegmentPrefix(std::string("DE"));
   if (desData.valid())
   {
      setDesName(desData->getDesName());
      setDesData(desData);
   }
}

ossimNitfDesInformation::~ossimNitfDesInformation()
{
}

void ossimNitfDesInformation::parseStream(std::istream &in, ossim_uint64 dataLength)
{
   if(in)
   {
      clearFields();
      m_desOffset = in.tellg();
      in.read(m_de, DE_SIZE);
      in.read(m_desid, DESID_SIZE);
      in.read(m_desver, DESVER_SIZE);
      
      m_segSecurityMetadata.parseStream(in);

#if 0
      in.read(m_declas, DECLAS_SIZE);
      in.read(m_desclsy, DESCLSY_SIZE);
      in.read(m_descode, DESCODE_SIZE);
      in.read(m_desctlh, DESCTLH_SIZE);
      in.read(m_desrel, DESREL_SIZE);
      in.read(m_desdctp, DESDCTP_SIZE);
      in.read(m_desdcdt, DESDCDT_SIZE);
      in.read(m_desdcxm, DESDCXM_SIZE);
      in.read(m_desdg, DESDG_SIZE);
      in.read(m_desdgdt, DESDGDT_SIZE);
      in.read(m_descltx, DESCLTX_SIZE);
      in.read(m_descatp, DESCATP_SIZE);
      in.read(m_descaut, DESCAUT_SIZE);
      in.read(m_descrsn, DESCRSN_SIZE);
      in.read(m_dessrdt, DESSRDT_SIZE);
      in.read(m_desctln, DESCTLN_SIZE);
#endif
      
      if (getDesId() == "TRE_OVERFLOW")
      {
        in.read(m_desoflw, DESOFLW_SIZE);
        in.read(m_desitem, DESITEM_SIZE);
      }
      in.read(m_desshl, DESSHL_SIZE);

      if(in.good())
      {
         m_desDataOffset = in.tellg();

         m_desData = ossimNitfDesFactoryRegistry::instance()->create(getDesId());

         if (m_desData.valid())
         {
            //---
            // Dess with dynamic des length construct with 0 length.
            // Set if 0.
            //---
            if (m_desData->getDesLength() == 0)
            {
               m_desData->setDesLength(dataLength);
            }
            // Sanity check fixed length in code with length from CEL field:
            else if (m_desData->getDesLength() != getDesLength())
            {
               ossimNotify(ossimNotifyLevel_WARN)
                   << "ossimNitfDesInformation::parseStream WARNING!"
                   << "\nCEL field length does not match fixed des length for des: "
                   << m_desData->getDesName().c_str()
                   << "\nCEL: " << getDesLength()
                   << "\nDes: " << m_desData->getDesLength()
                   << std::endl;
            }

            m_desData->parseStream(in);
            if(!in.good())
            {
               m_desData = 0;
               in.clear();
            }
         }
         else
         {
            m_desData = (ossimNitfRegisteredDes *)NULL;
         }
      }
   }
   
}

void ossimNitfDesInformation::writeStream(std::ostream &out)
{
   m_desOffset = out.tellp(); // Capture the offset.

   out.write(m_de, DE_SIZE);
   out.write(m_desid, DESID_SIZE);
   out.write(m_desver, DESVER_SIZE);

   m_segSecurityMetadata.writeStream(out);

#if 0
   out.write(m_declas, DECLAS_SIZE);
   out.write(m_desclsy, DESCLSY_SIZE);
   out.write(m_descode, DESCODE_SIZE);
   out.write(m_desctlh, DESCTLH_SIZE);
   out.write(m_desrel,  DESREL_SIZE);
   out.write(m_desdctp, DESDCTP_SIZE);
   out.write(m_desdcdt, DESDCDT_SIZE);
   out.write(m_desdcxm, DESDCXM_SIZE);
   out.write(m_desdg, DESDG_SIZE);
   out.write(m_desdgdt, DESDGDT_SIZE);
   out.write(m_descltx, DESCLTX_SIZE);
   out.write(m_descatp, DESCATP_SIZE);
   out.write(m_descaut, DESCAUT_SIZE);
   out.write(m_descrsn, DESCRSN_SIZE);
   out.write(m_dessrdt, DESSRDT_SIZE);
   out.write(m_desctln, DESCTLN_SIZE);
#endif
   
   if (getDesId() == "TRE_OVERFLOW")
   {
     out.write(m_desoflw, DESOFLW_SIZE);
     out.write(m_desitem, DESITEM_SIZE);
   }
   out.write(m_desshl, DESSHL_SIZE);


   //out.write(m_desLength, 5);
   /*
   if(m_desData.valid())
   {
      m_desData = out.tellp();
      m_desData->writeStream(out);
   }
   */
}

ossim_uint32 ossimNitfDesInformation::getTotalDesLength()const
{
   return (getDesLength() + (ossim_uint32)11);
}

ossim_uint32 ossimNitfDesInformation::getDesLength()const
{
   return ossimString(m_desLength).toUInt32();
}

ossim_uint64 ossimNitfDesInformation::getDesOffset()const
{
   return m_desOffset;
}

ossim_uint64 ossimNitfDesInformation::getDesDataOffset()const
{
   return m_desDataOffset;
}

ossimString ossimNitfDesInformation::getDesId()const
{
   return ossimString(m_desid).trim();
}

ossimString ossimNitfDesInformation::getDesVer()const
{
   return ossimString(m_desver).trim();
}

std::ostream& ossimNitfDesInformation::print(std::ostream& out, 
                                             const std::string& prefix)const
{
    ossimString pfx = prefix;
    int w = (int)((pfx.size()<29)?29-pfx.size():24);
    
    bool typeinfo = ossimString(ossimPreferences::instance()->findPreference("kwl_type_info")).toBool();
    out << std::setiosflags(std::ios::left)
        << pfx << std::setw(w) << "DE:" << ((typeinfo) ? "(string)" : "") << m_de << "\n"
        << pfx << std::setw(w) << "DESID:" << ((typeinfo) ? "(string)" : "") << m_desid << "\n"
        << pfx << std::setw(w) << "DESVER:" << ((typeinfo) ? "(string)" : "") << m_desver << "\n";
    
    m_segSecurityMetadata.print(out, pfx);

#if 0
           << pfx << std::setw(w) << "DECLAS:" << ((typeinfo) ? "(string)" : "") << m_declas << "\n"
           << pfx << std::setw(w) << "DESCLSY:" << ((typeinfo) ? "(string)" : "") << m_desclsy << "\n"
           << pfx << std::setw(w) << "DESCODE:" << ((typeinfo) ? "(string)" : "") << m_descode << "\n"
           << pfx << std::setw(w) << "DESCTLH:" << ((typeinfo) ? "(string)" : "") << m_desctlh << "\n"
           << pfx << std::setw(w) << "DESREL:" << ((typeinfo) ? "(string)" : "") << m_desrel << "\n"
           << pfx << std::setw(w) << "DESDCTP:" << ((typeinfo) ? "(string)" : "") << m_desdctp << "\n"
           << pfx << std::setw(w) << "DESDCDT:" << ((typeinfo) ? "(string)" : "") << m_desdcdt << "\n"
           << pfx << std::setw(w) << "DESDCXM:" << ((typeinfo) ? "(string)" : "") << m_desdcxm << "\n"
           << pfx << std::setw(w) << "DESDG:" << ((typeinfo) ? "(string)" : "") << m_desdg << "\n"
           << pfx << std::setw(w) << "DESDGDT:" << ((typeinfo) ? "(string)" : "") << m_desdgdt << "\n"
           << pfx << std::setw(w) << "DESCLTX:" << ((typeinfo) ? "" : "") << m_descltx << "\n"
           << pfx << std::setw(w) << "DESCATP:" << ((typeinfo) ? "(string)" : "") << m_descatp << "\n"
           << pfx << std::setw(w) << "DESCAUT:" << ((typeinfo) ? "(string)" : "") << m_descaut << "\n"
           << pfx << std::setw(w) << "DESCRSN:" << ((typeinfo) ? "(string)" : "") << m_descrsn << "\n"
           << pfx << std::setw(w) << "DESSRDT:" << ((typeinfo) ? "(string)" : "") << m_dessrdt << "\n"
           << pfx << std::setw(w) << "DESCTLN:" << ((typeinfo) ? "(string)" : "") << m_desctln << "\n";
#endif
    
  if (getDesId() == "TRE_OVERFLOW")
   {
     out
         << pfx << std::setw(w) << "DESOFLW:" << ((typeinfo) ? "(string)" : "") << m_desoflw << "\n"
         << pfx << std::setw(w) << "DESITEM:" << ((typeinfo) ? "(string)" : "") << m_desitem << "\n"
     ;
   }

   out
       << pfx << std::setw(w) << "DESSHL:" << ((typeinfo) ? "(string)" : "") << m_desshl << "\n"
   ;

   if (getDesData().valid())
   {
      getDesData()->print(out, prefix);
   }
   return out;
}

void ossimNitfDesInformation::clearFields()
{
   memset(m_de, ' ', DE_SIZE);
   m_de[DE_SIZE] = '\0';
   memset(m_desid, ' ', DESID_SIZE);
   m_desid[DESID_SIZE] = '\0';
   memcpy(m_desver, "01", DESVER_SIZE); // 01 or 02 per spec
   // memset(m_desver, ' ', DESVER_SIZE);
   m_desver[DESVER_SIZE] = '\0';

#if 0
   memset(m_declas, ' ', DECLAS_SIZE);
   m_declas[DECLAS_SIZE] = '\0';
   memset(m_desclsy, ' ', DESCLSY_SIZE);
   m_desclsy[DESCLSY_SIZE] = '\0';
   memset(m_descode, ' ', DESCODE_SIZE);
   m_descode[DESCODE_SIZE] = '\0';
   memset(m_desctlh, ' ', DESCTLH_SIZE);
   m_desctlh[DESCTLH_SIZE] = '\0';
   memset(m_desrel, ' ', DESREL_SIZE);
   m_desrel[DESREL_SIZE] = '\0';
   memset(m_desdctp, ' ', DESDCTP_SIZE);
   m_desdctp[DESDCTP_SIZE] = '\0';
   memset(m_desdcdt, ' ', DESDCDT_SIZE);
   m_desdcdt[DESDCDT_SIZE] = '\0';
   memset(m_desdcxm, ' ', DESDCXM_SIZE);
   m_desdcxm[DESDCXM_SIZE] = '\0';
   memset(m_desdg, ' ', DESDG_SIZE);
   m_desdg[DESDG_SIZE] = '\0';
   memset(m_desdgdt, ' ', DESDGDT_SIZE);
   m_desdgdt[DESDGDT_SIZE] = '\0';
   memset(m_descltx, ' ', DESCLTX_SIZE);
   m_descltx[DESCLTX_SIZE] = '\0';
   memset(m_descatp, ' ', DESCATP_SIZE);
   m_descatp[DESCATP_SIZE] = '\0';
   memset(m_descaut, ' ', DESCAUT_SIZE);
   m_descaut[DESCAUT_SIZE] = '\0';
   memset(m_descrsn, ' ', DESCRSN_SIZE);
   m_descrsn[DESCRSN_SIZE] = '\0';
   memset(m_dessrdt, ' ', DESSRDT_SIZE);
   m_dessrdt[DESSRDT_SIZE] = '\0';
   memset(m_desctln, ' ', DESCTLN_SIZE);
   m_desctln[DESCTLN_SIZE] = '\0';
#endif
   
   memset(m_desoflw, ' ', DESOFLW_SIZE);
   m_desoflw[DESOFLW_SIZE] = '\0';
   memset(m_desitem, ' ', DESITEM_SIZE);
   m_desitem[DESITEM_SIZE] = '\0';
   memset(m_desshl, '0', DESSHL_SIZE);
   m_desshl[DESSHL_SIZE] = '\0';
   
   m_desOffset     = 0;
   m_desDataOffset = 0;
}

void ossimNitfDesInformation::setDesName(const ossimString& desName)
{
   ossimNitfCommon::setField(m_desid, desName, DESID_SIZE);

#if 0
   memset(m_desid, ' ', DESID_SIZE);
   std::ostringstream out;
   out << std::setw(DESID_SIZE)
       << std::setfill(' ')
       << m_desid;
   memcpy(m_desid, out.str().c_str(), DESID_SIZE);
#endif
}

void ossimNitfDesInformation::setDesLength(ossim_uint32 desLength)
{
   /*
   memset(m_desLength, 0, 5);
   
   if(desLength > 99999)
   {
      desLength = 99999;
   }

   std::ostringstream out;

   out << std::setw(5)
       << std::setfill('0')
       << desLength;
   
   memcpy(m_desLength, out.str().c_str(), 5);
   */
}

ossimRefPtr<ossimNitfRegisteredDes> ossimNitfDesInformation::getDesData()
{
   return m_desData;
}

const ossimRefPtr<ossimNitfRegisteredDes> ossimNitfDesInformation::getDesData()const
{
   return m_desData;
}

void ossimNitfDesInformation::setDesData(ossimRefPtr<ossimNitfRegisteredDes> desData)
{
   m_desData = desData;

   /*
   memset(m_desid, ' ', DESID_SIZE);
   memset(m_desLength, ' ', 5);
   
   if(m_desData.valid())
   {
      setDesName(m_desData->getRegisterDesName());
      setDesLength(m_desData->getSizeInBytes());
   }
   */
}

ossimString ossimNitfDesInformation::get_desshl() const
{
   return ossimString(m_desshl);
}

ossim_uint32 ossimNitfDesInformation::getDesSubHeaderLength() const
{
   return ossimString(m_desshl).toUInt32();
}

void ossimNitfDesInformation::setSegmentSecurityMetadata(
   const ossimNitfSegmentSecurityMetadataV1& obj)
{
   m_segSecurityMetadata = obj;
}

const ossimNitfSegmentSecurityMetadataV1& ossimNitfDesInformation::getSegmentSecurityMetadata() const
{
   return m_segSecurityMetadata;
}

ossimNitfSegmentSecurityMetadataV1& ossimNitfDesInformation::getSegmentSecurityMetadata()
{
   return m_segSecurityMetadata;
}

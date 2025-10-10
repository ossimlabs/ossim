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
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfDesFactoryRegistry.h>
#include <ossim/base/ossimPreferences.h>
#include <sstream>
#include <iomanip>
#include <cstring>

static ossimTrace traceDebug(ossimString("ossimNitfDesInformation:debug"));

ossimNitfDesInformation::ossimNitfDesInformation(ossimRefPtr<ossimNitfRegisteredDes> desData)
   : m_desOffset(0),
     m_desDataOffset(0),
     m_desDataSize(0),
     m_desData(desData)
{
   clearFields();
   m_segSecurityMetadata.setFilePartType(std::string("DE"));
   if (desData.valid())
   {
      setDesName(desData->get_desid());
      set_desshl(desData->getDesSubHeaderLength());
      setDesLength(desData->getDesDataLength());
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

      m_desDataSize = dataLength;
      m_desOffset = in.tellg();
      
      in.read(m_de, DE_SIZE);
      in.read(m_desid, DESID_SIZE);
      in.read(m_desver, DESVER_SIZE);
      
      m_segSecurityMetadata.parseStream(in);
      
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
            // Record parsing is split between ossimNitfDesInformation and
            // registered DES class so set things parsed here but needed in
            // registered des class.
            //---
            m_desData->set_desid(getDesId().string());
            m_desData->set_desver(getDesVer().string());
            m_desData->setDesSubHeaderLength(getDesSubHeaderLength());
            m_desData->setDesDataLength(dataLength);

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

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimNitfDesInformation::parseStream(...)"
         << "\ndes id:             " << m_desid
         << "\ndes version:        " << m_desver
         << "\ndes offset:         " << m_desOffset
         << "\ndes sub hdr length: " << m_desshl
         << "\ndes data offset:    " << m_desDataOffset
         << "\ndes data length:    " << dataLength
         << "\n";
   }
}

void ossimNitfDesInformation::writeStream(std::ostream &out)
{
   m_desOffset = out.tellp(); // Capture the offset.

   out.write(m_de, DE_SIZE);
   out.write(m_desid, DESID_SIZE);
   out.write(m_desver, DESVER_SIZE);

   m_segSecurityMetadata.writeStream(out);
   
   if (getDesId() == "TRE_OVERFLOW")
   {
     out.write(m_desoflw, DESOFLW_SIZE);
     out.write(m_desitem, DESITEM_SIZE);
   }

   out.write(m_desshl, DESSHL_SIZE);

   if(m_desData.valid())
   {
      m_desDataOffset = out.tellp();
      m_desData->writeStream(out);
   }
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
   return ossimString(m_desid).trim().upcase();
}

bool ossimNitfDesInformation::isTreOverflow() const
{
   return ossimString(m_desoflw).trim().upcase().contains("TRE_OVERFLOW");
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
   
   if (getDesId() == "TRE_OVERFLOW")
   {
      out
         << pfx << std::setw(w) << "DESOFLW:" << ((typeinfo) ? "(string)" : "") << m_desoflw << "\n"
         << pfx << std::setw(w) << "DESITEM:" << ((typeinfo) ? "(string)" : "") << m_desitem << "\n"
         ;
   }
   
   out << pfx << std::setw(w) << "DESSHL:" << ((typeinfo) ? "(string)" : "") << m_desshl << "\n";
   
   if (getDesData().valid())
   {
      getDesData()->print(out, prefix);
   }
   return out;
}

void ossimNitfDesInformation::clearFields()
{
   memcpy(m_de, "DE", DE_SIZE);
   memset(m_desid, ' ', DESID_SIZE);
   memcpy(m_desver, "01", DESVER_SIZE); // 01 or 02 per spec
   memset(m_desoflw, ' ', DESOFLW_SIZE);
   memset(m_desitem, ' ', DESITEM_SIZE);
   memset(m_desshl, '0', DESSHL_SIZE);
   
   m_de[DE_SIZE] = '\0';
   m_desid[DESID_SIZE] = '\0';
   m_desver[DESVER_SIZE] = '\0';
   m_desoflw[DESOFLW_SIZE] = '\0';
   m_desitem[DESITEM_SIZE] = '\0';
   m_desshl[DESSHL_SIZE] = '\0';
   
   m_desOffset     = 0;
   m_desDataOffset = 0;
}

void ossimNitfDesInformation::setDesName(const ossimString& desName)
{
   ossimNitfCommon::setField(m_desid, desName, DESID_SIZE);
}

void ossimNitfDesInformation::setDesLength(ossim_uint32 desLength)
{
   setDesDataLength(desLength);
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

std::string ossimNitfDesInformation::get_desshl() const
{
   return std::string(m_desshl);
}

void ossimNitfDesInformation::set_desshl(ossim_uint32 length)
{
   ossimNitfCommon::setField(m_desshl,
                             ossimString::toString(length),
                             DESSHL_SIZE,
                             std::ios::right,
                             '0');
}

ossim_uint32 ossimNitfDesInformation::getDesSubHeaderLength() const
{
   return ossimString(m_desshl).toUInt32();
}

ossim_uint32 ossimNitfDesInformation::getDesHeaderLength() const
{
   // If overflow add bytes for m_desoflw and m_desitem fields.
   return 200 + getDesSubHeaderLength() + (isTreOverflow()?9:0);
}

void ossimNitfDesInformation::setDesDataLength(ossim_uint32 length)
{
   m_desDataSize = length;
}

ossim_uint32 ossimNitfDesInformation::getDesDataLength() const
{
   return m_desDataSize;
}

ossim_uint32 ossimNitfDesInformation::getDesTotalLength() const
{
   return getDesDataLength() + getDesHeaderLength();
}

bool ossimNitfDesInformation::operator<(const ossimNitfDesInformation& rhs) const
{
   return getDesTotalLength() < rhs.getDesTotalLength();
}

void ossimNitfDesInformation::setSegmentSecurityMetadata(
   const ossimNitfSegmentSecurityMetadataV2_1& obj)
{
   m_segSecurityMetadata = obj;
}

const ossimNitfSegmentSecurityMetadataV2_1& ossimNitfDesInformation::getSegmentSecurityMetadata() const
{
   return m_segSecurityMetadata;
}

ossimNitfSegmentSecurityMetadataV2_1& ossimNitfDesInformation::getSegmentSecurityMetadata()
{
   return m_segSecurityMetadata;
}

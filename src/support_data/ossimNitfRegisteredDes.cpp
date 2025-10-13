//*******************************************************************
//
// License: MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id$

#include <ossim/support_data/ossimNitfRegisteredDes.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sstream>

RTTI_DEF2(ossimNitfRegisteredDes, "ossimNitfRegisteredDes", ossimObject, ossimPropertyInterface)

ossimNitfRegisteredDes::ossimNitfRegisteredDes()
   : 
   ossimObject(),
   ossimPropertyInterface(),
   m_desid(),
   m_desver(),
   m_desshl(0),
   m_desLength(0)
{}

ossimNitfRegisteredDes::ossimNitfRegisteredDes(const std::string& desName, ossim_uint32 desLength)
   : 
   ossimObject(),
   ossimPropertyInterface(),
   m_desid(desName),
   m_desver(),
   m_desshl(0),
   m_desLength(desLength)
{}

ossimNitfRegisteredDes::~ossimNitfRegisteredDes()
{}

std::string ossimNitfRegisteredDes::getRegisterDesName() const
{
   // Deprecated:
   return m_desid;
}

const std::string& ossimNitfRegisteredDes::get_desid() const
{
   return m_desid;
}

void ossimNitfRegisteredDes::set_desid(const std::string& desid)
{
   m_desid = desid;
}

const std::string& ossimNitfRegisteredDes::get_desver() const
{
   return m_desver;
}

ossim_uint32 ossimNitfRegisteredDes::getDesVersionNumber() const
{
   return ossimString(m_desver).toUInt32();
}

void ossimNitfRegisteredDes::set_desver(const std::string& desver)
{
   m_desver = desver;
}

ossim_uint32 ossimNitfRegisteredDes::getDesSubHeaderLength() const
{
   return m_desshl;
}

bool ossimNitfRegisteredDes::setDesSubHeaderLength(ossim_uint32 desshl)
{
   bool status = true;
   if ( desshl <= 9999)
   {
      m_desshl = desshl;
   }
   else
   {
      status = false;
   }
   return status;
}

#if 0
ossim_uint32 ossimNitfRegisteredDes::getSizeInBytes() const
{
   // Deprecated:
   return getDesLength();
}
#endif

const std::vector<ossim_int8> &ossimNitfRegisteredDes::getDesDataBuffer() const
{
   return m_desData;
}

ossim_uint32 ossimNitfRegisteredDes::getDesDataLength() const
{
   return m_desLength;
}

void ossimNitfRegisteredDes::setDesDataLength(ossim_uint32 length)
{
   m_desLength = length;
}

void ossimNitfRegisteredDes::setProperty(ossimRefPtr<ossimProperty> /* property */)
{
}

ossimRefPtr<ossimProperty> ossimNitfRegisteredDes::getProperty(const ossimString& /* name */)const
{
   return 0;
}

void ossimNitfRegisteredDes::getPropertyNames(std::vector<ossimString>& /* propertyNames */)const
{
}

void ossimNitfRegisteredDes::getMetadata(ossimKeywordlist& /* kwl */,
                                         const char* /* prefix */ ) const
{
}

std::ostream& ossimNitfRegisteredDes::print(
   std::ostream& out, const std::string& /* prefix */) const
{
   // return print(out);
   return out;
}

// bool ossimNitfRegisteredDes::saveState(ossimKeywordlist& kwl, const ossimString& prefix)const
bool ossimNitfRegisteredDes::saveState(ossimKeywordlist& kwl, const char* prefix)const   
{
   bool result = false;
   // by default we will use the already implemented print method as backup
   //
   std::ostringstream out;
   
   print(out);
   
   ossimKeywordlist kwlTemp;
   
   std::istringstream in(out.str());
   if(kwlTemp.parseStream(in))
   {
      result = true;
      kwl.add(prefix, kwlTemp);
   }
   
   return result;
}

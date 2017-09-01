//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfRegisteredDes.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <ossim/support_data/ossimNitfRegisteredDes.h>

RTTI_DEF2(ossimNitfRegisteredDes, "ossimNitfRegisteredDes", ossimObject, ossimPropertyInterface)

ossimNitfRegisteredDes::ossimNitfRegisteredDes()
   : 
   ossimObject(),
   ossimPropertyInterface(),
   m_desName(),
   m_desLength(0)
{}
ossimNitfRegisteredDes::ossimNitfRegisteredDes(const std::string& desName, ossim_uint32 desLength)
   : 
   ossimObject(),
   ossimPropertyInterface(),
   m_desName(desName),
   m_desLength(desLength)
{}

ossimNitfRegisteredDes::~ossimNitfRegisteredDes()
{}

std::string ossimNitfRegisteredDes::getRegisterDesName() const
{
   // Deprecated:
   return m_desName;
}

const std::string& ossimNitfRegisteredDes::getDesName() const
{
   return m_desName;
}

void ossimNitfRegisteredDes::setDesName(const std::string& desName)
{
   m_desName = desName;
}

ossim_uint32 ossimNitfRegisteredDes::getSizeInBytes() const
{
   // Deprecated:
   return getDesLength();
}

ossim_uint32 ossimNitfRegisteredDes::getDesLength() const
{
   return m_desLength;
}

void ossimNitfRegisteredDes::setDesLength(ossim_uint32 length)
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

bool ossimNitfRegisteredDes::saveState(ossimKeywordlist& kwl, const ossimString& prefix)const
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

//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: CCF Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <iostream>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/support_data/ossimCcfInfo.h>
#include <ossim/imaging/ossimCcfHead.h>

ossimCcfInfo::ossimCcfInfo()
{
}

ossimCcfInfo::~ossimCcfInfo()
{
}

bool ossimCcfInfo::open(const ossimFilename& file)
{
   std::string connectionString = file.c_str();
   std::shared_ptr<ossim::istream> str = ossim::StreamFactoryRegistry::instance()->
      createIstream( file.c_str(), std::ios_base::in|std::ios_base::binary);

   if(!str) return false;
   
   return open(str, connectionString);

}

bool ossimCcfInfo::open(std::shared_ptr<ossim::istream>& str,
                        const std::string& connectionString)
{
   bool result = false;
   m_fileStr.reset();
   m_connectionString.clear();
   ossimString extension = ossimFilename(connectionString).ext();

   extension.downcase();

   if (extension == "ccf")
   {
      m_fileStr = str;
      m_connectionString = connectionString;
      result = true;
   }

   return result;

} 

std::ostream& ossimCcfInfo::print(std::ostream& out) const
{
   if(m_fileStr)
   {
      ossimCcfHead ccf(m_fileStr, m_connectionString);
      out << "File:  " << m_connectionString << "\n" << ccf;
   }
   return out;
}

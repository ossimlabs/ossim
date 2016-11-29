//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: USGS DEM Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <iostream>

#include <ossim/support_data/ossimDemInfo.h>

#include <ossim/base/ossimErrorCodes.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>

#include <ossim/support_data/ossimDemHeader.h>
#include <ossim/support_data/ossimDemUtil.h>

ossimDemInfo::ossimDemInfo()
{
}

ossimDemInfo::~ossimDemInfo()
{
}

bool ossimDemInfo::open(const ossimFilename& file)
{
   std::string connectionString = file.c_str();
   std::shared_ptr<ossim::istream> str = ossim::StreamFactoryRegistry::instance()->
      createIstream( file.c_str(), std::ios_base::in|std::ios_base::binary);

   return open(str, connectionString);

   // bool result = ossimDemUtil::isUsgsDem(file);

   // if ( result )
   // {
   //    theFile = file;
   // }
   // else
   // {
   //    theFile = ossimFilename::NIL;
   // }
   
   // return result;
}

bool ossimDemInfo::open(std::shared_ptr<ossim::istream>& str,
                        const std::string& connectionString)
{

   bool result = ossimDemUtil::isUsgsDem(str, connectionString);

   if ( result )
   {
      m_fileStr = str;
      m_connectionString = connectionString;
   }
   else
   {
      m_connectionString = "";
   }
   
   return result;
  
}

std::ostream& ossimDemInfo::print(std::ostream& out) const
{
   if ( m_fileStr )
   {
      ossimDemHeader hdr;
      m_fileStr->clear();
      m_fileStr->seekg(0);
      if ( hdr.open(m_fileStr, m_connectionString) )
      {
         // std::string prefix;
         hdr.print(std::cout);
      }
   }
   return out;
}

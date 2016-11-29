//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Description: NITF Info object.
// 
//----------------------------------------------------------------------------
// $Id$



#include <ossim/support_data/ossimNitfInfo.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimIoStream.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>

#include <ostream>
#include <sstream>
#include <memory>

ossimNitfInfo::ossimNitfInfo()
   : m_nitfFile(0)
{
}

ossimNitfInfo::~ossimNitfInfo()
{
   m_nitfFile.reset();
}

bool ossimNitfInfo::open(const ossimFilename& file)
{
   std::string connectionString = file.c_str();
   std::shared_ptr<ossim::istream> str = ossim::StreamFactoryRegistry::instance()->
      createIstream( file.c_str(), std::ios_base::in|std::ios_base::binary);

   return open(str, connectionString);

}

bool ossimNitfInfo::open(std::shared_ptr<ossim::istream>& str,
                         const std::string& connectionString)
{
   m_nitfFile = std::make_shared<ossimNitfFile>();
   bool result = m_nitfFile->parseStream(ossimFilename(connectionString), *str);


   return result;
}

std::ostream& ossimNitfInfo::print(std::ostream& out) const
{
   if ( m_nitfFile )
   {
      std::string prefix;
      m_nitfFile->print(out, prefix, getProcessOverviewFlag());
   }
   return out;
}


bool ossimNitfInfo::getKeywordlist(ossimKeywordlist& kwl,
                                   ossim_uint32 entryIndex)const
{
   // Do a print to a memory stream.
   std::ostringstream out;
   m_nitfFile->print( out, entryIndex );

   std::istringstream in( out.str() );

   // Give the result to the keyword list.
   return kwl.parseStream( in );
}

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
#include <ostream>
#include <sstream>

ossimNitfInfo::ossimNitfInfo()
   : m_nitfFile(0)
{
}

ossimNitfInfo::~ossimNitfInfo()
{
   m_nitfFile = 0;
}

bool ossimNitfInfo::open(const ossimFilename& file)
{
   m_nitfFile = new ossimNitfFile();

   bool result = m_nitfFile->parseFile(file);

   if (result == false)
   {
      m_nitfFile = 0;
   }

   return result;
}

std::ostream& ossimNitfInfo::print(std::ostream& out) const
{
   if ( m_nitfFile.valid() )
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

//---
//
// License: MIT
// 
// Author: David Burken
//
// Description: Info object for Quickbird metadata with a collection of
// metadata sidecar files, i.e. dot TIL, IMD, ATT, GEO, EPH, and RPB.
// 
//---
// $Id$

#include <ossim/support_data/ossimQuickbirdMetaDataInfo.h>
#include <ossim/support_data/ossimQuickbirdMetaData.h>
#include <iostream>

ossimQuickbirdMetaDataInfo::ossimQuickbirdMetaDataInfo()
   : ossimInfoBase(),
     m_file(),
     m_info()
{
}

bool ossimQuickbirdMetaDataInfo::open(const ossimFilename& file)
{
   bool result = false;
   m_info.clear();
   m_file = file;
   m_file.setExtension(ossimString("IMD"));
   if ( m_file.exists() )
   {
      ossimQuickbirdMetaData qbmd;
      result = qbmd.open( m_file ) ;
      if ( result )
      {
         std::string s = "qbmd.";
         qbmd.saveState( m_info, s.c_str() );
      }
   }
   return result;
}

std::ostream& ossimQuickbirdMetaDataInfo::print(std::ostream& out) const
{
   out << m_info << "\n";
   return out;
}

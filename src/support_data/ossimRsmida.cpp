//---
// File: ossimRsmida.cpp
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//---

#include <ossim/support_data/ossimRsmida.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/support_data/ossimNitfRsmidaTag.h>

static std::string IID_KW     = "iid";
static std::string EDITION_KW = "edition";

#if 0
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";
static std::string _KW = "";

#endif

ossimRsmida::ossimRsmida()
   :
   m_iid(),
   m_edition()
{
}

ossimRsmida::ossimRsmida( const ossimRsmida& obj )
   :
   m_iid( obj.m_iid ),
   m_edition( obj.m_edition )
{
}

const ossimRsmida& ossimRsmida::operator=( const ossimRsmida& rhs )
{
   if (this != &rhs)
   {
      m_iid = rhs.m_iid;
      m_edition = rhs.m_edition;
   }
   return *this;
}

void ossimRsmida::saveState( ossimKeywordlist& kwl,
                             const std::string& prefix ) const
{
   std::string pfx = prefix + std::string("rsmida.");
   
   kwl.add(pfx.c_str(), IID_KW.c_str(), m_iid.c_str());
   kwl.add(pfx.c_str(), EDITION_KW.c_str(), m_edition.c_str());
   
} // End: ossimRsmida::saveState( ... )

bool ossimRsmida::loadState( const ossimKeywordlist& kwl,
                             const std::string& prefix )
{
   std::string pfx = prefix + std::string("rsmida.");
   std::string key;
   std::string value;
   
   bool result = false; // Set to true on last key.
      
   while( 1 ) // Break out on error.
   {
      key = IID_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         m_iid = value;
      }
      else
      {
         break;
      }

      key = EDITION_KW;
      value = kwl.findKey( pfx, key );
      if ( value.size() )
      {
        m_edition = value;
      }
      else
      {
         break;
      }


      // If we get here we're good, so set the status for downstream code.
      result = true;

      // Final break from while forever loop.
      break;
      
   } // Matches: while( FOREVER )

   if ( result == false )
   {  
      // Find on key failed...
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossimRsmida::loadState WARNING:\n"
         << "Error encountered parsing the following required keyword: "
         << "<" << key << ">. Check the keywordlist for proper syntax."
         << std::endl;
   }
   
   return result;
   
} // End: ossimRsmida::loadState( ... )

bool ossimRsmida::initialize( const ossimNitfRsmidaTag* rsmidaTag )
{
   bool status = false;
   
   if ( rsmidaTag )
   {
      m_iid = rsmidaTag->getIid().trim().string();
      m_edition = rsmidaTag->getEdition().trim().string();

      // m_r0 = rsmidaTag->getR0().toFloat64();
      // m_rnis = rsmidaTag->getRnis().toUInt32();

      status = true;
      
   } // Matches: if ( rsmidaTag )

   return status;
   
} // End: ossimRsmida::initializeModel( rsmidaTag )

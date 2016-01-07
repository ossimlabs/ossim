//---
// File: ossimRsmpca.h
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//---
#ifndef ossimRsmpca_H
#define ossimRsmpca_H 1

#include <ossim/base/ossimConstants.h>
#include <string>
#include <vector>

class ossimKeywordlist;
class ossimNitfRsmpcaTag;

/**
 * @class Public container for RSM Polynomial Coefficients(RSMPCA) TRE.
 *
 * Reference:
 * Replacement Sensor Model Tagged Record Extensions Specification for NITF 2.1
 * section 9.0(RSMPCA).
 * 
 */
class ossimRsmpca
{
public:
   
   /** @brief default constructor */
   ossimRsmpca();

   /** @brief copy constructor */
   ossimRsmpca( const ossimRsmpca& obj );

   /** @brief assignment operator */
   const ossimRsmpca& operator=( const ossimRsmpca& rhs );

   /**
    * @brief saveState Saves state to keyword list.
    * @param kwl
    * @param prefix
    * @param index
    */
   void saveState( ossimKeywordlist& kwl,
                   const std::string& prefix,
                   ossim_uint32 index ) const;
   
   /**
    * @brief loadState Loads state from keyword list.
    * @param kwl
    * @param prefix
    * @param index
    * @return true on success; false, on error.
    */
   bool loadState( const ossimKeywordlist& kwl,
                   const std::string& prefix,
                   ossim_uint32 index );
   
   /**
    * @brief initialize Initializes from nitf rsmpia tag.
    * @param rsmpiaTag
    * @return true on success; false, on error.
    */
   bool initialize( const ossimNitfRsmpcaTag* rsmpcaTag );

   std::string m_iid;
   std::string m_edition;

   ossim_uint32 m_rsn;
   ossim_uint32 m_csn;

   ossim_float64 m_rfep;
   ossim_float64 m_cfep;

   ossim_float64 m_rnrmo;
   ossim_float64 m_cnrmo;
   ossim_float64 m_xnrmo;
   ossim_float64 m_ynrmo;
   ossim_float64 m_znrmo;

   ossim_float64 m_rnrmsf;
   ossim_float64 m_cnrmsf;
   ossim_float64 m_xnrmsf;
   ossim_float64 m_ynrmsf;
   ossim_float64 m_znrmsf;
   
   ossim_uint32 m_rnpwrx;
   ossim_uint32 m_rnpwry;
   ossim_uint32 m_rnpwrz;
   ossim_uint32 m_rntrms;

   std::vector<ossim_float64> m_rnpcf;

   ossim_uint32 m_rdpwrx;
   ossim_uint32 m_rdpwry;
   ossim_uint32 m_rdpwrz;
   ossim_uint32 m_rdtrms;

   std::vector<ossim_float64> m_rdpcf;

   ossim_uint32 m_cnpwrx;
   ossim_uint32 m_cnpwry;
   ossim_uint32 m_cnpwrz;
   ossim_uint32 m_cntrms;

   std::vector<ossim_float64> m_cnpcf;

   ossim_uint32 m_cdpwrx;
   ossim_uint32 m_cdpwry;
   ossim_uint32 m_cdpwrz;
   ossim_uint32 m_cdtrms;

   std::vector<ossim_float64> m_cdpcf;

}; // End: class ossimRsmpca()

#endif /* #ifndef ossimRsmpca_H */

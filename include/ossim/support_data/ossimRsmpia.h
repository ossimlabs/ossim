//---
// File: ossimRsmpia.h
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//---
#ifndef ossimRsmpia_H
#define ossimRsmpia_H 1

#include <ossim/base/ossimConstants.h>
#include <string>

class ossimKeywordlist;
class ossimNitfRsmpiaTag;

/**
 * @class Public container RSM Polynomial Identification(RSMPIA) TRE.
 *
 * Reference:
 * Replacement Sensor Model Tagged Record Extensions Specification for NITF 2.1
 * section 8.0(RSMPIA).
 */
class ossimRsmpia
{
public:
   
   /** @brief default constructor */
   ossimRsmpia();

   /** @brief copy constructor */
   ossimRsmpia( const ossimRsmpia& obj );

   /** @brief assignment operator */
   const ossimRsmpia& operator=( const ossimRsmpia& rhs );

   /**
    * @brief saveState Saves state to keyword list.
    * @param kwl
    * @param prefix
    */
   void saveState( ossimKeywordlist& kwl,
                   const std::string& prefix ) const;

   /**
    * @brief loadState Loads state from keyword list.
    * @param kwl
    * @param prefix
    * @return true on success; false, on error.
    */
   bool loadState( const ossimKeywordlist& kwl,
                   const std::string& prefix );

   /**
    * @brief initialize Initializes from nitf rsmpia tag.
    * @param rsmpiaTag
    * @return true on success; false, on error.
    */
   bool initialize( const ossimNitfRsmpiaTag* rsmpiaTag );
   
   std::string   m_iid;
   std::string   m_edition;

   ossim_float64 m_r0;
   ossim_float64 m_rx;
   ossim_float64 m_ry;
   ossim_float64 m_rz;

   ossim_float64 m_rxx;
   ossim_float64 m_rxy;
   ossim_float64 m_rxz;

   ossim_float64 m_ryy;
   ossim_float64 m_ryz;
   ossim_float64 m_rzz;

   ossim_float64 m_c0;
   ossim_float64 m_cx;
   ossim_float64 m_cy;
   ossim_float64 m_cz;

   ossim_float64 m_cxx;
   ossim_float64 m_cxy;
   ossim_float64 m_cxz;   

   ossim_float64 m_cyy;
   ossim_float64 m_cyz;
   ossim_float64 m_czz;

   ossim_uint32  m_rnis;
   ossim_uint32  m_cnis;
   ossim_uint32  m_tnis;

   ossim_uint32  m_rssiz;
   ossim_uint32  m_cssiz;

}; // End: class ossimRsmpia()

#endif /* #ifndef ossimRsmpia_H */

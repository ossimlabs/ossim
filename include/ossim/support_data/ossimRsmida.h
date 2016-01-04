//---
// File: ossimRsmida.h
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//---
#ifndef ossimRsmida_H
#define ossimRsmida_H 1

#include <ossim/base/ossimConstants.h>
#include <string>

class ossimKeywordlist;
class ossimNitfRsmidaTag;

/**
 * @class Public container RSM Identification(RSMIDA) TRE.
 *
 * Reference:
 * Replacement Sensor Model Tagged Record Extensions Specification for NITF 2.1
 * section 7.0(RSMIDA).
 */
class ossimRsmida
{
public:
   
   /** @brief default constructor */
   ossimRsmida();

   /** @brief copy constructor */
   ossimRsmida( const ossimRsmida& obj );

   /** @brief assignment operator */
   const ossimRsmida& operator=( const ossimRsmida& rhs );

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
    * @brief initialize Initializes from nitf rsmida tag.
    * @param rsmidaTag
    * @return true on success; false, on error.
    */
   bool initialize( const ossimNitfRsmidaTag* rsmidaTag );
   
   std::string   m_iid;
   std::string   m_edition;

   std::string m_isid;
   std::string m_sid;
   std::string m_stid;
   
   ossim_uint32 m_year;
   ossim_uint32 m_month;
   ossim_uint32 m_day;
   ossim_uint32 m_hour;
   ossim_uint32 m_minute;
   ossim_float64 m_second;

   ossim_uint32 m_nrg;
   ossim_uint32 m_ncg;
   
   ossim_float64 m_trg;
   ossim_float64 m_tcg;

   std::string m_grndd;

   ossim_float64 m_xuor;
   ossim_float64 m_yuor;
   ossim_float64 m_zuor;

   ossim_float64 m_xuxr;
   ossim_float64 m_xuyr;
   ossim_float64 m_xuzr;

   ossim_float64 m_yuxr;
   ossim_float64 m_yuyr;
   ossim_float64 m_yuzr;

   ossim_float64 m_zuxr;
   ossim_float64 m_zuyr;
   ossim_float64 m_zuzr;

   ossim_float64 m_v1x;
   ossim_float64 m_v1y;
   ossim_float64 m_v1z;

   ossim_float64 m_v2x;
   ossim_float64 m_v2y;
   ossim_float64 m_v2z;

   ossim_float64 m_v3x;
   ossim_float64 m_v3y;
   ossim_float64 m_v3z;

   ossim_float64 m_v4x;
   ossim_float64 m_v4y;
   ossim_float64 m_v4z;

   ossim_float64 m_v5x;
   ossim_float64 m_v5y;
   ossim_float64 m_v5z;

   ossim_float64 m_v6x;
   ossim_float64 m_v6y;
   ossim_float64 m_v6z;

   ossim_float64 m_v7x;
   ossim_float64 m_v7y;
   ossim_float64 m_v7z;

   ossim_float64 m_v8x;
   ossim_float64 m_v8y;
   ossim_float64 m_v8z;

   ossim_float64 m_grpx;
   ossim_float64 m_grpy;
   ossim_float64 m_grpz;

   ossim_uint32 m_fullr;
   ossim_uint32 m_fullc;

   ossim_uint32 m_minr;
   ossim_uint32 m_maxr;
   ossim_uint32 m_minc;
   ossim_uint32 m_maxc;

   ossim_float64 m_ie0;
   ossim_float64 m_ier;
   ossim_float64 m_iec;
   
   ossim_float64 m_ierr;
   ossim_float64 m_ierc;
   ossim_float64 m_iecc;

   ossim_float64 m_ia0;
   ossim_float64 m_iar;
   ossim_float64 m_iac;
   
   ossim_float64 m_iarr;
   ossim_float64 m_iarc;
   ossim_float64 m_iacc;

   ossim_float64 m_spx;
   ossim_float64 m_svx;
   ossim_float64 m_sax;

   ossim_float64 m_spy;
   ossim_float64 m_svy;
   ossim_float64 m_say;

   ossim_float64 m_spz;
   ossim_float64 m_svz;
   ossim_float64 m_saz;

}; // End: class ossimRsmida()

#endif /* #ifndef ossimRsmida!_H */

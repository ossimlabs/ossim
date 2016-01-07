//---
// File: ossimNitfRsmidaTag.h
//---
#ifndef ossimNitfRsmidaTag_HEADER
#define ossimNitfRsmidaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>

/**
 * @class ossimNitfRsmidaTag
 * RSM Identification: Id's, time-of-image model, optional illumination model,
 * footprint information.
 */
class OSSIM_DLL ossimNitfRsmidaTag : public ossimNitfRegisteredTag
{
public:
   enum
   {
      IID_SIZE     = 80,
      EDITION_SIZE = 40,
      ISID_SIZE    = 40,
      SID_SIZE     = 40,
      STID_SIZE    = 40,
      YEAR_SIZE    = 4,
      MONTH_SIZE   = 2,
      DAY_SIZE     = 2,
      HOUR_SIZE    = 2,
      MINUTE_SIZE  = 2,
      SECOND_SIZE  = 9,
      NRG_SIZE     = 8,
      NCG_SIZE     = 8,
      FLOAT21_SIZE = 21,
      GRNDD_SIZE   = 1,
      FULL_SIZE    = 8,
      MIN_SIZE     = 8,
      MAX_SIZE     = 8,
      CEL_SIZE     = 1628
   };
   ossimNitfRsmidaTag();

   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   virtual void clearFields();

   ossimString getIid() const;
   ossimString getEdition() const;
   ossimString getIsid() const;
   ossimString getSid() const;
   ossimString getStid() const;
   ossimString getYear() const;
   ossimString getMonth() const;
   ossimString getDay() const;
   ossimString getHour() const;
   ossimString getMinute() const;
   ossimString getSecond() const;
   ossimString getNrg() const;
   ossimString getNcg() const;
   ossimString getTrg() const;
   ossimString getTcg() const;
   ossimString getGrndd() const;
   
   ossimString getXuor() const;
   ossimString getYuor() const;
   ossimString getZuor() const;
   ossimString getXuxr() const;
   ossimString getXuyr() const;
   ossimString getXuzr() const;
   ossimString getYuxr() const;
   ossimString getYuyr() const;
   ossimString getYuzr() const;
   ossimString getZuxr() const;
   ossimString getZuyr() const;
   ossimString getZuzr() const;
   
   ossimString getV1x() const;
   ossimString getV1y() const;
   ossimString getV1z() const;
   ossimString getV2x() const;
   ossimString getV2y() const;
   ossimString getV2z() const;
   ossimString getV3x() const;
   ossimString getV3y() const;
   ossimString getV3z() const;
   ossimString getV4x() const;
   ossimString getV4y() const;
   ossimString getV4z() const;
   ossimString getV5x() const;
   ossimString getV5y() const;
   ossimString getV5z() const;
   ossimString getV6x() const;
   ossimString getV6y() const;
   ossimString getV6z() const;
   ossimString getV7x() const;
   ossimString getV7y() const;
   ossimString getV7z() const;
   ossimString getV8x() const;
   ossimString getV8y() const;
   ossimString getV8z() const;
   
   ossimString getGrpx() const;
   ossimString getGrpy() const;
   ossimString getGrpz() const;

   ossimString getFullr() const;
   ossimString getFullc() const;

   ossimString getMinr() const;
   ossimString getMaxr() const;
   ossimString getMinc() const;
   ossimString getMaxc() const;

   ossimString getIe0() const;
   ossimString getIer() const;
   ossimString getIec() const;
   ossimString getIerr() const;
   ossimString getIerc() const;
   ossimString getIecc() const;
   ossimString getIa0() const;
   ossimString getIar() const;
   ossimString getIac() const;
   ossimString getIarr() const;
   ossimString getIarc() const;
   ossimString getIacc() const;

   ossimString getSpx() const;
   ossimString getSvx() const;
   ossimString getSax() const;
   ossimString getSpy() const;
   ossimString getSvy() const;
   ossimString getSay() const;
   ossimString getSpz() const;
   ossimString getSvz() const;
   ossimString getSaz() const;

protected:
   char m_iid[IID_SIZE+1];
   char m_edition[EDITION_SIZE+1];
   char m_isid[ISID_SIZE+1];
   char m_sid[SID_SIZE+1];
   char m_stid[STID_SIZE+1];
   char m_year[YEAR_SIZE+1];
   char m_month[MONTH_SIZE+1];
   char m_day[DAY_SIZE+1];
   char m_hour[HOUR_SIZE+1];
   char m_minute[MINUTE_SIZE+1];
   char m_second[SECOND_SIZE+1];
   char m_nrg[NRG_SIZE+1];
   char m_ncg[NCG_SIZE+1];
   char m_trg[FLOAT21_SIZE+1];
   char m_tcg[FLOAT21_SIZE+1];
   char m_grndd[GRNDD_SIZE+1];

   char m_xuor[FLOAT21_SIZE+1];
   char m_yuor[FLOAT21_SIZE+1];
   char m_zuor[FLOAT21_SIZE+1];
   char m_xuxr[FLOAT21_SIZE+1];
   char m_xuyr[FLOAT21_SIZE+1];
   char m_xuzr[FLOAT21_SIZE+1];
   char m_yuxr[FLOAT21_SIZE+1];
   char m_yuyr[FLOAT21_SIZE+1];
   char m_yuzr[FLOAT21_SIZE+1];
   char m_zuxr[FLOAT21_SIZE+1];
   char m_zuyr[FLOAT21_SIZE+1];
   char m_zuzr[FLOAT21_SIZE+1];

   char m_v1x[FLOAT21_SIZE+1];
   char m_v1y[FLOAT21_SIZE+1];
   char m_v1z[FLOAT21_SIZE+1];
   char m_v2x[FLOAT21_SIZE+1];
   char m_v2y[FLOAT21_SIZE+1];
   char m_v2z[FLOAT21_SIZE+1];
   char m_v3x[FLOAT21_SIZE+1];
   char m_v3y[FLOAT21_SIZE+1];
   char m_v3z[FLOAT21_SIZE+1];
   char m_v4x[FLOAT21_SIZE+1];
   char m_v4y[FLOAT21_SIZE+1];
   char m_v4z[FLOAT21_SIZE+1];
   char m_v5x[FLOAT21_SIZE+1];
   char m_v5y[FLOAT21_SIZE+1];
   char m_v5z[FLOAT21_SIZE+1];
   char m_v6x[FLOAT21_SIZE+1];
   char m_v6y[FLOAT21_SIZE+1];
   char m_v6z[FLOAT21_SIZE+1];
   char m_v7x[FLOAT21_SIZE+1];
   char m_v7y[FLOAT21_SIZE+1];
   char m_v7z[FLOAT21_SIZE+1];
   char m_v8x[FLOAT21_SIZE+1];
   char m_v8y[FLOAT21_SIZE+1];
   char m_v8z[FLOAT21_SIZE+1];

   char m_grpx[FLOAT21_SIZE+1];
   char m_grpy[FLOAT21_SIZE+1];
   char m_grpz[FLOAT21_SIZE+1];

   char m_fullr[FULL_SIZE+1];
   char m_fullc[FULL_SIZE+1];

   char m_minr[MIN_SIZE+1];
   char m_maxr[MAX_SIZE+1];
   char m_minc[MIN_SIZE+1];
   char m_maxc[MAX_SIZE+1];

   char m_ie0[FLOAT21_SIZE+1];
   char m_ier[FLOAT21_SIZE+1];
   char m_iec[FLOAT21_SIZE+1];
   char m_ierr[FLOAT21_SIZE+1];
   char m_ierc[FLOAT21_SIZE+1];
   char m_iecc[FLOAT21_SIZE+1];
   char m_ia0[FLOAT21_SIZE+1];
   char m_iar[FLOAT21_SIZE+1];
   char m_iac[FLOAT21_SIZE+1];
   char m_iarr[FLOAT21_SIZE+1];
   char m_iarc[FLOAT21_SIZE+1];
   char m_iacc[FLOAT21_SIZE+1];

   char m_spx[FLOAT21_SIZE+1];
   char m_svx[FLOAT21_SIZE+1];
   char m_sax[FLOAT21_SIZE+1];
   char m_spy[FLOAT21_SIZE+1];
   char m_svy[FLOAT21_SIZE+1];
   char m_say[FLOAT21_SIZE+1];
   char m_spz[FLOAT21_SIZE+1];
   char m_svz[FLOAT21_SIZE+1];
   char m_saz[FLOAT21_SIZE+1];

TYPE_DATA
   
}; // End: class ossimNitfRsmidaTag

#endif /* #ifndef ossimNitfRsmidaTag_HEADER */

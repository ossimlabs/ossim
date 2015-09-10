//---
// File: ossimNitfRsmpiaTag.h
//---
#ifndef ossimNitfRsmpiaTag_HEADER
#define ossimNitfRsmpiaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>

class OSSIM_DLL ossimNitfRsmpiaTag : public ossimNitfRegisteredTag
{
public:
   enum
   {
      IID_SIZE     = 80,
      EDITION_SIZE = 40,
      FLOAT21_SIZE = 21,
      NIS_SIZE     = 3,
      CEL_SIZE     = 591
   };
   
   ossimNitfRsmpiaTag();

   virtual void parseStream(std::istream& in);
   virtual void writeStream(std::ostream& out);

   virtual std::ostream& print( std::ostream& out,
                                const std::string& prefix=std::string() ) const;
   virtual void clearFields();

protected:
   char m_iid[IID_SIZE+1];
   char m_edition[EDITION_SIZE+1];
   char m_r0[FLOAT21_SIZE+1];
   char m_rx[FLOAT21_SIZE+1];
   char m_ry[FLOAT21_SIZE+1];
   char m_rz[FLOAT21_SIZE+1];
   char m_rxx[FLOAT21_SIZE+1];
   char m_rxy[FLOAT21_SIZE+1];
   char m_rxz[FLOAT21_SIZE+1];
   char m_ryy[FLOAT21_SIZE+1];
   char m_ryz[FLOAT21_SIZE+1];
   char m_rzz[FLOAT21_SIZE+1];
   char m_c0[FLOAT21_SIZE+1];
   char m_cx[FLOAT21_SIZE+1];
   char m_cy[FLOAT21_SIZE+1];
   char m_cz[FLOAT21_SIZE+1];
   char m_cxx[FLOAT21_SIZE+1];
   char m_cxy[FLOAT21_SIZE+1];
   char m_cxz[FLOAT21_SIZE+1];
   char m_cyy[FLOAT21_SIZE+1];
   char m_cyz[FLOAT21_SIZE+1];
   char m_czz[FLOAT21_SIZE+1];
   char m_rnis[NIS_SIZE+1];
   char m_cnis[NIS_SIZE+1];
   char m_tnis[NIS_SIZE+1];
   char m_rssiz[FLOAT21_SIZE+1];
   char m_cssiz[FLOAT21_SIZE+1];

TYPE_DATA
   
}; // End: class ossimNitfRsmpiaTag

#endif /* #ifndef ossimNitfRsmpiaTag_HEADER */

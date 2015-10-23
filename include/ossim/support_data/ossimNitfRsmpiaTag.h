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

   /**
    * @brief Reads in tag from stream.
    * @param in
    */
   virtual void parseStream(std::istream& in);

   /**
    * @brief Writes out tag to stream.
    * @param out
    */
   virtual void writeStream(std::ostream& out);

   /**
    * @brief Memsets fields to space.  Null terminates fields.
    */
   virtual void clearFields();

   /**
    * @brief prints all fields.
    * @param out Output stream.
    * @param prefix e.g. "nitf.image0.".
    */
   virtual std::ostream& print( std::ostream& out,
                                const std::string& prefix=std::string() ) const;

   ossimString getIid() const;
   ossimString getEdition() const;
   ossimString getR0() const;
   ossimString getRx() const;
   ossimString getRy() const;
   ossimString getRz() const;
   ossimString getRxx() const;
   ossimString getRxy() const;
   ossimString getRxz() const;
   ossimString getRyy() const;
   ossimString getRyz() const;
   ossimString getRzz() const;
   ossimString getC0() const;
   ossimString getCx() const;
   ossimString getCy() const;
   ossimString getCz() const;
   ossimString getCxx() const;
   ossimString getCxy() const;
   ossimString getCxz() const;
   ossimString getCyy() const;
   ossimString getCyz() const;
   ossimString getCzz() const;
   ossimString getRnis() const;
   ossimString getCnis() const;
   ossimString getTnis() const;
   ossimString getRssiz() const;
   ossimString getCssiz() const;

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

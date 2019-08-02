//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: ENGRDA - Engineering Data tag class declaration.
//
// See document STDI-0002 (version 3), Appendix N for more info.
// 
//----------------------------------------------------------------------------
// $Id

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <ossim/support_data/ossimNitfEngrdaTag.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>

using namespace std;

static const ossimTrace traceDebug(ossimString("ossimNitfEngrda:debug"));

RTTI_DEF1(ossimNitfEngrdaTag, "ossimNitfEngrdaTag", ossimNitfRegisteredTag);

ossimNitfEngrdaTag::ossimNitfEngrdaTag()
   : ossimNitfRegisteredTag(std::string("ENGRDA"), 0)
{
   clearFields();
}

ossimNitfEngrdaTag::~ossimNitfEngrdaTag()
{
}

void ossimNitfEngrdaTag::parseStream(std::istream& in)
{
   clearFields();

   in.read(m_reSrc, RESRC_SIZE);
   in.read(m_reCnt, RECNT_SIZE);
 
   const ossim_uint16 ELEMENT_COUNT = ossimString(m_reCnt).toUInt16();
   
   for (ossim_uint16 i = 0; i < ELEMENT_COUNT; ++i)
   {
      ossimString os;
      ossim_uint32 size(0);
      
      char buf[TMP_BUF_SIZE];

      ossimEngDataElement element;

      // ENGLN - label length field
      in.read(element.m_engLn, ENGLN_SIZE);
      element.m_engLn[ENGLN_SIZE] = '\0';

      // ENGLBL - label field
      os = element.m_engLn;
      size = os.toUInt16();
      in.read(buf, size);
      buf[size] = '\0';
      element.m_engLbl = buf;

      // ENGMTXC - data column count
      in.read(element.m_engMtxC, ENGMTXC_SIZE);
      element.m_engMtxC[ENGMTXC_SIZE] = '\0';

      // ENGMTXR - data row count
      in.read(element.m_engMtxR, ENGMTXR_SIZE);
      element.m_engMtxR[ENGMTXR_SIZE] = '\0';

      // ENGTYP - Value Type of Engineering Data Element.
      in.get( element.m_engTyp );

      // ENGDTS - Engineering Data Element Size
      in.get( element.m_engDts );
      os = element.m_engDts;
      ossim_uint32 engDataElementSize = os.toUInt32();

      // ENGDATU - Engineering Data Units.
      in.read( element.m_engDatU, ENGDATU_SIZE );
      element.m_engDatU[ENGDATU_SIZE] = '\0';

      // ENGDATC - Engineering Data Count
      in.read(element.m_engDatC, ENGDATC_SIZE);
      element.m_engDatC[ENGDATC_SIZE] = '\0';
      os = element.m_engDatC;
      ossim_uint32 engDataCount = os.toUInt32();

      // ENGDATA - Engineering Data
      size = engDataElementSize * engDataCount;
      element.m_engDat.resize(size);
      in.read((char*)&(element.m_engDat.front()), size);
      
      m_data.push_back(element);

   } // Matches: for (ossim_uint16 i = 0; i < ELEMENT_COUNT; ++i)
}

void ossimNitfEngrdaTag::writeStream(std::ostream& out)
{
   //out.write(theAcMsnId, AC_MSN_ID_SIZE);
   // out.write(theAcTailNo, AC_TAIL_NO_SIZE);

   out.write(m_reSrc, RESRC_SIZE);
   out.write(m_reCnt, RECNT_SIZE);

   const ossim_uint16 ELEMENT_COUNT = ossimString(m_reCnt).toUInt16();
   
   for (ossim_uint16 i = 0; i < ELEMENT_COUNT; ++i)
   {
      // ENGLN - label length field      
      out.write(m_data[i].m_engLn, ENGLN_SIZE);

      // ENGLBL - label field
      out.write(m_data[i].m_engLbl.data(), (std::streamsize)m_data[i].m_engLbl.size());

      // ENGMTXC - data column count
      out.write(m_data[i].m_engMtxC, ENGMTXC_SIZE );

      // ENGMTXR - data row count
      out.write(m_data[i].m_engMtxR, ENGMTXR_SIZE );

      // ENGTYP - Value Type of Engineering Data Element.
      out.write( (char*)(&m_data[i].m_engTyp), ENGTYP_SIZE );

      // ENGDTS - Engineering Data Element Size
      out.write((char*)(&m_data[i].m_engDts), ENGDTS_SIZE );

      // ENGDATU - Engineering Data Units.
      out.write( m_data[i].m_engDatU, ENGDATU_SIZE );

      // ENGDATC - Engineering Data Count
      out.write( m_data[i].m_engDatC, ENGDATC_SIZE );

      // ENGDATA - Engineering Data NOTE: should be big endian...
      out.write((char*)&(m_data[i].m_engDat.front()),
                (std::streamsize)m_data[i].m_engDat.size());

   } // Matches: for (ossim_uint16 i = 0; i < ELEMENT_COUNT; ++i)
   
}

void ossimNitfEngrdaTag::clearFields()
{
   // BCS-N's to '0's, BCS-A's to ' '(spaces)

   // clear
   memset(m_reSrc, ' ', RESRC_SIZE);
   memset(m_reCnt, ' ', RECNT_SIZE);
   m_data.clear();

   // null terminate
   m_reSrc[RESRC_SIZE] = '\0';
   m_reCnt[RECNT_SIZE] = '\0';
}

std::ostream& ossimNitfEngrdaTag::print(
   std::ostream& out, const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";

   const ossim_uint32 W = 17;
   
   out << setiosflags(std::ios::left)
       << pfx << std::setw(W) << "CETAG:"
       << getTagName() << "\n"
       << pfx << std::setw(W) << "CEL:"   << getSizeInBytes() << "\n"
       << pfx << std::setw(W) << "RESRC:" << m_reSrc << "\n"
       << pfx << std::setw(W) << "RECNT:" << m_reCnt << "\n";

   for (ossim_uint32 i = 0; i < m_data.size(); ++i)
   {
      out << pfx << "ENGLN[" << i << std::setw(W-7) << "]:"
          << m_data[i].m_engLn << "\n"
          << pfx << "ENGLBL[" << i << std::setw(W-8) << "]:"
          << m_data[i].m_engLbl << "\n"
          << pfx << "ENGMTXC[" << i << std::setw(W-9) << "]:"
          << m_data[i].m_engMtxC << "\n"
          << pfx << "ENGMTXR[" << i << std::setw(W-9) << "]:"
          << m_data[i].m_engMtxR << "\n"
          << pfx << "ENGTYP[" << i << std::setw(W-8) << "]:"
          << std::string(1, m_data[i].m_engTyp) << "\n"
          << pfx << "ENGDTS[" << i << std::setw(W-8) << "]:"
          << std::string(1, m_data[i].m_engDts) << "\n"
          << pfx << "ENGDATU[" << i << std::setw(W-9) << "]:"
          << m_data[i].m_engDatU << "\n"
          << pfx << "ENGDATC[" << i << std::setw(W-9) << "]:"
          << m_data[i].m_engDat.size() << "\n";
      
      printData(out, m_data[i], i, pfx);
   }
   
   return out;
}

std::ostream& ossimNitfEngrdaTag::printData( std::ostream& out,
                                             const ossimEngDataElement& element,
                                             ossim_uint32 elIndex,
                                             const std::string& prefix ) const
{
   if (element.m_engTyp == 'A')
   {
      out << prefix << "ENGDATA[" << elIndex << std::setw(8) << "]:";
      
      std::vector<ossim_uint8>::const_iterator i = element.m_engDat.begin();
      while (i != element.m_engDat.end())
      {
         out << static_cast<ossim_int8>(*i);
         ++i;
      }
      out << "\n";
   }
   else
   {
      out << prefix << "ENGDATA[" << elIndex << std::setw(8) << "]:      NOT DISPLAYED\n";
      if ( traceDebug() )
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfEngrdaTag::printData WARNING unhandled data type."
            << "Data type = " << (char)element.m_engTyp << "\n"
            << std::endl;
      }
   } 
   return out;
}

template <class T>
void ossimNitfEngrdaTag::getValueAsString(
   T v, ossim_uint16 w, std::string& s) const
{
   std::ostringstream os;
   os << std::setw(w) << std::setfill('0') << setiosflags(ios::right) << v;
   s = os.str();
}
   

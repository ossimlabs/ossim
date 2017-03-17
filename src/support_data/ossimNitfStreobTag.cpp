//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
//
// Description: STREOB tag class definition.
//
// Dataset Indentification TRE.
//
//----------------------------------------------------------------------------
// $Id

#include <cstring>
#include <istream>
#include <iostream>
#include <iomanip>

#include <ossim/support_data/ossimNitfStreobTag.h>


RTTI_DEF1(ossimNitfStreobTag, "ossimNitfStreobTag", ossimNitfRegisteredTag);

ossimNitfStreobTag::ossimNitfStreobTag()
   : ossimNitfRegisteredTag(std::string("STREOB"), 94)
{
   clearFields();
}

ossimNitfStreobTag::~ossimNitfStreobTag()
{
}

void ossimNitfStreobTag::parseStream(std::istream& in)
{
   clearFields();

   in.read(m_stdId, 60);
   in.read(n_nMates, 1);
   in.read(m_mateInstance, 1);
   in.read(m_bConv, 5);
   in.read(m_eConv, 5);
   in.read(m_bAsym, 5);
   in.read(m_eAsym, 5);
   in.read(m_bBie, 6);
   in.read(m_eBie, 6);
}

void ossimNitfStreobTag::writeStream(std::ostream& out)
{
   out.write(m_stdId, 60);
   out.write(n_nMates, 1);
   out.write(m_mateInstance, 1);
   out.write(m_bConv, 5);
   out.write(m_eConv, 5);
   out.write(m_bAsym, 5);
   out.write(m_eAsym, 5);
   out.write(m_bBie, 6);
   out.write(m_eBie, 6);
}

void ossimNitfStreobTag::clearFields()
{
   // BCS-N's to '0's, BCS-A's to ' '(spaces)
   
   memset(m_stdId, '\0', 61);
   memset(n_nMates, '\0', 2);
   memset(m_mateInstance, '\0', 2);
   memset(m_bConv, '\0', 6);
   memset(m_eConv, '\0', 6);
   memset(m_bAsym, '\0', 6);
   memset(m_eAsym, '\0', 7);
   memset(m_bBie, '\0', 7);
}

std::ostream& ossimNitfStreobTag::print(
   std::ostream& out, const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";
   
   out << setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:"
       << getTagName() << "\n"
       << pfx << std::setw(24) << "ST_ID:"   << m_stdId << "\n"
       << pfx << std::setw(24) << "N_MATES:" << n_nMates << "\n"
       << pfx << std::setw(24) << "MATE_INSTANCE:" << m_mateInstance << "\n"
       << pfx << std::setw(24) << "B_CONV:" << m_bConv << "\n"
       << pfx << std::setw(24) << "E_CONV:" << m_eConv << "\n"
       << pfx << std::setw(24) << "B_ASYM:" << m_bAsym << "\n"
       << pfx << std::setw(24) << "E_ASYM:" << m_eAsym << "\n"
       << pfx << std::setw(24) << "B_BIE:" << m_bBie << "\n";
   
   return out;
}

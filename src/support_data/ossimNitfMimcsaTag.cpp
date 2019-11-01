
//---
//
// License:  MIT
//
// Author:  David Burken
//
// Description:
//
// Motion Imagery Collection Summary TRE(MIMCSA) class definition.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 11 for more info.
//
//---
// $Id

#include <cstring> /* for memcpy */
#include <sstream>
#include <iomanip>

#include <ossim/support_data/ossimNitfMimcsaTag.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimDms.h>
#include <ossim/base/ossimDpt.h>

static const ossimTrace traceDebug(ossimString("ossimNitfMimcsaTag:debug"));

RTTI_DEF1(ossimNitfMimcsaTag, "ossimNitfMimcsaTag", ossimNitfRegisteredTag);

ossimNitfMimcsaTag::ossimNitfMimcsaTag()
   : ossimNitfRegisteredTag(std::string("MIMCSA"), 121)
{
   clearFields();
}

void ossimNitfMimcsaTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(m_layerId, LAYER_ID_SIZE);
   in.read(m_nominalFrameRate, NOMINAL_FRAME_RATE_SIZE);
   in.read(m_minFrameRate, MIN_FRAME_RATE_SIZE);
   in.read(m_maxFrameRate, MAX_FRAME_RATE_SIZE);
   in.read(m_tRset, T_RSET_SIZE);
   in.read(m_miReqDecoder, MI_REQ_DECODER_SIZE);
   in.read(m_miReqProfile, MI_REQ_PROFILE_SIZE);
   in.read(m_miReqLevel, MI_REQ_LEVEL_SIZE);
}

void ossimNitfMimcsaTag::writeStream(std::ostream& out)
{
   out.write(m_layerId, LAYER_ID_SIZE);
   out.write(m_nominalFrameRate, NOMINAL_FRAME_RATE_SIZE);
   out.write(m_minFrameRate, MIN_FRAME_RATE_SIZE);
   out.write(m_maxFrameRate, MAX_FRAME_RATE_SIZE);
   out.write(m_tRset, T_RSET_SIZE);
   out.write(m_miReqDecoder, MI_REQ_DECODER_SIZE);
   out.write(m_miReqProfile, MI_REQ_PROFILE_SIZE);
   out.write(m_miReqLevel, MI_REQ_LEVEL_SIZE);
}

void ossimNitfMimcsaTag::clearFields()
{
   const char NAN_STRING[] = "NaN          "; // NaN + 10 spaces.
   std::memset(m_layerId, ' ', LAYER_ID_SIZE);
   std::memcpy(m_nominalFrameRate, NAN_STRING, NOMINAL_FRAME_RATE_SIZE);
   std::memcpy(m_minFrameRate,  NAN_STRING, MIN_FRAME_RATE_SIZE);
   std::memcpy(m_maxFrameRate,  NAN_STRING, MAX_FRAME_RATE_SIZE);
   std::memset(m_tRset, '0', T_RSET_SIZE);
   std::memset(m_miReqDecoder, ' ', MI_REQ_DECODER_SIZE);
   std::memset(m_miReqProfile, ' ', MI_REQ_PROFILE_SIZE);
   std::memset(m_miReqLevel, ' ', MI_REQ_LEVEL_SIZE);

   m_layerId[LAYER_ID_SIZE] = '\0';
   m_nominalFrameRate[NOMINAL_FRAME_RATE_SIZE] = '\0';
   m_minFrameRate[MIN_FRAME_RATE_SIZE] = '\0';
   m_maxFrameRate[MAX_FRAME_RATE_SIZE] = '\0';
   m_tRset[T_RSET_SIZE] = '\0';
   m_miReqDecoder[MI_REQ_DECODER_SIZE] = '\0';
   m_miReqProfile[MI_REQ_PROFILE_SIZE] = '\0';
   m_miReqLevel[MI_REQ_LEVEL_SIZE] = '\0';
}

std::ostream& ossimNitfMimcsaTag::print(std::ostream& out,
                                        const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";

   const ossim_int32 W = 24;
   
   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(W) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(W) << "CEL:" << getTagLength() << "\n"
       << pfx << std::setw(W) << "LAYER_ID:" << m_layerId << "\n"
       << pfx << std::setw(W) << "NOMINAL_FRAME_RATE:" << m_nominalFrameRate << "\n"
       << pfx << std::setw(W) << "MIN_FRAME_RATE:" << m_minFrameRate << "\n"
       << pfx << std::setw(W) << "MAX_FRAME_RATE:" << m_maxFrameRate << "\n"
       << pfx << std::setw(W) << "T_RSET:" << m_tRset << "\n"
       << pfx << std::setw(W) << "MI_REQ_DECODER:" << m_miReqDecoder << "\n"
       << pfx << std::setw(W) << "MI_REQ_PROFILE:" << m_miReqProfile << "\n"
       << pfx << std::setw(W) << "MI_REQ_LEVEL:" <<  m_miReqLevel<< "\n";

   return out;
}

ossimString ossimNitfMimcsaTag::getLayerId() const
{
   return ossimString(m_layerId);
}

ossimString ossimNitfMimcsaTag::getNominalFrameRate() const
{
   return ossimString(m_nominalFrameRate);
}

ossimString ossimNitfMimcsaTag::getMinFrameRate() const
{
   return ossimString(m_minFrameRate);
}

ossimString ossimNitfMimcsaTag::getMaxFrameRate() const
{
   return ossimString(m_maxFrameRate);
}

ossimString ossimNitfMimcsaTag::getTRset() const
{
   return ossimString(m_tRset);
}

ossimString ossimNitfMimcsaTag::getMiReqDecoder() const
{
   return ossimString(m_miReqDecoder);
}

ossimString ossimNitfMimcsaTag::getMiReqProfile() const
{
   return ossimString(m_miReqProfile);
}

ossimString ossimNitfMimcsaTag::getMiReqLevel() const
{
   return ossimString(m_miReqLevel);
}

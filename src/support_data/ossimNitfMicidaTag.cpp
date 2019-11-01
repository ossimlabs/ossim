//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//
// Motion Imagery Core Identification TRE(MICIDA) class definition.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 13 for more info.
//
//---
// $Id

#include <ossim/support_data/ossimNitfMicidaTag.h>
#include <ossim/base/ossimString.h>
#include <cstring>
#include <iomanip>
#include <iostream>

void ossimNitfMicidaCamera::parseStream(std::istream& in)
{
   clearFields();

   in.read(m_cameraId, CAMERAS_ID_SIZE);
   in.read(m_coreIdLength, CORE_ID_LENGTH_SIZE);

   ossim_uint32 size = ossimString(m_coreIdLength).toUInt32();
   if ( size )
   {
      m_cameraCoreId.resize( size );
      in.read(&m_cameraCoreId.front(), size);
   }
}

void ossimNitfMicidaCamera::writeStream(std::ostream& out) const
{
   out.write(m_cameraId, CAMERAS_ID_SIZE);
   out.write(m_coreIdLength, CORE_ID_LENGTH_SIZE);
   out.write(m_cameraCoreId.data(), m_cameraCoreId.size());
}

std::ostream& ossimNitfMicidaCamera::print( std::ostream& out,
                                            const std::string& prefix,
                                            ossim_uint32 index) const
{
   std::string pfx = prefix;
   pfx += "CAMERA";
   pfx += ossimString::toString(index).string();
   pfx += ".";
   
   const ossim_int32 W = 20;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(W) << "CAMERAS_ID:" << ossimString(m_cameraId).trim() << "\n"
       << pfx << std::setw(W) << "CORE_ID_LENGTH:" << m_coreIdLength << "\n"
       << pfx << std::setw(W) << "CAMERA_CORE_ID:" << m_cameraCoreId << "\n";

   return out;
}

ossim_uint32 ossimNitfMicidaCamera::getSizeInBytes() const
{
   return (ossim_uint32)
      (CAMERAS_ID_SIZE + CORE_ID_LENGTH_SIZE + m_cameraCoreId.size());
}

void ossimNitfMicidaCamera::clearFields()
{
   std::memset( m_cameraId, ' ', CAMERAS_ID_SIZE );
   std::memset( m_coreIdLength, '0', CORE_ID_LENGTH_SIZE );
   
   m_cameraId[CAMERAS_ID_SIZE] = '\0';
   m_coreIdLength[CORE_ID_LENGTH_SIZE] = '\0';

   m_cameraCoreId.clear();
}

// For ossimRtti stuff...
RTTI_DEF1(ossimNitfMicidaTag, "ossimNitfMicidaTag", ossimNitfRegisteredTag);

ossimNitfMicidaTag::ossimNitfMicidaTag()
   : ossimNitfRegisteredTag(std::string("MICIDA"), 0)
{
   clearFields();
}

void ossimNitfMicidaTag::parseStream(std::istream& in)
{
   clearFields();

   in.read(m_miisCoreIdVersion, MIIS_CORE_ID_VERSION_SIZE);
   in.read(m_numCameraIdsInTre, NUM_CAMERA_IDS_IN_TRE_SIZE);

   ossim_uint32 count = ossimString(m_numCameraIdsInTre).toUInt32();
   if ( count )
   {
      m_camera.resize( count );
      for ( auto&& i : m_camera )
      {
         i.parseStream( in );
      }
   }
}

void ossimNitfMicidaTag::writeStream(std::ostream& out)
{
   out.write(m_miisCoreIdVersion, MIIS_CORE_ID_VERSION_SIZE);
   out.write(m_numCameraIdsInTre, NUM_CAMERA_IDS_IN_TRE_SIZE);

   const auto& cv = m_camera;
   for (auto&& i : cv)
   {
      i.writeStream( out );
   }
}

void ossimNitfMicidaTag::clearFields()
{
   std::memset(m_miisCoreIdVersion, '0', MIIS_CORE_ID_VERSION_SIZE);
   std::memset(m_numCameraIdsInTre, '0', NUM_CAMERA_IDS_IN_TRE_SIZE);
   m_miisCoreIdVersion[MIIS_CORE_ID_VERSION_SIZE] = '\0';
   m_numCameraIdsInTre[NUM_CAMERA_IDS_IN_TRE_SIZE] = '\0';
   m_camera.clear();
}

std::ostream& ossimNitfMicidaTag::print( std::ostream& out,
                                         const std::string& prefix ) const
{
   std::string pfx = prefix;
   pfx += "MICIDA.";

   const ossim_int32 W = 28;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(W) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(W) << "CEL:" << getTagLength() << "\n"
       << pfx << std::setw(W) << "MIIS_CORE_ID_VERSION: " << m_miisCoreIdVersion << "\n"
       << pfx << std::setw(W) << "NUM_CAMERA_IDS_IN_TRE: " << m_numCameraIdsInTre << "\n";

   for (ossim_uint32 i = 0; i < (ossim_uint32)m_camera.size(); ++i )
   {
      m_camera[i].print(out, pfx, i);
   } 
   
   return out;
}

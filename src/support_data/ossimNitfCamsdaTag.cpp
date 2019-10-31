//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//
// Camera Set Definition TRE(CAMSDA) class definition.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 12 for more info.
//
//---
// $Id

#include <ossim/support_data/ossimNitfCamsdaTag.h>
#include <ossim/base/ossimString.h>
#include <cstring>
#include <iomanip>
#include <iostream>



void ossimNitfCamsdaCamera::parseStream(std::istream& in)
{
   clearFields();

   in.read(m_cameraId, CAMERA_ID_SIZE);
   in.read(m_cameraDesc, CAMERA_DESC_SIZE);
   in.read(m_layerId, LAYER_ID_SIZE);
   in.read(m_idLvl, IDLVL_SIZE);
   in.read(m_iaLvl, IALVL_SIZE);
   in.read(m_iloc, ILOC_SIZE);
   in.read(m_nRows, NROWS_SIZE);
   in.read(m_nCols, NCOLS_SIZE);
}

void ossimNitfCamsdaCamera::writeStream(std::ostream& out) const
{
   out.write(m_cameraId, CAMERA_ID_SIZE);
   out.write(m_cameraDesc, CAMERA_DESC_SIZE);
   out.write(m_layerId, LAYER_ID_SIZE);
   out.write(m_idLvl, IDLVL_SIZE);
   out.write(m_iaLvl, IALVL_SIZE);
   out.write(m_iloc, ILOC_SIZE);
   out.write(m_nRows, NROWS_SIZE);
   out.write(m_nCols, NCOLS_SIZE); 
}

std::ostream& ossimNitfCamsdaCamera::print( std::ostream& out,
                                            const std::string& prefix,
                                            ossim_uint32 index) const
{
   std::string pfx = prefix;
   pfx += "CAMERA";
   pfx += ossimString::toString(index).string();
   pfx += ".";

   const ossim_int32 W = 13;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(W) << "CAMERA_ID:" << ossimString(m_cameraId).trim() << "\n"
       << pfx << std::setw(W) << "CAMERA_DESC:" << ossimString(m_cameraDesc).trim() << "\n"
       << pfx << std::setw(W) << "LAYER_ID:" << ossimString(m_layerId).trim() << "\n"
       << pfx << std::setw(W) << "IDLVL:" << m_idLvl << "\n"
       << pfx << std::setw(W) << "IALVL:" << m_iaLvl << "\n"
       << pfx << std::setw(W) << "ILOC:" << m_iloc << "\n"
       << pfx << std::setw(W) << "NROWS:" << m_nRows << "\n"
       << pfx << std::setw(W) << "NCOLS:" << m_nCols << "\n";

   return out;
}

void ossimNitfCamsdaCamera::clearFields()
{
   std::memset( m_cameraId, ' ', CAMERA_ID_SIZE );
   std::memset( m_cameraDesc, ' ', CAMERA_DESC_SIZE );
   std::memset( m_layerId, ' ', LAYER_ID_SIZE );
   std::memset( m_idLvl, '1', IDLVL_SIZE ); // ?
   std::memset( m_iaLvl, '0',  IALVL_SIZE );
   std::memset( m_iloc, '0', ILOC_SIZE );
   std::memset( m_nRows, '0', NROWS_SIZE );
   std::memset( m_nCols, '0', NCOLS_SIZE );

   m_cameraId[CAMERA_ID_SIZE] = '\0';
   m_cameraDesc[CAMERA_DESC_SIZE] = '\0';
   m_layerId[LAYER_ID_SIZE] = '\0';
   m_idLvl[IDLVL_SIZE] = '\0';
   m_iaLvl[IALVL_SIZE] = '\0';
   m_iloc[ILOC_SIZE] = '\0';
   m_nRows[NROWS_SIZE] = '\0';
   m_nCols[NCOLS_SIZE] = '\0';
}

ossimNitfCamsdaCameraSet::ossimNitfCamsdaCameraSet()
   : m_camera(0)
{
}

void ossimNitfCamsdaCameraSet::parseStream(std::istream& in)
{
   clearFields();

   in.read(m_numCamerasInSet, NUM_CAMERAS_IN_SET_SIZE);

   ossim_uint32 count = ossimString(m_numCamerasInSet).toUInt32();
   if ( count )
   {
      m_camera.resize( count );
      for ( auto&& i : m_camera )
      {
         i.parseStream( in );
      }
   }
}

void ossimNitfCamsdaCameraSet::writeStream(std::ostream& out) const
{
   out.write(m_numCamerasInSet, NUM_CAMERAS_IN_SET_SIZE);

   const auto& cv = m_camera;
   for (auto&& i : cv)
   {
      i.writeStream( out );
   }
}

std::ostream& ossimNitfCamsdaCameraSet::print( std::ostream& out,
                                               const std::string& prefix,
                                               ossim_uint32 index ) const
{
   std::string pfx = prefix;
   pfx += "CAMERA_SET";
   pfx += ossimString::toString(index).string();
   pfx += ".";

   const ossim_int32 W = 21;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(W) << "NUM_CAMERAS_IN_SET:" << m_numCamerasInSet << "\n";

   ossim_uint32 count = ossimString(m_numCamerasInSet).toUInt32();
   if ( count && (count == m_camera.size() ) )
   {
      for ( ossim_uint32 i = 0; i < count; ++i )
      {
         m_camera[i].print( out, pfx, i );
      }
   }
   return out;
}

ossim_uint32 ossimNitfCamsdaCameraSet::getSizeInBytes() const
{
   return (ossim_uint32)
      (NUM_CAMERAS_IN_SET_SIZE + (m_camera.size() * ossimNitfCamsdaCamera::RECORD_SIZE) );
}

void ossimNitfCamsdaCameraSet::clearFields()
{
   std::memset(m_numCamerasInSet, '0', NUM_CAMERAS_IN_SET_SIZE);
   m_numCamerasInSet[NUM_CAMERAS_IN_SET_SIZE] = '\0';
   m_camera.clear();
}

RTTI_DEF1(ossimNitfCamsdaTag, "ossimNitfCamsdaTag", ossimNitfRegisteredTag);

ossimNitfCamsdaTag::ossimNitfCamsdaTag()
   : ossimNitfRegisteredTag(std::string("CAMSDA"), 0)
{
   clearFields();
}

void ossimNitfCamsdaTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(m_numCameraSets, NUM_CAMERA_SETS_SIZE);
   in.read(m_numCameraSetsInTre, NUM_CAMERA_SETS_IN_TRE_SIZE);
   in.read(m_firstCameraSetInTre, FIRST_CAMERA_SETS_IN_TRE_SIZE);

   ossim_uint32 count = ossimString(m_numCameraSetsInTre).toUInt32();
   if ( count )
   {
      m_cameraSet.resize( count );
      for ( auto&& i : m_cameraSet )
      {
         i.parseStream( in );
      }
   }
}

void ossimNitfCamsdaTag::writeStream(std::ostream& out)
{
   out.write(m_numCameraSets, NUM_CAMERA_SETS_SIZE);
   out.write(m_numCameraSetsInTre, NUM_CAMERA_SETS_IN_TRE_SIZE);
   out.write(m_firstCameraSetInTre, FIRST_CAMERA_SETS_IN_TRE_SIZE);

   const auto& cv = m_cameraSet;
   for (auto&& i : cv)
   {
      i.writeStream( out );
   }
}

ossimString ossimNitfCamsdaTag::getNumberOfCameraSets() const
{
   return ossimString(m_numCameraSets);
}

ossimString ossimNitfCamsdaTag::getNumberOfCameraSetsInTre() const
{
   return ossimString(m_numCameraSetsInTre);
}

ossimString ossimNitfCamsdaTag::getFirstCameraSetInTre() const
{
   return ossimString(m_firstCameraSetInTre);
}

void ossimNitfCamsdaTag::clearFields()
{
   std::memset(m_numCameraSets, '0', NUM_CAMERA_SETS_SIZE);
   std::memset(m_numCameraSetsInTre, '0', NUM_CAMERA_SETS_IN_TRE_SIZE);
   std::memset(m_firstCameraSetInTre, '0', FIRST_CAMERA_SETS_IN_TRE_SIZE);
   
   m_numCameraSets[NUM_CAMERA_SETS_SIZE] = '\0';
   m_numCameraSetsInTre[NUM_CAMERA_SETS_IN_TRE_SIZE] = '\0';
   m_firstCameraSetInTre[FIRST_CAMERA_SETS_IN_TRE_SIZE] = '\0';

   m_cameraSet.clear();
}

std::ostream& ossimNitfCamsdaTag::print( std::ostream& out,
                                         const std::string& prefix ) const
{
   std::string pfx = prefix;
   pfx += "CAMSDA.";

   const ossim_int32 W = 33;

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(W) << "CETAG:" << getTagName() << "\n"
       << pfx << std::setw(W) << "CEL:" << getTagLength() << "\n"
       << pfx << std::setw(W) << "NUM_CAMERA_SETS:" << m_numCameraSets << "\n"
       << pfx << std::setw(W) << "NUM_CAMERA_SETS_IN_TRE: " << m_numCameraSetsInTre << "\n"
       << pfx << std::setw(W) << "FIRST_CAMERA_SETS_IN_TRE: " << m_firstCameraSetInTre << "\n";

   ossim_uint32 count = ossimString(m_numCameraSetsInTre).toUInt32();
   if ( count && (count == (ossim_uint32)m_cameraSet.size()) )
   {
      ossim_uint32 cameraSetIndex = ossimString(m_numCameraSetsInTre).toUInt32();
      for ( ossim_uint32 i = 0; i < count; ++i )
      {
         m_cameraSet[i].print(out, pfx, cameraSetIndex++);
      }
   }
   
   return out;
}

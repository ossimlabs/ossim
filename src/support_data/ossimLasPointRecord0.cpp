//----------------------------------------------------------------------------
//
// File: ossimLasPointRecord0.cpp
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Container class LAS point record type 1.
//
//----------------------------------------------------------------------------
// $Id$

#include <ossim/support_data/ossimLasPointRecord0.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>
#include <iomanip>
#include <iostream>
#include <istream>
#include <ostream>

ossimLasPointRecord0::ossimLasPointRecord0Data::ossimLasPointRecord0Data()
   :
   m_x(0),
   m_y(0),
   m_z(0),
   m_intensity(0),
   m_returnByte(0),
   m_classification(0),
   m_scanAngleRank(0),
   m_userData(0),
   m_pointSourceID(0)
{
}

ossimLasPointRecord0::ossimLasPointRecord0Data::ossimLasPointRecord0Data(
   const ossimLasPointRecord0::ossimLasPointRecord0Data& obj)
   :
   m_x(obj.m_x),
   m_y(obj.m_y),
   m_z(obj.m_z),
   m_intensity(obj.m_intensity),
   m_returnByte(obj.m_returnByte),
   m_classification(obj.m_classification),
   m_scanAngleRank(obj.m_scanAngleRank),
   m_userData(obj.m_userData),
   m_pointSourceID(obj.m_pointSourceID)
{
}

const ossimLasPointRecord0::ossimLasPointRecord0Data&
ossimLasPointRecord0::ossimLasPointRecord0Data::operator=(
   const ossimLasPointRecord0::ossimLasPointRecord0Data& obj)
{
   if ( this != &obj )
   {
      m_x = obj.m_x;
      m_y = obj.m_y;
      m_z = obj.m_z;
      m_intensity      = obj.m_intensity;
      m_returnByte     = obj.m_returnByte;
      m_classification = obj.m_classification;
      m_scanAngleRank  = obj.m_scanAngleRank;
      m_userData       = obj.m_userData;
      m_pointSourceID  = obj.m_pointSourceID;
   }
   return *this;
}

ossimLasPointRecord0::ossimLasPointRecord0()
   :
   m_record()
{
}

ossimLasPointRecord0::ossimLasPointRecord0(const ossimLasPointRecord0& obj)
   :
   m_record(obj.m_record)
{
}

const ossimLasPointRecord0& ossimLasPointRecord0::operator=(const ossimLasPointRecord0& copy_this)
{
   if (this != &copy_this)
   {
      m_record = copy_this.m_record;
   }
   return *this;
}

ossimLasPointRecord0::~ossimLasPointRecord0()
{
}

void ossimLasPointRecord0::readStream(std::istream& in)
{
   in.read((char*)&m_record, 28);

   if ( ossim::byteOrder() == OSSIM_BIG_ENDIAN )
   {
      swap();
   }
}

void ossimLasPointRecord0::writeStream(std::ostream& out)
{
   if ( ossim::byteOrder() == OSSIM_BIG_ENDIAN )
   {
      // Write little endian per spec:
      swap();
   }

   out.write((char*)&m_record, 28);

   if ( ossim::byteOrder() == OSSIM_BIG_ENDIAN )
   {
      // Swap back to native byte order if needed:
      swap();
   }
}

ossim_int32 ossimLasPointRecord0::getX() const
{
   return m_record.m_x;
}

ossim_int32 ossimLasPointRecord0::getY() const
{
   return m_record.m_y;
}

ossim_int32 ossimLasPointRecord0::getZ() const
{
   return m_record.m_z;
}

ossim_uint16 ossimLasPointRecord0::getRed() const
{
   return 0;
}

ossim_uint16 ossimLasPointRecord0::getGreen() const
{
   return 0;
}

ossim_uint16 ossimLasPointRecord0::getBlue() const
{
   return 0;
}

ossim_uint16 ossimLasPointRecord0::getIntensity() const
{
   return m_record.m_intensity;
}

ossim_uint8 ossimLasPointRecord0::getReturnNumber() const
{
   return (0x07 & m_record.m_returnByte);
}

ossim_uint8 ossimLasPointRecord0::getNumberOfReturns() const
{
   return (0x34 & m_record.m_returnByte);
}

ossim_uint8 ossimLasPointRecord0::getEdgeFlag() const
{
   return (0x40 & m_record.m_returnByte);
}

std::ostream& ossimLasPointRecord0::print(std::ostream& out) const
{
   // Capture the original flags.
   std::ios_base::fmtflags f = out.flags();

   out << std::setiosflags(std::ios_base::fixed) << std::setprecision(8);

   out << "x:                 " << m_record.m_x
       << "\ny:                 " << m_record.m_y
       << "\nz:                 " << m_record.m_z
       << "\nintensity:         " << m_record.m_intensity
       << "\nreturn:            " << int(getReturnNumber())
       << "\nnumber_of_returns: " << int(getNumberOfReturns())
       << "\nedge:              " << (getEdgeFlag()?"true":"false")
       << "\nclassification:    " << m_record.m_classification
       << "\nscan_angle_rank:   " << int(m_record.m_scanAngleRank)
       << "\nuser_data:         " << m_record.m_userData
       << "\npoint_source_id:   " << m_record.m_pointSourceID
       << std::endl;

   // Reset flags.
   out.setf(f);
   return out;
}

void ossimLasPointRecord0::swap()
{
   ossimEndian endian;
   endian.swap(m_record.m_x);
   endian.swap(m_record.m_y);
   endian.swap(m_record.m_z);
   endian.swap(m_record.m_intensity);
   endian.swap(m_record.m_pointSourceID);
}


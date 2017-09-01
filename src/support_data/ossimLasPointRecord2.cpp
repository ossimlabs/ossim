//----------------------------------------------------------------------------
//
// File: ossimLasPointRecord2.cpp
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description: Container class LAS point record type 2.
//
//----------------------------------------------------------------------------
// $Id$

#include <ossim/support_data/ossimLasPointRecord2.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>
#include <iomanip>
#include <iostream>
#include <istream>
#include <ostream>

ossimLasPointRecord2::ossimLasPointRecord2Data::ossimLasPointRecord2Data()
   :
   m_x(0),
   m_y(0),
   m_z(0),
   m_intensity(0),
   m_bitFlags(0),
   m_classification(0),
   m_scanAngleRank(0),
   m_userData(0),
   m_pointSourceId(0),
   m_red(0),
   m_green(0),
   m_blue(0)
{
}

ossimLasPointRecord2::ossimLasPointRecord2Data::ossimLasPointRecord2Data(
   const ossimLasPointRecord2::ossimLasPointRecord2Data& obj)
   :
   m_x(obj.m_x),
   m_y(obj.m_y),
   m_z(obj.m_z),
   m_intensity(obj.m_intensity),
   m_bitFlags(obj.m_bitFlags),
   m_classification(obj.m_classification),
   m_scanAngleRank(obj.m_scanAngleRank),
   m_userData(obj.m_userData),
   m_pointSourceId(obj.m_pointSourceId),
   m_red(obj.m_red),
   m_green(obj.m_green),
   m_blue(obj.m_blue)   
{
}

const ossimLasPointRecord2::ossimLasPointRecord2Data&
ossimLasPointRecord2::ossimLasPointRecord2Data::operator=(
   const ossimLasPointRecord2::ossimLasPointRecord2Data& obj)
{
   if ( this != &obj )
   {
      m_x              = obj.m_x;
      m_y              = obj.m_y;
      m_z              = obj.m_z;
      m_intensity      = obj.m_intensity;
      m_bitFlags       = obj.m_bitFlags;
      m_classification = obj.m_classification;
      m_scanAngleRank  = obj.m_scanAngleRank;
      m_userData       = obj.m_userData;
      m_pointSourceId  = obj.m_pointSourceId;
      m_red            = obj.m_red;
      m_green          = obj.m_green;
      m_blue           = obj.m_blue;
   }
   return *this;
}

ossimLasPointRecord2::ossimLasPointRecord2()
   :
   m_record()
{
}

ossimLasPointRecord2::ossimLasPointRecord2(const ossimLasPointRecord2& obj)
   :
   m_record(obj.m_record)
{
}

const ossimLasPointRecord2& ossimLasPointRecord2::operator=(const ossimLasPointRecord2& copy_this)
{
   if (this != &copy_this)
   {
      m_record = copy_this.m_record;
   }
   return *this;
}

ossimLasPointRecord2::~ossimLasPointRecord2()
{
}

void ossimLasPointRecord2::readStream(std::istream& in)
{
   in.read((char*)&m_record, 26); // sizeof reports 28 due to actual space used for STRUCT //sizeof(ossimLasPointRecord2Data));

   if ( ossim::byteOrder() == OSSIM_BIG_ENDIAN )
   {
      swap();
   }
}

void ossimLasPointRecord2::writeStream(std::ostream& out)
{
   if ( ossim::byteOrder() == OSSIM_BIG_ENDIAN )
   {
      // Write little endian per spec:
      swap();
   }

   out.write((char*)&m_record, 26); // sizeof reports 28 due to actual space used for STRUCT //sizeof(ossimLasPointRecord2Data));

   if ( ossim::byteOrder() == OSSIM_BIG_ENDIAN )
   {
      // Swap back to native byte order if needed:
      swap();
   }
}

ossim_int32 ossimLasPointRecord2::getX() const
{
   return m_record.m_x;
}

ossim_int32 ossimLasPointRecord2::getY() const
{
   return m_record.m_y;
}

ossim_int32 ossimLasPointRecord2::getZ() const
{
   return m_record.m_z;
}

ossim_uint16 ossimLasPointRecord2::getRed() const
{  
   return m_record.m_red;
}

ossim_uint16 ossimLasPointRecord2::getGreen() const
{
   return m_record.m_green;
}

ossim_uint16 ossimLasPointRecord2::getBlue() const
{
   return m_record.m_blue;
}

ossim_uint16 ossimLasPointRecord2::getIntensity() const
{
   return m_record.m_intensity;
}

ossim_uint8 ossimLasPointRecord2::getReturnNumber() const
{
   return (m_record.m_bitFlags.m_returnNumber);
}

ossim_uint8 ossimLasPointRecord2::getNumberOfReturns() const
{
   return (m_record.m_bitFlags.m_numberOfReturns);
}

ossim_uint8 ossimLasPointRecord2::getEdgeFlag() const
{
   return (m_record.m_bitFlags.m_edgeOfFlightLine);
}

std::ostream& ossimLasPointRecord2::print(std::ostream& out) const
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
       << "\nscan_direction:    " << int(m_record.m_bitFlags.m_scanDirection)
       << "\nclassification:    " << m_record.m_classification
       << "\nscan_angle_rank:   " << int(m_record.m_scanAngleRank)
       << "\nuser_data:         " << m_record.m_userData
       << "\npoint_source_id:   " << m_record.m_pointSourceId
       << "\nred:               " << m_record.m_red
       << "\ngreen:             " << m_record.m_green
       << "\nblue:              " << m_record.m_blue
       << std::endl;

   // Reset flags.
   out.setf(f);
   return out;
}

void ossimLasPointRecord2::swap()
{
   ossimEndian endian;
   endian.swap(m_record.m_x);
   endian.swap(m_record.m_y);
   endian.swap(m_record.m_z);
   endian.swap(m_record.m_intensity);
   endian.swap(m_record.m_pointSourceId);
   endian.swap(m_record.m_red);
   endian.swap(m_record.m_green);
   endian.swap(m_record.m_blue);   
}


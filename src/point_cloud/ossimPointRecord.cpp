//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id: ossimPointRecord.cpp 23352 2015-05-29 17:38:12Z okramer $

#include <ossim/point_cloud/ossimPointRecord.h>

using namespace std;

ossimPointRecord::ossimPointRecord(ossim_uint32 field_code)
      : m_pointId(0)
{
   if (field_code & Intensity)
      m_fieldMap[Intensity] = ossim::nan();
   if (field_code & ReturnNumber)
      m_fieldMap[ReturnNumber] = ossim::nan();
   if (field_code & NumberOfReturns)
      m_fieldMap[NumberOfReturns] = ossim::nan();
   if (field_code & Red)
      m_fieldMap[Red] = ossim::nan();
   if (field_code & Green)
      m_fieldMap[Green] = ossim::nan();
   if (field_code & Blue)
      m_fieldMap[Blue] = ossim::nan();
   if (field_code & GpsTime)
      m_fieldMap[GpsTime] = ossim::nan();
   if (field_code & Infrared)
      m_fieldMap[Infrared] = ossim::nan();
}

ossimPointRecord::ossimPointRecord(const ossimPointRecord& pcr)
      : m_pointId(pcr.m_pointId),
        m_position(pcr.m_position),
        m_fieldMap (pcr.m_fieldMap)
{
}

ossimPointRecord::ossimPointRecord(const ossimGpt& pos)
      : m_pointId(0),
        m_position(pos)
{
}

ossimPointRecord::~ossimPointRecord()
{
}

inline bool ossimPointRecord::hasFields(ossim_uint32 field_code) const
{
   bool found = true;

   if (field_code & Intensity)
      found &= m_fieldMap.find(Intensity) != m_fieldMap.end();
   if (found && (field_code & ReturnNumber))
         found = m_fieldMap.find(ReturnNumber) != m_fieldMap.end();
   if (found && (field_code & NumberOfReturns))
         found = m_fieldMap.find(NumberOfReturns) != m_fieldMap.end();
   if (found && (field_code & Red))
         found = m_fieldMap.find(Red) != m_fieldMap.end();
   if (found && (field_code & Green))
         found = m_fieldMap.find(Green) != m_fieldMap.end();
   if (found && (field_code & Blue))
         found = m_fieldMap.find(Blue) != m_fieldMap.end();
   if (found && (field_code & GpsTime))
         found = m_fieldMap.find(GpsTime) != m_fieldMap.end();
   if (found && (field_code & Infrared))
         found = m_fieldMap.find(Infrared) != m_fieldMap.end();

   return found;
}

inline ossim_uint32 ossimPointRecord::getFieldCode() const
{
   ossim_uint32 field_code = 0;

   if (m_fieldMap.find(Intensity) != m_fieldMap.end())
      field_code |= Intensity;
   if (m_fieldMap.find(ReturnNumber) != m_fieldMap.end())
      field_code |= ReturnNumber;
   if (m_fieldMap.find(NumberOfReturns) != m_fieldMap.end())
      field_code |= NumberOfReturns;
   if (m_fieldMap.find(Red) != m_fieldMap.end())
      field_code |= Red;
   if (m_fieldMap.find(Green) != m_fieldMap.end())
      field_code |= Green;
   if (m_fieldMap.find(Blue) != m_fieldMap.end())
      field_code |= Blue;
   if (m_fieldMap.find(GpsTime) != m_fieldMap.end())
      field_code |= GpsTime;
   if (m_fieldMap.find(Infrared) != m_fieldMap.end())
      field_code |= Infrared;

   return field_code;
}

ossim_float32 ossimPointRecord::getField(FIELD_CODES fc) const
{
   std::map<FIELD_CODES, ossim_float32>::const_iterator v = m_fieldMap.find(fc);
   if (v == m_fieldMap.end())
      return ossim::nan();
   return v->second;
}

void  ossimPointRecord::setField(FIELD_CODES fc, ossim_float32 value)
{
   m_fieldMap[fc] = value;
}


ossimPointRecord& ossimPointRecord::operator=(const ossimPointRecord& pcr)
{
   if (this == &pcr)
      return *this;

   m_pointId = pcr.m_pointId;
   m_position = pcr.m_position;
   m_fieldMap = pcr.m_fieldMap;

   return *this;
}

std::ostream& operator << (std::ostream& out, const ossimPointRecord& p)
{
   out   << "ossimPointCloudRecord for pointId: " << p.m_pointId
         << "\n   Position: " << p.m_position << ends;

   std::map<ossimPointRecord::FIELD_CODES, ossim_float32>::const_iterator iter = p.m_fieldMap.begin();
   while (iter != p.m_fieldMap.end())
   {
      switch (iter->first)
      {
      case ossimPointRecord::Intensity:
         out << "\n   Intensity: ";
         break;
      case ossimPointRecord::ReturnNumber:
         out << "\n   ReturnNumber: ";
         break;
      case ossimPointRecord::NumberOfReturns:
         out << "\n   NumberOfReturns: ";
         break;
      case ossimPointRecord::Red:
         out << "\n   Red: ";
         break;
      case ossimPointRecord::Green:
         out << "\n   Green: ";
         break;
      case ossimPointRecord::Blue:
         out << "\n   Blue: ";
         break;
      case ossimPointRecord::GpsTime:
         out << "\n   GpsTime: ";
         break;
      case ossimPointRecord::Infrared:
         out << "\n   Infrared: ";
         break;
      default:
         out << "\n   Unidentified: ";
      }
      out << iter->second;
      ++iter;
   }
   return out;
}

void ossimPointRecord::clear()
{
   m_pointId = 0;
}

bool ossimPointRecord::isValid() const
{
   if ((m_pointId == 0) || m_fieldMap.empty() || m_position.hasNans())
      return false;
   return true;
}



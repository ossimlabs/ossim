//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
#include <ossim/point_cloud/ossimPointBlock.h>

using namespace std;

RTTI_DEF1(ossimPointBlock, "ossimPointBlock", ossimDataObject)

ossimPointBlock::ossimPointBlock(ossimSource* owner, ossim_uint32 fields)
:  ossimDataObject(owner),
   m_nullPCR(fields),
   m_minMaxValid(false),
   m_fieldCode(0),
   m_isNormalized(false)
{
}

ossimPointBlock::~ossimPointBlock()
{

}

void ossimPointBlock::getFieldMin(ossimPointRecord::FIELD_CODES field, ossim_float32& value) const
{
   if (empty())
      value = ossim::nan();
   else if (!m_minMaxValid)
      scanForMinMax();

   value = m_minRecord.getField(field);
}

void ossimPointBlock::getFieldMax(ossimPointRecord::FIELD_CODES field, ossim_float32& value) const
{
   if (empty())
      value = ossim::nan();
   else if (!m_minMaxValid)
      scanForMinMax();

   value = m_maxRecord.getField(field);
}


void ossimPointBlock::getBounds(ossimGrect& block_bounds) const
{
   if (!m_minMaxValid)
      scanForMinMax();

   block_bounds = ossimGrect(m_minRecord.getPosition(), m_maxRecord.getPosition());
}

const ossimPointRecord* ossimPointBlock::getPoint(ossim_uint32 point_offset) const
{
   if (point_offset < m_pointList.size())
      return m_pointList[point_offset].get();
   return 0;
}

ossimPointRecord* ossimPointBlock::getPoint(ossim_uint32 point_offset)
{
   if (point_offset < m_pointList.size())
      return m_pointList[point_offset].get();
   return 0;
}

const ossimPointBlock& ossimPointBlock::operator=(const ossimPointBlock& block )
{
   ossim_uint32 numPoints = block.size();
   if (numPoints == 0)
      return *this;

   for (ossim_uint32 i=0; i<numPoints; ++i)
      m_pointList.push_back(new ossimPointRecord(*(block[i])));

   m_nullPCR = block.m_nullPCR;
   m_minRecord = block.m_minRecord;
   m_maxRecord = block.m_maxRecord;
   m_minMaxValid = block.m_minMaxValid;
   m_fieldCode = block.m_fieldCode;
   m_isNormalized = block.m_isNormalized;

   return *this;
}

ossimObject* ossimPointBlock::dup() const
{
   ossimPointBlock* copy = new ossimPointBlock(0, m_fieldCode);
   *copy = *this;
   return copy;
}

ossim_uint32 ossimPointBlock::getFieldCode() const
{
   if (!m_pointList.empty())
      m_fieldCode = m_pointList[0]->getFieldCode();

   return m_fieldCode;
}

vector<ossimPointRecord::FIELD_CODES> ossimPointBlock::getFieldCodesAsList() const
{
   vector<ossimPointRecord::FIELD_CODES> code_list;
   if (m_fieldCode & ossimPointRecord::Intensity)
      code_list.push_back(ossimPointRecord::Intensity);
   if (m_fieldCode & ossimPointRecord::ReturnNumber)
      code_list.push_back(ossimPointRecord::ReturnNumber);
   if (m_fieldCode & ossimPointRecord::NumberOfReturns)
      code_list.push_back(ossimPointRecord::NumberOfReturns);
   if (m_fieldCode & ossimPointRecord::Red)
      code_list.push_back(ossimPointRecord::Red);
   if (m_fieldCode & ossimPointRecord::Green)
      code_list.push_back(ossimPointRecord::Green);
   if (m_fieldCode & ossimPointRecord::Blue)
      code_list.push_back(ossimPointRecord::Blue);
   if (m_fieldCode & ossimPointRecord::GpsTime)
      code_list.push_back(ossimPointRecord::GpsTime);
   if (m_fieldCode & ossimPointRecord::Infrared)
      code_list.push_back(ossimPointRecord::Infrared);
  return code_list;
}

void ossimPointBlock::setFieldCode(ossim_uint32 code)
{
   if (getFieldCode() != code)
      clear();

   m_fieldCode = code;
}

void ossimPointBlock::addPoint(ossimPointRecord* opr)
{
   // First check that the fields match the expected. If not, sync up to this point:
   if ((opr->getFieldCode() & m_fieldCode) != m_fieldCode)
      m_fieldCode = opr->getFieldCode();

   m_pointList.push_back(ossimRefPtr<ossimPointRecord>(opr));
   m_minMaxValid = false;
}

void ossimPointBlock::scanForMinMax() const
{
   ossim_uint32 numPoints = size();
   if (numPoints == 0)
      return;

   // Latch first point:
   m_minRecord = *(m_pointList[0].get());
   m_maxRecord = m_minRecord;
   ossimGpt minPos (m_minRecord.getPosition());
   ossimGpt maxPos (minPos);

   // For shorthand later:
   const ossimPointRecord::FIELD_CODES I = ossimPointRecord::Intensity;
   const ossimPointRecord::FIELD_CODES R = ossimPointRecord::Red;
   const ossimPointRecord::FIELD_CODES G = ossimPointRecord::Green;
   const ossimPointRecord::FIELD_CODES B = ossimPointRecord::Blue;
   const ossimPointRecord::FIELD_CODES S = ossimPointRecord::ReturnNumber;
   const ossimPointRecord::FIELD_CODES N = ossimPointRecord::NumberOfReturns;

   // If color available, latch the min for all bands as one to minimize color distortion:
   bool hasRGB = false;
   if (m_minRecord.hasFields(R|G|B))
   {
      hasRGB = true;
      ossim_float32 r = m_pointList[0]->getField(R);
      ossim_float32 g = m_pointList[0]->getField(G);
      ossim_float32 b = m_pointList[0]->getField(B);
      float minC = std::min(r, std::min(g, b));
      float maxC = std::max(r, std::max(g, b));
      m_minRecord.setField(R,   minC);
      m_minRecord.setField(G, minC);
      m_minRecord.setField(B,  minC);
      m_maxRecord.setField(R,   maxC);
      m_maxRecord.setField(G, maxC);
      m_maxRecord.setField(B,  maxC);
   }

   // Now loop over all points in file to latch min/max:
   for (ossim_uint32 i=0; i<numPoints; ++i)
   {
      if (m_pointList[i]->getPosition().lat < minPos.lat)
         minPos.lat = m_pointList[i]->getPosition().lat;
      if (m_pointList[i]->getPosition().lon < minPos.lon)
         minPos.lon = m_pointList[i]->getPosition().lon;
      if (m_pointList[i]->getPosition().hgt < minPos.hgt)
         minPos.hgt = m_pointList[i]->getPosition().hgt;

      if (m_pointList[i]->getPosition().lat > maxPos.lat)
         maxPos.lat = m_pointList[i]->getPosition().lat;
      if (m_pointList[i]->getPosition().lon > maxPos.lon)
         maxPos.lon = m_pointList[i]->getPosition().lon;
      if (m_pointList[i]->getPosition().hgt > maxPos.hgt)
         maxPos.hgt = m_pointList[i]->getPosition().hgt;

      if (m_minRecord.hasFields(I) &&  (m_minRecord.getField(I) > m_pointList[i]->getField(I)))
         m_minRecord.setField(I, m_pointList[i]->getField(I));

      if (m_minRecord.hasFields(S) &&  (m_minRecord.getField(S) > m_pointList[i]->getField(S)))
         m_minRecord.setField(S, m_pointList[i]->getField(S));

      if (m_minRecord.hasFields(N) &&  (m_minRecord.getField(N) > m_pointList[i]->getField(N)))
         m_minRecord.setField(N, m_pointList[i]->getField(N));

      if (hasRGB)
      {
         ossim_float32 r = m_pointList[i]->getField(R);
         ossim_float32 g = m_pointList[i]->getField(G);
         ossim_float32 b = m_pointList[i]->getField(B);

         float minC = std::min(r, std::min(g, b));
         if (m_minRecord.getField(R) > minC)
         {
            m_minRecord.setField(R, minC);
            m_minRecord.setField(G, minC);
            m_minRecord.setField(B, minC);
         }

         float maxC = std::max(r, std::max(g, b));
         if (m_maxRecord.getField(R) < maxC)
         {
            m_maxRecord.setField(R, maxC);
            m_maxRecord.setField(G, maxC);
            m_maxRecord.setField(B, maxC);
         }
      }
   } // end loop over all points

   m_minRecord.setPosition(minPos);
   m_maxRecord.setPosition(maxPos);
   m_minMaxValid = true;
}





//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id$

#include <ossim/point_cloud/ossimPointCloudHandler.h>

RTTI_DEF1(ossimPointCloudHandler, "ossimPointCloudHandler" , ossimPointCloudSource);

ossim_uint32 ossimPointCloudHandler::DEFAULT_BLOCK_SIZE = 0x400000;

ossimPointCloudHandler::ossimPointCloudHandler()
:  m_currentPID(0)
{
}

ossimPointCloudHandler::ossimPointCloudHandler(ossimObject* owner)
:  ossimPointCloudSource(owner),
   m_currentPID(0)
{

}

ossimPointCloudHandler::~ossimPointCloudHandler()
{
}

void ossimPointCloudHandler::getNextFileBlock(ossimPointBlock& block, ossim_uint32 maxPts) const
{
   getFileBlock(m_currentPID, block, maxPts);
   return;
}

void ossimPointCloudHandler::getBlock(const ossimGrect& bounds, ossimPointBlock& block) const
{
   block.clear();

   // This default implementation simply reads the whole datafile in file-blocks, retaining
   // only those points inside the bounds:
   ossimPointBlock file_block;
   rewind();
   ossimGpt gpt;

   do
   {
      file_block.clear();
      getNextFileBlock(file_block, DEFAULT_BLOCK_SIZE);
      ossimPointBlock::PointList& pointList = file_block.getPoints();
      ossimPointBlock::PointList::iterator iter = pointList.begin();
      while (iter != pointList.end())
      {
         gpt = (*iter)->getPosition();
         if (bounds.pointWithin(gpt))
         {
            block.addPoint(iter->get());
         }
         ++iter;
      }
   } while (file_block.size() == DEFAULT_BLOCK_SIZE);
}

void ossimPointCloudHandler::getBounds(ossimGrect& bounds) const
{
   if (m_minRecord.valid() && m_maxRecord.valid())
      bounds = ossimGrect(m_minRecord->getPosition(), m_maxRecord->getPosition());
   else
      bounds.makeNan();
}

void ossimPointCloudHandler::normalizeBlock(ossimPointBlock& block)
{
   if (!m_minRecord.valid() || !m_maxRecord.valid())
      return;

   ossim_uint32 numPoints = block.size();
   float val, norm, min, max;
   vector<ossimPointRecord::FIELD_CODES> field_codes = block.getFieldCodesAsList();
   vector<ossimPointRecord::FIELD_CODES>::const_iterator iter;
   ossimPointRecord::FIELD_CODES field_code;
   for (ossim_uint32 i=0; i<numPoints; ++i)
   {
      ossimPointRecord* opr = block[i];
      iter = field_codes.begin();
      while (iter != field_codes.end())
      {
         field_code = *iter;
         min = m_minRecord->getField(field_code);
         max = m_maxRecord->getField(field_code);
         val = opr->getField(field_code);
         norm = (val - min) / (max - min);
         opr->setField(field_code, norm);
         ++iter;
      }
   }
}

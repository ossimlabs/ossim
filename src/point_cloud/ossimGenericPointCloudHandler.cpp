#include <ossim/point_cloud/ossimGenericPointCloudHandler.h>

ossimGenericPointCloudHandler::ossimGenericPointCloudHandler(vector<ossimEcefPoint>& ecef_points)
{
   // Fill the point storage in any order.
   // Loop to add your points (assume your points are passed in a vector ecef_points[])
   for (ossim_uint32 i=0; i<ecef_points.size(); ++i)
   {
      ossimPointRecord* point = new ossimPointRecord(ossimGpt(ecef_points[i]));
      m_pointBlock.addPoint(point);
   }
   ossimGrect bounds;
   m_pointBlock.getBounds(bounds);
   m_minRecord = new ossimPointRecord(bounds.ll());
   m_maxRecord = new ossimPointRecord(bounds.ur());
}

ossimGenericPointCloudHandler::ossimGenericPointCloudHandler(vector<ossimGpt>& ground_points)
{
   // Fill the point storage in any order.
   // Loop to add your points (assume your points are passed in a vector ecef_points[])
   for (ossim_uint32 i=0; i<ground_points.size(); ++i)
   {
      ossimPointRecord* point = new ossimPointRecord(ground_points[i]);
      m_pointBlock.addPoint(point);
   }
   ossimGrect bounds;
   m_pointBlock.getBounds(bounds);
   m_minRecord = new ossimPointRecord(bounds.ll());
   m_maxRecord = new ossimPointRecord(bounds.ur());
}

ossimGenericPointCloudHandler::~ossimGenericPointCloudHandler() 
{ 
   m_pointBlock.clear(); 
}

ossim_uint32 ossimGenericPointCloudHandler::getNumPoints() const 
{ 
   return m_pointBlock.size(); 
}

void ossimGenericPointCloudHandler::getFileBlock(ossim_uint32 offset,
                                                 ossimPointBlock& block,
                                                 ossim_uint32 /* maxNumPoints */) const
{
   block.clear();
   if (offset >= m_pointBlock.size())
      return;

   for (ossim_uint32 i=offset; i<m_pointBlock.size(); ++i)
      block.addPoint(new ossimPointRecord(*(m_pointBlock[i])));

   m_currentPID = block.size();
}

ossim_uint32 ossimGenericPointCloudHandler::getFieldCode() const 
{ 
  return 0; 
}

bool ossimGenericPointCloudHandler::open(const ossimFilename& /* pointsFile */)
{ 
   return false; 
}
 
void ossimGenericPointCloudHandler::close() 
{  
}

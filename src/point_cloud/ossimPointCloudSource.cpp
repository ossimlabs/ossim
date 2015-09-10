//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************

#include <ossim/point_cloud/ossimPointCloudSource.h>
#include <ossim/point_cloud/ossimPointCloudGeometry.h>
#include <ossim/point_cloud/ossimPointRecord.h>

RTTI_DEF1(ossimPointCloudSource, "ossimPointCloudSource" , ossimSource);


ossimPointCloudSource::ossimPointCloudSource(ossimObject* owner)
      : ossimSource(owner)
{
}

ossimPointCloudSource::ossimPointCloudSource(const ossimPointCloudSource& rhs)
      : ossimSource(rhs)
{
}

ossimPointCloudSource::~ossimPointCloudSource()
{
}

const ossimPointRecord*  ossimPointCloudSource::getMinPoint() const
{
   if (getInput(0) == 0)
      return &m_nullPCR;

   return ((ossimPointCloudSource*)getInput(0))->getMinPoint();
}

const ossimPointRecord*  ossimPointCloudSource::getMaxPoint() const
{
   if (getInput(0) == 0)
      return &m_nullPCR;

   return ((ossimPointCloudSource*)getInput(0))->getMaxPoint();
}

ossim_uint32  ossimPointCloudSource::getFieldCode() const
{
   if (getInput(0) == 0)
      return 0;

   return ((ossimPointCloudSource*)getInput(0))->getFieldCode();
}

void ossimPointCloudSource::setFieldCode(ossim_uint32 fc)
{
   if (getInput(0) != 0)
      ((ossimPointCloudSource*) getInput(0))->setFieldCode(fc);
}

bool ossimPointCloudSource::canConnectMyInputTo(ossim_int32 i,const ossimConnectableObject* p) const
{
   if ( (i>0) || (p==0) )
      return false;

   return (p->canCastTo("ossimPointCloudSource"));
}

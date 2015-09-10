//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id$

#include <ossim/point_cloud/ossimPointCloudGeometry.h>
#include <ossim/base/ossimEcefPoint.h>
#include <ossim/base/ossimUnitConversionTool.h>
#include <ossim/projection/ossimWktProjectionFactory.h>
#include <ossim/projection/ossimMapProjection.h>

RTTI_DEF1(ossimPointCloudGeometry, "ossimPointCloudGeometry" , ossimObject);

ossimPointCloudGeometry::ossimPointCloudGeometry(const ossimString& wkt)
      : m_format(GEOGRAPHIC),
        m_projection(0),
        m_datum(ossimDatumFactory::instance()->wgs84())
{
   // Establish projection from WKT:
   m_projection = dynamic_cast<ossimMapProjection*>(
         ossimWktProjectionFactory::instance()->createProjection(wkt));

   if (m_projection.valid())
   {
      m_datum = m_projection->getDatum();
      if (!m_projection->isGeographic())
         m_format = MAP_PROJECTED;
   }
}

bool ossimPointCloudGeometry::operator==(const ossimPointCloudGeometry& other) const
{
   if (m_format != other.m_format)
      return false;

   if (m_projection.valid() && other.m_projection.valid() && (*m_projection == *(other.m_projection)))
      return true;

   return false;
}

void ossimPointCloudGeometry::convertPos(const ossimDpt3d& oldPt, ossimGpt& converted_gpos) const
{
   converted_gpos.makeNan();

   // If the data buffer has existing points, transform them into the new projection and datum:
   if (m_format == GEOGRAPHIC)
   {
      ossimGpt gpt (oldPt.y, oldPt.x, oldPt.z, m_datum);
      gpt.changeDatum(converted_gpos.datum());
      converted_gpos = gpt;
   }
   else if ((m_format == MAP_PROJECTED) && m_projection.valid())
   {
      ossimUnitConversionTool uct (1.0, m_projection->getProjectionUnits());
      double coord_scale = uct.getMeters();

      //Need to convert map E, N to lat, lon
      ossimDpt oldMapPt(oldPt.x*coord_scale, oldPt.y*coord_scale);
      ossimGpt gpt(getProjection()->inverse(oldMapPt));
      gpt.changeDatum(m_datum);
      gpt.hgt = oldPt.z*coord_scale;
      gpt.changeDatum(converted_gpos.datum());
      converted_gpos = gpt;
   }
   else if (m_format == ECF_METERS)
   {
      ossimEcefPoint oldEcfPt(oldPt);
      converted_gpos = ossimGpt (oldEcfPt, converted_gpos.datum());
   }

   return;
}


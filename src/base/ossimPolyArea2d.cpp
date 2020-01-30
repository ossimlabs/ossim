//---
// License:  See top level LICENSE.txt file.
//
// $Id: ossimPolyArea2d.cpp 23623 2015-11-13 18:24:28Z gpotts $
//---
#include <ossim/base/ossimPolyArea2d.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimPolygon.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <vector>
#include <cstdio>
#include <cstdarg>
#include <geos_c.h>
#define GEOS_VERSION_COMBINED ((GEOS_VERSION_MAJOR * 10000) +  (GEOS_VERSION_MINOR*1000))
#define VERSION_38 38000

class ossimPolyArea2dPrivate
{
public:
   typedef GEOSGeometry *GEOSGeometryPtr;
   typedef const GEOSGeometry *ConstGEOSGeometryPtr;
   ossimPolyArea2dPrivate() : m_geometry(GEOSGeom_createEmptyPolygon()) {}
   virtual ~ossimPolyArea2dPrivate() { deleteGeometry(); }
   void deleteGeometry()
   {
      if (m_geometry)
         GEOSGeom_destroy(m_geometry);
      m_geometry = 0;
   }

   void setGeometry(GEOSGeometryPtr geom)
   {
      if(geom != m_geometry)
      {
         deleteGeometry();
         m_geometry = geom;
      }
   }
   void setGeometry(const ossimPolygon &polygon, const std::vector<ossimPolygon> &holes = std::vector<ossimPolygon>());

   void ringToPoints(const ConstGEOSGeometryPtr geom, std::vector<ossimDpt> &points) const;
   void recurseVisibleGeometries(ossimPolygon::Vector &polyList) const
   {
      recurseVisibleGeometries(m_geometry, polyList);
   }

   void recurseVisibleGeometries(ConstGEOSGeometryPtr geom,
                                 ossimPolygon::Vector &polygons) const;

   void getVisiblePolygons(ConstGEOSGeometryPtr geom,
                           ossimPolygon::Vector &polygons) const;

   bool getVisiblePolygons(ossimPolygon::Vector &polygons) const;

   void getHoles(ConstGEOSGeometryPtr geom,
                 ossimPolygon::Vector &polygons) const;
   bool getPolygonHoles(ossimPolygon::Vector &polygons) const;
   bool getPolygonHoles(ConstGEOSGeometryPtr geom,
                        ossimPolygon::Vector &polygons) const;
   void recurseGeometryHoles(ConstGEOSGeometryPtr geom,
                             ossimPolygon::Vector &polygons) const;
   void getBoundingRect(ossimDrect &bounds) const
   {
      bounds.makeNan();
      if (!isEmpty() && m_geometry)
      {
         GEOSGeometry *geom = GEOSEnvelope(m_geometry);
         if (geom)
         {
            ossimPolygon::Vector polys;
            getVisiblePolygons(geom, polys);
            for (ossim_int32 idx = 0; idx < polys.size(); ++idx)
            {
               if (bounds.isNan())
               {
                  polys[idx].getBoundingRect(bounds);
               }
               else
               {
                  ossimDrect tempRect;
                  polys[idx].getBoundingRect(tempRect);
                  bounds = bounds.combine(tempRect);
               }
            }
            GEOSGeom_destroy(geom);
            geom = 0;
         }
      }
   }
   std::string toString() const;
   bool setFromWkt(const std::string &s);

   bool isEmpty() const;
   bool isValid(bool displayValidationError = false) const;
   bool isPointWithin(const ossimDpt &pt) const;
   GEOSGeometryPtr m_geometry;
};

void ossimPolyArea2dPrivate::setGeometry(const ossimPolygon &exteriorRing,
                                         const std::vector<ossimPolygon> &interiorRings)
{

   deleteGeometry();

   if (exteriorRing.getNumberOfVertices() < 1)
      return;
   GEOSGeometryPtr shell = 0;
   std::vector<GEOSGeometryPtr> holes;
   const std::vector<ossimDpt> &pts = exteriorRing.getVertexList();
   ossim_int32 idx = 0;
   ossim_int32 n = (int)pts.size();

   bool firstAndLastSame = ((pts[0].x == pts[n - 1].x) && (pts[0].y == pts[n - 1].y));
   if (n > 0)
   {
      GEOSCoordSequence *shellSeq = GEOSCoordSeq_create(
          n + ((firstAndLastSame) ? 0 : 1), 2);

      //fill the exterior ring
      for (idx = 0; idx < n; idx++)
      {
         GEOSCoordSeq_setX(shellSeq, idx, pts[idx].x);
         GEOSCoordSeq_setY(shellSeq, idx, pts[idx].y);
      }
      //if the original polygon didn't have the first and last point the same, make it so
      if (!firstAndLastSame)
      {
         GEOSCoordSeq_setX(shellSeq, n, pts[0].x);
         GEOSCoordSeq_setY(shellSeq, n, pts[0].y);
      }
      shell = GEOSGeom_createLinearRing(shellSeq);
      //fill the interior rings
      if (!interiorRings.empty())
      {
         for (ossim_uint32 interiorRingIdx = 0; interiorRingIdx < interiorRings.size(); ++interiorRingIdx)
         {
            if (interiorRings[interiorRingIdx].getNumberOfVertices() > 0)
            {
               const std::vector<ossimDpt> &vertexPts = interiorRings[interiorRingIdx].getVertexList();
               firstAndLastSame = ((vertexPts[0].x == vertexPts[n - 1].x) && (vertexPts[0].y == vertexPts[n - 1].y));

               GEOSCoordSequence *ring = GEOSCoordSeq_create(
                   vertexPts.size() + ((firstAndLastSame) ? 0 : 1), 2);
               for (ossim_uint32 vertexIndex = 0; vertexIndex < vertexPts.size(); ++vertexIndex)
               {
                  GEOSCoordSeq_setX(ring, vertexIndex, vertexPts[vertexIndex].x);
                  GEOSCoordSeq_setY(ring, vertexIndex, vertexPts[vertexIndex].y);
               }

               //if the original polygon didn't have the first and last point the same, make it so
               if (!firstAndLastSame)
               {
                  GEOSCoordSeq_setX(ring, vertexPts.size(), vertexPts[0].x);
                  GEOSCoordSeq_setY(ring, vertexPts.size(), vertexPts[0].y);
               }
               GEOSGeometryPtr hole = GEOSGeom_createLinearRing(ring);
               holes.push_back(hole);
            }
         }
      }

      if (shell)
      {
         if (holes.size())
         {
            m_geometry = GEOSGeom_createPolygon(shell, &holes.front(), holes.size());
         }
         else
         {
            m_geometry = GEOSGeom_createPolygon(shell, 0, 0);
         }
      }
      else
      {
         m_geometry = 0;
      }
   }
}

void ossimPolyArea2dPrivate::ringToPoints(const ConstGEOSGeometryPtr geom, std::vector<ossimDpt> &points) const
{
   double x, y;
   if (!geom)
      return;
   ossim_int32 nPoints = GEOSGetNumCoordinates(geom);
   if (nPoints > 0)
   {
      const GEOSCoordSequence *seq = GEOSGeom_getCoordSeq(geom);
      ossim_int32 i = 0;
      for (i = 0; i < nPoints; i++)
      {
         GEOSCoordSeq_getX(seq, i, &x);
         GEOSCoordSeq_getY(seq, i, &y);
         points.push_back(ossimDpt(x, y));
      }
   }
}

void ossimPolyArea2dPrivate::getHoles(ConstGEOSGeometryPtr geom,
                                      ossimPolygon::Vector &polygons) const
{
   int geomType = GEOSGeomTypeId(geom);
   std::vector<ossimDpt> points;
   switch (geomType)
   {
   case GEOS_LINESTRING:
   case GEOS_LINEARRING:
   {
      ringToPoints(geom, points);
      polygons.push_back(ossimPolygon(points));
      break;
   }
   }
}

void ossimPolyArea2dPrivate::getVisiblePolygons(ConstGEOSGeometryPtr geom,
                                                ossimPolygon::Vector &polygons) const
{
   int geomType = GEOSGeomTypeId(geom);
   std::vector<ossimDpt> points;

   switch (geomType)
   {
   case GEOS_LINESTRING:
   case GEOS_LINEARRING:
   {
      ringToPoints(geom, points);
      polygons.push_back(ossimPolygon(points));
      break;
   }
   case GEOS_POLYGON:
   {
      ConstGEOSGeometryPtr geom2 = GEOSGetExteriorRing(geom);
      ringToPoints(geom2, points);
      polygons.push_back(ossimPolygon(points));

      break;
   }
   }
}
void ossimPolyArea2dPrivate::recurseVisibleGeometries(ConstGEOSGeometryPtr geom,
                                                      ossimPolygon::Vector &polygons) const
{
   if (!geom)
      return;

   ossim_int32 nGeoms = GEOSGetNumGeometries(geom);
   ConstGEOSGeometryPtr geomPtr = 0;
   if (nGeoms == 0)
      return;

   if (nGeoms == 1)
   {
      geomPtr = GEOSGetGeometryN(geom, 0);
      if (geomPtr)
         getVisiblePolygons(geomPtr, polygons);
   }
   else
   {
      for (int idx = 0; idx < nGeoms; ++idx)
      {
         geomPtr = GEOSGetGeometryN(geom, idx);
         if (geomPtr)
            recurseVisibleGeometries(geomPtr, polygons);
      }
   }
}

void ossimPolyArea2dPrivate::recurseGeometryHoles(ConstGEOSGeometryPtr geom,
                                                  ossimPolygon::Vector &polygons) const
{
   if (!geom)
      return;

   ossim_int32 nGeoms = GEOSGetNumGeometries(geom);
   if (nGeoms == 0)
      return;

   if (nGeoms == 1)
   {
      ossim_int32 nInteriorRings = GEOSGetNumInteriorRings(geom);
      ossim_int32 idx = 0;

      for (idx = 0; idx < nInteriorRings; ++idx)
      {
         const GEOSGeometry *ringGeom = GEOSGetInteriorRingN(geom, idx);
         getHoles(ringGeom, polygons);
      }
   }
   else
   {
      ConstGEOSGeometryPtr geomPtr = 0;
      for (int idx = 0; idx < nGeoms; ++idx)
      {
         geomPtr = GEOSGetGeometryN(geom, idx);
         recurseGeometryHoles(geomPtr, polygons);
      }
   }
}

bool ossimPolyArea2dPrivate::getVisiblePolygons(ossimPolygon::Vector &polygons) const
{
   bool foundPolys = false;
   if (m_geometry)
   {
      ossim_uint32 sizeBefore = (ossim_uint32)polygons.size();
      recurseVisibleGeometries(m_geometry, polygons);
      foundPolys = (sizeBefore != polygons.size());
   }

   return foundPolys;
}
bool ossimPolyArea2dPrivate::getPolygonHoles(ossimPolygon::Vector &polygons) const
{
   return getPolygonHoles(m_geometry, polygons);
}

bool ossimPolyArea2dPrivate::getPolygonHoles(ConstGEOSGeometryPtr geom,
                                             ossimPolygon::Vector &polygons) const
{
   bool foundPolys = false;
   if (m_geometry)
   {
      ossim_uint32 sizeBefore = (ossim_uint32)polygons.size();
      recurseGeometryHoles(m_geometry, polygons);
      foundPolys = (sizeBefore != polygons.size());
   }

   return foundPolys;
}

std::string ossimPolyArea2dPrivate::toString() const
{
   std::string result;

   if (m_geometry)
   {
      GEOSWKTWriter *wktWriter = GEOSWKTWriter_create();
      GEOSWKTWriter_setRoundingPrecision(wktWriter, 20);
      char *wkt_c = GEOSWKTWriter_write(wktWriter, m_geometry);

      result = wkt_c;
      GEOSWKTWriter_destroy(wktWriter);

      GEOSFree(wkt_c);
   }

   return result;
}
bool ossimPolyArea2dPrivate::setFromWkt(const std::string &s)
{
   bool result = false;

   GEOSWKTReader *reader = GEOSWKTReader_create();
   GEOSGeometry *geom = GEOSWKTReader_read(reader, s.c_str());
   result = (geom != 0);
   setGeometry(geom);

   GEOSWKTReader_destroy(reader);

   return result;
}


bool ossimPolyArea2dPrivate::isEmpty() const
{
   bool result = true;
   if (m_geometry)
   {
      result = (GEOSisEmpty(m_geometry) == 1);
   }

   return result;
}

bool ossimPolyArea2dPrivate::isValid(bool displayValidationError) const
{
   bool result = false;

   if (m_geometry)
   {
      if (!displayValidationError)
      {
         result = GEOSisValid(m_geometry) == 1;
      }
      else
      {
         char *reason = GEOSisValidReason(m_geometry);
         if (reason)
         {
            ossimNotify(ossimNotifyLevel_INFO)
               << "ossimPolyArea2dPrivate::isValid: " << reason << "\n";

            GEOSFree(reason);
            reason = 0;
         }
      }
   }
   return result;
}
bool ossimPolyArea2dPrivate::isPointWithin(const ossimDpt &pt) const
{
   bool result = false;

   if (!isEmpty())
   {
      GEOSCoordSequence *pointSeq = GEOSCoordSeq_create(1, 2);
      GEOSCoordSeq_setX(pointSeq, 0, pt.x);
      GEOSCoordSeq_setY(pointSeq, 0, pt.y);
      GEOSGeometry *geom = GEOSGeom_createPoint(pointSeq);
      result = (GEOSWithin(geom, m_geometry) == 1);

      GEOSGeom_destroy(geom);
   }

   return result;
}


ossimPolyArea2d::ossimPolyArea2d()
    : m_privateData(new ossimPolyArea2dPrivate())
{
}

ossimPolyArea2d::ossimPolyArea2d(const std::vector<ossimGpt> &polygon)
    : m_privateData(new ossimPolyArea2dPrivate())
{
   m_privateData->setGeometry(polygon);
}

ossimPolyArea2d::ossimPolyArea2d(const std::vector<ossimDpt> &polygon)
    : m_privateData(new ossimPolyArea2dPrivate())
{
   m_privateData->setGeometry(polygon);
}

ossimPolyArea2d::ossimPolyArea2d(const ossimPolygon &shell, const std::vector<ossimPolygon> &holes)
    : m_privateData(new ossimPolyArea2dPrivate())
{
   m_privateData->setGeometry(shell, holes);
}

ossimPolyArea2d::ossimPolyArea2d(const ossimDpt &p1,
                                 const ossimDpt &p2,
                                 const ossimDpt &p3,
                                 const ossimDpt &p4)
    : m_privateData(new ossimPolyArea2dPrivate())
{
   m_privateData->setGeometry(ossimPolygon(p1, p2, p3, p4));
}

ossimPolyArea2d::ossimPolyArea2d(const ossimPolyArea2d &rhs)
    : m_privateData(new ossimPolyArea2dPrivate())
{
   if (rhs.m_privateData->m_geometry)
   {
      m_privateData->deleteGeometry();
      m_privateData->m_geometry = GEOSGeom_clone(rhs.m_privateData->m_geometry);
   }
}

ossimPolyArea2d::ossimPolyArea2d(const ossimIrect &rect)
    : m_privateData(new ossimPolyArea2dPrivate())
{
   m_privateData->setGeometry(ossimPolygon(rect));
}

ossimPolyArea2d::ossimPolyArea2d(const ossimDrect &rect)
    : m_privateData(new ossimPolyArea2dPrivate())
{
   m_privateData->setGeometry(ossimPolygon(rect));
}

ossimPolyArea2d::ossimPolyArea2d(const ossimPolygon &polygon)
    : m_privateData(new ossimPolyArea2dPrivate())
{
   m_privateData->setGeometry(polygon);
}

ossimPolyArea2d::~ossimPolyArea2d()
{
   if (m_privateData)
   {
      delete m_privateData;
   }
   m_privateData = 0;
}

void ossimPolyArea2d::clearPolygons()
{
   m_privateData->setGeometry(GEOSGeom_createEmptyPolygon());
}

const ossimPolyArea2d &ossimPolyArea2d::operator=(const ossimPolyArea2d &rhs)
{
   if (&rhs != this)
   {
      m_privateData->deleteGeometry();
      if (rhs.m_privateData->m_geometry)
         m_privateData->m_geometry = GEOSGeom_clone(rhs.m_privateData->m_geometry);
   }

   return *this;
}

const ossimPolyArea2d &ossimPolyArea2d::operator=(const ossimPolygon &rhs)
{
   m_privateData->setGeometry(rhs);

   return *this;
}

const ossimPolyArea2d &ossimPolyArea2d::operator=(const ossimIrect &rect)
{
   m_privateData->setGeometry(ossimPolygon(rect));

   return *this;
}

const ossimPolyArea2d &ossimPolyArea2d::operator=(const ossimDrect &rect)
{
   m_privateData->setGeometry(ossimPolygon(rect));

   return *this;
}

const ossimPolyArea2d &ossimPolyArea2d::operator=(const std::vector<ossimGpt> &polygon)
{
   m_privateData->setGeometry(ossimPolygon(polygon));

   return *this;
}

const ossimPolyArea2d &ossimPolyArea2d::operator=(const std::vector<ossimDpt> &polygon)
{
   m_privateData->setGeometry(ossimPolygon(polygon));

   return *this;
}

const ossimPolyArea2d &ossimPolyArea2d::operator&=(const ossimPolyArea2d &rhs)
{
   GEOSGeometry *geom = nullptr;
   if (m_privateData->m_geometry && rhs.m_privateData->m_geometry)
      geom = GEOSIntersection(m_privateData->m_geometry, rhs.m_privateData->m_geometry);
   else if (rhs.m_privateData->m_geometry)
      geom = GEOSGeom_clone(rhs.m_privateData->m_geometry);

   m_privateData->setGeometry(geom);
   return *this;
}

ossimPolyArea2d ossimPolyArea2d::operator&(const ossimPolyArea2d &rhs) const
{
   ossimPolyArea2d result(*this);

   result &= rhs;

   return result;
}

ossimPolyArea2d ossimPolyArea2d::operator+(const ossimPolyArea2d &rhs) const
{
   ossimPolyArea2d result(*this);

   result += rhs;

   return result;
}

const ossimPolyArea2d &ossimPolyArea2d::operator+=(const ossimPolyArea2d &rhs)
{
   GEOSGeometry *geom = nullptr;
   if (m_privateData->m_geometry && rhs.m_privateData->m_geometry)
      geom = GEOSUnion(m_privateData->m_geometry, rhs.m_privateData->m_geometry);
   else if (rhs.m_privateData->m_geometry)
      geom = GEOSGeom_clone(rhs.m_privateData->m_geometry);

   m_privateData->setGeometry(geom);
   return *this;
}

ossimPolyArea2d ossimPolyArea2d::operator-(const ossimPolyArea2d &rhs) const
{
   ossimPolyArea2d result(*this);

   result -= rhs;

   return result;
}

const ossimPolyArea2d &ossimPolyArea2d::operator-=(const ossimPolyArea2d &rhs)
{
   GEOSGeometry *geom = nullptr;
   if (m_privateData->m_geometry && rhs.m_privateData->m_geometry)
      geom = GEOSDifference(m_privateData->m_geometry, rhs.m_privateData->m_geometry);
   else if (rhs.m_privateData->m_geometry)
      geom = GEOSGeom_clone(rhs.m_privateData->m_geometry);

   m_privateData->setGeometry(geom);
   return *this;
}

ossim_float64 ossimPolyArea2d::getArea()const
{
   double result = 0.0;
   
   if(!isEmpty())
      GEOSArea(m_privateData->m_geometry, &result);

   return result;
}

bool ossimPolyArea2d::isEmpty() const
{
   return m_privateData->isEmpty();
}

bool ossimPolyArea2d::isValid(bool displayValidationError) const
{
   return m_privateData->isValid(displayValidationError);
}

bool ossimPolyArea2d::isPointWithin(const ossimDpt &point) const
{
   return m_privateData->isPointWithin(point);
}

bool ossimPolyArea2d::isPointWithin(double x, double y) const
{
   return isPointWithin(ossimDpt(x, y));
}

void ossimPolyArea2d::getBoundingRect(ossimDrect &rect) const
{
   m_privateData->getBoundingRect(rect);
}

bool ossimPolyArea2d::intersects(const ossimPolyArea2d &rhs) const
{
   bool result=false;
   if (!isEmpty() && !rhs.isEmpty())
      result = (GEOSIntersects(m_privateData->m_geometry,rhs.m_privateData->m_geometry) == 1);
   return result;
}

void ossimPolyArea2d::makeValid()
{
#if (GEOS_VERSION_COMBINED < VERSION_38)
   ossimNotify(ossimNotifyLevel_WARN) << "ossimPolyArea2d::makeValid() is only callable from geos 3.8 and above\n";
#else
   if (m_privateData->m_geometry)
   {
      ossimPolyArea2dPrivate::GEOSGeometryPtr geom = GEOSMakeValid(m_privateData->m_geometry);
      if(geom) m_privateData->setGeometry(geom);
   }
#endif
}


void ossimPolyArea2d::add(const ossimPolyArea2d &rhs)
{
   *this += rhs;
}

bool ossimPolyArea2d::getVisiblePolygons(std::vector<ossimPolygon> &polyList) const
{
   m_privateData->getVisiblePolygons(polyList);

   return (polyList.size() > 0);
}

bool ossimPolyArea2d::getPolygonHoles(std::vector<ossimPolygon> &polyList) const
{
   m_privateData->getPolygonHoles(polyList);

   return (polyList.size() > 0);
}

ossimPolyArea2d &ossimPolyArea2d::toMultiPolygon()
{
   if (m_privateData->m_geometry)
   {
      int geomType = GEOSGeomTypeId(m_privateData->m_geometry);

      if (geomType != GEOS_MULTIPOLYGON)
      {
         std::vector<GEOSGeometry *> geoms(1);
         geoms[0] = GEOSGeom_clone(m_privateData->m_geometry);
         GEOSGeometry *result = GEOSGeom_createCollection(GEOS_MULTIPOLYGON,
                                                          &geoms.front(), 1);
         m_privateData->setGeometry(result);
      }
   }
   return *this;
}

std::string ossimPolyArea2d::toString() const
{
   return m_privateData->toString();
}

bool ossimPolyArea2d::setFromWkt(const std::string &s)
{
   return m_privateData->setFromWkt(s);
}

bool ossimPolyArea2d::saveState(ossimKeywordlist &kwl,
                                const char *prefix) const
{
   kwl.add(prefix,
           ossimKeywordNames::TYPE_KW,
           "ossimPolyArea2d",
           true);

   if (!isEmpty())
   {

      kwl.add(prefix,
              "wkt",
              toString().c_str(),
              true);
   }
   return true;
}

bool ossimPolyArea2d::loadState(const ossimKeywordlist &kwl,
                                const char *prefix)
{
   bool result = true;

   if (m_privateData)
   {
      ossimString wkt = kwl.find(prefix, "wkt");

      if (!wkt.empty())
      {
         result = setFromWkt(wkt.string());
      }
   }

   return result;
}

std::ostream &operator<<(std::ostream &out, const ossimPolyArea2d &rhs)
{
   if (!rhs.isEmpty())
   {
      out << rhs.toString();
   }

   return out;
}

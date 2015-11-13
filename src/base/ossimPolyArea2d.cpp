//---
// License:  See top level LICENSE.txt file.
//
// $Id: ossimPolyArea2d.cpp 23623 2015-11-13 18:24:28Z gpotts $
//---

#include <ossim/base/ossimPolyArea2d.h>

#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/opBuffer.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/util/GEOSException.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/opBuffer.h>
#include <cstdlib>
#include <exception>
#include <vector>

class ossimGeometryFactoryWrapper : public ossimReferenced
{
public:
   ossimGeometryFactoryWrapper()
      : m_geomFactory(0)
   {
      geos::geom::PrecisionModel *pm =
         new geos::geom::PrecisionModel(geos::geom::PrecisionModel::FLOATING);
      m_geomFactory = new geos::geom::GeometryFactory(pm, -1); 
   }
   virtual ~ossimGeometryFactoryWrapper(){if(m_geomFactory) delete m_geomFactory;m_geomFactory=0;}
   
   geos::geom::GeometryFactory* m_geomFactory;
};

class OssimPolyArea2dPrivate
{
public:
   typedef geos::geom::Geometry* GeometryPtr;
   typedef const geos::geom::Geometry* ConstGeometryPtr;
   
   OssimPolyArea2dPrivate(GeometryPtr geom=0);
   ~OssimPolyArea2dPrivate();
   
   void deleteGeometry() { if(m_geometry) { delete m_geometry; m_geometry = 0; }}
   void setGeometry(const ossimPolygon& polygon, const vector<ossimPolygon>& holes = vector<ossimPolygon>());
   void setGeometry(GeometryPtr geom){deleteGeometry();m_geometry=geom;}
   geos::geom::GeometryFactory* geomFactory(){{return m_globalFactory.valid()?m_globalFactory->m_geomFactory:0;}}
   GeometryPtr m_geometry;
   static ossimRefPtr<ossimGeometryFactoryWrapper> m_globalFactory; 
};

ossimRefPtr<ossimGeometryFactoryWrapper> OssimPolyArea2dPrivate::m_globalFactory;

OssimPolyArea2dPrivate::OssimPolyArea2dPrivate(GeometryPtr geom)
:m_geometry(geom)
{
   static OpenThreads::Mutex globalFactoryMutex;
   
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(globalFactoryMutex);
      if(!m_globalFactory.valid())
      {
         m_globalFactory = new ossimGeometryFactoryWrapper();
      }    
   }
}

OssimPolyArea2dPrivate::~OssimPolyArea2dPrivate()
{
   deleteGeometry();
}

void OssimPolyArea2dPrivate::setGeometry(
   const ossimPolygon& exteriorRing, const vector<ossimPolygon>& interiorRings)
{
   deleteGeometry();
   
   geos::geom::CoordinateArraySequence *cas = new geos::geom::CoordinateArraySequence();
   
   const std::vector<ossimDpt>& pts = exteriorRing.getVertexList();

   int idx = 0;
   int n = (int)pts.size();
   
   if(n > 0)
   {
      //fill the exterior ring
      for (idx = 0; idx < n; idx++)
      {
         cas->add(geos::geom::Coordinate(pts[idx].x, pts[idx].y));
      }
      
      //if the original polygon didn't have the first and last point the same, make it so
      if((pts[0].x != pts[n-1].x) || (pts[0].y!=pts[n-1].y))
      {
         cas->add(geos::geom::Coordinate(pts[0].x, pts[0].y));
      }
      
      //fill the interior rings
      vector<geos::geom::Geometry*> *holes = new vector<geos::geom::Geometry*>();
      for (ossim_uint32 interiorRingIdx = 0; interiorRingIdx < interiorRings.size(); ++interiorRingIdx)
      {
         geos::geom::CoordinateArraySequence *interiorCas =
            new geos::geom::CoordinateArraySequence();
         const std::vector<ossimDpt>& vertexPts = interiorRings[interiorRingIdx].getVertexList();
         for(ossim_uint32 vertexIndex=0; vertexIndex < vertexPts.size(); ++vertexIndex)
         {
            interiorCas->add(geos::geom::Coordinate(vertexPts[vertexIndex].x,
                                                    vertexPts[vertexIndex].y));
         }
         
         //if the original polygon didn't have the first and last point the same, make it so
         if((vertexPts[0].x != vertexPts[vertexPts.size()-1].x) ||
            (vertexPts[0].y!=vertexPts[vertexPts.size()-1].y))
         {
            interiorCas->add(geos::geom::Coordinate(vertexPts[0].x, vertexPts[0].y));
         }
         
         geos::geom::LinearRing *hole = geomFactory()->createLinearRing(interiorCas);
         holes->push_back(hole);
      }
      
      geos::geom::LinearRing* shell = geomFactory()->createLinearRing(cas);
      if ( shell )
      {
         m_geometry = geomFactory()->createPolygon(shell, holes);
      }
      else
      {
         m_geometry = 0;
      }
   }
}

void ossimPolyArea2d::recurseVisibleGeometries(
   std::vector<ossimPolygon>& polyList, const geos::geom::Geometry* geom) const
{
   int nGeoms = (int)geom->getNumGeometries();
   
   if(nGeoms < 2 )
   {
      const geos::geom::Polygon* poly = dynamic_cast<const geos::geom::Polygon*> (geom);
      
      if (poly)
      {
         const geos::geom::LineString* lineString = dynamic_cast<const geos::geom::LineString*> (poly->getExteriorRing());
         if (lineString)
         {
            int currentPolyIdx = (int)polyList.size();
            int nPoints = (int)lineString->getNumPoints();
            int idx = 0;
            
            polyList.push_back(ossimPolygon());
            
            for (idx=0; idx<nPoints; idx++)
            {
               std::auto_ptr<const geos::geom::Point> point(lineString->getPointN(idx));
               polyList[currentPolyIdx].addPoint(point->getX(), point->getY());
            }
         }
      }
   }
   else
   {
      for (int idx=0; idx < nGeoms; ++idx)
      {
         recurseVisibleGeometries(polyList, geom->getGeometryN(idx));
      }
   }
}

void ossimPolyArea2d::recurseHoles(std::vector<ossimPolygon>& polyList,
                                   const geos::geom::Geometry* geom) const
{
   int nGeoms = (int)geom->getNumGeometries();
   
   if(nGeoms < 2 )
   {
      const geos::geom::Polygon* poly = dynamic_cast<const geos::geom::Polygon*> (geom);

      if (poly)
      {
         ossim_uint32 nInteriorRings = (ossim_uint32)poly->getNumInteriorRing();
         ossim_uint32 idx = 0;
         
         for(idx = 0; idx < nInteriorRings; ++idx)
         {
            const geos::geom::LineString* lineString = poly->getInteriorRingN(idx);
            if (lineString)
            {
               int currentPolyIdx = (int)polyList.size();
               int nPoints = (int)lineString->getNumPoints();
               int idx = 0;

               polyList.push_back(ossimPolygon());

               for (idx=0; idx<nPoints; idx++)
               {
                  std::auto_ptr<const geos::geom::Point> point(lineString->getPointN(idx));
                  polyList[currentPolyIdx].addPoint(point->getX(), point->getY());
               }
            }
         }
      }
   }
   else
   {
      int idx = 0;
      
      for (idx=0; idx < nGeoms; idx++)
      {
         recurseHoles(polyList, geom->getGeometryN(idx));
      }
   }
}

void ossimPolyArea2d::recurseCompleteGeometries(std::vector<ossimPolyArea2d>& polyList,
                                                const geos::geom::Geometry* geom) const
{
   int nGeoms = (int)geom->getNumGeometries();
   if(nGeoms < 2 )
   {
      const geos::geom::Polygon* poly = dynamic_cast<const geos::geom::Polygon*> (geom);

      if (poly)
      {
         //get exterior shell for the geometry
         ossimPolygon shell;
         const geos::geom::LineString* lineString =
            dynamic_cast<const geos::geom::LineString*> (poly->getExteriorRing());
         if (lineString)
         {
            int nPoints = (int)lineString->getNumPoints();
            for (int idx = 0; idx<nPoints; idx++)
            {
               std::auto_ptr<const geos::geom::Point> point(lineString->getPointN(idx));
               shell.addPoint(point->getX(), point->getY());
            }
         }
         
         // Get interior rings for the geometry.
         std::size_t nInteriorRings = poly->getNumInteriorRing();
         vector<ossimPolygon> holes(nInteriorRings);
         for(std::size_t holeIdx = 0; holeIdx < nInteriorRings; ++holeIdx)
         {
            const geos::geom::LineString* lineString = poly->getInteriorRingN(holeIdx);
            if (lineString)
            {
               std::size_t nPoints = lineString->getNumPoints();
               for (std::size_t idx = 0; idx<nPoints; ++idx)
               {
                  std::auto_ptr<const geos::geom::Point> point(lineString->getPointN(idx));
                  holes[holeIdx].addPoint(point->getX(), point->getY());
               }
            }
         }
         polyList.push_back(ossimPolyArea2d(shell, holes));
      }
   }
   else
   {
      int idx = 0;
      
      for (idx=0; idx < nGeoms; idx++)
      {
         recurseCompleteGeometries(polyList, geom->getGeometryN(idx));
      }
   }
}

std::ostream& operator <<(std::ostream& out, const ossimPolyArea2d& rhs)
{
   if(rhs.m_privateData->m_geometry)
   {
      out << rhs.m_privateData->m_geometry->toString();
   }
   return out;
}

ossimPolyArea2d::ossimPolyArea2d()
   :m_privateData(new OssimPolyArea2dPrivate)
{
}

ossimPolyArea2d::ossimPolyArea2d(const vector<ossimGpt>& polygon)
   :m_privateData(new OssimPolyArea2dPrivate)
{
   (*this) = polygon;
}

ossimPolyArea2d::ossimPolyArea2d(const ossimIrect& rect)
   :m_privateData(new OssimPolyArea2dPrivate)
{
   (*this) = rect;
}

ossimPolyArea2d::ossimPolyArea2d(const ossimDrect& rect)
   :m_privateData(new OssimPolyArea2dPrivate)
{
   (*this) = rect;
}

ossimPolyArea2d::ossimPolyArea2d(const ossimPolygon& polygon)
   :m_privateData(new OssimPolyArea2dPrivate)
{
   (*this) = polygon;
}

ossimPolyArea2d::ossimPolyArea2d(const ossimPolygon& exteriorRing, const vector<ossimPolygon>& interiorRings)
   :m_privateData(new OssimPolyArea2dPrivate)
{	
	m_privateData->setGeometry(exteriorRing, interiorRings);
}

ossimPolyArea2d::ossimPolyArea2d(const ossimPolyArea2d& rhs)
   :m_privateData(new OssimPolyArea2dPrivate) 
{
   *this = rhs;
}

ossimPolyArea2d::ossimPolyArea2d(const ossimDpt& p1,
                                 const ossimDpt& p2,
                                 const ossimDpt& p3,
                                 const ossimDpt& p4)
   :
   m_privateData(new OssimPolyArea2dPrivate)
{
   ossimPolygon temp(p1,p2,p3,p4);
   *this = temp;
}

ossimPolyArea2d::~ossimPolyArea2d()
{
   if(m_privateData)
   {
      delete m_privateData;
      m_privateData = 0;
   }
}

const ossimPolyArea2d& ossimPolyArea2d::operator =(const ossimPolyArea2d& rhs)
{ 
   if(this != &rhs)
   {
      if(rhs.m_privateData->m_geometry)
      {
         m_privateData->setGeometry(rhs.m_privateData->m_geometry->clone());
      }
   }
   return *this;
}

const ossimPolyArea2d& ossimPolyArea2d::operator =(const ossimPolygon& polygon)
{
   m_privateData->setGeometry(polygon);

   return *this;
}

const ossimPolyArea2d& ossimPolyArea2d::operator =(const ossimIrect& rect)
{
   return (*this = ossimPolygon(rect));
}

const ossimPolyArea2d& ossimPolyArea2d::operator =(const ossimDrect& rect)
{
   return (*this = ossimPolygon(rect));
}

const ossimPolyArea2d& ossimPolyArea2d::operator =(const vector<ossimGpt>& polygon)
{
   std::vector<ossimDpt> pts;
   int idx = 0;
   int n = (int)polygon.size();
   for(idx = 0; idx < n;++idx)
   {
      pts.push_back(polygon[idx]);
   }
  
   return (*this = ossimPolygon(pts));
}

const ossimPolyArea2d& ossimPolyArea2d::operator =(const vector<ossimDpt>& polygon)
{
   return (*this = ossimPolygon(polygon));
}

bool ossimPolyArea2d::intersects(const ossimPolyArea2d& rhs)const
{
   bool result = false;

   if(m_privateData->m_geometry&&rhs.m_privateData->m_geometry)
   {
      result = m_privateData->m_geometry->intersects(rhs.m_privateData->m_geometry); 
   }

   return result;
}

ossimPolyArea2d ossimPolyArea2d::operator &(const ossimPolyArea2d& rhs)const
{
   if((this!=&rhs) && m_privateData->m_geometry && rhs.m_privateData->m_geometry)
   {
      ossimPolyArea2d result;
      try // GEOS code throws exceptions...
      {
         result.m_privateData->setGeometry(m_privateData->m_geometry->intersection(
                                              rhs.m_privateData->m_geometry));
      }
      catch( const std::exception& e )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator& Caught exception: " << e.what() << std::endl;
         result.clearPolygons();
      }
      catch( ... )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator& Caught exception!" << std::endl;
         result.clearPolygons();
      }
      return result;
   }
   return *this;
}

ossimPolyArea2d ossimPolyArea2d::operator +(const ossimPolyArea2d& rhs)const
{
   if((this!=&rhs) && m_privateData->m_geometry && rhs.m_privateData->m_geometry)
   {
      ossimPolyArea2d result;
      try // GEOS code throws exceptions...
      {
         result.m_privateData->setGeometry(m_privateData->m_geometry->Union(
                                              rhs.m_privateData->m_geometry));
      }  
      catch( const std::exception& e )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator+ Caught exception: " << e.what() << std::endl;
         result.clearPolygons();
      }
      catch( ... )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator+ Caught exception!" << std::endl;
         result.clearPolygons();
      }
      return result;
   }
   return *this;
}
ossimPolyArea2d ossimPolyArea2d::operator -(const ossimPolyArea2d& rhs)const
{
   if((this!=&rhs) && m_privateData->m_geometry && rhs.m_privateData->m_geometry)
   {
      ossimPolyArea2d result;
      try // GEOS code throws exceptions...
      {
         result.m_privateData->setGeometry(m_privateData->m_geometry->difference(
                                              rhs.m_privateData->m_geometry));
      }
      catch( const std::exception& e )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator- Caught exception: " << e.what() << std::endl;
         result.clearPolygons();
      }
      catch( ... )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator- Caught exception!" << std::endl;
         result.clearPolygons();
      }
      return result;
   }
   return *this;
}

const ossimPolyArea2d& ossimPolyArea2d::operator &=(const ossimPolyArea2d& rhs)
{
   if((this!=&rhs) && m_privateData->m_geometry && rhs.m_privateData->m_geometry)
   {
      try // GEOS code throws exceptions...
      {
         m_privateData->setGeometry(m_privateData->m_geometry->intersection(
                                       rhs.m_privateData->m_geometry));
      }
      catch( const std::exception& e )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator&= Caught exception: " << e.what() << std::endl;
         this->clearPolygons();
      }
      catch( ... )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator&= Caught exception!" << std::endl;
         this->clearPolygons();
      }      
   }
   return *this;
}

const ossimPolyArea2d& ossimPolyArea2d::operator +=(const ossimPolyArea2d& rhs)
{
   if((this!=&rhs) && m_privateData->m_geometry && rhs.m_privateData->m_geometry)
   {
      try // GEOS code throws exceptions...
      {
         m_privateData->setGeometry(m_privateData->m_geometry->Union(
                                       rhs.m_privateData->m_geometry));
      }
      catch( const std::exception& e )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator+= Caught exception: " << e.what() << std::endl;
         this->clearPolygons();
      }
      catch( ... )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator+= Caught exception!" << std::endl;
         this->clearPolygons();
      }      
   }
   return *this;
}

const ossimPolyArea2d& ossimPolyArea2d::operator -=(const ossimPolyArea2d& rhs)
{
   if((this!=&rhs) && m_privateData->m_geometry && rhs.m_privateData->m_geometry)
   {
      try // GEOS code throws exceptions...
      {
         m_privateData->setGeometry(m_privateData->m_geometry->difference(
                                       rhs.m_privateData->m_geometry));
      }
      catch( const std::exception& e )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator-= Caught exception: " << e.what() << std::endl;
         this->clearPolygons();
      }
      catch( ... )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimPolyArea2d::operator-= Caught exception!" << std::endl;
         this->clearPolygons();
      }      
   }
   return *this;
}

void ossimPolyArea2d::add(const ossimPolyArea2d& rhs)
{
   geos::geom::Geometry* geom = m_privateData->m_geometry->Union(rhs.m_privateData->m_geometry);
   if(geom) m_privateData->setGeometry(geom);
}

void ossimPolyArea2d::clearPolygons()
{
   m_privateData->deleteGeometry();
#if 0
   clearEngine();
#endif
}

bool ossimPolyArea2d::getVisiblePolygons(vector<ossimPolygon>& polyList)const
{
   bool foundPolys = false;
   if(m_privateData->m_geometry)
   {
      ossim_uint32 sizeBefore = (ossim_uint32)polyList.size();
      recurseVisibleGeometries(polyList, m_privateData->m_geometry);
      foundPolys = (sizeBefore != polyList.size());
   }

   return foundPolys;
}

bool ossimPolyArea2d::getPolygonHoles(vector<ossimPolygon>& polyList)const
{
   bool foundPolys = false;
   if(m_privateData->m_geometry)
   {
      ossim_uint32 sizeBefore = (ossim_uint32)polyList.size();
      recurseHoles(polyList, m_privateData->m_geometry);
      foundPolys = (sizeBefore != polyList.size());
   }

   return foundPolys;
}

bool ossimPolyArea2d::getCompletePolygons(vector<ossimPolyArea2d>& polyList)const
{
	bool foundPolys = false;
	if(m_privateData->m_geometry){
		ossim_uint32 sizeBefore = (ossim_uint32)polyList.size();
		recurseCompleteGeometries(polyList, m_privateData->m_geometry);
		foundPolys = (sizeBefore != polyList.size());
	}
	return foundPolys;
}

bool ossimPolyArea2d::isEmpty()const
{
   bool result = true;
   if (m_privateData&&m_privateData->m_geometry)
   {
      result = m_privateData->m_geometry->isEmpty();
   }

   return result;
}

bool ossimPolyArea2d::isValid(bool displayValidationError)const
{
   bool result = false;
   
   if(m_privateData&&m_privateData->m_geometry)
   {
      if(displayValidationError)
      {
         geos::operation::valid::IsValidOp validityCheck(m_privateData->m_geometry);
         geos::operation::valid::TopologyValidationError*
            topologyValidationError(validityCheck.getValidationError());
         // if(topologyValidationError == nullptr)
         if(topologyValidationError == 0)
         {
            result = true;
         }
         else
         {
            ossimNotify(ossimNotifyLevel_INFO)
               << "ossimPolyArea2d::isValid: " << topologyValidationError->toString() << std::endl;
         }
      }
      else
      {
         result = m_privateData->m_geometry->isValid();
      }
   }
   
   return result;
}

bool ossimPolyArea2d::isPointWithin(const ossimDpt& point)const
{
   return isPointWithin(point.x, point.y);
}

bool ossimPolyArea2d::isPointWithin(double x, double y)const
{
   bool result = false;

   if(!isEmpty())
   {
      geos::geom::Coordinate c(x,y);
      geos::geom::Geometry* geom = m_privateData->geomFactory()->createPoint(c);
  
      result = m_privateData->m_geometry->intersects(geom);

      delete geom;
   }

   return result;
}

void ossimPolyArea2d::getBoundingRect(ossimDrect& rect)
{
   rect.makeNan();

   if(!isEmpty())
   {
      const geos::geom::Envelope* envelope = m_privateData->m_geometry->getEnvelopeInternal();

      rect = ossimDrect(envelope->getMinX(), envelope->getMinY(), envelope->getMaxX(), envelope->getMaxY());
   }
}

std::string ossimPolyArea2d::toString()const
{
   std::string result = "";

   if(m_privateData->m_geometry)
   {
      result = m_privateData->m_geometry->toString();
   }

   return result;
}

ossimPolyArea2d ossimPolyArea2d::getBufferedShape(double distance) const{
	ossimPolyArea2d result;
	try{
		geos::operation::buffer::BufferOp buffer_operation(m_privateData->m_geometry);
		result.m_privateData->setGeometry( buffer_operation.getResultGeometry(distance));
	}catch( const std::exception& e ){
		ossimNotify(ossimNotifyLevel_DEBUG)
			<< "ossimPolyArea2d::getBufferedShape Caught exception: " << e.what() << std::endl;
		result.clearPolygons();
	}catch( ... ){
		ossimNotify(ossimNotifyLevel_DEBUG)
			<< "ossimPolyArea2d::getBufferedShape Caught exception!" << std::endl;
		result.clearPolygons();
	}
	return result;
}
ossimPolyArea2d& ossimPolyArea2d::setToBufferedShape(double distance)
{
   try{
      geos::operation::buffer::BufferOp buffer_operation(m_privateData->m_geometry);
      m_privateData->setGeometry( buffer_operation.getResultGeometry(distance));
   }catch( const std::exception& e ){
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimPolyArea2d::getBufferedShape Caught exception: " << e.what() << std::endl;
   }catch( ... ){
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimPolyArea2d::getBufferedShape Caught exception!" << std::endl;
   }
   return *this;
}

ossimPolyArea2d& ossimPolyArea2d::toMultiPolygon()
{


   try{
      if(m_privateData->m_geometry)
      {
         switch(m_privateData->m_geometry->getGeometryTypeId())
         {
            case geos::geom::GEOS_POLYGON:
            {
               std::vector<geos::geom::Geometry*> values;
               values.push_back(m_privateData->m_geometry->clone());

               m_privateData->setGeometry(m_privateData->m_geometry->getFactory()->createMultiPolygon(values));
               break;
            }
            case geos::geom::GEOS_MULTIPOLYGON:
            {
               // intentionally left blank
               break;
            }
            default:
            {  
               // might need an error at a later date
               ossimNotify(ossimNotifyLevel_WARN)
                  << "ossimPolyArea2d::toMultiPolygon Geometry type can not be converted to a multi polygon: " <<m_privateData->m_geometry->getGeometryType()<< std::endl;

               break;
            }
         }
      }
   }
   catch(const std::exception& e)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossimPolyArea2d::toMultiPolygon Caught exception: " << e.what() << std::endl;
   }
   catch(...)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossimPolyArea2d::toMultiPolygon Caught exception!" << std::endl;
   }

   return *this;
}

bool ossimPolyArea2d::saveState(ossimKeywordlist& kwl,
                                const char* prefix)const
{
   kwl.add(prefix,
           ossimKeywordNames::TYPE_KW,
           "ossimPolyArea2d",
           true);

   if(!isEmpty())
   {
      geos::io::WKTWriter writer;

      kwl.add(prefix,
              "wkt",
              writer.write(m_privateData->m_geometry).c_str(),
              true);
   }
   // else
   // {
   //
   // }

   return true;
}

bool ossimPolyArea2d::loadState(const ossimKeywordlist& kwl,
                                const char* prefix)
{
   if(m_privateData)
   {
      ossimString wkt = kwl.find(prefix, "wkt");

      if(!wkt.empty())
      {
         geos::io::WKTReader reader(m_privateData->geomFactory());
         try
         {
            m_privateData->setGeometry(reader.read(wkt.c_str()));
         }
         catch( const std::exception& e )
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "ossimPolyArea2d::loadState Caught exception: " << e.what() << std::endl;
            this->clearPolygons();
         }
         catch(...)
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "ossimPolyArea2d::loadState Caught exception!" << std::endl;
            this->clearPolygons(); 
         }
      }
   }
   return true;
}

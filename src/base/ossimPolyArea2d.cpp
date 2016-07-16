//---
// License:  See top level LICENSE.txt file.
//
// $Id: ossimPolyArea2d.cpp 23623 2015-11-13 18:24:28Z gpotts $
//---

#include <ossim/base/ossimPolyArea2d.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <cstdlib>
#include <exception>
#include <vector>

#include <geos_c.h>

class ossimGeometryFactoryWrapper : public ossimReferenced
{
public:
  ossimGeometryFactoryWrapper()
  {
   }

   virtual ~ossimGeometryFactoryWrapper(){
   }
};


class OssimPolyArea2dPrivate
{
public:
  typedef GEOSGeometry* GeometryPtr;
  typedef const GEOSGeometry* ConstGeometryPtr;

   OssimPolyArea2dPrivate(GeometryPtr geom=0);
   ~OssimPolyArea2dPrivate();
   
   void deleteGeometry() {
       if(m_geometry) {
	 GEOSFree(m_geometry);
	 m_geometry = 0;
       }
   }
   void setGeometry(const ossimPolygon& polygon, const vector<ossimPolygon>& holes = vector<ossimPolygon>());
  
   void setGeometry(GeometryPtr geom) {
     deleteGeometry();
     m_geometry=geom;
   }

  //     geos::geom::GeometryFactory* geomFactory(){{         return m_globalFactory.valid()?m_globalFactory->m_geomFactory:0; }}

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

void OssimPolyArea2dPrivate::setGeometry( const ossimPolygon& exteriorRing, const vector<ossimPolygon>& interiorRings )
{

  const std::vector<ossimDpt>& pts = exteriorRing.getVertexList();   
  int idx;
  int n = static_cast<int> (pts.size());
  
  if(n > 0)
    {
      GEOSCoordSequence* csExteriorRing = GEOSCoordSeq_create(n, 2);
      //fill the exterior ring
      for (idx = 0; idx < n; idx++)
	{
	  GEOSCoordSeq_setY(csExteriorRing, idx, pts[idx].y);
	  GEOSCoordSeq_setX(csExteriorRing, idx, pts[idx].x);
	}
      
      //if the original polygon didn't have the first and last point the same, make it so
      if((pts[0].x != pts[n-1].x) || (pts[0].y!=pts[n-1].y))
	{
	  GEOSCoordSeq_setY(csExteriorRing, idx, pts[0].y);
	  GEOSCoordSeq_setX(csExteriorRing, idx, pts[0].x);
	}

      //fill the interior rings
      int numHoles =  static_cast<int> (interiorRings.size());
      GEOSGeometry *holes[ numHoles ];

      for (ossim_uint32 interiorRingIdx = 0; interiorRingIdx < numHoles ; ++interiorRingIdx)
	{
	  
	  const std::vector<ossimDpt>& vertexPts = interiorRings[interiorRingIdx].getVertexList();
	  
	  GEOSCoordSequence* interiorCas = GEOSCoordSeq_create(vertexPts.size(), 2);
	  
	  ossim_uint32 vertexIndex;
	  for(vertexIndex = 0; vertexIndex < vertexPts.size(); ++vertexIndex)
	    {
	      GEOSCoordSeq_setY(interiorCas, vertexIndex, (vertexPts[vertexIndex].y));
	      GEOSCoordSeq_setX(interiorCas, vertexIndex, (vertexPts[vertexIndex].x));
	    }
	  
	  //if the original polygon didn't have the first and last point the same, make it so
	  if( ( vertexPts[0].x != vertexPts[vertexPts.size()-1].x ) ||
	      ( vertexPts[0].y!=vertexPts[vertexPts.size()-1].y) )
	    {
	      GEOSCoordSeq_setY( interiorCas, vertexIndex, (vertexPts[0].y) );
	      GEOSCoordSeq_setX( interiorCas, vertexIndex, (vertexPts[0].x) );
	    }
	  
	  GEOSGeometry* hole_;
	  hole = GEOSGeom_createLinearRing( interiorCas );
	  holes[interiorRingIdx] = hole;
	}

      GEOSGeometry* shell = GEOSGeom_createLinearRing( csExteriorRing );
      
      m_geometry = GEOSGeom_createPolygon( shell, holes, interiorRings.size() );
   }
}

void ossimPolyArea2d::recurseVisibleGeometries(ossimPolygon::Vector& polyList,
			      const GEOSGeometry* geom) const
{  
  int nGeoms =  GEOSGetNumGeometries(geom);
  
  if(nGeoms < 2 )
    {
      if (geom)
	{
	  const GEOSGeometry* lineString = GEOSGetExteriorRing(geom);
	  if (lineString)
	    {	   
	      int currentPolyIdx = (int)polyList.size();
	      
	      int nPoints = GEOSGeomGetNumPoints(lineString);
      
	      polyList.push_back(ossimPolygon());
	      
	      for (int idx = 0; idx < nPoints; idx++)
		{
		  GEOSGeometry *point =  GEOSGeomGetPointN( lineString, idx );
		  double x, y;
		  GEOSGeomGetX( point, &x );
		  GEOSGeomGetY( point, &y );	  
		  polyList[currentPolyIdx].addPoint(x, y);
		}
	    }
	}
    }
  else
    {
      for (int idx=0; idx < nGeoms; ++idx) {
	recurseVisibleGeometries(polyList, GEOSGetGeometryN(geom, idx));
      }
    }
}
 
void ossimPolyArea2d::recurseHoles(std::vector<ossimPolygon>& polyList,
                                   const GEOSGeometry* geom) const
{
    int nGeoms =  GEOSGetNumGeometries(geom);
   
   if(nGeoms < 2 )
     {
       if (geom)
	 {
	   ossim_uint32 nInteriorRings = (ossim_uint32) GEOSGetNumInteriorRings(geom);
	   
	   ossim_uint32 idx;
	   
	   for(ossim_uint32 idx = 0; idx < nInteriorRings; ++idx)
	     {

	       const GEOSGeometry* lineString = GEOSGetInteriorRingN(geom, idx);
	       if (lineString)
		 {
		   int currentPolyIdx = static_cast<int>( polyList.size() );
		   
		   int numPoints = GEOSGeomGetNumPoints(lineString);
		   
		   polyList.push_back(ossimPolygon());
		   
		   for (int idx = 0; idx < numPoints ; idx++)
		     {
		       GEOSGeometry *point =  GEOSGeomGetPointN(lineString, idx);
		       double x, y;
		       GEOSGeomGetX(point, &x);
		       GEOSGeomGetY(point, &y);	  
		       polyList[currentPolyIdx].addPoint(x, y);
		     }
		 }
	     }
	 }
     }
   else
     {
       for (int idx = 0; idx < nGeoms; idx++) {
	 recurseHoles(polyList, GEOSGetGeometryN(geom, idx));
       }
     }
}
void ossimPolyArea2d::recurseCompleteGeometries(std::vector<ossimPolyArea2d>& polyList,
                                                const GEOSGeometry* geom) const
{

  int nGeoms =  GEOSGetNumGeometries(geom);
  
  if(nGeoms < 2 )
    {
      
      if (geom)
	{
	  //get exterior shell for the geometry
	  ossimPolygon shell;
	  
	  const GEOSGeometry* lineString = GEOSGetExteriorRing(geom);
	  if (lineString)
	    {
	      
	      int nPoints = GEOSGeomGetNumPoints(lineString);
	      for (int idx = 0; idx<nPoints; idx++)
		{
		  
		  GEOSGeometry *point =  GEOSGeomGetPointN(lineString, idx);
		  double x, y;
		  GEOSGeomGetX(point, &x);
		  GEOSGeomGetY(point, &y);	  
		  shell.addPoint(x, y);
		}
	    } //end if (linestring)

	  // Get interior rings for the geometry.
	  std::size_t nInteriorRings = GEOSGetNumInteriorRings(geom);
	  vector<ossimPolygon> holes(nInteriorRings);
	  for(std::size_t holeIdx = 0; holeIdx < nInteriorRings; ++holeIdx)
	    {

	      const GEOSGeometry* lineString = GEOSGetInteriorRingN(geom, holeIdx);
	      
	      if (lineString)
		{		  
		  std::size_t nPoints = GEOSGeomGetNumPoints(lineString);
 
		  for (std::size_t idx = 0; idx<nPoints; ++idx)
		    {
		      GEOSGeometry *point =  GEOSGeomGetPointN(lineString, idx);
		      double x, y;
		      GEOSGeomGetX(point, &x);
		      GEOSGeomGetY(point, &y);	  
		      holes[holeIdx].addPoint(x, y);
		    }
		}
	    }
	  polyList.push_back(ossimPolyArea2d(shell, holes));
	}
    }
  else
    {
      
      
      for ( int idx = 0; idx < nGeoms; idx++)
	{
	  recurseCompleteGeometries(polyList, GEOSGetGeometryN(geom, idx)); 
	}
    }
}

std::ostream& operator <<(std::ostream& out, const ossimPolyArea2d& rhs)
{
  out << rhs.toString();
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
   :m_privateData(new OssimPolyArea2dPrivate)
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
	  m_privateData->setGeometry( GEOSGeom_clone(rhs.m_privateData->m_geometry) );
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
   
   int n = (int)polygon.size();
   for(int idx = 0; idx < n;++idx)
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
     if( GEOSIntersection ( m_privateData->m_geometry, rhs.m_privateData->m_geometry) ) { result = true; }
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
	  result.m_privateData->setGeometry( GEOSIntersection ( m_privateData->m_geometry, rhs.m_privateData->m_geometry) ) ; 
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
	  result.m_privateData->setGeometry(GEOSUnion ( m_privateData->m_geometry, rhs.m_privateData->m_geometry)) ;
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
	  result.m_privateData->setGeometry( GEOSDifference( m_privateData->m_geometry,
							     rhs.m_privateData->m_geometry) );
	
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
	  m_privateData->setGeometry(GEOSIntersection( m_privateData->m_geometry, rhs.m_privateData->m_geometry));
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
	  m_privateData->setGeometry(GEOSUnion(m_privateData->m_geometry, rhs.m_privateData->m_geometry));
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
	m_privateData->setGeometry(GEOSDifference(m_privateData->m_geometry,
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
  GEOSGeometry* geom = GEOSUnion(m_privateData->m_geometry, rhs.m_privateData->m_geometry);
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

     result = GEOSisEmpty(m_privateData->m_geometry);
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

 char *topologyValidationError = NULL;
 topologyValidationError = GEOSisValidReason(m_privateData->m_geometry);
 if(topologyValidationError == NULL)
    {
      result = true;
    }
  else
    {
      ossimNotify(ossimNotifyLevel_INFO)
	<< "ossimPolyArea2d::isValid: " << std::string(topologyValidationError) << std::endl;
    } 	    

      }
      else
      {

	 result = GEOSisValid(m_privateData->m_geometry);

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
	    GEOSCoordSequence* cspoint_;
	    cspoint_ = GEOSCoordSeq_create(1, 2);
	    GEOSCoordSeq_setY(cspoint_, 0, y);
	    GEOSCoordSeq_setX(cspoint_, 0, x);
	    GEOSGeometry* geom = GEOSGeom_createPoint(cspoint_);
	    result = GEOSIntersects( m_privateData->m_geometry, geom);
	    GEOSFree(geom);
	    geom = NULL;
      

   }

   return result;
}

void ossimPolyArea2d::getBoundingRect(ossimDrect& rect)
{
   rect.makeNan();

   if(!isEmpty())
   {
     
// GEOSGeometry *envelope =  GEOSEnvelope(m_privateData->m_geometry);
   const GEOSGeometry *c = GEOSGeomGetPointN(m_privateData->m_geometry, 0);
   double cx, cy;
   GEOSGeomGetX(c, &cx);
   GEOSGeomGetY(c, &cy);
   double minx = cx;
   double miny = cy;
   double maxx = cx;
   double maxy = cy;
   
   int npts =  GEOSGeomGetNumPoints( m_privateData->m_geometry );
   
   for (int i=1; i<npts; i++) {
     const GEOSGeometry *c = GEOSGeomGetPointN(m_privateData->m_geometry, i);
     double cx2, cy2;
     GEOSGeomGetX(c, &cx2);
     GEOSGeomGetY(c, &cy2);		
     minx = minx < cx2 ? minx : cx2;
     maxx = maxx > cx2 ? maxx : cx2;
     miny = miny < cy2 ? miny : cy2;
     maxy = maxy > cy2 ? maxy : cy2;
   }
   rect = ossimDrect(minx, miny, maxx, maxy);

   }
}
 
std::string ossimPolyArea2d::toString()const
{
   std::string result = "";

   if(m_privateData->m_geometry)
   {

  GEOSWKTWriter *writer = GEOSWKTWriter_create();
  result = std::string(GEOSWKTWriter_write(writer, m_privateData->m_geometry));

   }

   return result;
}


ossimPolyArea2d ossimPolyArea2d::getBufferedShape(double distance) const{
	ossimPolyArea2d result;
	try{

  result.m_privateData->setGeometry( GEOSBuffer(m_privateData->m_geometry, distance, 30) );
  
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

  m_privateData->setGeometry( GEOSBuffer(m_privateData->m_geometry, distance, 30) );

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
	switch( GEOSGeomTypeId(m_privateData->m_geometry) ) 
         {
            case GEOS_POLYGON:
            {

	      GEOSGeometry *values[1];
	      values[0] = GEOSGeom_clone( m_privateData->m_geometry );
	      GEOSGeometry * multipolygon =
		GEOSGeom_createCollection(GEOS_MULTIPOLYGON, values, 1);
	      m_privateData->setGeometry(multipolygon);

               break;
            }
            case GEOS_MULTIPOLYGON:
            {
               // intentionally left blank
               break;
            }
            default:
            {  
               // might need an error at a later date
               ossimNotify(ossimNotifyLevel_WARN)
		 << "ossimPolyArea2d::toMultiPolygon Geometry type can not be converted to a multi polygon: "
		 << GEOSGeomType(m_privateData->m_geometry) << std::endl;

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
     GEOSWKTWriter *writer = GEOSWKTWriter_create();
     kwl.add(prefix,
	     "wkt",
	     GEOSWKTWriter_write(writer,m_privateData->m_geometry),
	     true);
   }

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
	GEOSWKTReader* reader_;
	reader_ = GEOSWKTReader_create();
         try
         {
	   
	   m_privateData->setGeometry( GEOSWKTReader_read( reader_, wkt.c_str() )  );
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

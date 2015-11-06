//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//*******************************************************************
//$Id: ossimPolyArea2d.h 23608 2015-10-28 13:51:35Z gpotts $

#ifndef ossimPolyArea2d_HEADER
#define ossimPolyArea2d_HEADER 1

#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimPolygon.h>
#include <ossim/base/ossimReferenced.h>
#include <vector>

class OssimPolyArea2dPrivate;

namespace geos
{
   namespace geom
   {
      class Geometry;
   }
}

class OSSIM_DLL ossimPolyArea2d : public ossimReferenced
{
public:
   friend class OssimPolyArea2dPrivate;
   friend OSSIM_DLL std::ostream& operator <<(std::ostream& out, const ossimPolyArea2d& data);
   
   ossimPolyArea2d();
   ossimPolyArea2d(const vector<ossimGpt>& polygon);
   ossimPolyArea2d(const ossimPolygon& shell, const vector<ossimPolygon>& holes); 
   
   ossimPolyArea2d(const ossimDpt& p1,
                   const ossimDpt& p2,
                   const ossimDpt& p3,
                   const ossimDpt& p4);
   ossimPolyArea2d(const ossimPolyArea2d& rhs);
   
   ossimPolyArea2d(const ossimIrect& rect);
   ossimPolyArea2d(const ossimDrect& rect);
   ossimPolyArea2d(const ossimPolygon& polygon);
   ~ossimPolyArea2d();
   
   void clear()
   {
      clearPolygons();
   }
   const ossimPolyArea2d& operator =(const ossimPolyArea2d& rhs);
   const ossimPolyArea2d& operator =(const ossimPolygon& rhs);
   const ossimPolyArea2d& operator =(const ossimIrect& rect);
   const ossimPolyArea2d& operator =(const ossimDrect& rect);
   const ossimPolyArea2d& operator =(const vector<ossimGpt>& polygon);
   const ossimPolyArea2d& operator =(const vector<ossimDpt>& polygon);
   const ossimPolyArea2d& operator &=(const ossimPolyArea2d& rhs);
   ossimPolyArea2d operator &(const ossimPolyArea2d& rhs)const;
   ossimPolyArea2d operator +(const ossimPolyArea2d& rhs)const;
   const ossimPolyArea2d& operator +=(const ossimPolyArea2d& rhs);
   ossimPolyArea2d operator -(const ossimPolyArea2d& rhs)const;
   const ossimPolyArea2d& operator -=(const ossimPolyArea2d& rhs);
   
   bool intersects(const ossimPolyArea2d& rhs)const;
   
   void add(const ossimPolyArea2d& rhs);
   bool getVisiblePolygons(vector<ossimPolygon>& polyList)const;
   bool getPolygonHoles(vector<ossimPolygon>& polyList)const;
   
   /**
    * @brief Gets all of the polygons stored with their holes embedded. This
    * may be useful if an operation was performed on the original ossimPolyArea2d 
    * that caused multiple polygons to be created internally.
    *	 
    * For example, if a rectangle is intersected with a U shape, the two top
    * portions of the U would be their own separate polygon. It's also possible 
    * for these polygons to contain their own holes. This function will return 
    * the two top polygons as separate ossimPolyArea2d objects (with any of 
    * their holes embedded inside them).
    *
    *  --------------------------------		 
    * |                                |
    * |                                |
    * | ........           .........   |
    * | .      .           .       .   |
    *  -.------.-----------.-------.---
    *   .      .           .       .
    *   .      .............       .
    *   .                          .
    *   ............................
    *   
    * @param polylist an empty vector of ossimPolyArea2d that will be filled
    * @return returns true if it successfully places polygons in the input vector
    */
   bool getCompletePolygons(vector<ossimPolyArea2d>& polyList)const;
   
   bool isEmpty()const;
   bool isValid(bool displayValidationError = false)const;
   bool isPointWithin(const ossimDpt& point)const;
   bool isPointWithin(double x, double y)const;
   void getBoundingRect(ossimDrect& rect);
   
   /**
   * Returns the Well Known Text string
   */
   std::string toString()const;

   /**
    * @brief Buffers the ossimPolyArea2d shape and returns a copy. This method
    * does not alter polygon.
    *  
    * @param distance is the distance to buffer the shape by. Positive values
    * will expand the shape, and negative values will shrink the shape.
    * @return A shape that is a buffered (expanded/contracted) version of this
    * shape
    */
   ossimPolyArea2d getBufferedShape(double distance=FLT_EPSILON) const;
   
   ossimPolyArea2d& setToBufferedShape(double distance=FLT_EPSILON);

   ossimPolyArea2d& toMultiPolygon();
   bool saveState(ossimKeywordlist& kwl,
                  const char* prefix=0)const;
   bool loadState(const ossimKeywordlist& kwl,
                  const char* prefix=0);
   
protected:
   
   void clearPolygons();
   void recurseVisibleGeometries(ossimPolygon::Vector& polyList,
                                 const geos::geom::Geometry* geom) const;
   
   void recurseHoles(ossimPolygon::Vector& polyList,
                     const geos::geom::Geometry* geom) const;
   
   /**
    * @brief Recurses over the Geometry object to load all complete polygons
    * (a shell and any internal holes) into the ossimPolyArea2d.
    */
   void recurseCompleteGeometries(std::vector<ossimPolyArea2d>& polyList,
                                  const geos::geom::Geometry* geom) const;
   
   OssimPolyArea2dPrivate* m_privateData;
};

#endif /* #ifndef ossimPolyArea2d_HEADER */

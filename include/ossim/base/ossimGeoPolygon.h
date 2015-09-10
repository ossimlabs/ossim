//*****************************************************************************
// FILE: ossimPolygon.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Garrett Potts
//
//*****************************************************************************
//  $Id: ossimGeoPolygon.h 23166 2015-02-24 20:57:50Z dburken $
#ifndef ossimGeoPolygon_HEADER
#define ossimGeoPolygon_HEADER
#include <vector>
#include <ossim/base/ossimGpt.h>

class ossimKeywordlist;

class OSSIMDLLEXPORT ossimGeoPolygon
{
public:
   friend OSSIM_DLL std::ostream& operator <<(std::ostream& out, const ossimGeoPolygon& poly);

   ossimGeoPolygon();

   ossimGeoPolygon(const std::vector<ossimGpt>& points);

   ossimGeoPolygon(const ossimGeoPolygon& rhs);

    bool addWmsBbox(const ossimString& wmsBbox);  
    void addPoint(const ossimGpt& pt)
      {
         theVertexList.push_back(pt);
      }
   void addPoint(double lat, double lon, double h=ossim::nan(), const ossimDatum* datum=0)
      {
         theVertexList.push_back(ossimGpt(lat, lon, h, datum));
      }
   void addAttribute( const ossimString& attribute )
     {
       theAttributeList.push_back( attribute );
     }
   void addHole( const ossimGeoPolygon& polygon )
     {
       theHoleList.push_back( polygon );
     }
   ossimGpt& operator[](int index)
      {
         return theVertexList[index];
      }
   const ossimGpt& operator[](int index)const
      {
         return theVertexList[index];
      }
   
   const std::vector<ossimGpt>& getVertexList()const
      {
         return theVertexList;
      }
   std::vector<ossimString>& getAttributeList()
      {
         return theAttributeList;
      }
   std::vector<ossimGeoPolygon>& getHoleList()
      {
         return theHoleList;
      }

   void clear()
      {
         theVertexList.clear();
      }
   ossim_uint32 size()const
      {
         return (ossim_uint32)theVertexList.size();
      }
   void resize(ossim_uint32 newSize)
      {
         theVertexList.resize(newSize);
         theCurrentVertex = 0;
         theOrderingType  = OSSIM_VERTEX_ORDER_UNKNOWN;
      }
   const ossimGeoPolygon& operator = (const std::vector<ossimGpt>& rhs)
      {
         theVertexList = rhs;
         theCurrentVertex = 0;
         theOrderingType = OSSIM_VERTEX_ORDER_UNKNOWN;

         return *this;
      }

   const ossimGeoPolygon& operator = (const ossimGeoPolygon& rhs);

   void stretchOut(ossimGeoPolygon& newPolygon,
                   double displacement);
   double area()const;

   ossimGpt computeCentroid()const;
   
   bool vertex(int index, ossimGpt& v) const;
   bool nextVertex(ossimDpt& v) const;
   bool hasNans()const;
   void reverseOrder();
   
   bool saveState(ossimKeywordlist& kwl,
                  const char* prefix=0)const;
   
   bool loadState(const ossimKeywordlist& kwl,
                  const char* prefix=0);
    
   void checkOrdering()const;
   ossimVertexOrdering getOrdering()const
      {
         if(theOrderingType == OSSIM_VERTEX_ORDER_UNKNOWN)
         {
            checkOrdering();
         }
         return theOrderingType;
      }
   void setOrdering(ossimVertexOrdering ordering)
      {
         theOrderingType = ordering;
      }
protected:
   std::vector<ossimGpt> theVertexList;
   std::vector<ossimString> theAttributeList;
   std::vector<ossimGeoPolygon> theHoleList;
   mutable ossim_int32 theCurrentVertex;
   
   /*!
    * This enumeration is found in ossimConstants.h
    */
   mutable ossimVertexOrdering theOrderingType;
   
};

#endif

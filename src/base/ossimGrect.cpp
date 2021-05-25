//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: ossimGrect.cpp 23461 2015-08-05 20:20:20Z okramer $

#include <ossim/base/ossimGrect.h>
using namespace std;

ostream& operator<<(ostream& os, const ossimGrect& rect)
{
   return os << rect.theUlCorner << ", " << rect.theLrCorner << endl;
}

ossimGrect::ossimGrect(vector<ossimGpt>& points)
   :
   theUlCorner(0.0, 0.0, 0.0),
   theLrCorner(0.0, 0.0, 0.0)
{
   // initialize everyone to the first point
   if(points.size() > 0)
   {
      double minHgt = 0.0;
      double maxHgt = 0.0;
      theUlCorner = points[0];
      theLrCorner = theUlCorner;
      minHgt = theUlCorner.hgt;
      maxHgt = minHgt;
      
      // find the bounds
      for(ossim_uint32 index = 1; index < points.size(); index++)
      {
         // find left most
         if(theUlCorner.lond() > points[index].lond())
         {
            theUlCorner.lond(points[index].lond());
         } // find right most
         else if(theLrCorner.lond() < points[index].lond())
         {
            theLrCorner.lond(points[index].lond());
         }
         //find top most
         if(points[index].latd() > theUlCorner.latd())
         {
            theUlCorner.latd(points[index].latd());
         }// find bottom most
         else if(points[index].latd() < theLrCorner.latd())
         {
            theLrCorner.latd(points[index].latd());
         }
         
         if (points[index].hgt > maxHgt)
         {
            maxHgt = points[index].hgt;
         }
         else if (points[index].hgt < minHgt)
         {
            minHgt = points[index].hgt;
         }
      }
      
      theUlCorner.hgt = maxHgt;
      theLrCorner.hgt = minHgt;
   }
}

ossimGrect::ossimGrect(const ossimGpt& p1, const ossimGpt& p2)
{
   if(p1.lon < p2.lon)
   {
      theUlCorner.lon = p1.lon;
      theLrCorner.lon = p2.lon;
   }
   else
   {
      theUlCorner.lon = p2.lon;
      theLrCorner.lon = p1.lon;
   }

   if(p1.lat > p2.lat)
   {
      theUlCorner.lat = p1.lat;
      theLrCorner.lat = p2.lat;
   }
   else
   {
      theUlCorner.lat = p2.lat;
      theLrCorner.lat = p1.lat;
   }

   if(p1.hgt > p2.hgt)
   {
      theUlCorner.hgt = p1.hgt;
      theLrCorner.hgt = p2.hgt;
   }
   else
   {
      theUlCorner.hgt = p2.hgt;
      theLrCorner.hgt = p1.hgt;
   }
}

ossimGrect::ossimGrect(const ossimGpt& p1,
                       const ossimGpt& p2,
                       const ossimGpt& p3,
                       const ossimGpt& p4)
{
   unsigned long index;
   double minHgt, maxHgt;

   std::vector<ossimGpt> points(4);
   points[0] = p1;
   points[1] = p2;
   points[2] = p3;
   points[3] = p4;

   // initialize everyone to the first point
   theUlCorner = points[0];
   theLrCorner = theUlCorner;
   minHgt = theUlCorner.hgt;
   maxHgt = minHgt;
   
   // find the bounds
   for(index = 1; index < points.size(); index++)
   {
      // find left most
      if(theUlCorner.lond() > points[index].lond())
      {
         theUlCorner.lond(points[index].lond());
      } // find right most
      else if(theLrCorner.lond() < points[index].lond())
      {
         theLrCorner.lond(points[index].lond());
      }
      //find top most
      if(points[index].latd() > theUlCorner.latd())
      {
         theUlCorner.latd(points[index].latd());
      }// find bottom most
      else if(points[index].latd() < theLrCorner.latd())
      {
         theLrCorner.latd(points[index].latd());
      }

      if (points[index].hgt > maxHgt)
         maxHgt = points[index].hgt;
      else if (points[index].hgt < minHgt)
         minHgt = points[index].hgt;
   }
   theUlCorner.hgt = maxHgt;
   theLrCorner.hgt = minHgt;
}

ossimGrect ossimGrect::stretchToEvenBoundary(double latSpacingInDegrees,
                                             double lonSpacingInDegrees)const
{
   double ulLat = ((long)ceil(theUlCorner.latd()/latSpacingInDegrees))*
                  latSpacingInDegrees;
   double ulLon = ((long)floor(theUlCorner.lond()/lonSpacingInDegrees))*
                  lonSpacingInDegrees;
   double lrLat = ((long)floor(theLrCorner.latd()/latSpacingInDegrees))*
                  latSpacingInDegrees;
   double lrLon = ((long)ceil(theLrCorner.lond()/lonSpacingInDegrees))*
                  lonSpacingInDegrees;
   
   ossimGpt ul (ulLat, ulLon, theUlCorner.hgt, theUlCorner.datum());
   ossimGpt lr (lrLat, lrLon, theLrCorner.hgt, theLrCorner.datum());

   return ossimGrect(ul, lr);
}

void ossimGrect::computeEvenTiles(std::vector<ossimGrect>& result,
                                  double latSpacingInDegrees,
                                  double lonSpacingInDegrees,
                                  bool clipToGeographicBounds)const
{
   ossimGrect clipRect = ossimGrect(90, -180, -90, 180);
   result.clear();
   ossimGrect temp = stretchToEvenBoundary(latSpacingInDegrees,
                                           lonSpacingInDegrees);

   ossimGpt point = temp.ul();
   
   while(temp.pointWithin(point))
   {
      while(temp.pointWithin(point))
      {
         ossimGrect rect(point.latd(),
                         point.lond(),
                         point.latd()-latSpacingInDegrees,
                         point.lond()+lonSpacingInDegrees);

         rect.theUlCorner.datum( theUlCorner.datum());
         rect.theLrCorner.datum( theUlCorner.datum());
         if(clipToGeographicBounds)
         {
            rect = rect.clipToRect(clipRect);
         }
         result.push_back(rect);

         point.lond(point.lond()+lonSpacingInDegrees);
      }
      point.lond(temp.ul().lond());
      point.latd(point.latd()-latSpacingInDegrees);
   }
}

//*******************************************************************
// Public Method: ossimGrect::completely_within
//*******************************************************************
bool ossimGrect::completely_within(const ossimGrect& rect) const
{
   if(rect.isLonLatNan() || isLonLatNan())
   {
      return false;
   }
   
   /*  --------------
       |     1      |
       | ---------- |
       | |        | |
       | |        | |
       | |   2    | |
       | |        | |
       | |        | |
       | ---------- |
       |            |
       --------------  */

   bool rtn = true;
   
   if ((theUlCorner.lon < rect.ul().lon)||
       (theUlCorner.lon > rect.ur().lon))
   {
      rtn = false;
   }
   else if ((theLrCorner.lon > rect.lr().lon)||
            (theLrCorner.lon < rect.ul().lon))
   {
      rtn = false;
   }
   else if ((theUlCorner.lat > rect.ul().lat)||
            (theUlCorner.lat < rect.lr().lat))
   {
      rtn = false;
   }
   else if ((theLrCorner.lat < rect.lr().lat)||
            (theLrCorner.lat > rect.ul().lat))
   {
      rtn = false;
   }

   return rtn;
}

//*******************************************************************
// Public Method: ossimGrect::intersects
//*******************************************************************
bool ossimGrect::intersects(const ossimGrect& rect) const
{
		
   if(rect.isLonLatNan() || isLonLatNan())
   {
      return false;
   }
   
   ossim_float64  ulx = ossim::max(rect.ul().lon, ul().lon);
   ossim_float64  lrx = ossim::min(rect.lr().lon, lr().lon);
   ossim_float64  uly, lry;
   bool rtn;
   
   uly  = ossim::min(rect.ul().lat, ul().lat);
   lry  = ossim::max(rect.lr().lat, lr().lat);
   rtn = ((ulx <= lrx) && (uly >= lry));
   
      return (rtn);
}

ossim_float64 ossimGrect::heightMeters() const
{
   ossimDpt scale (midPoint().metersPerDegree());
   return height()*scale.y;
}

ossim_float64 ossimGrect::widthMeters()  const
{
   ossimDpt scale (midPoint().metersPerDegree());
   return width()*scale.x;
}


void ossimGrect::expandToInclude(const ossimGpt& gpt)
{
   if (isLonLatNan())
   {
      theUlCorner = gpt;
      theLrCorner = gpt;
   }
   else if (!pointWithin(gpt, false))
   {
      if (gpt.lat > theUlCorner.lat)
         theUlCorner.lat = gpt.lat;
      else if (gpt.lat < theLrCorner.lat)
         theLrCorner.lat = gpt.lat;
      if (gpt.lon < theUlCorner.lon)
         theUlCorner.lon = gpt.lon;
      else if (gpt.lon > theLrCorner.lon)
         theLrCorner.lon = gpt.lon;
   }
}

void ossimGrect::expandToInclude(const ossimGrect& rect)
{
   expandToInclude(rect.ul());
   expandToInclude(rect.lr());
}

std::string ossimGrect::toString(ossim_uint32 precision) const
{
   std::string result = theUlCorner.toString( precision ).string();
   result += ",";
   result += theLrCorner.toString( precision ).string();
   return result;
}

bool ossimGrect::toRect(const std::string& s)
{
   bool result = false;

   // String example: (41,-105,0,WGE),(40,-104,0,WGE)
   
   // Get the upper left:
   std::string ss = ",(";
   std::string::size_type found = s.find(ss);
   if ( found != std::string::npos )
   {
      // cout << "s.substr( 0, found ): " <<  s.substr( 0, found ) << "\n";
      theUlCorner.toPoint( s.substr( 0, found ) );

      // Get the lower right:
      ss = "),";
      std::string::size_type found = s.find(ss);
      if ( found != std::string::npos )
      {
         if ( found+2 < s.size() )
         {
            // cout << "s.substr( found+2, s.size()-(found+2) ): "
            //      << s.substr( found+2, s.size()-(found+2) ) << "\n";
            theLrCorner.toPoint( s.substr(found+2, s.size()-(found+2)) );
            result = true;
         }
      }
   }

   return result;
}

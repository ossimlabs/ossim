//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: ossimGrect.h 23461 2015-08-05 20:20:20Z okramer $
#ifndef ossimGrect_HEADER
#define ossimGrect_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimDatumFactory.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimCommon.h>

#include <vector>
#include <iostream>

class OSSIM_DLL ossimGrect
{
public:
   friend OSSIM_DLL std::ostream& operator<<(std::ostream& os, const ossimGrect& rect);

   /**
    * Will default to 0,0,0,0.
    */
    ossimGrect()
      :
         theUlCorner(0.0, 0.0, 0.0),
         theLrCorner(0.0, 0.0, 0.0)
      {}

   /**
    * Copies the passed in rectangle to this
    * object.
    */
   ossimGrect(const ossimGrect& rect)
      :
         theUlCorner(rect.ul()),
         theLrCorner(rect.lr())
      {}

   /**
    * WIll take two ground points and fill the
    * bounding rect appropriately.
    */
   ossimGrect(const ossimGpt& p1, const ossimGpt& p2);

   /**
    * Takes the upper left and lower right ground
    * points
    */
   ossimGrect(double ulLat,
              double ulLon,
              double lrLat,
              double lrLon,
              const ossimDatum* aDatum=ossimDatumFactory::instance()->wgs84())
      : 
         theUlCorner(ulLat, ulLon,0, aDatum),
         theLrCorner(lrLat, lrLon, 0, aDatum)
      {}
   ossimGrect(const ossimGpt& point,
              double latSpacingInDegrees,
              double lonSpacingInDegrees)
      :
         theUlCorner(point),
         theLrCorner(point)
      {
         std::vector<ossimGrect> v;
         computeEvenTiles(v, latSpacingInDegrees, lonSpacingInDegrees);
         if(v.size())
            *this = v[0];
      }
   ossimGrect(std::vector<ossimGpt>& points);
   ossimGrect(const ossimGpt& p1,
              const ossimGpt& p2,
              const ossimGpt& p3,
              const ossimGpt& p4);
              

   const ossimGrect& operator=(const ossimGrect& rect)
      {
         theUlCorner = rect.ul();
         theLrCorner = rect.lr();
         return *this;
      }

   inline ossimGpt midPoint()const;
   
   /** Returns the height of a rectangle in deg. */
   inline ossim_float64 height() const;

   /** Returns the width of a rectangle in deg. */
   inline ossim_float64 width()  const;
   
   /** Returns the height of a rectangle in meters. */
   ossim_float64 heightMeters() const;

   /** Returns the width of a rectangle in meters using the center lat for scaling EW direction. */
   ossim_float64 widthMeters()  const;

   inline const ossimGpt& ul()const;
   inline const ossimGpt& lr()const;
   inline ossimGpt ur() const;
   inline ossimGpt ll() const;

   inline ossimGpt& ul();
   inline ossimGpt& lr();
	
   inline void makeNan();
   
   inline bool isLonLatNan()const;
	
   inline bool hasNans()const;
   
   inline bool isNan()const;
   
   /*!
    * Returns true if "this" rectangle is contained completely within the
    * input rectangle "rect".
    */
   bool completely_within(const ossimGrect& rect) const;
	
   /*!
    * Returns true if any portion of an input rectangle "rect" intersects
    * "this" rectangle.  
    */
   bool intersects(const ossimGrect& rect) const;

   inline ossimGrect clipToRect(const ossimGrect& rect)const;

   inline ossimGrect combine(const ossimGrect& rect)const;

   /**
    * Expands existing rect to accommodate argument point. If this object contains NaNs
    * (uninitialized), then the rect is set to 0-area with the arg point as both UL and LR --
    * convenient for establishing bounds in a loop over collection of points. This method does not
    * consider height.
    * @param gpt Argument point.
    */
   void expandToInclude(const ossimGpt& gpt);
   void expandToInclude(const ossimGrect& rect);

   /**
    * METHOD: pointWithin(ossimGpt)
    *
    * @param gpt Point to test for withinness.
    * @param considerHgt true if the height is to be compared to be between ul.hgt and lr.hgt
    * @return true if argument is inside rectangle (or cuboid when hgt considered)
    *
    * @note There is no datum shift applied if
    * gpt is of a different datum than this datum.
    */
   inline bool pointWithin(const ossimGpt& gpt, bool considerHgt=false) const; //inline below

   ossimGrect stretchToEvenBoundary(double latSpacingInDegrees,
                                    double lonSpacingInDegrees)const;
   
   void computeEvenTiles(std::vector<ossimGrect>& result,
                         double latSpacingInDegrees,
                         double lonSpacingInDegrees,
                         bool clipToGeographicBounds = true)const;

   /**
    * @brief Get the rect as a string.
    *
    * Output format is two ossimGpts separated by a comma:
    * (ul),(lr)
    * ( 30.0, -90.0, 0.0, WGE ),( 29.0, -89.0, 0.0, WGE )
    * (  lat,  lon, hgt, datum),( lat, lon, hgt, datum),
    */
   std::string toString(ossim_uint32 precision=15) const;
   
   /**
    * Initializes this rect from string.
    *
    * Expected input string format:
    * (ul),(lr)
    * ( 30.0, -90.0, 0.0, WGE ),( 29.0, -89.0, 0.0, WGE )
    * ( lat,   lon,  hgt, datum),( lat,  lon,  hgt, datum)
    */
   bool toRect(const std::string& s);

private:
   ossimGpt theUlCorner; // Contains max height as well
   ossimGpt theLrCorner; // Contains min height as well
  
};

//==================== BEGIN INLINE DEFINITIONS ===============================

//*****************************************************************************
//  INLINE METHOD: ossimGrect::midPoint()
//*****************************************************************************
inline ossimGpt ossimGrect::midPoint()const
{
   return ossimGpt((ul().latd() + ur().latd() + ll().latd() + lr().latd())*.25,
                   (ul().lond() + ur().lond() + ll().lond() + lr().lond())*.25,
                   (ul().height()+ur().height()+ll().height()+
                    lr().height())*.25,
                   ul().datum() );
}

//*****************************************************************************
//  INLINE METHOD: ossimGrect::clipToRect()
//*****************************************************************************
inline ossimGrect ossimGrect::clipToRect(const ossimGrect& rect)const
{
    double     ulx, uly, lrx, lry;

    ulx = ossim::max<ossim_float64>(rect.ul().lond(),ul().lond());
    uly = ossim::min<ossim_float64>(rect.ul().latd(),ul().latd());
    lrx = ossim::min<ossim_float64>(rect.lr().lond(),lr().lond());
    lry = ossim::max<ossim_float64>(rect.lr().latd(),lr().latd());

    if( lrx < ulx || lry > uly )
    {
       return ossimGrect(ossimGpt(0,0,0),ossimGpt(0,0,0));
    }
    else
    {
       return ossimGrect(ossimGpt(uly, ulx, 0, rect.ul().datum()),
                         ossimGpt(lry, lrx, 0, rect.ul().datum()));
    }
}

//*******************************************************************
// Inline Method: ossimDrect::combine(const ossimDrect& rect)
//*******************************************************************
inline ossimGrect ossimGrect::combine(const ossimGrect& rect)const
{
   if (isLonLatNan())
      return rect;

   ossimGpt ulCombine;
   ossimGpt lrCombine;
	
	ulCombine.lon = ((ul().lon <= rect.ul().lon)?ul().lon:rect.ul().lon);
   ulCombine.lat = ((ul().lat >= rect.ul().lat)?ul().lat:rect.ul().lat);
   lrCombine.lon = ((lr().lon >= rect.lr().lon)?lr().lon:rect.lr().lon);
   lrCombine.lat = ((lr().lat <= rect.lr().lat)?lr().lat:rect.lr().lat);
	
   return ossimGrect(ulCombine, lrCombine);
}

//*****************************************************************************
//  INLINE METHOD: ossimGrect::pointWithin()
//*****************************************************************************
inline bool ossimGrect::pointWithin(const ossimGpt& gpt, bool considerHgt) const
{
   bool within = (gpt.lat <= theUlCorner.lat) && (gpt.lat >= theLrCorner.lat) &&
                 (gpt.lon >= theUlCorner.lon) && (gpt.lon <= theLrCorner.lon);
   if (considerHgt)
      within &= (gpt.hgt <= theUlCorner.hgt) && (gpt.hgt >= theLrCorner.hgt);

   return within;
}

inline ossim_float64 ossimGrect::height() const
{
   return (theUlCorner.latd() - theLrCorner.latd());
}

inline ossim_float64 ossimGrect::width() const
{
   return (theLrCorner.lond() - theUlCorner.lond());
}

inline const ossimGpt& ossimGrect::ul() const
{
   return theUlCorner;
}

inline ossimGpt ossimGrect::ur() const
{
   ossimGpt gpt (theUlCorner.lat, theLrCorner.lon, theUlCorner.hgt, theUlCorner.datum());
   return gpt;
}

inline ossimGpt ossimGrect::ll() const
{
   ossimGpt gpt (theLrCorner.lat, theUlCorner.lon, theLrCorner.hgt, theLrCorner.datum());
   return gpt;
}

inline const ossimGpt& ossimGrect::lr() const
{
   return theLrCorner;
}
   
inline ossimGpt& ossimGrect::ul()
{
   return theUlCorner;
}

inline ossimGpt& ossimGrect::lr()
{
   return theLrCorner;
}
	
inline void ossimGrect::makeNan()
{
   theUlCorner.makeNan();
   theLrCorner.makeNan();
}

inline bool ossimGrect::isLonLatNan() const
{
   return ( ossim::isnan(theUlCorner.lat) ||
            ossim::isnan(theUlCorner.lon) ||
            ossim::isnan(theLrCorner.lat) ||
            ossim::isnan(theLrCorner.lon) );
}

inline bool ossimGrect::hasNans() const
{
   return ( theUlCorner.hasNans() ||
            theLrCorner.hasNans() );
}

inline bool ossimGrect::isNan()const
{
   return ( theUlCorner.hasNans() &&
            theLrCorner.hasNans() );
}

#endif /* End of "#ifndef ossimGrect_HEADER" */

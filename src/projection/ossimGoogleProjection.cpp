//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
//
//*******************************************************************
#include <ossim/projection/ossimGoogleProjection.h>

RTTI_DEF1(ossimGoogleProjection, "ossimGoogleProjection", ossimMapProjection)


//***************** THIS CODE WAS PULLED FROM THE INTERNET FOR GLOBAL MERCATOR ******
// I will leave as is without modifications
//
#define deg2rad(d) (((d)*M_PI)/180)
#define rad2deg(d) (((d)*180)/M_PI)
#define earth_radius 6378137
 
/* The following functions take or return there results in degrees */
 
double y2lat_d(double y) { return rad2deg(2 * atan(exp(  deg2rad(y) ) ) - M_PI/2); }
double x2lon_d(double x) { return x; }
double lat2y_d(double lat) { return rad2deg(log(tan(M_PI/4+ deg2rad(lat)/2))); }
double lon2x_d(double lon) { return lon; }
 
/* The following functions take or return there results in something close to meters, along the equator */
 
double y2lat_m(double y) { return rad2deg(2 * atan(exp( (y / earth_radius ) )) - M_PI/2); }
double x2lon_m(double x) { return rad2deg(x / earth_radius); }
double lat2y_m(double lat) { return earth_radius * log(tan(M_PI/4+ deg2rad(lat)/2)); }
double lon2x_m(double lon) { return deg2rad(lon) * earth_radius; }

//******************** END GLOBAL MERCATOR CODE ***********************

ossimGoogleProjection::ossimGoogleProjection(const ossimEllipsoid& ellipsoid,
                                           const ossimGpt& origin)
   :ossimMapProjection(ellipsoid, origin)
{
   thePcsCode = 3857;
   update();
}
ossimGoogleProjection::ossimGoogleProjection(const ossimGoogleProjection& src)
   :ossimMapProjection(src)
{
   thePcsCode = 3857;
   update();
}
   
ossimGoogleProjection::~ossimGoogleProjection()
{

}

void ossimGoogleProjection::update()
{
   ossimMapProjection::update();
}

ossimGpt ossimGoogleProjection::inverse(const ossimDpt &eastingNorthing)const
{
   /*
	double lon = (eastingNorthing.x / 20037508.34) * 180.0;
	double lat = (eastingNorthing.y / 20037508.34) * 180.0;

  	lat = 180.0/M_PI * (2.0 * atan(exp(lat * M_PI / 180)) - M_PI / 2.0);

   return ossimGpt(lat, lon, 0, theDatum);  
   */
   //return ossimGpt(merc_lat(eastingNorthing.y), merc_lon(eastingNorthing.x), 0, theDatum);
   return ossimGpt(y2lat_m(eastingNorthing.y), x2lon_m(eastingNorthing.x), 0, theDatum);
}

ossimDpt ossimGoogleProjection::forward(const ossimGpt &latLon)const
{
   /*
   double easting  = 0.0;
   double northing = 0.0;
   ossimGpt gpt = latLon;
   
   if (theDatum)
   {
      if (theDatum->code() != latLon.datum()->code())
      {
         gpt.changeDatum(theDatum); // Shift to our datum.
      }
   }
   //double y = Math.Log(Math.Tan((90 + lat) * Math.PI / 360)) / (Math.PI / 180);
   easting = gpt.lond()*20037508.34/180.0;
   northing = log(tan((90.0 + gpt.latd())*M_PI/360.0))/(M_PI/180.0);
   northing *=20037508.34/180.0;

   return ossimDpt(easting, northing);
   */

   return ossimDpt(lon2x_m(latLon.lond()), lat2y_m(latLon.latd()));
}

bool ossimGoogleProjection::saveState(ossimKeywordlist& kwl, const char* prefix) const
{
   return ossimMapProjection::saveState(kwl, prefix);
}

bool ossimGoogleProjection::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   thePcsCode = 3857;
   // Must do this first.
   bool flag = ossimMapProjection::loadState(kwl, prefix);
   
   update();
   return flag;
}

bool ossimGoogleProjection::operator==(const ossimProjection& projection) const
{
   if (!ossimMapProjection::operator==(projection))
      return false;

   const ossimGoogleProjection* p = dynamic_cast<const ossimGoogleProjection*>(&projection);

   return (p != 0);
}


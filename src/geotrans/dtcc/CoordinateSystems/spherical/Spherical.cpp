//	CLASSIFICATION: UNCLASSIFIED
/***************************************************************************
 *
 * MODIFICATIONS
 *
 *	Date				Description
 *	----				------------
 *  03-25-19			Original Code
 */

#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "Spherical.h"
#include "CartesianCoordinates.h"
#include "GeodeticCoordinates.h"
#include "SphericalCoordinates.h"

#include "CoordinateConversionException.h"
#include "ErrorMessages.h"

using namespace MSP::CCS;

//***************************************************************************
//						FUNCTIONS
//***************************************************************************


Spherical::Spherical(
   double ellipsoidSemiMajorAxis,
   double ellipsoidFlattening) :
   Geocentric(ellipsoidSemiMajorAxis, ellipsoidFlattening)
{
}

Spherical::Spherical(const Spherical &g)
   : Geocentric::Geocentric(g)
{
}

Spherical::~Spherical()
{
}

MSP::CCS::SphericalCoordinates* Spherical::convertFromGeodetic(
   const MSP::CCS::GeodeticCoordinates* geodeticCoordinate)
{
   CartesianCoordinates *geodetic = Geocentric::convertFromGeodetic(geodeticCoordinate);

   double x = geodetic->x();
   double y = geodetic->y();
   double z = geodetic->z();
   delete geodetic;

   double lat, lon, radius;

   geocentricToSpherical(x, y, z, lon, lat, radius);

   return new SphericalCoordinates(CoordinateType::spherical, lon, lat, radius);
}

MSP::CCS::GeodeticCoordinates* Spherical::convertToGeodetic(
   MSP::CCS::SphericalCoordinates* sphericalCoordinate)
{
   double lon    = sphericalCoordinate->sphericalLongitude();
   double lat    = sphericalCoordinate->sphericalLatitude();
   double radius = sphericalCoordinate->radius();
   double x, y, z;

   sphericalToGeocentric(lon, lat, radius, x, y, z);
   CartesianCoordinates geocent(CoordinateType::geocentric, x, y, z);

   return Geocentric::convertToGeodetic(&geocent);
}

void Spherical::sphericalToGeocentric(
   const double longitude,
   const double latitude,
   const double radius,
   double		&x,
   double 		&y,
   double		&z) const
{
   double sinLon = sin(longitude);
   double cosLon = cos(longitude);
   double sinLat = sin(latitude);
   double cosLat = cos(latitude);

   x = radius * cosLat * cosLon;
   y = radius * cosLat * sinLon;
   z = radius * sinLat;
}

void Spherical::geocentricToSpherical(
   const double x,
   const double y,
   const double z,
   double 		&longitude,
   double		&latitude,
   double		&radius) const
{   
   if(x == 0.0 && y == 0.0)
      longitude = 0.0;
   else
      longitude = atan2(y, x);
  
   radius    = sqrt(x*x + y*y + z*z);
   if(radius == 0.0)
      latitude = 0.0;
   else
      latitude  = asin(z / radius);
}

//	CLASSIFICATION: UNCLASSIFIED

#include <string.h>
#include "SphericalCoordinates.h"

#define PI_OVER_2 (3.14159265358979323e0 / 2.0)

using namespace MSP::CCS;

SphericalCoordinates::SphericalCoordinates() :
CoordinateTuple(CoordinateType::spherical),
_azimuth(0),
_elevation(0),
_radius(0)
{
}

SphericalCoordinates::SphericalCoordinates(CoordinateType::Enum _coordinateType) :
CoordinateTuple(_coordinateType),
_azimuth(0),
_elevation(0),
_radius(0)
{
}

SphericalCoordinates::SphericalCoordinates(
   CoordinateType::Enum _coordinateType, double azOrLon, double elOrLat, double radius) :
   CoordinateTuple(_coordinateType)
{
   if( _coordinateType == MSP::CCS::CoordinateType::spherical )
   {
      setSphericalLongitude( azOrLon );
      setSphericalLatitude( elOrLat );
      setRadius( radius );
   }
   else
   {
      setAzimuth( azOrLon );
      setElevationAngle( elOrLat );
      setRange( radius );      
   }
}

SphericalCoordinates::SphericalCoordinates(
   CoordinateType::Enum _coordinateType,
   const char* __warningMessage,
   double azOrLon, double elOrLat, double radius) :
   CoordinateTuple(_coordinateType)
{
  int length = strlen( __warningMessage );
  strncpy( _warningMessage, __warningMessage, length );
  _warningMessage[ length ] = '\0';

   if( _coordinateType == MSP::CCS::CoordinateType::spherical )
   {
      setSphericalLongitude( azOrLon );
      setSphericalLatitude( elOrLat );
      setRadius( radius );
   }
   else
   {
      setAzimuth( azOrLon );
      setElevationAngle( elOrLat );
      setRange( radius );      
   }
}

SphericalCoordinates::SphericalCoordinates(const SphericalCoordinates &c)
{
   _coordinateType = c._coordinateType;

   _azimuth   = c._azimuth;
   _elevation = c._elevation;
   _radius    = c._radius;

   int length = strlen(c._warningMessage);
   strncpy(_warningMessage, c._warningMessage, length);
   _warningMessage[ length ] = '\0';
}

SphericalCoordinates::~SphericalCoordinates()
{
}

SphericalCoordinates& SphericalCoordinates::operator=(const SphericalCoordinates &c)
{
   if (this != &c) 
   {
      _coordinateType = c._coordinateType;

      _azimuth   = c._azimuth;
      _elevation = c._elevation;
      _radius    = c._radius;

      int length = strlen( c._warningMessage );
      strncpy( _warningMessage, c._warningMessage, length );
      _warningMessage[ length ] = '\0';
   }

   return *this;
}

void SphericalCoordinates::setLocalSphericalCoord(double az, double el, double range)
{
   _azimuth   = az;
   _elevation = el;
   _radius    = range;
}

void SphericalCoordinates::setAzimuth(double az)
{
   _azimuth = az;
}

void SphericalCoordinates::setElevationAngle(double el)
{
   _elevation = el;
}

void SphericalCoordinates::setRange(double range)
{
   _radius = range;
}

double SphericalCoordinates::azimuth() const
{
   return _azimuth;
}

double SphericalCoordinates::elevationAngle() const
{
   return _elevation;
}

double SphericalCoordinates::range() const
{
   return _radius;
}


void SphericalCoordinates::setSphericalCoord(
   double longitude,
   double latitude,
   double radius)
{
   _azimuth   = PI_OVER_2 - longitude;
   _elevation = latitude;
   _radius    = radius;
}

void SphericalCoordinates::setSphericalLongitude(double lon)
{
   _azimuth = PI_OVER_2 - lon;
}

void SphericalCoordinates::setSphericalLatitude(double lat)
{
   _elevation = lat;
}

void SphericalCoordinates::setRadius(double radius)
{
   _radius = radius;
}

double SphericalCoordinates::sphericalLongitude() const
{
   return PI_OVER_2 - _azimuth;
}

double SphericalCoordinates::sphericalLatitude() const
{
   return _elevation;
}

double SphericalCoordinates::radius() const
{
   return _radius;
}


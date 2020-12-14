//	CLASSIFICATION: UNCLASSIFIED

/***************************************************************************/
/*	RSC IDENTIFIER:	LOCAL SPHERICAL
 *
 *
 * MODIFICATIONS
 *
 *	Date				Description
 *	----				------------
 *	03-10-19				Original Code
 *
 */

#include <math.h>

#include "Geocentric.h"
#include "LocalSpherical.h"
#include "CartesianCoordinates.h"
#include "GeodeticCoordinates.h"
#include "SphericalCoordinates.h"
#include "CoordinateConversionException.h"
#include "ErrorMessages.h"

using namespace MSP::CCS;

/***************************************************************************/
/*						        FUNCTIONS								 */


LocalSpherical::LocalSpherical(
   double ellipsoidSemiMajorAxis, double ellipsoidFlattening,
   double originLongitude, double originLatitude, double originHeight, double orientation) :
   LocalCartesian(
   ellipsoidSemiMajorAxis, ellipsoidFlattening,
   originLongitude, originLatitude, originHeight, orientation)
{
}

LocalSpherical::LocalSpherical(const LocalSpherical &lc)
   : LocalCartesian(lc)
{
}

LocalSpherical::~LocalSpherical()
{
}

MSP::CCS::SphericalCoordinates* LocalSpherical::convertFromGeodetic(
   MSP::CCS::GeodeticCoordinates* geodeticCoordinate)
{
   CartesianCoordinates *localCartesian =
      LocalCartesian::convertFromGeodetic(geodeticCoordinate);

   double x = localCartesian->x();
   double y = localCartesian->y();
   double z = localCartesian->z();
   delete localCartesian;

   double az, el, range;
   localCartesianToLocalSpherical(x, y, z, az, el, range);

   return new SphericalCoordinates(CoordinateType::localSpherical, az, el, range);
}

MSP::CCS::GeodeticCoordinates* LocalSpherical::convertToGeodetic(
   MSP::CCS::SphericalCoordinates* localSphericalCoordinate)
{
   double az = localSphericalCoordinate->azimuth();
   double el = localSphericalCoordinate->elevationAngle();
   double range = localSphericalCoordinate->range();

   double x, y, z;

   localSphericalToLocalCartesian(az, el, range, x, y, z);
   CartesianCoordinates localCartesian(CoordinateType::localCartesian, x, y, z);

   return LocalCartesian::convertToGeodetic(&localCartesian);
}

void LocalSpherical::localSphericalToLocalCartesian(
   const double azimuth,
   const double elevation,
   const double range,
   double		&x,
   double		&y,
   double		&z) const
{
   double sinAz = sin(azimuth);
   double cosAz = cos(azimuth);
   double sinEl = sin(elevation);
   double cosEl = cos(elevation);

   x = range * cosEl * sinAz;
   y = range * cosEl * cosAz;
   z = range * sinEl;
}

void LocalSpherical::localCartesianToLocalSpherical(
   const double x,
   const double y,
   const double z,
   double 		&azimuth,
   double		&elevation,
   double		&range) const
{
   if(x == 0.0 && y == 0.0)
      azimuth = 0.0;
   else
      azimuth = atan2(x, y);

   range = sqrt(x*x + y*y + z*z);
   if (range == 0.0)
	   elevation = 0.0;
   else
	   elevation = asin(z / range);
}


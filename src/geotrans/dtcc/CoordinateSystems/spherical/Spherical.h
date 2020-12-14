//	CLASSIFICATION: UNCLASSIFIED
/***************************************************************************/
/* RSC IDENTIFIER:	SPHERICAL
 *
 * ABSTRACT
 *
 *	This component provides conversions between Geodetic coordinates (latitude,
 *	longitude in radians and height in meters) and Spherical coordinates
 *	(lat, lon, radius).
 *
 * REFERENCES
 *
 *
 *
 *
 * LICENSES
 *	None apply to this component.
 *
 * MODIFICATIONS
 *
 *	Date				Description
 *	----				------------
 *	03-25-19			Original Code
 *
 */
#ifndef GTS_SPHERICAL_H
#define GTS_SPHERICAL_H

#include "DtccApi.h"
#include "CoordinateSystem.h"
#include "Geocentric.h"

namespace MSP
{
   namespace CCS
   {
      class CartesianCoordinates;
      class GeodeticCoordinates;
      class SphericalCoordinates;

      class MSP_DTCC_API Spherical : public Geocentric
      {
      public:

         /*
         * The constructor receives the ellipsoid parameters
         * as inputs and sets the corresponding state variables.
         *
         *	ellipsoidSemiMajorAxis	: Semi-major axis of ellipsoid, in meters (input)
         * 	ellipsoidFlattening		: Flattening of ellipse. (input)
         */
         Spherical(double ellipsoidSemiMajorAxis, double ellipsoidFlattening);
         Spherical(const Spherical &g);

         ~Spherical(void);

         /*
         * The function convertFromGeodetic converts geodetic coordinates
         * (latitude, longitude, and height) to geocentric coordinates (X, Y, Z)
         * according to the current ellipsoid parameters.
         *
         *	geodeticCoordinate 	: Coordinate to convert from (input)
         *
         */
         MSP::CCS::SphericalCoordinates* convertFromGeodetic(
            const MSP::CCS::GeodeticCoordinates* geodeticCoordinate);

         /*
         * The function convertToGeodetic converts geocentric
         * coordiantes (X, Y, Z) to geodetic coordinates (latitude, longitude
         * and height), according to the current ellipsoid parameters.
         *
         *	SphericalCoordinate	: Spherical coordinate to convert from (input)
         *
         */
         MSP::CCS::GeodeticCoordinates* convertToGeodetic(
            MSP::CCS::SphericalCoordinates* sphericalCoordinate);

      protected:

         void sphericalToGeocentric(
            const double longitude,
            const double latitude,
            const double radius,
            double      &x,
            double      &y,
            double      &z) const;

         void geocentricToSpherical(
            const double x,
            const double y,
            const double z,
            double      &longitude,
            double      &latitude,
            double      &radius) const;
      };
   }
}

#endif

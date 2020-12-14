//	CLASSIFICATION: UNCLASSIFIED

#ifndef GTS_LOCAL_SPHERICAL_H
#define GTS_LOCAL_SPHERICAL_H

/***************************************************************************/
/*	RSC IDENTIFIER:	LOCAL SPHERICAL
 *
 * ABSTRACT
 *	This component provides conversions between Geodetic coordinates (latitude,
 *	longitude in radians and height in meters) and Local Spherical coordinates.
 *
 *
 * REFERENCES
 *
 * MODIFICATIONS
 *
 *	Date				Description
 *	----				------------
 *		03-16-19				Original Code
 *
 */

#include "CoordinateSystem.h"
#include "LocalCartesian.h"

namespace MSP
{
   namespace CCS
   {
      class SphericalCoordinates;

      class LocalSpherical : public LocalCartesian
      {
      public:

         /*
         * The constructor receives the ellipsoid parameters
         * and local origin parameters as inputs and sets the corresponding state variables.
         *
         * 	ellipsoidSemiMajorAxis	: Semi-major axis of ellipsoid, in meters (input)
         *	ellipsoidFlattening		: Flattening of ellipsoid (input)
         *	originLongitude		    : Longitude of the local origin, in radians (input)
         *	originLatitude			: Latitude of the local origin , in radians (input)
         *	originHeight			: Ellipsoid height of the local origin, in meters (input)
         *	orientation			    : Orientation angle of the local cartesian coordinate system, in radians (input)
         */
         LocalSpherical(double ellipsoidSemiMajorAxis, double ellipsoidFlattening,
            double originLongitude, double originLatitude, double originHeight, double orientation);

         LocalSpherical(const LocalSpherical &lc);

         ~LocalSpherical(void);

         LocalSpherical& operator=(const LocalSpherical &lc);

         /*
         * The function convertFromGeodetic converts geodetic coordinates
         * (latitude, longitude, and height) to local cartesian coordinates (X, Y, Z),
         * according to the current ellipsoid and local origin parameters.
         *
         *	geodeticCoordinate	: Geodetic coordinate to be converted (input)
         *
         */
         MSP::CCS::SphericalCoordinates* convertFromGeodetic(
            MSP::CCS::GeodeticCoordinates* geodeticCoordinates);

         /*
      * The function convertToGeodetic converts local cartesian
      * coordinates (X, Y, X) to geodetic coordinates (latitude, longitude
      * and height), according to the current ellipsoid and local origin parameters.
      *
      * sphericalCoordinate : Local spherical coordinate to be converted (input)
      */
         MSP::CCS::GeodeticCoordinates* convertToGeodetic(
            MSP::CCS::SphericalCoordinates* sphericalCoordinate);

      protected:
         void localSphericalToLocalCartesian(
            const double azimuth,
            const double elevation,
            const double radius,
            double	&x,
            double	&y,
            double	&z) const;

         void localCartesianToLocalSpherical(
            const double x,
            const double y,
            const double z,
            double	&azimuth,
            double	&elevation,
            double	&radius) const;
      };
   }
}
#endif

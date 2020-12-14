//	CLASSIFICATION: UNCLASSIFIED

#ifndef GTS_SPHERICAL_COORDINATES_H
#define GTS_SPHERICAL_COORDINATES_H

#include "CoordinateTuple.h"
#include "DtccApi.h"

namespace MSP
{
   namespace CCS
   {
      class MSP_DTCC_API SphericalCoordinates : public CoordinateTuple
      {
      public:

         SphericalCoordinates();
         SphericalCoordinates(CoordinateType::Enum _coordinateType);
         SphericalCoordinates(
            CoordinateType::Enum _coordinateType, double az, double el, double radius);
         SphericalCoordinates(
            CoordinateType::Enum _coordinateType, const char* _warningMessage,
            double az, double el, double radius);
         SphericalCoordinates(const SphericalCoordinates& cc);

         ~SphericalCoordinates();
         SphericalCoordinates& operator=(const SphericalCoordinates &cc);


         // For Local Spherical Coordinates
         void setLocalSphericalCoord(double az, double el, double range);

         void setAzimuth(double az);
         void setElevationAngle(double el);
         void setRange(double range);

         double azimuth()		   const;
         double elevationAngle()	const;
         double range()		   const;

         // For Earth centered Spherical Coordinates
         void setSphericalCoord(double longitude, double latitude, double radius);
         double sphericalLongitude() const;
         double sphericalLatitude()	 const;
         double radius()		       const;

         void setSphericalLongitude(double az);
         void setSphericalLatitude(double el);
         void setRadius(double radius);

      private:

         double _azimuth;
         double _elevation;
         double _radius;
      };
   }
}

#endif

//*****************************************************************************
// License:  See top level LICENSE.txt file.
//*****************************************************************************

#include <ossim/base/ossimEciPoint.h>
#include <ossim/base/ossimEcefPoint.h>
#include <cmath>
#include <ctime>

// Constants
const double EARTH_ROTATION_RATE = 7.2921159e-5; // rad/s (Earth's angular velocity)
const double J2000_JD = 2451545.0;               // Julian Date for J2000 epoch

ossimEciPoint::ossimEciPoint(const ossimEcefPoint& ecef, const time_t& time)
   : m_t (time)
{
   // Calculate Julian Date amd GMST (Greenwich Mean Sidereal Time) from timestammp:
   double jd = unixToJulianDate(m_t);
   double gmst = calculateGMST(jd);

   // Rotation matrix elements for Z-axis rotation by GMST angle
   double cosGmst = cos(gmst);
   double sinGmst = sin(gmst);

   // Apply rotation matrix to convert ECEF to ECI
   m_x = cosGmst * ecef.x() - sinGmst * ecef.y();
   m_y = sinGmst * ecef.x() + cosGmst * ecef.y();
   m_z = ecef.z(); // Z-component remains unchanged
}

void ossimEciPoint::toEcef(ossimEcefPoint& ecf_pt) const
{
   // Calculate Julian Date
   double jd = unixToJulianDate(m_t);

   // Calculate GMST (Greenwich Mean Sidereal Time)
   double gmst = calculateGMST(jd);

   // Rotation matrix elements for Z-axis rotation by -GMST angle
   // Note: We use -GMST because we're rotating from ECI to ECEF (inverse rotation)
   double cosGmst = cos(-gmst);
   double sinGmst = sin(-gmst);

   // Apply rotation matrix to convert ECI to ECEF
   ecf_pt.x() = cosGmst * m_x - sinGmst * m_y;
   ecf_pt.y() = sinGmst * m_x + cosGmst * m_y;
   ecf_pt.z() = m_z; // Z-component remains unchanged
}

double ossimEciPoint::distanceTo(const ossimEciPoint& point2) const
{
   ossimEcefPoint p1 (*this);
   ossimEcefPoint p2 (point2);
   return p1.distanceTo(p2);
}

ossimString ossimEciPoint::toString() const
{
   std::ostringstream os;
   os << *this;
   return os.str();
}

std::ostream &operator<<(std::ostream& os, const ossimEciPoint& p)
{
   os << std::setprecision(15);
   os << "( ";

   if ( ossim::isnan(p.m_x) )
      os << "nan";
   else
      os << p.m_x;

   os << ", ";
   if ( ossim::isnan(p.m_y) )
      os << "nan";
   else
      os << p.m_y;

   os << ", ";
   if ( ossim::isnan(p.m_z) )
      os << "nan";
   else
      os << p.m_z;

   struct tm* gmt = gmtime(&(p.m_t));
   os  << " ), " << asctime(gmt) ;

   return os;
}

double ossimEciPoint::unixToJulianDate(const time_t& timestamp)
{
   return (timestamp / 86400.0) + 2440587.5;
}

// Function to calculate Greenwich Mean Sidereal Time (GMST) in radians
double ossimEciPoint::calculateGMST(const double& julianDate)
{
   // Days since J2000.0
   double T = (julianDate - J2000_JD) / 36525.0;

   // GMST at 0h UT (in seconds)
   double gmst = 67310.54841 +
                 (876600.0 * 3600.0 + 8640184.812866) * T +
                 0.093104 * T * T -
                 6.2e-6 * T * T * T;

   // Convert to degrees and then to radians
   gmst = fmod(gmst / 240.0, 360.0) * (M_PI / 180.0);

   // Ensure positive angle
   if (gmst < 0)
      gmst += 2.0 * M_PI;

   return gmst;
}


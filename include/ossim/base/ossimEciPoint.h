//*****************************************************************************
// License:  See top level LICENSE.txt file.
//*****************************************************************************

#ifndef ossimEciPoint_HEADER
#define ossimEciPoint_HEADER

#include <iosfwd>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimColumnVector3d.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimDpt3d.h>
#include <ossim/matrix/newmat.h>

class ossimEcefVector;
class ossimEcefPoint;

//*****************************************************************************
///   Contains declaration of a 3D point object in the Earth-centered inertial
///   (ECI) coordinate system.
//*****************************************************************************
class OSSIMDLLEXPORT ossimEciPoint
{
public:
   /*!
    * CONSTRUCTORS: 
    */
   ossimEciPoint() : m_x(0), m_y(0), m_z(0), m_t(0) {}

   /** Computes ECI point from ECEF and time since start of epoch */
   ossimEciPoint(const ossimEcefPoint& convert_this, const time_t& time);

   ossimEciPoint(const double& x,
                 const double& y,
                 const double& z,
                 const time_t& t)
      : m_x(x), m_y(y), m_z(z), m_t(t) {}

   /** assigns argument with ECEF version of this point */
   void toEcef(ossimEcefPoint& ecf_pos) const;

   void makeNan()
   {
      m_x = ossim::nan();
      m_y = ossim::nan();
      m_z = ossim::nan();
   }

   bool hasNans() const
   { return (ossim::isnan(m_x) || ossim::isnan(m_y) || ossim::isnan(m_z)); }

   bool isNan() const
   { return (ossim::isnan(m_x) && ossim::isnan(m_y) && ossim::isnan(m_z)); }

   /* OPERATORS: */
   bool operator==(const ossimEciPoint&) const; // inline
   bool operator!=(const ossimEciPoint&) const; // inline

   /* COMPONENT ACCESS METHODS: */
   double x() const { return m_x; }
   double &x()      { return m_x; }
   double y() const { return m_y; }
   double &y()      { return m_y; }
   double z() const { return m_z; }
   double &z()      { return m_z; }
   time_t t() const { return m_t; }
   time_t& t()      { return m_t; }

   /** @brief To string method.
    * @param precision Output floating point precision.
    * @return ossimString representing point.
    * Output format:
    * ( 0.0000000,  0.0000000,  0.00000000, <ISO 8601 timestamp>)
    *   -----x----  -----y----  ------z----  */
   ossimString toString() const;

   /**
    * Computes distance from this to point2 (points are converted to ECEF first)
    * @param point2
    * @return distance in same units as point coordinates
    */
   double distanceTo(const ossimEciPoint& point2) const;

   friend OSSIM_DLL std::ostream &operator<<(std::ostream& os,
                                             const ossimEciPoint& instance);

private:
   /// Function to calculate Julian Date from Unix timestamp
   static double unixToJulianDate(const time_t& timestamp);

   /// Function to calculate Greenwich Mean Sidereal Time (GMST) in radians
   static double calculateGMST(const double& julianDate);

   double m_x;
   double m_y;
   double m_z;
   time_t m_t;
};

//================== BEGIN DEFINITIONS FOR INLINE METHODS =====================

//*****************************************************************************
//  INLINE METHOD: ossimEciPoint::operator==(ossimEciPoint)
//*****************************************************************************
inline bool ossimEciPoint::operator==(const ossimEciPoint& p) const
{
   return (m_x == p.m_x && m_y == p.m_y && m_z == p.m_z && m_t == p.m_t);
}

//*****************************************************************************
//  INLINE METHOD: ossimEciPoint::operator!=(ossimEciPoint)
//*****************************************************************************
inline bool ossimEciPoint::operator!=(const ossimEciPoint& p) const
{
   return (m_x != p.m_x || m_y != p.m_y || m_z != p.m_z || m_t != p.m_t);
}

#endif

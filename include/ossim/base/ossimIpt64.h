//---
// License: MIT
//
// Description: Contains class definition for 64 bit integer point.
//---
// $Id$

#ifndef ossimIpt64_H
#define ossimIpt64_H 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimDpt.h>
#include <iosfwd>

// Forward class declarations.
class ossimFpt;
class ossimDpt3d;
class ossimIpt;
class ossimString;

class OSSIMDLLEXPORT ossimIpt64
{
public:

   ossimIpt64() : x(0), y(0) {}

   ossimIpt64(ossim_int64 aX, ossim_int64 aY) : x(aX), y(aY) {}

   ossimIpt64(ossim_int32 aX, ossim_int32 aY) : x(aX), y(aY) {}

   ossimIpt64(ossim_float64 aX, ossim_float64 aY);
   
   ossimIpt64(const ossimIpt64& pt) : x(pt.x), y(pt.y) {}

   ossimIpt64(const ossimIpt& pt);

   ossimIpt64(const ossimDpt& pt);

   ossimIpt64(const ossimFpt& pt);

   ossimIpt64(const ossimDpt3d &pt);

   const ossimIpt64& operator=(const ossimIpt64& pt)
   {
      if (this != &pt)
      {
         x = pt.x;
         y = pt.y;
      }
      return *this;
   }

   const ossimIpt64& operator=(const ossimIpt& pt);

   const ossimIpt64& operator=(const ossimDpt& pt);

   const ossimIpt64& operator=(const ossimFpt& pt);

   bool operator==(const ossimIpt64& pt) const
   { return ( (x == pt.x) && (y == pt.y) ); } 
   
   bool operator!=(const ossimIpt64& pt) const
   { return ( (x != pt.x) || (y != pt.y) ); }
   
   void makeNan(){x = OSSIM_INT64_NAN; y=OSSIM_INT64_NAN;}
   
   bool hasNans()const
   {
      return ( (x==OSSIM_INT64_NAN) || (y==OSSIM_INT64_NAN) );
   }
   bool isNan()const
   {
      return ( (x==OSSIM_INT64_NAN) && (y==OSSIM_INT64_NAN) );
   }
   
   std::ostream& print(std::ostream& os) const;
   
   friend OSSIMDLLEXPORT std::ostream& operator<<(std::ostream& os,
                                                  const ossimIpt64& pt);

   /**
    * @return ossimString representing point.
    *
    * Format:  ( 30, -90 )
    *            -x- -y-
    */
   std::string toString() const;

   /**
    * Initializes this point from string.  This method opens an istream to
    * s and then calls operator>>.
    *
    * Expected format:  ( 30, -90 )
    *                     -x- -y-
    *
    * @param s String to initialize from.
    *
    * @see operator>>
    */
   void toPoint(const std::string& s);
   
   /**
    * Method to input the formatted string of the "operator<<".
    *
    * Expected format:  ( 30, -90 )
    *                     -x- -y-
    * 
    * This method starts by doing a "makeNan" on pt.  So if anything goes
    * wrong with the stream or parsing pt could be all or partially nan.
    *
    * @param is Input stream istream to formatted text.
    * @param pt osimIpt to be initialized from stream.
    * @return istream pass in.
    */
   friend OSSIMDLLEXPORT std::istream& operator>>(std::istream& is,
                                                  ossimIpt64& pt);
   
   bool isEqualTo(const ossimIpt64& rhs,
                  ossimCompareType compareType=OSSIM_COMPARE_FULL)const;
   
   /*!
    * METHOD: length()
    * Returns the RSS of the components.
    */
   double length() const { return sqrt((double)x*x + (double)y*y); }
   
   //***
   // OPERATORS: +, -, +=, -=
   // Point add/subtract with other point:
   //***
   ossimIpt64 operator+(const ossimIpt64& p) const
      { return ossimIpt64(x+p.x, y+p.y); }
   ossimIpt64 operator-(const ossimIpt64& p) const
      { return ossimIpt64(x-p.x, y-p.y); }
   const ossimIpt64& operator+=(const ossimIpt64& p)
      { x += p.x; y += p.y; return *this; }
   const ossimIpt64& operator-=(const ossimIpt64& p)
      { x -= p.x; y -= p.y; return *this; }

   //***
   // OPERATORS: *, /
   // Scale point components by scalar:
   //***
   ossimIpt64 operator*(const ossim_int64& i) const
      { return ossimIpt64(i*x, i*y); }
   ossimDpt operator*(const double& d) const
      { return ossimDpt(d*x, d*y); }
   ossimDpt operator/(const double& d) const
      { return ossimDpt(x/d, y/d); }

   //***
   // Public data members:
   //***
   union {ossim_int64 x; ossim_int64 u; ossim_int64 samp;};
   union {ossim_int64 y; ossim_int64 v; ossim_int64 line;};

};
   
#endif /* #ifndef ossimIpt64_H */

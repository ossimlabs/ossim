//---
// License: MIT
//
// Description: Contains class definition for 64 bit integer point.
//---
// $Id$

#include <ossim/base/ossimIpt64.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimDpt3d.h>
#include <ossim/base/ossimFpt.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimString.h>

#include <iostream>
#include <sstream>

ossimIpt64::ossimIpt64(ossim_float64 aX, ossim_float64 aY)
{
   if (ossim::isnan(aX) == false)
   {
      x = ossim::round<ossim_int64>(aX);
   }
   else
   {
      x = OSSIM_INT64_NAN;
   }

   if (ossim::isnan(aY) == false)
   {
      y = ossim::round<ossim_int64>(aY);
   }
   else
   {
      y = OSSIM_INT64_NAN;
   }
}

ossimIpt64::ossimIpt64(const ossimIpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = pt.x;
      y = pt.y;
   }
}

ossimIpt64::ossimIpt64(const ossimDpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = ossim::round<ossim_int64>(pt.x);
      y = ossim::round<ossim_int64>(pt.y);
   }
}

ossimIpt64::ossimIpt64(const ossimFpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = ossim::round<ossim_int64>(pt.x);
      y = ossim::round<ossim_int64>(pt.y);
   }
}

ossimIpt64::ossimIpt64(const ossimDpt3d &pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = ossim::round<ossim_int64>(pt.x);
      y = ossim::round<ossim_int64>(pt.y);
   }
}

const ossimIpt64& ossimIpt64::operator=(const ossimIpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = pt.x;
      y = pt.y;
   }
   
   return *this;
}

const ossimIpt64& ossimIpt64::operator=(const ossimDpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = ossim::round<ossim_int64>(pt.x);
      y = ossim::round<ossim_int64>(pt.y);
   }
   
   return *this;
}

const ossimIpt64& ossimIpt64::operator=(const ossimFpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = ossim::round<ossim_int64>(pt.x);
      y = ossim::round<ossim_int64>(pt.y);
   }
   
   return *this;
}

std::ostream& ossimIpt64::print(std::ostream& os) const
{
   os << "( ";

   if (x != OSSIM_INT64_NAN)
   {
      os << x;
   }
   else
   {
      os << "nan";
   }
   
   os << ", ";

   if (y != OSSIM_INT64_NAN)
   {
      os << y;
   }
   else
   {
      os << "nan";
   }

   os << " )";

   return os;
}

std::ostream& operator<<(std::ostream& os, const ossimIpt64& pt)
{
   return pt.print(os);
}

std::string ossimIpt64::toString() const
{
   std::ostringstream os;
   os << "(";
   
   if (x != OSSIM_INT64_NAN)
   {
      os << x;
   }
   else
   {
      os << "nan";
   }
   
   os << ",";
   
   if (y != OSSIM_INT64_NAN)
   {
      os << y;
   }
   else
   {
      os << "nan";
   }
   
   os << ")";
   
   return os.str();
}


void ossimIpt64::toPoint(const std::string& s)
{
   std::istringstream is(s);
   is >> *this;
}

std::istream& operator>>(std::istream& is, ossimIpt64 &pt)
{
   //---
   // Expected input format:
   // ( 30, -90 )
   //   -x- -y-
   //---

   // Start with a nan point.
   pt.makeNan();

   // Check the stream.
   if (!is) return is;
   
   const int SZ = 64; // Handle real big number...
   ossimString os;
   char buf[SZ];

   //---
   // X SECTION:
   //---
   
   // Grab data up to the first comma.
   is.get(buf, SZ, ',');

   if (!is) return is;

   // Copy to ossim string.
   os = buf;

   // Get rid of the '(' if there is any.
   std::string::size_type pos = os.find('(');
   if (pos != std::string::npos)
   {
      os.erase(pos, 1);
   }   
   if (os.contains("nan") == false)
   {
      pt.x = os.toInt64();
   }
   else
   {
      pt.x = OSSIM_INT64_NAN;
   }

   //---
   // Y SECTION:
   //---
   
   // Grab the data up to the ')'
   is.get(buf, SZ, ')');

   if (!is) return is;

   // Copy to ossim string.
   os = buf;

   // Get rid of the ',' if there is any.
   pos = os.find(',');
   if (pos != std::string::npos)
   {
      os.erase(pos, 1);
   }
   
   if (os.contains("nan") == false)
   {
      pt.y = os.toInt64();
   }
   else
   {
      pt.y = OSSIM_INT64_NAN;
   }

   // Gobble the trailing ")".
   char c = 0;
   while (c != ')')
   {
      is.get(c);
      if (!is) break;
   }

   // Finished
   return is;
}

bool ossimIpt64::isEqualTo(const ossimIpt64& rhs, ossimCompareType /* compareType */)const
{
   return ((x==rhs.x)&&
           (y==rhs.y));
}


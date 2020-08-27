//---
//
// License: MIT
//
// Description:
//
// Contains class definition for osimIrect64.
//
//---
// $Id$


#include <ossim/base/ossimIrect64.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ostream>
#include <sstream>

ossimIrect64::ossimIrect64()
   :
   m_origin(0, 0),
   m_size(0, 0), 
   m_mode(OSSIM_LEFT_HANDED)
{}

ossimIrect64::ossimIrect64(ossimIpt64 origin,
                           ossimIpt64 size,
                           ossimCoordSysOrientMode mode)
   :
   m_origin(origin),
   m_size(size),
   m_mode(mode)
{}

ossimIrect64::ossimIrect64(ossim_int64 origin_x,
                           ossim_int64 origin_y,
                           ossim_int64 size_x,
                           ossim_int64 size_y,
                           ossimCoordSysOrientMode mode)
   :
   m_origin(origin_x, origin_y),
   m_size(size_x, size_y),
   m_mode(mode)
{}

ossimIrect64::ossimIrect64(const ossimIrect64& rect)
   :
   m_origin(rect.m_origin),
   m_size(rect.m_size),
   m_mode(rect.m_mode)
{}

ossimIrect64::ossimIrect64(const ossimIrect& rect)
   :
   m_origin(rect.ul()),
   m_size(rect.size()),
   m_mode(rect.orientationMode())
{
   if ( m_mode == OSSIM_RIGHT_HANDED )
   {
      m_origin = rect.ll();
   }
}

ossimIrect64::ossimIrect64(const ossimDrect& rect)
   :
   m_origin(rect.ul()),
   m_size(rect.size()),
   m_mode(rect.orientationMode())
{
   if ( m_mode == OSSIM_RIGHT_HANDED )
   {
      m_origin = rect.ll();
   }
}

const ossimIrect64& ossimIrect64::operator=(const ossimIrect64& rect)
{
   if (this != &rect)
   {
      m_origin = rect.m_origin;
      m_size   = rect.m_size;
      m_mode   = rect.m_mode;
   }
   return *this;
}

const ossimIrect64& ossimIrect64::operator=(const ossimIrect& rect)
{
   m_mode = rect.orientationMode();
   if ( m_mode == OSSIM_LEFT_HANDED )
   {
      m_origin = rect.ul();
   }
   else
   {
      m_origin = rect.ll();
   }
   m_size = rect.size();
   
   return *this;
}

const ossimIrect64& ossimIrect64::operator=(const ossimDrect& rect)
{
   m_mode = rect.orientationMode();
   if ( m_mode == OSSIM_LEFT_HANDED )
   {
      m_origin = rect.ul();
   }
   else
   {
      m_origin = rect.ll();
   }
   m_size = rect.size();
   return *this;
}

const ossimIpt64& ossimIrect64::origin() const
{
   return m_origin;
}

ossimIpt64& ossimIrect64::origin()
{
   return m_origin;
}

const ossimIpt64& ossimIrect64::size() const
{
   return m_size;
}

ossimIpt64& ossimIrect64::size()
{
   return m_size;
}

ossimCoordSysOrientMode ossimIrect64::orientationMode() const
{
   return m_mode;
}

ossimCoordSysOrientMode ossimIrect64::orientationMode()
{
   return m_mode;
}

void ossimIrect64::end(ossimIpt64& pt) const
{
   pt.x = m_origin.x + ( (m_size.x > 0) ? m_size.x - 1 : 0);
   pt.y = m_origin.y + ( (m_size.y > 0) ? m_size.y - 1 : 0);
}

ossimIpt64 ossimIrect64::end() const
{
   ossimIpt64 pt;
   end( pt );
   return pt;
}

void ossimIrect64::ul(ossimIpt64& pt) const
{
   if ( m_mode == OSSIM_LEFT_HANDED )
   {
      pt = m_origin;
   }
   else
   {
      pt.x = m_origin.x;
      pt.y = m_origin.y + ( (m_size.y > 0) ? m_size.y - 1 : 0);
   }
}

ossimIpt64 ossimIrect64::ul() const
{
   ossimIpt64 pt;
   ul( pt );
   return pt;
}

void ossimIrect64::ur(ossimIpt64& pt) const
{
   if ( m_mode == OSSIM_LEFT_HANDED )
   {
      pt.x = m_origin.x + ( (m_size.x > 0) ? m_size.x - 1 : 0);
      pt.y = m_origin.y;
   }
   else
   {
      end( pt );
   }
}

ossimIpt64 ossimIrect64::ur() const
{
   ossimIpt64 pt;
   ur( pt );
   return pt;
}

void ossimIrect64::lr(ossimIpt64& pt) const
{
   if ( m_mode == OSSIM_LEFT_HANDED )
   {
      end( pt );
   }
   else
   {
      pt.x = m_origin.x + ( (m_size.x > 0) ? m_size.x - 1 : 0);
      pt.y = m_origin.y;
   }
}

ossimIpt64 ossimIrect64::lr() const
{
   ossimIpt64 pt;
   lr( pt );
   return pt;
}

void ossimIrect64::ll(ossimIpt64& pt) const
{
   if ( m_mode == OSSIM_LEFT_HANDED )
   {
      pt.x = m_origin.x;
      pt.y = m_origin.y + ( (m_size.y > 0) ? m_size.y - 1 : 0);
   }
   else
   {
      pt =  m_origin;
   } 
}

ossimIpt64 ossimIrect64::ll() const
{
   ossimIpt64 pt;
   ll( pt );
   return pt;
}

ossim_uint64 ossimIrect64::height() const
{
   return static_cast<ossim_uint64>(m_size.y);
}

ossim_uint64 ossimIrect64::width() const
{  
   return static_cast<ossim_uint64>(m_size.x);
}

ossim_uint64 ossimIrect64::area()const
{
   return width()*height();
}

bool ossimIrect64::operator!=(const ossimIrect64& rect) const
{
   return ( (m_origin != rect.m_origin) ||
            (m_size   != rect.m_size)   ||
            (m_mode   != rect.m_mode) );
}

bool ossimIrect64::operator==(const ossimIrect64& rect) const
{
   return ( (m_origin == rect.m_origin) &&
            (m_size   == rect.m_size)   &&
            (m_mode   == rect.m_mode) );
}

const ossimIrect64& ossimIrect64::operator+=(const ossimIpt64& shift)
{
   m_origin += shift;
   return *this;
}

const ossimIrect64& ossimIrect64::operator+=(const ossimIpt& shift)
{
   m_origin += ossimIpt64(shift);
   return *this;
}

const ossimIrect64& ossimIrect64::operator+=(const ossimDpt& shift)
{
   m_origin += ossimIpt64(shift);
   return *this;
}
   
const ossimIrect64& ossimIrect64::operator-=(const ossimIpt64& shift)
{
   m_origin -= shift;
   return *this;
}

const ossimIrect64& ossimIrect64::operator-=(const ossimIpt& shift)
{
   m_origin -= ossimIpt64(shift);
   return *this;
}

const ossimIrect64& ossimIrect64::operator-=(const ossimDpt& shift)
{
   m_origin -= ossimIpt64(shift);
   return *this;
}
   
ossimIrect64 ossimIrect64::operator+(const ossimIpt64& shift)const
{
   return ossimIrect64(m_origin + shift,
                       m_size,
                       m_mode);
}

ossimIrect64 ossimIrect64::operator+(const ossimIpt& shift) const
{
   ossimIpt64 origin = m_origin + ossimIpt64(shift);
   return ossimIrect64(origin,
                       m_size,
                       m_mode);
}

ossimIrect64 ossimIrect64::operator+(const ossimDpt& shift) const
{
   ossimIpt64 origin = m_origin + ossimIpt64(shift);
   return ossimIrect64(origin,
                       m_size,
                       m_mode);
}


ossimIrect64 ossimIrect64::operator-(const ossimIpt64& shift)const
{
   return ossimIrect64(m_origin - shift,
                       m_size,
                       m_mode);
}

ossimIrect64 ossimIrect64::operator-(const ossimIpt& shift) const
{
   ossimIpt64 origin = m_origin - ossimIpt64(shift);
   return ossimIrect64(origin,
                       m_size,
                       m_mode);
}

ossimIrect64 ossimIrect64::operator-(const ossimDpt& shift)const
{
   ossimIpt64 origin = m_origin - ossimIpt64(shift);
   return ossimIrect64(origin,
                       m_size,
                       m_mode);
}

void ossimIrect64::getBounds(ossim_int64& minx, ossim_int64& miny,
                             ossim_int64& maxx, ossim_int64& maxy)const
{
   minx = m_origin.x;
   miny = m_origin.y;
   maxx = m_origin.x + ( (m_size.x > 0) ? m_size.x - 1 : 0);
   maxy = m_origin.y + ( (m_size.y > 0) ? m_size.y - 1 : 0);
}

void ossimIrect64::getCenter(ossimDpt& center_point) const
{
   if ( hasNans() == false )
   {
      center_point.x = m_origin.x + m_size.x / 2.0;
      center_point.y = m_origin.y + m_size.y / 2.0;
   }
   else
   {
      center_point.makeNan();
   }
}

void ossimIrect64::makeNan()
{
   m_origin.makeNan();
   m_size.makeNan();
}

bool ossimIrect64::hasNans() const
{
   return m_origin.hasNans() || m_size.hasNans();
}

bool ossimIrect64::isNan() const

{
   return m_origin.hasNans() && m_size.hasNans();
}

bool ossimIrect64::stretchToTileBoundary(const ossimIpt64& tileWidthHeight)
{
   bool result = false;
   
   if ( ( tileWidthHeight.x > 0 ) && ( tileWidthHeight.y > 0 ) )
   {
      ossimIpt64 endPt;
      end( endPt );
      
      if ( m_origin.x % tileWidthHeight.x )
      {
         ossim_int64 x = (m_origin.x/tileWidthHeight.x) * tileWidthHeight.x;
         if ( x > m_origin.x )
         {
            x -= tileWidthHeight.x;
         }
         m_origin.x = x;
      }

      if ( m_origin.y % tileWidthHeight.y )
      {
         ossim_int64 y = (m_origin.y/tileWidthHeight.y) * tileWidthHeight.y;
         if ( y > m_origin.y )
         {
            y -= tileWidthHeight.y;
         }
         m_origin.y = y;
      }

      ossim_int64 x = (endPt.x/tileWidthHeight.x) * tileWidthHeight.x;
      if ( x < endPt.x )
      {
         x += tileWidthHeight.x;
      }
      endPt.x = x - 1;

      ossim_int64 y = (endPt.y/tileWidthHeight.y) * tileWidthHeight.y;
      if ( y < endPt.y )
      {
         y += tileWidthHeight.y;
      }
      endPt.y = y - 1;

      m_size.x = endPt.x - m_origin.x + 1;
      m_size.y = endPt.y - m_origin.y + 1;
      
      result = true;
   }

   return result;
}

ossimString ossimIrect64::toString()const
{
   ossimString result="(";
   if ( m_origin.x != OSSIM_INT64_NAN )
   {
      result += (ossimString::toString(m_origin.x) + ",");
   }
   else
   {
      result += "nan,";
   }
   if ( m_origin.y != OSSIM_INT64_NAN )
   {
      result += (ossimString::toString(m_origin.y) + ",");
   }
   else
   {
      result += "nan,";
   }
   if ( m_size.x != OSSIM_INT64_NAN )
   {
      result += (ossimString::toString(m_size.x) + ",");
   }
   else
   {
      result += "nan,";
   }
   if ( m_size.y != OSSIM_INT64_NAN )
   {
      result += (ossimString::toString(m_size.y) + ",");
   }
   else
   {
      result += "nan,";
   }
   if(m_mode == OSSIM_LEFT_HANDED)
   {
      result += "LH";
   }
   else 
   {
      result += "RH";
   }
   result += ")";
   return result;
}

bool ossimIrect64::toRect(const ossimString& rectString)
{
   bool result = false;
   makeNan();
   
   std::istringstream in(rectString);
   ossim::skipws(in);
   char charString[2];
   charString[1] = '\0';
   ossimString interior;
   if(in.peek() == '(')
   {
      in.ignore();
      while((in.peek() != ')')&&
            (in.peek() != '\n') &&
            in.good())
      {
         charString[0] = in.get();
         interior += charString;
      }
      if(in.peek() == ')')
      {
         result = true;
      }
   }
   if(result)
   {
      std::vector<ossimString> splitArray;
      interior.split(splitArray, ",");
      
      // assume left handed
      if(splitArray.size() >= 4)
      {
         if ( splitArray[0].trim() != "nan" )
         {
            m_origin.x = splitArray[0].toInt64();
         }
         else
         {
            m_origin.x = OSSIM_INT64_NAN;
         }
         if ( splitArray[1].trim() != "nan" )
         {
            m_origin.y = splitArray[1].toInt64();
         }
         else
         {
            m_origin.y = OSSIM_INT64_NAN;
         }
         if ( splitArray[2].trim() != "nan" )
         {
            m_size.x   = splitArray[2].toInt64();
         }
         else
         {
            m_size.x = OSSIM_INT64_NAN;
         }
         if ( splitArray[3].trim() != "nan" )
         {
            m_size.y   = splitArray[3].toInt64();
         }
         else
         {
            m_size.y = OSSIM_INT64_NAN;
         }
         
         ossimString orientation = "lh";
         if(splitArray.size() == 5)
         {
            orientation = splitArray[4].downcase().trim();
         }
         if(orientation == "lh")
         {
            m_mode = OSSIM_LEFT_HANDED;
         }
         else // origin lower left
         {
            m_mode = OSSIM_RIGHT_HANDED;
         }
      }
      else
      {
         result = false;
      }
   }
   return result;
}

const ossimIrect64& ossimIrect64::expand(const ossimIpt64& padding)
{
   m_origin.x -= padding.x;
   m_origin.y -= padding.y;
   m_size.x   += padding.x;
   m_size.y   += padding.y;
   return *this;
}

bool ossimIrect64::pointWithin(const ossimIpt64& pt) const
{
   bool result = false;
   if( hasNans() == false )
   {
      ossimIpt64 endPt;
      end( endPt );
      result = ((pt.x >= m_origin.x) &&
                (pt.x <= endPt.x) &&
                (pt.y >= m_origin.y) &&
                (pt.y <= endPt.y));
   }
   return result;
}

bool ossimIrect64::intersects(const ossimIrect64& rect) const
{
   bool result = false;
   if( (rect.hasNans() == false) && (hasNans() == false) &&
       (m_mode == rect.m_mode) )
   {
      ossimIpt64 endPt1;
      end( endPt1 );
      
      ossimIpt64 endPt2;
      rect.end( endPt2 );
   
      ossim_int64 x0 = ossim::max(rect.m_origin.x,m_origin.x);
      ossim_int64 x1 = ossim::min(endPt2.x,endPt1.x);
      ossim_int64 y0 = ossim::max(rect.m_origin.y,m_origin.y);
      ossim_int64 y1 = ossim::min(endPt2.y,endPt1.y);

      result = ((x0 <= x1) && (y0 <= y1));
   }
   
   return result;
}

bool ossimIrect64::intersects(const ossimIrect& rect) const
{
   bool result = false;
   if( (rect.hasNans() == false) && (hasNans() == false) &&
       (m_mode == rect.orientationMode()) )
   {
      ossimIpt64 endPt1;
      end( endPt1 );

      ossimIpt64 origin2;
      ossimIpt64 endPt2;
      if ( rect.orientationMode() == OSSIM_LEFT_HANDED )
      {
         origin2 = rect.ul();
         endPt2  = rect.lr();
      }
      else
      {
         origin2 = rect.lr();
         endPt2  = rect.ur();
      }

      ossim_int64 x0 = ossim::max(origin2.x,m_origin.x);
      ossim_int64 x1 = ossim::min(endPt2.x,endPt1.x);
      ossim_int64 y0 = ossim::max(origin2.y,m_origin.y);
      ossim_int64 y1 = ossim::min(endPt2.y,endPt1.y);

      result = ((x0 <= x1) && (y0 <= y1));
   }
   return result;
}

bool ossimIrect64::completely_within(const ossimIrect64& rect) const
{
   bool result = false;
   if( (rect.hasNans() == false) && (hasNans() == false) &&
       (m_mode == rect.m_mode) )
   {
   
   /*  --------------
       |     1      |
       | ---------- |
       | |        | |
       | |        | |
       | |   2    | |
       | |        | |
       | |        | |
       | ---------- |
       |            |
       --------------  */

      ossimIpt64 endPt1;
      end( endPt1 );
      
      ossimIpt64 endPt2;
      rect.end( endPt2 );

      result = ( ( m_origin.x >= rect.m_origin.x ) &&
                 ( m_origin.y >= rect.m_origin.y ) &&
                 ( endPt1.x <= endPt2.x ) &&
                 ( endPt1.y <= endPt2.y ) );
   }
   
   return result;
}

ossimIrect64 ossimIrect64::clipToRect(const ossimIrect64& rect)const
{
   ossimIrect64 result;
   result.makeNan();

   if( (rect.hasNans() == false) && (hasNans() == false) &&
       (m_mode == rect.m_mode) )
   {
      ossimIpt64 endPt1;
      end( endPt1 );
      
      ossimIpt64 endPt2;
      rect.end( endPt2 );
      
      ossim_int64 x0 = ossim::max(rect.m_origin.x, m_origin.x);
      ossim_int64 x1 = ossim::min(endPt2.x, endPt1.x);
      ossim_int64 y0 = ossim::max(rect.m_origin.y, m_origin.y);
      ossim_int64 y1 = ossim::min(endPt2.y, endPt1.y);
      
      if ((x0 <= x1) && (y0 <= y1)) // intersect test
      {
         result.m_origin.x = x0;
         result.m_origin.y = y0;
         result.m_size.x = x1 - x0 + 1;
         result.m_size.y = y1 - y0 + 1;
         result.m_mode = m_mode;
      }
   }

   return result;
}

ossimIpt64 ossimIrect64::midPoint()const
{
   ossimIpt64 result;
   if( hasNans() == false )
   {
      ossimIpt64 endPt;
      end( endPt );

      double x = (m_origin.x + endPt.x) * 0.5;
      double y = (m_origin.y + endPt.y) * 0.5;
      result.x = ossim::round<ossim_int64>(x);
      result.y = ossim::round<ossim_int64>(y);
   }
   else
   {
      result.makeNan();
   }
   
   return result;
}

void ossimIrect64::print(std::ostream& os) const
{
   os << toString();
}

std::ostream& operator<<(std::ostream& os, const ossimIrect64& rect)
{
   rect.print(os);
   return os;
}

ossimIrect64 ossimIrect64::combine(const ossimIrect64& rect) const
{
   ossimIrect64 result;

   //---
   // If any rect has NANs, size is zero, or orientation modes do not match,
   // the returned result will be nan.
   //---
   if ( ( hasNans() == false ) && (rect.hasNans() == false) &&
        ( m_size.x != 0 ) && ( rect.m_size.x != 0 ) &&
        ( m_size.y != 0 ) && ( rect.m_size.y != 0 ) &&
        ( m_mode == rect.m_mode ) )
   {
      ossimIpt64 endPt0;
      end( endPt0 );
      
      ossimIpt64 endPt1;
      rect.end( endPt1 );

      result.m_origin.x = ossim::min(m_origin.x, rect.m_origin.x);
      result.m_origin.y = ossim::min(m_origin.y, rect.m_origin.y);
      result.m_size.x = ossim::max(endPt0.x, endPt1.x) - result.m_origin.x + 1;
      result.m_size.y = ossim::max(endPt0.y, endPt1.y) - result.m_origin.y + 1;
      result.m_mode = m_mode;
   }
   else
   {
      result.makeNan();
   }

   return result;
}

bool ossimIrect64::saveState(ossimKeywordlist& kwl,
                           const char* prefix)const
{
   kwl.add(prefix,
           ossimKeywordNames::TYPE_KW,
           "ossimIrect64",
           true);
   kwl.add(prefix, "rect", toString());
   return true;
}

bool ossimIrect64::loadState(const ossimKeywordlist& kwl,
                             const char* prefix)
{
  makeNan();
  const char* type = kwl.find(prefix, ossimKeywordNames::TYPE_KW);
  if ( type )
  {
     if ( std::string(type) == "ossimIrect64" )
     {
        const char* rect = kwl.find(prefix, "rect");
        if(rect)
        {
           toRect(rect);
        }
     }
  }
  return true;
}

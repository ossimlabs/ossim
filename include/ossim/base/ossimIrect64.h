//---
//
// License: MIT
//
// Description:
//
// Contains class declaration for ossimIrect64.
//
//---
// $Id$

#ifndef ossimIrect64_H
#define ossimIrect64_H 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIpt64.h>
#include <iosfwd>

// Forward class declarations.
class ossimDrect;
class ossimIrect;
class ossimKeywordlist;

/**
 * @class ossimIrect64
 * 
 * This class supports both left and right-handed coordinate systems. For both,
 * the positive x-axis extends to the "right".
 *
 * Notes:
 * 1) If size == 0, operations that return "origin + size - 1" will just return
 *    the origin.
 * 2) No support for negative size right now. Not sure if we want to support
 *    that.
 * 3) There are very few nan checks.   
 */
class OSSIMDLLEXPORT ossimIrect64
{
public:

   /** @brief Default constructor. */
   ossimIrect64();

   /**
    * @brief Constructor that takes an origin, size and mode.
    * @param origin
    * @param size
    * @param mode
    */
   ossimIrect64(ossimIpt64 origin,
                ossimIpt64 size,
                ossimCoordSysOrientMode mode=OSSIM_LEFT_HANDED);

   /**
    * @brief Constructor that takes an origin, size and mode.
    * @param origin_x
    * @param origin_y
    * @param size_x
    * @param size_y
    * @param mode
    */  
   ossimIrect64(ossim_int64 origin_x,
                ossim_int64 origin_y,
                ossim_int64 size_x,
                ossim_int64 size_y,
                ossimCoordSysOrientMode mode=OSSIM_LEFT_HANDED);

   /**
    * @brief Copy Constructor.
    * @rect
    */
   ossimIrect64(const ossimIrect64& rect);

   /**
    * @brief Constructor that takes 32 bit rect.
    * @rect
    */
   ossimIrect64(const ossimIrect& rect);

   /**
    * @brief Constructor that takes double rect.
    * @rect
    */
   ossimIrect64(const ossimDrect& rect);
   
   /**
    * @brief assignment operator from a 64 bit rect.
    * @param rect
    */
   const ossimIrect64& operator=(const ossimIrect64& rect);

   /**
    * @brief assignment operator from a 32 bit rect.
    * @param rect
    */
   const ossimIrect64& operator=(const ossimIrect& rect);

   /**
    * @brief assignment operator from a doubl rect.
    * @param rect
    */
   const ossimIrect64& operator=(const ossimDrect& rect);

   /**
    * @brief Access to origin data member.
    *
    * If orientation mode is left handed this will be the upper left; else
    * lower left for right handed.
    * 
    * @return Const reference to origin data member.
    */
   const ossimIpt64& origin() const;

   /**
    * @brief Access to origin data member.
    *
    * If orientation mode is left handed this will be the upper left; else
    * lower left for right handed.
    * 
    * @return Reference to origin data member.
    */
   ossimIpt64& origin();

   /**
    * @brief Access to size data member.
    * @return Const reference to size data member.
    */
   const ossimIpt64& size() const;

   /**
    * @brief Access to size data member.
    * @return Reference to size data member.
    */
   ossimIpt64& size();

   /**
    * @brief Access to orientation mode data member.
    * @return Const reference to orientation mode data member.
    */
   ossimCoordSysOrientMode orientationMode() const;

   /**
    * @brief Access to orientation mode data member.
    * @return Reference to orientation mode data member.
    */
   ossimCoordSysOrientMode orientationMode();

   /**
    * @brief Get the end point, i.e. origin + size - 1.
    *
    * If mode is left handed this is lower right.
    * If mode is right handed this is upper right.
    * 
    * @param pt Initialized by this.
    */
   void end(ossimIpt64& pt) const;

   /** @return the end point. */
   ossimIpt64 end() const;
   
   /**
    * @brief Upper left corner.
    * @param pt Initialized by this.
    */
   void ul(ossimIpt64& pt) const;

   /** @return Upper left corner. */
   ossimIpt64 ul() const;

   /**
    * @brief Upper right corner.
    * @param pt Initialized by this.
    */  
   void ur(ossimIpt64& pt) const;

   /** @return Upper right corner. */
   ossimIpt64 ur() const;

   /**
    * @brief Lower right corner.
    * @param pt Initialized by this.
    */
   void lr(ossimIpt64& pt) const;

   /** @return Lower right corner. */
   ossimIpt64 lr() const;

   /**
    * @brief Lower left corner.
    * @param pt Initialized by this.
    */   
   void ll(ossimIpt64& pt) const;

   /** @return Lower left corner. */
   ossimIpt64 ll() const;

   /**
    * @brief height
    * @return height
    */
   ossim_uint64 height() const;

   /**
    * @brief width
    * @return width
    */
   ossim_uint64 width() const;

   /**
    * @brief Area
    * @return width*height
    */
   ossim_uint64 area() const;
   
   /**
    * @brief operator!=
    * @param rect
    * @return true if not equal; else, false.
    */
   bool operator!=(const ossimIrect64& rect) const;

   /**
    * @brief operator==
    * @param rect
    * @return true if equal; else, false.
    */
   bool operator==(const ossimIrect64& rect) const;

   /**
    * @brief operator+=
    * @param shift Added to this.
    * @return Reference to this.
    */
   const ossimIrect64& operator+=(const ossimIpt64& shift);
   const ossimIrect64& operator+=(const ossimIpt& shift);
   const ossimIrect64& operator+=(const ossimDpt& shift);

   /**
    * @brief operator-=
    * @param shift Subtracted from this.
    * @return Reference to this.
    */  
   const ossimIrect64& operator-=(const ossimIpt64& shift);
   const ossimIrect64& operator-=(const ossimIpt& shift);
   const ossimIrect64& operator-=(const ossimDpt& shift);

   /**
    * @brief operator+
    * @param shift Added to this.
    * @return Rectangle with this + shift.
    */
   ossimIrect64 operator+(const ossimIpt64& shift)const;
   ossimIrect64 operator+(const ossimIpt& shift) const;
   ossimIrect64 operator+(const ossimDpt& shift) const;
   
   /**
    * @brief operator-
    * @param shift Subtracted from  this.
    * @return Rectangle with this - shift.
    */ 
   ossimIrect64 operator-(const ossimIpt64& shift) const;
   ossimIrect64 operator-(const ossimIpt& shift) const;
   ossimIrect64 operator-(const ossimDpt& shift) const;   

   /**
    * @brief Gets the bounds of the rectangle.
    * @param minx
    * @param miny
    * @param maxx
    * @param maxy
    */
   void getBounds(ossim_int64& minx, ossim_int64& miny,
                  ossim_int64& maxx, ossim_int64& maxy) const;

   /**
    * Initializes center_point with center of the rectangle.
    * Makes center_point nan if this rectangle has nans.
    */
   void getCenter(ossimDpt& center_point) const;

   /**
    * @brief Assigns m_origin and m_size to OSSIM_INT64_NAN.
    */
   void makeNan();

   /**
    * @brief Test for nans.
    * @return true if any nans; false, if not.
    */
   bool hasNans() const;

   /**
    * @brief Test for nans.
    * @return true if everything is nan; false, if not.
    */ 
   bool isNan() const;

   /**
    * @brief Stretches this rectangle to tile boundaries.
    * @param tileWidthHeight
    */
   bool stretchToTileBoundary(const ossimIpt64& tileWidthHeight);
   
   /**
    * @return ossimString representing ossimIrect64.
    *
    * Format:  ( 30, -90, 512, 512, [LH|RH] )
    *            -x- -y-  -w-  -h-   -Right or left handed-
    *
    * where:
    *     x and y are origins either upper left if LEFT HANDED (LH) or
    *                                lower left if RIGHT HANDED (RH)
    *     w and h are width and height respectively
    *     The last value is LH or RH to indicate LeftHanded or RightHanded
    *    
    */
   ossimString toString()const;
   
   
   /**
    * expected Format:  form 1: ( 30, -90, 512, 512, [LH|RH] )
    *                            -x- -y-  -w-  -h-   -Right or left handed-
    * 
    *                   form 2: ( 30, -90, 512, 512)
    *                            -x- -y-  -w-  -h-
    *
    * NOTE: Form 2 assumes Left handed were x,y is origin upper left and y
    * positive down.
    *
    * This method starts by doing a "makeNan" on rect. 
    *
    * @param rectString String to initialize from.
    * @return true or false to indicate successful parsing.
    */
   bool toRect(const ossimString& rectString);
   
   /**
    * @brief Expands rect by padding.
    * @param padding
    */
   const ossimIrect64& expand(const ossimIpt64& padding);
   
   /*!
    * @param pt
    * @return true if "pt" falls within rectangle.
    */
   bool pointWithin(const ossimIpt64& pt) const;

   /*!
    * @return true if any portion of an input rectangle "rect" intersects
    * "this" rectangle.  
    */
   bool intersects(const ossimIrect64& rect) const;

   /*!
    * @brief Overloaded intersect for ossimIrect.
    * @return true if any portion of an input rectangle "rect" intersects
    * "this" rectangle.
    */
   bool intersects(const ossimIrect& rect) const;

   /*!
    * Returns true if "this" rectangle is contained completely within
    * the input rectangular "rect".
    */
   bool completely_within(const ossimIrect64& rect) const;
   
   ossimIrect64 clipToRect(const ossimIrect64& rect)const;

   ossimIpt64 midPoint()const;

   void print(std::ostream& os) const;

   friend OSSIM_DLL std::ostream& operator<<(std::ostream& os,
                                             const ossimIrect64& rect);

   /**
    * @brief Combines this rectangle with rect arg and returns the result.
    *
    * @note If any rect has NANs, size is zero, or orientation modes do not
    * match, the returned result will be nan.
    *
    * @return combined rect.
    */
   ossimIrect64 combine(const ossimIrect64& rect)const;

   bool saveState(ossimKeywordlist& kwl,
                  const char* prefix=0)const;
   bool loadState(const ossimKeywordlist& kwl,
                  const char* prefix=0);
private:

   // Private data members:
   ossimIpt64              m_origin;
   ossimIpt64              m_size;
   ossimCoordSysOrientMode m_mode; 
};

#endif /* #ifndef ossimIrect64_H */

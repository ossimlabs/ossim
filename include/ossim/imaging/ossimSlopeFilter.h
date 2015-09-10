//*******************************************************************
// OSSIM
//
// License:  See top level LICENSE.txt file.
// 
//*******************************************************************
//  $Id: ossimSlopeFilter.h 23451 2015-07-27 15:42:17Z okramer $
#ifndef ossimSlopeFilter_HEADER
#define ossimSlopeFilter_HEADER

#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/imaging/ossimImageToPlaneNormalFilter.h>

/**
 * Filter class for computing the slope image of the input image connection. The slope
 * dP/dR is defined as the maximum change in pixel value for a differential change in ground
 * position. This filter would typically be applied to elevation data (represented as an image),
 * where dP is the change in height when walking some distance dR either straight uphill or
 * downhill. Numerically , this quantity is computed from the dot product of the normal vector with
 * the local vertical.
 *
 * The output is a floating point single-band image. The input should be a single-band, floating
 * point image. The slope quantity can be represented as an angle from local vertical, i.e., the
 * arccos(dP/dR) (in radians, degrees, or normalized) or as the simple ratio dP/dR.
 */
class OSSIMDLLEXPORT ossimSlopeFilter : public ossimImageSourceFilter
{
public:
   /** Specifies how the slope quantity is represented */
   enum SlopeType
   {
      RADIANS,   // Angle from local vertical in radians
      DEGREES,   // Angle from local vertical in degrees (default)
      RATIO,     // dH/dR ratio of change in height to change in horizontal ground distance
      NORMALIZED // Angle from local vertical normalized so that 0.0 = 0 deg, 1.0 = 90 deg)
   };

   ossimSlopeFilter();
   ossimSlopeFilter(ossimImageSource* inputSource);

   virtual void initialize();

   virtual ossimString getLongName()  const;
   virtual ossimString getShortName() const;
   
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tile_rect, ossim_uint32 rLevel=0);

   /**
    *  Returns the output pixel type of the tile source.  This override the
    *  base class since it simply returns it's input scalar type which is
    *  not correct with this object.
    */
   virtual ossimScalarType getOutputScalarType() const { return OSSIM_FLOAT32; }

   /**
    *  Method to save the state of an object to a keyword list.
    *  Return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=NULL)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=NULL);

   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;
   
   void setSlopeType(SlopeType t) { m_slopeType = t; }

protected:
   virtual ~ossimSlopeFilter();
   static ossimString getSlopeTypeString(SlopeType t);

   ossimRefPtr<ossimImageToPlaneNormalFilter> m_normals;
   SlopeType m_slopeType;

   TYPE_DATA
};

#endif /* #ifndef ossimSlopeFilter_HEADER */

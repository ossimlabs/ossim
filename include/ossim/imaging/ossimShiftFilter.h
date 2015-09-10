//----------------------------------------------------------------------------
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Filter for shifting input to an output range.  See class description
// below for more.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimShiftFilter_HEADER
#define ossimShiftFilter_HEADER 1

#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/imaging/ossimImageData.h>

/**
 * @class ossimShiftFilter
 *
 * Class to shift/stretch input values to given min/max.  Callers must set
 * the min, max, null output values.  Input data shifted stretch base on:
 *
 * pix = outMin + (inPix - inMin) * (outMax-outMin)/(inMax-inMin)
 *
 * See ossim-shift-filter-test.cpp for concrete example of usage.
 */
class OSSIMDLLEXPORT ossimShiftFilter : public ossimImageSourceFilter
{
public:

   /** default constructor */
   ossimShiftFilter();

   virtual void initialize();

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tileRect,
                                               ossim_uint32 resLevel=0);
   /** @return "ossimShiftFilter" */
   virtual ossimString getClassName() const;

   /** @return "OSSIM shift filter" */
   virtual ossimString getLongName()  const;

   /** @return "shift filter" */
   virtual ossimString getShortName() const;

   
   /** @return The null pixel of the band. */
   virtual double getNullPixelValue(ossim_uint32 band)const;

   /** @return The min pixel of the band. */
   virtual double getMinPixelValue(ossim_uint32 band)const;
   
   /** @return The max pixel of the band. */
   virtual double getMaxPixelValue(ossim_uint32 band)const;

   /** @brief Set the null output pixel. */
   void setNullPixelValue(double null);

   /** @brief Set the min output pixel. */
   void setMinPixelValue(double min);

   /** @brief Set the max output pixel. */
   void setMaxPixelValue(double max);

protected:

   /** virtual protected destructor */
   virtual ~ossimShiftFilter();

private:
   
   /** @brief Private to disallow use... */
   ossimShiftFilter(const ossimShiftFilter&);

   /** @brief Private to disallow use... */
   ossimShiftFilter& operator=(const ossimShiftFilter&);

   /** @brief Allocates the tile.  Called on first getTile. */
   void allocate();

   /**
    * @brief Template to fill the tile.
    * @param dummy template type.
    * @param inputTile
    * @param outputTile
    */
   template <class T> void fillTile(T dummy,
                                    const ossimImageData* inputTile,
                                    ossimImageData* outputTile) const;
   
   ossimRefPtr<ossimImageData> m_tile;
   double m_min;
   double m_max;
   double m_null;

TYPE_DATA
};

#endif /* #ifndef ossimShiftFilter_HEADER */

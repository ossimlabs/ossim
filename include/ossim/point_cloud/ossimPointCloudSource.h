//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  MIT -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
#ifndef ossimPointCloudSource_HEADER
#define ossimPointCloudSource_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h> /* for ossim::isnan */
#include <ossim/base/ossimSource.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/point_cloud/ossimPointBlock.h>
#include <ossim/point_cloud/ossimPointRecord.h>
#include <vector>

/***************************************************************************************************
 * Base class for all Point Cloud data sources, including input file handlers and processing nodes.
 **************************************************************************************************/
class OSSIMDLLEXPORT ossimPointCloudSource: public ossimSource
{
public:
   ossimPointCloudSource(ossimObject* owner = 0);
   virtual ~ossimPointCloudSource();

   /**
    * Fetches the block of points inside the block bounds. If the height components of the bounds
    * are NaN, then only the horizontal bounds are considered. Thread-safe version accepts data
    * block object from caller.
    */
   virtual void getBlock(const ossimGrect& bounds, ossimPointBlock& block) const = 0;

   /**
    * Returns the total number of points in the dataset (not just the block returned in getPoints)
    */
   virtual ossim_uint32 getNumPoints() const = 0;

   /**
    * Assigns the two points with fields representing the extremes of the dataset. Useful for
    * normalizing and establishing bounds.
    */
   virtual const ossimPointRecord*  getMinPoint() const;
   virtual const ossimPointRecord*  getMaxPoint() const;

   /**
    * Fetches the data fields ids available from this source, OR'd together for testing against
    * specific field (@see ossimPointRecord::FIELD_CODES). e.g. (though should use hasField()),
    *
    * bool hasIntensity = getFields() & ossimPointRecord::Intensity;
    */
   virtual ossim_uint32 getFieldCode() const;

   /**
    * Sets the data fields ID of interest for this source, and all input sources connected to this.
    * This is an OR'd mash-up of @see ossimPointRecord::FIELD_CODES
    */
   virtual void setFieldCode (ossim_uint32 fieldCode);

   /**
    * Returns true if the data field is available from this source. The argument can be OR'd
    * together for testing multiple fileds simultaneously. Example:
    *
    * bool hasRGB = hasField(ossimPointRecord::Red|ossimPointRecord::Green|ossimPointRecord::Blue);
    */
   virtual bool hasFields(ossim_uint32 id) const { return getFieldCode()&id; }

   /** Special for convenience */
   bool hasRGB() const
   { return hasFields(ossimPointRecord::Red|ossimPointRecord::Green|ossimPointRecord::Blue); }

   virtual bool canConnectMyInputTo(ossim_int32, const ossimConnectableObject*) const;

protected:
   ossimPointCloudSource(const ossimPointCloudSource& rhs);
   ossimPointRecord m_nullPCR;

TYPE_DATA
};

#endif /* #ifndef ossimPointCloudSource_HEADER */

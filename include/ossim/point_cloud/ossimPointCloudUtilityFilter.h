//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  MIT -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id$

#ifndef ossimPointCloudUtilityFilter_HEADER
#define ossimPointCloudUtilityFilter_HEADER

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/point_cloud/ossimPointCloudHandler.h>
#include <ossim/point_cloud/ossimPointCloudImageHandler.h>
#include <ossim/projection/ossimImageViewTransform.h>
#include <vector>

class ossimImageData;
class ossimPointCloudUtil;

class OSSIMDLLEXPORT ossimPointCloudUtilityFilter : public ossimImageSourceFilter
{
public:
   ossimPointCloudUtilityFilter( ossimPointCloudUtil* pc_util);
   virtual ~ossimPointCloudUtilityFilter() {}

   virtual bool getTile(ossimImageData* result, ossim_uint32 resLevel=0);
   
   ossimScalarType getOutputScalarType() const { return OSSIM_FLOAT32; }
;
   virtual ossim_uint32 getNumberOfOutputBands() const { return 1; }

   virtual ossimRefPtr<ossimImageGeometry> getImageGeometry();

protected:
   class PcrBucket
   {
   public:
      PcrBucket() : m_bucket(0), m_numSamples(0) {}
      PcrBucket(const ossim_float32* init_value, ossim_uint32 numBands);
      PcrBucket(const ossim_float32& R, const ossim_float32& G, const ossim_float32& B);
      PcrBucket(const ossim_float32& init_value);
      ~PcrBucket();
      ossim_float32* m_bucket;
      int m_numSamples;
   };

   void initTile();

   void addSample(std::map<ossim_int32, PcrBucket*>& accumulator,
                  ossim_int32 index,
                  const ossimPointRecord* sample);

   void normalize(std::map<ossim_int32, PcrBucket*>& accumulator);

   ossim_uint32 componentToFieldCode() const;

   ossimRefPtr<ossimPointCloudUtil> m_util;

   TYPE_DATA
};

#endif

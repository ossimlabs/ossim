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
class ossimPointCloudTool;

class OSSIMDLLEXPORT ossimPointCloudUtilityFilter : public ossimImageSourceFilter
{
public:
   ossimPointCloudUtilityFilter( ossimPointCloudTool* pc_util);
   virtual ~ossimPointCloudUtilityFilter() {}

   virtual bool getTile(ossimImageData* result, ossim_uint32 resLevel);
   
   ossimScalarType getOutputScalarType() const { return OSSIM_FLOAT32; }

   virtual ossim_uint32 getNumberOfOutputBands() const { return 1; }

   virtual ossimRefPtr<ossimImageGeometry> getImageGeometry();

protected:
   ossimRefPtr<ossimPointCloudTool> m_util;

   TYPE_DATA
};

#endif

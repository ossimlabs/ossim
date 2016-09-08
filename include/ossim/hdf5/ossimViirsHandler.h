/*****************************************************************************
*                                                                            *
*                                 O S S I M                                  *
*            Open Source, Geospatial Image Processing Project                *
*          License: MIT, see LICENSE at the top-level directory              *
*                                                                            *
*****************************************************************************/

#ifndef OSSIM_PLUGINS_HDF5_SRC_OSSIMVIIRSHANDLER_H_
#define OSSIM_PLUGINS_HDF5_SRC_OSSIMVIIRSHANDLER_H_

#include <ossim/plugin/ossimPluginConstants.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/hdf5/ossimHdf5ImageHandler.h>

/**
 * Concrete class for reading VIIRS data from HDF5 file
 */
class OSSIM_PLUGINS_DLL ossimViirsHandler : public ossimHdf5ImageHandler
{
public:
   ossimViirsHandler();
   virtual ossimRefPtr<ossimImageGeometry> getImageGeometry();
   virtual double getNullPixelValue(ossim_uint32 band=0)const;

};



#endif /* OSSIM_PLUGINS_HDF5_SRC_OSSIMVIIRSHANDLER_H_ */

/*****************************************************************************
*                                                                            *
*                                 O S S I M                                  *
*            Open Source, Geospatial Image Processing Project                *
*          License: MIT, see LICENSE at the top-level directory              *
*                                                                            *
*****************************************************************************/

#ifndef ossimViirsHandler_HEADER
#define ossimViirsHandler_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/hdf5/ossimHdf5ImageHandler.h>

/**
 * Concrete class for reading VIIRS data from HDF5 file
 */
class OSSIM_DLL ossimViirsHandler : public ossimHdf5ImageHandler
{
public:
   ossimViirsHandler();
   virtual ossimRefPtr<ossimImageGeometry> getImageGeometry();
   virtual double getNullPixelValue(ossim_uint32 band=0)const;

};



#endif /* #ifndef ossimViirsHandler_HEADER */

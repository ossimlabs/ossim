/*****************************************************************************
*                                                                            *
*                                 O S S I M                                  *
*            Open Source, Geospatial Image Processing Project                *
*          License: MIT, see LICENSE at the top-level directory              *
*                                                                            *
*****************************************************************************/

#include <ossim/hdf5/ossimViirsHandler.h>
#include <ossim/hdf5/ossimHdf5GridModel.h>

static const ossimString VIIRS_DATASET  ("/All_Data/VIIRS-DNB-SDR_All/Radiance");
static const ossimString VIIRS_GEOMETRY ("/All_Data/VIIRS-DNB-GEO_All");

ossimViirsHandler::ossimViirsHandler()
{
   m_renderableNames.push_back(VIIRS_DATASET);
}

ossimRefPtr<ossimImageGeometry> ossimViirsHandler::getImageGeometry()
{
   if (theGeometry.valid())
      return theGeometry;

   theGeometry = getExternalImageGeometry();
   if (!theGeometry.valid() && isOpen())
   {
      theGeometry =  new ossimImageGeometry();

      // Attempt to create an OSSIM coarse grid model from HDF5 lat lon grids:
      ossimRefPtr<ossimHdf5GridModel> hdf5GridModel = new ossimHdf5GridModel;
      if ( hdf5GridModel->initialize(m_hdf5.get(), VIIRS_GEOMETRY) )
      {
         theGeometry->setProjection(hdf5GridModel.get());
         initImageParameters( theGeometry.get() );
      }
      else
         theGeometry = 0;
   }

   return theGeometry;
}


double ossimViirsHandler::getNullPixelValue(ossim_uint32 band) const
{
   // NPP VIIRS data has null of "-999.3".
   return -999.3;
}

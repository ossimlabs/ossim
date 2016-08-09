//*****************************************************************************
// FILE: ossimHdf5GridModel.cc
//
// License:  See LICENSE.txt file in the top level directory.
//
// AUTHOR: David Burken
//
// Copied from Mingjie Su's ossimHdfGridModel.
//
// DESCRIPTION:
//   Contains implementation of class ossimHdf5GridModel. This is an
//   implementation of an interpolation sensor model. 
//
//   IMPORTANT: The lat/lon grid is for ground points on the ellipsoid.
//   The dLat/dHgt and dLon/dHgt partials therefore are used against
//   elevations relative to the ellipsoid.
//
//*****************************************************************************
//  $Id$

#include <ossim/hdf5/ossimHdf5GridModel.h>
#include <ossim/hdf5/ossimHdf5.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <sstream>
#include <string>

using namespace std;

static const int    GRID_SAMPLING_INTERVAL = 4;

RTTI_DEF1(ossimHdf5GridModel, "ossimHdf5GridModel", ossimCoarseGridModel);


ossimHdf5GridModel::ossimHdf5GridModel()
:
         ossimCoarseGridModel(),
         m_crossesDateline(false),
         m_boundGndPolygon()
{
   theLatGrid.setDomainType(ossimDblGrid::SAWTOOTH_90);
}

ossimHdf5GridModel::~ossimHdf5GridModel()
{
}

bool ossimHdf5GridModel::initialize(ossimRefPtr<ossimHdf5>& hdf5)
{
   if (!hdf5.valid())
      return false;

   try
   {
      initCoarseGrid(hdf5.get(), "Latitude",  theLatGrid);
      initCoarseGrid(hdf5.get(), "Longitude", theLonGrid);
   }
   catch (ossimException& x)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<x.what();
      return false;
   }

   // Finish initializing base class. theImageSize assigned in calls to initCoarseGrid() above:
   ossimIrect bounds (0, 0, theImageSize.u-1, theImageSize.v-1);
   initializeModelParams(bounds);

   return true;
}

bool ossimHdf5GridModel::initCoarseGrid(ossimHdf5* hdf5, const char* datasetName,
                                        ossimDblGrid& coarseGrid)
{
   ostringstream xmsg;

   // Convention used: (u,v) is file space, (x,y) is CG space
   H5::DataSet* dataset  = hdf5->findDatasetByName(datasetName);
   if (dataset == NULL)
   {
      xmsg  << "ossimHdf5GridModel:"<<__LINE__
            <<" ERROR: Could not find dataset \""<<datasetName<<"\" in file.";
      throw ossimException(xmsg.str());
   }

   // Verify dimensions:
   H5::DataSpace dataSpace = dataset->getSpace();
   if (dataSpace.getSimpleExtentNdims() != 2)
   {
      xmsg << "ossimHdf5GridModel:"<<__LINE__<<" ERROR: lat/lon grid dataspace rank != 2.";
      throw ossimException(xmsg.str());
   }

   // Fetch size of grid in file. Extents are assumed to be the same for both lat and lon grids:
   hsize_t datExtents[2];
   dataSpace.getSimpleExtentDims(datExtents);
   if ((datExtents[0] < 2) || (datExtents[1] < 2))
   {
      xmsg << "ossimHdf5GridModel:"<<__LINE__<<" ERROR: lat/lon grid size is < 2.";
      throw ossimException(xmsg.str());
   }

   // Initialize the base class coarse grids:
   theImageSize  = ossimDpt(datExtents[0], datExtents[1]);
   ossimDrect uvRect(0, 0, theImageSize.u-1, theImageSize.v-1);
   ossimDpt cgSpacing(GRID_SAMPLING_INTERVAL, GRID_SAMPLING_INTERVAL);
   coarseGrid.initialize(uvRect, cgSpacing, ossim::nan());

   // Declare data of interest in file (the whole thing):
   hsize_t offset[2] = { 0, 0 };
   dataSpace.selectHyperslab( H5S_SELECT_SET, datExtents, offset );

   // Initialize dataspace for memory buffer needed by dataset read operation:
   hsize_t bufExtents[2] = { (hsize_t) theImageSize.u, 1 };
   H5::DataSpace bufSpace(2, bufExtents);
   H5::DataType dataType = dataset->getDataType();
   if (dataType.getClass() != H5T_FLOAT)
   {
      xmsg << "ossimHdf5GridModel:"<<__LINE__<<" ERROR: lat/lon grid datatype must be float.";
      throw ossimException(xmsg.str());
   }
   ossim_float32* buffer = new ossim_float32 [bufExtents[0]]; // assumes float datatype

   // See if we need to swap bytes:
   ossimEndian* endian = 0;
   H5::AtomType* atomType = dynamic_cast<H5::AtomType*>(&dataType);
   if(atomType)
   {
      ossimByteOrder ossimByteOrder = ossim::byteOrder();
      H5T_order_t h5order = atomType->getOrder();
      if( ((h5order == H5T_ORDER_LE) && (ossimByteOrder != OSSIM_LITTLE_ENDIAN)) ||
            ((h5order == H5T_ORDER_BE) && (ossimByteOrder != OSSIM_BIG_ENDIAN)))
         endian = new ossimEndian();
   }

   // Loop over input grid rows, sampling according to desired interval to fill output
   // coarse grid, for latitude:
   ossimIpt cgGridSize (coarseGrid.size());
   ossim_uint32 x=0, y=0, u=0, v=0;
   for ( ; (y<cgGridSize.y) && (v<theImageSize.v); ++y, v+=GRID_SAMPLING_INTERVAL )
   {
      offset[1] = v; // offset[0] always = 0
      dataSpace.selectHyperslab( H5S_SELECT_SET, bufExtents, offset);
      dataset->read( buffer, dataType,  bufSpace, dataSpace );
      if ( endian )
         endian->swap( buffer, bufExtents[0] );

      // Need to subsample the input row and save into coarse grid:
      for ( x=0, u=0; (x<cgGridSize.x)&&(u<theImageSize.u); ++x, v+=GRID_SAMPLING_INTERVAL)
      {
         if ( ossim::isnan(buffer[u]))
         {
            xmsg << "ossimHdf5GridModel:"<<__LINE__<<" ERROR: encountered nans in lat/lon grid.";
            throw ossimException(xmsg.str());
         }
         coarseGrid.setNode( x, y, buffer[u] );
      }

      // Check if last column is outside of image bounds.
      if (x < cgGridSize.x)
         coarseGrid.setNode( x, y, coarseGrid.getNode( x-1, y ) );
   }

   // Check if last row is outside of image bounds.
   if (y < cgGridSize.y)
   {
      for ( x=0; x<cgGridSize.x; ++x)
         coarseGrid.setNode( x, y, coarseGrid.getNode( x, y-1 ) );
   }

   delete dataset;
   delete buffer;
   delete endian;

   return true;
}


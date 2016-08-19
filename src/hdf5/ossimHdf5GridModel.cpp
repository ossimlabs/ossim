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
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimBilinearProjection.h>
#include <ossim/hdf5/ossimHdf5ImageHandler.h>
#include <sstream>
#include <string>

using namespace std;

static const int    GRID_SAMPLING_INTERVAL = 8;

RTTI_DEF1(ossimHdf5GridModel, "ossimHdf5GridModel", ossimCoarseGridModel);


ossimHdf5GridModel::ossimHdf5GridModel()
: ossimCoarseGridModel()
{
   theLatGrid.setDomainType(ossimDblGrid::SAWTOOTH_90);
}

ossimHdf5GridModel::~ossimHdf5GridModel()
{
}

bool ossimHdf5GridModel::initialize(ossimHdf5* hdf5, ossimHdf5ImageHandler* handler)
{
   if (!hdf5 || !handler)
      return false;

   m_hdf5 = hdf5;
   m_handler = handler;

   vector<ossimIpt> validVertices;
   handler->getValidImageVertices(validVertices);
   ossimIrect validImageRect(validVertices);

   try
   {
      initCoarseGrid("Latitude",  validImageRect, theLatGrid);
      initCoarseGrid("Longitude", validImageRect, theLonGrid);
   }
   catch (ossimException& x)
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<x.what();
      return false;
   }

   // Check for dateline crossing among the longitude grid:
   crossesDateline();

   ossimGpt ulg (theLatGrid.maxValue(), theLonGrid.minValue());
   ossimGpt urg (theLatGrid.maxValue(), theLonGrid.maxValue());
   ossimGpt lrg (theLatGrid.minValue(), theLonGrid.maxValue());
   ossimGpt llg (theLatGrid.minValue(), theLonGrid.minValue());
   ossimDrect imageRect(validImageRect);
   ossimBilinearProjection* seedFunction =
         new ossimBilinearProjection(imageRect.ul(), imageRect.ur(), imageRect.lr(), imageRect.ll(),
                                     ulg, urg, lrg, llg);
   theSeedFunction = seedFunction;

   // Bileaner projection to handle
   initializeModelParams(imageRect);

   ossimIrect bounds (0, 0, theImageSize.u-1, theImageSize.v-1);
   initializeModelParams(bounds);

   return true;
}

bool ossimHdf5GridModel::initCoarseGrid(const char* datasetName, const ossimIrect& validRect,
                                        ossimDblGrid& coarseGrid)
{
   ostringstream xmsg;

   // Convention used: (u,v) is file space, (x,y) is CG space
   H5::DataSet* dataset  = m_hdf5->findDatasetByName(datasetName, 0, true);
   if (dataset == NULL)
   {
      xmsg  << "ossimHdf5GridModel:"<<__LINE__
            <<" ERROR: Could not find dataset \""<<datasetName<<"\" in file.";
      throw ossimException(xmsg.str());
   }

   ossimRefPtr<ossimHdf5ImageDataset> imageDataset = m_handler->getCurrentDataset();
   if (!imageDataset.valid())
      return false;

   //****************************************************************************************

   // Get dataspace of the dataset.
   H5::DataSpace dataSpace = dataset->getSpace();
   const ossim_int32 DIM_COUNT = dataSpace.getSimpleExtentNdims();
   if ( DIM_COUNT != 2  )
      return false;

   // Number of dimensions of the input dataspace:
   const ossim_uint32 imageRows = validRect.height();
   const ossim_uint32 imageCols = validRect.width();
   const ossim_uint32 GRID_SPACING = 4; // Only grab every 4th value.

   // Get the extents. dimsOut[0] is height, dimsOut[1] is width:
   std::vector<hsize_t> dimsOut(DIM_COUNT);
   dataSpace.getSimpleExtentDims( &dimsOut.front(), 0 );

   // Verify valid rect within our bounds:
   if ( (imageRows > dimsOut[0] ) || (imageCols > dimsOut[1] ) )
      return false;

   // Initialize the ossimDblGrid. Round up if size doesn't fall on end pixel.
   ossimDpt dspacing (GRID_SPACING, GRID_SPACING);
   ossim_uint32 gridRows = imageRows / GRID_SPACING + 1;
   ossim_uint32 gridCols = imageCols / GRID_SPACING + 1;
   if ( imageRows % GRID_SPACING)
      ++gridRows;
   if ( imageCols % GRID_SPACING)
      ++gridCols;
   ossimIpt gridSize (gridCols, gridRows);

   // The grid as used in base class, has UV-space always at 0,0 origin
   ossimDpt gridOrigin(0.0,0.0);
   coarseGrid.setNullValue(ossim::nan());
   coarseGrid.initialize(gridSize, gridOrigin, dspacing);

   std::vector<hsize_t> inputCount(DIM_COUNT);
   std::vector<hsize_t> inputOffset(DIM_COUNT);

   inputOffset[0] = 0; // y_img is set below.
   inputOffset[1] = validRect.ul().x; // x_img
   inputCount[0] = 1; // y_img
   inputCount[1] = (hsize_t)imageCols; // x_img

   // Output dataspace dimensions. Reading a line at a time.
   const ossim_int32 OUT_DIM_COUNT = 3;
   std::vector<hsize_t> outputCount(OUT_DIM_COUNT);
   outputCount[0] = 1;    // band
   outputCount[1] = 1;    // y_img
   outputCount[2] = imageCols; // x_img

   // Output dataspace offset.
   std::vector<hsize_t> outputOffset(OUT_DIM_COUNT);
   outputOffset[0] = 0;
   outputOffset[1] = 0;
   outputOffset[2] = 0;

   ossimScalarType scalar = imageDataset->getScalarType( );
   if ( scalar != OSSIM_FLOAT32 )
      return false;

   // See if we need to swap bytes:
   ossimEndian endian;
   bool needSwap = false;
   if (m_hdf5->getByteOrder(dataset) != ossim::byteOrder())
      needSwap = true;
   H5::DataType dataType = dataset->getDataType();

   // Output dataspace always the same, width of one line.
   H5::DataSpace bufferDataSpace( OUT_DIM_COUNT, &outputCount.front());
   bufferDataSpace.selectHyperslab( H5S_SELECT_SET,
                                    &outputCount.front(),
                                    &outputOffset.front() );

   //  Arrays to hold a single line of latitude longitude values.
   vector<ossim_float32> values(imageCols);
   ossim_float32 val = 0;
   hsize_t y_img = 0;
   const ossim_float64 NULL_VALUE = -999.0;

   // Line loop:
   for ( ossim_uint32 y = 0; y < gridRows; ++y )
   {
      // y_img = line in image space
      y_img = y*GRID_SPACING;
      if ( y_img < imageRows )
      {
         inputOffset[0] = y_img + validRect.ul().y;
         dataSpace.selectHyperslab( H5S_SELECT_SET, &inputCount.front(), &inputOffset.front() );

         // Read data from file into the buffer.
         dataset->read( &(values.front()), dataType, bufferDataSpace, dataSpace );
         if ( needSwap )
            endian.swap( &(values.front()), imageCols );

         // Sample loop:
         hsize_t x_img = 0;
         for ( ossim_uint32 x = 0; x < gridCols; ++x )
         {
            // x_img = sample in image space
            x_img = x*GRID_SPACING;
            if ( x_img < imageCols )
            {
               val = values[x_img];
               if (ossim::isnan(val)) // Nulls in grid!
               {
                  xmsg  << "ossimHdf5GridModel:"<<__LINE__<<" encountered nans!";
                  throw ossimException(xmsg.str());
               }
            }
            else // Last column is outside of image bounds.
            {
               // Delta between last two latitude grid values.
               ossim_float32 val1 = coarseGrid.getNode( x-2, y );
               ossim_float32 val2 = coarseGrid.getNode( x-1, y );
               ossim_float32 spacing = val2 - val1;
               val = val2 + spacing;

#if 0 /* Please leave for debug. (drb) */
               cout << "val1: " << val1 << " val2 " << val2<<endl;;
#endif
            }

            coarseGrid.setNode( x, y, val );

#if 0 /* Please leave for debug. (drb) */
            cout << "x,y,x_img,y_img,val:" << x << "," << y << ","<< x_img << "," << y_img << ","
                  << coarseGrid.getNode(x, y) << endl;
#endif
         } // End sample loop.
      }
      else // Row is outside of image bounds:
      {
         // Sample loop:
         for ( ossim_uint32 x = 0; x < gridCols; ++x )
         {
            ossim_float32 val = ossim::nan();
            ossim_float32 val1 = coarseGrid.getNode( x, y-2 );
            ossim_float32 val2 = coarseGrid.getNode( x, y-1 );
            ossim_float32 spacing = val2 - val1;
            val = val2 + spacing;
           coarseGrid.setNode( x, y, val );

#if 0 /* Please leave for debug. (drb) */
            hsize_t x_img = x*GRID_SPACING; // Sample in image space
            cout << "val1: " << val1 << " val2 " << val2
                  << "\nx,y,x_img,y_img,val:" << x << "," << y << ","
                  << x_img << "," << y_img << "," << val << endl;
#endif
         } // End sample loop.
      } // Matches if ( y_img < imageRows ){...}else{
   } // End line loop.

   dataSpace.close();

   return true;

#if 0
   // Original refactor code ###############################################

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
   string cname = dataType.fromClass();
   cout << cname<<endl;
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
      for ( x=0, u=0; (x<cgGridSize.x)&&(u<theImageSize.u); ++x, u+=GRID_SAMPLING_INTERVAL)
      {
         { // TODO REMOVE DEBUG BLOCK
            cout<<datasetName<<" ("<<x<<", "<<y<<"): "<<buffer[u]<<endl;
         }

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
#endif
}


bool ossimHdf5GridModel::crossesDateline()
{
   bool crossesDateline = false;

   ossim_int32 longitude = 0;
   bool found179 = false;
   bool found181 = false;

   ossimIpt size (theLonGrid.size());
   double left, right;
   int xr = size.x-1;
   for (ossim_uint32 y=0; (y<size.y) && !crossesDateline; ++y )
   {
      left  = theLonGrid.getNode(0,  y);
      right = theLonGrid.getNode(xr, y);
      if (left > right)
         crossesDateline = true;
   }

   if ( crossesDateline )
      theLonGrid.setDomainType(ossimDblGrid::WRAP_360);
   else
      theLonGrid.setDomainType(ossimDblGrid::WRAP_180);

   return crossesDateline;
}



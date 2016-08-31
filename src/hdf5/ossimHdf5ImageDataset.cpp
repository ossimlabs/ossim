//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: OSSIM HDF5 Image DataSet.
//
//----------------------------------------------------------------------------
// $Id

#include <ossim/hdf5/ossimHdf5ImageDataset.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/hdf5/ossimHdf5ImageHandler.h>

//---
// This includes everything!  Note the H5 includes are order dependent; hence,
// the mongo include.
//---
#include <hdf5.h>
#include <H5Cpp.h>

#include <iostream>

ossimHdf5ImageDataset::ossimHdf5ImageDataset(ossimHdf5ImageHandler* owner)
:  m_handler(owner),
   m_dataset(0),
   m_scalar(OSSIM_SCALAR_UNKNOWN),
   m_bands(1),
   m_lines(0),
   m_samples(0),
   m_endian(0)
{   
   if (owner)
      m_hdf5 = owner->m_hdf5;

   m_validRect.makeNan();
}

ossimHdf5ImageDataset::ossimHdf5ImageDataset( const ossimHdf5ImageDataset& obj )
:  m_handler(obj.m_handler),
   m_hdf5 (obj.m_hdf5),
   m_dataset(obj.m_dataset),
   m_scalar(obj.m_scalar),
   m_bands(obj.m_bands),
   m_lines(obj.m_lines),
   m_samples(obj.m_samples),
   m_validRect(obj.m_validRect),
   m_endian( obj.m_endian ? new ossimEndian() : 0 )
{
}

ossimHdf5ImageDataset::~ossimHdf5ImageDataset()
{
   close();
}

const ossimHdf5ImageDataset& ossimHdf5ImageDataset::operator=( const ossimHdf5ImageDataset& rhs )
{
   if ( this != &rhs )
   {
      m_dataset     = rhs.m_dataset;
      m_dataSpace   = rhs.m_dataSpace;
      m_scalar      = rhs.m_scalar;
      m_bands       = rhs.m_bands;
      m_lines       = rhs.m_lines;
      m_samples     = rhs.m_samples;
      m_validRect   = rhs.m_validRect;
      m_endian      = ( rhs.m_endian ? new ossimEndian() : 0 );
   }
   return *this;
}

bool ossimHdf5ImageDataset::initialize( const H5::DataSet& dataset)
{
   close();

   m_dataset = dataset;

   determineScalarType();

   if (!determineExtents() || !scanForValidImageRect() || !scanForMinMax())
      return false;

   return true;

} // End: ossimH5ImageDataset::initialize

bool ossimHdf5ImageDataset::determineExtents()
{
   // Find the valid image rect. dataset may have null padding:
   H5::DataSpace imageDataspace = m_dataset.getSpace();
   int rank = imageDataspace.getSimpleExtentNdims();
   if (rank < 2)
      return false;

   // Get the extents. Assuming dimensions are same for lat lon dataset.
   std::vector<hsize_t> inputSize(rank);
   imageDataspace.getSimpleExtentDims( &inputSize.front(), 0 );
   m_lines = inputSize[0];
   m_samples = inputSize[1];
   if ( rank >= 3 )
      m_bands = inputSize[2];
   else
      m_bands = 1;

   if ( (m_lines == 0) || (m_samples == 0) )
      return false;

   return true;
}

bool ossimHdf5ImageDataset::scanForValidImageRect()
{
   // Find the valid image rect. dataset may have null padding:
   H5::DataSpace imageDataspace = m_dataset.getSpace();
   H5::DataType dataType = m_dataset.getDataType();
   ossim_uint32 elem_size = dataType.getSize();

   // Get the extents. Assuming dimensions are same for lat lon dataset.
   hsize_t rowSize[2]  = { 1, m_samples };
   hsize_t imageOffset[2] = { 0, 0 };

   // Allocate space for read buffer:
   char *rowBuf = new char[elem_size*m_samples];
   char *fill_value = new char[elem_size];

   // Output dataspace always the same one line.
   H5::DataSpace bufferDataSpace( 2, rowSize);
   bufferDataSpace.selectHyperslab( H5S_SELECT_SET, rowSize, imageOffset ); // offset = (0,0) here

   // Figure out the null pixel value:
   H5:H5Pget_fill_value(m_dataset.getId(), dataType.getId(), fill_value);

   // Find the ul pixel. Loop over rows:
   ossimIpt ulIpt (0,0);
   bool found_valid = false;
   for (; (ulIpt.y<m_lines) && !found_valid; ulIpt.y++)
   {
      imageOffset[0] = ulIpt.y;
      imageDataspace.selectHyperslab( H5S_SELECT_SET, rowSize, imageOffset);
      m_dataset.read(rowBuf, dataType, bufferDataSpace, imageDataspace );

      // Scan row for valid pixel:
      ossim_int64 rowOffset = 0;
      for (ulIpt.x=0; (ulIpt.x<m_samples) && !found_valid; ulIpt.x++, rowOffset+=elem_size)
         found_valid = (memcmp(&rowBuf[rowOffset], fill_value, elem_size) != 0);
   }
   if (!found_valid)
      ulIpt = ossimIpt(0,0);

   // Find the lr pixel. Loop over rows:
   ossimIpt lrIpt (m_samples-1, m_lines-1);
   found_valid = false;
   for (; (lrIpt.y>ulIpt.y) && !found_valid; lrIpt.y--)
   {
      imageOffset[0] = lrIpt.y;
      imageDataspace.selectHyperslab( H5S_SELECT_SET, rowSize, imageOffset);
      m_dataset.read(rowBuf, dataType, bufferDataSpace, imageDataspace );

      // Scan row for valid pixel:
      ossim_int64 rowOffset = m_samples*elem_size - 1;
      for (lrIpt.x=m_samples-1; (lrIpt.x>ulIpt.x) && !found_valid; lrIpt.x-- , rowOffset-=elem_size)
         found_valid = (memcmp(&rowBuf[rowOffset], fill_value, elem_size) != 0);
   }
   if (!found_valid)
      lrIpt = ossimIpt (m_samples-1, m_lines-1);

   m_validRect.set_ul(ulIpt);
   m_validRect.set_lr(lrIpt);

   imageDataspace.close();

   delete [] rowBuf;
   delete [] fill_value;
   return true;
}

bool ossimHdf5ImageDataset::scanForMinMax()
{
   // Create buffer to hold the clip rect for a single band.
   ossimScalarType scalarType = getScalarType();
   if ((scalarType != OSSIM_FLOAT32) && (scalarType != OSSIM_FLOAT64) &&
       (scalarType != OSSIM_UINT32)  && (scalarType != OSSIM_SINT32)  &&
       (scalarType != OSSIM_UINT8)   && (scalarType != OSSIM_SINT8)   &&
       (scalarType != OSSIM_UINT16)  && (scalarType != OSSIM_SINT16))
   {
      return false;
   }

   ossim_uint32 bufSizeInBytes = m_validRect.width()*ossim::scalarSizeInBytes(scalarType);
   vector<char> dataBuffer(bufSizeInBytes);

   // Get the extents. Assuming dimensions are same for lat lon dataset.
   ossimIpt ulIpt (m_validRect.ul());
   ossimIpt lrIpt (m_validRect.lr());
   const ossim_float32 nullpix = m_handler->getNullPixelValue();
   ossim_float32 epsilon = 2*FLT_EPSILON;
   if (nullpix == 0.0)
      epsilon = 0;

   m_minValue.clear();
   m_maxValue.clear();

   ossimIrect clipRect (m_validRect.ul(), m_validRect.ur());
   for (int band=0; band<m_bands; ++band)
   {
      m_minValue.push_back(OSSIM_DEFAULT_MAX_PIX_FLOAT);
      m_maxValue.push_back(OSSIM_DEFAULT_MIN_PIX_FLOAT);

      for (int y=ulIpt.y; y<=lrIpt.y; y++)
      {
         clipRect.set_uly(y);
         clipRect.set_lry(y);

         getTileBuf(&dataBuffer.front(), clipRect, band);

         // Scan and fix non-standard null value:
         ossim_float32 value = 0;
         for ( int x=ulIpt.x; x<=lrIpt.x; ++x )
         {
            switch (scalarType)
            {
            case OSSIM_FLOAT32:
               value = ((ossim_float32*)&dataBuffer.front())[x];
               break;
            case OSSIM_FLOAT64:
               value = (ossim_float32) ((ossim_float64*)&dataBuffer.front())[x];
               break;
            case OSSIM_UINT8:
            case OSSIM_SINT8:
               value = (ossim_float32) ((char*)&dataBuffer.front())[x];
               break;
            case OSSIM_UINT16:
            case OSSIM_SINT16:
               value = (ossim_float32) ((ossim_int16*)&dataBuffer.front())[x];
               break;
            case OSSIM_UINT32:
            case OSSIM_SINT32:
               value = (ossim_float32) ((ossim_int32*)&dataBuffer.front())[x];
               break;
            }

            if (ossim::almostEqual<ossim_float32>(value, nullpix, epsilon))
               continue;
            if (value > m_maxValue[band])
               m_maxValue[band] = value;
            if (value < m_minValue[band])
               m_minValue[band] = value;
         }
      }
   }

   return true;
}

void ossimHdf5ImageDataset::close()
{
   m_dataset.close();
   delete m_endian;
}

const H5::DataSet* ossimHdf5ImageDataset::getDataset() const
{
   return &m_dataset;
}

H5::DataSet* ossimHdf5ImageDataset::getDataset()
{
   return &m_dataset;
}

string ossimHdf5ImageDataset::getName() const
{
   return m_dataset.getObjName();
}

ossimScalarType ossimHdf5ImageDataset::getScalarType() const
{
   return m_scalar;
}

bool ossimHdf5ImageDataset::determineScalarType()
{
   m_scalar = OSSIM_SCALAR_UNKNOWN;

   H5T_class_t typeClass = m_dataset.getTypeClass();
   if ( ( typeClass != H5T_INTEGER ) && ( typeClass != H5T_FLOAT ) )
      return false;

   hid_t mem_type_id = H5Dget_type( m_dataset.getId() );
   if( mem_type_id < 0 )
      return false;

   hid_t native_type = H5Tget_native_type(mem_type_id, H5T_DIR_DEFAULT);
   if( H5Tequal(H5T_NATIVE_CHAR, native_type) )
      m_scalar = OSSIM_SINT8;
   else if ( H5Tequal( H5T_NATIVE_UCHAR, native_type) )
      m_scalar = OSSIM_UINT8;
   else if( H5Tequal( H5T_NATIVE_SHORT, native_type) )
      m_scalar = OSSIM_SINT16;
   else if(H5Tequal(H5T_NATIVE_USHORT, native_type))
      m_scalar = OSSIM_UINT16;
   else if(H5Tequal( H5T_NATIVE_INT, native_type))
      m_scalar = OSSIM_SINT32;
   else if(H5Tequal( H5T_NATIVE_UINT, native_type ) )
      m_scalar = OSSIM_UINT32;
   else if(H5Tequal( H5T_NATIVE_LONG, native_type))
      m_scalar = OSSIM_SINT32;
   else if(H5Tequal( H5T_NATIVE_ULONG, native_type))
      m_scalar = OSSIM_UINT32;
   else if(H5Tequal( H5T_NATIVE_LLONG, native_type))
      m_scalar = OSSIM_SINT64;
   else if(H5Tequal( H5T_NATIVE_ULLONG, native_type))
      m_scalar = OSSIM_UINT64;
   else if(H5Tequal( H5T_NATIVE_FLOAT, native_type))
      m_scalar = OSSIM_FLOAT32;
   else if(H5Tequal( H5T_NATIVE_DOUBLE, native_type))
      m_scalar = OSSIM_FLOAT64;

   // See if we need to swap bytes:
   if (m_hdf5->getByteOrder(&m_dataset) != ossim::byteOrder())
      m_endian = new ossimEndian();

   return true;
}

ossim_uint32 ossimHdf5ImageDataset::getNumberOfBands() const
{
   return m_bands;
}

ossim_uint32 ossimHdf5ImageDataset::getNumberOfLines() const
{
   return m_validRect.height();
}

ossim_uint32 ossimHdf5ImageDataset::getNumberOfSamples() const
{
   return m_validRect.width();
}

bool ossimHdf5ImageDataset::getSwapFlag() const
{
   return (m_endian ? true: false);
}

const ossimIpt& ossimHdf5ImageDataset::getSubImageOffset() const
{
   return m_validRect.ul();
}

const ossimIrect& ossimHdf5ImageDataset::getValidImageRect() const
{
   return m_validRect;
}

void ossimHdf5ImageDataset::getTileBuf(void* buffer, const ossimIrect& rect, ossim_uint32 band)
{
   static const char MODULE[] = "ossimH5ImageDataset::getTileBuf";

   try
   {
      // Shift rectangle by the sub image offse (if any) from the m_validRect.
      ossimIrect irect = rect + m_validRect.ul();

      //--
      // Turn off the auto-printing when failure occurs so that we can
      // handle the errors appropriately
      //---
      // H5::Exception::dontPrint();

      // NOTE: rank == array dimensions in hdf5 documentation lingo.

      // Get dataspace of the dataset.
      H5::DataSpace imageDataSpace = m_dataset.getSpace();

      // Number of dimensions of the input dataspace.:
      const ossim_int32 IN_DIM_COUNT = imageDataSpace.getSimpleExtentNdims();

      // Native type:
      H5::DataType dataType = m_dataset.getDataType();

      std::vector<hsize_t> inputCount(IN_DIM_COUNT);
      std::vector<hsize_t> inputOffset(IN_DIM_COUNT);

      if ( IN_DIM_COUNT == 2 )
      {
         inputOffset[0] = irect.ul().y;
         inputOffset[1] = irect.ul().x;

         inputCount[0] = irect.height();
         inputCount[1] = irect.width();
      }
      else
      {
         inputOffset[0] = band;
         inputOffset[1] = irect.ul().y;
         inputOffset[2] = irect.ul().x;

         inputCount[0] = 1;
         inputCount[1] = irect.height();
         inputCount[2] = irect.width();
      }

      // Define hyperslab in the dataset; implicitly giving strike strike and block NULL.
      imageDataSpace.selectHyperslab( H5S_SELECT_SET,
                                      &inputCount.front(),
                                      &inputOffset.front() );

      // Output dataspace dimensions.
      const ossim_int32 OUT_DIM_COUNT = 3;
      std::vector<hsize_t> outputCount(OUT_DIM_COUNT);
      outputCount[0] = 1;             // single band
      outputCount[1] = irect.height(); // lines
      outputCount[2] = irect.width();  // samples

      // Output dataspace offset.
      std::vector<hsize_t> outputOffset(OUT_DIM_COUNT);
      outputOffset[0] = 0;
      outputOffset[1] = 0;
      outputOffset[2] = 0;

      // Output dataspace.
      H5::DataSpace bufferDataSpace( OUT_DIM_COUNT, &outputCount.front());
      bufferDataSpace.selectHyperslab( H5S_SELECT_SET,
                                       &outputCount.front(),
                                       &outputOffset.front() );

      // Read data from file into the buffer.
      m_dataset.read( buffer, dataType, bufferDataSpace, imageDataSpace );

      if ( m_endian )
      {
         // If the m_endian pointer is initialized(not zero) swap the bytes.
         m_endian->swap( m_scalar, buffer, irect.area() );
      }

      // Cleanup:
      bufferDataSpace.close();
      dataType.close();
      imageDataSpace.close();

      // memSpace.close();
      // dataType.close();
      // dataSpace.close();
   }
   catch( const H5::FileIException& error )
   {
      ossimNotify(ossimNotifyLevel_WARN)
                                                << MODULE << " caught H5::FileIException!" << std::endl;
      error.printError();
   }

   // catch failure caused by the DataSet operations
   catch( const H5::DataSetIException& error )
   {
      ossimNotify(ossimNotifyLevel_WARN)
                                                << MODULE << " caught H5::DataSetIException!" << std::endl;
      error.printError();
   }

   // catch failure caused by the DataSpace operations
   catch( const H5::DataSpaceIException& error )
   {
      ossimNotify(ossimNotifyLevel_WARN)
                                                << MODULE << " caught H5::DataSpaceIException!" << std::endl;
      error.printError();
   }

   // catch failure caused by the DataSpace operations
   catch( const H5::DataTypeIException& error )
   {
      ossimNotify(ossimNotifyLevel_WARN)
                                                << MODULE << " caught H5::DataTypeIException!" << std::endl;
      error.printError();
   }
   catch( ... )
   {
      ossimNotify(ossimNotifyLevel_WARN)
                                                << MODULE << " caught unknown exception !" << std::endl;
   }

} // End: ossimH5ImageDataset::getTileBuf


double ossimHdf5ImageDataset::getMaxPixelValue(ossim_uint32 band) const
{
   if (band < m_bands)
      return m_maxValue[band];
   return 0;
}

double ossimHdf5ImageDataset::getMinPixelValue(ossim_uint32 band) const
{
   if (band < m_bands)
      return m_minValue[band];
   return 0;
}

std::ostream& ossimHdf5ImageDataset::print( std::ostream& out ) const
{
   out << "ossimH5ImageDataset: "
         << "\nH5::DataSet::id: " << m_dataset.getId()
         << "\nname:            " << m_dataset.getObjName()
         << "\nscalar:          " << ossimScalarTypeLut::instance()->getEntryString( m_scalar )
         << "\nbands:           " << m_bands
         << "\nlines:           " << m_lines
         << "\nsamples:         " << m_samples
         << "\nvalid rect:      " << m_validRect
         << "\nswap_flage:      " << (m_endian?"true":"false")
         << std::endl;
   return out;
}

std::ostream& operator<<( std::ostream& out, const ossimHdf5ImageDataset& obj )
{
   return obj.print( out );
}


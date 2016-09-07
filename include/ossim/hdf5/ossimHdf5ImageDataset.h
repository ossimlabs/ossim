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

#ifndef ossimHdf5ImageDataset_HEADER
#define ossimHdf5ImageDataset_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIrect.h>
#include <iosfwd>
#include <string>
#include <ossim/hdf5/ossimHdf5.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimRefPtr.h>

// Forward class declarations:
class ossimImageData;
class ossimEndian;
class ossimIpt;
class ossimIrect;
class ossimHdf5ImageHandler;

/**
 * @brief Class encapsulates a HDF5 Data set that can be loaded as an image.
 */
class OSSIM_PLUGINS_DLL ossimHdf5ImageDataset : public ossimReferenced
{
public:
   /** default constructor */
   ossimHdf5ImageDataset(ossimHdf5ImageHandler* owner=0);
   
   /** copy constructor */
   ossimHdf5ImageDataset( const ossimHdf5ImageDataset& obj );
   
   /** destructor */
   ~ossimHdf5ImageDataset();

   /** @brief Calls H5::DataSet::close then deletes data set. */
   void close();

   const ossimHdf5ImageDataset& operator=( const ossimHdf5ImageDataset& rhs );

   /**
    * @brief Opens datasetName and initializes all data members on success.
    * @return true on success, false on error.
    */
   bool initialize( const H5::DataSet& dataset);

   /**
    * @brief Get const pointer to dataset.
    *
    * This can be null if not open.
    * 
    * @return const pointer to dataset.
    */
   const H5::DataSet* getDataset() const;
   
   /**
    * @brief Get pointer to dataset.
    *
    * This can be null if not open.
    * 
    * @return pointer to dataset.
    */
   H5::DataSet* getDataset();

   /** @return The dataset name.  This is the full path used for open. */
   std::string getName() const;

   /** @return The output scalar type. */
   ossimScalarType getScalarType() const;

   /** @return the number of . */
   ossim_uint32 getNumberOfBands() const;

   /** @return The number of lines. */
   ossim_uint32 getNumberOfLines() const;
   
   /** @return The number of samples. */
   ossim_uint32 getNumberOfSamples() const;

   /** @return The swap flag. */
   bool getSwapFlag() const;

   const ossimIpt& getSubImageOffset() const;

   const ossimIrect& getValidImageRect() const;

   double getMaxPixelValue(ossim_uint32 band=0) const;
   double getMinPixelValue(ossim_uint32 band=0) const;

   /**
    *  @brief Method to grab a tile(rectangle) from image.
    *
    *  @param buffer Buffer for data for this method to copy data to.
    *  Should be the size of rect * bytes_per_pixel for scalar type.
    *
    *  @param rect The zero based rectangle to grab.  Rectangle is relative to
    *  any sub image offset. E.g. A request for 0,0 is the upper left corner
    *  of the valid image rect.
    *
    *  @param scale If true, uses min and max to stretch the data to a UINT16 range.
    */
   void getTileBuf(void* buffer, const ossimIrect& rect, ossim_uint32 band, bool scale=true);

   /**
    * @brief print method.
    * @return std::ostream&
    */
   std::ostream& print(std::ostream& out) const;

   friend OSSIMDLLEXPORT std::ostream& operator<<(std::ostream& out,
                                                  const ossimHdf5ImageDataset& obj);
private:
   /** Returns true if datasets's endianness differs from this platform */
   bool determineExtents();
   bool scanForValidImageRect();
   bool determineScalarType();
   bool scanForMinMax();

   ossimRefPtr<ossimHdf5ImageHandler> m_handler;
   ossimRefPtr<ossimHdf5> m_hdf5;
   H5::DataSet     m_dataset;
   H5::DataSpace   m_dataSpace;
   ossimScalarType m_scalar;
   ossim_uint32    m_bands;   
   ossim_uint32    m_lines;
   ossim_uint32    m_samples;
   ossimEndian*    m_endian; // For byte swapping if needed.
   std::vector<ossim_float32> m_minValue;
   std::vector<ossim_float32> m_maxValue;


   /** H5 data can have null rows on the front or end.  The valid rect is the scanned rectangle
    * disregarding leading or trailing nulls. This doesn't handle nulls in the middle of image. */
   ossimIrect      m_validRect; // image rect offset relative to full image
   
}; // End: class ossimH5ImageDataset

#endif /* #ifndef ossimH5ImageDataset_HEADER */

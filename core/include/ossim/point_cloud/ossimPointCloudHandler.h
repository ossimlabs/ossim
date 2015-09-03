//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id$

#ifndef ossimPointCloudHandler_HEADER
#define ossimPointCloudHandler_HEADER

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/point_cloud/ossimPointCloudSource.h>
#include <ossim/point_cloud/ossimPointBlock.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/point_cloud/ossimPointCloudGeometry.h>
#include <vector>


/**
 * Base class for all point-cloud file readers
 */
class OSSIMDLLEXPORT ossimPointCloudHandler: public ossimPointCloudSource
{
public:
   static ossim_uint32 DEFAULT_BLOCK_SIZE;

   ossimPointCloudHandler();
   ossimPointCloudHandler(ossimObject* owner);

   virtual ~ossimPointCloudHandler();

   /**
    *  @brief Method to open input point cloud dataset.
    *  @param imageFile Name of point data file to be opened. (or basename for multi-file sets.)
    *  @return Returns true on success, false on error.
    */
   virtual bool open(const ossimFilename& imageFile) = 0;

   /**
    * Closes the point cloud file(s).
    */
   virtual void close() = 0;

   /**
    * Fetches up to maxNumPoints points starting at the dataset's <offset> in the order they
    * appear in the data file. Thread-safe implementation accepts an allocated block from the
    * calling method. caller can check block.size() to see if block is full.
    */
   virtual void getFileBlock(ossim_uint32 offset,
                             ossimPointBlock& block,
                             ossim_uint32 maxNumPoints=0xFFFFFFFF) const = 0;

   /**
    * @see getFileBlock.
    */
   virtual void getNextFileBlock(ossimPointBlock& block,
                                 ossim_uint32 maxNumPoints=0xFFFFFFFF) const;

   virtual void rewind() const { m_currentPID = 0; }

   /**
    * Fetches the block of points inside the block bounds. If the height components of the bounds
    * are NaN, then only the horizontal bounds are considered. Thread-safe version accepts data
    * block object from caller. The block object is cleared before points are pushed on the vector.
    * The block size will be non-zero if points were found.
    */
   virtual void getBlock(const ossimGrect& bounds, ossimPointBlock& block) const;

   virtual const ossimPointRecord*  getMinPoint() const { return m_minRecord.get(); }
   virtual const ossimPointRecord*  getMaxPoint() const { return m_maxRecord.get(); }

   virtual void getBounds(ossimGrect& bounds) const;

   /**
    * Handler is always on the start of the pipeline
    */
   virtual bool canConnectMyInputTo(ossim_int32, const ossimConnectableObject*) const
   { return false; }

   void normalizeBlock(ossimPointBlock& block);

protected:

   ossimFilename m_inputFilename;
   ossimRefPtr<ossimPointCloudGeometry> m_geometry;
   ossimRefPtr<ossimPointRecord> m_minRecord;
   ossimRefPtr<ossimPointRecord>  m_maxRecord;
   mutable ossim_uint32 m_currentPID;

TYPE_DATA
};

#endif /* ossimPointCloudHandler_HEADER */

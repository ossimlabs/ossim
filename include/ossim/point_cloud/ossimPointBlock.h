//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  MIT -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
#ifndef ossimPointBlock_HEADER
#define ossimPointBlock_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h> /* for ossim::isnan */
#include <ossim/base/ossimDataObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/point_cloud/ossimPointRecord.h>
#include <vector>

class ossimSource;

/***************************************************************************************************
 * Base class for all Point Cloud data sources, including input file handlers and processing nodes.
 **************************************************************************************************/
class OSSIMDLLEXPORT ossimPointBlock: public ossimDataObject
{
public:
   typedef std::vector< ossimRefPtr<ossimPointRecord> > PointList;

   explicit ossimPointBlock(ossimSource* owner=0, ossim_uint32 fields=0);

   ~ossimPointBlock();

   /** Returns allocated size. The pointList may contain only null points if not assigned */
   virtual ossim_uint32 size() const { return (ossim_uint32)m_pointList.size(); }

   bool empty() const { return (size() == 0); }

   /**
    * Returns OR'd mash-up of ossimPointRecord field codes being stored (or desired to be stored)
    */
   ossim_uint32 getFieldCode() const;
   std::vector<ossimPointRecord::FIELD_CODES> getFieldCodesAsList() const;

   /**
    * Initializes the desired fields to be stored. This will affect future getBlock() calls. If
    * the point list contains points from prior read, they will be deleted unless the points field
    * code matches the code argument.
    */
   void setFieldCode(ossim_uint32 code);

   /** Adds single point to the tail of the list. */
   virtual void addPoint(ossimPointRecord* point);

   virtual const ossimPointRecord* getPoint(ossim_uint32 point_offset) const;
   virtual ossimPointRecord* getPoint(ossim_uint32 point_offset);

   const ossimPointRecord* operator[](ossim_uint32 i) const { return getPoint(i); }
   ossimPointRecord* operator[](ossim_uint32 i) { return getPoint(i); }

   virtual const PointList&  getPoints() const { return m_pointList; }
   virtual PointList&  getPoints()  { return m_pointList; }

   void getFieldMin(ossimPointRecord::FIELD_CODES field, ossim_float32& value) const;
   void getFieldMax(ossimPointRecord::FIELD_CODES field, ossim_float32& value) const;
   void getBounds(ossimGrect& block_bounds) const;

   virtual const ossimPointBlock& operator=(const ossimPointBlock &rhs);

   virtual ossimObject* dup() const;

   /** Resets any storage to empty. */
   virtual void clear() { m_pointList.clear(); m_isNormalized = false; m_minMaxValid = false;}

   /**
    *  Fulfills base class pure virtual. TODO: Needs to be correctly implemented
    */
   virtual bool isEqualTo(const ossimDataObject& /*rhs*/, bool /*deep_copy*/) const { return false; }
   virtual ossim_uint32 getHashId() const { return 0; }
   virtual ossim_uint64 getDataSizeInBytes() const { return 0; };
   virtual void initialize() {};

protected:
   ossimPointBlock(const ossimPointBlock& /* rhs */) {}
   void scanForMinMax() const;

   ossimPointRecord m_nullPCR;
   mutable ossimPointRecord m_minRecord;
   mutable ossimPointRecord m_maxRecord;
   mutable bool m_minMaxValid;
   PointList m_pointList;
   mutable ossim_uint32 m_fieldCode; // OR'd mash-up of ossimPointRecord::FIELD_CODES
   bool m_isNormalized;

TYPE_DATA
};

#endif /* #ifndef ossimPointBlock_HEADER */

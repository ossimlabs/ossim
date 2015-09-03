//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id: ossimPointRecord.h 23352 2015-05-29 17:38:12Z okramer $

#ifndef ossimPointCloudRecord_HEADER
#define ossimPointCloudRecord_HEADER

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimGpt.h>
#include <vector>
#include <map>

/***************************************************************************************************
 * This class represents a single sample (point) in a point cloud.
 **************************************************************************************************/
class OSSIMDLLEXPORT ossimPointRecord : public ossimReferenced
{
public:
   // Field codes intended to be OR'd to indicate fields present in point record. All point records
   // have at least an X, Y, Z position so those fields are implied.
   enum FIELD_CODES // and associated types
   {
      Intensity       = 0x0008, // float 32
      ReturnNumber    = 0x0010, // unsigned int 32
      NumberOfReturns = 0x0020, // unsigned int 32
      Red             = 0x0040, // float 32
      Green           = 0x0080, // float 32
      Blue            = 0x0100, // float 32
      GpsTime         = 0x0200, // unsigned long Unix epoch (microsec from 01/01/1970)
      Infrared        = 0x0400, // float 32
      All             = 0x0777
   };

   ossimPointRecord(ossim_uint32 fields_code=0);
   ossimPointRecord(const ossimPointRecord& pcr);
   ossimPointRecord(const ossimGpt& pos);

   /** Creates a point record with fields set to null values */
   ~ossimPointRecord();

   ossimPointRecord& operator=(const ossimPointRecord& pcr);

   ossim_uint32 getPointId() const { return m_pointId; }
   void         setPointId(ossim_uint32 id) {m_pointId = id; }

   /**
    * Returns the 3D position vector in the dataset's coodinate reference system (available from
    * the ossimPointCloudSource object. This is either map/meters (x, y, z) or geog (lat, lon, hgt).
    * Refer to the ossimPointCloudGeometry::getStoredPosFormat() available from the associated
    * ossimPointCloudSource->getGeometry()
    */
   const ossimGpt& getPosition() const { return m_position; }
   void setPosition(const ossimGpt& p)  { m_position = p; }

   /** Argument can be mash-up of OR'd codes for check of multiple fields. Returns TRUE if ALL
    * fields are present. */
   virtual bool hasFields(ossim_uint32 code_mashup) const;

   /** Returns mash-up of OR'd codes of multiple fields being stored*/
   virtual ossim_uint32 getFieldCode() const;

   /** Return the float value of the requested field */
   ossim_float32 getField(FIELD_CODES fc) const;

   const std::map<FIELD_CODES, ossim_float32>& getFieldMap() const { return m_fieldMap; }

   void setField(FIELD_CODES fc, ossim_float32 value);

   friend std::ostream& operator << (std::ostream& ostr, const ossimPointRecord& p);

   /**
    * Initializes point to undefined state:
    */
   void clear();
   bool isValid() const;

protected:
   ossim_uint32 m_pointId;
   ossimGpt     m_position;
   std::map<FIELD_CODES, ossim_float32> m_fieldMap;
};

#endif /* ossimPointCloudRecord_HEADER */

//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id$

#ifndef ossimPointCloudGeometry_HEADER
#define ossimPointCloudGeometry_HEADER

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimDatumFactory.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/projection/ossimMapProjection.h>


/***************************************************************************************************
 * This class represents the 3D geometry of the point cloud dataset.
 * NOTE: See below for details on position format. It is the responsibility of the derived
 * ossimPointCloudHandlers to convert the ray point positions into either geographic or map-meters
 * before storing a point in ossimPointCloudRecord.
 **************************************************************************************************/
class OSSIMDLLEXPORT ossimPointCloudGeometry : public ossimObject
{
public:
   enum PositionFormat
   {
      UNKNOWN,
      GEOGRAPHIC,     // x=lon, y=lat, z=ellipsoid height
      MAP_PROJECTED, // x=easting, y=northing, z=ellipsoid height
      ECF_METERS      // X, Y, Z (see base/ossimEcef)
   };

   /**
    * Constructs from WKT specification. Since the projection units are unknown
    * when the bounds are established, the caller will not know the units of the point coordinates
    * for establishing the bounds.
    */
   ossimPointCloudGeometry(const ossimString& wkt);

   /**
    * If arg left to default GEOGRAPHIC, this geometry is essentially unity transform except for a
    * possible datum change if requested.
    */
   ossimPointCloudGeometry(PositionFormat f=GEOGRAPHIC)
         : m_format(f),
           m_projection(0),
           m_datum (ossimDatumFactory::instance()->wgs84()) {}

   ~ossimPointCloudGeometry() {}

   bool operator==(const ossimPointCloudGeometry& other) const;

   /**
   * While the raw point positions may be specified in the data file as either geographic, map
   * projected (feet or meters), or ECF meters, the point positions are stored in OSSIM as either as
   * geographic or map (meters) only. This method is used by the Point cloud handler object for
   * interpreting the input file's raw position format.
   */
   PositionFormat getRawPosFormat() const { return m_format; }

   /**
    * This method will return either GEOGRAPHIC or MAP_PROJECTION. For the latter, the stored
    * coordinates will be in meters.
    */
   PositionFormat getStoredPosFormat() const { return (m_format==ECF_METERS) ? GEOGRAPHIC : m_format; }

   const ossimMapProjection* getProjection()   const { return m_projection.get(); }
   const ossimDatum*         getDatum()        const { return m_datum; }

   /**
    * Method converts a generic 3D point as stored in the argument ossim3Dpt object,
    * and converts it to an ossimGpt, using the preset datum provided in the argument gpt.
    */
   void convertPos(const ossimDpt3d& stored_pos, ossimGpt& converted_gpos) const;

private:
   PositionFormat m_format;
   ossimRefPtr<ossimMapProjection> m_projection;
   const ossimDatum* m_datum;

TYPE_DATA
};

#endif /* ossimPointCloudGeometry_HEADER */

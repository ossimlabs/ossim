//**************************************************************************************************
//
// OSSIM (http://trac.osgeo.org/ossim/)
//
// License:  LGPL -- See LICENSE.txt file in the top level directory for more details.
//
//**************************************************************************************************
// $Id: ossimRangeDomeTileSource.h 23464 2015-08-07 18:39:47Z okramer $

#ifndef ossimRangeDomeTileSource_HEADER
#define ossimRangeDomeTileSource_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimGrect.h>
#include <vector>

class ossimImageData;

struct ossimRangeDome
{
   enum Classification
   {
      UNASSIGNED = 0,
      FRIENDLY_UNSPECIFIED = 1,
      FRIENDLY_LEVEL_1= 2,
      FRIENDLY_LEVEL_2 = 3,
      FRIENDLY_LEVEL_3 = 4,
      THREAT_UNSPECIFIED = 101,
      THREAT_LEVEL_1 = 102,
      THREAT_LEVEL_2 = 103,
      THREAT_LEVEL_3 = 104
   };

   ossimRangeDome(std::vector<ossimString>& tokens);
   ossimGrect boundingRect() const;

   bool valid;
   ossim_uint32 id;
   ossimGpt centerGpt;
   ossimDpt centerIpt;
   double radiusMeters;
   ossim_uint32 radiusPixelsSq;
   Classification classification;
   double startAz;
   double endAz;
   ossimString description;
};


/**
 * Class used for rendering range domes (a.k.a. "threat domes") as 2-D ortho-images.
 *
 * This base class has the rudimentary capability of reading range dome specifications as a CSV file
 * in the following format (one line per dome object):
 *
 *    id, lat, lon, hgt, R, C [,Az0, Az1] [, description]
 *
 * where
 *
 *    id -- integer ID of data object
 *    lat, lon, hgt -- Location of center of dome (actually sphere) in WGS84
 *    R -- radius (in meters) of dome boundary
 *    C -- classification id (unsigned 8-bit) corresponding to area inside r
 *    Az0, Az1 -- [optional] starting and ending azimuth (clockwise from Az0 in degrees). If both
 *         Az0 and Az1 are equal, then 360 deg is implied.
 *    description -- [optional] string
 */
class OSSIMDLLEXPORT ossimRangeDomeTileSource : public ossimImageHandler
{
public:
   ossimRangeDomeTileSource();
   virtual ~ossimRangeDomeTileSource();

   /**  @brief Reads CSV file representing range domes.  */
   virtual bool open();

   virtual bool isOpen() const;

   virtual void close();

   virtual void initialize();

   virtual ossimRefPtr<ossimImageData> getTile(const  ossimIrect& rect, ossim_uint32 resLevel=0);
   virtual bool getTile(ossimImageData* result, ossim_uint32 resLevel=0);

   virtual ossim_uint32    getNumberOfInputBands() const;
   virtual ossim_uint32    getNumberOfLines(ossim_uint32 resLevel = 0) const;
   virtual ossim_uint32    getNumberOfSamples(ossim_uint32 resLevel = 0) const;
   virtual ossim_uint32    getImageTileWidth() const { return 0; }
   virtual ossim_uint32    getImageTileHeight() const { return 0; }
   virtual ossimScalarType getOutputScalarType() const { return OSSIM_UINT8; }


   virtual ossimRefPtr<ossimImageGeometry> getImageGeometry() { return theGeometry; }

   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0) const;
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

   /**
    * The reader properties are:
    * -- the GSD ("meters_per_pixel")
    */
   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name) const;

   virtual void getGSD(ossimDpt& gsd, ossim_uint32 resLevel) const;
   virtual void setGSD( const ossim_float64& gsd );

   ossim_uint32 getNumRangeDomes() const { return (ossim_uint32)m_rangeDomes.size(); }

protected:
   std::vector<ossimRangeDome>       m_rangeDomes;
   ossimDpt                     m_gsd;

   static const ossimString OSSIM_RANGE_DOME_SPEC_MAGIC_NUMBER;

   TYPE_DATA
};

#endif

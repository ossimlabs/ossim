//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// ossimWkt class declaration. A well known text(WKT) utility class.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimWkt_HEADER
#define ossimWkt_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimKeywordlist.h>

#include <iosfwd>
#include <string>
#include <vector>

/**
 * @class ossimWkt
 *
 * Utility/support data class to parse WKT text string to an ossimKeywordlist.
 *
 * Example keyword list( WKT string dependent ):
 *
 * PROJCS.AUTHORITY.name:  EPSG
 * PROJCS.AUTHORITY.param0:  "32641"
 * PROJCS.GEOGCS.AUTHORITY.name:  EPSG
 * PROJCS.GEOGCS.AUTHORITY.param0:  "4326"
 * PROJCS.GEOGCS.DATUM.AUTHORITY.name:  EPSG
 * PROJCS.GEOGCS.DATUM.AUTHORITY.param0:  "6326"
 * PROJCS.GEOGCS.DATUM.SPHEROID.AUTHORITY.name:  EPSG
 * PROJCS.GEOGCS.DATUM.SPHEROID.AUTHORITY.param0:  "7030"
 * PROJCS.GEOGCS.DATUM.SPHEROID.name:  WGS 84
 * PROJCS.GEOGCS.DATUM.SPHEROID.param0:  6378137
 * PROJCS.GEOGCS.DATUM.SPHEROID.param1:  298.2572235630016
 * PROJCS.GEOGCS.DATUM.name:  WGS_1984
 * PROJCS.GEOGCS.PRIMEM.name:  Greenwich
 * PROJCS.GEOGCS.PRIMEM.param0:  0
 * PROJCS.GEOGCS.UNIT.name:  degree
 * PROJCS.GEOGCS.UNIT.param0:  0.0174532925199433
 * PROJCS.GEOGCS.name:  WGS 84
 * PROJCS.PARAMETER0.name:  latitude_of_origin
 * PROJCS.PARAMETER0.param0:  0
 * PROJCS.PARAMETER1.name:  central_meridian
 * PROJCS.PARAMETER1.param0:  63
 * PROJCS.PARAMETER2.name:  scale_factor
 * PROJCS.PARAMETER2.param0:  0.9996
 * PROJCS.PARAMETER3.name:  false_easting
 * PROJCS.PARAMETER3.param0:  500000
 * PROJCS.PARAMETER4.name:  false_northing
 * PROJCS.PARAMETER4.param0:  0
 * PROJCS.PROJECTION.name:  Transverse_Mercator
 * PROJCS.UNIT.AUTHORITY.name:  EPSG
 * PROJCS.UNIT.AUTHORITY.param0:  "9001"
 * PROJCS.UNIT.name:  metre
 * PROJCS.UNIT.param0:  1
 * PROJCS.name:  WGS 84 / UTM zone 41N 
 *
 */
class OSSIM_DLL ossimWkt
{
public:
   
   /** default constructor */
   ossimWkt();

   /** destructor */
   ~ossimWkt();

   /**
    * @brief Parses string to keyword list.
    * @param wkt String to parse.
    */
   bool parse( const std::string& wkt );

   /** @return Refeerence to keyword list. */
   const ossimKeywordlist& getKwl() const;

private:
   bool parseWktGroup( std::istringstream& is, ossimKeywordlist& kwl );

   bool parseObject( std::istringstream& is,
                     const std::string& prefix,
                     const std::string& object,
                     ossimKeywordlist& kwl );
   
   bool parseName( std::istringstream& is,
                   const std::string& prefix,
                   const std::string& object,
                   ossimKeywordlist& kwl );
   
   bool parseParam( std::istringstream& is,
                    const std::string& prefix,
                    const std::string& object,
                    ossim_uint32& objectIndex,
                    ossim_uint32& paramIndex,
                    ossimKeywordlist& kwl );

   ossimKeywordlist m_kwl;

};

#endif /* End of "#ifndef ossimWkt_HEADER" */

//---
//
// License: MIT
// Author:  David Burken
// Description: Interface for Kakadu compressor.
// 
//---
// $Id$

#ifndef ossimKakaduCompressorInterface_HEADER
#define ossimKakaduCompressorInterface_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIosFwd.h>
#include <ossim/base/ossimRefPtr.h>

class ossimImageData;
class ossimIpt;
class ossimIrect;
class ossimNitfJ2klraTag;
class ossimProperty;

/**
 * @class GeoPackage writer interface.
 *
 * This interface is for using the ossimKakaduCompressor outside of the
 * kakadu plugin.
 */
class OSSIM_DLL ossimKakaduCompressorInterface
{
public:

   // Matches static "COMPRESSION_QUALITY" string array in .cpp.
   enum ossimKakaduCompressionQuality
   {
      // Prefixed with OKP for OSSIM Kakadu Plugin to avoid clashes.
      OKP_UNKNOWN              = 0,
      OKP_USER_DEFINED         = 1,
      OKP_NUMERICALLY_LOSSLESS = 2,
      OKP_VISUALLY_LOSSLESS    = 3,
      OKP_LOSSY                = 4,
      OKP_LOSSY2               = 5,
      OKP_LOSSY3               = 6,
      OKP_EPJE                 = 7,  // Exploitation Preferred J2K Encoding      
   };

   /**
    * GP:  I had to add this or windows would not link with the latest
    * compiler. Also had to put in dot.cpp for debug mode(again windows).
    * (drb)
    */
   ossimKakaduCompressorInterface();

   /**
    * @brief Create method.
    * @param os Stream to write to.
    * @param scalar Scalar type of source tiles to be fed to compressor.
    * @param bands Number of bands in source tiles to be fed to compressor.
    * @param imageRect The image rectangle.
    * @param tileSize The size of a tile.
    * @param tilesTileWrite The number of tiles to be written.
    * If zero, the tlm marker segment will not be used.
    * @param jp2 If true jp2 header and jp2 geotiff block will be written out.
    * @note Throws ossimException on error.
    */
   virtual void create(ossim::ostream* os,
                       ossimScalarType scalar,
                       ossim_uint32 bands,
                       const ossimIrect& imageRect,
                       const ossimIpt& tileSize,
                       ossim_uint32 tilesToWrite,
                       bool jp2) = 0;

   /**
    * @brief Write tile method.
    *
    * Writes tiles stream provided to create method.  Note that tiles should
    * be fed to compressor in left to right, top to bottom order.
    * 
    * @param srcTile The source tile to write.
    *
    * @return true on success, false on error.
    */
   virtual bool writeTile(ossimImageData& srcTile) = 0;

   /**
    * @brief Finish method.  Every call to "create" should be matched by a
    * "finish".  Note the destructor calls finish.
    */
   virtual void finish() = 0;

   /**
    * Set the writer to add an alpha channel to the output.
    *
    * @param flag true to create an alpha channel.
    */
   virtual void setAlphaChannelFlag( bool flag ) = 0;

   /**
    * @brief Sets the number of levels.
    *
    * This must be positive and at least 1.
    * Default = 5 ( r0 - r5 )
    *
    * @param levels Levels to set.
    */
   virtual void setLevels(ossim_int32 levels) = 0;

   /**
    * Will set the property whose name matches the argument
    * "property->getName()".
    *
    * @param property Object containing property to set.
    *
    * @return true if property was consumed, false if not.
    */
   virtual bool setProperty(ossimRefPtr<ossimProperty> property) = 0;

   /**
    * @brief Sets the quality type.
    *
    * Type enumerations:
    *   OKP_UNKNOWN              = 0,
    *   OKP_USER_DEFINED         = 1,
    *   OKP_NUMERICALLY_LOSSLESS = 2,
    *   OKP_VISUALLY_LOSSLESS    = 3,
    *   OKP_EPJE                 = 4
    *
    * @param type See enumeration for types.
    */
   virtual void setQualityType(ossimKakaduCompressionQuality type) = 0;

   
   
};

#endif /* End of "#ifndef ossimKakaduCompressorInterface_HEADER" */

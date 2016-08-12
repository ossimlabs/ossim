//----------------------------------------------------------------------------
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Interface for GeoPackage(gpkg) writers.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimGpkgWriterInterface_HEADER
#define ossimGpkgWriterInterface_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimRefPtr.h>

class ossimImageData;
class ossimKeywordlist;

/**
 * @class GeoPackage writer interface.
 *
 * This interface is for using the ossimGeopackageWriter in a connectionless
 * manner.  See ossim-gpkg-writer-test.cpp for example usage.
 */
class OSSIM_DLL ossimGpkgWriterInterface
{
public:

   /**
    * GP:  I had to add this or windows would not link with the latest
    * compiler. Also had to put in dot.cpp for debug mode(again windows).
    * (drb)
    */
   ossimGpkgWriterInterface();

   /**
    * @brief Opens file for writing, appending, merging without an input
    * connection. I.e. opening, then calling writeTile directly.
    * 
    * @param options.  Keyword list containing all options.
    */
   virtual bool openFile( const ossimKeywordlist& options ) = 0;

   /**
    * @brief Calls initial sqlite3_prepare_v2 statement.  Must be called
    * prior to calling writeTile.
    * @return SQLITE_OK(0) on success, something other(non-zero) on failure.
    */
   virtual ossim_int32 beginTileProcessing() = 0;
   
   /**
    * @brief Direct interface to writing a tile to database.
    * @param tile to write.
    * @param zoolLevel
    * @param row
    * @param col
    * @return true on success, false on error.
    */   
   virtual bool writeTile( ossimRefPtr<ossimImageData>& tile,
                           ossim_int32 zoomLevel,
                           ossim_int64 row,
                           ossim_int64 col ) = 0;
   virtual bool writeCodecTile( ossim_uint8* codecTile,
                                ossim_int32 codecTileSize,
                                ossim_int32 zoomLevel,
                                ossim_int64 row,
                                ossim_int64 col ) = 0;

   /**
    * @brief Calls sqlite3_finalize(pStmt) terminating tile processing.
    */
   virtual void finalizeTileProcessing() = 0;
   
};

#endif /* End of "#ifndef ossimGpkgWriterInterface_HEADER" */

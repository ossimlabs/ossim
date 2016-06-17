//*******************************************************************
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Mingjie Su
//
// Description:
//
// Contains class declaration for NitfTileSource_12.
//
//*******************************************************************
//  $Id$
#ifndef ossimNitfTileSource_12_HEADER
#define ossimNitfTileSource_12_HEADER 1

#if defined(OSSIM_WITH_12BIT_JPEG)

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimRefPtr.h>
#include <vector>

class ossimImageData;
class ossimNitfImageHeader;
struct jpeg12_decompress_struct;

class OSSIM_DLL ossimNitfTileSource_12 
{
public:
   static bool uncompressJpeg12Block( /* ossim_uint32 x, ossim_uint32 y, */
                                     ossimRefPtr<ossimImageData> cacheTile,
                                     ossimNitfImageHeader* hdr,
                                     const ossimIpt& cacheSize,
                                     std::vector<ossim_uint8> compressedBuf,
                                     ossim_uint32 readBlockSizeInBytes,
                                     ossim_uint32 bands);
   
   
   static bool loadJpegQuantizationTables(ossimNitfImageHeader* hdr,
                                          jpeg12_decompress_struct& cinfo);
   
   static bool loadJpegHuffmanTables(jpeg12_decompress_struct& cinfo);
};

#endif /* #if defined(OSSIM_WITH_12BIT_JPEG) */

#endif /* #ifndef ossimNitfTileSource_12_HEADER */

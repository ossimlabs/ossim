//----------------------------------------------------------------------------
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken, original code from Thomas G. Lane
//
// Description:
// Code interfaces to use with jpeg-6b library to read a jpeg image from
// memory.
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimJpegMemSrc_HEADER
#define ossimJpegMemSrc_HEADER 1

#include <ossim/base/ossimConstants.h> /** for OSSIM_DLL export macro */
#include <cstdlib> /* size_t */

// Forward declaration:
struct jpeg_common_struct;
struct jpeg_decompress_struct;

extern "C"
{

/**
 * @brief Error routine that will replace jpeg's standard error_exit method.
 */
OSSIM_DLL void ossimJpegErrorExit (jpeg_common_struct* cinfo);

/**
 * @brief Method which uses memory instead of a FILE* to read from.
 * @note Used in place of "jpeg_stdio_src(&cinfo, infile)".
 * @note "unsigned char = JOCTET
 */
OSSIM_DLL void ossimJpegMemorySrc (jpeg_decompress_struct* cinfo,
                                   const ossim_uint8* buffer,
                                   std::size_t bufsize);
}
#endif /* #ifndef ossimJpegMemSrc_HEADER */

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
#ifndef ossimJpegMemSrc12_HEADER
#define ossimJpegMemSrc12_HEADER 1

#if defined(OSSIM_WITH_12BIT_JPEG)

#include <ossim/base/ossimConstants.h> /** for OSSIM_DLL export macro */
extern "C"
{
#include <cstdio>                      /** for size_t  */
#include <csetjmp>                     /** for jmp_buf */
#include <jpeg12/jpeglib.h>            /** for jpeg stuff */


/** @brief Extended error handler struct. */
struct OSSIM_DLL ossimJpegErrorMgr12
{
  struct jpeg12_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct ossimJpegErrorMgr12* ossimJpegErrorPtr12;

/**
 * @brief Error routine that will replace jpeg's standard error_exit method.
 */
OSSIM_DLL void ossimJpegErrorExit12 (j_common_ptr cinfo);

/**
 * @brief Method which uses memory instead of a FILE* to read from.
 * @note Used in place of "jpeg_stdio_src(&cinfo, infile)".
 */
OSSIM_DLL void ossimJpegMemorySrc12 ( j_decompress_ptr cinfo,
                                      const JOCTET * buffer,
                                      std::size_t bufsize );
}

#endif /* #if defined(OSSIM_WITH_12BIT_JPEG) */

#endif /* #ifndef ossimJpegMemSrc12_HEADER */

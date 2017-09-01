//----------------------------------------------------------------------------
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer, original code from Thomas G. Lane
//
// Most of code and comments below are from jpeg-6b "example.c" file. See 
// http://www4.cs.fau.de/Services/Doc/graphics/doc/jpeg/libjpeg.html
//
// Description:
// Code interfaces to use with jpeg-6b library to write a jpeg image from memory.
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimJpegMemDest_HEADER
#define ossimJpegMemDest_HEADER 1

#include <ossim/base/ossimConstants.h> /** for OSSIM_DLL export macro */
#include <iosfwd> /* ostream& */

// Forward declaration:
struct jpeg_compress_struct;

#if 0 /* Avoiding jpeg library includes in header files. */
#include <ostream>                     /** for std::ostream */
#include <cstdio>                      /** for size_t  */
#include <csetjmp>                     /** for jmp_buf */
extern "C"
{
#if defined(_MSC_VER) || defined(__MINGW32__)
#  ifndef XMD_H
#    define XMD_H
#  endif
#endif
// #include <jpeglib.h> /** for jpeg stuff */
}
#endif /* End: #if 0 */

/**
 * @brief Method which uses memory instead of a FILE* to write to.
 * @note Used in place of "jpeg_stdio_dest(&cinfo, outfile)".
 */
// OSSIM_DLL void jpeg_cpp_stream_dest (j_compress_ptr cinfo, std::ostream& stream);
OSSIM_DLL void jpeg_cpp_stream_dest (jpeg_compress_struct* cinfo, std::ostream& stream);

#endif /* #ifndef ossimJpegMemDest_HEADER */

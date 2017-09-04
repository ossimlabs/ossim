//*******************************************************************
//
// License:  LGPL
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
//  $Id: ossimNitfTileSource_12.cpp 958 2010-06-03 23:00:32Z ming.su $

//#if defined(JPEG_DUAL_MODE_8_12)
#include <fstream>

#if defined(JPEG_DUAL_MODE_8_12)
#include <jpeg12/jpeglib.h>

//ossim includes
#include <ossim/imaging/ossimJpegMemSrc.h>
#include <ossim/imaging/ossimNitfTileSource_12.h>
#include <ossim/imaging/ossimJpegDefaultTable.h>


#endif /* defined(JPEG_DUAL_MODE_8_12) */

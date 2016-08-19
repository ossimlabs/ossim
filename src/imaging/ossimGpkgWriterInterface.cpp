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

#include <ossim/imaging/ossimGpkgWriterInterface.h>

//---
// Explicit constructor.  Fix for undefine in sqlite plugin, windows vs14 debug
// configuration.
//---
ossimGpkgWriterInterface::ossimGpkgWriterInterface()
{
}

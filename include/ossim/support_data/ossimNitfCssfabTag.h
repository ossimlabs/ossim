//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSSFAB tag class declaration.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.6 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfGenericTag.h>

#ifndef ossimNitfCssfabTag_HEADER
#define ossimNitfCssfabTag_HEADER 1

class OSSIM_DLL ossimNitfCssfabTag : public ossimNitfGenericTag
{
public:
   ossimNitfCssfabTag(ossim_uint32 tagLength=0);
};
#endif

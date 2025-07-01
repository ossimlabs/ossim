//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSEXRB tag class declaration.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.1 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfGenericTag.h>

#ifndef ossimNitfCsexrbTag_HEADER
#define ossimNitfCsexrbTag_HEADER 1

class OSSIM_DLL ossimNitfCsexrbTag : public ossimNitfGenericTag
{
   public:
      ossimNitfCsexrbTag(ossim_uint32 tagLength=0);
};
#endif

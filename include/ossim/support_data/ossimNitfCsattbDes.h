//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSATTB des class declaration.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.6 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfGenericDes.h>

#ifndef ossimNitfCsattbDes_HEADER
#define ossimNitfCsattbDes_HEADER 1

class OSSIM_DLL ossimNitfCsattbDes : public ossimNitfGenericDes
{
public:
   ossimNitfCsattbDes(ossim_uint32 desLength=0);
};
#endif

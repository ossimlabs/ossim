//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: CSEPHB des class declaration.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.6 for more info.
//
//----------------------------------------------------------------------------

#include <ossim/support_data/ossimNitfGenericDes.h>

#ifndef ossimNitfCsephbDes_HEADER
#define ossimNitfCsephbDes_HEADER 1

class OSSIM_DLL ossimNitfCsephbDes : public ossimNitfGenericDes
{
public:
   
   ossimNitfCsephbDes();
   
   ossimNitfCsephbDes(ossim_uint32 tagLength);
   
   virtual ossimString getClassName() const;

   static const std::string CETAG_KW;

private:

   void initializeFieldDefinitions();

};
#endif

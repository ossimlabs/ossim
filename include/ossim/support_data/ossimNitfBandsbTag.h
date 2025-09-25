//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: BANDSB tag class declaration.
//
// Exploitation Reference Data TRE.
//
// See document App AH - GLAS-GFM table AH.6.1 for more info.
//
//----------------------------------------------------------------------------

#ifndef ossimNitfBandsbTag_HEADER
#define ossimNitfBandsbTag_HEADER 1

#include <ossim/support_data/ossimNitfGenericTag.h>

class OSSIM_DLL ossimNitfBandsbTag : public ossimNitfGenericTag
{
public:

   ossimNitfBandsbTag();
   
   ossimNitfBandsbTag(ossim_uint32 tagLength);
   
   virtual ossimString getClassName() const;

   static const std::string CETAG_KW;
   
private:
   
   /**
    * @brief Initializes ossimNitfGenericTag FIELD_DEFINITIONS.
    */
   void initializeFieldDefinitions();

};

#endif /* #ifndef ossimNitfBandsbTag_HEADER */

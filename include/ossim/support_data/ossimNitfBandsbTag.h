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

   /*!
    * @brief Method to the load (recreate) the state of the object from a
    * keyword list.
    * @param kwl
    * @param prefix Typically, somthing like: "image0.BANDSB."
    * @return true on success, false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

   static const std::string CETAG_KW;
   
private:
   
   /**
    * @brief Initializes ossimNitfGenericTag FIELD_DEFINITIONS.
    */
   void initializeFieldDefinitions();
   
   /**
    * @brief Initializes ossimNitfGenericTag m_fields_map required tags.
    */
   void initializeDefaults();

};

#endif /* #ifndef ossimNitfBandsbTag_HEADER */

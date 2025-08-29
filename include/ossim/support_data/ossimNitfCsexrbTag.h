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

#ifndef ossimNitfCsexrbTag_HEADER
#define ossimNitfCsexrbTag_HEADER 1

#include <ossim/support_data/ossimNitfGenericTag.h>

class OSSIM_DLL ossimNitfCsexrbTag : public ossimNitfGenericTag
{
public:

   ossimNitfCsexrbTag();
   
   ossimNitfCsexrbTag(ossim_uint32 tagLength);
   
   virtual ossimString getClassName() const;

   /*!
    * @brief Method to the load (recreate) the state of the object from a
    * keyword list.
    * @param kwl
    * @param prefix Typically, somthing like: "image0.CSEXRB."
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

#endif /* #ifndef ossimNitfCsexrbTag_HEADER */

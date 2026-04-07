//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: PIXQLA tag class declaration.
//
// Exploitation Reference Data TRE.
//
// See document Vol1-AppAK-MATESA table 6.5 for more info.
//
//----------------------------------------------------------------------------

#ifndef ossimNitfPixqlaTag_HEADER
#define ossimNitfPixqlaTag_HEADER 1

#include <ossim/support_data/ossimNitfGenericTag.h>

class OSSIM_DLL ossimNitfPixqlaTag : public ossimNitfGenericTag
{
public:

   ossimNitfPixqlaTag();
   
   ossimNitfPixqlaTag(ossim_uint32 tagLength);
   
   virtual ossimString getClassName() const;

   /*!
    * @brief Method to the load (recreate) the state of the object from a
    * keyword list.
    * @param kwl
    * @param prefix Typically, somthing like: "image0.PIXQLA."
    * @return true on success, false on error.
    */


   static const std::string CETAG_KW;

private:

   /**
    * @brief Initializes ossimNitfGenericTag FIELD_DEFINITIONS.
    */
   void initializeFieldDefinitions();

};

#endif /* #ifndef ossimNitfPixqlaTag_HEADER */

//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description: HISTOA tag class declaration.
// 
//----------------------------------------------------------------------------
// $Id: ossimNitfHistoaTag.h 22013 2012-12-19 17:37:20Z dburken $

#ifndef ossimNitfHistoaTag_HEADER
#define ossimNitfHistoaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <vector>

/**
 * All comments in this document were taken from the online resource found at:
 *
 * http://www.gwg.nga.mil/ntb/baseline/docs/stdi0002/
 *
 * Reference: HISTOA section (currently in Appendix L)
 * 
 * The purpose of the Softcopy History Tagged Record Extension, HISTOA, is to
 * provide a history of the softcopy processing functions that have been applied to NSIF
 * imagery. It is meant to describe previous processing actions and the current state of the
 * imagery that was distributed within the intelligence and imagery user community. To be
 * effective, HISTOA needs to be applied to the NSIF product as early as practical and must
 * be updated each time the image is processed and saved by a softcopy processing system.
 * This will allow the user to know with confidence the complete history of the imagery.
 * HISTOA may be created as the NSIF image is created, or when the imagery is first
 * modified.
 */
#include <ossim/support_data/ossimNitfGenericTag.h>

class OSSIM_DLL ossimNitfHistoaTag : public ossimNitfGenericTag
{
public:

   ossimNitfHistoaTag();

   ossimNitfHistoaTag(ossim_uint32 tagLength);

   virtual ossimString getClassName() const;

   /*!
    * @brief Method to the load (recreate) the state of the object from a
    * keyword list.
    * @param kwl
    * @param prefix Typically, somthing like: "image0.HISTOA."
    * @return true on success, false on error.
    */


   static const std::string CETAG_KW;
   
private:

   /**
   * @brief Initializes ossimNitfGenericTag FIELD_DEFINITIONS.
   */
   void initializeFieldDefinitions();
};

#endif /* #ifndef ossimNitfHistoaTag_HEADER */

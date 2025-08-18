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
   static const std::string IMAGE_UUID_KW;
   static const std::string NUM_ASSOC_DES_KW;
   static const std::string ASSOC_DES_UUID_KW;
   static const std::string PLATFORM_ID_KW;
   static const std::string PAYLOAD_ID_KW;
   static const std::string SENSOR_ID_KW;
   static const std::string SENSOR_TYPE_KW;
   static const std::string DAY_FIRST_LINE_IMAGE_KW;
   static const std::string TIME_FIRST_LINE_IMAGE_KW;
   static const std::string TIME_IMAGE_DURATION_KW;   
   static const std::string GROUND_REF_POINT_X_KW;
   static const std::string GROUND_REF_POINT_Y_KW;
   static const std::string GROUND_REF_POINT_Z_KW;
   static const std::string MAX_GSD_KW;
   static const std::string ALONG_SCAN_GSD_KW;
   static const std::string CROSS_SCAN_GSD_KW;
   static const std::string GEO_MEAN_GSD_KW;
   static const std::string A_S_VERT_GSD_KW;
   static const std::string C_S_VERT_GSD_KW;
   static const std::string GEO_MEAN_VERT_GSD_KW;
   static const std::string GSD_BETA_ANGLE_KW;
   static const std::string DYNAMIC_RANGE_KW;
   static const std::string NUM_LINES_KW;
   static const std::string NUM_SAMPLES_KW;
   static const std::string ANGLE_TO_NORTH_KW;
   static const std::string OBLIQUITY_ANGLE_KW;
   static const std::string AZ_OF_OBLIQUITY_KW;
   static const std::string ATM_REFR_FLAG_KW;
   static const std::string VEL_ABER_FLAG_KW;
   static const std::string GRD_COVER_KW;
   static const std::string SNOW_DEPTH_CATEGORY_KW;
   static const std::string SUN_AZIMUTH_KW;
   static const std::string SUN_ELEVATION_KW;
   static const std::string PREDICTED_NIIRS_KW;
   static const std::string CIRCL_ERR_KW;
   static const std::string LINEAR_ERR_KW;
   static const std::string CLOUD_COVER_KW;
   
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

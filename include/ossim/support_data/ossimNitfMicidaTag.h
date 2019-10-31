//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//
// Motion Imagery Core Identification TRE(MICIDA) class declaration.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 13 for more info.
//
//---
// $Id

#ifndef ossimNitfMicidaTag_HEADER
#define ossimNitfMicidaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <string>
#include <vector>

/**
 * @class ossimNitfMicidaCamera
 *
 * Camera ID only portion of MICIDA tag.
 */
class ossimNitfMicidaCamera
{
public:
   enum
   {
      CAMERAS_ID_SIZE     = 36,
      CORE_ID_LENGTH_SIZE = 3
   };

   void parseStream(std::istream& in);
   
   void writeStream(std::ostream& out) const;
   
   std::ostream& print( std::ostream& out,
                        const std::string& prefix,
                        ossim_uint32 index ) const;

   /**
    * @brief Returns the length in bytes of the camera record.
    *
    * CAMERAS_ID_SIZE + CORE_ID_LENGTH_SIZE + m_cameraCoreId.size()
    * 
    * @return Size in bytes.
    */
   ossim_uint32 getSizeInBytes() const;
   
protected:

   void clearFields();

   /**
    * FIELD: CAMERAS_ID
    *
    * The UUID of the m'th camera in this TRE.
    * 
    * 36 bytes
    *
    * BCS-A, A valid UUID in the “8-4-4-4-12” hexadecimal format
    * (see ITU-T X.667)
    */
   char m_cameraId[CAMERAS_ID_SIZE+1];

   /**
    * FIELD: CORE_ID_LENGTH
    *
    * Length of the MIIS core identifier for this camera.
    * 
    * 3 bytes
    * 
    * BCS-N, 000 – 999
    */
   char m_coreIdLength[CORE_ID_LENGTH_SIZE+1];
   
   /**
    * FIELD: CAMERA_CORE_ID
    *
    * MIIS core identifier for the m'th camera. The text format of the core
    * identifier is used. See MISB ST 1204 for details.
    * 
    * variable length
    * 
    * BCS-A
    */
   std::string m_cameraCoreId;
};

class OSSIM_DLL ossimNitfMicidaTag : public ossimNitfRegisteredTag
{
public:

   enum
   {
      NUM_CAMERA_IDS_IN_TRE_SIZE = 3,
      MIIS_CORE_ID_VERSION_SIZE = 2
   };

   /** @brief default constructor */
   ossimNitfMicidaTag();
   
   /** @brief Method to parse data from stream. */
   virtual void parseStream(std::istream& in);
   
   /** @brief Method to write data to stream. */
   virtual void writeStream(std::ostream& out);

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const;
   
protected:

   /** @brief Method to clear all fields including null terminating. */
   virtual void clearFields();

   /**
    * FIELD: MIIS_CORE_ID_VERSION
    *
    * Version number of MISB ST 1204 used.
    * 
    * 2 bytes
    *
    * BCS-N, 01
    */
   char m_miisCoreIdVersion[MIIS_CORE_ID_VERSION_SIZE+1];
   
   /**
    * FIELD: NUM_CAMERA_IDS_IN_TRE
    *
    * The number of MIIS core identifiers, M, in this TRE.
    * 
    * 3 bytes
    *
    * BCS-N, 001 – 999
    */
   char m_numCameraIdsInTre[NUM_CAMERA_IDS_IN_TRE_SIZE+1];
   
   /**
    * Holds an array of camera sets
    */
   std::vector<ossimNitfMicidaCamera> m_camera;
   
TYPE_DATA   
};

#endif /* matches #ifndef ossimNitfMicidaTag_HEADER */

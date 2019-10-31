//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//
// Camera Set Definition TRE(CAMSDA) class declaration.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 12 for more info.
//
//---
// $Id

#ifndef ossimNitfCamsdaTag_HEADER
#define ossimNitfCamsdaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <vector>

/**
 * @class ossimNitfCamsdaCamera
 *
 * Camera only portion of CAMSDA tag.
 */
class ossimNitfCamsdaCamera
{
public:
   enum
   {
      CAMERA_ID_SIZE   = 36,
      CAMERA_DESC_SIZE = 80,
      LAYER_ID_SIZE    = 36,
      IDLVL_SIZE       = 3,
      IALVL_SIZE       = 3,      
      ILOC_SIZE        = 10,
      NROWS_SIZE       = 8,
      NCOLS_SIZE       = 8,
      //               -----
      RECORD_SIZE     = 184 // bytes
   };

   void parseStream(std::istream& in);
   
   void writeStream(std::ostream& out) const;
   
   std::ostream& print( std::ostream& out,
                        const std::string& prefix,
                        ossim_uint32 index) const;
   
private:

   void clearFields();

   /**
    * FIELD: CAMERA_ID
    *
    * The UUID of the m'th camera in the n'th camera set. See NGA.RP.0001_1.0.0
    * for UUID details.
    * 
    * 36 bytes
    *
    * BCS-A, A valid UUID in the “8-4-4-4-12” hexadecimal format
    * (see ITU-T X.667)
    */
   char m_cameraId[CAMERA_ID_SIZE+1];

   /**
    * FIELD: CAMERA_DESC
    *
    * A human readable description of the camera.
    * 
    * 80 byte field BCS-A
    */
   char m_cameraDesc[CAMERA_DESC_SIZE+1];

   /**
    * FIELD: LAYER_ID
    *
    * A 36 character string identifying the phenomenological layer for the m'th
    * camera in the n'th camera set. Field is large enough to hold a UUID.
    * 
    * 36 byte field BCS-A
    */   
   char m_layerId[LAYER_ID_SIZE+1];

   /**
    * FIELD: IDLVL
    *
    * The image display level value for the m'th camera in the n'th camera set.
    * 
    * 3 bytes
    *
    * BCS-N positive integer 001 to 999
    */   
   char m_idLvl[IDLVL_SIZE+1];

   /**
    * FIELD: IALVL
    *
    * The image attachment level value for the m'th camera in the n'th camera
    * set.
    * 
    * 3 bytes
    *
    * BCS-N positive integer 000 to 998
    */
   char m_iaLvl[IALVL_SIZE+1];

   /**
    * FIELD: ILOC
    *
    * The image location of the first sample of the first line of the imagery
    * for the m'th camera in the n'th camera set at the original spatial
    * resolution of the data (Rset = 0, i.e. IMAG = 1.0).*
    * 
    * 10 bytes
    *
    * BCS-N positive integer RRRRRCCCCC RRRRR and CCCCC are in the range -9999
    * to 99999
    */  
   char m_iloc[ILOC_SIZE+1];

   /**
    * FIELD: NROWS
    *
    * The number of significant rows of pixels for the m'th camera in the n'th
    * camera set at the original spatial resolution of the data (Rset = 0, i.e.
    * IMAG = 1.0).*
    * 
    * 10 bytes
    *
    * BCS-N positive integer 00000001 to 99999999
    */  
   char m_nRows[NROWS_SIZE+1];

   /**
    * FIELD: NCOLS
    *
    * The number of significant columns of pixels for the m'th camera in the
    * n'th camera set at the original spatial resolution of the data (Rset = 0,
    * i.e. IMAG = 1.0).*
    * 
    * 10 bytes
    *
    * BCS-N positive integer 00000001 to 99999999
    */  
   char m_nCols[NCOLS_SIZE+1];
};


/**
 * @class ossimNitfCamsdaCameraSet
 *
 * Camera "Set" portion of CAMSDA tag. This holds an array of cameras.
 */
class ossimNitfCamsdaCameraSet
{
public:
   enum
   {
      NUM_CAMERAS_IN_SET_SIZE = 3
   };

   /** @brief default constructor */
   ossimNitfCamsdaCameraSet();
   
   void parseStream(std::istream& in);
   
   void writeStream(std::ostream& out) const;
   
   std::ostream& print( std::ostream& out,
                        const std::string& prefix,
                        ossim_uint32 index) const;

   /**
    * @brief Returns the length in bytes of the cameraset record.
    *
    * size_of_NUM_CAMERAS_IN_SET_field + (m_numCamerasInSet*camera_record_size)
    * @return Size in bytes.
    */
   ossim_uint32 getSizeInBytes()const;

private:

   void clearFields();

   /**
    * FIELD: NUM_CAMERAS_IN_SET
    *
    * The number of cameras in the n'th camera set, M.
    * 
    * 3 bytes
    *
    * BCS-N, 001 – 543
    */

   char m_numCamerasInSet[NUM_CAMERAS_IN_SET_SIZE+1];

   /**
    * Holds an array of cameras.
    */
   std::vector<ossimNitfCamsdaCamera> m_camera;
};

class OSSIM_DLL ossimNitfCamsdaTag : public ossimNitfRegisteredTag
{
public:

   enum
   {
      NUM_CAMERA_SETS_SIZE = 3,
      NUM_CAMERA_SETS_IN_TRE_SIZE = 3,
      FIRST_CAMERA_SETS_IN_TRE_SIZE = 3,
      NUM_CAMERAS_IN_SET_SIZE = 3
   };

   /** @brief default constructor */
   ossimNitfCamsdaTag();
   
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
   
   /**
    * @brief Get the NUM_CAMERA_SETS field.
    * @return The NUM_CAMERA_SETS field as a string.
    */
   ossimString getNumberOfCameraSets() const;

   /**
    * @brief Get the NUM_CAMERA_SETS_IN_TRE field.
    * @return The NUM_CAMERA_SETS_IN_TRE field as a string.
    */
   ossimString getNumberOfCameraSetsInTre() const;

   /**
    * @brief Get the FIRST_CAMERA_SETS_IN_TRE field.
    * @return The FIRST_CAMERA_SETS_IN_TRE field as a string.
    */
   ossimString getFirstCameraSetInTre() const;
   
protected:

   /** @brief Method to clear all fields including null terminating. */
   virtual void clearFields();

   /**
    * FIELD: NUM_CAMERA_SETS
    *
    * The number of camera sets, N, in the collection.
    * 
    * 3 bytes
    *
    * BCS-N, 001 – 999
    */
   char m_numCameraSets[NUM_CAMERA_SETS_SIZE+1];

   /**
    * FIELD: NUM_CAMERA_SETS_IN_TRE
    *
    * The number of camera sets defined in this instance of the CAMSDA TRE.
    * 
    * 3 bytes
    *
    * BCS-N, 001 – 534
    */
   char m_numCameraSetsInTre[NUM_CAMERA_SETS_IN_TRE_SIZE+1];

   /**
    * FIELD: FIRST_CAMERA_SETS_IN_TRE
    *
    * The index of the first camera set defined in this instance of the CAMSDA
    * TRE.
    * 
    * 3 bytes
    *
    * BCS-N, 001 – 999
    */
   char m_firstCameraSetInTre[FIRST_CAMERA_SETS_IN_TRE_SIZE+1];
   
   /**
    * Holds an array of camera sets
    */
   std::vector<ossimNitfCamsdaCameraSet> m_cameraSet;
   
TYPE_DATA   
};

#endif /* matches #ifndef ossimNitfCamsdaTag_HEADER */

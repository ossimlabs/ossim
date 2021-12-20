//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Scott Bortman
//
// Description: CSSFAA tag class declaration.
//
// Exploitation Reference Data TRE.
//
// See document STDI-0006-NCDRD Table 3.7-1 for more info.
// 
//----------------------------------------------------------------------------
// $Id

#ifndef ossimNitfCssfaaTag_HEADER
#define ossimNitfCssfaaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>

class OSSIM_DLL ossimNitfCssfaaTag : public ossimNitfRegisteredTag
{
public:
   /** @brief default constructor */
   ossimNitfCssfaaTag();

   /** @brief destructor */
   virtual ~ossimNitfCssfaaTag();

   /** @brief Method to parse data from stream. */
   virtual void parseStream(std::istream& in);

   /** @brief Method to write data to stream. */
   virtual void writeStream(std::ostream& out);

   /** @brief Method to clear all fields including null terminating. */
   virtual void clearFields();

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const;
   
   /**
    * @brief Get the NUM_BANDS field.
    * @return The NUM_BANDS field as a string.
    */
   ossimString getNumBands() const;

   /**
    * @brief Get the BAND_TYPE field.
    * @return The BAND_TYPE field as a vector of strings.
    */
   std::vector<ossimString> getBandType() const;
   
   /**
    * @brief Get the BAND_ID field.
    * @return The BAND_ID field as a vector of strings.
    */
   std::vector<ossimString> getBandId() const;

   /**
    * @brief Get the FOC_LENGTH field.
    * @return The FOC_LENGTH field as a vector of strings.
    */
   std::vector<ossimString> getFocLength() const;

   /**
    * @brief Get the NUM_DAP field.
    * @return The NUM_DAP field as a vector of strings.
    */
   std::vector<ossimString> getNumDap() const;

   /**
    * @brief Get the NUM_FIR field.
    * @return The NUM_FIR field as a vector of strings.
    */
   std::vector<ossimString> getNumFir() const;

   /**
    * @brief Get the DELTA field.
    * @return The DELTA field as a vector of strings.
    */
   std::vector<ossimString> getDelta() const;

   /**
    * @brief Get the OPPOFF_X field.
    * @return The OPPOFF_X field as a vector of strings.
    */
   std::vector<ossimString> getOppOffX() const;

   /**
    * @brief Get the OPPOFF_Y field.
    * @return The OPPOFF_Y field as a vector of strings.
    */
   std::vector<ossimString> getOppOffY() const;

   /**
    * @brief Get the OPPOFF_Z field.
    * @return The OPPOFF_Z field as a vector of strings.
    */
   std::vector<ossimString> getOppOffZ() const;

   /**
    * @brief Get the START_X field.
    * @return The START_X field as a vector of strings.
    */
   std::vector<ossimString> getStartX() const;

   /**
    * @brief Get the START_Y field.
    * @return The START_Y field as a vector of strings.
    */
   std::vector<ossimString> getStartY() const;

   /**
    * @brief Get the FINISH_X field.
    * @return The FINISH_X field as a vector of strings.
    */
   std::vector<ossimString> getFinishX() const;

   /**
    * @brief Get the FINISH_Y field.
    * @return The FINISH_Y field as a vector of strings.
    */
   std::vector<ossimString> getFinishY() const;

protected:

   /**
    * FIELD: NUM_BANDS
    * 
    * 1 byte field BCS-A
    *
    */
   char theNumBands[2];

   /**
    * FIELD: BAND_TYPE
    *
    * 1 byte field BCS-A
    *
    */
   std::vector<ossimString> theBandType;

   /**
    * FIELD: BAND_ID
    *
    * 6 byte field BCS-A
    *
    */
   std::vector<ossimString> theBandId;

   /**
    * FIELD: FOC_LENGTH
    *
    * 11 byte field BCS-N (positive float)
    */
   std::vector<ossimString> theFocLength;

   /**
    * FIELD: NUM_DAP
    *
    * 8 byte field BCS-N (positive integer)
    *
    */
   std::vector<ossimString> theNumDap;

   /**
    * FIELD: NUM_FIR
    *
    * 8 byte field BCS-N (positive integer)
    *
    */
   std::vector<ossimString> theNumFir;

   /**
    * FIELD: DELTA
    *
    * 7 byte field BCS-N (positive integer)
    *
    */
   std::vector<ossimString> theDelta;

   /**
    * FIELD: OPPOFF_X
    *
    * 7 byte field BCS-N (signed float)
    *
    */
   std::vector<ossimString> theOppOffX;

   /**
    * FIELD: OPPOFF_Y
    *
    * 7 byte field BCS-N (signed float)
    *
    */
   std::vector<ossimString> theOppOffY;

   /**
    * FIELD: OPPOFF_Z
    *
    * 7 byte field BCS-N (signed float)
    *
    */
   std::vector<ossimString> theOppOffZ;


   /**
    * FIELD: START_X
    *
    * 11 byte field BCS-N (signed float)
    *
    */
   std::vector<ossimString> theStartX;

   /**
    * FIELD: START_Y
    *
    * 11 byte field BCS-N (signed float)
    *
    */
   std::vector<ossimString> theStartY;

   /**
    * FIELD: FINISH_X
    *
    * 11 byte field BCS-N (signed float)
    *
    */
   std::vector<ossimString> theFinishX;

   /**
    * FIELD: FINISH_Y
    *
    * 11 byte field BCS-N (signed float)
    *
    */
   std::vector<ossimString> theFinishY;


   ossimString vec2str(std::vector<ossimString>) const;

TYPE_DATA   
};

#endif /* matches #ifndef ossimNitfCssfaaTag_HEADER */

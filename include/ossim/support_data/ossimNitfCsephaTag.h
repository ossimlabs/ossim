//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Scott Bortman
//
// Description: CSEPHA tag class declaration.
//
// Exploitation Reference Data TRE.
//
// See document STDI-0006-NCDRD Table 3.5-16 for more info.
// 
//----------------------------------------------------------------------------
// $Id

#ifndef ossimNitfCsephaTag_HEADER
#define ossimNitfCsephaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>

class OSSIM_DLL ossimNitfCsephaTag : public ossimNitfRegisteredTag
{
public:
   /** @brief default constructor */
   ossimNitfCsephaTag();

   /** @brief destructor */
   virtual ~ossimNitfCsephaTag();

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
    * @brief Get the EPHEM_FLAG field.
    * @return The EPHEM_FLAG field as a string.
    */
   ossimString getEphemFlag() const;

   /**
    * @brief Get the DT_EPHEM field.
    * @return The DT_EPHEM field as a string.
    */
   ossimString getDtEphem() const;
   
   /**
    * @brief Get the DATE_EPHEM field.
    * @return The DATE_EPHEM field as a string.
    */
   ossimString getDateEphem() const;

   /**
    * @brief Get the T0_EPHEM field.
    * @return The T0_EPHEM field as a string.
    */
   ossimString getT0Ephem() const;

   /**
    * @brief Get the NUM_EPHEM field.
    * @return The NUM_EPHEM field as a string.
    */
   ossimString getNumEphem() const;

   /**
    * @brief Get the EPHEM_X field.
    * @return The EPHEM_X field as a vector of strings.
    */
   std::vector<ossimString> getEphemX() const;

   /**
    * @brief Get the EPHEM_Y field.
    * @return The EPHEM_Y field as a vector of strings.
    */
   std::vector<ossimString> getEphemY() const;

   /**
    * @brief Get the EPHEM_Z field.
    * @return The EPHEM_Z field as a vector of strings.
    */
   std::vector<ossimString> getEphemZ() const;

protected:

   /**
    * FIELD: EPHEM_FLAG
    * 
    * 12 byte field BCS-A
    *
    * PAN MS
    */
   char theEphemFlag[13];

   /**
    * FIELD: DT_EPHEM
    *
    * 5 byte field BCS-N Seconds (UTC)
    *
    */
   char theDtEphem[6];

   /**
    * FIELD: DATE_EPHEM
    *
    * 8 byte field BCS-N (UTC)
    *
    */
   char theDateEphem[9];

   /**
    * FIELD: MAX_GSD
    *
    * 13 byte field BCS-N (UTC)
    */
   char theT0Ephem[14];

   /**
    * FIELD: NUM_EPHEM
    *
    * 3 byte field BCS-N
    *
    */
   char theNumEphem[4];

   /**
    * FIELD: EPHEM_X
    *
    * 12 byte field BCS-A
    *
    */
   std::vector<ossimString> theEphemX;

   /**
    * FIELD: EPHEM_Y
    *
    * 12 byte field BCS-A
    *
    */
   std::vector<ossimString> theEphemY;

   /**
    * FIELD: EPHEM_Z
    *
    * 12 byte field BCS-A
    *
    */
   std::vector<ossimString> theEphemZ;

   ossimString vec2str(std::vector<ossimString>) const;

TYPE_DATA   
};

#endif /* matches #ifndef ossimNitfCsephaTag_HEADER */

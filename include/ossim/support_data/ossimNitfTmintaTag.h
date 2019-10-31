//---
//
// License: MIT
//
// Author: David Burken
//
// Description:
//
// Time Interval Definition TRE(TMINTA) class declaration.
//
// See document NGA.STND.0044_1.3_MIE4NITF, table 14 for more info.
//
//---
// $Id

#ifndef ossimNitfTmintaTag_HEADER
#define ossimNitfTmintaTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <vector>

/**
 * @class ossimNitfTmintaCamera
 *
 * Time portion of TMINTA tag.
 */
class ossimNitfTmintaTime
{
public:
   enum
   {
      TIME_INTERVAL_INDEX_SIZE = 6,
      START_TIMESTAMP_SIZE     = 24,
      END_TIMESTAMP_SIZE       = 24
   };

   void parseStream(std::istream& in);
   
   void writeStream(std::ostream& out) const;
   
   std::ostream& print( std::ostream& out,
                        const std::string& prefix,
                        ossim_uint32 index) const;

private:

   void clearFields();

   /**
    * FIELD: TIME_INTERVAL_INDEX
    *
    * The index of this time interval.
    * 
    * 6 bytes
    *
    * BCS-N, 000000 – 999999
    */
   char m_timeIntervalIndex[TIME_INTERVAL_INDEX_SIZE+1];

   /**
    * FIELD: START_TIMESTAMP
    *
    * Start time of time interval n.
    * 
    * 24 bytes
    * 
    * BCS-A, Spaces or UTC format
    *
    * Year, month, day, hour, minute, seconds, nanosecs
    */
   char m_startTimestamp[START_TIMESTAMP_SIZE+1];

   /**
    * FIELD: END_TIMESTAMP
    *
    * End time of time interval n.
    * 
    * 24 bytes
    * 
    * BCS-A, Spaces or UTC format
    *
    * Year, month, day, hour, minute, seconds, nanosecs
    */
   char m_endTimestamp[END_TIMESTAMP_SIZE+1];

};

class OSSIM_DLL ossimNitfTmintaTag : public ossimNitfRegisteredTag
{
public:

   enum
   {
      NUM_TIM_INT_SIZE = 4
   };

   /** @brief default constructor */
   ossimNitfTmintaTag();
   
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
    * FIELD: NUM_TIM_INT
    *
    * The number of time intervals specified by this TRE. Note that only 1,851
    * fit within the maximum length of a TRE.
    * 
    * 4 bytes
    *
    * BCS-N, positive integer 0001 – 1851
    */
   char m_numTimInt[NUM_TIM_INT_SIZE+1];
   
   /**
    * Holds an array of camera sets
    */
   std::vector<ossimNitfTmintaTime> m_time;
   
TYPE_DATA   
};

#endif /* matches #ifndef ossimNitfTmintaTag_HEADER */

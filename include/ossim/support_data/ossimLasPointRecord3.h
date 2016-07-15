//----------------------------------------------------------------------------
//
// File: ossimLasPointRecord3.h
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimLasPointRecord3_HEADER
#define ossimLasPointRecord3_HEADER 1

#include <ossim/support_data/ossimLasPointRecordInterface.h>

/**
 * @class ossimLasPointRecord3
 *
 * Container class for LAS point record type 3.
 */
class ossimLasPointRecord3 : public ossimLasPointRecordInterface
{
public:

   /** @brief default constructor */
   ossimLasPointRecord3();

   /* @brief copy constructor */
   ossimLasPointRecord3(const ossimLasPointRecord3& obj);

   /* @brief assignment operator= */
   const ossimLasPointRecord3& operator=(const ossimLasPointRecord3& copy_this);

   /** @brief destructor */
   virtual ~ossimLasPointRecord3();

   /** @brief Initialize record from stream. */
   virtual void readStream(std::istream& in);

   /** @brief Writes record to stream. */
   virtual void writeStream(std::ostream& out);

   /** @return x */
   virtual ossim_int32 getX() const;

   /** @return y */
   virtual ossim_int32 getY() const;
   
   /** @return z */
   virtual ossim_int32 getZ() const;

   /** @return return number */
   virtual ossim_uint8 getReturnNumber() const;

   virtual ossim_uint8 getNumberOfReturns() const;
   
   virtual ossim_uint8 getEdgeFlag() const;
      
   virtual std::ostream& print(std::ostream& out) const;

private:

   /** @brief Performs a swap if system byte order is not little endian. */
   void swap();

   union BitFlags
   {
      BitFlags(const BitFlags& obj):m_bits(obj.m_bits){}
      BitFlags(ossim_uint8 value):m_bits((ossim_uint8)value){}
      const BitFlags& operator =(const BitFlags& obj){m_bits = obj.m_bits;return *this;}
      struct
      {
         ossim_uint8 m_returnNumber     : 3;
         ossim_uint8 m_numberOfReturns  : 3;
         ossim_uint8 m_scanDirection    : 1;
         ossim_uint8 m_edgeOfFlightLine : 1;
      };
      ossim_uint8 m_bits;
   };

   //---
   // Private structure so we can do a single read/write in the readStream
   // and writeStream methods.  This sped the code up 5X when doing an ossim-icp
   // from a LAS file to a tif.
   //---
   struct ossimLasPointRecord3Data
   {
      ossimLasPointRecord3Data();
      ossimLasPointRecord3Data(const ossimLasPointRecord3Data& obj);
      const ossimLasPointRecord3Data& operator=(const ossimLasPointRecord3Data& obj);
      ossim_int32   m_x;
      ossim_int32   m_y;
      ossim_int32   m_z;
      ossim_uint16  m_intensity;
      BitFlags      m_bitFlags;
      ossim_uint8   m_classification;
      ossim_int8    m_scanAngleRank;
      ossim_uint8   m_userData;
      ossim_uint16  m_pointSourceId;
      ossim_float64 m_gpsTime;
      ossim_uint16  m_red;
      ossim_uint16  m_green;
      ossim_uint16  m_blue;
   };
   
   ossimLasPointRecord3Data m_record;
};

#endif /* #ifndef ossimLasPointRecord3_HEADER */


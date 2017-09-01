//----------------------------------------------------------------------------
//
// File: ossimLasPointRecord1.h
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimLasPointRecord1_HEADER
#define ossimLasPointRecord1_HEADER 1

#include <ossim/support_data/ossimLasPointRecordInterface.h>

/**
 * @class ossimLasPointRecord1
 *
 * Container class for LAS point record type 1.
 */
class ossimLasPointRecord1 : public ossimLasPointRecordInterface
{
public:

   /** @brief default constructor */
   ossimLasPointRecord1();

   /* @brief copy constructor */
   ossimLasPointRecord1(const ossimLasPointRecord1& obj);

   /* @brief assignment operator= */
   const ossimLasPointRecord1& operator=(const ossimLasPointRecord1& copy_this);

   /** @brief destructor */
   virtual ~ossimLasPointRecord1();

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

   /** @return red */
   virtual ossim_uint16 getRed() const;

   /** @return green */
   virtual ossim_uint16 getGreen() const;

   /** @return blue */
   virtual ossim_uint16 getBlue() const;

   /** @return intensity */
   virtual ossim_uint16 getIntensity() const;

   /** @return return number */
   virtual ossim_uint8 getReturnNumber() const;

   virtual ossim_uint8 getNumberOfReturns() const;
   
   virtual ossim_uint8 getEdgeFlag() const;
      
   virtual std::ostream& print(std::ostream& out) const;

private:

   /** @brief Performs a swap if system byte order is not little endian. */
   void swap();

   //---
   // Private structure so we can do a single read/write in the readStream
   // and writeStream methods.  This sped the code up 5X when doing an ossim-icp
   // from a LAS file to a tif.
   //---
   struct ossimLasPointRecord1Data
   {
      ossimLasPointRecord1Data();
      ossimLasPointRecord1Data(const ossimLasPointRecord1Data& obj);
      const ossimLasPointRecord1Data& operator=(const ossimLasPointRecord1Data& obj);
      ossim_int32   m_x;
      ossim_int32   m_y;
      ossim_int32   m_z;
      ossim_uint16  m_intensity;
      ossim_uint8   m_returnByte;
      ossim_uint8   m_classification;
      ossim_int8    m_scanAngleRank;
      ossim_uint8   m_userData;
      ossim_uint16  m_pointSourceID;
      ossim_float64 m_gpsTime;
   };
   
   ossimLasPointRecord1Data m_record;
};

#endif /* #ifndef ossimLasPointRecord1_HEADER */


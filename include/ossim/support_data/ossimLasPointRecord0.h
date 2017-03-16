//----------------------------------------------------------------------------
//
// File: ossimLasPointRecord0.h
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimLasPointRecord0_HEADER
#define ossimLasPointRecord0_HEADER 1

#include <ossim/support_data/ossimLasPointRecordInterface.h>

/**
 * @class ossimLasPointRecord0
 *
 * Container class for LAS point record type 1.
 */
class ossimLasPointRecord0 : public ossimLasPointRecordInterface
{
public:

   /** @brief default constructor */
   ossimLasPointRecord0();

   /* @brief copy constructor */
   ossimLasPointRecord0(const ossimLasPointRecord0& obj);

   /* @brief assignment operator= */
   const ossimLasPointRecord0& operator=(const ossimLasPointRecord0& copy_this);

   /** @brief destructor */
   virtual ~ossimLasPointRecord0();

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
   struct ossimLasPointRecord0Data
   {
      ossimLasPointRecord0Data();
      ossimLasPointRecord0Data(const ossimLasPointRecord0Data& obj);
      const ossimLasPointRecord0Data& operator=(const ossimLasPointRecord0Data& obj);
      ossim_int32   m_x;
      ossim_int32   m_y;
      ossim_int32   m_z;
      ossim_uint16  m_intensity;
      ossim_uint8   m_returnByte;
      ossim_uint8   m_classification;
      ossim_int8    m_scanAngleRank;
      ossim_uint8   m_userData;
      ossim_uint16  m_pointSourceID;
   };
   
   ossimLasPointRecord0Data m_record;
};

#endif /* #ifndef ossimLasPointRecord0_HEADER */


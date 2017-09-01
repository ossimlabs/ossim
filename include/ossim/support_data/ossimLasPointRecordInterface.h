//----------------------------------------------------------------------------
//
// File: ossimLasPointRecordInterface.h
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimLasPointRecordInterface_HEADER
#define ossimLasPointRecordInterface_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <iosfwd>

/**
 * @class ossimLasPointRecordInterface
 *
 * OSSIM Interface for LAS point record types.
 */
class ossimLasPointRecordInterface
{
public:

   /** @brief Virtual destructor defined to squash compiler errors. */
   virtual ~ossimLasPointRecordInterface(){}
   
   virtual void readStream(std::istream& in) = 0;
   virtual void writeStream(std::ostream& out) = 0;

   virtual ossim_uint16 getRed() const = 0;
   virtual ossim_uint16 getGreen() const = 0;
   virtual ossim_uint16 getBlue() const = 0;
   virtual ossim_uint16 getIntensity() const = 0;
   virtual ossim_int32  getX() const = 0;
   virtual ossim_int32  getY() const = 0;
   virtual ossim_int32  getZ() const = 0;
   virtual ossim_uint8  getReturnNumber() const = 0;
   virtual ossim_uint8  getNumberOfReturns() const = 0;
   virtual ossim_uint8  getEdgeFlag() const = 0;
   
   virtual std::ostream& print(std::ostream& out) const = 0;
};

#endif /* #ifndef ossimLasPointRecordInterface_HEADER */


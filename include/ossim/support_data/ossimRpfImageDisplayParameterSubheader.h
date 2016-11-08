//*******************************************************************
//
// License: MIT
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id$

#ifndef ossimRpfImageDisplayParameterSubheader_HEADER
#define ossimRpfImageDisplayParameterSubheader_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimErrorContext.h>
#include <ossim/base/ossimIosFwd.h>

class ossimRpfImageDisplayParameterSubheader
{
public:
   friend std::ostream& operator<<(
      std::ostream& out, const ossimRpfImageDisplayParameterSubheader& data);
   ossimRpfImageDisplayParameterSubheader();
   ossimErrorCode parseStream(ossim::istream& in, ossimByteOrder byteOrder);
   
   void print(std::ostream& out)const;

   ossim_uint32 getStartOffset()const
      {
         return theStartOffset;
      }
   ossim_uint32 getEndOffset()const
      {
         return theEndOffset;
      }
   ossim_uint32 getNumberOfImageRows()const
      {
         return theNumberOfImageRows;
      }
   ossim_uint32 getNumberOfImageCodesPerRow()const
      {
         return theNumberOfImageCodesPerRow;
      }
   ossim_uint32 getImageCodeLength()const
      {
         return theImageCodeBitLength;
      }
   
private:
   void clearFields();

   ossim_uint32 theStartOffset;
   ossim_uint32 theEndOffset;
   
   ossim_uint32 theNumberOfImageRows;
   ossim_uint32 theNumberOfImageCodesPerRow;
   ossim_uint8  theImageCodeBitLength;
};

#endif

//*******************************************************************
// License: MIT
//
// Author: Garrett Potts
// 
// Description: This class extends the stl's string class.
//
//********************************************************************
// $Id$

#include <ossim/support_data/ossimRpfImageDisplayParameterSubheader.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimErrorCodes.h>
#include <ossim/base/ossimIoStream.h>

std::ostream& operator<<(std::ostream& out,
                         const ossimRpfImageDisplayParameterSubheader& data)
{
   data.print(out);

   return out;
}

ossimRpfImageDisplayParameterSubheader::ossimRpfImageDisplayParameterSubheader()
{
   clearFields();
}

ossimErrorCode ossimRpfImageDisplayParameterSubheader::parseStream(ossim::istream& in,
                                                                   ossimByteOrder byteOrder)
{
   clearFields();
   if(in)
   {
      theStartOffset = in.tellg();
      in.read((char*)&theNumberOfImageRows, 4);
      in.read((char*)&theNumberOfImageCodesPerRow, 4);
      in.read((char*)&theImageCodeBitLength, 1);
      theEndOffset = in.tellg();

      ossimEndian anEndian;
      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theNumberOfImageRows);
         anEndian.swap(theNumberOfImageCodesPerRow);
      }
   }
   else
   {
      return ossimErrorCodes::OSSIM_ERROR;
   }
   
   return ossimErrorCodes::OSSIM_OK;
}
   
void ossimRpfImageDisplayParameterSubheader::print(std::ostream& out)const
{
   out << "theNumberOfImageRows:          " << theNumberOfImageRows << "\n"
       << "theNumberOfImageCodesPerRow:   " << theNumberOfImageCodesPerRow << "\n"
       << "theImageCodeBitLength:         " << (unsigned long)theImageCodeBitLength;
}

void ossimRpfImageDisplayParameterSubheader::clearFields()
{
   theNumberOfImageRows        = 0;
   theNumberOfImageCodesPerRow = 0;
   theImageCodeBitLength       = 0;
}

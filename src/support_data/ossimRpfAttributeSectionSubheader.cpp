//*******************************************************************
//
// License: MIT
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id$

#include <ossim/support_data/ossimRpfAttributeSectionSubheader.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimErrorCodes.h>
#include <ossim/base/ossimIoStream.h>

std::ostream& operator <<(
   std::ostream& out, const ossimRpfAttributeSectionSubheader& data)
{
   data.print(out);
   
   return out;
}

ossimRpfAttributeSectionSubheader::ossimRpfAttributeSectionSubheader()
{
   clearFields();
}

ossimErrorCode ossimRpfAttributeSectionSubheader::parseStream(ossim::istream& in,
                                                              ossimByteOrder byteOrder)
{
   theAttributeSectionSubheaderStart = 0;
   theAttributeSectionSubheaderEnd = 0;
   if(in)
   {
      theAttributeSectionSubheaderStart = in.tellg();
      in.read((char*)&theNumberOfAttributeOffsetRecords, 2);
      in.read((char*)&theNumberOfExplicitArealCoverageRecords, 2);
      in.read((char*)&theAttributeOffsetTableOffset, 4);
      in.read((char*)&theAttribteOffsetRecordLength, 2);
      ossimEndian anEndian;

      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theNumberOfAttributeOffsetRecords);
         anEndian.swap(theNumberOfExplicitArealCoverageRecords);
         anEndian.swap(theAttributeOffsetTableOffset);
         anEndian.swap(theAttribteOffsetRecordLength);
      }
      theAttributeSectionSubheaderEnd = in.tellg();
   }
   else
   {
      return ossimErrorCodes::OSSIM_ERROR;
   }
   
   return ossimErrorCodes::OSSIM_OK;
}

void ossimRpfAttributeSectionSubheader::print(std::ostream& out)const
{
   out << "theNumberOfAttributeOffsetRecords:         " << theNumberOfAttributeOffsetRecords
       << "\ntheNumberOfExplicitArealCoverageRecords:   " << theNumberOfExplicitArealCoverageRecords
       << "\ntheAttributeOffsetTableOffset:             " << theAttributeOffsetTableOffset
       << "\ntheAttribteOffsetRecordLength:             " << theAttribteOffsetRecordLength;
}

void ossimRpfAttributeSectionSubheader::clearFields()
{
   theNumberOfAttributeOffsetRecords       = 0;
   theNumberOfExplicitArealCoverageRecords = 0;
   theAttributeOffsetTableOffset           = 0;
   theAttribteOffsetRecordLength           = 0;
   
   theAttributeSectionSubheaderStart       = 0;
   theAttributeSectionSubheaderEnd         = 0;
}

ossim_uint64 ossimRpfAttributeSectionSubheader::getSubheaderStart()const
{
   return theAttributeSectionSubheaderStart;
}

ossim_uint64 ossimRpfAttributeSectionSubheader::getSubheaderEnd()const
{
   return theAttributeSectionSubheaderEnd;
}

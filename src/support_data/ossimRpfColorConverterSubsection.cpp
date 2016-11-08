//*******************************************************************
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//********************************************************************
// $Id$

#include <ossim/support_data/ossimRpfColorConverterSubsection.h>
#include <ossim/support_data/ossimRpfColorConverterOffsetRecord.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimErrorCodes.h>
#include <ossim/base/ossimIoStream.h>
#include <iterator>

std::ostream& operator <<(std::ostream& out,
                          const ossimRpfColorConverterSubsection& data)
{
   data.print(out);
   return out;
}

ossimRpfColorConverterSubsection::ossimRpfColorConverterSubsection()
{
   theNumberOfColorConverterOffsetRecords = 0;
   clearFields();
}

void ossimRpfColorConverterSubsection::clearFields()
{
   theColorConverterOffsetTableOffset     = 0;
   theColorConverterOffsetRecordLength    = 0;
   theConverterRecordLength               = 0;

   theTableList.clear();
}

const ossimRpfColorConverterTable* ossimRpfColorConverterSubsection::getColorConversionTable(ossim_uint32 givenThisNumberOfEntires)const
{
   std::vector<ossimRpfColorConverterTable>::const_iterator listElement = theTableList.begin();

   while(listElement != theTableList.end())
   {
      if((*listElement).getNumberOfEntries() == givenThisNumberOfEntires)
      {
         return &(*listElement);
      }
   }

   return NULL;
}

ossimErrorCode ossimRpfColorConverterSubsection::parseStream(ossim::istream& in,
                                                             ossimByteOrder byteOrder)
{
   if(in)
   {
      clearFields();

      // this is the start of the subsection
      theStartOffset = in.tellg();
      in.read((char*)&theColorConverterOffsetTableOffset, 4);
      in.read((char*)&theColorConverterOffsetRecordLength, 2);
      in.read((char*)&theConverterRecordLength, 2);

      // this grabs the end of the subsection
      theEndOffset   = in.tellg();

      ossimEndian anEndian;

      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theColorConverterOffsetTableOffset);
         anEndian.swap(theColorConverterOffsetRecordLength);
         anEndian.swap(theConverterRecordLength);

      }
      theTableList.resize(theNumberOfColorConverterOffsetRecords);
      for(unsigned long index = 0;
          index < theNumberOfColorConverterOffsetRecords;
          ++index)
      {
         ossimRpfColorConverterOffsetRecord recordInfo;
         
         if(recordInfo.parseStream(in, byteOrder) ==
            ossimErrorCodes::OSSIM_OK)
         {
            unsigned long rememberGet = in.tellg();

            theTableList[index].setNumberOfEntries(recordInfo.theNumberOfColorConverterRecords);
            theTableList[index].setTableId(recordInfo.theColorConverterTableId);
            in.seekg(theStartOffset + recordInfo.theColorConverterTableOffset, std::ios_base::beg);
            theTableList[index].parseStream(in, byteOrder);
            
            in.seekg(rememberGet, std::ios_base::beg);
         }
         else
         {
            return ossimErrorCodes::OSSIM_ERROR;
         }
      }
   }
   else
   {
      return ossimErrorCodes::OSSIM_ERROR;
   }

   return ossimErrorCodes::OSSIM_OK;
}

void ossimRpfColorConverterSubsection::setNumberOfColorConverterOffsetRecords(ossim_uint16 numberOfRecords)
{
   theNumberOfColorConverterOffsetRecords = numberOfRecords;
}

void ossimRpfColorConverterSubsection::print(std::ostream& out)const
{
   out << "theColorConverterOffsetTableOffset:      "
       << theColorConverterOffsetTableOffset
       << "\ntheColorConverterOffsetRecordLength:     "
       << theColorConverterOffsetRecordLength
       << "\ntheConverterRecordLength:                "
       << theConverterRecordLength << "\n";

   copy(theTableList.begin(),
        theTableList.end(),
        std::ostream_iterator<ossimRpfColorConverterTable>(out, "\n"));
}

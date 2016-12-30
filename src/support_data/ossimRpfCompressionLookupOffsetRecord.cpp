#include <ossim/support_data/ossimRpfCompressionLookupOffsetRecord.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimErrorCodes.h>
#include <ossim/base/ossimIoStream.h>

std::ostream& operator <<(std::ostream &out,
                          const ossimRpfCompressionLookupOffsetRecord& data)
{
   data.print(out);
   
   return out;
}

ossimRpfCompressionLookupOffsetRecord::ossimRpfCompressionLookupOffsetRecord()
{
   clearFields();
}

ossimErrorCode ossimRpfCompressionLookupOffsetRecord::parseStream(ossim::istream& in,
                                                                  ossimByteOrder byteOrder)
{
   if(in)
   {
      ossimEndian anEndian;

      in.read((char*)&theCompressionLookupTableId, 2);
      in.read((char*)&theNumberOfCompressionLookupRecords, 4);
      in.read((char*)&theNumberOfValuesPerCompressionLookupRecord, 2);
      in.read((char*)&theCompressionLookupValueBitLength, 2);
      in.read((char*)&theCompressionLookupTableOffset, 4);
      
      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theCompressionLookupTableId);
         anEndian.swap(theNumberOfCompressionLookupRecords);
         anEndian.swap(theNumberOfValuesPerCompressionLookupRecord);
         anEndian.swap(theCompressionLookupValueBitLength);
         anEndian.swap(theCompressionLookupTableOffset);
      }
   }
   else
   {
      return ossimErrorCodes::OSSIM_ERROR;
   }
   
   return ossimErrorCodes::OSSIM_OK;
}

void ossimRpfCompressionLookupOffsetRecord::print(std::ostream& out)const
{
   out << "theCompressionLookupTableId:                 " << theCompressionLookupTableId
       << "\ntheNumberOfCompressionLookupRecords:         " << theNumberOfCompressionLookupRecords
       << "\ntheNumberOfValuesPerCompressionLookupRecord: " << theNumberOfValuesPerCompressionLookupRecord
       << "\ntheCompressionLookupValueBitLength:          " << theCompressionLookupValueBitLength
       << "\ntheCompressionLookupTableOffset:             " << theCompressionLookupTableOffset;
}

void ossimRpfCompressionLookupOffsetRecord::clearFields()
{
   theCompressionLookupTableId                  = 0;
   theNumberOfCompressionLookupRecords          = 0;
   theNumberOfValuesPerCompressionLookupRecord  = 0;
   theCompressionLookupValueBitLength           = 0;
   theCompressionLookupTableOffset              = 0;
}

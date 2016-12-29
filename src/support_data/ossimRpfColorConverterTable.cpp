#include <ossim/support_data/ossimRpfColorConverterTable.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimErrorCodes.h>
#include <ossim/base/ossimIoStream.h>

std::ostream& operator <<(
   std::ostream& out, const ossimRpfColorConverterTable& data)
{
   data.print(out);
   return out;
}

ossimRpfColorConverterTable::ossimRpfColorConverterTable()
{
   theTableId                      = 0;
   theNumberOfEntries              = 0;
   theColorGrayscaleTableEntryList = NULL;
}

ossimRpfColorConverterTable::~ossimRpfColorConverterTable()
{
   if(theColorGrayscaleTableEntryList)
   {
      delete [] theColorGrayscaleTableEntryList;
      theColorGrayscaleTableEntryList = NULL;
   }
}
ossimErrorCode ossimRpfColorConverterTable::parseStream(ossim::istream& in,
                                                        ossimByteOrder byteOrder)
{
   if(in)
   {
      if(theNumberOfEntries > 0)
      {
         in.read((char*)theColorGrayscaleTableEntryList, 4*theNumberOfEntries);

         ossimEndian anEndian;
         if(anEndian.getSystemEndianType() != byteOrder)
         {
            for(ossim_uint32 index = 0;
                index < theNumberOfEntries;
                ++index)
            {
               anEndian.swap(theColorGrayscaleTableEntryList[index]);
            }
         }
      }
   }
   else
   {
      return ossimErrorCodes::OSSIM_ERROR;
   }

   return ossimErrorCodes::OSSIM_OK;
}

void ossimRpfColorConverterTable::print(std::ostream& out)const
{
   out << "theTableId:                   " << theTableId << "\n"
       << "theNumberOfEntries:           " << theNumberOfEntries << "\n";

   if(theColorGrayscaleTableEntryList)
   {
      out << "Values:\n";
      for(ossim_uint32 index=0; index < theNumberOfEntries; index++)
      {
         out<< theColorGrayscaleTableEntryList[index] << "\n";
      }
   }
   
}

void ossimRpfColorConverterTable::setNumberOfEntries(ossim_uint32 entries)
{
   if(theColorGrayscaleTableEntryList)
   {
      delete [] theColorGrayscaleTableEntryList;
      theColorGrayscaleTableEntryList = NULL;
   }
   theColorGrayscaleTableEntryList = new ossim_uint32[entries];
   theNumberOfEntries = entries;
   
   for(ossim_uint32 index=0;
       index < theNumberOfEntries;
       ++index)
   {
      theColorGrayscaleTableEntryList[index] = 0;
   }
}
void ossimRpfColorConverterTable::setTableId(ossim_uint16 id)
{
   theTableId = id;
}

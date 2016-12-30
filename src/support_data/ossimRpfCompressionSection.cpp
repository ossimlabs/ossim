#include <ossim/support_data/ossimRpfCompressionSection.h>
#include <ossim/support_data/ossimRpfCompressionSectionSubheader.h>
#include <ossim/support_data/ossimRpfCompressionLookupOffsetRecord.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimErrorCodes.h>
#include <ossim/base/ossimIoStream.h>
#include <cstring> // for memset
#include <iterator>

std::ostream& operator<<(std::ostream& out,
                         const ossimRpfCompressionOffsetTableData& data)
{
   unsigned long size = (data.theNumberOfValuesPerLookup*data.theNumberOfLookupValues*
                         data.theCompressionLookupValueBitLength)/8;
   
   out << "theTableId:                         " << data.theTableId
       << "\ntheNumberOfLookupValues:            " << data.theNumberOfLookupValues
       << "\ntheCompressionLookupValueBitLength: " << data.theCompressionLookupValueBitLength
       << "\ntheNumberOfValuesPerLookup:         " << data.theNumberOfValuesPerLookup
       << "\nData Size in bytes:                 "
       << size;

   return out;
}


ossimRpfCompressionOffsetTableData::ossimRpfCompressionOffsetTableData()
   :theTableId(0),
    theNumberOfLookupValues(0),
    theCompressionLookupValueBitLength(0),
    theNumberOfValuesPerLookup(0),
    theData(NULL)
{
}

ossimRpfCompressionOffsetTableData::ossimRpfCompressionOffsetTableData(
   const ossimRpfCompressionOffsetTableData& rhs)
   :theNumberOfLookupValues(0),
    theCompressionLookupValueBitLength(0),
    theNumberOfValuesPerLookup(0),
    theData(NULL)
{
   *this = rhs;
}

const ossimRpfCompressionOffsetTableData& ossimRpfCompressionOffsetTableData::operator = (
   const ossimRpfCompressionOffsetTableData& rhs)
{
   if(this != &rhs)
   {
      if(theData)
      {
         delete [] theData;
         theData = 0;
      }

      if(rhs.theData)
      {
         theTableId                         = rhs.theTableId;
         theNumberOfLookupValues            = rhs.theNumberOfLookupValues;
         theCompressionLookupValueBitLength = rhs.theCompressionLookupValueBitLength;
         theNumberOfValuesPerLookup         = rhs.theNumberOfValuesPerLookup;
         
         unsigned long size = (theNumberOfValuesPerLookup*
                               theNumberOfLookupValues*
                               theCompressionLookupValueBitLength)/8;
         if(size > 0)
         {
            theData = new unsigned char[size];
            memcpy(theData, rhs.theData, size);
         }
      }
      else
      {
         theTableId = 0;
         theNumberOfLookupValues = 0;
         theCompressionLookupValueBitLength = 0;
         theNumberOfValuesPerLookup = 0;
      }
   }
   
   return *this;
}

ossimRpfCompressionOffsetTableData::~ossimRpfCompressionOffsetTableData()
{
   if(theData)
   {
      delete [] theData;
      theData = 0;
   }
}


std::ostream& operator << (
   std::ostream& out, const ossimRpfCompressionSection& data)
{
   data.print(out);
   return out;
}

ossimRpfCompressionSection::ossimRpfCompressionSection()
   :theSubheader(NULL)
{
   theSubheader = new ossimRpfCompressionSectionSubheader;
}

ossimRpfCompressionSection::~ossimRpfCompressionSection()
{
   if(theSubheader)
   {
      delete theSubheader;
      theSubheader = NULL;
   }
}

ossimErrorCode ossimRpfCompressionSection::parseStream(ossim::istream& in,
                                                       ossimByteOrder byteOrder)
{
   ossimErrorCode result = ossimErrorCodes::OSSIM_OK;
   
   if(in&&theSubheader)
   {
      result = theSubheader->parseStream(in, byteOrder);

      if(result == ossimErrorCodes::OSSIM_OK)
      {
         ossimEndian anEndian;
         ossimRpfCompressionLookupOffsetRecord record;
         
         in.read((char*)&theCompressionLookupOffsetTableOffset, 4);
         in.read((char*)&theCompressionLookupTableOffsetRecordLength, 2);
         
         if(byteOrder != anEndian.getSystemEndianType())
         {
            anEndian.swap(theCompressionLookupOffsetTableOffset);
            anEndian.swap(theCompressionLookupTableOffsetRecordLength);
         }

         if(theSubheader->getNumberOfCompressionLookupOffsetRecords() > 0)
         {
            // clear the table
            theTable.clear();

            // resize it
            theTable.resize(theSubheader->getNumberOfCompressionLookupOffsetRecords());
            
            // now populate it
            for(long index = 0;
                index < theSubheader->getNumberOfCompressionLookupOffsetRecords();
                ++index)
            {
               record.parseStream(in, byteOrder);
               unsigned long rememberLocation = in.tellg();
               in.seekg(record.getCompressionLookupTableOffset()+
                        theSubheader->getEndOffset(), std::ios_base::beg);

               theTable[index].theTableId                            = record.getCompressionLookupTableId();
               theTable[index].theNumberOfLookupValues               = record.getNumberOfCompressionLookupRecords();
               theTable[index].theCompressionLookupValueBitLength    = record.getCompressionLookupValueBitLength();
               theTable[index].theNumberOfValuesPerLookup            = record.getNumberOfValuesPerCompressionLookupRecord();
               
               // store the information about this compress/decompress algorithm
               // I am not sure but I will assume that the bit length can be
               // arbitrary.  In other words if someone says that the bit length of
               // the lookup value is 12 this should be handled ok.
               //
               unsigned long size = (theTable[index].theNumberOfValuesPerLookup*
                                     theTable[index].theNumberOfLookupValues*
                                     theTable[index].theCompressionLookupValueBitLength)/8;
               
               theTable[index].theData = new unsigned char[size];

               in.read( (char*)theTable[index].theData, size);
               
               in.seekg(rememberLocation, std::ios_base::beg);
            }
         }
      }
   }
   else
   {
      result = ossimErrorCodes::OSSIM_ERROR;
   }

   return result;
}

void ossimRpfCompressionSection::print(std::ostream& out)const
{
   if(theSubheader)
   {
      out << *theSubheader << "\n";
      copy(theTable.begin(),
           theTable.end(),
           std::ostream_iterator<ossimRpfCompressionOffsetTableData>(out, "\n"));
   }
}


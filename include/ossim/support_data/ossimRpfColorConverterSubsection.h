#ifndef ossimRpfColorConverterSubsection_HEADER
#define ossimRpfColorConverterSubsection_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIosFwd.h>
#include <ossim/base/ossimErrorContext.h>
#include <ossim/support_data/ossimRpfColorConverterTable.h>
#include <vector>

class ossimRpfColorConverterSubsection
{
public:
   friend std::ostream& operator <<(
      std::ostream& out, const ossimRpfColorConverterSubsection& data);

   ossimRpfColorConverterSubsection();
   virtual ~ossimRpfColorConverterSubsection(){}

   ossimErrorCode parseStream(ossim::istream& in,
                              ossimByteOrder byteOrder);
   
   ossim_uint32 getStartOffset()const{return theStartOffset;}
   ossim_uint32 getEndOffset()const{return theEndOffset;}

   const std::vector<ossimRpfColorConverterTable>& getColorConversionTable()const
   {
      return theTableList;
   }
   const ossimRpfColorConverterTable* getColorConversionTable(ossim_uint32 givenThisNumberOfEntires)const;
   
   void setNumberOfColorConverterOffsetRecords(ossim_uint16 numberOfRecords);
   void print(std::ostream& out)const;

   void clearFields();
   
private:
   ossimRpfColorConverterSubsection(const ossimRpfColorConverterSubsection&){}//hide
   void operator =(const ossimRpfColorConverterSubsection&){}//hide

   ossim_uint32  theStartOffset;
   ossim_uint32  theEndOffset;
   
   ossim_uint16  theNumberOfColorConverterOffsetRecords;
   
   ossim_uint32  theColorConverterOffsetTableOffset;
   ossim_uint16  theColorConverterOffsetRecordLength;
   ossim_uint16  theConverterRecordLength;

   std::vector<ossimRpfColorConverterTable> theTableList;
};

#endif

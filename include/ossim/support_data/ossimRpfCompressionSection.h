#ifndef ossimRpfCompressionSection_HEADER
#define ossimRpfCompressionSection_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimErrorContext.h>
#include <ossim/base/ossimIosFwd.h>
#include <vector>

class ossimRpfCompressionSectionSubheader;

struct ossimRpfCompressionOffsetTableData
{
   friend std::ostream& operator<<(
      std::ostream& out, const ossimRpfCompressionOffsetTableData& data);
   ossimRpfCompressionOffsetTableData();
   ossimRpfCompressionOffsetTableData(const ossimRpfCompressionOffsetTableData& rhs);
   ~ossimRpfCompressionOffsetTableData();
   const ossimRpfCompressionOffsetTableData& operator =(const ossimRpfCompressionOffsetTableData& rhs);
   
   ossim_uint16 theTableId;
   ossim_uint32 theNumberOfLookupValues;
   ossim_uint16 theCompressionLookupValueBitLength;
   ossim_uint16 theNumberOfValuesPerLookup;
   ossim_uint8* theData;
};

class ossimRpfCompressionSection
{
public:
   friend std::ostream& operator << (
      std::ostream& out, const ossimRpfCompressionSection& data);
   ossimRpfCompressionSection();
   virtual ~ossimRpfCompressionSection();
   ossimErrorCode parseStream(ossim::istream& in,
                              ossimByteOrder byteOrder);
   void print(std::ostream& out)const;
   const std::vector<ossimRpfCompressionOffsetTableData>& getTable()const
      {
         return theTable;
      }
   const ossimRpfCompressionSectionSubheader* getSubheader()const
   {
      return theSubheader;
   }
private:
   void clearTable();
   
   ossimRpfCompressionSectionSubheader* theSubheader;
   
   ossim_uint32 theCompressionLookupOffsetTableOffset;
   ossim_uint16 theCompressionLookupTableOffsetRecordLength;

   std::vector<ossimRpfCompressionOffsetTableData> theTable;
};

#endif

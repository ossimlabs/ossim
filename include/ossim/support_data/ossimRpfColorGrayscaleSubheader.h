#ifndef ossimRpfColorGrayscaleSubheader_HEADER
#define ossimRpfColorGrayscaleSubheader_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimErrorContext.h>
#include <ossim/base/ossimIosFwd.h>

class ossimRpfColorGrayscaleSubheader
{
public:
   friend std::ostream& operator <<(
      std::ostream& out, const ossimRpfColorGrayscaleSubheader& data);
   
   ossimRpfColorGrayscaleSubheader();
   ~ossimRpfColorGrayscaleSubheader(){}
   
   ossimErrorCode parseStream(ossim::istream& in,
                              ossimByteOrder byteOrder);
   void print(std::ostream& out)const;
   unsigned long getStartOffset()const
      {
         return theStartOffset;
      }
   unsigned long getEndOffset()const
      {
         return theEndOffset;
      }
   unsigned long getNumberOfColorGreyscaleOffsetRecords()const
      {
         return theNumberOfColorGreyscaleOffsetRecords;
      }
   unsigned long getNumberOfColorConverterOffsetRecords()const
      {
         return theNumberOfColorConverterOffsetRecords;
      }
   
private:
   void clearFields();

   unsigned long theStartOffset;
   unsigned long theEndOffset;
   
   unsigned char theNumberOfColorGreyscaleOffsetRecords;
   unsigned char theNumberOfColorConverterOffsetRecords;

   /*!
    * 12 byte field.
    */
   ossimString         theColorGrayscaleFilename;
};

#endif

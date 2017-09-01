#ifndef ossimRpfColorGrayscaleTable_HEADER
#define ossimRpfColorGrayscaleTable_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimErrorContext.h>
#include <ossim/base/ossimIosFwd.h>

class ossimRpfColorGrayscaleTable
{
public:
   friend std::ostream& operator <<(
      std::ostream& out, const ossimRpfColorGrayscaleTable& data);
   
   ossimRpfColorGrayscaleTable();
   ossimRpfColorGrayscaleTable(const ossimRpfColorGrayscaleTable& rhs);
   virtual ~ossimRpfColorGrayscaleTable();

   ossimErrorCode parseStream(std::istream& in, ossimByteOrder byteOrder);
   
   void setTableData(unsigned short id,
                     unsigned long  numberOfElements);
   
   const ossimRpfColorGrayscaleTable& operator =(const ossimRpfColorGrayscaleTable&);

   const unsigned char* getData()const{return theData;}
   const unsigned char* getStartOfData(unsigned long entry)const;
   unsigned short getTableId()const{return theTableId;}
   unsigned long getNumberOfElements()const{return theNumberOfElements;}
   
private:   
   unsigned long theNumberOfElements;

   unsigned long theTotalNumberOfBytes;
   
   /*!
    * This will not be parsed from the stream.  This is set when
    * reading the offset record for this table.  The id defines
    * the format of the data buffer. if the id is :
    *
    * 1           then the buffer is in the format of RGB
    * 2                     ""                        RGBM
    * 3                     ""                        M
    * 4                     ""                        CMYK
    */
   unsigned short theTableId;
   unsigned char* theData;
};

#endif

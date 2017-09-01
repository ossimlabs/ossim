//*******************************************************************
//
// License: MIT
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id$

#ifndef ossimRpfAttributeSectionSubheader_HEADER
#define ossimRpfAttributeSectionSubheader_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIosFwd.h>
#include <ossim/base/ossimErrorContext.h>

class ossimRpfAttributeSectionSubheader
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const ossimRpfAttributeSectionSubheader& data);
   ossimRpfAttributeSectionSubheader();

   virtual ~ossimRpfAttributeSectionSubheader(){}
   ossimErrorCode parseStream(ossim::istream& in, ossimByteOrder byteOrder);
   virtual void print(std::ostream& out)const;
   ossim_uint16 getNumberOfAttributeOffsetRecords()const
      {
         return theNumberOfAttributeOffsetRecords;
      }
   ossim_uint16 getAttributeOffsetTableOffset()const
      {
         return theAttributeOffsetTableOffset;
      }

   ossim_uint64 getSubheaderStart()const;
   ossim_uint64 getSubheaderEnd()const;
   
private:
   void clearFields();
   
   ossim_uint16 theNumberOfAttributeOffsetRecords;
   ossim_uint16 theNumberOfExplicitArealCoverageRecords;
   ossim_uint32 theAttributeOffsetTableOffset;
   ossim_uint16 theAttribteOffsetRecordLength;


   mutable ossim_uint64 theAttributeSectionSubheaderStart;
   mutable ossim_uint64 theAttributeSectionSubheaderEnd;
};

#endif

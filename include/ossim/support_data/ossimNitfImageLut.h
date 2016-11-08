//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License: MIT
// 
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id$

#ifndef ossimNitfImageLut_HEADER
#define ossimNitfImageLut_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIosFwd.h>
#include <ossim/base/ossimObject.h>

class ossimNitfImageLut : public ossimObject
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const ossimNitfImageLut &data);
   ossimNitfImageLut();
   virtual ~ossimNitfImageLut();
   virtual void parseStream(ossim::istream& in)=0;
   virtual std::ostream& print(std::ostream& out)const=0;
   virtual ossim_uint32 getNumberOfEntries()const=0;
   virtual void setNumberOfEntries(ossim_uint32 numberOfEntries)=0;
   virtual ossim_uint8 getValue(ossim_uint32 idx)const=0;

TYPE_DATA;
};
#endif

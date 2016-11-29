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

#include <cstring> // memcpy
#include <ostream>

#include <ossim/support_data/ossimNitfImageLut.h>

std::ostream& operator <<(std::ostream& out,
                          const ossimNitfImageLut &data)
{
   data.print(out);

   return out;
}

RTTI_DEF1(ossimNitfImageLut, "ossimNitfImageLut", ossimObject);
ossimNitfImageLut::ossimNitfImageLut()
{
}

ossimNitfImageLut::~ossimNitfImageLut()
{
}


//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// LICENSE: LGPL see top level LICENSE.txt
// 
// Author: Garrett Potts
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfRegisteredDesFactory.h 22875 2014-08-27 13:52:03Z dburken $
#ifndef ossimNitfRegisteredDesFactory_HEADER
#define ossimNitfRegisteredDesFactory_HEADER 1

#include <ossim/support_data/ossimNitfDesFactory.h>

class ossimNitfRegisteredDesFactory : public ossimNitfDesFactory
{
public:
   virtual ~ossimNitfRegisteredDesFactory();
   static ossimNitfRegisteredDesFactory* instance();
   
   virtual ossimRefPtr<ossimNitfRegisteredDes> create(const ossimString &desName)const;

protected:
   ossimNitfRegisteredDesFactory();

TYPE_DATA   
};

#endif

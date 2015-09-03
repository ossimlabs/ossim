//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// LICENSE: LGPL see top level LICENSE.txt
// 
// Author: Garrett Potts
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfRegisteredTagFactory.h 22875 2014-08-27 13:52:03Z dburken $
#ifndef ossimNitfRegisteredTagFactory_HEADER
#define ossimNitfRegisteredTagFactory_HEADER 1

#include <ossim/support_data/ossimNitfTagFactory.h>

class ossimNitfRegisteredTagFactory : public ossimNitfTagFactory
{
public:
   virtual ~ossimNitfRegisteredTagFactory();
   static ossimNitfRegisteredTagFactory* instance();
   
   virtual ossimRefPtr<ossimNitfRegisteredTag> create(const ossimString &tagName)const;

protected:
   ossimNitfRegisteredTagFactory();

TYPE_DATA   
};

#endif

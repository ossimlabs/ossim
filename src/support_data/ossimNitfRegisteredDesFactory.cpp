//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Garrett Potts
//
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfRegisteredDesFactory.cpp 23113 2015-01-28 17:04:17Z gpotts $
#include <ossim/support_data/ossimNitfRegisteredDesFactory.h>

#include <ossim/support_data/ossimNitfSicdXmlDataContentDes.h>

#include "support_data/ossimNitfCsattbDes.h"
#include "support_data/ossimNitfCssfabDes.h"

RTTI_DEF1(ossimNitfRegisteredDesFactory, "ossimNitfRegisteredDesFactory", ossimNitfDesFactory);

static const char XML_DATA_CONTENT_DES[]                = "XML_DATA_CONTENT";
static const char SICD_XML[]                            = "SICD_XML";
static const char CSATTB_DES[]                          = "CSSFAB";
static const char CSSFAB_DES[]                          = "CSSFAB";

ossimNitfRegisteredDesFactory::ossimNitfRegisteredDesFactory()
{
}

ossimNitfRegisteredDesFactory::~ossimNitfRegisteredDesFactory()
{
}

ossimNitfRegisteredDesFactory* ossimNitfRegisteredDesFactory::instance()
{
   static ossimNitfRegisteredDesFactory inst;
   return &inst;
}

ossimRefPtr<ossimNitfRegisteredDes> ossimNitfRegisteredDesFactory::create(
   const ossimString& desName)const
{
   ossimString name = ossimString(desName).trim().upcase();
   ossimRefPtr<ossimNitfRegisteredDes> result;
   // We have removed the generic XML_DATA_CONTENT for the SICD
   // we will need to have a different parsers for that.
   // If it explicitly specifies SICD then we will assume SICD 
   // for the parse
   //
   if(desName == SICD_XML)
   {
       result = new ossimNitfSicdXmlDataContentDes;

       result->setDesName(desName);
   }
   else if(desName == CSATTB_DES)
   {
      result = new ossimNitfCsattbDes;

      result->setDesName(desName);
   }
   else if(desName == CSSFAB_DES)
   {
      result = new ossimNitfCssfabDes;

      result->setDesName(desName);
   }
   return result;
}

//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:  Factory class definition for codec(encoder/decoder).
// 
//----------------------------------------------------------------------------
// $Id$

#include <ossim/imaging/ossimCodecFactory.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>

#include <ossim/imaging/ossimJpegCodec.h>

#include <string>

ossimCodecFactory* ossimCodecFactory::theInstance = 0;

static const std::string TYPE_KW    = "type";

ossimCodecFactory::~ossimCodecFactory()
{}

ossimCodecFactory* ossimCodecFactory::instance()
{
   if ( !theInstance )
   {
      theInstance = new ossimCodecFactory();
   }
   return theInstance;
}
ossimCodecBase* ossimCodecFactory::createCodec(const ossimString& type)const
{
   ossimRefPtr<ossimCodecBase> result;

   if((type.downcase() == "jpeg") ||
      (type == "ossimJpegCodec"))
   {
      result =  new ossimJpegCodec();
   }

   return result.release();
}

ossimCodecBase* ossimCodecFactory::createCodec(const ossimKeywordlist& kwl, const char* prefix)const
{
   ossimString type = kwl.find(prefix, ossimKeywordNames::TYPE_KW);
   ossimCodecBase* result = 0;
   if(!type.empty())
   {
      result = this->createCodec(type);
      if(result)
      {
         result->loadState(kwl, prefix);
      }
   }

   return result;
}

void ossimCodecFactory::getTypeNameList(std::vector<ossimString>& typeNames)const
{
   typeNames.push_back("jpeg");
}

ossimCodecFactory::ossimCodecFactory()
{}

ossimCodecFactory::ossimCodecFactory(const ossimCodecFactory& /* obj */ )
{}

const ossimCodecFactory& ossimCodecFactory::operator=(
   const ossimCodecFactory& /* rhs */)
{
   return *this;
}

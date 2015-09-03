#include <ossim/imaging/ossimCodecFactoryInterface.h>
#include <ossim/base/ossimObject.h>
#include <ossim/imaging/ossimCodecBase.h>

ossimObject* ossimCodecFactoryInterface::createObject(const ossimString& type)const
{
  return createCodec(type);
}

ossimObject* ossimCodecFactoryInterface::createObject(const ossimKeywordlist& kwl, const char* prefix)const
{
  return createCodec(kwl, prefix);
}


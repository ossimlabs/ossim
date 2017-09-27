#include <ossim/imaging/TiffHandlerState.h>

const ossimString ossim::TiffHandlerState::m_typeName = "ossim::TiffHandlerState";

ossim::TiffHandlerState::TiffHandlerState()
{

}
      
ossim::TiffHandlerState::~TiffHandlerState()
{

}

const ossimString& ossim::TiffHandlerState::getTypeName()const
{
   return m_typeName;
}

const ossimString& ossim::TiffHandlerState::getStaticTypeName()
{
   return m_typeName;
}

void ossim::TiffHandlerState::load(const ossimKeywordlist& kwl,
                                   const ossimString& prefix)
{
   ossim::ImageHandlerState::load(kwl, prefix);
}

void ossim::TiffHandlerState::save(ossimKeywordlist& kwl,
                                   const ossimString& prefix)const
{
   ossim::ImageHandlerState::save(kwl, prefix);

}


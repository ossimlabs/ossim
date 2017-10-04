//*******************************************************************
//
// License:  See top level LICENSE.txt file
//
//*************************************************************************

#include <ossim/imaging/ImageHandlerState.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/imaging/ImageHandlerStateRegistry.h>

const ossimString ossim::ImageHandlerState::m_typeName = "ossim::ImageHandlerState";

ossim::ImageHandlerState::ImageHandlerState()
: State(),
m_currentEntry("0")
{

}

ossim::ImageHandlerState::ImageHandlerState::~ImageHandlerState()
{

}

const ossimString& ossim::ImageHandlerState::getTypeName()const
{
   return m_typeName;
}

const ossimString& ossim::ImageHandlerState::getStaticTypeName()
{
   return m_typeName;
}

void ossim::ImageHandlerState::load(const ossimKeywordlist& kwl,
                                    const ossimString& prefix)
{
   ossim::State::load(kwl, prefix);
   m_overviewState = ossim::ImageHandlerStateRegistry::instance()->createState(kwl, prefix+"overview.");
   m_connectionString = kwl.find(prefix, "connection_string");
   m_imageHandlerType = kwl.find(prefix, "image_handler_type");
   m_currentEntry     = kwl.find(prefix, "current_entry");

   if(m_currentEntry.empty()) m_currentEntry = "0";
}

void ossim::ImageHandlerState::save(ossimKeywordlist& kwl,
                                    const ossimString& prefix)const
{
   ossim::State::save(kwl, prefix);
   if(m_overviewState)
   {
      ossimString tempPrefix = prefix + "overview.";
      m_overviewState->save(kwl, tempPrefix);
   }
   kwl.add(prefix, "connection_string",  m_connectionString.c_str());
   kwl.add(prefix, "image_handler_type", m_imageHandlerType.c_str());
   kwl.add(prefix, "current_entry",      m_currentEntry.c_str());
}

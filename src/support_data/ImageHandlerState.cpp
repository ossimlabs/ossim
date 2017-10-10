//*******************************************************************
//
// License:  See top level LICENSE.txt file
//
//*************************************************************************

#include <ossim/support_data/ImageHandlerState.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/support_data/ImageHandlerStateRegistry.h>

const ossimString ossim::ImageHandlerState::m_typeName = "ossim::ImageHandlerState";

ossim::ImageHandlerState::ImageHandlerState()
: State(),
m_currentEntry(0)
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

bool ossim::ImageHandlerState::hasMetaData()const
{
  if(m_omd)
  {
      return m_omd->isValid(); 
  }

  return false;
}

void ossim::ImageHandlerState::load(const ossimKeywordlist& kwl,
                                    const ossimString& prefix)
{
   ossim::State::load(kwl, prefix);
   m_overviewState = ossim::ImageHandlerStateRegistry::instance()->createState(kwl, prefix+"overview.");
   m_connectionString = kwl.find(prefix, "connection_string");
   m_imageHandlerType = kwl.find(prefix, "image_handler_type");
   ossimString currentEntry = kwl.find(prefix, "current_entry");

   if(currentEntry.empty())
   {
      m_currentEntry = 0;
   } 
   else
   {
      m_currentEntry = currentEntry.toUInt32();
   }

   ossimString omdType = kwl.find(prefix, "ossimImageMetaData");
   if(!omdType.empty())
   {
      m_omd = std::make_shared<ossimImageMetaData>();
      
      m_omd->loadState(kwl, prefix+"omd.");
   }
   ossimKeywordlist tempKwl;
   kwl.extractKeysThatMatch(tempKwl, "^("+prefix+"valid_vertices.)");
   if(tempKwl.getSize())
   {
      tempKwl.stripPrefixFromAll("^("+prefix+"valid_vertices.)");
      m_validVertices = std::make_shared<ossimKeywordlist>(tempKwl);
   }
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
   if(m_omd)
   {
      ossimString tempPrefix = prefix + "omd.";
      m_omd->saveState(kwl, tempPrefix);      
   }
   kwl.add(prefix, "connection_string",  m_connectionString.c_str(), true);
   kwl.add(prefix, "image_handler_type", m_imageHandlerType.c_str(), true);
   kwl.add(prefix, "current_entry",      m_currentEntry, true);

   if(m_validVertices)
   {
      ossimString validVerticesPrefix = prefix+"valid_vertices.";
      kwl.add(validVerticesPrefix.c_str(), *m_validVertices);
      kwl.add(validVerticesPrefix, "type", "ossimValidVertices");
   }
}

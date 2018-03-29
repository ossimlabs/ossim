//----------------------------------------------------------------------------
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Factory class declaration for codec(encoder/decoder).
// 
//----------------------------------------------------------------------------
// $Id$
#include <ossim/imaging/ossimNitfCodecFactory.h>
#include <ossim/imaging/ossimCodecFactoryRegistry.h>
 
ossimNitfCodecFactory* ossimNitfCodecFactory::m_instance=0;
std::mutex ossimNitfCodecFactory::m_mutex;

ossimNitfCodecFactory::ossimNitfCodecFactory()
{
   m_instance = this;
}

ossimNitfCodecFactory::~ossimNitfCodecFactory()
{
   std::lock_guard<std::mutex> lock(m_mutex);
   m_instance = 0;
}


ossimNitfCodecFactory* ossimNitfCodecFactory::instance()
{
   std::lock_guard<std::mutex> lock(m_mutex);
   if(!m_instance)
   {
      m_instance = new ossimNitfCodecFactory();
   }

   return m_instance;
}

ossimCodecBase* ossimNitfCodecFactory::createCodec(ossimRefPtr<ossimNitfImageHeader> imageHeader)
{
   ossimCodecBase* result=0;

   if(imageHeader.valid()&&imageHeader->isCompressed())
   {
      ossimKeywordlist kwl;

      imageHeader->saveState(kwl);
      // rename the type to be a NITF IC compression type
      // J2K should be C8
      //
      kwl.add("type", imageHeader->getCompressionCode().c_str(), true);
      result = ossimCodecFactoryRegistry::instance()->createCodec(kwl);
   }

   return result;
}

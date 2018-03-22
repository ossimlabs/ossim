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
#ifndef ossimNitfCodecFactory_HEADER
#define ossimNitfCodecFactory_HEADER
#include <ossim/imaging/ossimCodecBase.h>
#include <ossim/support_data/ossimNitfImageHeader.h>
#include <mutex>

/**
* This is a convenience class that is used by the NITF handler to create the proper keywordlist
* from the TREs and then calls the CodecRegistry to actuall return and allocate a new codec 
*/
class ossimNitfCodecFactory
{
public:
   ~ossimNitfCodecFactory();
   static ossimNitfCodecFactory* instance();

   ossimCodecBase* createCodec(ossimRefPtr<ossimNitfImageHeader> imageHeader);
protected:
   static std::mutex m_mutex;
   static ossimNitfCodecFactory* m_instance;
   ossimNitfCodecFactory();
};


#endif

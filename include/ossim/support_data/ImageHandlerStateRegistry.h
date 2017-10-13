//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
//*****************************************************************************
#ifndef ossimImageHandlerStateRegistry_HEADER
#define ossimImageHandlerStateRegistry_HEADER 1
#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ImageHandlerState.h>
#include <ossim/base/StateFactoryBase.h>
#include <ossim/base/FactoryListBase.h>
#include <memory>

namespace ossim
{
   /**
   * This is the state registry for all image handler states that
   * will be supported.
   */
   class OSSIM_DLL ImageHandlerStateRegistry : public ossim::FactoryListBase<
                                                      std::shared_ptr<ossim::StateFactoryBase< std::shared_ptr<ossim::ImageHandlerState> > > >
   {
   public:
      ImageHandlerStateRegistry();
      static std::shared_ptr<ImageHandlerStateRegistry> instance();
      std::shared_ptr<ossim::ImageHandlerState> createState(const ossimKeywordlist& kwl, 
                                                            const ossimString& prefix = "")const;
      std::shared_ptr<ossim::ImageHandlerState> createState(const ossimString& typeName)const;
   };
};
#endif


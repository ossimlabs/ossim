//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
//*****************************************************************************
#ifndef ossimImageHandlerStateFactory_HEADER
#define ossimImageHandlerStateFactory_HEADER 1
#include <ossim/base/StateFactoryBase.h>
#include <ossim/support_data/ImageHandlerState.h>
#include <memory>

namespace ossim
{
   class OSSIM_DLL ImageHandlerStateFactory : public StateFactoryBase< std::shared_ptr<ossim::ImageHandlerState> >
   {
   public:
      ImageHandlerStateFactory();
      static std::shared_ptr<ossim::ImageHandlerStateFactory> instance();

      virtual std::shared_ptr<ossim::ImageHandlerState> createState(const ossimKeywordlist& kwl, 
                                                                    const ossimString& prefix="")const override;
      virtual std::shared_ptr<ossim::ImageHandlerState> createState(const ossimString& typeName)const override; 

   };
}
#endif

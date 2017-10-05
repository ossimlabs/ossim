#include <ossim/imaging/ImageHandlerStateFactory.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimException.h>
#include <ossim/support_data/TiffHandlerState.h>
#include <mutex>


ossim::ImageHandlerStateFactory::ImageHandlerStateFactory()
{

}

std::shared_ptr<ossim::ImageHandlerStateFactory> ossim::ImageHandlerStateFactory::instance()
{
   static std::shared_ptr<ImageHandlerStateFactory> 
                 imageHandlerFactory = std::make_shared< ossim::ImageHandlerStateFactory >();
   return imageHandlerFactory;                  
}

std::shared_ptr<ossim::ImageHandlerState> ossim::ImageHandlerStateFactory::createState(const ossimKeywordlist& kwl, 
                                                                    const ossimString& prefix)const
{
   std::shared_ptr<ossim::ImageHandlerState> result;
   ossimString typeValue = kwl.find(prefix, ossimKeywordNames::TYPE_KW);
   if(typeValue)
   {
      result = createState(typeValue);
      if(result)
      {
         try
         {
            result->load(kwl, prefix);
         }
         catch(ossimException& e)
         {
            result = nullptr;
         }
      }   
   }

   return result;  
}

std::shared_ptr<ossim::ImageHandlerState> ossim::ImageHandlerStateFactory::createState(const ossimString& typeName)const
{
   std::shared_ptr<ossim::ImageHandlerState> result;

   if(typeName == ossim::TiffHandlerState::getStaticTypeName())
   {
      result = std::make_shared<ossim::TiffHandlerState>(); 
   }

   return result;  
}

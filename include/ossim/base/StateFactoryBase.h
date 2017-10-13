#ifndef ossimStateFactoryBase_HEADER
#define ossimStateFactoryBase_HEADER 1
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimString.h>

namespace ossim 
{
   template< class T >
   class OSSIM_DLL StateFactoryBase
   {
   public:
      StateFactoryBase(){}
      virtual T createState(const ossimKeywordlist& kwl, 
                            const ossimString& prefix="")const=0;
      virtual T createState(const ossimString& typeName)const=0; 
   };
}

#endif
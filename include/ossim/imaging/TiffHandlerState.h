//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
//*****************************************************************************
#ifndef ossimTiffHandlerState_HEADER
#define ossimTiffHandlerState_HEADER 1
#include <ossim/imaging/ImageHandlerState.h>

namespace ossim
{
   class OSSIM_DLL TiffHandlerState : public ossim::ImageHandlerState
   {
   public:
      TiffHandlerState();
      virtual ~TiffHandlerState();
      virtual const ossimString& getTypeName()const override;
      static const ossimString& getStaticTypeName();

      virtual void load(const ossimKeywordlist& kwl,
                             const ossimString& prefix="") override;
      virtual void save(ossimKeywordlist& kwl,
                             const ossimString& prefix="")const override;
    private:
      static const ossimString m_typeName;

   };
}
#endif

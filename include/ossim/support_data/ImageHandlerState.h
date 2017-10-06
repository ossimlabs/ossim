//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
//*****************************************************************************
#ifndef ossimImageHandlerState_HEADER
#define ossimImageHandlerState_HEADER 1
#include <ossim/base/State.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimFilename.h>

namespace ossim
{
   class OSSIM_DLL ImageHandlerState : public ossim::State
   {
   public:
      ImageHandlerState();
      virtual ~ImageHandlerState();
      virtual const ossimString& getTypeName()const override;
      static const ossimString& getStaticTypeName();
      virtual void load(const ossimKeywordlist& kwl,
                        const ossimString& prefix="") override;
      virtual void save(ossimKeywordlist& kwl,
                        const ossimString& prefix="")const override;
      std::shared_ptr<ImageHandlerState> getOverviewState(){return m_overviewState;}
      std::shared_ptr<const ImageHandlerState> getOverviewState()const{return m_overviewState;}
      void setOverviewState(std::shared_ptr<ImageHandlerState> overviewState){m_overviewState=overviewState;}
      void setConnectionString(const ossimString& connectionString){m_connectionString = connectionString;}
      const ossimString& getConnectionString()const{return m_connectionString;}
      void setImageHandlerType(const ossimString& typeName){m_imageHandlerType = typeName;}
      const ossimString& getImageHandlerType()const{return m_imageHandlerType;}
      void setCurrentEntry(const ossimString& entry){m_currentEntry = entry;}
      const ossimString& getCurrentEntry()const{return m_currentEntry;}
      // const ossimKeywordlist& getMetaData()const{return m_omd;}
      // void setMetaData(const ossimKeywordlist& omd){m_omd=omd;}
      // bool hasMetaData()const{return m_omd.getSize()>0;}
    private:
      static const ossimString m_typeName;
      std::shared_ptr<ImageHandlerState> m_overviewState;
      ossimString                        m_connectionString;
      ossimString                        m_imageHandlerType;
      ossimString                        m_currentEntry;
//      ossimKeywordlist                   m_omd;
   };
};

#endif
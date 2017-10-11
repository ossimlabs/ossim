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
#include <ossim/support_data/ossimImageMetaData.h>

namespace ossim
{
   class OSSIM_DLL ImageHandlerState : public ossim::State
   {
   public:
      ImageHandlerState();
      virtual ~ImageHandlerState();
      virtual const ossimString& getTypeName()const override;
      static const ossimString& getStaticTypeName();
      virtual bool load(const ossimKeywordlist& kwl,
                        const ossimString& prefix="") override;
      virtual bool save(ossimKeywordlist& kwl,
                        const ossimString& prefix="")const override;
      std::shared_ptr<ImageHandlerState> getOverviewState(){return m_overviewState;}
      std::shared_ptr<const ImageHandlerState> getOverviewState()const{return m_overviewState;}
      void setOverviewState(std::shared_ptr<ImageHandlerState> overviewState){m_overviewState=overviewState;}
      void setConnectionString(const ossimString& connectionString){m_connectionString = connectionString;}
      const ossimString& getConnectionString()const{return m_connectionString;}
      void setImageHandlerType(const ossimString& typeName){m_imageHandlerType = typeName;}
      const ossimString& getImageHandlerType()const{return m_imageHandlerType;}
      void setCurrentEntry(ossim_uint32 entry){m_currentEntry = entry;}
      const ossim_uint32 getCurrentEntry()const{return m_currentEntry;}
      std::shared_ptr<const ossimImageMetaData> getMetaData()const{return m_omd;}
      std::shared_ptr<ossimImageMetaData> getMetaData(){return m_omd;}
      void setMetaData(std::shared_ptr<ossimImageMetaData> omd){m_omd=omd;}
      void setValidVertices(std::shared_ptr<ossimKeywordlist> kwl){m_validVertices=kwl;}
      std::shared_ptr<ossimKeywordlist> getValidVertices(){return m_validVertices;}
      std::shared_ptr<const ossimKeywordlist> getValidVertices()const{return m_validVertices;}
      bool hasMetaData()const;

      /**
      * Overridable and loads the defaults given the main entry. 
      */
      bool virtual loadDefaults(const ossimFilename& filename, 
                                ossim_uint32 entry=0);
    private:
      static const ossimString            m_typeName;
      std::shared_ptr<ImageHandlerState>  m_overviewState;
      std::shared_ptr<ossimImageMetaData> m_omd;
      std::shared_ptr<ossimKeywordlist>   m_validVertices;
      ossimString                         m_connectionString;
      ossimString                         m_imageHandlerType;
      ossim_uint32                        m_currentEntry;
   };
};

#endif
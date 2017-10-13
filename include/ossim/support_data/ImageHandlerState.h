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
#include <memory>

namespace ossim
{
  /**
  * This is a Image handler state object.  This handles caching
  * state of a ossimImageHandler.  The example code provided will 
  * work for any Image handler that supports state caching.  Note,  
  * if anything changes on the input image then the state will need 
  * to be recalculated by opening the image without the state and 
  * grabbing it's state again.
  *
  * Example code to get a state of an image source:
  *
  * @code
  * #include <ossim/base/ossimKeywordlist.h>
  * #include <ossim/imaging/ossimImageHandlerRegistry.h>
  * #include <ossim/imaging/ossimImageHandler.h>
  * #include <ossim/support_data/ImageHandlerState.h>
  *
  * ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(filename);
  *
  * if(h)
  * {
  *   std::shared_ptr<ossim::ImageHandlerState> state = h->getState();
  *   if(state)
  *   {
  *     ossimKeywordlist kwl;
  *     state->save(kwl);
  *     std::cout << kwl << "\n";
  *   }
  * }
  * @endCode
  *
  * Example code using a state
  *
  * @code
  * #include <ossim/base/ossimKeywordlist.h>
  * #include <ossim/imaging/ossimImageHandlerRegistry.h>
  * #include <ossim/imaging/ossimImageHandler.h>
  * #include <ossim/support_data/ImageHandlerState.h>
  *
  * ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(filename);
  *
  * if(h)
  * {
  *   std::shared_ptr<ossim::ImageHandlerState> state = h->getState();
  *   if(state)
  *   {
  *      h = ossimImageHandlerRegistry::instance()->open(state);
  *      if(h)
  *      {
  *         std::cout << "Successfully opened with a state\n";  
  *      } 
  *   }
  * }
  * @endCode
  */
   class OSSIM_DLL ImageHandlerState : public ossim::State
   {
   public:
      ImageHandlerState();
      virtual ~ImageHandlerState();
      virtual const ossimString& getTypeName()const override;
      static const ossimString& getStaticTypeName();
      
      /**
      * Loads the the state object from keywordlist.
      *
      * @param kwl keywordlist that olds the state of the object
      * @param prefix optional prefix value that is used as a prefix 
      *        for all keywords.
      */
      virtual bool load(const ossimKeywordlist& kwl,
                        const ossimString& prefix="") override;
      /**
      * Saves the state of the object to a keyword list.
      *
      * @param kwl keywordlist that the state will be saved to
      * @param prefix optional prefix value that is used as a prefix 
      *        for all keywords.
      */
      virtual bool save(ossimKeywordlist& kwl,
                        const ossimString& prefix="")const override;

      /**
      * @return if one exists it will have a valid overview state.
      */
      std::shared_ptr<ImageHandlerState> getOverviewState(){return m_overviewState;}

      /**
      * @return if one exists it will have a valid overview state.
      */
      std::shared_ptr<const ImageHandlerState> getOverviewState()const{return m_overviewState;}

      /**
      * Allows one to set the overview state object for this handler
      */
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
      
      /**
      * @return true if it has metadata
      */
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
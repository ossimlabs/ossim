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
    private:
      static const ossimString m_typeName;

    protected:
      ossimString   m_connectionString;
      ossimString   m_overviewFile;
      ossimFilename m_supplementaryDirectory;
      std::shared_ptr<ossim::ImageHandlerState> m_overviewState;
      std::vector<ossimIpt> m_validImageVertices;
      // ossimImageMetaData m_metaData;
      // ossimRefPtr<ossimImageGeometry> m_geometry;
      // ossimRefPtr<ossimNBandLutDataObject> m_lut;
      // std::vector<ossimDpt> m_decimationFactors;
      ossimString m_imageID;
      ossim_uint32 m_startingResLevel; // 0 being full or highest res.
      bool m_openOverviewFlag;

      /**
      * pixel-is-point or pixel-is-area
      */
      ossimPixelType m_pixelType; 
   };
};

#endif
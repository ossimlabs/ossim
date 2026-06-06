//*******************************************************************
// Copyright (C) 2001 ImageLinks Inc. 
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*******************************************************************
// $Id: ossimImageRenderer.h 23548 2015-09-28 21:01:36Z dburken $

#ifndef ossimImageRenderer_HEADER
#define ossimImageRenderer_HEADER
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/projection/ossimImageViewTransform.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimPolyArea2d.h>
#include <ossim/base/ossimViewInterface.h>
#include <ossim/base/ossimRationalNumber.h>
#include <memory>

class ossimImageData;
class ossimDiscreteConvolutionKernel;
class ossimFilterResampler;

class OSSIMDLLEXPORT ossimImageRenderer : public ossimImageSourceFilter,
                                          public ossimViewInterface
{
public:
   struct RenderingStats
   {
      RenderingStats()
      : m_requestedViewRect(),
        m_clippedViewRect(),
        m_requestedViewPixels(0),
        m_clippedViewPixels(0),
        m_nodes(0),
        m_vertices(0),
        m_reusedVertexReferences(0),
        m_visitedNodes(0),
        m_splitNodes(0),
        m_filledLeaves(0),
        m_inputTileCalls(0),
        m_inputTilePixels(0),
        m_maxInputTileWidth(0),
        m_maxInputTileHeight(0),
        m_minInputResLevel(0),
        m_maxInputResLevel(0),
        m_clipped(false)
      {
      }

      ossimIrect m_requestedViewRect;
      ossimIrect m_clippedViewRect;
      ossim_uint64 m_requestedViewPixels;
      ossim_uint64 m_clippedViewPixels;
      ossim_uint64 m_nodes;
      ossim_uint64 m_vertices;
      ossim_uint64 m_reusedVertexReferences;
      ossim_uint64 m_visitedNodes;
      ossim_uint64 m_splitNodes;
      ossim_uint64 m_filledLeaves;
      ossim_uint64 m_inputTileCalls;
      ossim_uint64 m_inputTilePixels;
      ossim_uint32 m_maxInputTileWidth;
      ossim_uint32 m_maxInputTileHeight;
      ossim_uint32 m_minInputResLevel;
      ossim_uint32 m_maxInputResLevel;
      bool m_clipped;
   };

   ossimImageRenderer();
   ossimImageRenderer(ossimImageSource* inputSource,
                      ossimImageViewTransform* imageViewTrans = NULL);

   virtual ~ossimImageRenderer();

   virtual ossimString getLongName()  const;
   virtual ossimString getShortName() const;

   /**
    * the resampler will need the tile request to come from the view.
    * It will use the view to transoform this to world and then use the
    * image's projection to get it into final line sample.  This way
    * the resampler can fill a tile width by height on the screen
    * correctly.
    */
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& origin,
                                               ossim_uint32 resLevel=0);

   virtual void initialize();

   /**
    * m_Resampler will adjust the rect to whatever the view is.  So it
    * will project the full image rect onto the view and return the upright
    * bounding rect.
    */
   virtual ossimIrect getBoundingRect(ossim_uint32 resLevel=0)const;

   /**
    * @brief Gets the bounding rectangle of the source.
    *
    * This is the output view bounds.
    * 
    * @param rect Initialized with bounding rectangle by this.
    * @param resLevel Reduced resolution level if applicable.
    */
   virtual void getBoundingRect(ossimIrect& rect,
                                ossim_uint32 resLevel=0) const;

   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);

   void setImageViewTransform(ossimImageViewTransform* transform);
   ossimImageViewTransform* getImageViewTransform() { return m_ImageViewTransform.get(); }

   //! Returns instance to the input image geometry. This may be a NULL pointer.
   //! This is only valid if the IVT is a projection type IVT (IVPT) 
   virtual ossimRefPtr<ossimImageGeometry> getImageGeometry();

   virtual bool setView(ossimObject* baseObject);
   ossimFilterResampler* getResampler() { return m_Resampler; }
   virtual ossimObject* getView();
   virtual const ossimObject* getView()const;

   virtual void getDecimationFactor(ossim_uint32 resLevel, ossimDpt& result)const;
  virtual void getDecimationFactors(std::vector<ossimDpt>& decimations)const;
  virtual ossim_uint32 getNumberOfDecimationLevels()const;
  
   virtual void setAutoUpdateInputTransformFlag(bool flag){ m_AutoUpdateInputTransform = flag; }
   
   /**
    * ordering specifies how the vertices should be arranged.
    * valid image vertices is basically the tightly fit convex hull
    * of the image.  Usually an image has NULL values and are
    * internally not upright rectangular.  This can cause
    * problems some spatial filters.
    *
    * We need to make sure that the resampler overrides this method.
    * it needs to transform the vertices to the view's side.
    */
   virtual void getValidImageVertices(std::vector<ossimIpt>& validVertices,
                                      ossimVertexOrdering ordering=OSSIM_CLOCKWISE_ORDER,
                                      ossim_uint32 resLevel=0)const;

   void setMaxLevelsToCompute(ossim_uint32 maxLevels);
   ossim_uint32 getMaxLevelsToCompute()const;
   
   void connectInputEvent(ossimConnectionEvent& event);
   void disconnectInputEvent(ossimConnectionEvent& event);
   void propertyEvent(ossimPropertyEvent& event);
   virtual void refreshEvent(ossimRefreshEvent& event);
   
   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;

   /**
    * @brief Enables source.
    *
    * Overrides ossimSource::enableSource so bounding rects are recomputed on
    * state change.
    */
   virtual void enableSource();

   /**
    * @brief Disables source.
    *
    * Overrides ossimSource::disableSource so bounding rects are recomputed on
    * state change.
    */
   virtual void disableSource();

   /**
    * @brief Sets the enable flag.
    *
    * @param flag True to enable, false to disable.
    * 
    * Overrides ossimSource::setEnableFlag so bounding rects are recomputed on
    * state change.
    */
   virtual void setEnableFlag(bool flag);

   const RenderingStats& getLastRenderingStats()const;
   
protected:
private:
   class ossimRendererVertexCache;
   
   class ossimRendererSubRectInfo
   {
    enum SplitFlag{
      SPLIT_NONE = 0,
      UPPER_LEFT_SPLIT_FLAG  = 1,
      UPPER_RIGHT_SPLIT_FLAG = 2,
      LOWER_RIGHT_SPLIT_FLAG = 4,
      LOWER_LEFT_SPLIT_FLAG  = 8,
      SPLIT_ALL = UPPER_LEFT_SPLIT_FLAG|UPPER_RIGHT_SPLIT_FLAG|LOWER_RIGHT_SPLIT_FLAG|LOWER_LEFT_SPLIT_FLAG
    };
   public:
      friend std::ostream& operator <<(std::ostream& out, const ossimRendererSubRectInfo& rhs)
      {
         return out << "vul:   " << rhs.vul() << std::endl
            << "vur:   " << rhs.vur() << std::endl
            << "vlr:   " << rhs.vlr() << std::endl
            << "vll:   " << rhs.vll() << std::endl
            << "iul:   " << rhs.iul() << std::endl
            << "iur:   " << rhs.iur() << std::endl
            << "ilr:   " << rhs.ilr() << std::endl
            << "ill:   " << rhs.ill() << std::endl
            << "scale: " << rhs.viewToImageScale() << std::endl;

      }

      ossimRendererSubRectInfo(ossimImageViewTransform* transform=0);
         
      bool imageHasNans()const;
      bool imageIsNan()const;
      bool viewHasNans()const;
      bool viewIsNan()const;

      void transformViewToImage();
      void setVertexCache(ossimRendererVertexCache* cache);
      void setVertexIndices(ossim_int64 ul,
                            ossim_int64 ur,
                            ossim_int64 lr,
                            ossim_int64 ll);
      bool hasVertexIndices()const;
      bool tooBig()const;
      void stretchImageOut(bool enableRound=false);
      ossimDrect getViewRect()const;
      ossimDrect getImageRect()const;
      void roundImageToInteger();
      bool isViewEqual(const ossimRendererSubRectInfo& infoRect)const;
      bool isViewEqual(const ossimDrect& viewRect)const;
      ossimDpt computeViewToImageScale(const ossimDpt& viewPt, const ossimDpt& delta=ossimDpt(1.0,1.0))const;
      ossimDpt getAbsValueViewToImageScales()const;
      ossimDpt getAbsValueImageToViewScales()const;
      ossimDpt iul()const;
      ossimDpt iur()const;
      ossimDpt ilr()const;
      ossimDpt ill()const;
      ossimIpt vul()const;
      ossimIpt vur()const;
      ossimIpt vlr()const;
      ossimIpt vll()const;
      ossimDpt ulScale()const;
      ossimDpt urScale()const;
      ossimDpt lrScale()const;
      ossimDpt llScale()const;
      ossimDpt viewToImageScale()const;
      ossimDpt imageToViewScale()const;
      ossimDpt computeRoundTripErrorViewPt(const ossimDpt& dpt)const;
      bool isViewAPoint()const;
      bool isIdentity()const;
      //bool canBilinearInterpolate(double error) const;
      bool canBilinearInterpolate() const;

      ossimDpt getParametricCenter(const ossimDpt& ul, const ossimDpt& ur, 
				    const ossimDpt& lr, const ossimDpt& ll)const;

     void getViewMids(ossimDpt& upperMid,
		      ossimDpt& rightMid,
		      ossimDpt& bottomMid,
		      ossimDpt& leftMid,
		      ossimDpt& center)const;

     void getImageMids(ossimDpt& upperMid,
		       ossimDpt& rightMid,
		       ossimDpt& bottomMid,
		       ossimDpt& leftMid,
		       ossimDpt& center)const;
     ossim_uint16 getSplitFlags()const;

      mutable ossimRefPtr<ossimImageViewTransform> m_transform;
      mutable std::shared_ptr<const ossimPolyArea2d> m_viewBounds;
      mutable ossimRendererVertexCache* m_vertexCache;
      mutable ossim_int64 m_ulVertex;
      mutable ossim_int64 m_urVertex;
      mutable ossim_int64 m_lrVertex;
      mutable ossim_int64 m_llVertex;

   };

   void recursiveResample(ossimRefPtr<ossimImageData> outputData,
                          const ossimRendererSubRectInfo& rectInfo,
			  ossim_uint32 level);
   

   void fillTile(ossimRefPtr<ossimImageData> outputData,
                 const ossimRendererSubRectInfo& rectInfo);
                 
   ossimIrect getBoundingImageRect()const;

   
   //! this is called on a property event and on input connection changes.
   void checkIVT();

   /**
    * @brief Initializes m_inputR0Rect and m_viewBoundingRect and sets
    * m_rectsDirty appropriately.
    */
   void initializeBoundingRects();

   ossimRefPtr<ossimImageData> getTileAtResLevel(const ossimIrect& boundingRect,
                                     ossim_uint32 resLevel);
  template <class T>
  void resampleTileToDecimation(T dummyVariable,
				ossimRefPtr<ossimImageData> result,
				ossimRefPtr<ossimImageData> tile,
				ossim_uint32 multiplier);

   long computeClosestResLevel(const std::vector<ossimDpt>& decimationFactors,
                               double scale)const;
   void stretchQuadOut(const ossimDpt& amount,
                       ossimDpt& ul,
                       ossimDpt& ur,
                       ossimDpt& lr,
                       ossimDpt& ll);

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();

   /**
    *  Deletes all allocated tiles.
    */
   void deallocate();

   ossimFilterResampler*       m_Resampler;
   ossimRefPtr<ossimImageData> m_BlankTile;
   ossimRefPtr<ossimImageData> m_Tile;
   ossimRefPtr<ossimImageData> m_TemporaryBuffer;

   /**
    * This is going to allow us to chain multiple
    * renderers together.  So if we have one
    * renderer doing a scale and they pass an r-level
    * down and we have another renderer within the
    * chain we will be starting at a different r-level.
    * The default will be r-level 0 request coming
    * from the right.
    */
   ossim_uint32             m_StartingResLevel;
   ossimRefPtr<ossimImageViewTransform> m_ImageViewTransform;

   ossimIrect               m_inputR0Rect;
   ossimIrect               m_viewRect;
   bool                     m_rectsDirty;

   ossim_uint32             m_MaxRecursionLevel;
   bool                     m_AutoUpdateInputTransform;
   ossim_uint32             m_MaxLevelsToCompute;

   ossimPolyArea2d          m_viewArea;
   bool                     m_crossesDateline;

   double                   m_averageViewToImageScale;
   double                   m_averageViewToImageRLevelScale;
   RenderingStats           m_lastRenderingStats;
   RenderingStats           m_currentRenderingStats;
   static double            m_interpErrorThreshold;

   TYPE_DATA
};

inline ossimImageRenderer::ossimRendererSubRectInfo::ossimRendererSubRectInfo(ossimImageViewTransform* transform)
:m_transform(transform),
m_viewBounds(),
m_vertexCache(0),
m_ulVertex(-1),
m_urVertex(-1),
m_lrVertex(-1),
m_llVertex(-1)
{
}

inline bool ossimImageRenderer::ossimRendererSubRectInfo::imageHasNans()const
{
   return ( iul().hasNans()||
      iur().hasNans()||
      ilr().hasNans()||
      ill().hasNans());
}

inline void ossimImageRenderer::ossimRendererSubRectInfo::setVertexCache(
   ossimRendererVertexCache* cache)
{
   m_vertexCache = cache;
}

inline void ossimImageRenderer::ossimRendererSubRectInfo::setVertexIndices(
   ossim_int64 ul,
   ossim_int64 ur,
   ossim_int64 lr,
   ossim_int64 ll)
{
   m_ulVertex = ul;
   m_urVertex = ur;
   m_lrVertex = lr;
   m_llVertex = ll;
}

inline bool ossimImageRenderer::ossimRendererSubRectInfo::hasVertexIndices()const
{
   return ((m_ulVertex >= 0)&&
           (m_urVertex >= 0)&&
           (m_lrVertex >= 0)&&
           (m_llVertex >= 0));
}

inline bool ossimImageRenderer::ossimRendererSubRectInfo::imageIsNan()const
{
   return ( iul().hasNans()&&
      iur().hasNans()&&
      ilr().hasNans()&&
      ill().hasNans());
}

inline bool ossimImageRenderer::ossimRendererSubRectInfo::viewHasNans()const
{
   return ( vul().hasNans()||
      vur().hasNans()||
      vlr().hasNans()||
      vll().hasNans());
}

inline bool ossimImageRenderer::ossimRendererSubRectInfo::viewIsNan()const
{
   return ( vul().hasNans()&&
      vur().hasNans()&&
      vlr().hasNans()&&
      vll().hasNans());
}

inline ossimDrect ossimImageRenderer::ossimRendererSubRectInfo::getViewRect()const
{
   return ossimDrect(vul(),
      vur(),
      vlr(),
      vll());
}

inline ossimDrect ossimImageRenderer::ossimRendererSubRectInfo::getImageRect()const
{
   return ossimDrect(iul(),
      iur(),
      ilr(),
      ill());
}

inline void ossimImageRenderer::ossimRendererSubRectInfo::roundImageToInteger()
{
}

inline bool ossimImageRenderer::ossimRendererSubRectInfo::isViewEqual(const ossimRendererSubRectInfo& infoRect)const
{
   return ( (vul() == infoRect.vul())&&
      (vur() == infoRect.vur())&&
      (vlr() == infoRect.vlr())&&
      (vll() == infoRect.vll()));
}

inline bool ossimImageRenderer::ossimRendererSubRectInfo::isViewEqual(const ossimDrect& viewRect)const
{
   return ( (vul() == viewRect.ul())&&
      (vur() == viewRect.ur())&&
      (vlr() == viewRect.lr())&&
      (vll() == viewRect.ll()));
}

inline ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::getAbsValueViewToImageScales()const
{
   const ossimDpt scale = viewToImageScale();
   if(scale.hasNans())
   {
      return scale;
   }
   return ossimDpt(fabs(scale.x), fabs(scale.y));
}

inline ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::getAbsValueImageToViewScales()const
{
   const ossimDpt scale = imageToViewScale();
   if(scale.hasNans())
   {
      return scale;
   }

   return ossimDpt(fabs(scale.x), fabs(scale.y));
}

inline bool ossimImageRenderer::ossimRendererSubRectInfo::isViewAPoint()const
{
   return ((vul() == vur())&&
      (vul() == vlr())&&
      (vul() == vll()));
}

#endif

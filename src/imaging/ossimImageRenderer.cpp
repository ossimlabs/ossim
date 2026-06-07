//*******************************************************************
//
// License: MIT
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: ossimImageRenderer.cpp 23663 2015-12-11 21:10:54Z dburken $

#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimDpt3d.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimPolyArea2d.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossim2dBilinearTransform.h>
#include <ossim/base/ossimProcessProgressEvent.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimConnectableContainerInterface.h>
#include <ossim/base/ossimViewController.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimNumericProperty.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimDiscrete3x3HatFilter.h>
#include <ossim/imaging/ossimDiscreteNearestNeighbor.h>
#include <ossim/imaging/ossimFilterResampler.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimImageViewTransformFactory.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimBilinearMapProjection.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>
#include <ossim/base/ossimPreferences.h>

// using namespace std;

using namespace std;

#ifdef OSSIM_ID_ENABLED
static const char OSSIM_ID[] = "$Id: ossimImageRenderer.cpp 23663 2015-12-11 21:10:54Z dburken $";
#endif

static ossimTrace traceDebug("ossimImageRenderer:debug");

RTTI_DEF2(ossimImageRenderer, "ossimImageRenderer", ossimImageSourceFilter, ossimViewInterface);

double ossimImageRenderer::m_interpErrorThreshold = 1.0;

namespace
{
ossim_uint64 rendererRectPixels(const ossimIrect& rect)
{
   const ossim_int64 w = rect.width();
   const ossim_int64 h = rect.height();
   if(w < 1 || h < 1)
   {
      return 0;
   }
   return static_cast<ossim_uint64>(w) * static_cast<ossim_uint64>(h);
}

double rendererElapsedSeconds(
   const std::chrono::steady_clock::time_point& start)
{
   return std::chrono::duration<double>(
      std::chrono::steady_clock::now() - start).count();
}
}

class ossimImageRenderer::ossimRendererVertexCache
{
public:
   enum Edge
   {
      LEFT_EDGE   = 0,
      TOP_EDGE    = 1,
      RIGHT_EDGE  = 2,
      BOTTOM_EDGE = 3
   };

   struct Vertex
   {
      ossimIpt m_viewPt;
      ossimDpt m_imagePt;
      ossimDpt m_scale;
   };

   struct Node
   {
      Node()
         : m_parent(-1)
      {
         m_child[0] = -1;
         m_child[1] = -1;
         m_child[2] = -1;
         m_child[3] = -1;
         m_neighbor[0] = -1;
         m_neighbor[1] = -1;
         m_neighbor[2] = -1;
         m_neighbor[3] = -1;
      }

      ossimRendererSubRectInfo m_rect;
      ossim_int64 m_parent;
      ossim_int64 m_child[4];
      ossim_int64 m_neighbor[4];
   };

   ossimRendererVertexCache(ossimImageViewTransform* transform)
      : m_transform(transform)
      , m_reusedVertexReferences(0)
      , m_collectStats(std::getenv("OSSIM_RENDERER_VERTEX_CACHE_STATS") != 0)
   {
   }

   ossim_int64 addVertex(const ossimIpt& viewPt)
   {
      Vertex vertex;
      vertex.m_viewPt = viewPt;
      if(m_transform.valid())
      {
         m_transform->viewToImage(viewPt, vertex.m_imagePt);
         m_transform->getViewToImageScale(vertex.m_scale, viewPt);
      }
      else
      {
         vertex.m_imagePt.makeNan();
         vertex.m_scale.makeNan();
      }

      m_vertices.push_back(vertex);
      return static_cast<ossim_int64>(m_vertices.size() - 1);
   }

   ossim_int64 addVertex(const ossimIpt& viewPt,
                         ossim_int64 nodeIndex,
                         const ossim_int64* candidates,
                         ossim_uint32 candidateCount)
   {
      for(ossim_uint32 idx = 0; idx < candidateCount; ++idx)
      {
         if(vertexMatches(candidates[idx], viewPt))
         {
            referenceVertex(candidates[idx]);
            return candidates[idx];
         }
      }

      ossim_int64 neighborVertex = -1;
      if(findNeighborVertex(nodeIndex, viewPt, neighborVertex))
      {
         referenceVertex(neighborVertex);
         return neighborVertex;
      }

      return addVertex(viewPt);
   }

   ossim_int64 addVertex(const ossimIpt& viewPt,
                         const ossim_int64* candidates,
                         ossim_uint32 candidateCount)
   {
      return addVertex(viewPt, -1, candidates, candidateCount);
   }

   ossim_int64 addNode(const ossimRendererSubRectInfo& rect,
                       ossim_int64 parent=-1)
   {
      Node node;
      node.m_rect = rect;
      node.m_parent = parent;
      node.m_rect.setVertexCache(this);
      m_nodes.push_back(node);
      return static_cast<ossim_int64>(m_nodes.size() - 1);
   }

   const Node& getNode(ossim_int64 index) const
   {
      return m_nodes[static_cast<std::size_t>(index)];
   }

   bool splitNode(ossim_int64 nodeIndex,
                  std::vector<ossim_int64>& result)
   {
      if(nodeIndex < 0 ||
         static_cast<std::size_t>(nodeIndex) >= m_nodes.size())
      {
         return false;
      }

      Node& node = m_nodes[static_cast<std::size_t>(nodeIndex)];
      const ossimRendererSubRectInfo& rect = node.m_rect;
      const ossim_uint16 splitFlags = rect.getSplitFlags();
      if(!splitFlags)
      {
         return false;
      }

      ossimIrect vrect(rect.vul(),
                       rect.vur(),
                       rect.vlr(),
                       rect.vll());
      const ossim_int32 w  = vrect.width();
      const ossim_int32 h  = vrect.height();
      const ossim_int32 w2 = w>>1;
      const ossim_int32 h2 = h>>1;

      if((w2 < 2)&&(h2 < 2))
      {
         ossimRendererSubRectInfo child(rect.m_transform.get());
         addChild(nodeIndex, 0, child,
                  rect.m_ulVertex,
                  rect.m_ulVertex,
                  rect.m_ulVertex,
                  rect.m_ulVertex,
                  result);
      }
      else if((splitFlags == (1|8)) ||
              (splitFlags == (2|4)))
      {
         if(w > 1)
         {
            splitHorizontal(nodeIndex, result);
         }
      }
      else if((splitFlags == (1|2)) ||
              (splitFlags == (4|8)))
      {
         if(h > 1)
         {
            splitVertical(nodeIndex, result);
         }
      }
      else
      {
         if((w < 2)&&(h > 1))
         {
            splitVertical(nodeIndex, result);
         }
         else if((w > 1)&&(h < 2))
         {
            splitHorizontal(nodeIndex, result);
         }
         else
         {
            splitAll(nodeIndex, result);
         }
      }

      return !result.empty();
   }

   void referenceVertex(ossim_int64 index)
   {
      if(index >= 0)
      {
         ++m_reusedVertexReferences;
      }
   }

   bool getVertex(ossim_int64 index,
                  ossimIpt& viewPt,
                  ossimDpt& imagePt,
                  ossimDpt& scale) const
   {
      if(index < 0 ||
         static_cast<std::size_t>(index) >= m_vertices.size())
      {
         viewPt.makeNan();
         imagePt.makeNan();
         scale.makeNan();
         return false;
      }

      const Vertex& vertex = m_vertices[static_cast<std::size_t>(index)];
      viewPt = vertex.m_viewPt;
      imagePt = vertex.m_imagePt;
      scale = vertex.m_scale;
      return true;
   }

   void printStats(const ossimIrect& tileRect) const
   {
      if(!m_collectStats)
      {
         return;
      }

      ossimNotify(ossimNotifyLevel_INFO)
         << "ossimImageRenderer vertex cache: rect=" << tileRect
         << " vertices=" << m_vertices.size()
         << " reused_vertex_refs=" << m_reusedVertexReferences
         << " nodes=" << m_nodes.size()
         << std::endl;
   }

   void updateRenderingStats(RenderingStats& stats) const
   {
      stats.m_nodes = static_cast<ossim_uint64>(m_nodes.size());
      stats.m_vertices = static_cast<ossim_uint64>(m_vertices.size());
      stats.m_reusedVertexReferences = m_reusedVertexReferences;
   }

private:
   ossim_int64 addChild(ossim_int64 parentIndex,
                        ossim_uint32 childSlot,
                        ossimRendererSubRectInfo& rect,
                        ossim_int64 ul,
                        ossim_int64 ur,
                        ossim_int64 lr,
                        ossim_int64 ll,
                        std::vector<ossim_int64>& result)
   {
      Node& parent = m_nodes[static_cast<std::size_t>(parentIndex)];
      rect.m_viewBounds = parent.m_rect.m_viewBounds;
      rect.setVertexCache(this);
      rect.setVertexIndices(ul, ur, lr, ll);
      rect.transformViewToImage();

      if(rect.imageIsNan() &&
         rect.m_viewBounds &&
         !(rect.m_viewBounds->intersects(rect.getViewRect())))
      {
         return -1;
      }

      const ossim_int64 childIndex = addNode(rect, parentIndex);
      if(childSlot < 4)
      {
         m_nodes[static_cast<std::size_t>(parentIndex)].m_child[childSlot] =
            childIndex;
      }
      result.push_back(childIndex);
      return childIndex;
   }

   void splitHorizontal(ossim_int64 nodeIndex,
                        std::vector<ossim_int64>& result)
   {
      const ossimRendererSubRectInfo rect =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_rect;
      ossimIrect vrect(rect.vul(),
                       rect.vur(),
                       rect.vlr(),
                       rect.vll());
      const ossim_int32 w2 = vrect.width()>>1;
      ossimIrect leftRect(rect.vul().x,
                          rect.vul().y,
                          rect.vul().x+w2-1,
                          rect.vlr().y);
      ossimIrect rightRect(leftRect.ur().x+1,
                           rect.vul().y,
                           rect.vur().x,
                           rect.vlr().y);

      ossimRendererSubRectInfo left(rect.m_transform.get());
      ossimRendererSubRectInfo right(rect.m_transform.get());
      ossim_int64 candidates[8];
      ossim_uint32 candidateCount = 0;
      candidates[candidateCount++] = rect.m_ulVertex;
      candidates[candidateCount++] = rect.m_urVertex;
      candidates[candidateCount++] = rect.m_lrVertex;
      candidates[candidateCount++] = rect.m_llVertex;

      const ossim_int64 leftUr =
         addVertex(leftRect.ur(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = leftUr;
      const ossim_int64 leftLr =
         addVertex(leftRect.lr(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = leftLr;
      const ossim_int64 rightUl =
         addVertex(rightRect.ul(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = rightUl;
      const ossim_int64 rightLl =
         addVertex(rightRect.ll(), nodeIndex, candidates, candidateCount);

      const ossim_int64 parentLeft =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[LEFT_EDGE];
      const ossim_int64 parentTop =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[TOP_EDGE];
      const ossim_int64 parentRight =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[RIGHT_EDGE];
      const ossim_int64 parentBottom =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[BOTTOM_EDGE];

      const ossim_int64 leftIndex =
         addChild(nodeIndex, 0, left,
                  rect.m_ulVertex, leftUr, leftLr, rect.m_llVertex, result);
      const ossim_int64 rightIndex =
         addChild(nodeIndex, 1, right,
                  rightUl, rect.m_urVertex, rect.m_lrVertex, rightLl, result);

      setNeighbor(leftIndex, LEFT_EDGE, parentLeft);
      setNeighbor(leftIndex, TOP_EDGE, parentTop);
      setNeighbor(leftIndex, RIGHT_EDGE, rightIndex);
      setNeighbor(leftIndex, BOTTOM_EDGE, parentBottom);
      setNeighbor(rightIndex, LEFT_EDGE, leftIndex);
      setNeighbor(rightIndex, TOP_EDGE, parentTop);
      setNeighbor(rightIndex, RIGHT_EDGE, parentRight);
      setNeighbor(rightIndex, BOTTOM_EDGE, parentBottom);
   }

   void splitVertical(ossim_int64 nodeIndex,
                      std::vector<ossim_int64>& result)
   {
      const ossimRendererSubRectInfo rect =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_rect;
      ossimIrect vrect(rect.vul(),
                       rect.vur(),
                       rect.vlr(),
                       rect.vll());
      const ossim_int32 h2 = vrect.height()>>1;
      ossimIrect topRect(rect.vul().x,
                         rect.vul().y,
                         rect.vlr().x,
                         rect.vul().y+h2-1);
      ossimIrect bottomRect(rect.vul().x,
                            topRect.lr().y+1,
                            rect.vlr().x,
                            rect.vlr().y);

      ossimRendererSubRectInfo top(rect.m_transform.get());
      ossimRendererSubRectInfo bottom(rect.m_transform.get());
      ossim_int64 candidates[8];
      ossim_uint32 candidateCount = 0;
      candidates[candidateCount++] = rect.m_ulVertex;
      candidates[candidateCount++] = rect.m_urVertex;
      candidates[candidateCount++] = rect.m_lrVertex;
      candidates[candidateCount++] = rect.m_llVertex;

      const ossim_int64 topLr =
         addVertex(topRect.lr(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = topLr;
      const ossim_int64 topLl =
         addVertex(topRect.ll(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = topLl;
      const ossim_int64 bottomUl =
         addVertex(bottomRect.ul(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = bottomUl;
      const ossim_int64 bottomUr =
         addVertex(bottomRect.ur(), nodeIndex, candidates, candidateCount);

      const ossim_int64 parentLeft =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[LEFT_EDGE];
      const ossim_int64 parentTop =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[TOP_EDGE];
      const ossim_int64 parentRight =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[RIGHT_EDGE];
      const ossim_int64 parentBottom =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[BOTTOM_EDGE];

      const ossim_int64 topIndex =
         addChild(nodeIndex, 0, top,
                  rect.m_ulVertex, rect.m_urVertex, topLr, topLl, result);
      const ossim_int64 bottomIndex =
         addChild(nodeIndex, 2, bottom,
                  bottomUl, bottomUr, rect.m_lrVertex, rect.m_llVertex, result);

      setNeighbor(topIndex, LEFT_EDGE, parentLeft);
      setNeighbor(topIndex, TOP_EDGE, parentTop);
      setNeighbor(topIndex, RIGHT_EDGE, parentRight);
      setNeighbor(topIndex, BOTTOM_EDGE, bottomIndex);
      setNeighbor(bottomIndex, LEFT_EDGE, parentLeft);
      setNeighbor(bottomIndex, TOP_EDGE, topIndex);
      setNeighbor(bottomIndex, RIGHT_EDGE, parentRight);
      setNeighbor(bottomIndex, BOTTOM_EDGE, parentBottom);
   }

   void splitAll(ossim_int64 nodeIndex,
                 std::vector<ossim_int64>& result)
   {
      const ossimRendererSubRectInfo rect =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_rect;
      ossimIrect vrect(rect.vul(),
                       rect.vur(),
                       rect.vlr(),
                       rect.vll());
      const ossim_int32 w2 = vrect.width()>>1;
      const ossim_int32 h2 = vrect.height()>>1;

      const ossimIrect ulRect(rect.vul().x,
                              rect.vul().y,
                              rect.vul().x + (w2 - 1),
                              rect.vul().y + (h2 - 1));
      const ossimIrect urRect(ulRect.ur().x+1,
                              rect.vul().y,
                              rect.vur().x,
                              rect.vul().y + (h2 - 1));
      const ossimIrect lrRect(ulRect.lr().x,
                              ulRect.lr().y+1,
                              rect.vlr().x,
                              rect.vlr().y);
      const ossimIrect llRect(rect.vul().x,
                              ulRect.ll().y+1,
                              lrRect.ul().x,
                              lrRect.ll().y);

      ossimRendererSubRectInfo ul(rect.m_transform.get());
      ossimRendererSubRectInfo ur(rect.m_transform.get());
      ossimRendererSubRectInfo lr(rect.m_transform.get());
      ossimRendererSubRectInfo ll(rect.m_transform.get());

      ossim_int64 candidates[15];
      ossim_uint32 candidateCount = 0;
      candidates[candidateCount++] = rect.m_ulVertex;
      candidates[candidateCount++] = rect.m_urVertex;
      candidates[candidateCount++] = rect.m_lrVertex;
      candidates[candidateCount++] = rect.m_llVertex;

      const ossim_int64 ulUr =
         addVertex(ulRect.ur(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = ulUr;
      const ossim_int64 ulLr =
         addVertex(ulRect.lr(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = ulLr;
      const ossim_int64 ulLl =
         addVertex(ulRect.ll(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = ulLl;
      const ossim_int64 urUl =
         addVertex(urRect.ul(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = urUl;
      const ossim_int64 urLr =
         addVertex(urRect.lr(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = urLr;
      const ossim_int64 urLl =
         addVertex(urRect.ll(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = urLl;
      const ossim_int64 lrUl =
         addVertex(lrRect.ul(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = lrUl;
      const ossim_int64 lrUr =
         addVertex(lrRect.ur(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = lrUr;
      const ossim_int64 lrLl =
         addVertex(lrRect.ll(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = lrLl;
      const ossim_int64 llUl =
         addVertex(llRect.ul(), nodeIndex, candidates, candidateCount);
      candidates[candidateCount++] = llUl;
      const ossim_int64 llUr =
         addVertex(llRect.ur(), nodeIndex, candidates, candidateCount);

      const ossim_int64 parentLeft =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[LEFT_EDGE];
      const ossim_int64 parentTop =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[TOP_EDGE];
      const ossim_int64 parentRight =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[RIGHT_EDGE];
      const ossim_int64 parentBottom =
         m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[BOTTOM_EDGE];

      const ossim_int64 ulIndex =
         addChild(nodeIndex, 0, ul,
                  rect.m_ulVertex, ulUr, ulLr, ulLl, result);
      const ossim_int64 urIndex =
         addChild(nodeIndex, 1, ur,
                  urUl, rect.m_urVertex, urLr, urLl, result);
      const ossim_int64 lrIndex =
         addChild(nodeIndex, 2, lr,
                  lrUl, lrUr, rect.m_lrVertex, lrLl, result);
      const ossim_int64 llIndex =
         addChild(nodeIndex, 3, ll,
                  llUl, llUr, lrLl, rect.m_llVertex, result);

      setNeighbor(ulIndex, LEFT_EDGE, parentLeft);
      setNeighbor(ulIndex, TOP_EDGE, parentTop);
      setNeighbor(ulIndex, RIGHT_EDGE, urIndex);
      setNeighbor(ulIndex, BOTTOM_EDGE, llIndex);
      setNeighbor(urIndex, LEFT_EDGE, ulIndex);
      setNeighbor(urIndex, TOP_EDGE, parentTop);
      setNeighbor(urIndex, RIGHT_EDGE, parentRight);
      setNeighbor(urIndex, BOTTOM_EDGE, lrIndex);
      setNeighbor(lrIndex, LEFT_EDGE, llIndex);
      setNeighbor(lrIndex, TOP_EDGE, urIndex);
      setNeighbor(lrIndex, RIGHT_EDGE, parentRight);
      setNeighbor(lrIndex, BOTTOM_EDGE, parentBottom);
      setNeighbor(llIndex, LEFT_EDGE, parentLeft);
      setNeighbor(llIndex, TOP_EDGE, ulIndex);
      setNeighbor(llIndex, RIGHT_EDGE, lrIndex);
      setNeighbor(llIndex, BOTTOM_EDGE, parentBottom);
   }

   bool vertexMatches(ossim_int64 index, const ossimIpt& viewPt) const
   {
      if(index < 0 ||
         static_cast<std::size_t>(index) >= m_vertices.size())
      {
         return false;
      }

      return (m_vertices[static_cast<std::size_t>(index)].m_viewPt == viewPt);
   }

   bool findNeighborVertex(ossim_int64 nodeIndex,
                           const ossimIpt& viewPt,
                           ossim_int64& result) const
   {
      if(nodeIndex < 0 ||
         static_cast<std::size_t>(nodeIndex) >= m_nodes.size())
      {
         return false;
      }

      const Node& node = m_nodes[static_cast<std::size_t>(nodeIndex)];
      const ossim_int32 minX = ossim::min(node.m_rect.vul().x,
                                          node.m_rect.vll().x);
      const ossim_int32 maxX = ossim::max(node.m_rect.vur().x,
                                          node.m_rect.vlr().x);
      const ossim_int32 minY = ossim::min(node.m_rect.vul().y,
                                          node.m_rect.vur().y);
      const ossim_int32 maxY = ossim::max(node.m_rect.vll().y,
                                          node.m_rect.vlr().y);

      if(viewPt.x == minX &&
         findVertexOnEdge(node.m_neighbor[LEFT_EDGE],
                          RIGHT_EDGE,
                          viewPt,
                          result))
      {
         return true;
      }
      if(viewPt.x == maxX &&
         findVertexOnEdge(node.m_neighbor[RIGHT_EDGE],
                          LEFT_EDGE,
                          viewPt,
                          result))
      {
         return true;
      }
      if(viewPt.y == minY &&
         findVertexOnEdge(node.m_neighbor[TOP_EDGE],
                          BOTTOM_EDGE,
                          viewPt,
                          result))
      {
         return true;
      }
      if(viewPt.y == maxY &&
         findVertexOnEdge(node.m_neighbor[BOTTOM_EDGE],
                          TOP_EDGE,
                          viewPt,
                          result))
      {
         return true;
      }

      return false;
   }

   bool findVertexOnEdge(ossim_int64 nodeIndex,
                         Edge edge,
                         const ossimIpt& viewPt,
                         ossim_int64& result) const
   {
      if(nodeIndex < 0 ||
         static_cast<std::size_t>(nodeIndex) >= m_nodes.size())
      {
         return false;
      }

      const Node& node = m_nodes[static_cast<std::size_t>(nodeIndex)];
      if(!rectContains(node.m_rect, viewPt))
      {
         return false;
      }

      if(vertexMatches(node.m_rect.m_ulVertex, viewPt))
      {
         result = node.m_rect.m_ulVertex;
         return true;
      }
      if(vertexMatches(node.m_rect.m_urVertex, viewPt))
      {
         result = node.m_rect.m_urVertex;
         return true;
      }
      if(vertexMatches(node.m_rect.m_lrVertex, viewPt))
      {
         result = node.m_rect.m_lrVertex;
         return true;
      }
      if(vertexMatches(node.m_rect.m_llVertex, viewPt))
      {
         result = node.m_rect.m_llVertex;
         return true;
      }

      for(ossim_uint32 idx = 0; idx < 4; ++idx)
      {
         const ossim_int64 childIndex = node.m_child[idx];
         if(childIndex >= 0 &&
            childTouchesEdge(childIndex, edge) &&
            findVertexOnEdge(childIndex, edge, viewPt, result))
         {
            return true;
         }
      }

      return false;
   }

   bool childTouchesEdge(ossim_int64 childIndex, Edge edge) const
   {
      if(childIndex < 0 ||
         static_cast<std::size_t>(childIndex) >= m_nodes.size())
      {
         return false;
      }

      const Node& child = m_nodes[static_cast<std::size_t>(childIndex)];
      const Node& parent = m_nodes[static_cast<std::size_t>(child.m_parent)];
      switch(edge)
      {
         case LEFT_EDGE:
            return child.m_rect.vul().x == parent.m_rect.vul().x;
         case TOP_EDGE:
            return child.m_rect.vul().y == parent.m_rect.vul().y;
         case RIGHT_EDGE:
            return child.m_rect.vur().x == parent.m_rect.vur().x;
         case BOTTOM_EDGE:
            return child.m_rect.vll().y == parent.m_rect.vll().y;
      }

      return false;
   }

   static bool rectContains(const ossimRendererSubRectInfo& rect,
                            const ossimIpt& viewPt)
   {
      const ossim_int32 minX = ossim::min(rect.vul().x, rect.vll().x);
      const ossim_int32 maxX = ossim::max(rect.vur().x, rect.vlr().x);
      const ossim_int32 minY = ossim::min(rect.vul().y, rect.vur().y);
      const ossim_int32 maxY = ossim::max(rect.vll().y, rect.vlr().y);

      return ((viewPt.x >= minX) &&
              (viewPt.x <= maxX) &&
              (viewPt.y >= minY) &&
              (viewPt.y <= maxY));
   }

   void setNeighbor(ossim_int64 nodeIndex,
                    Edge edge,
                    ossim_int64 neighborIndex)
   {
      if(nodeIndex < 0 ||
         static_cast<std::size_t>(nodeIndex) >= m_nodes.size())
      {
         return;
      }

      m_nodes[static_cast<std::size_t>(nodeIndex)].m_neighbor[edge] =
         neighborIndex;
   }

   ossimRefPtr<ossimImageViewTransform> m_transform;
   std::vector<Vertex> m_vertices;
   std::vector<Node> m_nodes;
   ossim_uint64 m_reusedVertexReferences;
   bool m_collectStats;
};

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::iul()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_ulVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return imagePt;
   }

   imagePt.makeNan();
   return imagePt;
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::iur()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_urVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return imagePt;
   }

   imagePt.makeNan();
   return imagePt;
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::ilr()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_lrVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return imagePt;
   }

   imagePt.makeNan();
   return imagePt;
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::ill()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_llVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return imagePt;
   }

   imagePt.makeNan();
   return imagePt;
}

ossimIpt ossimImageRenderer::ossimRendererSubRectInfo::vul()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_ulVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return viewPt;
   }

   viewPt.makeNan();
   return viewPt;
}

ossimIpt ossimImageRenderer::ossimRendererSubRectInfo::vur()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_urVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return viewPt;
   }

   viewPt.makeNan();
   return viewPt;
}

ossimIpt ossimImageRenderer::ossimRendererSubRectInfo::vlr()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_lrVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return viewPt;
   }

   viewPt.makeNan();
   return viewPt;
}

ossimIpt ossimImageRenderer::ossimRendererSubRectInfo::vll()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_llVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return viewPt;
   }

   viewPt.makeNan();
   return viewPt;
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::ulScale()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_ulVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return scale;
   }

   scale.makeNan();
   return scale;
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::urScale()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_urVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return scale;
   }

   scale.makeNan();
   return scale;
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::lrScale()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_lrVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return scale;
   }

   scale.makeNan();
   return scale;
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::llScale()const
{
   ossimIpt viewPt;
   ossimDpt imagePt;
   ossimDpt scale;
   if(m_vertexCache&&m_vertexCache->getVertex(m_llVertex,
                                               viewPt,
                                               imagePt,
                                               scale))
   {
      return scale;
   }

   scale.makeNan();
   return scale;
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::viewToImageScale()const
{
   ossimDpt result(0.0, 0.0);
   ossim_uint32 n = 0;
   const ossimDpt scales[4] = {ulScale(), urScale(), lrScale(), llScale()};
   for(ossim_uint32 idx = 0; idx < 4; ++idx)
   {
      if(!scales[idx].hasNans())
      {
         result += scales[idx];
         ++n;
      }
   }

   if(!n)
   {
      result.makeNan();
   }
   else
   {
      result.x /= n;
      result.y /= n;
   }

   return result;
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::imageToViewScale()const
{
   const ossimDpt scale = viewToImageScale();
   ossimDpt result;
   if(!scale.hasNans())
   {
      result.x = 1.0/scale.x;
      result.y = 1.0/scale.y;
   }
   else
   {
      result.makeNan();
   }

   return result;
}

bool ossimImageRenderer::ossimRendererSubRectInfo::tooBig()const
{
  ossimDrect vRect = getViewRect();

  return ((vRect.width() > 32) || (vRect.height() > 32));
}

ossim_uint16 ossimImageRenderer::ossimRendererSubRectInfo::getSplitFlags()const
{
  #if 1
   ossim_uint16 result = SPLIT_NONE;
   ossimDrect vRect = getViewRect();

   //---
   // Don't allow splits beyond 8x8 pixel. ossim2dBilinearTransform was core dumping with
   // very small rectangles in canBilinearInterpolate(...) method.
   // DRB 05 Dec. 2017
   //---
   if ( imageIsNan()||(vRect.width() < 8 && vRect.height() < 8) )
   {
      return result;
   }
  
   if(imageHasNans())
   {
      if(m_viewBounds && m_viewBounds->intersects(vRect))
      {
         result = SPLIT_ALL;
      }
      else
      {
         return result;
      }
   }
   /*
     if(result != SPLIT_ALL)
     {
     if(m_ulRoundTripError.hasNans()&&m_urRoundTripError.hasNans()&&
     m_lrRoundTripError.hasNans()&&m_llRoundTripError.hasNans())
     {
     if(m_viewBounds && m_viewBounds->intersects(getViewRect()))
     {
     result = SPLIT_ALL;
     }
     return result;
     }
     else if(tooBig())
     {
     result = SPLIT_ALL;
     }
     }

     if(result != SPLIT_ALL)
     {
     if(m_ulRoundTripError.hasNans()) result |= UPPER_LEFT_SPLIT_FLAG;
     if(m_urRoundTripError.hasNans()) result |= UPPER_RIGHT_SPLIT_FLAG;
     if(m_lrRoundTripError.hasNans()) result |= LOWER_RIGHT_SPLIT_FLAG;
     if(m_llRoundTripError.hasNans()) result |= LOWER_LEFT_SPLIT_FLAG;
     }
   */
   if(result != SPLIT_ALL)
   {
      //std::cout << "SCALE BIAS = " << bias << "\n";
      // if (bias < 1.0)
      //   bias = 1.0 / bias;
      // bias = std::sqrt(bias);

      // if(bias < 1) bias = 1.0;

      // if((m_ulRoundTripError.length() > bias)||
      //    (m_urRoundTripError.length() > bias)||
      //    (m_lrRoundTripError.length() > bias)||
      //    (m_llRoundTripError.length() > bias))
      // {
      // std::cout << "________________\n";

      // std::cout << "Bias:  " << bias << "\n"
      //           << "View:  " << getViewRect() << "\n"
      //           << "UL:    " << m_ulRoundTripError.length() << "\n"
      //           << "UR:   " << m_urRoundTripError.length() << "\n"
      //           << "LR:   " << m_lrRoundTripError.length() << "\n"
      //           << "LL:   " << m_llRoundTripError.length() << "\n";
//     }
      // if(m_ulRoundTripError.length() > sensitivityScale) result |= UPPER_LEFT_SPLIT_FLAG;
      // if(m_urRoundTripError.length() > sensitivityScale) result |= UPPER_RIGHT_SPLIT_FLAG;
      // if(m_lrRoundTripError.length() > sensitivityScale) result |= LOWER_RIGHT_SPLIT_FLAG;
      // if(m_llRoundTripError.length() > sensitivityScale) result |= LOWER_LEFT_SPLIT_FLAG;
      // std::cout << result << " == " << SPLIT_ALL << "\n";

      if(!canBilinearInterpolate())
      {
        //  std::cout << "SPLITTING\n";
         result = SPLIT_ALL;
      }
      else
      {
         // std::cout << "CAN BILINEAR!!!!\n";
      }
   }

   return result;
#else
  ossim_uint16 result = SPLIT_NONE;
  ossimDrect vRect = getViewRect();

  if(imageHasNans()||tooBig())
  {
     if(m_viewBounds && m_viewBounds->intersects(getViewRect()))
     {
      result = SPLIT_ALL;
     }
     else
     {
        return result;
     }
  }
  /*
  if(result != SPLIT_ALL)
  {
    if(m_ulRoundTripError.hasNans()&&m_urRoundTripError.hasNans()&&
        m_lrRoundTripError.hasNans()&&m_llRoundTripError.hasNans())
    {
      if(m_viewBounds && m_viewBounds->intersects(getViewRect()))
      {
        result = SPLIT_ALL;
      }
      return result;
    }
    else if(tooBig())
    {
      result = SPLIT_ALL;
    }
  }

  if(result != SPLIT_ALL)
  {
    if(m_ulRoundTripError.hasNans()) result |= UPPER_LEFT_SPLIT_FLAG;
    if(m_urRoundTripError.hasNans()) result |= UPPER_RIGHT_SPLIT_FLAG;
    if(m_lrRoundTripError.hasNans()) result |= LOWER_RIGHT_SPLIT_FLAG;
    if(m_llRoundTripError.hasNans()) result |= LOWER_LEFT_SPLIT_FLAG;
  }
*/
  if(result != SPLIT_ALL)
  {
    ossim_float64 sensitivityScale = imageToViewScale().length();
    //std::cout << sensitivityScale << std::endl;
    if(sensitivityScale < 1.0) sensitivityScale = 1.0/sensitivityScale;


     // if((m_ulRoundTripError.length() > sensitivityScale)||
     //    (m_urRoundTripError.length() > sensitivityScale)||
     //    (m_lrRoundTripError.length() > sensitivityScale)||
     //    (m_llRoundTripError.length() > sensitivityScale))
     // {
     //   std::cout << "________________\n";

     //   std::cout << "Sens:  " << sensitivityScale << "\n"
     //             << "View:  " << getViewRect() << "\n"
     //             << "UL:    " << m_ulRoundTripError.length() << "\n"
     //             << "UR:   " << m_urRoundTripError.length() << "\n"
     //             << "LR:   " << m_lrRoundTripError.length() << "\n"
     //             << "LL:   " << m_llRoundTripError.length() << "\n";
     // }
   // if(m_ulRoundTripError.length() > sensitivityScale) result |= UPPER_LEFT_SPLIT_FLAG;
   // if(m_urRoundTripError.length() > sensitivityScale) result |= UPPER_RIGHT_SPLIT_FLAG;
   // if(m_lrRoundTripError.length() > sensitivityScale) result |= LOWER_RIGHT_SPLIT_FLAG;
   // if(m_llRoundTripError.length() > sensitivityScale) result |= LOWER_LEFT_SPLIT_FLAG;
       // std::cout << result << " == " << SPLIT_ALL << "\n";

    if((result!=SPLIT_ALL)&&!canBilinearInterpolate(sensitivityScale))
    {
      // std::cout << "TESTING BILINEAR!!!!\n";
      result = SPLIT_ALL;

    }
    else
    {
      // std::cout << "CAN BILINEAR!!!!\n";
    }
  }

  return result;
#endif
}

void ossimImageRenderer::ossimRendererSubRectInfo::transformViewToImage()
{
//  std::cout << "TRANSFORM VIEW TO IMAGE!!!!!!!!!!!!!!\n";

   if(m_vertexCache)
   {
      return;
   }
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::computeViewToImageScale(const ossimDpt& viewPt,
                           const ossimDpt& delta)const
{
  ossimDpt result;
  result.makeNan();
  if(viewPt.hasNans()) return result; 
  ossimDpt ipt;
  m_transform->viewToImage(viewPt, ipt);

  if(!ipt.isNan())
  {
//    ossimDpt delta;
//    transform->viewToImage(viewPt+ossimDpt(0.5,0.5), delta);

//    delta = delta-ipt;
//    result.x = delta.length()/std::sqrt(2);
 //   result.y = result.x;

    ossimDpt dx;
    ossimDpt dy;

    m_transform->viewToImage(viewPt + ossimDpt(delta.x,0.0), dx);
    m_transform->viewToImage(viewPt + ossimDpt(0.0,delta.y), dy);
    dx = dx-ipt;
    dy = dy-ipt;

    result.x = dx.length()/fabs(delta.x);
    result.y = dy.length()/fabs(delta.y);
  }

  return result;
}

void ossimImageRenderer::ossimRendererSubRectInfo::stretchImageOut(bool enableRound)
{
   (void)enableRound;
}

bool ossimImageRenderer::ossimRendererSubRectInfo::isIdentity()const
{
    double iulDelta = (iul()-vul()).length();
    double iurDelta = (iur()-vur()).length();
    double ilrDelta = (ilr()-vlr()).length();
    double illDelta = (ill()-vll()).length();

    return ((iulDelta <= FLT_EPSILON)&&
            (iurDelta <= FLT_EPSILON)&&
            (ilrDelta <= FLT_EPSILON)&&
            (illDelta <= FLT_EPSILON));
}

bool ossimImageRenderer::ossimRendererSubRectInfo::canBilinearInterpolate() const
//bool ossimImageRenderer::ossimRendererSubRectInfo::canBilinearInterpolate(double error) const
{
  bool result = false;

  // now check point placement
  ossimDpt imageToViewScale = getAbsValueImageToViewScales();

  if(imageToViewScale.hasNans()) return true;

  // ossim_float64 bias = imageToViewScale.length();

  // root average of 1x1 = sqrt(2)
  const double ROOT_AVERAGE = 1.41421356237309504880;
  double testScale = imageToViewScale.length()/ROOT_AVERAGE;

  // if there is a large shrink or expansion then just return true.
  // You are probably not worried about error in bilinear interpolation
  //
  if ((testScale > 256) ||
      (testScale < 1.0 / 256.0))
  {
    return true;
  }

  if (ulScale().hasNans() ||
      urScale().hasNans() ||
      lrScale().hasNans() ||
      llScale().hasNans())
  {
    return result;
  }

  // check overage power of 2 variance
  // If there is a variance of 1 resolution level
  // then we must split further
  //
  ossim_float64 averageUlScale = ulScale().length();
  ossim_float64 averageUrScale = urScale().length();
  ossim_float64 averageLrScale = lrScale().length();
  ossim_float64 averageLlScale = llScale().length();

  // std::cout << "_________________________\n";
  // std::cout << log(averageUlScale)/(log(2)) << "\n";
  // std::cout << log(averageUrScale)/(log(2)) << "\n";
  // std::cout << log(averageLrScale)/(log(2)) << "\n";
  // std::cout << log(averageLlScale)/(log(2)) << "\n";

  ossim_float64 ratio1 = averageUlScale / averageUrScale;
  ossim_float64 ratio2 = averageUlScale / averageLrScale;
  ossim_float64 ratio3 = averageUlScale / averageLlScale;

  // std::cout << "_________________________\n";
  // std::cout << "ratio1: " << ratio1 << "\n";
  // std::cout << "ratio2: " << ratio2 << "\n";
  // std::cout << "ratio3: " << ratio3 << "\n";

  // make sure all are within a power of 2 shrink or expand
  // which means the range of each ratio should be
  // between .5 and 2
  result = (((ratio1 < 2) && (ratio1 > 0.5)) &&
            ((ratio2 < 2) && (ratio2 > 0.5)) &&
            ((ratio3 < 2) && (ratio3 > 0.5)));

  //result = ((diff1<=2)&&(diff2<=2)&&(diff3<=2));
  //std::cout << "DIFF1: " << diff1 << std::endl;
  //std::cout << "DIFF2: " << diff2 << std::endl;
  //std::cout << "DIFF3: " << diff3 << std::endl;

  if (result)
  {
#if 1
    ossimDpt vUpper, vRight, vBottom, vLeft, vCenter;
    ossimDpt iUpper, iRight, iBottom, iLeft, iCenter;
    ossimDpt testUpper, testRight, testBottom, testLeft, testCenter;

    getViewMids(vUpper, vRight, vBottom, vLeft, vCenter);
    getImageMids(iUpper, iRight, iBottom, iLeft, iCenter);

    // get the model centers for the mid upper left right bottom
    m_transform->viewToImage(vCenter, testCenter);

    if (testCenter.hasNans())
    {
      return false;
    }

    m_transform->viewToImage(vUpper, testUpper);
    if (testUpper.hasNans())
    {
      return false;
    }
    m_transform->viewToImage(vRight, testRight);
    if (testRight.hasNans())
    {
      return false;
    }
    m_transform->viewToImage(vBottom, testBottom);
    if (testBottom.hasNans())
    {
      return false;
    }
    m_transform->viewToImage(vLeft, testLeft);
    if (testLeft.hasNans())
    {
      return false;
    }

    // now get the model error to bilinear estimate of those points
    double errorCheck1 = (testCenter - iCenter).length();
    double errorCheck2 = (testUpper - iUpper).length();
    double errorCheck3 = (testRight - iRight).length();
    double errorCheck4 = (testBottom - iBottom).length();
    double errorCheck5 = (testLeft - iLeft).length();

    //  This is crude but we will do a simple scale to find the R0 delta.
    //  If the R0 delta is within a pixel then we will return true 
    // for canBilinear
    if (testScale <= FLT_EPSILON)
    {
      errorCheck1 = 0.0;
      errorCheck2 = 0.0;
      errorCheck3 = 0.0;
      errorCheck4 = 0.0;
      errorCheck5 = 0.0;
    }
    else if(testScale > 1.0)
    {
      // errorCheck1 *= testScale;
      // errorCheck2 *= testScale;
      // errorCheck3 *= testScale;
      // errorCheck4 *= testScale;
      // errorCheck5 *= testScale;
    }
    else if(testScale > FLT_EPSILON)
    {
      // errorCheck1 /= testScale;
      // errorCheck2 /= testScale;
      // errorCheck3 /= testScale;
      // errorCheck4 /= testScale;
      // errorCheck5 /= testScale;
    }
    // errorCheck1 = sqrt(errorCheck1);
    // errorCheck2 = sqrt(errorCheck2);
    // errorCheck3 = sqrt(errorCheck3);
    // errorCheck4 = sqrt(errorCheck4);
    // errorCheck5 = sqrt(errorCheck5);

    // std::cout << "SCALE BIAS: " << testScale << "\n";

    // std::cout << "errorCheck1:" << errorCheck1 << "\n";
    result = ((errorCheck1 < ossimImageRenderer::m_interpErrorThreshold) &&
              (errorCheck2 < ossimImageRenderer::m_interpErrorThreshold) &&
              (errorCheck3 < ossimImageRenderer::m_interpErrorThreshold) &&
              (errorCheck4 < ossimImageRenderer::m_interpErrorThreshold) &&
              (errorCheck5 < ossimImageRenderer::m_interpErrorThreshold));
    // std::cout <<"__________________________\n"
    //       << "ERROR1:" <<errorCheck1 << "\n"
    //       << "ERROR2:" <<errorCheck2 << "\n"
    //       << "ERROR3:" <<errorCheck3 << "\n"
    //       << "ERROR4:" <<errorCheck4 << "\n"
    //       << "ERROR5:" <<errorCheck5 << "\n"
    //       << "SENS:  " << error <<  "\n";

#else
    ossimDpt vUpper, vRight, vBottom, vLeft, vCenter;
    ossimDpt iUpper, iRight, iBottom, iLeft, iCenter;

    ossimDpt testCenter;
    getViewMids(vUpper, vRight, vBottom, vLeft, vCenter);
    getImageMids(iUpper, iRight, iBottom, iLeft, iCenter);

    ossimDpt iFullRes(iCenter.x * imageToViewScale.x,
                      iCenter.y * imageToViewScale.y);

    m_transform->viewToImage(vCenter, testCenter);

    if (testCenter.hasNans())
    {
      return false;
    }
    ossimDpt testFullRes(testCenter.x * imageToViewScale.x,
                         testCenter.y * imageToViewScale.y);

    double errorCheck1 = (testFullRes - iFullRes).length();

    iFullRes = ossimDpt(iUpper.x * imageToViewScale.x,
                        iUpper.y * imageToViewScale.y);

    m_transform->viewToImage(vUpper, testCenter);
    if (testCenter.hasNans())
    {
      return false;
    }
    testFullRes = ossimDpt(testCenter.x * imageToViewScale.x,
                           testCenter.y * imageToViewScale.y);
    double errorCheck2 = (testFullRes - iFullRes).length();

    iFullRes = ossimDpt(iRight.x * imageToViewScale.x,
                        iRight.y * imageToViewScale.y);

    m_transform->viewToImage(vRight, testCenter);
    if (testCenter.hasNans())
    {
      return false;
    }
    testFullRes = ossimDpt(testCenter.x * imageToViewScale.x,
                           testCenter.y * imageToViewScale.y);
    double errorCheck3 = (testFullRes - iFullRes).length();

    iFullRes = ossimDpt(iBottom.x * imageToViewScale.x,
                        iBottom.y * imageToViewScale.y);

    m_transform->viewToImage(vBottom, testCenter);
    if (testCenter.hasNans())
    {
      return false;
    }
    testFullRes = ossimDpt(testCenter.x * imageToViewScale.x,
                           testCenter.y * imageToViewScale.y);
    double errorCheck4 = (testFullRes - iFullRes).length();

    iFullRes = ossimDpt(iLeft.x * imageToViewScale.x,
                        iLeft.y * imageToViewScale.y);

    m_transform->viewToImage(vLeft, testCenter);
    testFullRes = ossimDpt(testCenter.x * imageToViewScale.x,
                           testCenter.y * imageToViewScale.y);
    double errorCheck5 = (testFullRes - iFullRes).length();

    std::cout << "__________________________\n"
              << "ERROR1:" << errorCheck1 << "\n"
              << "ERROR2:" << errorCheck2 << "\n"
              << "ERROR3:" << errorCheck3 << "\n"
              << "ERROR4:" << errorCheck4 << "\n"
              << "ERROR5:" << errorCheck5 << "\n"
              << "SENS:  " << error << "\n";

    result = ((errorCheck1 < error) &&
              (errorCheck2 < error) &&
              (errorCheck3 < error) &&
              (errorCheck4 < error) &&
              (errorCheck5 < error));
    // std::cout << "CAN INTERPOLATE? " << result <<"\n";
#endif
  }
  return result;
}

void ossimImageRenderer::ossimRendererSubRectInfo::getViewMids(ossimDpt& upperMid,
                 ossimDpt& rightMid,
                 ossimDpt& bottomMid,
                 ossimDpt& leftMid,
                 ossimDpt& center)const
{
  
  upperMid  = (vul() + vur())*.5;
  rightMid  = (vur() + vlr())*.5;
  bottomMid = (vlr() + vll())*.5;
  leftMid   = (vul() + vll())*.5;
  center    = (vul() + vur() + vlr() + vll())*.25;
}

void ossimImageRenderer::ossimRendererSubRectInfo::getImageMids(ossimDpt& upperMid,
                  ossimDpt& rightMid,
                  ossimDpt& bottomMid,
                  ossimDpt& leftMid,
                  ossimDpt& center)const
{
  if(imageHasNans())
  {
    upperMid.makeNan();
    rightMid.makeNan();
    bottomMid.makeNan();
    leftMid.makeNan();
    center.makeNan();
  }
  else
  {
    upperMid  = (iul() + iur())*.5;
    rightMid  = (iur() + ilr())*.5;
    bottomMid = (ilr() + ill())*.5;
    leftMid   = (iul() + ill())*.5;
    center    = (iul() + iur() + ilr() + ill())*.25;
  }
}

ossimDpt ossimImageRenderer::ossimRendererSubRectInfo::getParametricCenter(const ossimDpt& ul, const ossimDpt& ur, 
                              const ossimDpt& lr, const ossimDpt& ll)const
{
  ossimDpt top    = ur - ul;
  ossimDpt bottom = lr - ll;
  
  ossimDpt centerTop = ul + top * .5;
  ossimDpt centerBottom = ll + bottom * .5;

  return centerBottom + (centerBottom - centerTop)*.5;
}



ossimImageRenderer::ossimImageRenderer()
    : ossimImageSourceFilter(),
      ossimViewInterface(0),
      m_Resampler(0),
      m_BlankTile(0),
      m_Tile(0),
      m_TemporaryBuffer(0),
      m_StartingResLevel(0),
      m_ImageViewTransform(0),
      m_inputR0Rect(),
      m_viewRect(),
      m_rectsDirty(true),
      m_MaxRecursionLevel(5),
      m_AutoUpdateInputTransform(true),
      m_MaxLevelsToCompute(999999), // something large so it will always compute
      m_averageViewToImageScale(1.0),
      m_averageViewToImageRLevelScale(0.0)
{
  ossimViewInterface::theObject = this;
  m_Resampler = new ossimFilterResampler();
  m_ImageViewTransform = new ossimImageViewProjectionTransform;

  loadState(ossimPreferences::instance()->preferencesKWL(), "renderer.");
}

ossimImageRenderer::ossimImageRenderer(ossimImageSource *inputSource,
                                       ossimImageViewTransform *imageViewTrans)
    : ossimImageSourceFilter(inputSource),
      ossimViewInterface(0),
      m_Resampler(0),
      m_BlankTile(0),
      m_Tile(0),
      m_TemporaryBuffer(0),
      m_StartingResLevel(0),
      m_inputR0Rect(),
      m_viewRect(),
      m_rectsDirty(true),
      m_MaxRecursionLevel(5),
      m_AutoUpdateInputTransform(true),
      m_MaxLevelsToCompute(999999),  // something large so it will always compute
      m_averageViewToImageScale(1.0),
      m_averageViewToImageRLevelScale(0.0)

{
   ossimViewInterface::theObject = this;
   m_Resampler = new ossimFilterResampler();

   loadState(ossimPreferences::instance()->preferencesKWL(), "renderer.");
   if (imageViewTrans)
      m_ImageViewTransform = imageViewTrans;
}

ossimImageRenderer::~ossimImageRenderer()
{
  m_ImageViewTransform = 0;

   if(m_Resampler)
   {
      delete m_Resampler;
      m_Resampler = 0;
   }
}

const ossimImageRenderer::RenderingStats&
ossimImageRenderer::getLastRenderingStats()const
{
   return m_lastRenderingStats;
}

ossimRefPtr<ossimImageData> ossimImageRenderer::getTile(
   const  ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   const auto getTileStart = std::chrono::steady_clock::now();
  //  std::cout << "_________________________\n";
   static const char MODULE[] = "ossimImageRenderer::getTile";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " Requesting view rect = "
         << tileRect << endl;
   }
   m_currentRenderingStats = RenderingStats();
   m_currentRenderingStats.m_requestedViewRect = tileRect;
   m_currentRenderingStats.m_clippedViewRect = tileRect;
   m_currentRenderingStats.m_requestedViewPixels = rendererRectPixels(tileRect);
   m_currentRenderingStats.m_clippedViewPixels =
      m_currentRenderingStats.m_requestedViewPixels;
   m_lastRenderingStats = m_currentRenderingStats;
   const auto finishStats =
      [this, &getTileStart]()
      {
         m_currentRenderingStats.m_totalSeconds =
            rendererElapsedSeconds(getTileStart);
         m_lastRenderingStats = m_currentRenderingStats;
      };
   // long w = tileRect.width();
   // long h = tileRect.height();
   // ossimIpt origin = tileRect.ul();
   
   if( !m_BlankTile.valid() || !m_Tile.valid() )
   {
      allocate();
      if ( !m_BlankTile.valid() || !m_Tile.valid() )
      {
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << "ossimImageRenderer::getTile tile allocation failure!\n"
               << endl;
         }
         finishStats();
         return ossimImageSourceFilter::getTile(tileRect, resLevel);
      }
   }

   m_BlankTile->setImageRectangle(tileRect);
   
   if(!theInputConnection)
   {
      finishStats();
      return m_BlankTile;
   }
   
   if ( !isSourceEnabled()||(!m_ImageViewTransform.valid())||
        (!m_ImageViewTransform->isValid()) )
   {
      // This tile source bypassed, return the input tile source.
      finishStats();
      return theInputConnection->getTile(tileRect, resLevel);  
   }

   if( m_rectsDirty )
   {
      initializeBoundingRects();

      // We can't go on without these...
      if ( m_rectsDirty )
      {
         finishStats();
         return m_BlankTile;
      }
   }
   
   if(m_viewRect.width() < 4 && m_viewRect.height() < 4)
   {
      finishStats();
      return m_BlankTile;
   }

   if( !theInputConnection || 
       !m_viewRect.intersects(tileRect) ||
       !m_viewArea.intersects(tileRect) )
   {
      if(traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << MODULE << "No intersection, Returning...." << endl;
      }
      finishStats();
      return m_BlankTile;
   }
   
   // drb - handled above...
   // if(!m_Tile)
   // {
   //    return theInputConnection->getTile(tileRect, resLevel);
   // }

   // Check for identity transform:
   if( m_ImageViewTransform->isIdentity() == true )
   {
      finishStats();
      return theInputConnection->getTile(tileRect, resLevel);
   }
   
   // long tw = m_Tile->getWidth();
   // long th = m_Tile->getHeight();
   
   m_Tile->setImageRectangle(tileRect);
   m_Tile->makeBlank();
 

  //if(!(m_viewArea.intersects(ossimPolyArea2d(tileRect))))
  //{
  //  return m_BlankTile;
  //} 

#if 1
   ossimIrect tempRect = tileRect;
   // ossim_uint32 levels = theInputConnection->getNumberOfDecimationLevels();
   ossim_float64 length = ossim::max<ossim_float64>(m_inputR0Rect.width(), m_inputR0Rect.height());
   
   // if we are zoomed out or we are completely within the requesting tile rect
   bool canClip = (m_viewRect.completely_within(tileRect));

   if(!canClip)
   {
     if(m_averageViewToImageScale > 1.0)
     {
       if ((length / m_averageViewToImageScale) <= 256)
       {
         // we are very small so we can clip
         canClip = true;
       }
     }
   }
   // gpotts:  Until I can fix the entire resampling process we will add a sanity
   // if we are zoomed out or the rect completely fits within the tile
   // then we will clip to the view rect
   if (canClip)
   {
     ossimPolyArea2d tileRectArea(tileRect);
     if (!(m_viewArea.intersects(tileRectArea)))
     {
       finishStats();
       return m_BlankTile;
     }
     else
     {
       ossimDrect rect;
       tileRectArea &= m_viewArea;
       tileRectArea.getBoundingRect(rect);
       tempRect = rect;
       
     }
   }
//     tempRect = tileRect.clipToRect(m_viewRect);
   // expand a small patch just to alleviate errors in the size of the rect when resampling
     // ossimIrect viewRectClip = tileRect.clipToRect(ossimIrect(m_viewRect.ul() + ossimIpt(-8,-8),
     //                                                          m_viewRect.lr() + ossimIpt(8,8)));
   //ossimIrect viewRectClip = tileRect.clipToRect(m_viewRect);
   
       //   std::cout << "_____________________" << std::endl;
       //   std::cout << "viewRectClip = " <<  viewRectClip << std::endl;
       //   std::cout << "tileRect = " <<  tileRect << std::endl;
       //   std::cout << "m_viewRect = " <<  m_viewRect << std::endl;
   ossimRendererSubRectInfo subRectInfo(m_ImageViewTransform.get());
#else
   ossimRendererSubRectInfo subRectInfo(m_ImageViewTransform.get());


#endif
   std::unique_ptr<ossimRendererVertexCache> vertexCache(
      new ossimRendererVertexCache(m_ImageViewTransform.get()));
   std::shared_ptr<const ossimPolyArea2d> viewBounds(
      new ossimPolyArea2d(m_viewArea));
   subRectInfo.m_viewBounds = viewBounds;
   subRectInfo.setVertexCache(vertexCache.get());
   subRectInfo.setVertexIndices(vertexCache->addVertex(tempRect.ul()),
                                vertexCache->addVertex(tempRect.ur()),
                                vertexCache->addVertex(tempRect.lr()),
                                vertexCache->addVertex(tempRect.ll()));
   subRectInfo.transformViewToImage();

   if((!m_viewArea.intersects(subRectInfo.getViewRect())))
//   if((!m_viewRect.intersects(subRectInfo.getViewRect())))
   {
     finishStats();
     return m_BlankTile;
   }
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " image rect = " << subRectInfo.getImageRect() << std::endl;
   }

   // If the image rect is completely outside of the valid image, there is no need to resample:
   // (OLK 11/18)
//   if ((!subRectInfo.imageHasNans())&&!m_inputR0Rect.intersects(subRectInfo.getImageRect()))
//   {
//      return m_Tile;
//   }
   m_currentRenderingStats.m_clippedViewRect = tempRect;
   m_currentRenderingStats.m_clippedViewPixels = rendererRectPixels(tempRect);
   m_currentRenderingStats.m_clipped = (tileRect != tempRect);
   const auto recursiveStart = std::chrono::steady_clock::now();
   recursiveResample(m_Tile, subRectInfo, 1);
   m_currentRenderingStats.m_recursiveSeconds +=
      rendererElapsedSeconds(recursiveStart);
   vertexCache->updateRenderingStats(m_currentRenderingStats);
   finishStats();
   vertexCache->printStats(tileRect);
  
   if(m_Tile.valid())
   {
      m_Tile->validate();
   }
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << "Returning...." << endl;
   }
   return m_Tile;
}

void ossimImageRenderer::recursiveResample(ossimRefPtr<ossimImageData> outputData,
                                           const ossimRendererSubRectInfo& rectInfo,
                                           ossim_uint32 /* level */)
{
  // Removed recursion and just use the std::stack.
  //
  if(!rectInfo.m_vertexCache)
  {
     return;
  }

  std::stack<ossim_int64> rectStack;
  rectStack.push(rectInfo.m_vertexCache->addNode(rectInfo));

  while(!rectStack.empty())
  {
    const ossim_int64 currentNode = rectStack.top();
    const ossimRendererSubRectInfo currentRectInfo =
       rectInfo.m_vertexCache->getNode(currentNode).m_rect;
    ossimIrect tempViewRect = currentRectInfo.getViewRect();
    rectStack.pop();
    ++m_currentRenderingStats.m_visitedNodes;
    if(m_viewArea.intersects(tempViewRect))
    {
      if(tempViewRect.width() <2 ||
          tempViewRect.height() <2)
      {
          if(!currentRectInfo.imageHasNans())
          {
             fillTile(outputData,
                      currentRectInfo);
          }
      }
      else
      {
        ossim_uint32 idx = 0;
        std::vector<ossim_int64> splitRects;
        rectInfo.m_vertexCache->splitNode(currentNode, splitRects);
        if(!splitRects.empty())
        {
          ++m_currentRenderingStats.m_splitNodes;
          for(idx = 0; idx < splitRects.size();++idx)
          {
            const ossimRendererSubRectInfo& splitRect =
               rectInfo.m_vertexCache->getNode(splitRects[idx]).m_rect;
            if(m_viewArea.intersects(splitRect.getViewRect()))
            {
              rectStack.push(splitRects[idx]);
            }
            // recursiveResample(outputData,
            //                   splitRects[idx],
            //                   level + 1);
          }
        }
        else
        {
          if(!currentRectInfo.imageHasNans())
          {
            fillTile(outputData,
                    currentRectInfo);
          }
        }
      }

    }
  }
  #if 0
   ossimIrect tempViewRect = rectInfo.getViewRect();
   if(rectInfo.imageIsNan())
   {
      return;
   } 

  if(tempViewRect.width() <2 ||
      tempViewRect.height() <2)
  {
      if(!rectInfo.imageHasNans())
      {
         fillTile(outputData,
                  rectInfo);
      }
      return;
  }
  //
  std::vector<ossimRendererSubRectInfo> splitRects;
  rectInfo.splitView(splitRects);

//std::cout << "SHOULD BE SPLITTING: " << splitRects.size() <<"\n";
  ossim_uint32 idx = 0;
  if(!splitRects.empty())
  {
   // std::cout << "SPLITTING " << level << ", " << tempViewRect << "\n";
    for(idx = 0; idx < splitRects.size();++idx)
    {
      recursiveResample(outputData,
                        splitRects[idx],
                        level + 1);
    }
  }
  else if(!rectInfo.imageHasNans())
  {
    fillTile(outputData,
            rectInfo);
  }
  #endif
}

#define RSET_SEARCH_THRESHHOLD 0.1

void ossimImageRenderer::fillTile(ossimRefPtr<ossimImageData> outputData,
                                  const ossimRendererSubRectInfo& rectInfo)
{
   const auto fillStart = std::chrono::steady_clock::now();
   if(!outputData.valid() || !outputData->getBuf() || rectInfo.imageHasNans())
   {
      return;
   }
   ++m_currentRenderingStats.m_filledLeaves;
   ossimDrect vrect = rectInfo.getViewRect();
   
   ossimDpt imageToViewScale = rectInfo.getAbsValueImageToViewScales();
   
   if(imageToViewScale.hasNans()) return;
   
   ossimDpt tile_size = ossimDpt(vrect.width(), vrect.height());
   double kernelSupportX, kernelSupportY;
   
   double resLevelX = log( 1.0 / imageToViewScale.x )/ log( 2.0 );
   double resLevelY = log( 1.0 / imageToViewScale.y )/ log( 2.0 );
   double resLevel0 = resLevelX < resLevelY ? resLevelX : resLevelY;
   long closestFitResLevel = (long)floor( resLevel0 );
   
   //double averageScale = (imageToViewScale.x + imageToViewScale.y) / 2.0;
   //long closestFitResLevel = (long)floor( log( 1.0 / averageScale )/ log( 2.0 ) );
   
   ossim_uint32 resLevel = closestFitResLevel<0 ? 0:closestFitResLevel;
   resLevel += m_StartingResLevel;

   //---
   // ESH 02/2009: If requested resLevel is too high, let's lower it to one
   // that is ok.
   //---
#if 0
   const ossim_uint32 NUM_LEVELS = theInputConnection->getNumberOfDecimationLevels();
   if ( (NUM_LEVELS > 0) && (resLevel >=  NUM_LEVELS) )
   {
      resLevel = NUM_LEVELS - 1;
   }
#endif
   //---
   // ESH 11/2008: Check the rset at the calculated resLevel to see
   // if it has the expected decimation factor. It it does, we can 
   // use this rset and assume it is at resLevel.
   //--- 
   ossimDpt decimation;
   decimation.makeNan(); // initialize to nan.
   theInputConnection->getDecimationFactor(resLevel, decimation);
   double requestScale = 1.0 / (1<<resLevel);
   double closestScale = decimation.hasNans() ? requestScale : decimation.x;

#if 0
   double differenceTest = 0.0;
   if (closestScale != 0.0)
   {
      differenceTest = (1.0/closestScale) - (1.0/requestScale);
   }

   //---
   // ESH 11/2008: Add in threshold test so search only happens when 
   //              necessary.
   // We do an rset search if 1 of 2 conditions is met: either
   //   1) the rset is really different in size from the requested size, or
   //   2) they're similar in size, and the actual rset is smaller than 
   //      the requested size.
   //---
   if ( (fabs(differenceTest) > RSET_SEARCH_THRESHHOLD) || 
        ((fabs(differenceTest) < RSET_SEARCH_THRESHHOLD) &&
         (differenceTest < 0.0) ) )
   {
      //---
      // ESH 11/2008: We test for the best rset. We assume 
      // that decimation level always decreases as resLevel increases, so 
      // the search can end before testing all rsets.
      //---
      ossim_uint32 savedResLevel = resLevel;
      closestScale = 1.0; // resLevel 0
      resLevel = 0;
      ossim_uint32 i;
      for( i=1; i<NUM_LEVELS; ++i )
      {
         theInputConnection->getDecimationFactor(i, decimation);
         if(decimation.hasNans() == false )
         {
            double testDiscrepancy = decimation.x - requestScale;
            if ( testDiscrepancy < 0.0 ) // we're done
            {
               break;
            }
            else
            {
               closestScale = decimation.x;
               resLevel = i;
            }
         }
         else // use the default value
         {
            closestScale = requestScale;
            resLevel = savedResLevel;
            break;
         }
      }
   }
#endif
   const ossimDpt iul = rectInfo.iul();
   const ossimDpt iur = rectInfo.iur();
   const ossimDpt ilr = rectInfo.ilr();
   const ossimDpt ill = rectInfo.ill();
   ossimDpt nul(iul.x*closestScale,
                iul.y*closestScale);
   ossimDpt nll(ill.x*closestScale,
                ill.y*closestScale);
   ossimDpt nlr(ilr.x*closestScale,
                ilr.y*closestScale);
   ossimDpt nur(iur.x*closestScale,
                iur.y*closestScale);
   
   m_Resampler->getKernelSupport( kernelSupportX, kernelSupportY );
   
   ossimDrect boundingRect = ossimDrect( nul, nll, nlr, nur );
   

   boundingRect = ossimIrect((ossim_int32)floor(boundingRect.ul().x - (kernelSupportX)-.5),
                             (ossim_int32)floor(boundingRect.ul().y - (kernelSupportY)-.5),
                             (ossim_int32)ceil (boundingRect.lr().x + (kernelSupportX)+.5),
                             (ossim_int32)ceil (boundingRect.lr().y + (kernelSupportY)+.5));
   
   ossimDrect requestRect = boundingRect;
   ++m_currentRenderingStats.m_inputTileCalls;
   m_currentRenderingStats.m_inputTilePixels +=
      rendererRectPixels(ossimIrect(requestRect));
   m_currentRenderingStats.m_maxInputTileWidth =
      ossim::max(m_currentRenderingStats.m_maxInputTileWidth,
                 static_cast<ossim_uint32>(
                    ossim::max<ossim_int32>(0, ossimIrect(requestRect).width())));
   m_currentRenderingStats.m_maxInputTileHeight =
      ossim::max(m_currentRenderingStats.m_maxInputTileHeight,
                 static_cast<ossim_uint32>(
                    ossim::max<ossim_int32>(0, ossimIrect(requestRect).height())));
   if(m_currentRenderingStats.m_inputTileCalls == 1)
   {
      m_currentRenderingStats.m_minInputResLevel = resLevel;
      m_currentRenderingStats.m_maxInputResLevel = resLevel;
   }
   else
   {
      m_currentRenderingStats.m_minInputResLevel =
         ossim::min(m_currentRenderingStats.m_minInputResLevel, resLevel);
      m_currentRenderingStats.m_maxInputResLevel =
         ossim::max(m_currentRenderingStats.m_maxInputResLevel, resLevel);
   }
   
   const auto inputGetTileStart = std::chrono::steady_clock::now();
   ossimRefPtr<ossimImageData> data = getTileAtResLevel(requestRect, resLevel);
   m_currentRenderingStats.m_inputGetTileSeconds +=
      rendererElapsedSeconds(inputGetTileStart);
   
   ossimDataObjectStatus status = OSSIM_NULL;
   if( data.valid() )
   {
      status = data->getDataObjectStatus();
   }
   if( (status == OSSIM_NULL) || (status == OSSIM_EMPTY) )
   {
      return;
   }
   
   if((boundingRect.width() <2)&&(boundingRect.height()<2))
   {
              
//    return;
   }// std::cout << "SMALL RECT!!!!!!\n";
   else
   {
     ossimDrect inputRect = m_inputR0Rect;
     inputRect = inputRect*ossimDpt(closestScale, closestScale);
     m_Resampler->setBoundingInputRect(inputRect);
     
     double denominatorY = 1.0;
     if(tile_size.y > 2)
     {
        denominatorY = tile_size.y-1.0;
     }
     
     ossimDpt newScale( imageToViewScale.x / closestScale,
                       imageToViewScale.y / closestScale );
     m_Resampler->setScaleFactor(newScale);
     

  //std::cout << "SPLIT VIEW RECT: " << vrect << std::endl;
  //std::cout << "VIEW RECT: " << outputData->getImageRectangle() << std::endl;


     const auto resampleStart = std::chrono::steady_clock::now();
     m_Resampler->resample(data,
                           outputData,
                           vrect,
                           nul,
                           nur,
                           ossimDpt( ( (nll.x - nul.x)/denominatorY ),
                                     ( (nll.y - nul.y)/denominatorY ) ),
                           ossimDpt( ( (nlr.x - nur.x)/denominatorY ),
                                     ( (nlr.y - nur.y)/denominatorY ) ),
                           tile_size);
     m_currentRenderingStats.m_resampleSeconds +=
        rendererElapsedSeconds(resampleStart);
   }
   m_currentRenderingStats.m_fillSeconds += rendererElapsedSeconds(fillStart);
   
}

long ossimImageRenderer::computeClosestResLevel(const std::vector<ossimDpt>& decimationFactors,
                                                double scale)const
{
   long result = 0;
   long upper  = (long)decimationFactors.size();
   bool done   = false;
   
   if(upper > 1)
   {
      while((result < upper)&&!done)
      {
         if(scale < decimationFactors[result].x)
         {
            ++result;
         }
         else
         {
            done = true;
         }
      }
      
      // now use the higher res level and resample down
      //
      if(result)
      {
         --result;
      }
   }
   return result;
}

ossimIrect ossimImageRenderer::getBoundingRect(ossim_uint32 resLevel)const
{
   //---
   // 01 November 2011:
   // Backed out expand code as the ossimImageViewProjectionTransform::getImageToViewBounds
   // does not handle image on the edge of international date line. D. Burken
   //---
   
   // The input bounding rect as returned here corresponds to "pixel-is-point", i.e., the center
   // of the pixel area for the corners and not the edges which extend 1/2 pixel in all 
   // directions. Because the view is probably a different GSD, first expand the input
   // bounding rect to the edge before transforming to a view rect. Then shrink the view rect by
   // 1/2 pixel to get it to the pixel center (OSSIM convention for bounding rect -- this may 
   // need to be revisited in light of the need to match edges, not pixel centers, OLK 09/11).

   // Code functionality moved to below method. (drb - 08 Nov. 2011)
   
   ossimIrect result;
   getBoundingRect( result, resLevel );
   return result;
}

void ossimImageRenderer::getBoundingRect(ossimIrect& rect, ossim_uint32 resLevel) const
{
   if ( isSourceEnabled() )
   {
      rect = m_viewRect;
   }
   else if ( theInputConnection )
   {
      rect = theInputConnection->getBoundingRect(resLevel);
   }
   else
   {
      rect.makeNan();
   }
#if 0 /* Please leave for debug. */
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimImageRenderer::getBoundingRect(rect, resLevel) debug:\nbounds  = "
         << rect << "\n";
   }
#endif  
}

void ossimImageRenderer::initializeBoundingRects()
{
   m_averageViewToImageScale = 1.0;
   m_rectsDirty = true;
   ossimImageViewProjectionTransform *ivpt =
      dynamic_cast<ossimImageViewProjectionTransform *>(m_ImageViewTransform.get());
   if (!theInputConnection || !m_ImageViewTransform.valid())
      return;
   m_inputR0Rect = theInputConnection->getBoundingRect(0);
   if (!m_inputR0Rect.hasNans())
   {
      if (ivpt && ivpt->getImageGeometry() && ivpt->getViewGeometry())
      {
         // Little complicated but instead of always setting the edge walk
         // to a high number like 50 points per edge
         // we will look at the image to view scale change
         // and use that as a factor.  So as the image zooms out we
         // need fewer points to estimate the edge.
         //
         // ossim_uint32 idx;
         std::vector<ossimDrect> boundList;
         ossimImageGeometry *igeom = ivpt->getImageGeometry(); // look at projected meters
         ossimImageGeometry *vgeom = ivpt->getViewGeometry();  // look at projected meters
         ossimDrect testRect;
         igeom->getBoundingRect(testRect);
         ossimDpt mpp = igeom->getMetersPerPixel();
         ossimDpt vmpp = vgeom->getMetersPerPixel();
         ossim_float64 scale = 1.0;
         ossim_uint32 maxLen = ossim::max(testRect.width(), testRect.height());
         // (GP March 2, 2017) determine goodMatch : test if we have either enough samples to closely match the post spacing
         //  or if we have at least half the number of pixels along the edge of an image
         //
         // This is hopefully to help avoid when using the polygon for intersection to not have
         // bad intersection tests when zooming.  We might have to rethink it and implement
         // this as a windowed edge walker.  So when zooming we create a polygon that
         // is denser for only what the view can see and not the entire image.  Basically interatively
         // tesselate the input image model based on bounding volumes and then create a dense edge walker
         // for what lies in the view at the given scale.  Too much to implement right now so we will cheet
         // and take an easy way out for now.
         //

         if (!mpp.hasNans() && !vmpp.hasNans())
         {
            scale = mpp.y / vmpp.y;
            if (scale > 1.0)
               scale = 1.0;
         }
         ossim_float64 mppTest = mpp.y;
         ossim_float64 divisor = mppTest; // default to 30 meter elevation
         if (mppTest < 500)
         {
            if (mppTest >= 45)
            {
               divisor = 90; // 90 meters
            }
         }
         else
         {
            divisor = 1000; // 1 kilometer
         }
         if (divisor < 30)
            divisor = 30.0;

         // now test to see if our edge walk is close to matching enough
         // points for a good match.
         //
         ossim_uint32 maxEdgeSample = ossim::min(static_cast<ossim_uint32>(50), maxLen);
         if (maxEdgeSample < 1)
            maxEdgeSample = 1;
         ossim_uint32 testEdgeSample = ossim::round<ossim_uint32>((maxLen * scale * mpp.y) / divisor);

         bool goodMatch = (testEdgeSample <= maxEdgeSample) || (testEdgeSample >= (maxLen >> 1));
         ossim_float64 steps = ossim::min(testEdgeSample, maxEdgeSample);

         ossim_uint32 finalSteps = ossim::round<ossim_uint32>(steps);
         if (finalSteps < 1)
            finalSteps = 1;
         if (igeom->getCrossesDateline())
         {
            if (finalSteps < maxEdgeSample)
               finalSteps = maxEdgeSample;
         }

         ivpt->getViewSegments(boundList, m_viewArea, finalSteps);
         if (boundList.size())
         {
            m_viewRect = boundList[0];
            ossim_uint32 idx = 0;

            if (goodMatch)
            {
               m_viewArea = boundList[idx];
            }
            else
            {
               m_viewArea = m_viewRect;
            }
            for (idx = 1; idx < boundList.size(); ++idx)
            {
               ossimIrect rectBounds = ossimIrect(boundList[idx]);
               m_viewRect = m_viewRect.combine(rectBounds);
               if (goodMatch)
               {
                  m_viewArea.add(ossimPolygon(boundList[idx]));
               }
               else
               {
                  m_viewArea.add(rectBounds);
               }
            } //
            if (!m_viewRect.hasNans())
            {
               m_rectsDirty = false;
            }
         } //END if boundList.size()
      }
      else if (m_ImageViewTransform.valid())
      {
         m_viewRect = m_ImageViewTransform->getImageToViewBounds(m_inputR0Rect);

         if (!m_viewRect.hasNans())
         {
            m_rectsDirty = false;
         }

         m_viewArea = m_viewRect;
      }
   }
   if(!m_viewRect.hasNans()&&m_ImageViewTransform)
   {
      ossimDpt result;
      m_ImageViewTransform->getViewToImageScale(result, m_viewRect.midPoint());
      m_averageViewToImageScale = (result.x+result.y)/2.0;
      m_averageViewToImageRLevelScale = log(m_averageViewToImageScale) / log(2);
   }
   if ( m_rectsDirty )
   {
      m_viewRect.makeNan();
   }
   
#if 0 /* Please leave for debug. */
   ossimNotify(ossimNotifyLevel_DEBUG)
      << "ossimImageRenderer::initializeBoundingRects() debug:\n"
      << "\ninput rect: " << m_inputR0Rect
      << "\nview rect:  " << m_viewRect << endl;
#endif
}

void ossimImageRenderer::initialize()
{
   // Call the base class initialize.
   // Note:  This will reset "theInputConnection" if it changed...
   ossimImageSourceFilter::initialize();

   deallocate();

   m_rectsDirty = true;

   // we will only do this if we are enabled for this could be expensive
   if (m_ImageViewTransform.valid() && !m_ImageViewTransform->isValid() && isSourceEnabled())
   {
      checkIVT(); // This can initialize bounding rects and clear dirty flag.
   }

   if ( m_rectsDirty )
   {
      initializeBoundingRects();
   }
}

void ossimImageRenderer::deallocate()
{
   m_Tile            = 0;
   m_BlankTile       = 0;
   m_TemporaryBuffer = 0;
}

void ossimImageRenderer::allocate()
{
   deallocate();

   if(theInputConnection)
   {
      m_Tile = ossimImageDataFactory::instance()->create(this, this);
      m_BlankTile  = ossimImageDataFactory::instance()->create(this, this);
      
      m_Tile->initialize();
   }
}

bool ossimImageRenderer::saveState(ossimKeywordlist& kwl,
                                   const char* prefix)const
{
   if(m_ImageViewTransform.valid())
   {
      ossimString newPrefix = ossimString(prefix) + ossimString("image_view_trans.");
      
      m_ImageViewTransform->saveState(kwl, newPrefix.c_str());
   }
   if(m_Resampler)
   {
      m_Resampler->saveState(kwl,
                              (ossimString(prefix)+"resampler.").c_str());
   }
   kwl.add(prefix, "max_levels_to_compute", m_MaxLevelsToCompute);
   kwl.add(prefix, "interpolation_error_threshold", m_interpErrorThreshold);

   return ossimImageSource::saveState(kwl, prefix);
}

bool ossimImageRenderer::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimImageRenderer::loadState entered..." << endl;
      
#ifdef OSSIM_ID_ENABLED
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "OSSIM_ID:  " << OSSIM_ID << endl;
#endif      
   }
   
   ossimString newPrefix = ossimString(prefix) + ossimString("image_view_trans.");
   bool result = ossimImageSourceFilter::loadState(kwl, prefix);
   
   if(m_Resampler)
   {
      m_Resampler->loadState(kwl, (ossimString(prefix)+"resampler.").c_str());
   }
   m_ImageViewTransform = 0;
   m_ImageViewTransform = ossimImageViewTransformFactory::instance()->createTransform(kwl, newPrefix.c_str());
   if(!m_ImageViewTransform)
   {
      m_ImageViewTransform = new ossimImageViewProjectionTransform;
   }
   const char* maxLevelsToCompute = kwl.find(prefix, "max_levels_to_compute");
   if(maxLevelsToCompute)
   {
      m_MaxLevelsToCompute = ossimString(maxLevelsToCompute).toUInt32();
   }

   const ossimString threshold = kwl.find(prefix, "interpolation_error_threshold");
   if(!threshold.empty())
   {
      m_interpErrorThreshold = threshold.toDouble();
   }

   return result;
}

void ossimImageRenderer::setImageViewTransform(ossimImageViewTransform* ivt)
{
   m_ImageViewTransform = ivt;
   
   m_rectsDirty = true; // Want to recompute bounding rects.
   
   if ( m_ImageViewTransform.valid() && !m_ImageViewTransform->isValid() && isSourceEnabled() )
   {
      checkIVT(); // This can initialize bounding rects and clear dirty flag.
   }
   
   if ( m_rectsDirty ) 
   {
      initializeBoundingRects();
   }
}

bool ossimImageRenderer::setView(ossimObject* baseObject)
{
   bool new_view_set = false;
   if(m_ImageViewTransform.valid())
   {
      new_view_set = m_ImageViewTransform->setView(baseObject);

      // If view changes reset the bounding rects.
      initializeBoundingRects();
   }
   return new_view_set;
}

ossimObject* ossimImageRenderer::getView()
{
   if(m_ImageViewTransform.valid())
   {
      return m_ImageViewTransform->getView();
   }
   return 0;
}

const ossimObject* ossimImageRenderer::getView()const
{
   if(m_ImageViewTransform.valid())
   {
      return m_ImageViewTransform->getView();
   }
   return 0;
}

void ossimImageRenderer::getValidImageVertices(vector<ossimIpt>& validVertices,
                                               ossimVertexOrdering ordering,
                                               ossim_uint32 resLevel)const
{
   if(theInputConnection&&m_ImageViewTransform.valid()&&m_ImageViewTransform->isValid())
   {
      theInputConnection->getValidImageVertices(validVertices, ordering, resLevel);
      if(isSourceEnabled())
      {
         ossim_uint32 inputSize = (ossim_uint32)validVertices.size();
         if(inputSize)
         {
            ossimDpt viewPt;
            ossim_uint32 idx = 0;
            // transform each point to the view
            for(idx = 0; idx < inputSize; ++idx)
            {
               m_ImageViewTransform->imageToView(validVertices[idx], viewPt);
               
               // round it to the nearest pixel
               //
               validVertices[idx] = ossimIpt(viewPt);
            }
         }
      }
   }
}

//*************************************************************************************************
// Returns the geometry associated with the image being served out of the renderer. This
// corresponds to the view geometry defined in theIVT.
//*************************************************************************************************
ossimRefPtr<ossimImageGeometry> ossimImageRenderer::getImageGeometry()
{
   ossimRefPtr<ossimImageGeometry> geom = 0;
   if (isSourceEnabled())
   {
      // Make sure the IVT was properly initialized
      if (m_ImageViewTransform.valid() && !m_ImageViewTransform->isValid())
         checkIVT();

      if(m_ImageViewTransform->isIdentity())
      {
         geom = theInputConnection->getImageGeometry();
      }
      else
      {

         ossimImageViewProjectionTransform* ivpt =
               dynamic_cast<ossimImageViewProjectionTransform*>(m_ImageViewTransform.get());
         if (ivpt)
         {
            // we need to return the right side since the geometry changed to a view geometry
            geom = ivpt->getViewGeometry();
         }
      }
   }
   else if (theInputConnection)
   {
      geom = theInputConnection->getImageGeometry();
   }
   return geom;
}

void ossimImageRenderer::connectInputEvent(ossimConnectionEvent& /* event */)
{
   theInputConnection = PTR_CAST(ossimImageSource, getInput(0));
   if(!m_ImageViewTransform.valid())
      m_ImageViewTransform  = new ossimImageViewProjectionTransform;
   
   checkIVT();
   initialize();
}

void ossimImageRenderer::disconnectInputEvent(ossimConnectionEvent& /* event */)
{
   ossimImageViewProjectionTransform* ivpt = 
                   dynamic_cast<ossimImageViewProjectionTransform*>(m_ImageViewTransform.get()); 
   if(ivpt)
      ivpt->setImageGeometry(0);
   
   theInputConnection = 0;
}

void ossimImageRenderer::propertyEvent(ossimPropertyEvent& /* event */)
{
   checkIVT();
   initialize();
}

void ossimImageRenderer::setProperty(ossimRefPtr<ossimProperty> property)
{
   ossimString tempName = property->getName();
   
   if((tempName == "Filter type")||
      (tempName == "filter_type"))
   {
      if(m_Resampler)
      {
         m_Resampler->setFilterType(property->valueToString());
      }
   }
   //   else if(tempName == "Blur factor")
   //     {
   //       if(m_Resampler)
   //    {
   //      m_Resampler->setBlurFactor(property->valueToString().toDouble());
   //    }
   //     }
   else
   {
      ossimImageSourceFilter::setProperty(property);
   }
}
      
ossimRefPtr<ossimProperty> ossimImageRenderer::getProperty(const ossimString& name)const
{
   ossimString tempName = name;
   
   if((tempName == "Filter type")||
      (tempName == "filter_type"))
   {
      std::vector<ossimString> filterNames;
      m_Resampler->getFilterTypes(filterNames);
      
      ossimStringProperty* stringProp = new ossimStringProperty("filter_type",
                        m_Resampler->getMinifyFilterTypeAsString(),
                        false,
                        filterNames);
      stringProp->clearChangeType();
      stringProp->setReadOnlyFlag(false);
      stringProp->setCacheRefreshBit();
      
      return stringProp;
   }
//   else if(tempName == "Blur factor")
//   {
//      ossimNumericProperty* numericProperty = new ossimNumericProperty("Blur factor",
//                                                                       ossimString::toString((double)m_Resampler->getBlurFactor()));
   
//      numericProperty->setConstraints(0.0, 50.0);
//      numericProperty->setNumericType(ossimNumericProperty::ossimNumericPropertyType_FLOAT64);
//      numericProperty->setCacheRefreshBit();
   
//      return numericProperty;
//   }
   
   return ossimImageSourceFilter::getProperty(name);
}

void ossimImageRenderer::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
  ossimImageSourceFilter::getPropertyNames(propertyNames);

  propertyNames.push_back("Filter type");
//  propertyNames.push_back("Blur factor");
}

//*************************************************************************************************
// Insures that a proper IVT is established.
//*************************************************************************************************
void ossimImageRenderer::checkIVT()
{
   if(!isSourceEnabled())
      return;

   // Check validity of the IVT:
   if (m_ImageViewTransform->isValid())
      return;

   // Detected uninitialized IVT. We are only concerned with projection IVTs (IVPTs) so 
   // make sure that's what we're working with:
   ossimImageViewProjectionTransform* ivpt = 
                   dynamic_cast<ossimImageViewProjectionTransform*>(m_ImageViewTransform.get()); 
   ossimImageSource* inputSrc = PTR_CAST(ossimImageSource, getInput(0));

   if(!ivpt || !inputSrc) 
      return; // nothing to do here yet.

   // Fetch the input image geometry from the IVPT to see if one needs to be established:
   ossimRefPtr<ossimImageGeometry> inputGeom = ivpt->getImageGeometry();
   if ( !inputGeom )
   {
      // Ask the input source for a geometry:
      inputGeom = inputSrc->getImageGeometry();
      if ( !inputGeom )
      {
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)<<"ossimImageRenderer::checkTransform() -- "
            "No input image geometry could be established for this renderer."<<endl;
         }
         return;
      }
      ivpt->setImageGeometry( inputGeom.get() );
      m_rectsDirty = true;
   }

   // Now check the output view geometry:
   ossimRefPtr<ossimImageGeometry> outputGeom = ivpt->getViewGeometry();
   if (!outputGeom)
   {
      ossimRefPtr<ossimImageGeometry> myOutGeom = new ossimImageGeometry;

      //---
      // If the input geometry sports a map projection instead of a 3D
      // projector, use the same map projection for the view.
      // 
      // Note: Don't use map projections with model transforms as they don't
      // allow for changing resolution.
      //---
      const ossimProjection*  inputProj = inputGeom->getProjection();
      const ossimMapProjection* mapProj = PTR_CAST(ossimMapProjection, inputProj);
      if (mapProj)
      {
         if (dynamic_cast<const ossimBilinearMapProjection *>(mapProj))
         {
            if(traceDebug())
            {
               ossimNotify(ossimNotifyLevel_WARN) << "ossimBilinearMapProjection output projection is not scalable.  Change to another map projection if you want to scale." << std::endl;
            }
         }
         ossimProjection* my_proj = PTR_CAST(ossimProjection, mapProj->dup());
         myOutGeom->setProjection(my_proj);
      }
      else 
      {
         // The input geometry uses a 3D projection, so let's default here to a
         // ossimEquDistCylProjection for the view:
         ossimMapProjection* myMapProj = new ossimEquDistCylProjection;
         ossimDpt meters = inputGeom->getMetersPerPixel();
         double GSD = (meters.x + meters.y)/2.0;
         meters.x = GSD;
         meters.y = GSD;
         if(inputProj)
         {
            //---
            // Update the map projection.  Since ossimMapProjection::setOrigin calls
            // ossimMapProjection::update we do that before setUlTiePoints as it in
            // turn calls setUlEastingNorthing(forward(gpt)) which depends on the orgin.
            // We'll keep the central meridian at 0.0 so as to not mess up the
            // Easting / Northings of the EquDistCylProjection.
            //---
            ossimGpt origin = inputProj->origin();
            origin.lon = 0.0;
            // myMapProj->setOrigin(inputProj->origin());
            myMapProj->setOrigin( origin );
            myMapProj->setUlTiePoints( inputProj->origin() );

         }
         myMapProj->setMetersPerPixel(meters);
         myOutGeom->setProjection(myMapProj);
      }
      
      // Set up our IVT with the new output geometry:
      ivpt->setViewGeometry(myOutGeom.get());

      // Must reinitialize bounding rects before calling ossimImageGeometry::setImageSize().
      initializeBoundingRects();

      // Set the size on the ossimImageGeometry.
      ossimIrect rect;
      getBoundingRect(rect, 0);
      myOutGeom->setImageSize( rect.size() );
   }

   if ( m_rectsDirty )
   {
      initializeBoundingRects();
   }
}

void ossimImageRenderer::getDecimationFactor(ossim_uint32 resLevel,
                    ossimDpt& result)const
{
   if(isSourceEnabled())
   {
      result = ossimDpt(1,1);
   }
   else
   {
      ossimImageSourceFilter::getDecimationFactor(resLevel,
                    result);
   }
}

void ossimImageRenderer::getDecimationFactors(vector<ossimDpt>& decimations)const
{
   if(isSourceEnabled())
   {
      decimations.push_back(ossimDpt(1,1));
   }
   else
   {
      ossimImageSourceFilter::getDecimationFactors(decimations);
   }
}

ossim_uint32 ossimImageRenderer::getNumberOfDecimationLevels()const
{
   if(isSourceEnabled())
   {
      return 1;
   }
   
   return ossimImageSourceFilter::getNumberOfDecimationLevels();
}

void ossimImageRenderer::stretchQuadOut(const ossimDpt& amount,
                                        ossimDpt& ul,
                                        ossimDpt& ur,
                                        ossimDpt& lr,
                                        ossimDpt& ll)
{
   ossimDpt upper  = ur - ul;
   ossimDpt right  = lr - ur;
   ossimDpt bottom = ll - lr;
   ossimDpt left   = ul - ll;
   
   upper = (upper*(1.0 / upper.length()));
   upper.x *= amount.x;
   upper.y *= amount.y;
   right = (right*(1.0 / right.length()));
   right.x *= amount.x;
   right.y *= amount.y;
   bottom = (bottom*(1.0 / bottom.length()));
   bottom.x *= amount.x;
   bottom.y *= amount.y;
   left = (left*(1.0 / left.length()));
   left.x *= amount.x;
   left.y *= amount.y;
   
   
   ul = ul - upper + left;
   ur = ur + upper - right;
   lr = lr + left - bottom;
   ll = ll - left + bottom;
}

ossimRefPtr<ossimImageData>  ossimImageRenderer::getTileAtResLevel(const ossimIrect& boundingRect,
                                                       ossim_uint32 resLevel)
{
   if(!theInputConnection)
   {
      m_BlankTile->setImageRectangle(boundingRect);
      
      return m_BlankTile;
   }
   
   ossim_uint32 levels = theInputConnection->getNumberOfDecimationLevels();
   
   // ossim_uint32 maxValue = (ossim_uint32)ossim::max((ossim_uint32)m_BoundingRect.width(),
   //                                            (ossim_uint32)m_BoundingRect.height());
   if(resLevel == 0)
   {
      return theInputConnection->getTile(boundingRect);
   }
   if(resLevel < levels)
   {
      if(theInputConnection)
      {
         return theInputConnection->getTile(boundingRect,
                                            resLevel);
      }
      else
      {
         return m_BlankTile;
      }
   }
   else if((resLevel - levels) < m_MaxLevelsToCompute)
   {
      // check to see how many decimations we must achiev for the
      // request
      //
      int decimations = (resLevel - (levels-1));
      ossimIpt tileSize(theInputConnection->getTileWidth(),
                        theInputConnection->getTileHeight());
      
      ossim_int32 multiplier = (1 << decimations);
      
      ossim_int32 xIndex = 0;
      ossim_int32 yIndex = 0;
      
      // adjust the tilesize so it at least will cover the multiplier
      // We will probably come up with something better later but for now
      // this will do.
      if(multiplier > tileSize.x)
      {
         tileSize.x = multiplier;
      }
      if(multiplier > tileSize.y)
      {
         tileSize.y = multiplier;
      }
      
      // set the rect that covers the requested RLevel to the valid RLevel. 
      // the valid RLevel is what is available on the input side.
      //
      ossimIrect requestedRectAtValidRLevel = boundingRect;
      requestedRectAtValidRLevel.stretchToTileBoundary(tileSize);
      ossimIrect mappedRequestedRect = requestedRectAtValidRLevel;
      requestedRectAtValidRLevel  = requestedRectAtValidRLevel*((double)multiplier);
      
      if(!m_TemporaryBuffer)
      {
         m_TemporaryBuffer = (ossimImageData*)m_BlankTile->dup();
         m_TemporaryBuffer->setImageRectangle(mappedRequestedRect);
         m_TemporaryBuffer->initialize();
      }
      else
      {
         m_TemporaryBuffer->setImageRectangle(mappedRequestedRect);
         m_TemporaryBuffer->makeBlank();
      }
      
     // ossim_uint32 totalCount   = ((requestedRectAtValidRLevel.lr().y-requestedRectAtValidRLevel.ul().y)*
   //                                (requestedRectAtValidRLevel.lr().x-requestedRectAtValidRLevel.ul().x));
      ossim_uint32 currentCount = 0;
      ossimIrect boundingRect = theInputConnection->getBoundingRect(levels-1);
      for(yIndex = requestedRectAtValidRLevel.ul().y;yIndex < requestedRectAtValidRLevel.lr().y; yIndex += tileSize.y)
      {
         for(xIndex = requestedRectAtValidRLevel.ul().x; xIndex < requestedRectAtValidRLevel.lr().x; xIndex+=tileSize.x)
         {
            ossimIrect request(xIndex,
                               yIndex,
                               xIndex + (tileSize.x-1),
                               yIndex + (tileSize.y-1));
            ossimRefPtr<ossimImageData> data = theInputConnection->getTile(request, levels-1);
            
            if(data.valid() && (data->getDataObjectStatus()!=OSSIM_EMPTY)&&
               data->getBuf()&&
               boundingRect.intersects(request))
            {
               switch(data->getScalarType())
               {
                  case OSSIM_UINT8:
                  {
                     resampleTileToDecimation((ossim_uint8)0,
                                              m_TemporaryBuffer,
                                              data,
                                              multiplier);
                     break;
                  }
                  case OSSIM_SINT16:
                  {
                     resampleTileToDecimation((ossim_sint16)0,
                                              m_TemporaryBuffer,
                                              data,
                                              multiplier);
                     break;
                  }
                  case OSSIM_UINT16:
                  case OSSIM_USHORT11:
                  case OSSIM_USHORT12:
                  case OSSIM_USHORT13:
                  case OSSIM_USHORT14:
                  case OSSIM_USHORT15:
                  {
                     resampleTileToDecimation((ossim_uint16)0,
                                              m_TemporaryBuffer,
                                              data,
                                              multiplier);
                     break;
                  }
                  case OSSIM_FLOAT32:
                  case OSSIM_NORMALIZED_FLOAT:
                  {
                     resampleTileToDecimation((ossim_float32)0,
                                              m_TemporaryBuffer,
                                              data,
                                              multiplier);
                     break;
                  }
                  case OSSIM_FLOAT64:
                  case OSSIM_NORMALIZED_DOUBLE:
                  {
                     resampleTileToDecimation((ossim_float64)0,
                                              m_TemporaryBuffer,
                                              data,
                                              multiplier);
                     break;
                  }
                  case OSSIM_SCALAR_UNKNOWN:
                  default:
                  {
                     break;
                  }
               }
            }
            ++currentCount;
         }
      }
      m_TemporaryBuffer->validate();
      return m_TemporaryBuffer;
   }

   return 0;
}

void ossimImageRenderer::setMaxLevelsToCompute(ossim_uint32 maxLevels)
{
   m_MaxLevelsToCompute = maxLevels;
}

ossim_uint32 ossimImageRenderer::getMaxLevelsToCompute()const
{
   return m_MaxLevelsToCompute;
}

template <class T>
void ossimImageRenderer::resampleTileToDecimation(T /* dummyVariable */,
                    ossimRefPtr<ossimImageData> result,
                    ossimRefPtr<ossimImageData> tile,
                    ossim_uint32 multiplier)
{
   if(tile->getDataObjectStatus() == OSSIM_EMPTY ||
      !tile->getBuf())
   {
      return;
   }
   
   ossim_int32 maxX     = (ossim_int32)tile->getWidth();
   ossim_int32 maxY     = (ossim_int32)tile->getHeight();
   ossim_int32 resultHeight = result->getHeight();
   ossim_int32* offsetX = new ossim_int32[maxX];
   ossim_int32* offsetY = new ossim_int32[maxY];
   ossim_int32 i        = 0;
   ossim_int32 resultWidth     = (ossim_int32)result->getWidth();
   ossimIpt tileOrigin   = tile->getOrigin();
   ossimIpt resultOrigin = result->getOrigin();
   
   // create a lookup table. that maps the tile to the result
   for(i = 0; i < maxX; ++i)
   {
      offsetX[i] = (i+tileOrigin.x)/(ossim_int32)multiplier - resultOrigin.x;
      if(offsetX[i] < 0 )
      {
         offsetX[i] = 0;
      }
      else if(offsetX[i] >= resultWidth)
      {
         offsetX[i] = resultWidth-1;
      }
   }
   for(i = 0; i < maxY; ++i)
   {
      offsetY[i] = ( ((i+tileOrigin.y)/(ossim_int32)multiplier) - resultOrigin.y);
      if(offsetY[i] < 0 )
      {
         offsetY[i] = 0;
      }
      else if(offsetY[i] >= resultHeight)
      {
         offsetY[i] = resultHeight-1;
      }
      offsetY[i] *= resultWidth;
   }
   
   if(tile->getDataObjectStatus() == OSSIM_FULL)
   {
      ossim_int32 numberOfBands = (ossim_int32)std::min(result->getNumberOfBands(),
                                                        tile->getNumberOfBands());
      ossim_int32 band = 0;
      for(band = 0; band < numberOfBands; ++band)
      {
         T* tileBuf   = static_cast<T*>(tile->getBuf(band));
         T* resultBuf = static_cast<T*>(result->getBuf(band));
         ossim_int32 dx = 0;
         ossim_int32 dy = 0;
         ossim_int32 boxAverageX = 0;
         ossim_int32 boxAverageY = 0;
         for(dy = 0; dy < maxY; dy+=multiplier)
         {
            for(dx = 0; dx < maxX; dx+=multiplier)
            {
               double sum = 0.0;
               for(boxAverageY = 0; 
                   ((boxAverageY < (ossim_int32)multiplier)&& 
                    ((boxAverageY+dy)<maxY)); ++boxAverageY)
               {
                  for(boxAverageX = 0; 
                      ((boxAverageX < (ossim_int32)multiplier)&& 
                       ((boxAverageX+dx)<maxX)); ++boxAverageX)
                  {
                     sum += tileBuf[((boxAverageY+dy)*maxX + boxAverageX + dx)];
                  }
               }
               sum /= (double)(multiplier*multiplier);
               resultBuf[ offsetX[dx] + offsetY[dy] ] = (T)sum;
            }
         }
      }
   }
   else
   {
      ossim_int32 numberOfBands = (ossim_int32)std::min(result->getNumberOfBands(),
                                                        tile->getNumberOfBands());
      ossim_int32 band = 0;
      for(band = 0; band < numberOfBands; ++band)
      {
         T* tileBuf   = static_cast<T*>(tile->getBuf(band));
         T* resultBuf = static_cast<T*>(result->getBuf(band));
         T tileBufNp  = static_cast<T>(tile->getNullPix(band));
         ossim_int32 dx = 0;
         ossim_int32 dy = 0;
         ossim_int32 boxAverageX = 0;
         ossim_int32 boxAverageY = 0;
         for(dy = 0; dy < maxY; dy+=multiplier)
         {
            for(dx = 0; dx < maxX; dx+=multiplier)
            {
               double sum = 0.0;
               if(tileBuf[((dy+(multiplier>>1))*maxX + dx+(multiplier>>1))] != tileBufNp)
               {
                  ossim_uint32 nullCount = 0;
                  for(boxAverageY = 0; 
                      ((boxAverageY < (ossim_int32)multiplier)&& 
                       ((boxAverageY+dy)<maxY)); ++boxAverageY)
                  {
                     for(boxAverageX = 0; 
                         ((boxAverageX < (ossim_int32)multiplier)&& 
                          ((boxAverageX+dx)<maxX)); ++boxAverageX)
                     {
                        T value = tileBuf[((boxAverageY+dy)*maxX + boxAverageX + dx)];
                        if(value != tileBufNp)
                        {
                           sum += value;
                        }
                        else
                        {
                           ++nullCount;
                        }
                     }
                  }
                  ossim_uint32 area = multiplier*multiplier;
                  sum /= (double)(area);
                  if(nullCount!= area)
                  {
                     resultBuf[ offsetX[dx] + offsetY[dy] ] = (T)sum;
                  }
               }
            }
         }
      }
      
   }
   
  delete [] offsetX;
  delete [] offsetY;
}

ossimString ossimImageRenderer::getLongName() const
{
   return ossimString("Image Renderer");
}

ossimString ossimImageRenderer::getShortName() const
{
   return ossimString("Image Renderer"); 
}

void ossimImageRenderer::refreshEvent(ossimRefreshEvent& event)
{
   ossimImageSourceFilter::refreshEvent(event);
   ossimImageSourceFilter::initialize(); // init connections
   if((event.getObject()!=this)&&
      (event.getRefreshType() & ossimRefreshEvent::REFRESH_GEOMETRY))
   {
      ossimRefPtr<ossimImageGeometry> inputGeom =
         theInputConnection?theInputConnection->getImageGeometry().get():0;
      if(inputGeom)
      {
         ossimImageViewProjectionTransform* ivpt = 
                         dynamic_cast<ossimImageViewProjectionTransform*>(m_ImageViewTransform.get()); 
         if(ivpt)
         {
            ivpt->setImageGeometry(inputGeom.get());
         }
      }
   }
   initialize();
}

void ossimImageRenderer::enableSource()
{
   if ( isSourceEnabled() == false )
   {
      setEnableFlag( true );
   }
}

void ossimImageRenderer::disableSource()
{
   if ( isSourceEnabled() )
   {
      setEnableFlag( false );
   }
}

void ossimImageRenderer::setEnableFlag(bool flag)
{
   if ( getEnableFlag() != flag )
   {
      ossimImageSourceFilter::setEnableFlag( flag );
      checkIVT();
      initialize();
   }
}

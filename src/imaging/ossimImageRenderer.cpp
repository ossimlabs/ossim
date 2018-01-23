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
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <iostream>
#include <stack>
// using namespace std;

#ifdef OSSIM_ID_ENABLED
static const char OSSIM_ID[] = "$Id: ossimImageRenderer.cpp 23663 2015-12-11 21:10:54Z dburken $";
#endif

static ossimTrace traceDebug("ossimImageRenderer:debug");

RTTI_DEF2(ossimImageRenderer, "ossimImageRenderer", ossimImageSourceFilter, ossimViewInterface);



void ossimImageRenderer::ossimRendererSubRectInfo::splitHorizontal(std::vector<ossimRendererSubRectInfo>& result)const
{
   ossimIrect vrect(m_Vul,
                    m_Vur,
                    m_Vlr,
                    m_Vll);
   ossim_int32 w  = vrect.width();
   // ossim_int32 h  = vrect.height();
   ossim_int32 w2 = w>>1;
   ossimIrect tempLeftRect(m_Vul.x, 
                           m_Vul.y, 
                           m_Vul.x+w2-1, 
                           m_Vlr.y);
   ossimIrect tempRightRect(tempLeftRect.ur().x+1, 
                            m_Vul.y, 
                            m_Vur.x, 
                            m_Vlr.y);


   ossimRendererSubRectInfo left(m_transform.get());
   ossimRendererSubRectInfo right(m_transform.get());

   left.m_viewBounds = m_viewBounds;
   right.m_viewBounds = m_viewBounds;

   left.m_Vul = tempLeftRect.ul();
   left.m_Vur = tempLeftRect.ur();
   left.m_Vlr = tempLeftRect.lr();
   left.m_Vll = tempLeftRect.ll();

   right.m_Vul = tempRightRect.ul();
   right.m_Vur = tempRightRect.ur();
   right.m_Vlr = tempRightRect.lr();
   right.m_Vll = tempRightRect.ll();

   left.transformViewToImage();
   right.transformViewToImage();

   if(left.imageIsNan())
   {
      if(left.m_viewBounds->intersects(left.getViewRect()))
      {
         result.push_back(left);
      }
   }
   else
   {
      result.push_back(left);
   }
   if(right.imageIsNan())
   {
      if(right.m_viewBounds->intersects(right.getViewRect()))
      {
         result.push_back(right);
      }
   }
   else
   {
      result.push_back(right);
   }
}

void ossimImageRenderer::ossimRendererSubRectInfo::splitVertical(std::vector<ossimRendererSubRectInfo>& result)const
{
   ossimIrect vrect(m_Vul,
                    m_Vur,
                    m_Vlr,
                    m_Vll);
   // ossim_int32 w  = vrect.width();
   ossim_int32 h  = vrect.height();
   ossim_int32 h2 = h>>1;
   ossimIrect tempTopRect(m_Vul.x, 
                          m_Vul.y, 
                          m_Vlr.x, 
                          m_Vul.y+h2-1);
   ossimIrect tempBottomRect(m_Vul.x, 
                             tempTopRect.lr().y+1, 
                             m_Vlr.x, 
                             m_Vlr.y);

   ossimRendererSubRectInfo top(m_transform.get());
   ossimRendererSubRectInfo bottom(m_transform.get());

   top.m_viewBounds    = m_viewBounds;
   bottom.m_viewBounds = m_viewBounds;

   top.m_Vul = tempTopRect.ul();
   top.m_Vur = tempTopRect.ur();
   top.m_Vlr = tempTopRect.lr();
   top.m_Vll = tempTopRect.ll();

   bottom.m_Vul = tempBottomRect.ul();
   bottom.m_Vur = tempBottomRect.ur();
   bottom.m_Vlr = tempBottomRect.lr();
   bottom.m_Vll = tempBottomRect.ll();

   top.transformViewToImage();
   bottom.transformViewToImage();

   if(top.imageIsNan())
   {
      if(top.m_viewBounds->intersects(top.getViewRect()))
      {
         result.push_back(top);
      }
   }
   else
   {
      result.push_back(top);
   }
   if(bottom.imageIsNan())
   {
      if(bottom.m_viewBounds->intersects(bottom.getViewRect()))
      {
         result.push_back(bottom);
      }
   }
   else
   {
      result.push_back(bottom);
   }
}

void ossimImageRenderer::ossimRendererSubRectInfo::splitAll(std::vector<ossimRendererSubRectInfo>& result)const
{
   //std::cout << "FULL Split\n" << std::endl;
   // let splitAll for now.  We can still optimize but will do that later
   ossimIrect tempUlRect;
   ossimIrect tempUrRect;
   ossimIrect tempLrRect;
   ossimIrect tempLlRect;
   ossimIrect vrect(m_Vul,
                    m_Vur,
                    m_Vlr,
                    m_Vll);
   ossim_int32 w  = vrect.width();
   ossim_int32 h  = vrect.height();
   ossim_int32 w2 = w>>1;
   ossim_int32 h2 = h>>1;

   tempUlRect = ossimIrect(m_Vul.x,
                           m_Vul.y,
                           m_Vul.x + (w2 - 1),
                           m_Vul.y + (h2 - 1));
  
   tempUrRect = ossimIrect(tempUlRect.ur().x+1,
                           m_Vul.y,
                           m_Vur.x,
                           m_Vul.y + (h2 - 1));
  
   tempLrRect = ossimIrect(tempUlRect.lr().x,
                           tempUlRect.lr().y+1,
                           m_Vlr.x,
                           m_Vlr.y);
  
   tempLlRect = ossimIrect(m_Vul.x,
                           tempUlRect.ll().y+1,
                           tempLrRect.ul().x,
                           tempLrRect.ll().y);

//  std::cout << "VR: " << vrect.width() << ", " << vrect.height() << "\n"
//            << "UL: " << tempUlRect.width() << ", " << tempUlRect.height() << "\n"
//            << "UR: " << tempUrRect.width() << ", " << tempUrRect.height() << "\n"
//            << "LR: " << tempLrRect.width() << ", " << tempLrRect.height() << "\n"
//            << "LL: " << tempLlRect.width() << ", " << tempLlRect.height() << "\n";

   ossimRendererSubRectInfo ul(m_transform.get(),tempUlRect.ul(), tempUlRect.ur(),tempUlRect.lr(), tempUlRect.ll());
   ossimRendererSubRectInfo ur(m_transform.get(),tempUrRect.ul(), tempUrRect.ur(),tempUrRect.lr(), tempUrRect.ll());
   ossimRendererSubRectInfo lr(m_transform.get(),tempLrRect.ul(), tempLrRect.ur(),tempLrRect.lr(), tempLrRect.ll());
   ossimRendererSubRectInfo ll(m_transform.get(),tempLlRect.ul(), tempLlRect.ur(),tempLlRect.lr(), tempLlRect.ll());

   ul.m_viewBounds = m_viewBounds;
   ur.m_viewBounds = m_viewBounds;
   lr.m_viewBounds = m_viewBounds;
   ll.m_viewBounds = m_viewBounds;

   ul.transformViewToImage();
   ur.transformViewToImage();
   lr.transformViewToImage();
   ll.transformViewToImage();

   if(ul.imageIsNan())
   {
      if(ul.m_viewBounds->intersects(ul.getViewRect()))
      {
         result.push_back(ul);
      }
   }
   else
   {
      result.push_back(ul);
   }
   if(ur.imageIsNan())
   {
      if(ur.m_viewBounds->intersects(ur.getViewRect()))
      {
         result.push_back(ur);
      }
   }
   else
   {
      result.push_back(ur);
   }
   if(lr.imageIsNan())
   {
      if(lr.m_viewBounds->intersects(lr.getViewRect()))
      {
         result.push_back(lr);
      }
   }
   else
   {
      result.push_back(lr);
   }
   if(ll.imageIsNan())
   {
      if(ll.m_viewBounds->intersects(ll.getViewRect()))
      {
         result.push_back(ll);
      }
   }
   else
   {
      result.push_back(ll);
   }
}

void ossimImageRenderer::ossimRendererSubRectInfo::splitView(std::vector<ossimRendererSubRectInfo>& result)const
{
   ossim_uint16 splitFlags = getSplitFlags();
   if(!splitFlags)
   {
      return;
   } 
  
   // just do horizontal split for test
   ossimIrect vrect(m_Vul,
                    m_Vur,
                    m_Vlr,
                    m_Vll);
   ossim_int32 w  = vrect.width();
   ossim_int32 h  = vrect.height();
   ossim_int32 w2 = w>>1;
   ossim_int32 h2 = h>>1;
   
   if((w2 <2)&&(h2<2))
   {
      if(splitFlags)
      {
         ossimRendererSubRectInfo rect(m_transform.get(),m_Vul, 
                                       m_Vul, 
                                       m_Vul, 
                                       m_Vul);
         rect.m_viewBounds = m_viewBounds;
         rect.transformViewToImage();

         if(rect.imageHasNans())
         {
            if(rect.m_viewBounds->intersects(rect.getViewRect()))
            {
               result.push_back(rect);
            }
         }
         // if(rect.imageIsNan())
         // {
         //   if(rect.m_viewBounds->intersects(rect.getViewRect()))
         //   {
         //     result.push_back(rect);
         //   }
         // }
         // else
         // {
         //   result.push_back(rect);
         // }
      }
   }
   // horizontal split if only the upper left and lower left 
   // vertices need splitting 
   else if((splitFlags==(UPPER_LEFT_SPLIT_FLAG|LOWER_LEFT_SPLIT_FLAG))||
           (splitFlags==(UPPER_RIGHT_SPLIT_FLAG|LOWER_RIGHT_SPLIT_FLAG)))
   {
      // std::cout << "Horizontal Split\n" << std::endl;    
      if(w > 1)
      {
         splitHorizontal(result);
      }
   }  
   // check vertical only split
   else if((splitFlags==(UPPER_LEFT_SPLIT_FLAG|UPPER_RIGHT_SPLIT_FLAG))||
           (splitFlags==(LOWER_RIGHT_SPLIT_FLAG|LOWER_LEFT_SPLIT_FLAG)))
   {
      //std::cout << "Vertical Split\n" << std::endl;

      if(h>1)
      {
         splitVertical(result);
      }
   }
   else//if((w>1)&&(h>1)&&(splitFlags))
   {
      if((w<2)&&(h>1))
      {
         splitVertical(result);
      }
      else if((w>1)&&(h<2))
      {
         splitHorizontal(result);
      }
      else
      {
         splitAll(result);
      }
   }
}

void ossimImageRenderer::ossimRendererSubRectInfo::transformImageToView()
{
   ossimDpt vul;
   ossimDpt vur;
   ossimDpt vlr;
   ossimDpt vll;
   m_transform->imageToView(m_Iul,
                          vul);
   m_transform->imageToView(m_Iur,
                          vur);
   m_transform->imageToView(m_Ilr,
                          vlr);
   m_transform->imageToView(m_Ill,
                          vll);
   
   m_Vul = vul;
   m_Vur = vur;
   m_Vlr = vlr;
   m_Vll = vll;
}

bool ossimImageRenderer::ossimRendererSubRectInfo::tooBig()const
{
  ossimDrect vRect = getViewRect();

  return ((vRect.width() > 32) || (vRect.height() > 32));
}

ossim_uint16 ossimImageRenderer::ossimRendererSubRectInfo::getSplitFlags()const
{
   ossim_uint16 result = SPLIT_NONE;
   ossimDrect vRect = getViewRect();

   //---
   // Don't allow splits beyond 8x8 pixel. ossim2dBilinearTransform was core dumping with
   // very small rectangles in canBilinearInterpolate(...) method.
   // DRB 05 Dec. 2017
   //---
   if ( vRect.width() < 8 && vRect.height() < 8 )
   {
      return result;
   }
  
   if(imageHasNans())
   {
      if(m_viewBounds->intersects(vRect))
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
     if(m_viewBounds->intersects(getViewRect()))
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
      ossim_float64 bias = m_ImageToViewScale.length();
      if(bias < 1.0) bias = 1.0/bias;
      bias = std::sqrt(bias);

      if(bias < 1) bias = 1.0;

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

      if((result!=SPLIT_ALL)&&!canBilinearInterpolate(bias))
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
}

void ossimImageRenderer::ossimRendererSubRectInfo::transformViewToImage()
{
//  std::cout << "TRANSFORM VIEW TO IMAGE!!!!!!!!!!!!!!\n";

   ossimDrect vrect = getViewRect();
   ossim_float64 w = vrect.width() - 1; // subtract 1 to prevent core dump in full-earth view rect
   ossim_float64 h = vrect.height();

   m_transform->viewToImage(m_Vul, m_Iul);
   m_transform->viewToImage(m_Vur, m_Iur);
   m_transform->viewToImage(m_Vlr, m_Ilr);
   m_transform->viewToImage(m_Vll, m_Ill);

//  m_ulRoundTripError = m_transform->getRoundTripErrorView(m_Vul);
//  m_urRoundTripError = m_transform->getRoundTripErrorView(m_Vur);
//  m_lrRoundTripError = m_transform->getRoundTripErrorView(m_Vlr);
//  m_llRoundTripError = m_transform->getRoundTripErrorView(m_Vll);

#if 1
   m_VulScale = computeViewToImageScale(m_Vul, ossimDpt( w, h));
   m_VurScale = computeViewToImageScale(m_Vur, ossimDpt(-w, h));
   m_VlrScale = computeViewToImageScale(m_Vlr, ossimDpt(-w,-h));
   m_VllScale = computeViewToImageScale(m_Vll, ossimDpt( w,-h));

   ossim_int32 n = 0;
   m_ViewToImageScale.x = 0.0;
   m_ViewToImageScale.y = 0.0;

   if(!m_VulScale.hasNans())
   {
      m_ViewToImageScale += m_VulScale; 
      ++n;
   }
   if(!m_VurScale.hasNans())
   {
      m_ViewToImageScale += m_VurScale; 
      ++n;
   }
   if(!m_VlrScale.hasNans())
   {
      m_ViewToImageScale += m_VlrScale; 
      ++n;
   }
   if(!m_VllScale.hasNans())
   {
      m_ViewToImageScale += m_VllScale; 
      ++n;
   }
   if(!n)
   {
      m_ViewToImageScale.makeNan();
   }
   else
   {
      m_ViewToImageScale.x/=n;
      m_ViewToImageScale.y/=n;
   }

#else
   {
      m_ViewToImageScale = ossimDpt(1.0, 1.0);
      
      ossimDpt topDelta    = m_Iur - m_Iul;
      ossimDpt rightDelta  = m_Ilr - m_Iur;
      ossimDpt bottomDelta = m_Ill - m_Ilr;
      ossimDpt leftDelta   = m_Iul - m_Ill;
      
      double topLen    = topDelta.length();
      double bottomLen = bottomDelta.length();
      double rightLen  = rightDelta.length();
      double leftLen   = leftDelta.length();
      
      double averageHoriz = ((topLen)  + (bottomLen))*.5;
      double averageVert  = ((leftLen) + (rightLen))*.5;
      
      ossimDpt deltaViewP1P2 = m_Vul - m_Vur;
      ossimDpt deltaViewP1P3 = m_Vul - m_Vll;
      
      double lengthViewP1P2 = deltaViewP1P2.length();//+1;
      double lengthViewP1P3 = deltaViewP1P3.length();//+1;

      if(lengthViewP1P2 > FLT_EPSILON)
         m_ViewToImageScale.x = averageHoriz/lengthViewP1P2;
      else
         m_ViewToImageScale.makeNan();
      if(lengthViewP1P3 > FLT_EPSILON)
         m_ViewToImageScale.y = averageVert/lengthViewP1P3;
      else
         m_ViewToImageScale.makeNan();
   }
#endif

   //std::cout << m_ViewToImageScale << std::endl;
   if(!m_ViewToImageScale.hasNans())
   {
      m_ImageToViewScale.x = 1.0/m_ViewToImageScale.x;
      m_ImageToViewScale.y = 1.0/m_ViewToImageScale.y;
   }
   else
   {
      m_ImageToViewScale.makeNan();
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
   ossimDpt topDelta    = m_Iur - m_Iul;
   ossimDpt rightDelta  = m_Ilr - m_Iur;
   ossimDpt bottomDelta = m_Ill - m_Ilr;
   ossimDpt leftDelta   = m_Iul - m_Ill;

   topDelta = topDelta*(1.0/topDelta.length());
   rightDelta = rightDelta*(1.0/rightDelta.length());
   bottomDelta = bottomDelta*(1.0/bottomDelta.length());
   leftDelta = leftDelta*(1.0/leftDelta.length());

   m_Iul = m_Iul + ((leftDelta - topDelta)*.5);
   m_Iur = m_Iur + ((topDelta - rightDelta)*.5);
   m_Ilr = m_Ilr + ((rightDelta - bottomDelta)*.5);
   m_Ill = m_Ill + ((bottomDelta - leftDelta)*.5);

   if(enableRound)
   {
      m_Iul = ossimIpt(ossim::round<int>(m_Iul.x),
                        ossim::round<int>(m_Iul.y));
      m_Iur = ossimIpt(ossim::round<int>(m_Iur.x),
                        ossim::round<int>(m_Iur.y));
      m_Ilr = ossimIpt(ossim::round<int>(m_Ilr.x),
                        ossim::round<int>(m_Ilr.y));
      m_Ill = ossimIpt(ossim::round<int>(m_Ill.x),
                        ossim::round<int>(m_Ill.y));
   }
}

bool ossimImageRenderer::ossimRendererSubRectInfo::isIdentity()const
{
//    ossimDpt deltaP1P2 = m_Iul - m_Iur;
//    ossimDpt deltaP1P3 = m_Iul - m_Ill;
   
//    ossimDpt deltaViewP1P2 = m_Vul - m_Vur;
//    ossimDpt deltaViewP1P3 = m_Vul - m_Vll;
   
//    bool horizontalSigns = ossimGetSign(deltaP1P2.x)==ossimGetSign(deltaViewP1P2.x);
//    bool verticalSigns   = ossimGetSign(deltaP1P3.y)==ossimGetSign(deltaViewP1P3.y);

//    // check first to see if any horizontal or vertical flipping
//    //
//    if(horizontalSigns && verticalSigns)
//    {
//       // check scales to see if they are 1
//       if(fabs(1-m_ViewToImageScale.x) <= FLT_EPSILON &&
//          fabs(1-m_ViewToImageScale.y) <= FLT_EPSILON)
//       {
//          return true;
//       }
//    }

    double iulDelta = (m_Iul-m_Vul).length();
    double iurDelta = (m_Iur-m_Vur).length();
    double ilrDelta = (m_Ilr-m_Vlr).length();
    double illDelta = (m_Ill-m_Vll).length();

    return ((iulDelta <= FLT_EPSILON)&&
            (iurDelta <= FLT_EPSILON)&&
            (ilrDelta <= FLT_EPSILON)&&
            (illDelta <= FLT_EPSILON));
}


bool ossimImageRenderer::ossimRendererSubRectInfo::canBilinearInterpolate(double error)const
{
  bool result = false;

      // now check point placement
  ossimDpt imageToViewScale = getAbsValueImageToViewScales();

  double testScale = imageToViewScale.length();

//  ossimDpt errorUl = transform->getRoundTripErrorView(m_Vul);
//  ossimDpt errorUr = transform->getRoundTripErrorView(m_Vur);
//  ossimDpt errorLr = transform->getRoundTripErrorView(m_Vlr);
//  ossimDpt errorLl = transform->getRoundTripErrorView(m_Vll);

//  if((errorUl.length() > 2 )||
//     (errorUr.length() > 2 )||
//     (errorLr.length() > 2 )||
//     (errorLl.length() > 2))
//     {
//        return result;
//     }
//  std::cout << "_______________________\n"
//            << "errorUl: " << errorUl << "\n"
//            << "errorUr: " << errorUr << "\n"
//            << "errorLr: " << errorLr << "\n"
//            << "errorLl: " << errorLl << "\n";

  // if there is a large shrink or expansion then just return true.
  // You are probably not worried about error in bilinear interpolation
  //
  if((testScale > 256)||
     (testScale < 1.0/256.0))
  {
     return true;
  }

  if(m_VulScale.hasNans()||
     m_VurScale.hasNans()||
     m_VlrScale.hasNans()||
     m_VllScale.hasNans())
  {
    return result;
  }

//  std::cout << "ulScale: " << m_VulScale << "\n"
//            << "urScale: " << m_VurScale << "\n"
//            << "lrScale: " << m_VlrScale << "\n"
//            << "llScale: " << m_VllScale << "\n";


  // check overage power of 2 variance
  // If there is a variance of 1 resolution level
  // then we must split further
  //
  ossim_float64 averageUlScale = m_VulScale.length();
  ossim_float64 averageUrScale = m_VurScale.length();
  ossim_float64 averageLrScale = m_VlrScale.length();
  ossim_float64 averageLlScale = m_VllScale.length();

  // std::cout << "_________________________\n";
  // std::cout << log(averageUlScale)/(log(2)) << "\n";
  // std::cout << log(averageUrScale)/(log(2)) << "\n";
  // std::cout << log(averageLrScale)/(log(2)) << "\n";
  // std::cout << log(averageLlScale)/(log(2)) << "\n";


  ossim_float64 ratio1 = averageUlScale/averageUrScale;
  ossim_float64 ratio2 = averageUlScale/averageLrScale;
  ossim_float64 ratio3 = averageUlScale/averageLlScale;

  // std::cout << "_________________________\n";
  // std::cout << "ratio1: " << ratio1 << "\n";
  // std::cout << "ratio2: " << ratio2 << "\n";
  // std::cout << "ratio3: " << ratio3 << "\n";

  
  // make sure all are within a power of 2 shrink or expand
  // which means the range of each ratio should be 
  // between .5 and 2
  result = (((ratio1 < 2) && (ratio1 > 0.5))&&
            ((ratio2 < 2) && (ratio2 > 0.5))&&
            ((ratio3 < 2) && (ratio3 > 0.5))); 

  //result = ((diff1<=2)&&(diff2<=2)&&(diff3<=2));
  //std::cout << "DIFF1: " << diff1 << std::endl;
  //std::cout << "DIFF2: " << diff2 << std::endl;
  //std::cout << "DIFF3: " << diff3 << std::endl;


  if(result)
  {
#if 1
    ossimDpt vUpper, vRight, vBottom, vLeft, vCenter;
    ossimDpt iUpper, iRight, iBottom, iLeft, iCenter;
    ossimDpt testUpper, testRight, testBottom, testLeft, testCenter;

    ossim2dBilinearTransform viewToImageTransform(m_Vul, m_Vur, m_Vlr, m_Vll
                                                 ,m_Iul, m_Iur, m_Ilr, m_Ill);

//    std::cout << "vMid:  " << testMid << "\n";
//    std::cout << "testMid:  " << testMid << "\n";
//    std::cout << "testCenter:  " << testCenter << "\n";
    
    getViewMids(vUpper, vRight, vBottom, vLeft, vCenter);
    
    // do a bilinear transform of some test points
    viewToImageTransform.forward(vUpper, iUpper);
    viewToImageTransform.forward(vRight, iRight);
    viewToImageTransform.forward(vBottom, iBottom);
    viewToImageTransform.forward(vLeft, iLeft);
    viewToImageTransform.forward(vCenter, iCenter);


   // viewToImageTransform.forward(vMid, iTestMid);
    //m_transform->viewToImage(vMid, testCenter);

    //getImageMids(iUpper, iRight, iBottom, iLeft, iCenter);
    
    // get the model centers for the mid upper left right bottom
    m_transform->viewToImage(vCenter, testCenter);
    if(testCenter.hasNans())
    {
       return false;
    }
    m_transform->viewToImage(vUpper, testUpper);
    if(testUpper.hasNans())
    {
       return false;
    }
    m_transform->viewToImage(vRight, testRight);
    if(testRight.hasNans())
    {
       return false;
    }
    m_transform->viewToImage(vBottom, testBottom);
    if(testBottom.hasNans())
    {
       return false;
    }
    m_transform->viewToImage(vLeft, testLeft);
    if(testLeft.hasNans())
    {
       return false;
    }

    // now get the model error to bilinear estimate of those points
    double errorCheck1 = (testCenter - iCenter).length();
    double errorCheck2 = (testUpper - iUpper).length();
    double errorCheck3 = (testRight - iRight).length();
    double errorCheck4 = (testBottom - iBottom).length();
    double errorCheck5 = (testLeft - iLeft).length();
    result = ((errorCheck1 < error)&&
              (errorCheck2 < error)&&
              (errorCheck3 < error)&&
              (errorCheck4 < error)&&
              (errorCheck5 < error));
    // if(!result)
    // {
       // std::cout <<"__________________________\n"
       //       << "ERROR1:" <<errorCheck1 << "\n" 
       //       << "ERROR2:" <<errorCheck2 << "\n" 
       //       << "ERROR3:" <<errorCheck3 << "\n" 
       //       << "ERROR4:" <<errorCheck4 << "\n" 
       //       << "ERROR5:" <<errorCheck5 << "\n"
       //       << "SENS:  " << error <<  "\n"; 

    //   std::cout << "Can't bilinear!!\n";
    // }

#else
    ossimDpt vUpper, vRight, vBottom, vLeft, vCenter;
    ossimDpt iUpper, iRight, iBottom, iLeft, iCenter;

    ossimDpt testCenter;
    getViewMids(vUpper, vRight, vBottom, vLeft, vCenter);
    getImageMids(iUpper, iRight, iBottom, iLeft, iCenter);

    ossimDpt iFullRes(iCenter.x*imageToViewScale.x,
          iCenter.y*imageToViewScale.y);

    m_transform->viewToImage(vCenter, testCenter);

    if(testCenter.hasNans())
    {
       return false;
    }
    ossimDpt testFullRes(testCenter.x*imageToViewScale.x,
             testCenter.y*imageToViewScale.y);

    double errorCheck1 = (testFullRes - iFullRes).length();

    iFullRes = ossimDpt(iUpper.x*imageToViewScale.x,
            iUpper.y*imageToViewScale.y);

    m_transform->viewToImage(vUpper, testCenter);
    if(testCenter.hasNans())
    {
       return false;
    }
    testFullRes = ossimDpt(testCenter.x*imageToViewScale.x,
         testCenter.y*imageToViewScale.y);
    double errorCheck2 = (testFullRes - iFullRes).length();

    iFullRes = ossimDpt(iRight.x*imageToViewScale.x,
            iRight.y*imageToViewScale.y);

    m_transform->viewToImage(vRight, testCenter);
    if(testCenter.hasNans())
    {
       return false;
    }
    testFullRes = ossimDpt(testCenter.x*imageToViewScale.x,
         testCenter.y*imageToViewScale.y);
    double errorCheck3 = (testFullRes - iFullRes).length();

    iFullRes = ossimDpt(iBottom.x*imageToViewScale.x,
            iBottom.y*imageToViewScale.y);

    m_transform->viewToImage(vBottom, testCenter);
    if(testCenter.hasNans())
    {
       return false;
    }
    testFullRes = ossimDpt(testCenter.x*imageToViewScale.x,
         testCenter.y*imageToViewScale.y);
    double errorCheck4 = (testFullRes - iFullRes).length();

    iFullRes = ossimDpt(iLeft.x*imageToViewScale.x,
            iLeft.y*imageToViewScale.y);

    m_transform->viewToImage(vLeft, testCenter);
    testFullRes = ossimDpt(testCenter.x*imageToViewScale.x,
         testCenter.y*imageToViewScale.y);
    double errorCheck5 = (testFullRes - iFullRes).length();

   std::cout <<"__________________________\n"
         << "ERROR1:" <<errorCheck1 << "\n" 
         << "ERROR2:" <<errorCheck2 << "\n" 
         << "ERROR3:" <<errorCheck3 << "\n" 
         << "ERROR4:" <<errorCheck4 << "\n" 
         << "ERROR5:" <<errorCheck5 << "\n"
         << "SENS:  " << error <<  "\n"; 

    result = ((errorCheck1 < error)&&
      (errorCheck2 < error)&&
      (errorCheck3 < error)&&
      (errorCheck4 < error)&&
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
  
  upperMid  = (m_Vul + m_Vur)*.5;
  rightMid  = (m_Vur + m_Vlr)*.5;
  bottomMid = (m_Vlr + m_Vll)*.5;
  leftMid   = (m_Vul + m_Vll)*.5;
  center    = (m_Vul + m_Vur + m_Vlr + m_Vll)*.25;
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
    upperMid  = (m_Iul + m_Iur)*.5;
    rightMid  = (m_Iur + m_Ilr)*.5;
    bottomMid = (m_Ilr + m_Ill)*.5;
    leftMid   = (m_Iul + m_Ill)*.5;
    center    = (m_Iul + m_Iur + m_Ilr + m_Ill)*.25;
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
:
ossimImageSourceFilter(),
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
m_MaxLevelsToCompute(999999) // something large so it will always compute
{
   ossimViewInterface::theObject = this;
   m_Resampler = new ossimFilterResampler();
   m_ImageViewTransform = new ossimImageViewProjectionTransform;
}

ossimImageRenderer::ossimImageRenderer(ossimImageSource* inputSource,
                                       ossimImageViewTransform* imageViewTrans)
   : ossimImageSourceFilter(inputSource),
     ossimViewInterface(0),
     m_Resampler(0),
     m_BlankTile(0),
     m_Tile(0),
     m_TemporaryBuffer(0),
     m_StartingResLevel(0),
     m_ImageViewTransform(imageViewTrans),
     m_inputR0Rect(),
     m_viewRect(),
     m_rectsDirty(true),
     m_MaxRecursionLevel(5),
     m_AutoUpdateInputTransform(true),
     m_MaxLevelsToCompute(999999) // something large so it will always compute
{
   ossimViewInterface::theObject = this;
   m_Resampler = new ossimFilterResampler();
   if(!m_ImageViewTransform.valid())
   {
      m_ImageViewTransform = new ossimImageViewProjectionTransform;
   }
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

ossimRefPtr<ossimImageData> ossimImageRenderer::getTile(
   const  ossimIrect& tileRect,
   ossim_uint32 resLevel)
{
   // std::cout << "_________________________\n";
   static const char MODULE[] = "ossimImageRenderer::getTile";
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " Requesting view rect = "
         << tileRect << endl;
   }

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
         return ossimImageSourceFilter::getTile(tileRect, resLevel);
      }
   }

   m_BlankTile->setImageRectangle(tileRect);
   
   if(!theInputConnection)
   {
      return m_BlankTile;
   }
   
   if ( !isSourceEnabled()||(!m_ImageViewTransform.valid())||
        (!m_ImageViewTransform->isValid()) )
   {
      // This tile source bypassed, return the input tile source.
      return theInputConnection->getTile(tileRect, resLevel);  
   }

   if( m_rectsDirty )
   {
      initializeBoundingRects();

      // We can't go on without these...
      if ( m_rectsDirty )
      {
         return m_BlankTile;
      }
   }
   
   if(m_viewRect.width() < 4 && m_viewRect.height() < 4)
   {
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

#if 0
   // expand a small patch just to alleviate errors in the size of the rect when resampling
     // ossimIrect viewRectClip = tileRect.clipToRect(ossimIrect(m_viewRect.ul() + ossimIpt(-8,-8),
     //                                                          m_viewRect.lr() + ossimIpt(8,8)));
   ossimIrect viewRectClip = tileRect.clipToRect(m_viewRect);
   std::cout << "_____________________" << std::endl;
   std::cout << "viewRectClip = " <<  viewRectClip << std::endl;
//   std::cout << "tileRect = " <<  tileRect << std::endl;
//   std::cout << "m_viewRect = " <<  m_viewRect << std::endl;
   ossimRendererSubRectInfo subRectInfo(viewRectClip.ul(),
                                        viewRectClip.ur(),
                                        viewRectClip.lr(),
                                        viewRectClip.ll());
#else
   ossimRendererSubRectInfo subRectInfo(m_ImageViewTransform.get(),
                                        tileRect.ul(),
                                        tileRect.ur(),
                                        tileRect.lr(),
                                        tileRect.ll());


#endif
   subRectInfo.m_viewBounds = &m_viewArea;
   subRectInfo.transformViewToImage();

   if((!m_viewArea.intersects(subRectInfo.getViewRect())))
//   if((!m_viewRect.intersects(subRectInfo.getViewRect())))
   {
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
   recursiveResample(m_Tile, subRectInfo, 1);
  
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
  std::stack<ossimRendererSubRectInfo> rectStack;
  rectStack.push(rectInfo);

  while(!rectStack.empty())
  {
    ossimRendererSubRectInfo currentRectInfo = rectStack.top();
    ossimIrect tempViewRect = currentRectInfo.getViewRect();
    rectStack.pop();
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
        std::vector<ossimRendererSubRectInfo> splitRects;
        currentRectInfo.splitView(splitRects);
        if(!splitRects.empty())
        {
          for(idx = 0; idx < splitRects.size();++idx)
          {
            if(m_viewArea.intersects(splitRects[idx].getViewRect()))
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
   if(!outputData.valid() || !outputData->getBuf() || rectInfo.imageHasNans())
   {
      return;
   }
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
   ossimDpt nul(rectInfo.m_Iul.x*closestScale,
                rectInfo.m_Iul.y*closestScale);
   ossimDpt nll(rectInfo.m_Ill.x*closestScale,
                rectInfo.m_Ill.y*closestScale);
   ossimDpt nlr(rectInfo.m_Ilr.x*closestScale,
                rectInfo.m_Ilr.y*closestScale);
   ossimDpt nur(rectInfo.m_Iur.x*closestScale,
                rectInfo.m_Iur.y*closestScale);
   
   m_Resampler->getKernelSupport( kernelSupportX, kernelSupportY );
   
   ossimDrect boundingRect = ossimDrect( nul, nll, nlr, nur );
   

   boundingRect = ossimIrect((ossim_int32)floor(boundingRect.ul().x - (kernelSupportX)-.5),
                             (ossim_int32)floor(boundingRect.ul().y - (kernelSupportY)-.5),
                             (ossim_int32)ceil (boundingRect.lr().x + (kernelSupportX)+.5),
                             (ossim_int32)ceil (boundingRect.lr().y + (kernelSupportY)+.5));
   
   ossimDrect requestRect = boundingRect;
   
   ossimRefPtr<ossimImageData> data = getTileAtResLevel(requestRect, resLevel);
   
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
   }
   
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
   m_rectsDirty      = true;
   ossimImageViewProjectionTransform* ivpt = 
                   dynamic_cast<ossimImageViewProjectionTransform*>(m_ImageViewTransform.get()); 
   if(!theInputConnection||!m_ImageViewTransform.valid()) return;
   m_inputR0Rect = theInputConnection->getBoundingRect(0);
   if (!m_inputR0Rect.hasNans() )
   {
     if(ivpt&&ivpt->getImageGeometry()&&ivpt->getViewGeometry())
     {
         // Little complicated but instead of always setting the edge walk
         // to a high number like 50 points per edge
         // we will look at the image to view scale change
         // and use that as a factor.  So as the image zooms out we
         // need fewer points to estimate the edge.
         //
         // ossim_uint32 idx;
         std::vector<ossimDrect> boundList;
         ossimImageGeometry* igeom = ivpt->getImageGeometry(); // look at projected meters
         ossimImageGeometry* vgeom = ivpt->getViewGeometry(); // look at projected meters
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

         if(!mpp.hasNans()&&!vmpp.hasNans())
         {
            scale = mpp.y/vmpp.y;
            if(scale > 1.0) scale = 1.0; 
         }  
         ossim_float64 mppTest = mpp.y;
         ossim_float64 divisor = mppTest; // default to 30 meter elevation
         if(mppTest < 500 )
         {
            if(mppTest >= 45 )
            {
               divisor = 90; // 90 meters
            }
         }
         else
         {
            divisor = 1000; // 1 kilometer
         }
         if(divisor < 30) divisor = 30.0;


         // now test to see if our edge walk is close to matching enough
         // points for a good match.
         //
         ossim_uint32 maxEdgeSample = ossim::min(static_cast<ossim_uint32>(50), maxLen); 
         if(maxEdgeSample < 1) maxEdgeSample = 1;
         ossim_uint32 testEdgeSample = ossim::round<ossim_uint32>((maxLen*scale*mpp.y)/divisor);

         bool goodMatch = (testEdgeSample<=maxEdgeSample)||(testEdgeSample>=(maxLen>>1));
         ossim_float64 steps = ossim::min(testEdgeSample, maxEdgeSample); 

         ossim_uint32 finalSteps = ossim::round<ossim_uint32>(steps);
         if(finalSteps<1) finalSteps=1;
         if(igeom->getCrossesDateline())
         {
            if(finalSteps < maxEdgeSample) finalSteps = maxEdgeSample;
         }

         ivpt->getViewSegments(boundList, m_viewArea, finalSteps);
         if(boundList.size())
         {
            m_viewRect   = boundList[0];
            ossim_uint32 idx = 0;

            if(goodMatch)
            {
               m_viewArea = boundList[idx];
            }
            else
            {
               m_viewArea = m_viewRect;
            }
            for(idx=1;idx<boundList.size();++idx)
            {
               ossimIrect rectBounds = ossimIrect(boundList[idx]);
               m_viewRect = m_viewRect.combine(rectBounds);
               if(goodMatch)
               {
                  m_viewArea.add(ossimPolygon(boundList[idx]));
               }
               else
               {
                  m_viewArea.add(rectBounds);
               }
            } //
            if(!m_viewRect.hasNans())
            {
               m_rectsDirty = false;
            } 
        } //END if boundList.size()
     }
     else if(m_ImageViewTransform.valid())
     {
       m_viewRect = m_ImageViewTransform->getImageToViewBounds(m_inputR0Rect);

       if(!m_viewRect.hasNans())
       {
          m_rectsDirty = false;
       } 

       m_viewArea = m_viewRect;
     }
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
   kwl.add(prefix,
           "max_levels_to_compute",
           m_MaxLevelsToCompute);
   
   return ossimImageSource::saveState(kwl, prefix);
}

bool ossimImageRenderer::loadState(const ossimKeywordlist& kwl,
                                   const char* prefix)
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
      m_Resampler->loadState(kwl,
                              (ossimString(prefix)+"resampler.").c_str());
   }
   m_ImageViewTransform = 0;
   m_ImageViewTransform = ossimImageViewTransformFactory::instance()->createTransform(kwl, newPrefix.c_str());
   if(!m_ImageViewTransform)
   {
      m_ImageViewTransform = new ossimImageViewProjectionTransform;
   }
   const char* maxLevelsToCompute = kwl.find(prefix,
                                             "max_levels_to_compute");
   if(maxLevelsToCompute)
   {
      m_MaxLevelsToCompute = ossimString(maxLevelsToCompute).toUInt32();
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

      ossimImageViewProjectionTransform* ivpt =
            dynamic_cast<ossimImageViewProjectionTransform*>(m_ImageViewTransform.get());
      if (ivpt)
      {
         // we need to return the right side since the geometry changed to a view geometry
         geom = ivpt->getViewGeometry();
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
      if (mapProj && !mapProj->hasModelTransform() )
      {
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
            //---
            myMapProj->setOrigin(inputProj->origin());
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

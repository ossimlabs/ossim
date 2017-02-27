//*****************************************************************************
// FILE: ossimImageViewProjectionTransform.cc
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Garrett Potts
//         Oscar Kramer (oscar@krameranalytic.com)
//
// DESCRIPTION: Contains declaration of ossimImageViewProjectionTransform.
//    This class provides an image to view transform that utilizes two
//    independent 2D-to-3D projections. Intended for transforming view to
//    geographic "world" space to input image space.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: ossimImageViewProjectionTransform.cpp 20489 2012-01-23 20:07:56Z dburken $
//
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimGeoPolygon.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimPolyArea2d.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <cmath>

RTTI_DEF1(ossimImageViewProjectionTransform,
          "ossimImageViewProjectionTransform",
          ossimImageViewTransform);

//*****************************************************************************
//  CONSTRUCTOR: ossimImageViewProjectionTransform
//*****************************************************************************
ossimImageViewProjectionTransform::ossimImageViewProjectionTransform
(  ossimImageGeometry* imageGeometry, ossimImageGeometry* viewGeometry)
:  m_imageGeometry(imageGeometry),
   m_viewGeometry(viewGeometry),
   m_crossesDateline(false)
{
}

//*****************************************************************************
//  CONSTRUCTOR: ossimImageViewProjectionTransform
//*****************************************************************************
ossimImageViewProjectionTransform::
ossimImageViewProjectionTransform(const ossimImageViewProjectionTransform& src)
: ossimImageViewTransform(src),
  m_imageGeometry(src.m_imageGeometry),
  m_viewGeometry(src.m_viewGeometry),
  m_crossesDateline(false)
{
}

//*****************************************************************************
//  DESTRUCTOR: ~ossimImageViewProjectionTransform
//*****************************************************************************
ossimImageViewProjectionTransform::~ossimImageViewProjectionTransform()
{
}
//! Assigns the geometry to use for output view. This object does NOT own the geometry.
void ossimImageViewProjectionTransform::setViewGeometry(ossimImageGeometry* g) 
{
  m_viewGeometry = g;

  initializeDatelineCrossing();
}   

//! Assigns the geometry to use for input image. This object does NOT own the geometry.
void ossimImageViewProjectionTransform::setImageGeometry(ossimImageGeometry* g) 
{ 
  m_imageGeometry = g; 
  initializeDatelineCrossing();
}  

//*****************************************************************************
//  Workhorse of the object. Converts image-space to view-space.
//*****************************************************************************
void ossimImageViewProjectionTransform::imageToView(const ossimDpt& ip, ossimDpt& vp) const
{
   // Check for same geometries on input and output (this includes NULL geoms):
   if (m_imageGeometry == m_viewGeometry)
   {
      vp = ip;
      return;
   }
   
   // Otherwise we need access to good geoms. Check for a bad geometry object:
   if (!m_imageGeometry || !m_viewGeometry)
   {
      vp.makeNan();
      return;
   }
   
   // Check for same projection on input and output sides to save projection to ground:
   if (m_imageGeometry->getProjection() == m_viewGeometry->getProjection())
   {
      // Check for possible same 2D transforms as well:
      if ( (m_imageGeometry->getTransform() == m_viewGeometry->getTransform()) &&
           (m_imageGeometry->decimationFactor(0) == m_viewGeometry->decimationFactor(0)) )
      {
         vp = ip;
         return;
      }
      
      // Not the same 2D transform, so just perform local-image -> full-image -> local-view:
      ossimDpt fp;
      m_imageGeometry->rnToFull(ip, 0, fp);
      m_viewGeometry->fullToRn(fp, 0, vp);
      return;
   }
   
   //---
   // Completely different left and right side geoms (typical situation).
   // Need to project to ground.
   //---
   ossimGpt gp;
   m_imageGeometry->localToWorld(ip, gp);
   m_viewGeometry->worldToLocal(gp, vp);

#if 0 /* Please leave for debug. */
   cout <<"DEBUG ossimImageViewProjectionTransform::imageToView:"
        <<"\n    ip: "<<ip
        <<"\n    gp: "<<gp
        <<"\n    vp: "<<vp<<std::endl;
#endif
}

//*****************************************************************************
//  Other workhorse of the object. Converts view-space to image-space.
//*****************************************************************************
void ossimImageViewProjectionTransform::viewToImage(const ossimDpt& vp, ossimDpt& ip) const
{
   // Check for same geometries on input and output (this includes NULL geoms):
   if (m_imageGeometry == m_viewGeometry)
   {
      ip = vp;
      return;
   }
   
   // Otherwise we need access to good geoms. Check for a bad geometry object:
   if (!m_imageGeometry || !m_viewGeometry)
   {
      ip.makeNan();
      return;
   }
   
   // Check for same projection on input and output sides to save projection to ground:
   const ossimProjection* iproj = m_imageGeometry->getProjection();
   const ossimProjection* vproj = m_viewGeometry->getProjection();
   if ((iproj && vproj && iproj->isEqualTo(*vproj)) || (iproj == vproj))
   {
      // Check for possible same 2D transforms as well:
      const ossim2dTo2dTransform* ixform = m_imageGeometry->getTransform();
      const ossim2dTo2dTransform* vxform = m_viewGeometry->getTransform();
      if (((ixform && vxform && ixform->isEqualTo(*vxform)) || (ixform == vxform)) &&
          (m_imageGeometry->decimationFactor(0) == m_viewGeometry->decimationFactor(0)))
      {
         ip = vp;
         return;
      }
      
      // Not the same 2D transform, so just perform local-image -> full-image -> local-view:
      ossimDpt fp;
      m_viewGeometry->rnToFull(vp, 0, fp);
      m_imageGeometry->fullToRn(fp, 0, ip);
      return;
   }
   
   //---
   // Completely different left and right side geoms (typical situation).
   // Need to project to ground.
   //---
   ossimGpt gp;
   m_viewGeometry->localToWorld(vp, gp);
   m_imageGeometry->worldToLocal(gp, ip);

#if 0 /* Please leave for debug. */
   cout <<"DEBUG ossimImageViewProjectionTransform::viewToImage:"
        <<"\n    vp: "<<vp
        <<"\n    gp: "<<gp
        <<"\n    ip: "<<ip
        <<std::endl;
#endif
}

void ossimImageViewProjectionTransform::getViewSegments(std::vector<ossimDrect>& viewBounds, 
                                                      ossimPolyArea2d& polyArea,
                                                      ossim_uint32 numberOfEdgePoints)const
{
  ossimDrect imageRect;
  ossimDrect worldRect(ossimDpt(-180,-90),
                       ossimDpt(-180,90),
                       ossimDpt(180,90),
                       ossimDpt(180,-90));
  viewBounds.clear();
  polyArea.clear();

  if(m_imageGeometry.valid())
  {
    m_imageGeometry->getBoundingRect(imageRect);
    ossim_uint32 idx=0;
    std::vector<ossimDpt> points;
    std::vector<ossimGpt> gPoints;
    bool affectedByElevation = m_imageGeometry->isAffectedByElevation();


    if((numberOfEdgePoints > 2)&&(affectedByElevation))
    {
          m_imageGeometry->getImageEdgePoints(points, numberOfEdgePoints);
    }
    else
    {
      points.resize(4);

      points[0] = imageRect.ul();
      points[1] = imageRect.ur();
      points[2] = imageRect.lr();
      points[3] = imageRect.ll();
    }
    for(idx=0; idx < points.size();++idx)
    {
      ossimGpt testGpt;
      m_imageGeometry->localToWorld(points[idx], testGpt); 

      if(!testGpt.isLatNan()&&!testGpt.isLonNan())
      {
        gPoints.push_back(testGpt);        
      }
    }
    if(m_crossesDateline)
    {
      ossimDpt testPt;
      ossimGpt cg;
      m_imageGeometry->localToWorld(imageRect.midPoint(), cg);
      ossim_int32 sgn = static_cast<ossim_int32>(ossim::sgn(cg.lond()));
      std::vector<ossimPolygon> polyList;

      // first we get the list of ground points initialized
      // and shifted to one side of the full world rect
      // We will do the other side next
      //
      for(idx=0; idx < gPoints.size();++idx)
      {
         if(std::fabs(gPoints[idx].lond()-cg.lond()) > 180)
         {
            gPoints[idx].lond(gPoints[idx].lond()+sgn*360);
         }
      }
      // now clip the ground list to the full ground rect
      //
      ossimPolygon tempPoly(gPoints);
      tempPoly.clipToRect(polyList, worldRect);

      ossim_uint32 pointListIdx=0;

      // Loop through the clipped polygons and find their 
      // view projection.  We will add this to the view
      // bounds and we will add to the poly Area.  Poly Area
      // is used for a tighter clip.
      //
      for(pointListIdx=0;pointListIdx<polyList.size();++pointListIdx)
      {
         const std::vector<ossimDpt>& clipPoints = polyList[pointListIdx].getVertexList();
         points.clear();
         for(idx = 0; idx<clipPoints.size(); ++idx)
         {
            m_viewGeometry->worldToLocal(ossimGpt(clipPoints[idx]), testPt);
            if(!testPt.hasNans())
            {
               points.push_back(testPt);
            }
         }
         viewBounds.push_back(ossimDrect(points));

         if (points.size() >= 4)
         {
            points.push_back(points[0]);
            polyArea.add(ossimPolyArea2d(points));//ossimPolygon(points)));
         }
      }

      // now shift the ground points to the other side of the world rect
      //
      for(idx=0; idx < gPoints.size();++idx)
      {
        gPoints[idx].lond(gPoints[idx].lond()+(-sgn*360));
      }

      // Now we will do the same thing to the other side of the world
      // and find the view projection and add those to the poly area
      // and the view rect
      //
      tempPoly = gPoints;
      polyList.clear();
      tempPoly.clipToRect(polyList, worldRect);

      for(pointListIdx=0;pointListIdx<polyList.size();++pointListIdx)
      {
         const std::vector<ossimDpt>& clipPoints = polyList[pointListIdx].getVertexList();
         points.clear();
         for(idx = 0; idx<clipPoints.size(); ++idx)
         {
            m_viewGeometry->worldToLocal(ossimGpt(clipPoints[idx]), testPt);
            if(!testPt.hasNans())
            {
               points.push_back(testPt);
            }
         }
         viewBounds.push_back(ossimDrect(points));
         if (points.size() >= 4)
         {
            points.push_back(points[0]);
            polyArea.add(ossimPolyArea2d(points));//ossimPolygon(points)));
         }

      }
    }// end: if(m_crossesDateline)
    else 
    {
      for(idx=0; idx < gPoints.size();++idx)
      {
        m_viewGeometry->worldToLocal(gPoints[idx], points[idx]);

      }
      viewBounds.push_back(ossimDrect(points));
      polyArea.add(ossimPolyArea2d(points));//ossimPolygon(points)));
    }
  } 
}


bool ossimImageViewProjectionTransform::setView(ossimObject* baseObject)
{
   ossimImageGeometry* g = dynamic_cast<ossimImageGeometry*>(baseObject);
   bool new_view_set = false;
   m_crossesDateline = false;
   if (g)
   {
      m_viewGeometry = g;
      new_view_set = true;
   }
   else
   {
      ossimProjection* proj = dynamic_cast<ossimProjection*>(baseObject);
      if(proj)
      {
         if(m_viewGeometry.valid())
         {
            m_viewGeometry->setProjection(proj);
         }
         else
         {
            m_viewGeometry = new ossimImageGeometry(0, proj);
         }

         new_view_set = true;
      }
   }

   if ( m_viewGeometry.valid() && m_viewGeometry-> getImageSize().hasNans() )
   {
      // Sets the view image size from the image geometry if present.
      initializeViewSize();
   } 

   return new_view_set;
}

//*****************************************************************************
//  Dumps contents to stream
//*****************************************************************************
std::ostream& ossimImageViewProjectionTransform::print(std::ostream& out)const
{
    out << "ossimImageViewProjectionTransform::print: ..... entered " <<endl;

    if(m_imageGeometry.valid())
    {
        out << "  Input Image (LEFT) Geometry: " << endl;
        m_imageGeometry->print(out);
    }
    else
    {
        out << "  None defined." << endl;
    }
    if(m_viewGeometry.valid())
    {
        out << "Output View (RIGHT) Geometry: " << endl;
        m_viewGeometry->print(out);
    }
    else
    {
        out << "  None defined." << endl;
    }
    return out;
}

//**************************************************************************************************
// Converts the local image space rect into bounding view-space rect
//**************************************************************************************************
ossimDrect ossimImageViewProjectionTransform::getImageToViewBounds(const ossimDrect& imageRect)const
{
    // Let base class try:
   ossimDrect result = ossimImageViewTransform::getImageToViewBounds(imageRect);

   // If not successful, compute using input and output geometries:
   if (result.hasNans() && m_imageGeometry.valid() && m_viewGeometry.valid() &&
       m_imageGeometry->hasProjection() && m_viewGeometry->hasProjection())
   {
      ossimGeoPolygon viewClip;
      m_viewGeometry->getProjection()->getGroundClipPoints(viewClip);
      if(viewClip.size())
      {
         std::vector<ossimGpt> imageGpts(4);
         m_imageGeometry->localToWorld(imageRect.ul(), imageGpts[0]);
         m_imageGeometry->localToWorld(imageRect.ur(), imageGpts[1]);
         m_imageGeometry->localToWorld(imageRect.lr(), imageGpts[2]);
         m_imageGeometry->localToWorld(imageRect.ll(), imageGpts[3]);

         const ossimDatum* viewDatum = m_viewGeometry->getProjection()->origin().datum();
         imageGpts[0].changeDatum(viewDatum);
         imageGpts[1].changeDatum(viewDatum);
         imageGpts[2].changeDatum(viewDatum);
         imageGpts[3].changeDatum(viewDatum);
         
         ossimPolyArea2d viewPolyArea(viewClip.getVertexList());
         ossimPolyArea2d imagePolyArea(imageGpts);
         viewPolyArea &= imagePolyArea;
         std::vector<ossimPolygon> visiblePolygons;
         viewPolyArea.getVisiblePolygons(visiblePolygons);
         if(visiblePolygons.size())
         {
            std::vector<ossimDpt> vpts;
            ossim_uint32 idx = 0;
            for(idx=0; idx<visiblePolygons[0].getNumberOfVertices();++idx)
            {
               ossimDpt tempPt;
               ossimGpt gpt(visiblePolygons[0][idx].lat, visiblePolygons[0][idx].lon, 0.0,  viewDatum);
               m_viewGeometry->worldToLocal(gpt, tempPt);
               vpts.push_back(tempPt);
            }
            result = ossimDrect(vpts);
         }
      }
   }
   return result;
}

//*****************************************************************************
//  METHOD: ossimImageViewProjectionTransform::loadState
//*****************************************************************************
bool ossimImageViewProjectionTransform::loadState(const ossimKeywordlist& kwl,
                                                  const char* prefix)
{
   bool result = ossimImageViewTransform::loadState(kwl, prefix);
   if(result)
   {
      ossimString imagePrefix = ossimString(prefix)+"image_geometry.";
      ossimString viewPrefix  = ossimString(prefix)+"view_geometry.";
      if(kwl.numberOf(imagePrefix.c_str())>0)
      {
         m_imageGeometry = new ossimImageGeometry();
         m_imageGeometry->loadState(kwl, imagePrefix.c_str());
      }
      if(kwl.numberOf(viewPrefix.c_str())>0)
      {
         m_viewGeometry = new ossimImageGeometry();
         m_viewGeometry->loadState(kwl, viewPrefix.c_str());
      }
      
   }
   
   return result;
}

//**************************************************************************************************
// 
//**************************************************************************************************
bool ossimImageViewProjectionTransform::saveState(ossimKeywordlist& kwl,
                                                  const char* prefix)const
{
   ossimString imagePrefix = ossimString(prefix)+"image_geometry.";
   ossimString viewPrefix  = ossimString(prefix)+"view_geometry.";
   
   if(m_imageGeometry.valid())
   {
      m_imageGeometry->saveState(kwl, imagePrefix.c_str());
   }
   if(m_viewGeometry.valid())
   {
      m_viewGeometry->saveState(kwl, viewPrefix.c_str());
   }
   return ossimImageViewTransform::saveState(kwl, prefix);
}

//**************************************************************************************************
// Returns the GSD of input image.
//**************************************************************************************************
ossimDpt ossimImageViewProjectionTransform::getInputMetersPerPixel() const
{
    ossimDpt result;

    if (m_imageGeometry.valid() && m_imageGeometry->hasProjection())
        result = m_imageGeometry->getProjection()->getMetersPerPixel();
    else
        result.makeNan();

    return result;
}

//**************************************************************************************************
// Returns the GSD of the output view.
//**************************************************************************************************
ossimDpt ossimImageViewProjectionTransform::getOutputMetersPerPixel() const
{
    ossimDpt result;

    if(m_viewGeometry.valid() && m_viewGeometry->hasProjection())
        result = m_viewGeometry->getProjection()->getMetersPerPixel();
    else
        result.makeNan();

    return result;
}

// Initialize view geometry image size from image geometry.
bool ossimImageViewProjectionTransform::initializeViewSize()
{
   bool result = false;
   m_crossesDateline = false;
   if ( m_imageGeometry.valid() )
   {
      m_crossesDateline = m_imageGeometry->getCrossesDateline();
      ossimDrect imageRect( 0, 0,
                            m_imageGeometry->getImageSize().x - 1,
                            m_imageGeometry->getImageSize().y - 1);
      ossimDpt size;
      ossimDrect rect;
      if(m_crossesDateline)
      {
         ossimGpt ulg;
         ossimGpt urg;
         ossimGpt lrg;
         ossimGpt llg;
         ossimGpt cg;
         ossimDpt ul;
         ossimDpt ur;
         ossimDpt lr;
         ossimDpt ll;
         m_imageGeometry->localToWorld(imageRect.ul(), ulg);
         m_imageGeometry->localToWorld(imageRect.ur(), urg);
         m_imageGeometry->localToWorld(imageRect.lr(), lrg);
         m_imageGeometry->localToWorld(imageRect.ll(), llg);
         m_imageGeometry->localToWorld(imageRect.midPoint(), cg);

         if(std::fabs(ulg.lond()-cg.lond()) > 180)
         {
            ulg.lond(ulg.lond()+ossim::sgn(cg.lond())*360);
         }
         if(std::fabs(urg.lond()-cg.lond()) > 180)
         {
            urg.lond((urg.lond()+ossim::sgn(cg.lond())*360));
         }
         if(std::fabs(lrg.lond()-cg.lond()) > 180)
         {
            lrg.lond(lrg.lond()+ossim::sgn(cg.lond())*360);
         }
         if(std::fabs(llg.lond()-cg.lond()) > 180)
         {
            llg.lond(llg.lond()+ossim::sgn(cg.lond())*360);
         }

         m_viewGeometry->worldToLocal(ulg, ul);
         m_viewGeometry->worldToLocal(urg, ur);
         m_viewGeometry->worldToLocal(lrg, lr);
         m_viewGeometry->worldToLocal(llg, ll);

         rect= ossimDrect(ul,ur,lr,ll);
      }
      else
      {
        rect = getImageToViewBounds( imageRect );
      }
      size.x = ossim::round<ossim_int32>( rect.width()  );
      size.y = ossim::round<ossim_int32>( rect.height() );
      if ( size.hasNans() == false )
      {
         m_viewGeometry->setImageSize( size );
         result = true;
      }
//      std::cout << "VIEW SIZE ============== " << size << "\n";
#if 0

      if ( (imageRect.hasNans() == false) && m_viewGeometry.valid() )
      {
         ossimRefPtr<ossimProjection> viewProj = m_viewGeometry->getProjection();
         if ( viewProj.valid() )
         {
            ossimIpt size;
            size.makeNan();
            
            const ossimEquDistCylProjection* eqProj =
               dynamic_cast<const ossimEquDistCylProjection*>( viewProj.get() );
            if ( eqProj )
            {
               // Specialized to take into consideration a date line crossing.
               
               // Get the ground points we need:
               ossimDpt iRight(imageRect.ul().x+1, imageRect.ul().y);
               ossimDpt iDown(imageRect.ul().x, imageRect.ul().y+1);
               ossimGpt gul;
               ossimGpt gur; 
               ossimGpt glr;
               ossimGpt gll;
               ossimGpt gRight;
               ossimGpt gDown;
               ossim_float64 hdd; // height decimal degrees
               ossim_float64 wdd; // width decimal degrees
               ossim_float64 leftLon;
               ossim_float64 rightLon;
               
               m_imageGeometry->localToWorld( imageRect.ul(), gul );
               m_imageGeometry->localToWorld( imageRect.ur(), gur );
               m_imageGeometry->localToWorld( imageRect.lr(), glr );
               m_imageGeometry->localToWorld( imageRect.ll(), gll );
               m_imageGeometry->localToWorld( iRight, gRight );
               m_imageGeometry->localToWorld( iDown, gDown );
               
               //---
               // Determine the direction of the image and get the geographic bounding box.
               // For the longitude consider a date line crossing.
               //---
               if ( gul.lat > gDown.lat ) // oriented north up
               {
                  if ( gul.lat >= gRight.lat ) // straight up or leaning right
                  {
                     hdd      = gul.lat - glr.lat;
                     leftLon  = gll.lon;
                     rightLon = gur.lon;
                  }
                  else // leaning left
                  {
                     hdd      = gur.lat - gll.lat;
                     leftLon  = gul.lon;
                     rightLon = glr.lon;
                  }
               }
               else // south or on side
               {
                  if ( gRight.lat >= gul.lat ) // straight down or leaning right
                  {
                     hdd      = glr.lat - gul.lat;
                     leftLon  = gur.lon;
                     rightLon = gll.lon;
                  }
                  else // leaning left
                  {
                     hdd      = gll.lat - gur.lat;
                     leftLon  = glr.lon;
                     rightLon = gul.lon;
                  }
               }
               
               if ( rightLon > leftLon )
               {
                  wdd = rightLon - leftLon;
               }
               else // Date line cross:
               {
                  wdd = rightLon + 360.0 - leftLon;
               }

               //---
               // Add one pixel width/height to the point to point size to make it edge to edge
               // before dividing by gsd to get the size.  The view projection gsd is used here
               // since the view could be a different resolution than the input projection.
               //---
               size.x = ossim::round<ossim_int32>(
                  ( wdd + eqProj->getDecimalDegreesPerPixel().x ) /
                  eqProj->getDecimalDegreesPerPixel().x );
               size.y = ossim::round<ossim_int32>(
                  ( hdd + eqProj->getDecimalDegreesPerPixel().y ) /
                  eqProj->getDecimalDegreesPerPixel().y );
               
#if 0 /* Please leave for debug: */
               cout << "\nwdd: " << wdd
                    << "\nhdd: " << hdd
                    << "\ngul: " << gul
                    << "\ngur: " << gur
                    << "\nglr: " << glr
                    << "\ngll: " << gll
                    << endl;
#endif
            }
            else // Not an ossimEquDistCylProjection:
            {
               ossimDrect rect = getImageToViewBounds( imageRect );
               size.x = ossim::round<ossim_int32>( rect.width()  );
               size.y = ossim::round<ossim_int32>( rect.height() );
            }

#if 0 /* Please leave for debug: */
            cout << "m_imageGeometry:\n" << *(m_imageGeometry.get())
                 << "\n\nm_viewGeometry:\n" << *(m_viewGeometry.get())
                 << "\n\ncomputed view size: " << size << endl;
#endif
            
            if ( size.hasNans() == false )
            {
               m_viewGeometry->setImageSize( size );
               result = true;
            }
            
         } // Matches: if ( viewProj.valid() )

      } // Matches: if ( (imageRect.hasNans() == false) ... 
#endif
      
   } // Matches: if ( m_imageGeometry.valid() && ...

   return result;
   
} // End:  bool ossimImageViewProjectionTransform::initializeViewSize()

void ossimImageViewProjectionTransform::initializeDatelineCrossing()
{
  m_crossesDateline = false;
  if(m_imageGeometry.valid())
  {
    m_crossesDateline = m_imageGeometry->getCrossesDateline();
  }
}
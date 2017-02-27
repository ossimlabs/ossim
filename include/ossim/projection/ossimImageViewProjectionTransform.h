//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Garrett Potts (gpotts@imagelinks.com)  
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
//  $Id: ossimImageViewProjectionTransform.h 20352 2011-12-12 17:24:52Z dburken $

#ifndef ossimImageViewProjectionTransform_HEADER
#define ossimImageViewProjectionTransform_HEADER 1

#include <ossim/projection/ossimImageViewTransform.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/base/ossimPolyArea2d.h>

class OSSIMDLLEXPORT ossimImageViewProjectionTransform : public ossimImageViewTransform
{
public:
   ossimImageViewProjectionTransform(ossimImageGeometry* imageGeometry=0,
                                     ossimImageGeometry* viewGeometry=0);

   //! copy constructor 
   ossimImageViewProjectionTransform(const ossimImageViewProjectionTransform& src);

   virtual ossimObject* dup() const { return new ossimImageViewProjectionTransform(*this); }
   virtual ~ossimImageViewProjectionTransform();

   //! Satisfies base class pure virtual. Returns TRUE if both input and output geoms exist.
   virtual bool isValid() const { return (m_imageGeometry.valid() && m_viewGeometry.valid()); }

   //! Returns TRUE if both input and output geometries are identical. Presently implemented as
   //! limited compare of geometry pointers
   virtual bool isIdentity() const { return (m_imageGeometry == m_viewGeometry); }

   //! Assigns the geometry to use for output view. This object does NOT own the geometry.
   void setViewGeometry(ossimImageGeometry* g);   

   //! Assigns the geometry to use for input image. This object does NOT own the geometry.
   void setImageGeometry(ossimImageGeometry* g);  

   //! Workhorse of the object. Converts image-space to view-space.
   virtual void imageToView(const ossimDpt& imagePoint, ossimDpt& viewPoint) const;

   //! Other workhorse of the object. Converts view-space to image-space.
   virtual void viewToImage(const ossimDpt& viewPoint, ossimDpt& imagePoint) const;

   //! Dumps contents to stream
   virtual std::ostream& print(std::ostream& out) const;
   
   ossimImageGeometry* getImageGeometry()  { return m_imageGeometry.get(); }
   ossimImageGeometry* getViewGeometry()   { return m_viewGeometry.get(); }
   const ossimImageGeometry* getImageGeometry()const  { return m_imageGeometry.get(); }
   const ossimImageGeometry* getViewGeometry()const   { return m_viewGeometry.get(); }
   
   /**
   * Because of the fact we can have dateline crossings there exist on the view
   * plane both positive and negative bounds.  We need a generalized interface that allows
   * us to calculate the valid bounds of an image transformed to the view.
   *
   *
   * @param viewBounds Returns bounds for each segment
   * @param polyArea   Returns the polygons for each segment.
   * @param numberOfEdgePoints If the value is 0 it will do a standard bounds check.  If
   *                           the paramter is greater than 0 this will only be used if the
   *                           input projection is affected by elevation.  If it's not affected
   *                           by elevation then the default bounds will be calculated.
   */
   virtual void getViewSegments(std::vector<ossimDrect>& viewBounds, 
                                ossimPolyArea2d& polyArea,
                                ossim_uint32 numberOfEdgePoints=0)const; 


   /**
   * This is used a a general access point for setting a view to a chain.  
   * There are multiple locations that need a projector in order to run properly.  
   * When a "Set View" event is sent through the chain.  Typically this is passed
   * down from The renderer.
   */
   virtual bool setView(ossimObject* baseObject);
   
   virtual       ossimObject* getView()       { return m_viewGeometry.get(); }
   virtual const ossimObject* getView() const { return m_viewGeometry.get(); }

   //! Returns the GSD of input image.
   virtual ossimDpt getInputMetersPerPixel()const;

   //! Returns the GSD of the output view.
   virtual ossimDpt getOutputMetersPerPixel() const;

   //! Gets the image bounding rect in view-space coordinates
   virtual ossimDrect getImageToViewBounds(const ossimDrect& imageRect)const;
   
   //! After rewrite for incorporating ossimImageGeometry: No longer needed.  
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix =0);
   
   //! After rewrite for incorporating ossimImageGeometry: No longer needed.  
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix = 0)const;

protected:

   /**
    * @brief Initializes the view geometry image size from image geometry
    * bounding rect.
    *
    * This is needed for the ossimImageGeometry::worldToLocal if the underlying
    * projection is geographic to handle images that cross the date line.
    * 
    * @param Input image rectangle.
    * @return true on success, false on error.
    */
   bool initializeViewSize();  
   void initializeDatelineCrossing();

   ossimRefPtr<ossimImageGeometry> m_imageGeometry;
   ossimRefPtr<ossimImageGeometry> m_viewGeometry;

   bool m_crossesDateline;
TYPE_DATA
};

#endif

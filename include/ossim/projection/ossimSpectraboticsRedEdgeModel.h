#ifndef ossimSpectraboticsRedEdgeModel_HEADER
#define ossimSpectraboticsRedEdgeModel_HEADER 1
#include <ossim/projection/ossimFcsiModel.h>
#include <ossim/projection/ossimMeanRadialLensDistortion.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimDpt3d.h>
#include <ossim/projection/ossimTangentialRadialLensDistortion.h>

class OSSIM_DLL ossimSpectraboticsRedEdgeModel : public ossimSensorModel
{
public:
   ossimSpectraboticsRedEdgeModel();
   ossimSpectraboticsRedEdgeModel(const ossimDrect& imageRect, // center in image space
                          const ossimGpt& platformPosition,
                          double roll,
                          double pitch,
                          double heading,
                          const ossimDpt& principalPoint, // in millimeters
                          double focalLength, // in millimeters
                          const ossimDpt& pixelSize); // in millimeters
   ossimSpectraboticsRedEdgeModel(const ossimSpectraboticsRedEdgeModel& src);
   virtual ossimObject* dup()const;
   
   virtual void imagingRay(const ossimDpt& image_point,
                           ossimEcefRay&   image_ray) const;

   void lineSampleToWorld(const ossimDpt& image_point,
                          ossimGpt&       gpt) const;
  
   
   virtual void lineSampleHeightToWorld(const ossimDpt& image_point,
                                        const double&   heightEllipsoid,
                                        ossimGpt&       worldPoint) const;
   virtual void worldToLineSample(const ossimGpt& world_point,
                                  ossimDpt&       image_point) const;
   
   virtual void updateModel();

   void setPrincipalPoint(ossimDpt principalPoint);

   virtual bool insideImage(const ossimDpt& p) const
   {
      /*          return( (p.u>=(0.0-FLT_EPSILON)) && */
      /*                  (p.u<=(double)(theImageSize.u-(1-FLT_EPSILON))) && */
      /*                  (p.v>=(0.0-FLT_EPSILON)) && */
      /*                  (p.v<=(double)(theImageSize.v-(1-FLT_EPSILON))) ); */
      // if it's close to the edge we will consider it inside the image
      //
      return theImageClipRect.pointWithin(p, theImageClipRect.width());
   }


   void setRollPitchHeading(double roll,
                            double pitch,
                            double heading);
   
   void setPixelSize(const ossimDpt& pixelSize);
   void setImageRect(const ossimDrect& rect);
   void setFocalLength(double focalLength);
   void setPlatformPosition(const ossimGpt& gpt);

   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0) const;
   
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   virtual void initAdjustableParameters();
   
   /*!
    * ossimOptimizableProjection
    */
//   inline virtual bool useForward()const {return true;} //!ground to image faster (you don't need DEM)
   inline virtual bool useForward()const {return false;} //!ground to image faster (you don't need DEM)
   virtual bool setupOptimizer(const ossimString& init_file); //!uses file path to init model

protected:
   
   NEWMAT::Matrix m_compositeMatrix;
   NEWMAT::Matrix m_compositeMatrixInverse;
   double         m_roll;
   double         m_pitch;
   double         m_heading;
   double         m_fov;
   ossimDpt       m_principalPoint;
   ossimDpt       m_calibratedCenter;
   ossimDpt       m_pixelSize;
   double         m_focalLength;
   double         m_norm; // for lens normalization
   double         m_focalX;
   double         m_focalY;
   ossimEcefPoint m_ecefPlatformPosition;
   ossimRefPtr<ossimTangentialRadialLensDistortion> m_lensDistortion;


   ossimEcefPoint m_adjEcefPlatformPosition;
   
TYPE_DATA
};

#endif
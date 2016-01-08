#include <ossim/projection/ossimSpectraboticsRedEdgeModel.h>
#include <ossim/base/ossimEllipsoid.h>
#include <ossim/base/ossimUnitConversionTool.h>
#include <ossim/base/ossimUnitTypeLut.h>
#include <ossim/base/ossimDatumFactory.h>
#include <ossim/base/ossimDatum.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimLsrSpace.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimGeoidManager.h>
#include <ossim/projection/ossimUtmProjection.h>
#include <ossim/support_data/ossimApplanixEOFile.h>
#include <ossim/base/ossimMatrix4x4.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/base/ossimDms.h>


#include <sstream>


static ossimTrace traceDebug("ossimSpectraboticsRedEdgeModel:debug");

RTTI_DEF1(ossimSpectraboticsRedEdgeModel, "ossimSpectraboticsRedEdgeModel", ossimSensorModel);

#ifdef OSSIM_ID_ENABLED
static const char OSSIM_ID[] = "$Id: ossimSpectraboticsRedEdgeModel.cpp 23562 2015-10-02 13:12:40Z gpotts $";
#endif

ossimSpectraboticsRedEdgeModel::ossimSpectraboticsRedEdgeModel()
{
   m_compositeMatrix          = ossimMatrix4x4::createIdentity();
   m_compositeMatrixInverse   = ossimMatrix4x4::createIdentity();
   m_roll                     = 0.0;
   m_pitch                    = 0.0;
   m_heading                  = 0.0;
   m_focalLength              = 5.5;
   m_pixelSize = ossimDpt(0.003, 0.003);
   m_ecefPlatformPosition = ossimGpt(0.0,0.0, 1000.0);
   m_adjEcefPlatformPosition = ossimGpt(0.0,0.0, 1000.0);
   theGSD.x   = 0.076;
   theGSD.y   = 0.076;
   theMeanGSD = 0.076;
   m_fov = 48.8; // degrees
   m_lensDistortion = new ossimTangentialRadialLensDistortion();
   initAdjustableParameters();

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimSpectraboticsRedEdgeModel::ossimSpectrabotics DEBUG:" << endl;
#ifdef OSSIM_ID_ENABLED
      ossimNotify(ossimNotifyLevel_DEBUG)<< "OSSIM_ID:  " << OSSIM_ID << endl;
#endif
   }
}
ossimSpectraboticsRedEdgeModel::ossimSpectraboticsRedEdgeModel(const ossimDrect& imageRect,
                                               const ossimGpt& platformPosition,
                                               double roll,
                                               double pitch,
                                               double heading,
                                               const ossimDpt& /* principalPoint */, // in millimeters
                                               double focalLength, // in millimeters
                                               const ossimDpt& pixelSize) // in millimeters
{
   theImageClipRect = imageRect;
   theRefImgPt      = theImageClipRect.midPoint();
   m_compositeMatrix          = ossimMatrix4x4::createIdentity();
   m_compositeMatrixInverse   = ossimMatrix4x4::createIdentity();
   m_roll                     = roll;
   m_pitch                    = pitch;
   m_heading                  = heading;
   m_focalLength              = focalLength;
   m_pixelSize                = pixelSize;
   m_ecefPlatformPosition     = platformPosition;
   m_adjEcefPlatformPosition  = platformPosition;
   m_lensDistortion           = new ossimTangentialRadialLensDistortion();
   initAdjustableParameters();
   updateModel();

   try
   {
      // Method throws ossimException.
     // computeGsd();
   }
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossimSpectrabotics Constructor caught Exception:\n"
         << e.what() << std::endl;
   }
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
      << "ossimSpectraboticsRedEdgeModel::ossimSpectrabotics DEBUG:" << endl;
#ifdef OSSIM_ID_ENABLED
      ossimNotify(ossimNotifyLevel_DEBUG)<< "OSSIM_ID:  " << OSSIM_ID << endl;
#endif
   }
}

ossimSpectraboticsRedEdgeModel::ossimSpectraboticsRedEdgeModel(const ossimSpectraboticsRedEdgeModel& src)
   :ossimSensorModel(src)
{
   initAdjustableParameters();
   
   if(src.m_lensDistortion.valid())
   {
      m_lensDistortion = new ossimTangentialRadialLensDistortion(*(src.m_lensDistortion.get()));
   }
   else
   {
      m_lensDistortion = new ossimTangentialRadialLensDistortion();
   }
}

ossimObject* ossimSpectraboticsRedEdgeModel::dup()const
{
   return new ossimSpectraboticsRedEdgeModel(*this);
}

void ossimSpectraboticsRedEdgeModel::imagingRay(const ossimDpt& image_point,
                                    ossimEcefRay&   image_ray) const
{
    if(traceDebug())
    {
       ossimNotify(ossimNotifyLevel_DEBUG) << "ossimSpectraboticsRedEdgeModel::imagingRay: ..... entered" << std::endl;
    }
    ossimDpt film (image_point.x-m_calibratedCenter.x,
                   m_calibratedCenter.y - image_point.y); //- theRefImgPt);
//    ossimDpt film (image_point-m_calibratedCenter); //- theRefImgPt);
    if(m_lensDistortion.valid())
    {
      ossimDpt tempFilm(film.x/m_norm, film.y/m_norm);
      ossimDpt filmOut;
      m_lensDistortion->undistort(tempFilm, filmOut);
      film.x = filmOut.x*m_norm;
      film.y = filmOut.y*m_norm;
    }
    film.x *= m_pixelSize.x; // pixel size on the film
    film.y *= m_pixelSize.y; // pixel size on the film
    ossimColumnVector3d cam_ray_dir (film.x,
                                     film.y,
                                     -m_focalLength);
    ossimEcefVector     ecf_ray_dir (m_compositeMatrix*cam_ray_dir);
    ecf_ray_dir = ecf_ray_dir*(1.0/ecf_ray_dir.magnitude());

    image_ray.setOrigin(m_adjEcefPlatformPosition);
    image_ray.setDirection(ecf_ray_dir);
}

void ossimSpectraboticsRedEdgeModel::lineSampleToWorld(const ossimDpt& image_point,
                                               ossimGpt&       gpt) const
{
   if (traceDebug())  ossimNotify(ossimNotifyLevel_DEBUG) << "DEBUG ossimSpectraboticsRedEdgeModel::lineSampleToWorld:entering..." << std::endl;
   
   if(image_point.hasNans())
   {
      gpt.makeNan();
      return;
   }
   //***
   // Extrapolate if image point is outside image:
   //***
//   if (!insideImage(image_point))
//   {
//      gpt.makeNan();
//       gpt = extrapolate(image_point);
//      return;
//   }

   //***
   // Determine imaging ray and invoke elevation source object's services to
   // intersect ray with terrain model:
   //***
   ossimEcefRay ray;
   imagingRay(image_point, ray);
   ossimElevManager::instance()->intersectRay(ray, gpt);

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << "image_point = " << image_point << std::endl;
      ossimNotify(ossimNotifyLevel_DEBUG) << "ray = " << ray << std::endl;
      ossimNotify(ossimNotifyLevel_DEBUG) << "gpt = " << gpt << std::endl;
   }

   if (traceDebug())  ossimNotify(ossimNotifyLevel_DEBUG) << "DEBUG ossimSensorModel::lineSampleToWorld: returning..." << std::endl;
   return;
}

void ossimSpectraboticsRedEdgeModel::lineSampleHeightToWorld(const ossimDpt& image_point,
                                                 const double&   heightEllipsoid,
                                                 ossimGpt&       worldPoint) const
{
//  if (!insideImage(image_point))
//   {
//      worldPoint.makeNan();
//       worldPoint = extrapolate(image_point, heightEllipsoid);
//   }
//   else
   {
      //***
      // First establish imaging ray from image point:
      //***
      ossimEcefRay ray;
      imagingRay(image_point, ray);
      ossimEcefPoint Pecf (ray.intersectAboveEarthEllipsoid(heightEllipsoid));
      worldPoint = ossimGpt(Pecf);
   }
}

void ossimSpectraboticsRedEdgeModel::worldToLineSample(const ossimGpt& world_point,
                                           ossimDpt&       image_point) const
{
   #if 0
   if((theBoundGndPolygon.getNumberOfVertices() > 0)&&
      (!theBoundGndPolygon.hasNans()))
   {
      if (!(theBoundGndPolygon.pointWithin(world_point)))
      {
//         image_point.makeNan();
//          image_point = extrapolate(world_point);
//         return;
      }         
   }
   #endif
   ossimEcefPoint g_ecf(world_point);
   ossimEcefVector ecfRayDir(g_ecf - m_adjEcefPlatformPosition);
   ossimColumnVector3d camRayDir (m_compositeMatrixInverse*ecfRayDir.data());
   
      
    double scale = -m_focalLength/camRayDir[2];
    ossimDpt film (scale*camRayDir[0], scale*camRayDir[1]);
    film.x /= m_pixelSize.x; // get into pixel coordinates
    film.y /= m_pixelSize.y;
    film.x /= m_norm; // normalize radial
    film.y /= m_norm;
    // now distort to find the true image coordinate location
    if (m_lensDistortion.valid())
    {
      ossimDpt filmOut;
      m_lensDistortion->distort(film, filmOut);
      film = filmOut;//+m_lensDistortion->getCenter();
    }
    film.x *= m_norm;
    film.y *= m_norm; 

    // invert Y to get back to left handed image space
    ossimDpt f1(film.x+m_calibratedCenter.x, m_calibratedCenter.y-film.y);
    
    image_point = f1;
}

void ossimSpectraboticsRedEdgeModel::updateModel()
{
   ossimGpt gpt;
   ossimGpt wgs84Pt;
   double metersPerDegree = wgs84Pt.metersPerDegree().x;
   double degreePerMeter = 1.0/metersPerDegree;
   double latShift = computeParameterOffset(1)*theMeanGSD*degreePerMeter;
   double lonShift = computeParameterOffset(0)*theMeanGSD*degreePerMeter;

   gpt = m_ecefPlatformPosition;
   double height = gpt.height();
   gpt.height(height + computeParameterOffset(5));
   gpt.latd(gpt.latd() + latShift);
   gpt.lond(gpt.lond() + lonShift);
   m_adjEcefPlatformPosition = gpt;
   ossimLsrSpace lsrSpace(m_adjEcefPlatformPosition);

   NEWMAT::Matrix heading = ossimMatrix4x4::createRotationZMatrix(m_heading+computeParameterOffset(4), OSSIM_RIGHT_HANDED);
   NEWMAT::Matrix roll = ossimMatrix4x4::createRotationYMatrix(m_roll+computeParameterOffset(2), OSSIM_RIGHT_HANDED);
   NEWMAT::Matrix pitch = ossimMatrix4x4::createRotationXMatrix(m_pitch+computeParameterOffset(3), OSSIM_RIGHT_HANDED);
   ossimMatrix4x4 lsrMatrix(lsrSpace.lsrToEcefRotMatrix());
   NEWMAT::Matrix orientation = roll*pitch*heading;
   m_compositeMatrix        = (lsrMatrix.getData()*orientation);
   m_compositeMatrixInverse = m_compositeMatrix.i();

   theBoundGndPolygon.resize(4);
   // ossim_float64 w = theImageClipRect.width()*2.0;
   // ossim_float64 h = theImageClipRect.height()*2.0;
   theExtrapolateImageFlag = false;
   theExtrapolateGroundFlag = false;


   lineSampleToWorld(theImageClipRect.ul(),gpt);//+ossimDpt(-w, -h), gpt);
   theBoundGndPolygon[0] = gpt;
   lineSampleToWorld(theImageClipRect.ur(),gpt);//+ossimDpt(w, -h), gpt);
   theBoundGndPolygon[1] = gpt;
   lineSampleToWorld(theImageClipRect.lr(),gpt);//+ossimDpt(w, h), gpt);
   theBoundGndPolygon[2] = gpt;
   lineSampleToWorld(theImageClipRect.ll(),gpt);//+ossimDpt(-w, h), gpt);
   theBoundGndPolygon[3] = gpt;
}

void ossimSpectraboticsRedEdgeModel::initAdjustableParameters()
{
   
   resizeAdjustableParameterArray(6);
   
   setAdjustableParameter(0, 0.0);
   setParameterDescription(0, "x_offset");
   setParameterUnit(0, "pixels");

   setAdjustableParameter(1, 0.0);
   setParameterDescription(1, "y_offset");
   setParameterUnit(1, "pixels");

   setAdjustableParameter(2, 0.0);
   setParameterDescription(2, "roll");
   setParameterUnit(2, "degrees");

   setAdjustableParameter(3, 0.0);
   setParameterDescription(3, "pitch");
   setParameterUnit(3, "degrees");

   setAdjustableParameter(4, 0.0);
   setParameterDescription(4, "heading");
   setParameterUnit(4, "degrees");

   setAdjustableParameter(5, 0.0);
   setParameterDescription(5, "altitude");
   setParameterUnit(5, "meters");
   
   
   setParameterSigma(0, 50.0);
   setParameterSigma(1, 50.0);
   setParameterSigma(2, 1);
   setParameterSigma(3, 1);
   setParameterSigma(4, 10);
   setParameterSigma(5, 50);
}

void ossimSpectraboticsRedEdgeModel::setPrincipalPoint(ossimDpt principalPoint)
{
   m_principalPoint = principalPoint;
}

void ossimSpectraboticsRedEdgeModel::setRollPitchHeading(double roll,
                                                 double pitch,
                                                 double heading)
{
   m_roll    = roll;
   m_pitch   = pitch;
   m_heading = heading;
   
   updateModel();
}

void ossimSpectraboticsRedEdgeModel::setPixelSize(const ossimDpt& pixelSize)
{
   m_pixelSize = pixelSize;
}

void ossimSpectraboticsRedEdgeModel::setImageRect(const ossimDrect& rect)
{
   theImageClipRect = rect;
   theRefImgPt = rect.midPoint();
}

void ossimSpectraboticsRedEdgeModel::setFocalLength(double focalLength)
{
   m_focalLength = focalLength;
}

void ossimSpectraboticsRedEdgeModel::setPlatformPosition(const ossimGpt& gpt)
{
   theRefGndPt            = gpt;
   m_ecefPlatformPosition = gpt;
   updateModel();
   
}

bool ossimSpectraboticsRedEdgeModel::saveState(ossimKeywordlist& kwl,
                                   const char* prefix) const
{
   ossimSensorModel::saveState(kwl, prefix);
   
   kwl.add(prefix, "type", "ossimSpectraboticsRedEdgeModel", true);

   kwl.add(prefix, "roll", m_roll, true);
   kwl.add(prefix, "pitch", m_pitch, true);
   kwl.add(prefix, "heading", m_heading, true);
   kwl.add(prefix, "principal_point", ossimString::toString(m_principalPoint.x) + " " + ossimString::toString(m_principalPoint.y));
   kwl.add(prefix, "pixel_size",      ossimString::toString(m_pixelSize.x) + " " + ossimString::toString(m_pixelSize.y));
   kwl.add(prefix, "focal_length", m_focalLength);
   kwl.add(prefix, "field_of_view", m_fov);
   kwl.add(prefix, "cx", m_calibratedCenter.x);
   kwl.add(prefix, "cy", m_calibratedCenter.y);
   kwl.add(prefix, "fx", m_focalX);
   kwl.add(prefix, "fy", m_focalY);
   kwl.add(prefix, "ecef_platform_position",
           ossimString::toString(m_ecefPlatformPosition.x()) + " " +
           ossimString::toString(m_ecefPlatformPosition.y()) + " " +
           ossimString::toString(m_ecefPlatformPosition.z()));

   if(m_lensDistortion.valid())
   {
      m_lensDistortion->saveState(kwl,
                                   prefix);
   }
   
   return true;
}

bool ossimSpectraboticsRedEdgeModel::loadState(const ossimKeywordlist& kwl,
                                       const char* prefix)
{
   if(traceDebug())
   {
      std::cout << "ossimSpectraboticsRedEdgeModel::loadState: ......... entered" << std::endl;
   }
   //ossimSensorModel::loadState(kwl,prefix);

   ossimSensorModel::loadState(kwl, prefix);
   if(getNumberOfAdjustableParameters() < 1)
   {

      initAdjustableParameters();
   }
   m_ecefPlatformPosition    = ossimGpt(0.0,0.0,1000.0);
   m_adjEcefPlatformPosition = ossimGpt(0.0,0.0,1000.0);
   m_roll    = 0.0;
   m_pitch   = 0.0;
   m_heading = 0.0;


   // bool computeGsdFlag = false;
   const char* roll              = kwl.find(prefix, "Roll");
   const char* pitch             = kwl.find(prefix, "Pitch");
   const char* heading           = kwl.find(prefix, "Yaw");
   const char* focalLength       = kwl.find(prefix, "Focal Length");
   const char* imageWidth        = kwl.find(prefix, "Image Width");
   const char* imageHeight       = kwl.find(prefix, "Image Height");
   const char* fov               = kwl.find(prefix, "Field Of View");
   const char* gpsPos            = kwl.find(prefix, "GPS Position");
   const char* gpsAlt            = kwl.find(prefix, "GPS Altitude");
   const char* imageCenter       = kwl.find(prefix, "Image Center");
   const char* fx                = kwl.find(prefix, "fx");
   const char* fy                = kwl.find(prefix, "fy");
   const char* cx                = kwl.find(prefix, "cx");
   const char* cy                = kwl.find(prefix, "cy");
   const char* k                 = kwl.find(prefix, "k");
   const char* p                 = kwl.find(prefix, "p");

   bool result = true;

#if 0
   std::cout << "roll: " << roll << "\n";
   std::cout << "pitch: " << pitch << "\n";
   std::cout << "heading: " << heading << "\n";
   std::cout << "focalLength: " << focalLength << "\n";
   std::cout << "imageWidth: " << imageWidth << "\n";
   std::cout << "imageHeight: " << imageHeight << "\n";
  // std::cout << "fov: " << fov << "\n";
   std::cout << "gpsPos: " << gpsPos << "\n";
   std::cout << "gpsAlt: " << gpsAlt << "\n";
   #endif
   //
  if(k&&p)
  {
    m_lensDistortion = new ossimTangentialRadialLensDistortion();
    m_lensDistortion->loadState(kwl, prefix);
  }

   if(roll&&
      pitch&&
      heading&&
      focalLength&&
      imageWidth&&
      imageHeight&&
      gpsPos&&
      gpsAlt)
   {
    std::cout << "DOING EXIF LOAD STATE!!!!!!!!!!!!!!\n";
      theSensorID = "MicaSense RedEdge";
      m_roll = ossimString(roll).toDouble();
      m_pitch = ossimString(pitch).toDouble();
      m_heading = ossimString(heading).toDouble();
      m_focalLength = ossimString(focalLength).toDouble();
      m_fov = fov?ossimString(fov).toDouble():48.8;
      theImageSize.x = ossimString(imageWidth).toDouble();
      theImageSize.y = ossimString(imageHeight).toDouble();


      theImageClipRect = ossimDrect(0,0,theImageSize.x-1,theImageSize.y-1);
      theRefImgPt      = ossimDpt(theImageSize.x/2.0, theImageSize.y/2.0);
     
      // now lets use the field of view and the focal length to 
      // calculate the pixel size on the ccd in millimeters
      double d = tan((m_fov*0.5)*M_PI/180.0)*m_focalLength;
      d*=2.0;
      double tempRadiusPixel = theImageSize.length();
      m_pixelSize.x = (d)/tempRadiusPixel;
      m_pixelSize.y = m_pixelSize.x;
      if(imageCenter)
      {
         std::vector<ossimString> splitString;
         ossimString tempString(imageCenter);
         tempString.split(splitString, ossimString(" "));
         if(splitString.size() == 2)
         {
            theRefImgPt = ossimDpt(splitString[0].toDouble(), splitString[1].toDouble());
         }
      }
      else
      {
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG) << "No Image Center found" << std::endl;
        //    result = false;
         }
      }

      // now extract the GPS position and shift it to the ellipsoidal height.
      std::vector<ossimString> splitArray;

      ossimString(gpsPos).split(splitArray, ",");
      splitArray[0] = splitArray[0].replaceAllThatMatch("deg", " ");
      splitArray[1] = splitArray[1].replaceAllThatMatch("deg", " ");

      ossimDms dmsLat;
      ossimDms dmsLon;
      double h = ossimString(gpsAlt).toDouble();
      dmsLat.setDegrees(splitArray[0]);
      dmsLon.setDegrees(splitArray[1]);
      double lat = dmsLat.getDegrees();
      double lon = dmsLon.getDegrees();

      h = h+ossimGeoidManager::instance()->offsetFromEllipsoid(ossimGpt(lat,lon));
      m_ecefPlatformPosition = ossimGpt(lat,lon,h);
      //double height1 = ossimElevManager::instance()->getHeightAboveEllipsoid(ossimGpt(lat, lon));
     // std::cout << m_ecefPlatformPosition << std::endl;
     // std::cout << "POINT: " << ossimGpt(lat,lon,h) << std::endl;
     // std::cout << "MSL:   " << height1 << "\n";

      theRefGndPt = m_ecefPlatformPosition;
      theRefGndPt.height(0.0);

     m_norm = ossim::nan();

    // lens parameters
    if(m_lensDistortion.valid()&&cx&&cy&&fx&&fy)
    {
      m_focalX = ossimString(fx).toDouble();
      m_focalY = ossimString(fy).toDouble();

      // our lens distorion assume center point.  So
      // lets shift to center and then set calibrated relative to 
      // image center.  We will then normalize.
      //
      ossimDpt focal(m_focalX,m_focalY);
      m_norm = focal.length()*0.5; // convert from diameter to radial
      m_calibratedCenter = ossimDpt(ossimString(cx).toDouble(), ossimString(cy).toDouble());
      m_principalPoint = m_calibratedCenter-theImageClipRect.midPoint();
      m_principalPoint.x /= m_norm;
      m_principalPoint.y /= m_norm;

      // lets initialize the root to be about one pixel norm along the diagonal
      // and the convergence will be approximately 100th of a pixel.
      //
      double temp = m_norm;
      if(temp < FLT_EPSILON) temp = 1.0;
      else temp = 1.0/temp;
      m_lensDistortion->setCenter(m_principalPoint);
      m_lensDistortion->setDxDy(ossimDpt(temp,temp));
      m_lensDistortion->setConvergenceThreshold(temp*0.001);
    }
    else
    {
      m_lensDistortion = 0;
    }
     updateModel();
   }
   else // load from regular save state
   {
      const char* principal_point   = kwl.find(prefix, "principal_point");
      const char* pixel_size        = kwl.find(prefix, "pixel_size");
      const char* ecef_platform_position = kwl.find(prefix, "ecef_platform_position");
      const char* latlonh_platform_position = kwl.find(prefix, "latlonh_platform_position");
      const char* compute_gsd_flag  = kwl.find(prefix, "compute_gsd_flag");
      roll              = kwl.find(prefix, "roll");
      pitch             = kwl.find(prefix, "pitch");
      heading           = kwl.find(prefix, "heading");
      fov               = kwl.find(prefix, "field_of_view");
      focalLength       = kwl.find(prefix, "focal_length");
    std::cout << "DOING regular LOAD STATE!!!!!!!!!!!!!!\n";

      if(roll)
      {
         m_roll = ossimString(roll).toDouble();
      }
      if(pitch)
      {
         m_pitch = ossimString(pitch).toDouble();
      }
      if(heading)
      {
         m_heading = ossimString(heading).toDouble();
      }

      if(cx&&cy)
      {
         m_calibratedCenter = ossimDpt(ossimString(cx).toDouble(), ossimString(cy).toDouble());
      }
      if(principal_point)
      {
         std::vector<ossimString> splitString;
         ossimString tempString(principal_point);
         tempString.split(splitString, ossimString(" "));
         if(splitString.size() == 2)
         {
            m_principalPoint.x = splitString[0].toDouble();
            m_principalPoint.y = splitString[1].toDouble();
         }
      }
      else
      {
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG) << "No principal_point found" << std::endl;
           // result = false;
         }
      }

      if(pixel_size)
      {
         std::vector<ossimString> splitString;
         ossimString tempString(pixel_size);
         tempString.split(splitString, ossimString(" "));
         if(splitString.size() == 1)
         {
            m_pixelSize.x = splitString[0].toDouble();
            m_pixelSize.y = m_pixelSize.x;
         }
         else if(splitString.size() == 2)
         {
            m_pixelSize.x = splitString[0].toDouble();
            m_pixelSize.y = splitString[1].toDouble();
         }
      }
      else
      {
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG) << "No pixel size found" << std::endl;
           // result = false;
         }
      }
      if(ecef_platform_position)
      {
         std::vector<ossimString> splitString;
         ossimString tempString(ecef_platform_position);
         tempString.split(splitString, ossimString(" "));
         if(splitString.size() > 2)
         {
            m_ecefPlatformPosition  = ossimEcefPoint(splitString[0].toDouble(),
                                                      splitString[1].toDouble(),
                                                      splitString[2].toDouble());
         }
      }
      else if(latlonh_platform_position)
      {
         std::vector<ossimString> splitString;
         ossimString tempString(latlonh_platform_position);
         tempString.split(splitString, ossimString(" "));
         std::string datumString;
         double lat=0.0, lon=0.0, h=0.0;
         if(splitString.size() > 2)
         {
            lat = splitString[0].toDouble();
            lon = splitString[1].toDouble();
            h = splitString[2].toDouble();
         }
         
         m_ecefPlatformPosition = ossimGpt(lat,lon,h);
      }
      if(focalLength)
      {
         m_focalLength = ossimString(focalLength).toDouble();
      }
      else
      {
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG) << "No focal length found" << std::endl;
            result = false;
         }
      }
      if(fov)
      {
         m_fov = ossimString(fov).toDouble();
      }
      else
      {
         if(traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG) << "No field of view found" << std::endl;
            result = false;
         }
      }
      theRefGndPt = m_ecefPlatformPosition;
    if(m_lensDistortion.valid()&&cx&&cy&&fx&&fy)
    {
      m_focalX = ossimString(fx).toDouble();
      m_focalY = ossimString(fy).toDouble();

      // our lens distorion assume center point.  So
      // lets shift to center and then set calibrated relative to 
      // image center.  We will then normalize.
      //
      ossimDpt focal(m_focalX,m_focalY);
      m_norm = focal.length()*0.5;
      m_calibratedCenter = ossimDpt(ossimString(cx).toDouble(), ossimString(cy).toDouble());
     // m_principalPoint = m_calibratedCenter-theImageClipRect.midPoint();
     // m_principalPoint.x /= m_norm;
     // m_principalPoint.y /= m_norm;

      // lets initialize the root to be about one pixel norm along the diagonal
      // and the convergence will be approximately 100th of a pixel.
      //
      double temp = m_norm;
      if(temp < FLT_EPSILON) temp = 1.0;
      else temp = 1.0/temp;
      m_lensDistortion->setCenter(m_principalPoint);
      m_lensDistortion->setDxDy(ossimDpt(temp,temp));
      m_lensDistortion->setConvergenceThreshold(temp*0.001);
    }
    else
    {
      m_lensDistortion = 0;
    }
      updateModel();
   }
    try
    {
       //---
       // This will set theGSD and theMeanGSD. Method throws
       // ossimException.
       //---
       computeGsd();
    }
    catch (const ossimException& e)
    {
       ossimNotify(ossimNotifyLevel_WARN)
          << "ossimSpectraboticsRedEdgeModel::loadState Caught Exception:\n"
          << e.what() << std::endl;
    }
    std::cout << "METERS PER PIXEL :    " << getMetersPerPixel() << std::endl;
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << std::setprecision(15) << std::endl;
      ossimNotify(ossimNotifyLevel_DEBUG) << "roll:     " << m_roll << std::endl
                                          << "pitch:    " << m_pitch << std::endl
                                          << "heading:  " << m_heading << std::endl
                                          << "platform: " << m_ecefPlatformPosition << std::endl
                                          << "latlon Platform: " << ossimGpt(m_ecefPlatformPosition) << std::endl
                                          << "focal len: " << m_focalLength << std::endl
                                          << "FOV      : " << m_fov << std::endl
                                          << "principal: " << m_principalPoint << std::endl
                                          << "Ground:    " << ossimGpt(m_ecefPlatformPosition) << std::endl;
   }

   return result;
}

bool ossimSpectraboticsRedEdgeModel::setupOptimizer(const ossimString& init_file)
{
   ossimKeywordlist kwl;
   kwl.addFile(init_file.c_str());

   return loadState(kwl);
}

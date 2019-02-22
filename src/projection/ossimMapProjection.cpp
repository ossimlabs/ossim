//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Base class for all map projections.
// 
//*******************************************************************
//  $Id: ossimMapProjection.cpp 23418 2015-07-09 18:46:41Z gpotts $

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimEpsgProjectionFactory.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimDatumFactoryRegistry.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimDatum.h>
#include <ossim/base/ossimEllipsoid.h>
#include <ossim/base/ossimString.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/base/ossimMatrix3x3.h>
#include <ossim/base/ossimUnitConversionTool.h>
#include <ossim/base/ossimUnitTypeLut.h>
#include <ossim/base/ossimTrace.h>

#define USE_MODEL_TRANSFORM 1

static ossimTrace traceDebug("ossimMapProjection:debug");

// RTTI information for the ossimMapProjection
RTTI_DEF1(ossimMapProjection, "ossimMapProjection" , ossimProjection);

ossimMapProjection::ossimMapProjection(const ossimEllipsoid& ellipsoid,
                                       const ossimGpt& origin)
   :theEllipsoid(ellipsoid),
    theOrigin(origin),
    theDatum(origin.datum()), // force no shifting
    theUlGpt(0, 0),
    theUlEastingNorthing(0, 0),
    theFalseEastingNorthing(0, 0),
    thePcsCode(0),
    theElevationLookupFlag(false),
    theModelTransform(),
    theInverseModelTransform(),
    theProjectionUnits(OSSIM_METERS),
    theImageToModelAzimuth(0)
{
   theModelTransform.setIdentity();
   theInverseModelTransform.setIdentity();
   theUlGpt = theOrigin;
   theUlEastingNorthing.makeNan();
   theMetersPerPixel.makeNan();
   theDegreesPerPixel.makeNan();
}

ossimMapProjection::ossimMapProjection(const ossimMapProjection& src)
      : ossimProjection(src),
        theEllipsoid(src.theEllipsoid),
        theOrigin(src.theOrigin),
        theDatum(src.theDatum),
        theMetersPerPixel(src.theMetersPerPixel),
        theDegreesPerPixel(src.theDegreesPerPixel),
        theUlGpt(src.theUlGpt),
        theUlEastingNorthing(src.theUlEastingNorthing),
        theFalseEastingNorthing(src.theFalseEastingNorthing),
        thePcsCode(src.thePcsCode),
        theElevationLookupFlag(false),
        theModelTransform(src.theModelTransform),
        theInverseModelTransform(src.theInverseModelTransform),
        theProjectionUnits(src.theProjectionUnits),
        theImageToModelAzimuth(src.theImageToModelAzimuth)
{
}

ossimMapProjection::~ossimMapProjection()
{
}

ossimGpt ossimMapProjection::origin()const
{
   return theOrigin;
}

void ossimMapProjection::setPcsCode(ossim_uint32 pcsCode)
{
   thePcsCode = pcsCode;
}

ossim_uint32 ossimMapProjection::getPcsCode() const
{
   // The PCS code is not always set when the projection is instantiated with explicit parameters,
   // since the code is only necessary when looking up those parameters in a database. However, it
   // is still necessary to recognize when an explicit projection coincides with an EPSG-specified
   // projection, and assign our PCS code to match it. So let's take this opportunity now to make 
   // sure the PCS code is properly initialized.
   if (thePcsCode == 0)
   {
      thePcsCode = ossimEpsgProjectionDatabase::instance()->findProjectionCode(*this);
      if (thePcsCode == 0)
         thePcsCode = 32767; // user-defined (non-EPSG) projection
   }
  
   if (thePcsCode == 32767)
      return 0; // 32767 only used internally. To the rest of OSSIM, the PCS=0 is undefined
   
   return thePcsCode;
}

ossimString ossimMapProjection::getProjectionName() const
{
   return getClassName();
}

double ossimMapProjection::getA() const
{
   return theEllipsoid.getA();
}

double ossimMapProjection::getB() const
{
   return theEllipsoid.getB();
}

double ossimMapProjection::getF() const
{
   return theEllipsoid.getFlattening();
}

ossimDpt ossimMapProjection::getMetersPerPixel() const
{
   return theMetersPerPixel;
}

const ossimDpt& ossimMapProjection::getDecimalDegreesPerPixel() const
{
   return theDegreesPerPixel;
}

const ossimDpt& ossimMapProjection::getUlEastingNorthing() const
{
   return theUlEastingNorthing;
}

const ossimGpt&   ossimMapProjection::getUlGpt() const
{
   return theUlGpt;
}

const ossimGpt& ossimMapProjection::getOrigin() const
{
  return theOrigin;
}

const ossimDatum* ossimMapProjection::getDatum() const
{
   return theDatum;
}

bool ossimMapProjection::isGeographic()const
{
   return false;
}

void ossimMapProjection::setEllipsoid(const ossimEllipsoid& ellipsoid)
{
   theEllipsoid = ellipsoid; update();
}

void ossimMapProjection::setAB(double a, double b)
{
   theEllipsoid.setA(a); theEllipsoid.setB(b); update();
}

void ossimMapProjection::setDatum(const ossimDatum* datum)
{

   if (!datum || (*theDatum == *datum))
      return;

   theDatum = datum; 
   theEllipsoid = *(theDatum->ellipsoid());

   // Change the datum of the ossimGpt data members:
   theOrigin.changeDatum(theDatum);
   theUlGpt.changeDatum(theDatum);

   update();

   // A change of datum usually implies a change of EPSG codes. Reset the PCS code. It will be
   // reestablished as needed in the getPcsCode() method:
   thePcsCode = 0;
}

void ossimMapProjection::setOrigin(const ossimGpt& origin)
{
   // Set the origin and since the origin has a datum which in turn has
   // an ellipsoid, sync them up.
   // NOTE: Or perhaps we need to change the datum of the input origin to that of theDatum? (OLK 05/11)
   theOrigin    = origin;
   theOrigin.changeDatum(theDatum);
      
   update();
}

void ossimMapProjection::assign(const ossimProjection &aProjection)
{
   if(&aProjection!=this)
   {
      ossimKeywordlist kwl;

      aProjection.saveState(kwl);
      loadState(kwl);
   }
}

void ossimMapProjection::update()
{
   // if the delta lat and lon per pixel is set then
   // check to see if the meters were set.
   //
   if (!theDegreesPerPixel.hasNans() && theMetersPerPixel.hasNans())
   {
      computeMetersPerPixel();
   }
   else if (!theMetersPerPixel.hasNans())
   {
      computeDegreesPerPixel();
   }
   // compute the tie points if not already computed
   //
   // The tiepoint was specified either as easting/northing or lat/lon. Need to initialize the one
   // that has not been assigned yet:
   if (theUlEastingNorthing.hasNans() && !theUlGpt.hasNans())
      theUlEastingNorthing = forward(theUlGpt);
   else if (theUlGpt.hasNans() && !theUlEastingNorthing.hasNans())
      theUlGpt = inverse(theUlEastingNorthing);
   else if (theUlGpt.hasNans() && theUlEastingNorthing.hasNans())
   {
      theUlGpt = theOrigin;
      theUlEastingNorthing = forward(theUlGpt);
   }
   if (theMetersPerPixel.hasNans() &&
       theDegreesPerPixel.hasNans())
   {
      ossimDpt mpd = ossimGpt().metersPerDegree();
      if (isGeographic())
      {
         theDegreesPerPixel.lat = 1.0 / mpd.y;
         theDegreesPerPixel.lon = 1.0 / mpd.x;
         computeMetersPerPixel();
      }
      else
      {
         theMetersPerPixel.x = 1.0;
         theMetersPerPixel.y = 1.0;
         computeDegreesPerPixel();
      }
   }

   // The last bit to do is the most important: Update the model transform so that we properly
   // convert between E, N and line, sample:
   updateTransform();
}

void ossimMapProjection::setModelTransform (const ossimMatrix4x4& transform)
{
   theModelTransform = transform;
   theInverseModelTransform = theModelTransform;
   theInverseModelTransform.i();
   updateFromTransform();
}

void ossimMapProjection::updateTransform()
{
   // Assumes model coordinates in meters:
   theModelTransform.setIdentity();
   NEWMAT::Matrix& m = theModelTransform.getData();

   double cosAz = 1.0, sinAz = 0.0;
   if (theImageToModelAzimuth != 0)
   {
      cosAz = ossim::cosd(theImageToModelAzimuth);
      sinAz = ossim::sind(theImageToModelAzimuth);
   }

   // Note that northing in the map projection is positive up, while in image space the y-axis
   // is positive is down, so apply that inversion by forcing theMetersPerPixel.y to be negative:
   // Scale and rotation:
   m[0][0] =  theMetersPerPixel.x * cosAz;   m[0][1] = -theMetersPerPixel.y * sinAz;
   m[1][0] = -theMetersPerPixel.x * sinAz;   m[1][1] = -theMetersPerPixel.y * cosAz;

   // Offset:
   m[0][3] = theUlEastingNorthing.x;
   m[1][3] = theUlEastingNorthing.y;

   theInverseModelTransform = theModelTransform;
   theInverseModelTransform.i();
}

void ossimMapProjection::updateFromTransform()
{
   // Extract scale, rotation and offset from the transform matrix:
   const NEWMAT::Matrix& m = theModelTransform.getData();
   theMetersPerPixel.x = sqrt(m[0][0]*m[0][0] + m[1][0]*m[1][0]);
   theMetersPerPixel.y = sqrt(m[1][0]*m[1][0] + m[1][1]*m[1][1]);
   theUlEastingNorthing.x = m[0][3];
   theUlEastingNorthing.y = m[1][3];
   theImageToModelAzimuth = ossim::acosd(m[0][0]/theMetersPerPixel.x);
   computeDegreesPerPixel();
}

void ossimMapProjection::applyScale(const ossimDpt& scale, bool recenterTiePoint)
{
   ossimDpt mapTieDpt;
   ossimGpt mapTieGpt;
   if (recenterTiePoint)
   {
      if (isGeographic())
      {
         mapTieGpt = getUlGpt();
         mapTieGpt.lat += theDegreesPerPixel.lat/2.0;
         mapTieGpt.lon -= theDegreesPerPixel.lon/2.0;
      }
      else
      {
         mapTieDpt = getUlEastingNorthing();
         mapTieDpt.x -= theMetersPerPixel.x/2.0;
         mapTieDpt.y += theMetersPerPixel.y/2.0;
      }
   }

   theDegreesPerPixel.x *= scale.x;
   theDegreesPerPixel.y *= scale.y;
   theMetersPerPixel.x  *= scale.x;
   theMetersPerPixel.y  *= scale.y;

   if ( recenterTiePoint )
   {
      if (isGeographic())
      {
         mapTieGpt.lat -= theDegreesPerPixel.lat/2.0;
         mapTieGpt.lon += theDegreesPerPixel.lon/2.0;
         setUlTiePoints(mapTieGpt);
      }
      else
      {
         mapTieDpt.x += theMetersPerPixel.x/2.0;
         mapTieDpt.y -= theMetersPerPixel.y/2.0;
         setUlTiePoints(mapTieDpt);
      }
   }

   updateTransform();
}

void ossimMapProjection::applyRotation(const double& azimuthDeg)
{
   theImageToModelAzimuth += azimuthDeg;
   if (theImageToModelAzimuth >= 360.0)
      theImageToModelAzimuth -= 360.0;

   updateTransform();
}

ossimDpt ossimMapProjection::worldToLineSample(const ossimGpt &worldPoint)const
{
   ossimDpt result;

   worldToLineSample(worldPoint, result);

   return result;
}

ossimGpt ossimMapProjection::lineSampleToWorld(const ossimDpt &lineSample)const
{
   ossimGpt result;

   lineSampleToWorld(lineSample, result);

   return result;
}

void ossimMapProjection::worldToLineSample(const ossimGpt &worldPoint,
                                           ossimDpt&       lineSample) const
{
   lineSample.makeNan();

   if(worldPoint.isLatLonNan())
      return;

   // Shift the world point to the datum being used by this projection, if defined:
   ossimGpt gpt = worldPoint;
   if ( theDatum )
      gpt.changeDatum(theDatum);

   // Transform world point to model coordinates using the concrete map projection equations:
   ossimDpt modelPoint = forward(gpt);

   // Now convert map model coordinates to image line/sample space:
   eastingNorthingToLineSample(modelPoint, lineSample);
}

void ossimMapProjection::lineSampleHeightToWorld(const ossimDpt &lineSample,
                                                 const double&  hgtEllipsoid,
                                                 ossimGpt&      gpt)const
{
   gpt.makeNan();

   // make sure that the passed in lineSample is good and
   // check to make sure our easting northing is good so
   // we can compute the line sample.
   if(lineSample.hasNans())
      return;

   // Transform image coordinates (line, sample) to model coordinates (easting, northing):
   ossimDpt modelPoint;
   lineSampleToEastingNorthing(lineSample, modelPoint);

   // Transform model coordinates to world point using concrete map projection equations:
   gpt = inverse(modelPoint);
   gpt.hgt = hgtEllipsoid;
}

void ossimMapProjection::lineSampleToWorld (const ossimDpt& lineSampPt,
                                            ossimGpt&       worldPt) const
{
   lineSampleHeightToWorld(lineSampPt, ossim::nan(), worldPt);
   if(theElevationLookupFlag)
      worldPt.hgt = ossimElevManager::instance()->getHeightAboveEllipsoid(worldPt);
}

void ossimMapProjection::lineSampleToEastingNorthing(const ossimDpt& lineSample,
                                                     ossimDpt&       eastingNorthing)const
{
#ifdef USE_MODEL_TRANSFORM
   // Transform according to 4x4 transform embedded in the projection:
   const NEWMAT::Matrix& m = theModelTransform.getData();
   eastingNorthing.x = m[0][0]*lineSample.x + m[0][1]*lineSample.y + m[0][3];
   eastingNorthing.y = m[1][0]*lineSample.x + m[1][1]*lineSample.y + m[1][3];
#else
   /** Performs image to model coordinate transformation. This implementation bypasses
    *  theModelTransform. Probably should eventually switch to use theModelTransform
    *  because this cannot handle map rotation. */

   // make sure that the passed in lineSample is good and
   // check to make sure our easting northing is good so
   // we can compute the line sample.
   //
   if(lineSample.hasNans()||theUlEastingNorthing.hasNans())
   {
      eastingNorthing.makeNan();
      return;
   }
   ossimDpt deltaPoint = lineSample;

   eastingNorthing.x = theUlEastingNorthing.x + deltaPoint.x*theMetersPerPixel.x;
   eastingNorthing.y = theUlEastingNorthing.y + (-deltaPoint.y)*theMetersPerPixel.y ;

   //   eastingNorthing.x += (lineSample.x*theMetersPerPixel.x);

   // Note:  the Northing is positive up.  In image space
   // the positive axis is down so we must multiply by
   // -1
   //   eastingNorthing.y += (-lineSample.y*theMetersPerPixel.y);
#endif
}

void ossimMapProjection::eastingNorthingToLineSample(const ossimDpt& eastingNorthing,
                                                     ossimDpt&       lineSample)const
{
#ifdef USE_MODEL_TRANSFORM
   // Transform according to 4x4 transform embedded in the projection:
   const NEWMAT::Matrix& m = theInverseModelTransform.getData();
   lineSample.x = m[0][0]*eastingNorthing.x + m[0][1]*eastingNorthing.y + m[0][3];
   lineSample.y = m[1][0]*eastingNorthing.x + m[1][1]*eastingNorthing.y + m[1][3];
#else
   /** Performs model to image coordinate transformation. This implementation bypasses
    *  theModelTransform. Probably should eventually switch to use equivalent theModelTransform
    *  because this cannot handle map rotation. */

   if(eastingNorthing.hasNans())
   {
      lineSample.makeNan();
      return;
   }
   // check the final result to make sure there were no
   // problems.
   //
   lineSample.x = (eastingNorthing.x - theUlEastingNorthing.x)/theMetersPerPixel.x;

   // We must remember that the Northing is negative since the positive
   // axis for an image is assumed to go down since it's image space.
   lineSample.y = (-(eastingNorthing.y-theUlEastingNorthing.y))/theMetersPerPixel.y;
#endif
}

void ossimMapProjection::eastingNorthingToWorld(const ossimDpt& eastingNorthing,
                                                ossimGpt&       worldPt)const
{
   ossimDpt lineSample;
   eastingNorthingToLineSample(eastingNorthing, lineSample);
   lineSampleToWorld(lineSample, worldPt);
}

void ossimMapProjection::setMetersPerPixel(const ossimDpt& resolution)
{
   theMetersPerPixel = resolution;
   computeDegreesPerPixel();
   updateTransform();
}

void ossimMapProjection::setDecimalDegreesPerPixel(const ossimDpt& resolution)
{
   theDegreesPerPixel = resolution;
   computeMetersPerPixel(); // this method will update the transform
}

void ossimMapProjection::setUlTiePoints(const ossimGpt& gpt)
{
   setUlGpt(gpt);
}

void ossimMapProjection::setUlTiePoints(const ossimDpt& eastingNorthing)
{
   setUlEastingNorthing(eastingNorthing);
}


void ossimMapProjection::setUlEastingNorthing(const ossimDpt& ulEastingNorthing)
{
   theUlEastingNorthing = ulEastingNorthing;
   theUlGpt = inverse(ulEastingNorthing);
   updateTransform();
}

void ossimMapProjection::setUlGpt(const ossimGpt& ulGpt)
{
   theUlGpt = ulGpt;

   // The ossimGpt data members need to use the same datum as this projection:
   if (*theDatum != *(ulGpt.datum()))
      theUlGpt.changeDatum(theDatum);

   // Adjust the stored easting / northing.
   theUlEastingNorthing = forward(theUlGpt);
   updateTransform();
}

bool ossimMapProjection::saveState(ossimKeywordlist& kwl, const char* prefix) const
{
   ossimProjection::saveState(kwl, prefix);

   kwl.add(prefix,
           ossimKeywordNames::ORIGIN_LATITUDE_KW,
           theOrigin.latd(),
           true);

   kwl.add(prefix,
           ossimKeywordNames::CENTRAL_MERIDIAN_KW,
           theOrigin.lond(),
           true);

   theEllipsoid.saveState(kwl, prefix);

   if(theDatum)
   {
      kwl.add(prefix,
              ossimKeywordNames::DATUM_KW,
              theDatum->code(),
              true);
   }

   // Calling access method to give it an opportunity to update the code in case of param change:
   ossim_uint32 code = getPcsCode();
   if (code)
   {
      ossimString epsg_spec = ossimString("EPSG:") + ossimString::toString(code);
      kwl.add(prefix, ossimKeywordNames::SRS_NAME_KW, epsg_spec, true);
   }
   
   if(isGeographic())
   {
      kwl.add(prefix,
              ossimKeywordNames::TIE_POINT_XY_KW,
              ossimDpt(theUlGpt).toString().c_str(),
              true);
      kwl.add(prefix,
              ossimKeywordNames::TIE_POINT_UNITS_KW,
              ossimUnitTypeLut::instance()->getEntryString(OSSIM_DEGREES),
              true);
      kwl.add(prefix,
              ossimKeywordNames::PIXEL_SCALE_XY_KW,
              theDegreesPerPixel.toString().c_str(),
              true);
      kwl.add(prefix,
              ossimKeywordNames::PIXEL_SCALE_UNITS_KW,
              ossimUnitTypeLut::instance()->getEntryString(OSSIM_DEGREES),
              true);
   }
   else
   {
      kwl.add(prefix,
              ossimKeywordNames::TIE_POINT_XY_KW,
              theUlEastingNorthing.toString().c_str(),
              true);
      kwl.add(prefix,
              ossimKeywordNames::TIE_POINT_UNITS_KW,
              ossimUnitTypeLut::instance()->getEntryString(OSSIM_METERS),
              true);
      kwl.add(prefix,
              ossimKeywordNames::PIXEL_SCALE_XY_KW,
              theMetersPerPixel.toString().c_str(),
              true);
      kwl.add(prefix,
              ossimKeywordNames::PIXEL_SCALE_UNITS_KW,
              ossimUnitTypeLut::instance()->getEntryString(OSSIM_METERS),
              true);  
   }

   kwl.add(prefix, ossimKeywordNames::PCS_CODE_KW, code, true);
   kwl.add(prefix, ossimKeywordNames::FALSE_EASTING_NORTHING_KW,
           theFalseEastingNorthing.toString().c_str(), true);
   kwl.add(prefix, ossimKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW,
           ossimUnitTypeLut::instance()->getEntryString(OSSIM_METERS), true);
   kwl.add(prefix, ossimKeywordNames::ELEVATION_LOOKUP_FLAG_KW,
           ossimString::toString(theElevationLookupFlag), true);

   kwl.add(prefix, ossimKeywordNames::ORIGINAL_MAP_UNITS_KW,
           ossimUnitTypeLut::instance()->getEntryString(theProjectionUnits), true);

   kwl.add(prefix, ossimKeywordNames::IMAGE_MODEL_ROTATION_KW, theImageToModelAzimuth, true);

   return true;
}

bool ossimMapProjection::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimProjection::loadState(kwl, prefix);

   const char* elevLookupFlag = kwl.find(prefix, ossimKeywordNames::ELEVATION_LOOKUP_FLAG_KW);
   if(elevLookupFlag)
   {
      theElevationLookupFlag = ossimString(elevLookupFlag).toBool();
   }
   // Get the ellipsoid.
   theEllipsoid.loadState(kwl, prefix);

   const char *lookup;

   // Get the Projection Coordinate System (assumed from EPSG database). 
   // NOTE: the code is read here for saving in this object only. 
   // The code is not verified until a call to getPcs() is called. If ONLY this code
   // had been provided, then the EPSG projection factory would populate a new instance of the 
   // corresponding map projection and have it saveState for constructing again later in the 
   // conventional fashion here
   thePcsCode = 0; 
   lookup = kwl.find(prefix, ossimKeywordNames::PCS_CODE_KW);
   if(lookup)
      thePcsCode = ossimString(lookup).toUInt32(); // EPSG PROJECTION CODE

   // The datum can be specified in 2 ways: either via OSSIM/geotrans alpha-codes or EPSG code.
   // Last resort use WGS 84 (consider throwing an exception to catch any bad datums): 
   theDatum = ossimDatumFactoryRegistry::instance()->create(kwl, prefix);
   if (theDatum == NULL)
   {
      theDatum = ossimDatumFactory::instance()->wgs84();
   }

   // Set all ossimGpt-type members to use this datum:
   theOrigin.datum(theDatum);
   theUlGpt.datum(theDatum);

   // Fetch the ellipsoid from the datum:
   const ossimEllipsoid* ellipse = theDatum->ellipsoid();
   if(ellipse)
      theEllipsoid = *ellipse;
   
   // Get the latitude of the origin.
   lookup = kwl.find(prefix, ossimKeywordNames::ORIGIN_LATITUDE_KW);
   if (lookup)
   {
      theOrigin.latd(ossimString(lookup).toFloat64());
   }
   // else ???

   // Get the central meridian.
   lookup = kwl.find(prefix, ossimKeywordNames::CENTRAL_MERIDIAN_KW);
   if (lookup)
   {
      theOrigin.lond(ossimString(lookup).toFloat64());
   }
   // else ???


   // Get the pixel scale.
   theMetersPerPixel.makeNan();
   theDegreesPerPixel.makeNan();
   lookup = kwl.find(prefix, ossimKeywordNames::PIXEL_SCALE_UNITS_KW);
   if (lookup)
   {
      ossimUnitType units =
         static_cast<ossimUnitType>(ossimUnitTypeLut::instance()->
                                    getEntryNumber(lookup));
      
      lookup = kwl.find(prefix, ossimKeywordNames::PIXEL_SCALE_XY_KW);
      if (lookup)
      {
         ossimDpt scale;
         scale.toPoint(std::string(lookup));

         switch (units)
         {
            case OSSIM_METERS:
            {
               theMetersPerPixel = scale;
               break;
            }
            case OSSIM_DEGREES:
            {
               theDegreesPerPixel.x = scale.x;
               theDegreesPerPixel.y = scale.y;
               break;
            }
            case OSSIM_FEET:
            case OSSIM_US_SURVEY_FEET:
            {
               ossimUnitConversionTool ut;
               ut.setValue(scale.x, units);
               theMetersPerPixel.x = ut.getValue(OSSIM_METERS);
               ut.setValue(scale.y, units);
               theMetersPerPixel.y = ut.getValue(OSSIM_METERS);
               break;
            }
            default:
            {
               if(traceDebug())
               {
                  // Unhandled unit type!
                  ossimNotify(ossimNotifyLevel_WARN)
                  << "ossimMapProjection::loadState WARNING!"
                  << "Unhandled unit type for "
                  << ossimKeywordNames::PIXEL_SCALE_UNITS_KW << ":  "
                  << ( ossimUnitTypeLut::instance()->
                      getEntryString(units).c_str() )
                  << endl;
               }
               break;
            }
         } // End of switch (units)
         
      }  // End of if (PIXEL_SCALE_XY)

   } // End of if (PIXEL_SCALE_UNITS)
   else
   {
      // BACKWARDS COMPATIBILITY LOOKUPS...
      lookup =  kwl.find(prefix, ossimKeywordNames::METERS_PER_PIXEL_X_KW);
      if(lookup)
      {
         theMetersPerPixel.x = fabs(ossimString(lookup).toFloat64());
      }
      
      lookup =  kwl.find(prefix, ossimKeywordNames::METERS_PER_PIXEL_Y_KW);
      if(lookup)
      {
         theMetersPerPixel.y = fabs(ossimString(lookup).toFloat64());
      }
      
      lookup = kwl.find(prefix,
                        ossimKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT);
      if(lookup)
      {
         theDegreesPerPixel.y = fabs(ossimString(lookup).toFloat64());
      }
      
      lookup = kwl.find(prefix,
                        ossimKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON);
      if(lookup)
      {
         theDegreesPerPixel.x = fabs(ossimString(lookup).toFloat64());
      }
   }            

   // Get the tie point.
   theUlGpt.makeNan();

    // Since this won't be picked up from keywords set to 0 to keep nan out.
   theUlGpt.hgt = 0.0;
   
   theUlEastingNorthing.makeNan();
   lookup = kwl.find(prefix, ossimKeywordNames::TIE_POINT_UNITS_KW);
   if (lookup)
   {
      ossimUnitType units = static_cast<ossimUnitType>(ossimUnitTypeLut::instance()->
                                                       getEntryNumber(lookup));
      
      lookup = kwl.find(prefix, ossimKeywordNames::TIE_POINT_XY_KW);
      if (lookup)
      {
         ossimDpt tie;
         tie.toPoint(std::string(lookup));

         switch (units)
         {
            case OSSIM_METERS:
            {
               theUlEastingNorthing = tie;
               break;
            }
            case OSSIM_DEGREES:
            {
               theUlGpt.lond(tie.x);
               theUlGpt.latd(tie.y);
               break;
            }
            case OSSIM_FEET:
            case OSSIM_US_SURVEY_FEET:
            {
               ossimUnitConversionTool ut;
               ut.setValue(tie.x, units);
               theUlEastingNorthing.x = ut.getValue(OSSIM_METERS);
               ut.setValue(tie.y, units);
               theUlEastingNorthing.y = ut.getValue(OSSIM_METERS);
               break;
            }
            default:
            {
               if(traceDebug())
               {
                  // Unhandled unit type!
                  ossimNotify(ossimNotifyLevel_WARN)
                  << "ossimMapProjection::loadState WARNING!"
                  << "Unhandled unit type for "
                  << ossimKeywordNames::TIE_POINT_UNITS_KW << ": " 
                  << ( ossimUnitTypeLut::instance()->
                      getEntryString(units).c_str() )
                  << endl;
               }
               break;
            }
         } // End of switch (units)
         
      }  // End of if (TIE_POINT_XY)

   } // End of if (TIE_POINT_UNITS)
   else
   {
      // BACKWARDS COMPATIBILITY LOOKUPS...
      lookup =  kwl.find(prefix, ossimKeywordNames::TIE_POINT_EASTING_KW);
      if(lookup)
      {
         theUlEastingNorthing.x = (ossimString(lookup).toFloat64());
      }

      lookup =  kwl.find(prefix, ossimKeywordNames::TIE_POINT_NORTHING_KW);
      if(lookup)
      {
         theUlEastingNorthing.y = (ossimString(lookup).toFloat64());
      }

      lookup = kwl.find(prefix, ossimKeywordNames::TIE_POINT_LAT_KW);
      if (lookup)
      {
         theUlGpt.latd(ossimString(lookup).toFloat64());
      }

      lookup = kwl.find(prefix, ossimKeywordNames::TIE_POINT_LON_KW);
      if (lookup)
      {
         theUlGpt.lond(ossimString(lookup).toFloat64());
      }
   }
   
   // Get the false easting northing.
   theFalseEastingNorthing.x = 0.0;
   theFalseEastingNorthing.y = 0.0;
   ossimUnitType en_units = OSSIM_METERS;
   lookup = kwl.find(prefix, ossimKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW);
   if (lookup)
   {
      en_units = static_cast<ossimUnitType>(ossimUnitTypeLut::instance()->getEntryNumber(lookup));
   }

   lookup = kwl.find(prefix, ossimKeywordNames::FALSE_EASTING_NORTHING_KW);
   if (lookup)
   {
      ossimDpt eastingNorthing;
      eastingNorthing.toPoint(std::string(lookup));

      switch (en_units)
      {
         case OSSIM_METERS:
         {
            theFalseEastingNorthing = eastingNorthing;
            break;
         }
         case OSSIM_FEET:
         case OSSIM_US_SURVEY_FEET:
         {
            ossimUnitConversionTool ut;
            ut.setValue(eastingNorthing.x, en_units);
            theFalseEastingNorthing.x = ut.getValue(OSSIM_METERS);
            ut.setValue(eastingNorthing.y, en_units);
            theFalseEastingNorthing.y = ut.getValue(OSSIM_METERS);
            break;
         }
         default:
         {
            if(traceDebug())
            {
               // Unhandled unit type!
               ossimNotify(ossimNotifyLevel_WARN)
                  << "ossimMapProjection::loadState WARNING! Unhandled unit type for "
                  << ossimKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW << ":  " 
                  << (ossimUnitTypeLut::instance()->getEntryString(en_units).c_str())
                  << endl;
            }
            break;
         }
      } // End of switch (units)
   }  // End of if (FALSE_EASTING_NORTHING_KW)
   else
   {
      // BACKWARDS COMPATIBILITY LOOKUPS...
      lookup =  kwl.find(prefix, ossimKeywordNames::FALSE_EASTING_KW);
      if(lookup)
      {
         theFalseEastingNorthing.x = (ossimString(lookup).toFloat64());
      }
      
      lookup =  kwl.find(prefix, ossimKeywordNames::FALSE_NORTHING_KW);
      if(lookup)
      {
         theFalseEastingNorthing.y = (ossimString(lookup).toFloat64());
      }
   }            

//    if((theDegreesPerPixel.x!=OSSIM_DBL_NAN)&&
//       (theDegreesPerPixel.y!=OSSIM_DBL_NAN)&&
//       theMetersPerPixel.hasNans())
//    {
//       theMetersPerPixel    = theOrigin.metersPerDegree();
//       theMetersPerPixel.x *= theDegreesPerPixel.x;
//       theMetersPerPixel.y *= theDegreesPerPixel.y;
//    }

   lookup = kwl.find(prefix, ossimKeywordNames::PIXEL_TYPE_KW);
   if (lookup)
   {
      ossimString pixelType = lookup;
      pixelType=pixelType.trim();
      if(pixelType!="")
      {
         pixelType.downcase();
         if(pixelType.contains("area"))
         {
            if( theMetersPerPixel.hasNans() == false)
            {
               if(!theUlEastingNorthing.hasNans())
               {
                  theUlEastingNorthing.x += (theMetersPerPixel.x*0.5);
                  theUlEastingNorthing.y -= (theMetersPerPixel.y*0.5);
               }
            }
            if(theDegreesPerPixel.hasNans() == false)
            {
               theUlGpt.latd( theUlGpt.latd() - (theDegreesPerPixel.y*0.5) );
               theUlGpt.lond( theUlGpt.lond() + (theDegreesPerPixel.x*0.5) );
            }
         }
      }
   }
   
   // We preserve the units of the originally created projection (typically from EPSG proj factory)
   // in case user needs map coordinates in those units (versus default meters)
   lookup = kwl.find(prefix, ossimKeywordNames::ORIGINAL_MAP_UNITS_KW);
   if (lookup)
   {
      theProjectionUnits =
            static_cast<ossimUnitType>(ossimUnitTypeLut::instance()->getEntryNumber(lookup));
   }

   // Possible map-to-image rotation handled by theModelTransform:
   lookup = kwl.find(prefix, ossimKeywordNames::IMAGE_MODEL_ROTATION_KW);
   if (lookup)
   {
      theImageToModelAzimuth = ossimString(lookup).toFloat64();
   }

#if 0
   theModelUnitType = OSSIM_UNIT_UNKNOWN;
   const char* mapUnit = kwl.find(prefix, ossimKeywordNames::IMAGE_MODEL_TRANSFORM_UNIT_KW);
   theModelUnitType = static_cast<ossimUnitType>(ossimUnitTypeLut::instance()->getEntryNumber(mapUnit));
#endif

   //---
   // Set the datum of the origin and tie point.
   // Use method that does NOT perform a shift.
   //---
   if (theDatum)
   {
      theOrigin.datum(theDatum);
      theUlGpt.datum(theDatum);
   }

   if (theMetersPerPixel.hasNans() && theDegreesPerPixel.hasNans())
   {
      ossimDpt mpd = ossimGpt().metersPerDegree();
      if (isGeographic())
      {
         theDegreesPerPixel.lat = 1.0/mpd.y;
         theDegreesPerPixel.lon = 1.0/mpd.y;
      }
      else
      {
         theMetersPerPixel.x = 1.0;
         theMetersPerPixel.y = 1.0;
      }
   }

   ossimString transformElems = kwl.find(prefix, ossimKeywordNames::IMAGE_MODEL_TRANSFORM_MATRIX_KW);
   if (!transformElems.empty())
   {
      vector<ossimString> elements = transformElems.split(" ");
      NEWMAT::Matrix& m = theModelTransform.getData(); // At this scope for IDE debugging
      if (elements.size() != 16)
      {
         ossimNotify(ossimNotifyLevel_WARN)
               << __FILE__ << ": " << __LINE__<< "\nossimMapProjection::loadState ERROR: Model "
               "Transform matrix must have 16 elements!"<< std::endl;
      }
      else
      {
         int i = 0;
         for (ossimString &e : elements)
         {
            m[i / 4][i % 4] = e.toDouble();
            ++i;
         }
      }
      theInverseModelTransform = theModelTransform;
      theInverseModelTransform.i();
      updateFromTransform();
   }
   else
   {
      // No model transform matrix was provided, so calculate it given scale rotation and offset:
      update();
   }

   return true;
}

std::ostream& ossimMapProjection::print(std::ostream& out) const
{
   const char MODULE[] = "ossimMapProjection::print";

   out << setiosflags(ios::fixed) << setprecision(15)
       << "\n// " << MODULE
       << "\n" << ossimKeywordNames::TYPE_KW               << ":  "
       << getClassName()
       << "\n" << ossimKeywordNames::MAJOR_AXIS_KW         << ":  "
       << theEllipsoid.getA()
       << "\n" << ossimKeywordNames::MINOR_AXIS_KW         << ":  "
       << theEllipsoid.getB()
       << "\n" << ossimKeywordNames::ORIGIN_LATITUDE_KW    << ":  "
       << theOrigin.latd()
       << "\n" << ossimKeywordNames::CENTRAL_MERIDIAN_KW   << ":  "
       << theOrigin.lond()
       << "\norigin: " << theOrigin
       << "\n" << ossimKeywordNames::DATUM_KW              << ":  "
       << (theDatum?theDatum->code().c_str():"unknown")
       << "\n" << ossimKeywordNames::METERS_PER_PIXEL_X_KW << ":  "
       << ((ossim::isnan(theMetersPerPixel.x))?ossimString("nan"):ossimString::toString(theMetersPerPixel.x, 15))
       << "\n" << ossimKeywordNames::METERS_PER_PIXEL_Y_KW << ":  "
       << ((ossim::isnan(theMetersPerPixel.y))?ossimString("nan"):ossimString::toString(theMetersPerPixel.y, 15))
       << "\n" << ossimKeywordNames::FALSE_EASTING_NORTHING_KW << ": "
       << theFalseEastingNorthing.toString().c_str()
       << "\n" << ossimKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW << ": "
       << ossimUnitTypeLut::instance()->getEntryString(OSSIM_METERS)
       << "\n" << ossimKeywordNames::PCS_CODE_KW << ": " << thePcsCode;

   if(isGeographic())
   {
      out << "\n" << ossimKeywordNames::TIE_POINT_XY_KW << ": " 
          << ossimDpt(theUlGpt).toString().c_str()
          << "\n" << ossimKeywordNames::TIE_POINT_UNITS_KW << ": " 
          << ossimUnitTypeLut::instance()->getEntryString(OSSIM_DEGREES)
          << "\n" << ossimKeywordNames::PIXEL_SCALE_XY_KW << ": "
          << theDegreesPerPixel.toString().c_str()
          << "\n" << ossimKeywordNames::PIXEL_SCALE_UNITS_KW << ": "
          << ossimUnitTypeLut::instance()->getEntryString(OSSIM_DEGREES)
          << std::endl;
   }
   else
   {
      out << "\n" << ossimKeywordNames::TIE_POINT_XY_KW << ": " 
          << theUlEastingNorthing.toString().c_str()
          << "\n" << ossimKeywordNames::TIE_POINT_UNITS_KW << ": " 
          << ossimUnitTypeLut::instance()->getEntryString(OSSIM_METERS)
          << "\n" << ossimKeywordNames::PIXEL_SCALE_XY_KW << ": "
          << theMetersPerPixel.toString().c_str()
          << "\n" << ossimKeywordNames::PIXEL_SCALE_UNITS_KW << ": "
          << ossimUnitTypeLut::instance()->getEntryString(OSSIM_METERS)
          << std::endl;
   }
   
   return ossimProjection::print(out);
}

void ossimMapProjection::computeDegreesPerPixel()
{
   ossimDpt eastNorthGround = forward(theOrigin);
   ossimDpt rightEastNorth  =  eastNorthGround;
   ossimDpt downEastNorth   =  eastNorthGround;
   rightEastNorth.x += theMetersPerPixel.x;
   downEastNorth.y  -= theMetersPerPixel.y;

   ossimGpt rightGpt = inverse(rightEastNorth);
   ossimGpt downGpt  = inverse(downEastNorth);

   // use euclidean distance to get length along the horizontal (lon)
   // and vertical (lat) directions
   //
   double tempDeltaLat = rightGpt.latd() - theOrigin.latd();
   double tempDeltaLon = rightGpt.lond() - theOrigin.lond();
   theDegreesPerPixel.lon = sqrt(tempDeltaLat*tempDeltaLat + tempDeltaLon*tempDeltaLon);

   tempDeltaLat = downGpt.latd() - theOrigin.latd();
   tempDeltaLon = downGpt.lond() - theOrigin.lond();
   theDegreesPerPixel.lat = sqrt(tempDeltaLat*tempDeltaLat + tempDeltaLon*tempDeltaLon);
}

void ossimMapProjection::computeMetersPerPixel()
{
//#define USE_OSSIMGPT_METERS_PER_DEGREE
#ifdef USE_OSSIMGPT_METERS_PER_DEGREE
   ossimDpt metersPerDegree (theOrigin.metersPerDegree());
   theMetersPerPixel.x = metersPerDegree.x * theDegreesPerPixel.lon;
   theMetersPerPixel.y = metersPerDegree.y * theDegreesPerPixel.lat;
#elif USE_MODEL_TRANSFORM_XXX  // Not working so hide
   // Transform according to 4x4 transform embedded in the projection:
   const NEWMAT::Matrix& m = theModelTransform.getData();
   theMetersPerPixel.x = sqrt(m[0][0]*m[0][0] + m[1][0]*m[1][0]);
   theMetersPerPixel.y = sqrt(m[0][1]*m[0][1] + m[1][1]*m[1][1]);
#else
   ossimGpt right=theOrigin;
   ossimGpt down=theOrigin;

   down.latd(theOrigin.latd()  + theDegreesPerPixel.lat);
   right.lond(theOrigin.lond() + theDegreesPerPixel.lon);

   ossimDpt centerMeters = forward(theOrigin);
   ossimDpt rightMeters = forward(right);
   ossimDpt downMeters  = forward(down);

   theMetersPerPixel.x = (rightMeters - centerMeters).length();
   theMetersPerPixel.y = (downMeters  - centerMeters).length();
   updateTransform();
#endif

}

//**************************************************************************************************
//  METHOD: ossimMapProjection::operator==
//! Compares this to arg projection and returns TRUE if the same. 
//! NOTE: As currently implemented in OSSIM, map projections also contain image geometry 
//! information like tiepoint and scale. This operator is only concerned with the map 
//! specification and ignores image geometry differences.
//**************************************************************************************************
bool ossimMapProjection::operator==(const ossimProjection& projection) const
{
   // Verify that derived types match:
   if (getClassName() != projection.getClassName())
      return false;
   // If both PCS codes are non-zero, that's all we need to check:
   // unless there are model transforms
	//
	const ossimMapProjection* mapProj = dynamic_cast<const ossimMapProjection*>(&projection);
	if(!mapProj)
      return false;

   if (thePcsCode && mapProj->thePcsCode && (thePcsCode != 32767) && 
       (thePcsCode == mapProj->thePcsCode) )
   {
      return true;
	}

   if ( *theDatum != *(mapProj->theDatum) )
      return false;
   
   if (theOrigin != mapProj->theOrigin)
      return false;

   if (theFalseEastingNorthing != mapProj->theFalseEastingNorthing)
      return false;

#if 0
   THIS SECTION IGNORED SINCE IT DEALS WITH IMAGE GEOMETRY, NOT MAP PROJECTION
   if((theModelTransform.getData() != mapProj->theModelTransform.getData()))
      return false;
#endif

   return true;
}

bool ossimMapProjection::isEqualTo(const ossimObject& obj, ossimCompareType compareType)const
{
   const ossimMapProjection* mapProj = dynamic_cast<const ossimMapProjection*>(&obj);
   bool result = mapProj&&ossimProjection::isEqualTo(obj, compareType);
   
   if(result)
   {
      result = (theEllipsoid.isEqualTo(mapProj->theEllipsoid, compareType)&&
                theOrigin.isEqualTo(mapProj->theOrigin, compareType)&&
                theMetersPerPixel.isEqualTo(mapProj->theMetersPerPixel, compareType)&&             
                theDegreesPerPixel.isEqualTo(mapProj->theDegreesPerPixel, compareType)&&             
                theUlGpt.isEqualTo(mapProj->theUlGpt, compareType)&&             
                theUlEastingNorthing.isEqualTo(mapProj->theUlEastingNorthing, compareType)&&             
                theFalseEastingNorthing.isEqualTo(mapProj->theFalseEastingNorthing, compareType)&&             
                (thePcsCode == mapProj->thePcsCode)&&
                (theElevationLookupFlag == mapProj->theElevationLookupFlag)&&
                (theModelTransform.isEqualTo(mapProj->theModelTransform)));
      
      if(result)
      {
         if(compareType == OSSIM_COMPARE_FULL)
         {
            if(theDatum&&mapProj->theDatum)
            {
               result = theDatum->isEqualTo(*mapProj->theDatum, compareType);
            }
         }
         else 
         {
            result = (theDatum==mapProj->theDatum);
         }
      }
   }
   return result;
}

double ossimMapProjection::getFalseEasting() const
{
   return theFalseEastingNorthing.x;
}

double ossimMapProjection::getFalseNorthing() const
{
   return theFalseEastingNorthing.y;
}

double ossimMapProjection::getStandardParallel1() const
{
   return 0.0;
}

double ossimMapProjection::getStandardParallel2() const
{
   return 0.0;
}

void ossimMapProjection::snapTiePointTo(ossim_float64 multiple,
                                        ossimUnitType unitType)
{
   ossim_float64 convertedMultiple = multiple;
   
   if (isGeographic() && (unitType != OSSIM_DEGREES) )
   {
      // Convert to degrees.
      ossimUnitConversionTool convertor;
      convertor.setOrigin(theOrigin);
      convertor.setValue(multiple, unitType);
      convertedMultiple = convertor.getDegrees();
   }
   else if ( !isGeographic() && (unitType != OSSIM_METERS) )
   {
      // Convert to meters.
      ossimUnitConversionTool convertor;
      convertor.setOrigin(theOrigin);
      convertor.setValue(multiple, unitType);
      convertedMultiple = convertor.getMeters();
   }

   // Convert the tie point.
   if (isGeographic())
   {
      // Snap the latitude.
      ossim_float64 d = theUlGpt.latd();
      d = ossim::round<int>(d / convertedMultiple) * convertedMultiple;
      theUlGpt.latd(d);

      // Snap the longitude.
      d = theUlGpt.lond();
      d = ossim::round<int>(d / convertedMultiple) * convertedMultiple;
      theUlGpt.lond(d);

      // Adjust the stored easting / northing.
      theUlEastingNorthing = forward(theUlGpt);
   }
   else
   {
      // Snap the easting.
      ossim_float64 d = theUlEastingNorthing.x - getFalseEasting();
      d = ossim::round<int>(d / convertedMultiple) * convertedMultiple;
      theUlEastingNorthing.x = d + getFalseEasting();

      // Snap the northing.
      d = theUlEastingNorthing.y - getFalseNorthing();
      d = ossim::round<int>(d / convertedMultiple) * convertedMultiple;
      theUlEastingNorthing.y = d + getFalseNorthing();

      // Adjust the stored upper left ground point.
      theUlGpt = inverse(theUlEastingNorthing);
   }
   updateTransform();
}

void ossimMapProjection::snapTiePointToOrigin()
{
   // Convert the tie point.
   if (isGeographic())
   {
      // Note the origin may not be 0.0, 0.0:
      
      // Snap the latitude.
      ossim_float64 d = theUlGpt.latd() - origin().latd();
      d = ossim::round<int>(d / theDegreesPerPixel.y) * theDegreesPerPixel.y;
      theUlGpt.latd(d + origin().latd());

      // Snap the longitude.
      d = theUlGpt.lond() - origin().lond();
      d = ossim::round<int>(d / theDegreesPerPixel.x) * theDegreesPerPixel.x;
      theUlGpt.lond(d + origin().lond());

      // Adjust the stored easting / northing.
      theUlEastingNorthing = forward(theUlGpt);
   }
   else
   {
      // Snap the easting.
      ossim_float64 d = theUlEastingNorthing.x - getFalseEasting();
      d = ossim::round<int>(d / theMetersPerPixel.x) * theMetersPerPixel.x;
      theUlEastingNorthing.x = d + getFalseEasting();

      // Snap the northing.
      d = theUlEastingNorthing.y - getFalseNorthing();
      d = ossim::round<int>(d / theMetersPerPixel.y) * theMetersPerPixel.y;
      theUlEastingNorthing.y = d + getFalseNorthing();

      // Adjust the stored upper left ground point.
      theUlGpt = inverse(theUlEastingNorthing);
   }
   updateTransform();
}

void ossimMapProjection::setElevationLookupFlag(bool flag)
{
   theElevationLookupFlag = flag;
}

bool ossimMapProjection::getElevationLookupFlag()const
{
   return theElevationLookupFlag;
}
   


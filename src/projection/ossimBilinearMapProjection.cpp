//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
//********************************************************************
// $Id: ossimBilinearMapProjection.cpp 19682 2011-05-31 14:21:20Z dburken $

#include <sstream>
using namespace std;

#include <ossim/projection/ossimBilinearMapProjection.h>
#include <ossim/base/ossimDatumFactory.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimEcefPoint.h>
#include <ossim/base/ossimEcefVector.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimDatum.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/base/ossimTieGptSet.h>

#ifdef OSSIM_ID_ENABLED
static const char OSSIM_ID[] = "$Id: ossimBilinearMapProjection.cpp 19682 2011-05-31 14:21:20Z dburken $";
#endif

// static const ossim_uint32 MINIMUM_NMBER_OF_POINTS = 4;

static ossimTrace traceDebug("ossimBilinearMapProjection:debug");

RTTI_DEF2(ossimBilinearMapProjection, "ossimBilinearMapProjection", ossimMapProjection, ossimOptimizableProjection);

ossimBilinearMapProjection::ossimBilinearMapProjection()
   :
      ossimOptimizableProjection(),
      theLineSamplePt(0),
      theGeographicPt(0),
      theLatFit(),
      theLonFit()
{
#ifdef OSSIM_ID_ENABLED
   if (traceDebug()) ossimNotify(ossimNotifyLevel_DEBUG) << OSSIM_ID << endl;
#endif    
}

ossimBilinearMapProjection::ossimBilinearMapProjection(const ossimBilinearMapProjection& rhs)
   : ossimMapProjection(rhs),
      ossimOptimizableProjection(rhs),
      theLineSamplePt(rhs.theLineSamplePt),
      theGeographicPt(rhs.theGeographicPt),
      theLonFit(rhs.theLonFit)
{
   initializeBilinear();
}

ossimBilinearMapProjection::ossimBilinearMapProjection(const ossimDpt& ul,
                                         const ossimDpt& ur,
                                         const ossimDpt& lr,
                                         const ossimDpt& ll,
                                         const ossimGpt& ulg,
                                         const ossimGpt& urg,
                                         const ossimGpt& lrg,
                                         const ossimGpt& llg)
   :
      ossimOptimizableProjection(),
      theLineSamplePt(4),
      theGeographicPt(4),
      theLatFit(),
      theLonFit()
{
   theLineSamplePt[0] = ul;
   theLineSamplePt[1] = ur;
   theLineSamplePt[2] = lr;
   theLineSamplePt[3] = ll;

   theGeographicPt[0] = ulg;
   theGeographicPt[1] = urg;
   theGeographicPt[2] = lrg;
   theGeographicPt[3] = llg;
   
   initializeBilinear();
}
                       
ossimBilinearMapProjection::~ossimBilinearMapProjection()
{
}

ossimObject *ossimBilinearMapProjection::dup()const
{
   return new ossimBilinearMapProjection(*this);
}

ossimGpt ossimBilinearMapProjection::origin()const
{
   ossimGpt result;
   result.makeNan();
   if ( (theGeographicPt.size() == 0) || gPtsHaveNan() )
   {
      return result;
   }

   double lat        = 0.0;
   double lon        = 0.0;
   const double SIZE = theGeographicPt.size();
   
   vector<ossimGpt>::const_iterator i = theGeographicPt.begin();
   while (i != theGeographicPt.end())
   {
      lat += (*i).latd();
      lon += (*i).lond();
      ++i;
   }

   result.latd(lat/SIZE);
   result.lond(lon/SIZE);
   result.height(0.0);
   result.datum(theGeographicPt[0].datum());
   
   return result;
}

ossimRefPtr<ossimBilinearMapProjection> ossimBilinearMapProjection::newProjection(const ossimDrect& imageSpace, bool shiftToZeroOrigin)const
{
   ossimRefPtr<ossimBilinearMapProjection> result;
   ossimDrect targetRect = imageSpace;

   if(shiftToZeroOrigin)
   {
      targetRect = ossimDrect(0,0,imageSpace.width()-1, imageSpace.height()-1);
   }

   ossimGpt ulg, urg, lrg, llg;
   ossimDpt uld(imageSpace.ul());
   ossimDpt urd(imageSpace.ur());
   ossimDpt lrd(imageSpace.lr());
   ossimDpt lld(imageSpace.ll());

   lineSampleToWorld(uld, ulg); 
   lineSampleToWorld(urd, urg); 
   lineSampleToWorld(lrd, lrg); 
   lineSampleToWorld(lld, llg); 

   result = new ossimBilinearMapProjection(targetRect.ul(), targetRect.ur(), targetRect.lr(), targetRect.ll(), 
                                           ulg, urg, lrg, llg);

   
   return result; 
}


ossimDpt ossimBilinearMapProjection::forward(const ossimGpt &worldPoint) const
{
   ossimDpt result;

   worldToLineSample(worldPoint, result);

   return result;
}

ossimGpt ossimBilinearMapProjection::inverse(const ossimDpt &projectedPoint)const
{
   ossimGpt result;

   lineSampleToWorld(projectedPoint, result);

   return result;
}

void ossimBilinearMapProjection::worldToLineSample(const ossimGpt& worldPoint,
                                            ossimDpt&       lineSampPt) const
{
   if(!theInverseSupportedFlag)
   {
      ossimProjection::worldToLineSample(worldPoint, lineSampPt);
   }
   else 
   {
      lineSampPt.makeNan();
      
      if (!theInterpolationPointsHaveNanFlag)
      {
         lineSampPt.x = theXFit.lsFitValue(worldPoint.lond(),
                                           worldPoint.latd());
         lineSampPt.y = theYFit.lsFitValue(worldPoint.lond(),
                                           worldPoint.latd());
      }
   }
}

void ossimBilinearMapProjection::lineSampleToWorld(const ossimDpt& lineSampPt,
                                                ossimGpt&       worldPt) const
{
   lineSampleHeightToWorld(lineSampPt,
                           ossim::nan(),
                           worldPt);
   
}

void ossimBilinearMapProjection::lineSampleHeightToWorld(
   const ossimDpt& lineSampPt,
   const double&   heightAboveEllipsoid,
   ossimGpt&       worldPt) const
{
   worldPt.makeNan();

   if (theInterpolationPointsHaveNanFlag)
   {
      return;
   }

   
   worldPt.lat = theLatFit.lsFitValue(lineSampPt.x, lineSampPt.y);
   worldPt.lon = theLonFit.lsFitValue(lineSampPt.x, lineSampPt.y);
   if (ossim::isnan(heightAboveEllipsoid) == false)
   {
      worldPt.hgt = heightAboveEllipsoid;
   }
   if (theGeographicPt.size())
   {
      worldPt.datum(theGeographicPt[0].datum());
   }
}   

bool ossimBilinearMapProjection::saveState(ossimKeywordlist& kwl,
                                        const char* prefix)const
{
   if (theLineSamplePt.size() != theGeographicPt.size())
   {
      // Should never happen.
      return false;
   }

   ossimMapProjection::saveState(kwl, prefix);

   ossimString imagePoints;
   ossimString groundPoints;
   ossim::toStringList(imagePoints, theLineSamplePt);
   ossim::toStringList(groundPoints, theGeographicPt);
   kwl.add(prefix, 
           "image_points",
           imagePoints,
           true);
   kwl.add(prefix, 
           "ground_points",
           groundPoints,
           true);
#if 0
   const ossim_uint32 SIZE = (ossim_uint32)theLineSamplePt.size();

   for (ossim_uint32 i = 0; i < SIZE; ++i)
   {
      ossimString index_string = ossimString::toString(i);

      // Add the geographic point.
      ossimString kw = "gpt";
      kw += index_string;
      ostringstream os1;
      os1 << theGeographicPt[i];
      kwl.add(prefix, kw, os1.str().c_str());

      // Add the sample line..
      kw = "dpt";
      kw += index_string;
      ostringstream os2;
      os2 << theLineSamplePt[i];
      kwl.add(prefix, kw, os2.str().c_str());
   }
#endif
   return true;
}

bool ossimBilinearMapProjection::loadState(const ossimKeywordlist& kwl,
                                        const char* prefix)
{
   // Load the base class.
   ossimMapProjection::loadState(kwl, prefix);

   // Start with clear lists.
   theLineSamplePt.clear();
   theGeographicPt.clear();

   ossimString imagePoints  = kwl.find(prefix, "image_points");
   ossimString groundPoints = kwl.find(prefix, "ground_points");
   
   if(!imagePoints.empty()&&!groundPoints.empty())
   {
      ossim::toVector(theLineSamplePt, imagePoints);
      ossim::toVector(theGeographicPt, groundPoints);
   }
   else 
   {
      //---
      // Get the number of points.
      // If 0 or gpt size not equal to dpt size get out.
      //---
      const ossim_uint32 SIZE = kwl.numberOf(prefix, "gpt");
      if ( (SIZE == 0) || (SIZE != kwl.numberOf(prefix, "dpt")) )
      {
         return false;
      }
      
      for (ossim_uint32 i = 0; i < SIZE; ++i)
      {
         const char* lookup;
         ossimString index_string = ossimString::toString(i);
         
         // Get the geographic point.
         ossimString kw = "gpt";
         kw += index_string;
         lookup = kwl.find(prefix, kw);
         if (lookup)
         {
            ossimGpt gp;
            gp.toPoint(std::string(lookup));
            
            //---
            // Allow for "nan" height values by substituting with 0.0 so the
            // hasNans() will work.  "nan"s will get placed in the point if the
            // user doesn't have the elevation manager preferences set up
            // correctly.
            //---
            if (gp.isHgtNan())
            {
               gp.height(0.0);
            }
            theGeographicPt.push_back(gp);
         }
         
         // Get the line sample point.
         kw = "dpt";
         kw += index_string;
         lookup = kwl.find(prefix, kw);
         if (lookup)
         {
            ossimDpt dp;
            dp.toPoint(std::string(lookup));
            theLineSamplePt.push_back(dp);
         }
      }
   }

   if (traceDebug())
   {
      print(ossimNotify(ossimNotifyLevel_DEBUG));
   }
   
   initializeBilinear();

   return true;
}

bool ossimBilinearMapProjection::operator==(const ossimProjection&  projection ) const
{
   bool result = false;

   const ossimBilinearMapProjection* bilinear = dynamic_cast<const ossimBilinearMapProjection*>(&projection);

   if(this == bilinear)
   {
      result = true;
   }
   else if(bilinear)
   {
      bool sameSize = ((theLineSamplePt.size() == bilinear->theLineSamplePt.size())&&
                       (theGeographicPt.size() == bilinear->theGeographicPt.size()));
      if(sameSize)
      {
         int idx = 0;
         bool good = true;
         for(idx = 0 ; (idx<theLineSamplePt.size())&&good;++idx)
         {
            if(fabs(theLineSamplePt[idx].x - theLineSamplePt[idx].x) > FLT_EPSILON)
            {
               good = false;
            }
            else if(fabs(theLineSamplePt[idx].y - theLineSamplePt[idx].y) > FLT_EPSILON)
            {
               good = false;
            }
         }
         for(idx = 0 ; (idx<theGeographicPt.size())&&good;++idx)
         {
            if(fabs(theGeographicPt[idx].lond() - theGeographicPt[idx].lond()) > FLT_EPSILON)
            {
               good = false;
            }
            else if(fabs(theGeographicPt[idx].latd() - theGeographicPt[idx].latd()) > FLT_EPSILON)
            {
               good = false;
            }
         }
         result = good;
      }      
   }
   return result;
}

ossimDpt ossimBilinearMapProjection::getMetersPerPixel() const
{
   ossimGpt centerG;
   ossimGpt rightG;
   ossimGpt topG;

   ossimDpt midPoint = midLineSamplePt();

   lineSampleToWorld(midPoint, centerG);
   lineSampleToWorld(midPoint+ossimDpt(1,0), rightG);
   lineSampleToWorld(midPoint+ossimDpt(0,-1), topG);
   
   ossimEcefPoint centerP = centerG;
   ossimEcefPoint rightP  = rightG;
   ossimEcefPoint topP    = topG;

   ossimEcefVector horizontal = rightP-centerP;
   ossimEcefVector vertical   = topP-centerP;

   ossimDpt result(horizontal.magnitude(),
                   vertical.magnitude());

   result.x = (result.x + result.y)/2.0;
   result.y = result.x;
   return result;
}

void ossimBilinearMapProjection::initializeBilinear()
{
   theInterpolationPointsHaveNanFlag = dPtsHaveNan()||gPtsHaveNan();
   // seems the iterative approach for the inverse will achieve better accuracy by using the forward
   // so turn it off for Bilinear map for now
   //theInverseSupportedFlag = true;
   theInverseSupportedFlag = false;
   if(!theInterpolationPointsHaveNanFlag)
   {
      theLatFit.clear();
      theLonFit.clear();
      theXFit.clear();
      theYFit.clear();
      
      const ossim_uint32 SIZE = (ossim_uint32)theLineSamplePt.size();
      if (SIZE != theGeographicPt.size())
      {
         return;
      }
      
      for (ossim_uint32 i = 0; i < SIZE; ++i)
      {
         theLatFit.addSample(theLineSamplePt[i].x,
                             theLineSamplePt[i].y,
                             theGeographicPt[i].latd());
         
         theLonFit.addSample(theLineSamplePt[i].x,
                             theLineSamplePt[i].y,
                             theGeographicPt[i].lond());
         
         theXFit.addSample(theGeographicPt[i].lond(),
                           theGeographicPt[i].latd(),
                           theLineSamplePt[i].x);
         theYFit.addSample(theGeographicPt[i].lond(),
                           theGeographicPt[i].latd(),
                           theLineSamplePt[i].y);
         
      }
      
      theLatFit.solveLS();
      theLonFit.solveLS();
      theXFit.solveLS();
      theYFit.solveLS();
      ossimDpt errorResult;
      getRoundTripError(theLineSamplePt[0],
                        errorResult);
      if(errorResult.length() > 1)
      {
         theInverseSupportedFlag = false;
      }
   }
   if(theGeographicPt.size() > 0)
   {
      theDatum = theGeographicPt[0].datum();
   }
   theDegreesPerPixel.makeNan();
   theMetersPerPixel.makeNan();
   update();
}

bool ossimBilinearMapProjection::dPtsHaveNan() const
{
   if (theLineSamplePt.size() == 0)
   {
      return false;
   }

   vector<ossimDpt>::const_iterator i = theLineSamplePt.begin();
   while (i != theLineSamplePt.end())
   {
      if ( (*i).hasNans() )
      {
         return true;
      }
      ++i;
   }
   return false;
}

bool ossimBilinearMapProjection::gPtsHaveNan() const
{
   //---
   // NOTE:  This method ignores nans in the height field.
   //---
   
   if (theGeographicPt.size() == 0)
   {
      return false;
   }

   vector<ossimGpt>::const_iterator i = theGeographicPt.begin();
   while (i != theGeographicPt.end())
   {
      if ( (*i).isLatNan() || (*i).isLonNan() )
      {
         return true;
      }
      ++i;
   }
   return false;
}

ossimDpt ossimBilinearMapProjection::midLineSamplePt() const
{
   ossimDpt result;
   
   if ( (theLineSamplePt.size() == 0) || dPtsHaveNan())
   {
      result.makeNan();
      return result;
   }

   double x = 0.0;
   double y = 0.0;
   vector<ossimDpt>::const_iterator i = theLineSamplePt.begin();
   while (i != theLineSamplePt.end())
   {
      x += (*i).x;
      y += (*i).y;
      ++i;
   }
   const double SIZE = theLineSamplePt.size();
   result.x = x / SIZE;
   result.y = y / SIZE;

   return result;
}

std::ostream& ossimBilinearMapProjection::print(std::ostream& out) const
{
   ossimNotify(ossimNotifyLevel_INFO)
      << "ossimBilinearMapProjection::print\n";

   ossim_uint32 index = 0;
   vector<ossimDpt>::const_iterator di = theLineSamplePt.begin();
   while (di != theLineSamplePt.end())
   {
      ossimNotify(ossimNotifyLevel_INFO)
         << "theLineSamplePt[" << index << "]:  "
         << (*di) << endl;
      ++di;
      ++index;
   }

   index = 0;
   vector<ossimGpt>::const_iterator gi = theGeographicPt.begin();
   while (gi != theGeographicPt.end())
   {
      ossimNotify(ossimNotifyLevel_INFO)
         << "theGeographicPt[" << index << "]:  "
         << (*gi) << endl;
      ++gi;
      ++index;
   }

   return ossimProjection::print(out);
}

ossim_float64 ossimBilinearMapProjection::setTiePoints(const std::vector<ossimDpt>& lsPt, 
                                                    const std::vector<ossimGpt>& geoPt)
{
   if (lsPt.size() != geoPt.size())
   {
      ossimNotify(ossimNotifyLevel_INFO)
         << "mismatch in image and ground point number" << endl;
      return -1.0;
   }
   if (lsPt.size() < 4)
   {
      ossimNotify(ossimNotifyLevel_INFO)
         << "not enough tie points - need at least 4" << endl;
      return -1.0;
   }

   theLineSamplePt = lsPt;
   theGeographicPt = geoPt;

   //compute fit
   initializeBilinear();

   //compute variance
   ossim_float64 sumerr2=0.0;
   vector<ossimDpt>::const_iterator i;
   vector<ossimGpt>::const_iterator j;
   ossimGpt gres;
   for(i=theLineSamplePt.begin() , j=theGeographicPt.begin() ; i != theLineSamplePt.end() ; ++i, ++j )
   {
      gres.lat  = theLatFit.lsFitValue(i->x, i->y);
      gres.lon  = theLonFit.lsFitValue(i->x, i->y);
      gres.hgt  = j->hgt; //same height as ground point

      sumerr2 += ( ossimEcefPoint(gres) - ossimEcefPoint(*j) ).norm2(); //add squared error in meters
   }

   return sumerr2 / theLineSamplePt.size(); //variance in meter^2
}

bool ossimBilinearMapProjection::setupOptimizer(const ossimString& /* setup */)
{
   return false;
}

ossim_uint32
ossimBilinearMapProjection::degreesOfFreedom()const
{
   return 2*4; //height not used
}

double
ossimBilinearMapProjection::optimizeFit(const ossimTieGptSet& tieSet, double* /* targetVariance */)
{
   //NOTE : IGNORE targetVariance
   std::vector<ossimDpt> imagePoints;
   std::vector<ossimGpt> groundPoints;
   tieSet.getSlaveMasterPoints(imagePoints, groundPoints);
   return setTiePoints(imagePoints, groundPoints); //variance in meters
}

//**************************************************************************************************
//! Access method for tie point information
//**************************************************************************************************
void ossimBilinearMapProjection::getTiePoints(std::vector<ossimDpt>& lsPt, 
                                           std::vector<ossimGpt>& geoPt) const
{
   lsPt  = theLineSamplePt;
   geoPt = theGeographicPt;
}


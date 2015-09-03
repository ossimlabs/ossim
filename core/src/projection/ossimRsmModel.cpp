//---
// File: ossimRsmModel.cpp
//
//  RP 
//  LIMITATIONS - This is supporting only the RSM features that have been
//  observed in current data samples and does not attempt to support the entire
//  RSM specification.
//  Examples of currently unsupported items include ->
//  1.  multiple RSMPC tags for different polynomials for separate image sections 
//  2.  Error Covariance (this may gain priority as we have access to RSMECA data)
//  3.  Illumination model
//  4.  Rectangular coodinate system conversion (RSDIDA GRNDD = "R")
// 
//---

#include <ossim/projection/ossimRsmModel.h>
#include <ossim/base/ossimDatum.h>
#include <ossim/base/ossimEllipsoid.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/elevation/ossimHgtRef.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

// Define Trace flags for use within this file:
static ossimTrace traceExec  ("ossimRsmModel:exec");
static ossimTrace traceDebug ("ossimRsmModel:debug");

RTTI_DEF1(ossimRsmModel, "ossimRsmModel", ossimSensorModel);

static const char* MODEL_TYPE = "ossimRsmModel";
static const char* RNRMO_KW   = "rnrmo";
static const char* CNRMO_KW   = "cnrmo";
static const char* XNRMO_KW   = "xnrmo";
static const char* YNRMO_KW   = "ynrmo";
static const char* ZNRMO_KW   = "znrmo";
static const char* RNRMSF_KW  = "rnrmsf";
static const char* CNRMSF_KW  = "cnrmsf";
static const char* XNRMSF_KW  = "xnrmsf";
static const char* YNRMSF_KW  = "ynrmsf";
static const char* ZNRMSF_KW  = "znrmsf";
static const char* RNPWRX_KW  = "rnpwrx";
static const char* RNPWRY_KW  = "rnpwry";
static const char* RNPWRZ_KW  = "rnpwrz";
static const char* RNTRMS_KW  = "rntrms";
static const char* RNPCF_KW   = "rnpcf";
static const char* RDPWRX_KW  = "rdpwrx";
static const char* RDPWRY_KW  = "rdpwry";
static const char* RDPWRZ_KW  = "rdpwrz";
static const char* RDTRMS_KW  = "rdtrms";
static const char* RDPCF_KW   = "rdpcf";
static const char* CNPWRX_KW  = "cnpwrx";
static const char* CNPWRY_KW  = "cnpwry";
static const char* CNPWRZ_KW  = "cnpwrz";
static const char* CNTRMS_KW  = "cntrms";
static const char* CNPCF_KW   = "cnpcf";
static const char* CDPWRX_KW  = "cdpwrx";
static const char* CDPWRY_KW  = "cdpwry";
static const char* CDPWRZ_KW  = "cdpwrz";
static const char* CDTRMS_KW  = "cdtrms";
static const char* CDPCF_KW   = "cdpcf";
      
ossimRsmModel::ossimRsmModel()
   :
   ossimSensorModel(),

   m_iid(),
   m_edition(),

   m_rsn(0),
   m_csn(0),

   m_rfep(0.0),
   m_cfep(0.0),
   m_rnrmo(0.0),
   m_cnrmo(0.0),
   m_xnrmo(0.0),
   m_ynrmo(0.0),
   m_znrmo(0.0),
   m_rnrmsf(0.0),
   m_cnrmsf(0.0),
   m_xnrmsf(0.0),
   m_ynrmsf(0.0),
   m_znrmsf(0.0),
   
   m_rnpwrx(0),
   m_rnpwry(0),
   m_rnpwrz(0),
   m_rntrms(0),

   m_rnpcf(),
   
   m_rdpwrx(0),
   m_rdpwry(0),
   m_rdpwrz(0),
   m_rdtrms(0),

   m_rdpcf(),
   
   m_cnpwrx(0),
   m_cnpwry(0),
   m_cnpwrz(0),
   m_cntrms(0),

   m_cnpcf(),
   
   m_cdpwrx(0),
   m_cdpwry(0),
   m_cdpwrz(0),
   m_cdtrms(0),

   m_cdpcf()
{
   initAdjustableParameters();
   
} // End: ossimRsmModel::ossimRsmModel()

ossimRsmModel::ossimRsmModel( const ossimRsmModel& obj )
   :
   ossimSensorModel( obj ),

   m_iid( obj.m_iid ),
   m_edition( obj.m_edition ),

   m_rsn( obj.m_rsn ),
   m_csn( obj.m_csn ),

   m_rfep( obj.m_rfep ),
   m_cfep( obj.m_cfep ),
   m_rnrmo( obj.m_rnrmo ),
   m_cnrmo( obj.m_cnrmo ),
   m_xnrmo( obj.m_xnrmo ),
   m_ynrmo( obj.m_ynrmo ),
   m_znrmo( obj.m_znrmo ),
   m_rnrmsf( obj.m_rnrmsf ),
   m_cnrmsf( obj.m_cnrmsf ),
   m_xnrmsf( obj.m_xnrmsf ),
   m_ynrmsf( obj.m_ynrmsf ),
   m_znrmsf( obj.m_znrmsf ),
   
   m_rnpwrx( obj.m_rnpwrx ),
   m_rnpwry( obj.m_rnpwry ),
   m_rnpwrz( obj.m_rnpwrz ),
   m_rntrms( obj.m_rntrms ),

   m_rnpcf( obj.m_rnpcf ),
   
   m_rdpwrx( obj.m_rdpwrx ),
   m_rdpwry( obj.m_rdpwry ),
   m_rdpwrz( obj.m_rdpwrz ),
   m_rdtrms( obj.m_rdtrms ),

   m_rdpcf( obj.m_rdpcf ),
   
   m_cnpwrx( obj.m_cnpwrx ),
   m_cnpwry( obj.m_cnpwry ),
   m_cnpwrz( obj.m_cnpwrz ),
   m_cntrms( obj.m_cntrms ),

   m_cnpcf( obj.m_cnpcf ),
   
   m_cdpwrx( obj.m_cdpwrx ),
   m_cdpwry( obj.m_cdpwry ),
   m_cdpwrz( obj.m_cdpwrz ),
   m_cdtrms( obj.m_cdtrms ),

   m_cdpcf( obj.m_cdpcf )
{
   
} // End: ossimRsmModel::ossimRsmModel( const ossimRsmModel& obj )

const ossimRsmModel& ossimRsmModel::operator=( const ossimRsmModel& rhs )
{
   if (this != &rhs)
   {
      ossimSensorModel::operator=(rhs);

      m_iid = rhs.m_iid;
      m_edition = rhs.m_edition;
      
      m_rsn = rhs.m_rsn;
      m_csn = rhs.m_csn;
      
      m_rfep = rhs.m_rfep;
      m_cfep = rhs.m_cfep;
      m_rnrmo = rhs.m_rnrmo;
      m_cnrmo = rhs.m_cnrmo;
      m_xnrmo = rhs.m_xnrmo;
      m_ynrmo = rhs.m_ynrmo;
      m_znrmo = rhs.m_znrmo;
      m_rnrmsf = rhs.m_rnrmsf;
      m_cnrmsf = rhs.m_cnrmsf;
      m_xnrmsf = rhs.m_xnrmsf;
      m_ynrmsf = rhs.m_ynrmsf;
      m_znrmsf = rhs.m_znrmsf;
      
      m_rnpwrx = rhs.m_rnpwrx;
      m_rnpwry = rhs.m_rnpwry;
      m_rnpwrz = rhs.m_rnpwrz;
      m_rntrms = rhs.m_rntrms;
      
      m_rnpcf = rhs.m_rnpcf;
      
      m_rdpwrx = rhs.m_rdpwrx;
      m_rdpwry = rhs.m_rdpwry;
      m_rdpwrz = rhs.m_rdpwrz;
      m_rdtrms = rhs.m_rdtrms;
      
      m_rdpcf = rhs.m_rdpcf;
      
      m_cnpwrx = rhs.m_cnpwrx;
      m_cnpwry = rhs.m_cnpwry;
      m_cnpwrz = rhs.m_cnpwrz;
      m_cntrms = rhs.m_cntrms;
      
      m_cnpcf = rhs.m_cnpcf;
      
      m_cdpwrx = rhs.m_cdpwrx;
      m_cdpwry = rhs.m_cdpwry;
      m_cdpwrz = rhs.m_cdpwrz;
      m_cdtrms = rhs.m_cdtrms;
      
      m_cdpcf = rhs.m_cdpcf;
   }
   
   return *this;
   
} // End: ossimRsmModel::operator=( const ossimRsmModel& rhs )

ossimRsmModel::~ossimRsmModel()
{
}

//---
//  METHOD: ossimRsmModel::worldToLineSample()
//  
//  Overrides base class implementation. Directly computes line-sample from
//  the polynomials.
//---
void ossimRsmModel::worldToLineSample(const ossimGpt& ground_point,
                                      ossimDpt&       img_pt) const
{
   if(ground_point.isLatNan() || ground_point.isLonNan() )
   {
      img_pt.makeNan();
      return;
   }

   //---
   // Normalize the lat, lon, hgt:
   // a_norm = (a-offset)/scalefactor
   //
   // Note:
   //
   // Was getting bogus line sample values in Western hemisphere; hence, the
   // test on longitude. (drb - 22 May 2015)
   //---
   double lon = (ground_point.lon >= 0.0) ? ground_point.lon : ground_point.lon + 360.0;
   double y = (ossim::degreesToRadians(ground_point.lat) - m_ynrmo) / m_ynrmsf;
   // double x = (ossim::degreesToRadians(ground_point.lon) - m_xnrmo) / m_xnrmsf;
   double x = ( ossim::degreesToRadians(lon) - m_xnrmo) / m_xnrmsf;   
   double z;

   if( ground_point.isHgtNan() )
   {
     z = ( - m_znrmo) / m_znrmsf;
   }
   else
   {
     z = (ground_point.hgt - m_znrmo) / m_znrmsf;
   }

   double rnNrm =  polynomial(x, y, z, m_rnpwrx, m_rnpwry, m_rnpwrz, m_rnpcf); 
   double rdNrm =  polynomial(x, y, z, m_rdpwrx, m_rdpwry, m_rdpwrz, m_rdpcf);
   double cnNrm =  polynomial(x, y, z, m_cnpwrx, m_cnpwry, m_cnpwrz, m_cnpcf);
   double cdNrm =  polynomial(x, y, z, m_cdpwrx, m_cdpwry, m_cdpwrz, m_cdpcf);
   
   double rNrm  = rnNrm / rdNrm;
   double cNrm  = cnNrm / cdNrm;

   //---
   //  Unnormalize the computed value
   //  a = (a_norm * scalefactor) + offset
   //
   //  Note:
   //
   //  RSM (0,0) is upper left corner of pixel(0,0). OSSIM (0,0) is
   //  center of the pixel; hence, the - 0.5. (drb 22 May 2015)
   //---
   
   // img_pt.line = (rNrm * m_rnrmsf) + m_rnrmo; 
   // img_pt.samp = (cNrm * m_cnrmsf) + m_cnrmo; 
   img_pt.line = (rNrm * m_rnrmsf) + m_rnrmo - 0.5; 
   img_pt.samp = (cNrm * m_cnrmsf) + m_cnrmo - 0.5; 

} // End: ossimRsmModel::worldToLineSample( ... )

//---
//  METHOD: ossimRsmModel::lineSampleToWorld()
//  
//  Overrides base class implementation. Performs DEM intersection.
//---
void  ossimRsmModel::lineSampleToWorld(const ossimDpt& imagePoint,
                                       ossimGpt&       worldPoint) const
{
   if(!imagePoint.hasNans())
   {
      ossimEcefRay ray;
      
      //---
      // Note:
      // RSM (0,0) is upper left corner of pixel(0,0). OSSIM (0,0) is
      //  center of the pixel; hence, the + 0.5. (drb 22 May 2015)
      //---
      // imagingRay(imagePoint, ray);
      imagingRay(ossimDpt(imagePoint.x+0.5, imagePoint.y+0.5), ray);
      ossimElevManager::instance()->intersectRay(ray, worldPoint);
   }
   else
   {
      worldPoint.makeNan();
   }
}

//---
//  METHOD: ossimRsmModel::lineSampleHeightToWorld()
//  
//  Performs reverse projection of image line/sample to ground point.
//  The imaging ray is intersected with a level plane at height = elev.
//
//  NOTE: U = line, V = sample -- this differs from the convention.
//
//---
void ossimRsmModel::lineSampleHeightToWorld(const ossimDpt& image_point,
                                            const double&   ellHeight,
                                            ossimGpt&       gpt) const
{
   // Borrowed from ossimRpcModel algorithm to converge on polynomial roots

   //---
   // Constants for convergence tests:
   //---
   // SPEC says 1/20 of a pixel for polynomial fit, so converge to at least that point
   static const int    MAX_NUM_ITERATIONS  = 100;
   static const double CONVERGENCE_EPSILON = 0.05;  // pixels

   // double U    = (image_point.y-m_rnrmo) / (m_rnrmsf);
   // double V    = (image_point.x-m_cnrmo) / (m_cnrmsf);


   double U = (image_point.y+0.5-m_rnrmo) / (m_rnrmsf);
   double V = (image_point.x+0.5-m_cnrmo) / (m_cnrmsf);

   //---
   // Initialize quantities to be used in the iteration for ground point:
   //---
   double nlat = 0.0;  // normalized latitude
   double nlon = 0.0;  // normalized longitude
   double nhgt;

   if(ossim::isnan(ellHeight))
   {
     nhgt = (- m_znrmo) / m_znrmsf;  // norm height
   }
   else
   {
      nhgt = (ellHeight - m_znrmo) / m_znrmsf;  // norm height
   }

   double epsilonU = CONVERGENCE_EPSILON/m_rnrmsf;
   double epsilonV = CONVERGENCE_EPSILON/m_cnrmsf;
   int    iteration = 0;
   //---
   // Declare variables only once outside the loop. These include:
   // * polynomials (numerators Pu, Pv, and denominators Qu, Qv),
   // * partial derivatives of polynomials wrt X, Y,
   // * computed normalized image point: Uc, Vc,
   // * residuals of normalized image point: deltaU, deltaV,
   // * partial derivatives of Uc and Vc wrt X, Y,
   // * corrections to normalized lat, lon: deltaLat, deltaLon.
   //---
   double Pu, Qu, Pv, Qv;
   double dPu_dLat, dQu_dLat, dPv_dLat, dQv_dLat;
   double dPu_dLon, dQu_dLon, dPv_dLon, dQv_dLon;
   double Uc, Vc;
   double deltaU, deltaV;
   double dU_dLat, dU_dLon, dV_dLat, dV_dLon, W;
   double deltaLat, deltaLon;

   //---
   // Now iterate until the computed Uc, Vc is within epsilon of the desired
   // image point U, V:
   //---
   do
   {
      //---
      // Calculate the normalized line and sample Uc, Vc as ratio of
      // polynomials Pu, Qu and Pv, Qv:
      //---
      Pu = polynomial(nlon, nlat, nhgt, m_rnpwrx, m_rnpwry, m_rnpwrz, m_rnpcf);
      Qu = polynomial(nlon, nlat, nhgt, m_rdpwrx, m_rdpwry, m_rdpwrz, m_rdpcf);
      Pv = polynomial(nlon, nlat, nhgt, m_cnpwrx, m_cnpwry, m_cnpwrz, m_cnpcf);
      Qv = polynomial(nlon, nlat, nhgt, m_cdpwrx, m_cdpwry, m_cdpwrz, m_cdpcf);
      Uc = Pu/Qu;
      Vc = Pv/Qv;

      //---
      // Compute residuals between desired and computed line, sample:
      //---
      deltaU = U - Uc;
      deltaV = V - Vc;

      //---
      // Check for convergence and skip re-linearization if converged:
      //---
      if ((fabs(deltaU) > epsilonU) || (fabs(deltaV) > epsilonV))
      {
         //---
         // Analytically compute the partials of each polynomial wrt lat, lon:
         //---
         dPu_dLat = dPoly_dLat(nlon, nlat, nhgt, m_rnpwrx, m_rnpwry, m_rnpwrz, m_rnpcf);
         dQu_dLat = dPoly_dLat(nlon, nlat, nhgt, m_rdpwrx, m_rdpwry, m_rdpwrz, m_rdpcf);
         dPv_dLat = dPoly_dLat(nlon, nlat, nhgt, m_cnpwrx, m_cnpwry, m_cnpwrz, m_cnpcf);
         dQv_dLat = dPoly_dLat(nlon, nlat, nhgt, m_cdpwrx, m_cdpwry, m_cdpwrz, m_cdpcf);
         dPu_dLon = dPoly_dLon(nlon, nlat, nhgt, m_rnpwrx, m_rnpwry, m_rnpwrz, m_rnpcf);
         dQu_dLon = dPoly_dLon(nlon, nlat, nhgt, m_rdpwrx, m_rdpwry, m_rdpwrz, m_rdpcf);
         dPv_dLon = dPoly_dLon(nlon, nlat, nhgt, m_cnpwrx, m_cnpwry, m_cnpwrz, m_cnpcf);
         dQv_dLon = dPoly_dLon(nlon, nlat, nhgt, m_cdpwrx, m_cdpwry, m_cdpwrz, m_cdpcf);

         //---
         // Analytically compute partials of quotients U and V wrt lat, lon:
         //---
         dU_dLat = (Qu*dPu_dLat - Pu*dQu_dLat)/(Qu*Qu);
         dU_dLon = (Qu*dPu_dLon - Pu*dQu_dLon)/(Qu*Qu);
         dV_dLat = (Qv*dPv_dLat - Pv*dQv_dLat)/(Qv*Qv);
         dV_dLon = (Qv*dPv_dLon - Pv*dQv_dLon)/(Qv*Qv);

         W = dU_dLon*dV_dLat - dU_dLat*dV_dLon;

         //---
         // Now compute the corrections to normalized lat, lon:
         //---
         deltaLat = (dU_dLon*deltaV - dV_dLon*deltaU) / W;
         deltaLon = (dV_dLat*deltaU - dU_dLat*deltaV) / W;
         nlat += deltaLat;
         nlon += deltaLon;
      }

      ++iteration;

   } while (((fabs(deltaU)>epsilonU) || (fabs(deltaV)>epsilonV))
            && (iteration < MAX_NUM_ITERATIONS));
   //---
   // Test for exceeding allowed number of iterations. Flag error if so:
   //---
   if (iteration == MAX_NUM_ITERATIONS)
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "WARNING ossimRsmModel::lineSampleHeightToWorld:\n"
         << "Max number of iterations reached in ground point "
         << "solution. Results are inaccurate." << endl;
   }

   //---
   // Now un-normalize the ground point lat, lon and establish return quantity:
   //
   // lon will 0 to 2PI when image is near PI radians as specified in RSMIDA
   // GRNDD field when value is "H" versus "G".   OSSIMGPT wrap handles this
   // automatically, so no need to worry about it.
   //---
   gpt.lat = ossim::radiansToDegrees(nlat*m_ynrmsf + m_ynrmo);
   gpt.lon = ossim::radiansToDegrees(nlon*m_xnrmsf + m_xnrmo);
   gpt.hgt = (nhgt * m_znrmsf) + m_znrmo; //ellHeight;

   //---
   // Note: See above note. Added in wrap call. Longitude was coming out 242
   // when should have been -118. (drb - 22 May 2015)
   //---
   gpt.wrap();
   
} // End: ossimRsmModel::lineSampleHeightToWorld( ... )

//---
//  METHOD: ossimRsmModel::imagingRay()
//  
//  Constructs a ray by intersecting 2 ellipsoid heights above and
//  below the RPC height offset, and then forming a vector between the two.
//
//---
void ossimRsmModel::imagingRay(const ossimDpt& imagePoint,
                               ossimEcefRay&   imageRay) const
{
   //---
   // For "from point", "to point" we want the image ray to be from above the
   // ellipsoid down to Earth.
   // 
   // It appears the ray "from point" must be above the ellipsiod for the
   // ossimElevSource::intersectRay method; ultimately, the
   // ossimEllipsoid::nearestIntersection method, else it goes off in the
   // weeds...
   //---
   double vectorLength = m_znrmsf * 2.0;

   ossimGpt gpt;

   // "from" point
   double intHgt = m_znrmo + vectorLength;
   lineSampleHeightToWorld(imagePoint, intHgt, gpt);
   ossimEcefPoint intECFfrom(gpt);

   // "to" point
   lineSampleHeightToWorld(imagePoint, m_znrmo, gpt);
   ossimEcefPoint intECFto(gpt);

   // Construct ray
   ossimEcefRay ray(intECFfrom, intECFto);

   imageRay = ray;
}

void ossimRsmModel::updateModel()
{
}

void ossimRsmModel::initAdjustableParameters()
{
}

ossimObject* ossimRsmModel::dup() const
{
   return new ossimRsmModel(*this);
}

//---
//  METHOD: ossimRsmModel::print()
//  
//  Formatted dump of data members.
//---
std::ostream& ossimRsmModel::print(std::ostream& out) const
{
   std::string prefix = "";
   ossimKeywordlist kwl;
   saveState( kwl, prefix.c_str() );
   out << kwl;
   return out;
}

//---
//  METHOD: ossimRsmModel::saveState()
//  
//  Saves the model state to the KWL. This KWL also serves as a geometry file.
//  
//---
bool ossimRsmModel::saveState(ossimKeywordlist& kwl,
                              const char* prefix) const
{
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossimRsmModel::saveState(): entering..." << std::endl;
   }

   kwl.add(prefix, ossimKeywordNames::TYPE_KW, MODEL_TYPE);

   //---
   // Hand off to base class for common stuff:
   //---
   ossimSensorModel::saveState(kwl, prefix);

   //---
   // Save off offsets and scales:
   //---
   kwl.add(prefix, RNRMO_KW, m_rnrmo);
   kwl.add(prefix, CNRMO_KW, m_cnrmo);
   kwl.add(prefix, XNRMO_KW, m_xnrmo);
   kwl.add(prefix, YNRMO_KW, m_ynrmo);
   kwl.add(prefix, ZNRMO_KW, m_znrmo);
   kwl.add(prefix, RNRMSF_KW, m_rnrmsf);
   kwl.add(prefix, CNRMSF_KW, m_cnrmsf);
   kwl.add(prefix, XNRMSF_KW, m_xnrmsf);
   kwl.add(prefix, YNRMSF_KW, m_ynrmsf);
   kwl.add(prefix, ZNRMSF_KW, m_znrmsf);

   kwl.add(prefix, RNPWRX_KW, m_rnpwrx);
   kwl.add(prefix, RNPWRY_KW, m_rnpwry);
   kwl.add(prefix, RNPWRZ_KW, m_rnpwrz);
   kwl.add(prefix, RNTRMS_KW, m_rntrms);
   for (ossim_uint32 i=0; i<m_rntrms; ++i)
   {
      ossimString key;
      key = RNPCF_KW;
      key += ossimString::toString(i);
      kwl.add(prefix, key.c_str(), m_rnpcf[i]);
   }

   kwl.add(prefix, RDPWRX_KW, m_rdpwrx);
   kwl.add(prefix, RDPWRY_KW, m_rdpwry);
   kwl.add(prefix, RDPWRZ_KW, m_rdpwrz);
   kwl.add(prefix, RDTRMS_KW, m_rdtrms);
   for (ossim_uint32 i=0; i<m_rdtrms; ++i)
   {
	
      ossimString key;
      key = RDPCF_KW;
      key += ossimString::toString(i);
      kwl.add(prefix, key.c_str(), m_rdpcf[i]);
   }

   kwl.add(prefix, CNPWRX_KW, m_cnpwrx);
   kwl.add(prefix, CNPWRY_KW, m_cnpwry);
   kwl.add(prefix, CNPWRZ_KW, m_cnpwrz);
   kwl.add(prefix, CNTRMS_KW, m_cntrms);
   for (ossim_uint32 i=0; i<m_cntrms; ++i)
   {
      ossimString key;
      key = CNPCF_KW;
      key += ossimString::toString(i);
      kwl.add(prefix, key.c_str(), m_cnpcf[i]);
   }

   kwl.add(prefix, CDPWRX_KW, m_cdpwrx);
   kwl.add(prefix, CDPWRY_KW, m_cdpwry);
   kwl.add(prefix, CDPWRZ_KW, m_cdpwrz);
   kwl.add(prefix, CDTRMS_KW, m_cdtrms);
   for (ossim_uint32 i=0; i<m_cdtrms; ++i)
   {
      ossimString key;
      key = CDPCF_KW;
      key += ossimString::toString(i);
      kwl.add(prefix, key.c_str(), m_cdpcf[i]);
   }

   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossimRsmModel::saveState(): returning..." << std::endl;
   }

   return true;
}

//---
//  METHOD: ossimRsmModel::loadState()
//  
//  Restores the model's state from the KWL. This KWL also serves as a
//  geometry file.
//  
//---
bool ossimRsmModel::loadState(const ossimKeywordlist& kwl,
                                  const char* prefix) 
{
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossimRsmModel::loadState(): entering..." << std::endl;
   }
   
   const char* value;
   
   //---
   // Pass on to the base-class for parsing first:
   //---
   bool success = ossimSensorModel::loadState(kwl, prefix);
   if (!success)
   {
      if (traceExec())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "DEBUG ossimRsmModel::loadState(): returning with error..."
            << std::endl;
      }
      return false;
   }
   
   //---
   // Continue parsing for local members:
   //---
   value = kwl.find(prefix, RNRMO_KW);
   if (value)
   {
      m_rnrmo = ossimString(value).toFloat64();
   }
   value = kwl.find(prefix, CNRMO_KW);
   if (value)
   {
      m_cnrmo = ossimString(value).toFloat64();
   }
   value = kwl.find(prefix, XNRMO_KW);
   if (value)
   {
      m_xnrmo = ossimString(value).toFloat64();
   }
   value = kwl.find(prefix, YNRMO_KW);
   if (value)
   {
      m_ynrmo = ossimString(value).toFloat64();
   }
   value = kwl.find(prefix, ZNRMO_KW);
   if (value)
   {
      m_znrmo = ossimString(value).toFloat64();
   }
   value = kwl.find(prefix, RNRMSF_KW);
   if (value)
   {
      m_rnrmsf = ossimString(value).toFloat64();
   }
   value = kwl.find(prefix, CNRMSF_KW);
   if (value)
   {
      m_cnrmsf = ossimString(value).toFloat64();
   }
   value = kwl.find(prefix, XNRMSF_KW);
   if (value)
   {
      m_xnrmsf = ossimString(value).toFloat64();
   }
   value = kwl.find(prefix, YNRMSF_KW);
   if (value)
   {
      m_ynrmsf = ossimString(value).toFloat64();
   }
   value = kwl.find(prefix, ZNRMSF_KW);
   if (value)
   {
      m_znrmsf = ossimString(value).toFloat64();
   }

   value = kwl.find(prefix, RNPWRX_KW);
   if (value)
   {
      m_rnpwrx = ossimString(value).toUInt32();
   }
   value = kwl.find(prefix, RNPWRY_KW);
   if (value)
   {
      m_rnpwry = ossimString(value).toUInt32();
   }
   value = kwl.find(prefix, RNPWRZ_KW);
   if (value)
   {
      m_rnpwrz = ossimString(value).toUInt32();
   }
   
   value = kwl.find(prefix, RNTRMS_KW);
   if (value)
   {
      m_rntrms = ossimString(value).toUInt32();
      m_rnpcf.resize(m_rntrms);      
      for (ossim_uint32 i=0; i<m_rntrms; ++i)
      {
         ossimString keyword;
         keyword = RNPCF_KW;
         keyword += ossimString::toString(i); 
         value = kwl.find(prefix, keyword.c_str());
         if (!value)
         {
            ossimNotify(ossimNotifyLevel_FATAL)
               << "FATAL ossimRsmModel::loadState(): Error "
               << "encountered parsing the following required keyword: "
               << "<" << keyword << ">. Check the keywordlist for proper syntax."
               << std::endl;
            return false;
         }
         m_rnpcf[i] = ossimString(value).toFloat64();
      }
   }

   value = kwl.find(prefix, RDPWRX_KW);
   if (value)
   {
      m_rdpwrx = ossimString(value).toUInt32();
   }
   value = kwl.find(prefix, RDPWRY_KW);
   if (value)
   {
      m_rdpwry = ossimString(value).toUInt32();
   }
   value = kwl.find(prefix, RDPWRZ_KW);
   if (value)
   {
      m_rdpwrz = ossimString(value).toUInt32();
   }

   value = kwl.find(prefix, RDTRMS_KW);
   if (value)
   {
      m_rdtrms = ossimString(value).toUInt32();
      m_rdpcf.resize(m_rdtrms);
      for (ossim_uint32 i=0; i<m_rdtrms; ++i)
      {
         ossimString keyword;
         keyword = RDPCF_KW;
         keyword += ossimString::toString(i);
         value = kwl.find(prefix, keyword.c_str());
         if (!value)
         {
            ossimNotify(ossimNotifyLevel_FATAL)
               << "FATAL ossimRsmModel::loadState(): Error "
               << "encountered parsing the following required keyword: "
               << "<" << keyword << ">. Check the keywordlist for proper syntax."
               << std::endl;
            return false;
         }
         m_rdpcf[i] = ossimString(value).toFloat64();
      } 
   }

   value = kwl.find(prefix, CNPWRX_KW);
   if (value)
   {
      m_cnpwrx = ossimString(value).toUInt32();
   }
   value = kwl.find(prefix, CNPWRY_KW);
   if (value)
   {
      m_cnpwry = ossimString(value).toUInt32();
   }
   value = kwl.find(prefix, CNPWRZ_KW);
   if (value)
   {
      m_cnpwrz = ossimString(value).toUInt32();
   }

   value = kwl.find(prefix, CNTRMS_KW);
   if (value)
   {
      m_cntrms = ossimString(value).toUInt32();
      m_cnpcf.resize(m_cntrms);
      for (ossim_uint32 i=0; i<m_cntrms; ++i)
      {
         ossimString keyword;
         keyword = CNPCF_KW;
         keyword += ossimString::toString(i);
         value = kwl.find(prefix, keyword.c_str());
         if (!value)
         {
            ossimNotify(ossimNotifyLevel_FATAL)
               << "FATAL ossimRsmModel::loadState(): Error "
               << "encountered parsing the following required keyword: "
               << "<" << keyword << ">. Check the keywordlist for proper syntax."
               << std::endl;
            return false;
         }
         m_cnpcf[i] = ossimString(value).toFloat64();
      } 
   }

   value = kwl.find(prefix, CDPWRX_KW);
   if (value)
   {
      m_cdpwrx = ossimString(value).toUInt32();
   }
   value = kwl.find(prefix, CDPWRY_KW);
   if (value)
   {
      m_cdpwry = ossimString(value).toUInt32();
   }
   value = kwl.find(prefix, CDPWRZ_KW);
   if (value)
   {
      m_cdpwrz = ossimString(value).toUInt32();
   }

   value = kwl.find(prefix, CDTRMS_KW);
   if (value)
   {
      m_cdtrms = ossimString(value).toUInt32();
      m_cdpcf.resize(m_cdtrms);
      for (ossim_uint32 i=0; i<m_cdtrms; ++i)
      {
         ossimString keyword;
         keyword = CDPCF_KW;
         keyword += ossimString::toString(i);
         value = kwl.find(prefix, keyword.c_str());
         if (!value)
         {
            ossimNotify(ossimNotifyLevel_FATAL)
               << "FATAL ossimRsmModel::loadState(): Error "
               << "encountered parsing the following required keyword: "
               << "<" << keyword << ">. Check the keywordlist for proper syntax."
               << std::endl;
            return false;
         }
         m_cdpcf[i] = ossimString(value).toFloat64();
      }
   }

   updateModel();
   
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossimRsmModel::loadState(): returning..." << std::endl;
   }
   return true;
}

double ossimRsmModel::polynomial(
   const double& x, const double& y, const double& z, const ossim_uint32& maxx,
   const ossim_uint32& maxy, const ossim_uint32& maxz, std::vector<ossim_float64> pcf) const
{
   double r = 0.0;
   ossim_uint32 index = 0;
   for (ossim_uint32 k = 0; k <= maxz; ++k)
   {
      for (ossim_uint32 j = 0; j <= maxy; ++j)
      {
         for (ossim_uint32 i = 0; i <= maxx; ++i)
         {
            r+=pcf[index]*std::pow(x,(double)i)*std::pow(y,(double)j)*std::pow(z,(double)k);
            ++index;
         }
      }
   }
   return r;
}

double ossimRsmModel::dPoly_dLat(
   const double& x, const double& y, const double& z, const ossim_uint32& maxx,
   const ossim_uint32& maxy, const ossim_uint32& maxz, std::vector<ossim_float64> pcf) const
                                 
{
   double dr = 0.0;
   ossim_uint32 index = 0;
   for (ossim_uint32 k = 0; k <= maxz; ++k)
   {
      for (ossim_uint32 j = 0; j <= maxy; ++j)
      {
         for (ossim_uint32 i = 0; i <= maxx; ++i)
         {
            if (j>0)
            {
               dr+=j*pcf[index]*std::pow(x,(double)i)*std::pow(y,(double)(j-1))*std::pow(z,(double)k);
            }
            ++index;
         }
      }
   }
   return dr;
}

double ossimRsmModel::dPoly_dLon(
   const double& x, const double& y, const double& z, const ossim_uint32& maxx,
   const ossim_uint32& maxy, const ossim_uint32& maxz, std::vector<ossim_float64> pcf) const
{
   double dr = 0.0;
   ossim_uint32 index = 0;
   for (ossim_uint32 k = 0; k <= maxz; ++k)
   {
      for (ossim_uint32 j = 0; j <= maxy; ++j)
      {
         for (ossim_uint32 i = 0; i <= maxx; ++i)
         {
            if (i>0)
            {
               dr += i*pcf[index]*std::pow(x,(double)(i-1)) *
                  std::pow(y,(double)j)*std::pow(z,(double)k);
            }
            ++index;
         } 
      }     
   }
   return dr;
}

double ossimRsmModel::dPoly_dHgt(
   const double& x, const double& y, const double& z, const ossim_uint32& maxx,
   const ossim_uint32& maxy, const ossim_uint32& maxz, std::vector<ossim_float64> pcf) const
{
   double dr = 0.0;
   ossim_uint32 index = 0;
   for (ossim_uint32 k = 0; k <= maxz; ++k)
   {
      for (ossim_uint32 j = 0; j <= maxy; ++j)
      {
         for (ossim_uint32 i = 0; i <= maxx; ++i)
         {
            if (k>0)
            {
               dr += k*pcf[index]*std::pow(x,(double)i) *
                  std::pow(y,(double)j)*std::pow(z,(double)(k-1));
            }
            ++index;
         } 
      }     
   }
   return dr;
}

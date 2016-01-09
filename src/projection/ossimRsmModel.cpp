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

RTTI_DEF1(ossimRsmModel, "ossimRsmModel", ossimSensorModel);

// Define Trace flags for use within this file:
static const ossimTrace traceExec  ("ossimRsmModel:exec");
static const ossimTrace traceDebug ("ossimRsmModel:debug");

static std::string MODEL_TYPE_KW  = "ossimRsmModel";

ossimRsmModel::ossimRsmModel()
   :
   ossimSensorModel(),
   m_ida(),
   m_pia(),   
   m_pca()
{
   initAdjustableParameters();
   
}

ossimRsmModel::ossimRsmModel( const ossimRsmModel& obj )
   :
   ossimSensorModel( obj ),
   m_ida( obj.m_ida ),
   m_pia( obj.m_pia ),
   m_pca( obj.m_pca )
{
   
}

const ossimRsmModel& ossimRsmModel::operator=( const ossimRsmModel& rhs )
{
   if (this != &rhs)
   {
      ossimSensorModel::operator=(rhs);
      m_ida = rhs.m_ida;
      m_pia = rhs.m_pia;      
      m_pca = rhs.m_pca;
   }
   return *this;  
}

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
   // RSMIDA GRNDD Ground Domain Form:
   // G: Geodetic: range x is -pi to pi, y is -pi/2 to pi/2
   // H: Geodetic: range x is 0 to 2pi, y is -pi/2 to pi/2 (where image is close to pi.
   // R: Rectangular (not supported).
   //---

   // Initial xyz for computing the pca index.
   double x;
   if ( m_ida.m_grndd == 'H' )
   {
      x = ossim::degreesToRadians((ground_point.lon >= 0.0) ?
                                  ground_point.lon : ground_point.lon + 360.0);
   }
   else
   {
      x = ossim::degreesToRadians( ground_point.lon );
   }
      
   double y = ossim::degreesToRadians(ground_point.lat);
   double z = ground_point.hgt;
   if ( ossim::isnan( z ) )
   {
      z = 0.0; // ??? drb
   }

   ossim_uint32 pcaIndex = getPcaIndex( x, y, z );

   //---
   // Normalize the lat, lon, hgt:
   // a_norm = (a-offset)/scalefactor
   //---
   y = (y - m_pca[pcaIndex].m_ynrmo) / m_pca[pcaIndex].m_ynrmsf;
   x = (x - m_pca[pcaIndex].m_xnrmo) / m_pca[pcaIndex].m_xnrmsf;   
   if( ground_point.isHgtNan() )
   {
     z = ( - m_pca[pcaIndex].m_znrmo) / m_pca[pcaIndex].m_znrmsf;
   }
   else
   {
     z = (ground_point.hgt - m_pca[pcaIndex].m_znrmo) / m_pca[pcaIndex].m_znrmsf;
   }

   double rnNrm =  polynomial(x, y, z, m_pca[pcaIndex].m_rnpwrx, m_pca[pcaIndex].m_rnpwry, m_pca[pcaIndex].m_rnpwrz, m_pca[pcaIndex].m_rnpcf); 
   double rdNrm =  polynomial(x, y, z, m_pca[pcaIndex].m_rdpwrx, m_pca[pcaIndex].m_rdpwry, m_pca[pcaIndex].m_rdpwrz, m_pca[pcaIndex].m_rdpcf);
   double cnNrm =  polynomial(x, y, z, m_pca[pcaIndex].m_cnpwrx, m_pca[pcaIndex].m_cnpwry, m_pca[pcaIndex].m_cnpwrz, m_pca[pcaIndex].m_cnpcf);
   double cdNrm =  polynomial(x, y, z, m_pca[pcaIndex].m_cdpwrx, m_pca[pcaIndex].m_cdpwry, m_pca[pcaIndex].m_cdpwrz, m_pca[pcaIndex].m_cdpcf);
   
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
   img_pt.line = (rNrm * m_pca[pcaIndex].m_rnrmsf) + m_pca[pcaIndex].m_rnrmo - 0.5; 
   img_pt.samp = (cNrm * m_pca[pcaIndex].m_cnrmsf) + m_pca[pcaIndex].m_cnrmo - 0.5; 


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

   ossim_uint32 pcaIndex = getPcaIndex( image_point );
   
   // double U    = (image_point.y-m_rnrmo) / (m_rnrmsf);
   // double V    = (image_point.x-m_cnrmo) / (m_cnrmsf);


   double U = (image_point.y+0.5-m_pca[pcaIndex].m_rnrmo) / (m_pca[pcaIndex].m_rnrmsf);
   double V = (image_point.x+0.5-m_pca[pcaIndex].m_cnrmo) / (m_pca[pcaIndex].m_cnrmsf);

   //---
   // Initialize quantities to be used in the iteration for ground point:
   //---
   double nlat = 0.0;  // normalized latitude
   double nlon = 0.0;  // normalized longitude
   double nhgt;

   if(ossim::isnan(ellHeight))
   {
     nhgt = (- m_pca[pcaIndex].m_znrmo) / m_pca[pcaIndex].m_znrmsf;  // norm height
   }
   else
   {
      nhgt = (ellHeight - m_pca[pcaIndex].m_znrmo) / m_pca[pcaIndex].m_znrmsf;  // norm height
   }

   double epsilonU = CONVERGENCE_EPSILON/m_pca[pcaIndex].m_rnrmsf;
   double epsilonV = CONVERGENCE_EPSILON/m_pca[pcaIndex].m_cnrmsf;
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
      Pu = polynomial(nlon, nlat, nhgt, m_pca[pcaIndex].m_rnpwrx, m_pca[pcaIndex].m_rnpwry, m_pca[pcaIndex].m_rnpwrz, m_pca[pcaIndex].m_rnpcf);
      Qu = polynomial(nlon, nlat, nhgt, m_pca[pcaIndex].m_rdpwrx, m_pca[pcaIndex].m_rdpwry, m_pca[pcaIndex].m_rdpwrz, m_pca[pcaIndex].m_rdpcf);
      Pv = polynomial(nlon, nlat, nhgt, m_pca[pcaIndex].m_cnpwrx, m_pca[pcaIndex].m_cnpwry, m_pca[pcaIndex].m_cnpwrz, m_pca[pcaIndex].m_cnpcf);
      Qv = polynomial(nlon, nlat, nhgt, m_pca[pcaIndex].m_cdpwrx, m_pca[pcaIndex].m_cdpwry, m_pca[pcaIndex].m_cdpwrz, m_pca[pcaIndex].m_cdpcf);
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
         dPu_dLat = dPoly_dLat(nlon, nlat, nhgt, m_pca[pcaIndex].m_rnpwrx, m_pca[pcaIndex].m_rnpwry, m_pca[pcaIndex].m_rnpwrz, m_pca[pcaIndex].m_rnpcf);
         dQu_dLat = dPoly_dLat(nlon, nlat, nhgt, m_pca[pcaIndex].m_rdpwrx, m_pca[pcaIndex].m_rdpwry, m_pca[pcaIndex].m_rdpwrz, m_pca[pcaIndex].m_rdpcf);
         dPv_dLat = dPoly_dLat(nlon, nlat, nhgt, m_pca[pcaIndex].m_cnpwrx, m_pca[pcaIndex].m_cnpwry, m_pca[pcaIndex].m_cnpwrz, m_pca[pcaIndex].m_cnpcf);
         dQv_dLat = dPoly_dLat(nlon, nlat, nhgt, m_pca[pcaIndex].m_cdpwrx, m_pca[pcaIndex].m_cdpwry, m_pca[pcaIndex].m_cdpwrz, m_pca[pcaIndex].m_cdpcf);
         dPu_dLon = dPoly_dLon(nlon, nlat, nhgt, m_pca[pcaIndex].m_rnpwrx, m_pca[pcaIndex].m_rnpwry, m_pca[pcaIndex].m_rnpwrz, m_pca[pcaIndex].m_rnpcf);
         dQu_dLon = dPoly_dLon(nlon, nlat, nhgt, m_pca[pcaIndex].m_rdpwrx, m_pca[pcaIndex].m_rdpwry, m_pca[pcaIndex].m_rdpwrz, m_pca[pcaIndex].m_rdpcf);
         dPv_dLon = dPoly_dLon(nlon, nlat, nhgt, m_pca[pcaIndex].m_cnpwrx, m_pca[pcaIndex].m_cnpwry, m_pca[pcaIndex].m_cnpwrz, m_pca[pcaIndex].m_cnpcf);
         dQv_dLon = dPoly_dLon(nlon, nlat, nhgt, m_pca[pcaIndex].m_cdpwrx, m_pca[pcaIndex].m_cdpwry, m_pca[pcaIndex].m_cdpwrz, m_pca[pcaIndex].m_cdpcf);

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
   gpt.lat = ossim::radiansToDegrees(nlat*m_pca[pcaIndex].m_ynrmsf + m_pca[pcaIndex].m_ynrmo);
   gpt.lon = ossim::radiansToDegrees(nlon*m_pca[pcaIndex].m_xnrmsf + m_pca[pcaIndex].m_xnrmo);
   gpt.hgt = (nhgt * m_pca[pcaIndex].m_znrmsf) + m_pca[pcaIndex].m_znrmo; //ellHeight;

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
   ossim_uint32 pcaIndex = getPcaIndex( imagePoint );
   
   //---
   // For "from point", "to point" we want the image ray to be from above the
   // ellipsoid down to Earth.
   // 
   // It appears the ray "from point" must be above the ellipsiod for the
   // ossimElevSource::intersectRay method; ultimately, the
   // ossimEllipsoid::nearestIntersection method, else it goes off in the
   // weeds...
   //---
   double vectorLength = m_pca[pcaIndex].m_znrmsf * 2.0;

   ossimGpt gpt;

   // "from" point
   double intHgt = m_pca[pcaIndex].m_znrmo + vectorLength;
   lineSampleHeightToWorld(imagePoint, intHgt, gpt);
   ossimEcefPoint intECFfrom(gpt);

   // "to" point
   lineSampleHeightToWorld(imagePoint, m_pca[pcaIndex].m_znrmo, gpt);
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

bool ossimRsmModel::saveState(ossimKeywordlist& kwl,
                              const char* prefix) const
{
   static const char MODULE[] = "ossimRsmModel::saveState";
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   kwl.add(prefix, ossimKeywordNames::TYPE_KW, MODEL_TYPE_KW.c_str());

   //---
   // Hand off to base class for common stuff:
   //---
   ossimSensorModel::saveState(kwl, prefix);

   std::string pfx = (prefix ? prefix : "" );

   // IDA:
   m_ida.saveState( kwl, prefix );

   // PIA:
   m_pia.saveState( kwl, prefix );  

   // PCA:
   for ( ossim_uint32 i = 0; i < m_pca.size(); ++i )
   {
      m_pca[i].saveState( kwl, pfx, i );
   }
   
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }

   return true;
}

bool ossimRsmModel::loadState(const ossimKeywordlist& kwl,
                              const char* prefix) 
{
   static const char MODULE[] = "ossimRsmModel::loadState";
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   bool status = false;
   
   // Check for type match before preceeding:
   std::string pfx = ( prefix ? prefix : "" );
   std::string type = kwl.findKey( pfx, std::string(ossimKeywordNames::TYPE_KW) );
   if ( (type == "ossimNitfRsmModel" ) || ( type == MODEL_TYPE_KW ) )
   {
      // Pass on to the base-class for parsing first:
      if ( ossimSensorModel::loadState(kwl, prefix) )
      {
         if ( m_ida.loadState( kwl, pfx ) )
         {
            if ( m_pia.loadState( kwl, pfx ) )
            {
               m_pca.clear();
               
               for ( ossim_uint32 tagIndex = 0; tagIndex < m_pia.m_tnis; ++tagIndex )
               {
                  ossimRsmpca pca;
                  if ( pca.loadState( kwl, pfx, tagIndex ) )
                  {
                     m_pca.push_back( pca );
                  }
                  else
                  {
                     ossimNotify(ossimNotifyLevel_WARN)
                        << "WARNING! RSMPCA[" << tagIndex << "] intitialization failed!"
                        << std::endl;
                     break; // Get out...
                  }
               }

               // Should now have a rsmpca record for each segment.
               if ( m_pia.m_tnis == (ossim_uint32)m_pca.size() )
               {
                  // Set the status for downstream code.
                  status = true;
                  
                  updateModel();
               }
               
            } // Matches: if ( m_pia.loadState( kwl, pfx ) )
            
         } // Matches:if ( m_ida.loadState( kwl, pfx ) ) 
         
      } // Matches: if ( ossimSensorModel::loadState(kwl, prefix) )
      
   } // Matches: if ( (type == "ossimNitfRsmModel" ) || ...
   
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exit status = " << (status?"true":"false") << "\n";
   }
   
   return status;
}

ossim_uint32 ossimRsmModel::getPcaIndex(
   const double& x, const double& y, const double& z) const
{
   ossimDpt ipt;
   lowOrderPolynomial( x, y, z, ipt );
   return getPcaIndex( ipt );
}

ossim_uint32 ossimRsmModel::getPcaIndex( const ossimDpt& ipt ) const
{
   // Row section number:
   double rsn = std::floor( ( ipt.y - (double)(m_ida.m_minr) ) / (double)(m_pia.m_rssiz) );
   if ( rsn < 0.0 )
   {
      rsn = 0.0;
   }
   else if ( rsn > (m_pia.m_rnis-1) )
   {
      rsn = m_pia.m_rnis-1;
   }
   // Column section number:
   double csn = std::floor( ( ipt.x - (double)(m_ida.m_minc) ) / (double)(m_pia.m_cssiz) );
   if ( csn < 0.0 )
   {
      csn = 0.0;
   }
   else if ( csn > (m_pia.m_cnis-1) )
   {
      csn = m_pia.m_cnis-1;
   }

   return static_cast<ossim_uint32>(rsn) * m_pia.m_rnis + static_cast<ossim_uint32>(csn);
}

void ossimRsmModel::lowOrderPolynomial(
   const double& x, const double& y, const double& z, ossimDpt& ipt ) const
{
   ipt.y = m_pia.m_r0 + m_pia.m_rx * x + m_pia.m_ry * y + m_pia.m_rz * z +
      m_pia.m_rxx * x * x + m_pia.m_rxy * x * y + m_pia.m_rxz * x * z +
      m_pia.m_ryy * y * y + m_pia.m_ryz * y * z + m_pia.m_rzz * z * z;

   ipt.x = m_pia.m_c0 + m_pia.m_cx * x + m_pia.m_cy * y + m_pia.m_cz * z +
      m_pia.m_cxx * x * x + m_pia.m_cxy * x * y + m_pia.m_cxz * x * z +
      m_pia.m_cyy * y * y + m_pia.m_cyz * y * z + m_pia.m_czz * z * z;
}

double ossimRsmModel::polynomial(
   const double& x, const double& y, const double& z, const ossim_uint32& maxx,
   const ossim_uint32& maxy, const ossim_uint32& maxz, std::vector<double> pcf) const
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
   const ossim_uint32& maxy, const ossim_uint32& maxz, std::vector<double> pcf) const
                                 
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
   const ossim_uint32& maxy, const ossim_uint32& maxz, std::vector<double> pcf) const
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
   const ossim_uint32& maxy, const ossim_uint32& maxz, std::vector<double> pcf) const
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

bool ossimRsmModel::validate() const
{
   static const char MODULE[] = "ossimRsmModel::validate";

   bool status = true;

   if (  (m_pia.m_rnis == 0) ||  (m_pia.m_rnis == 0) || (m_pia.m_tnis == 0) )
   {
      status = false;
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE
         << " ERROR: rsmpia must have at least one section!" << std::endl;
   }

   if ( m_pca.size() != m_pia.m_tnis )
   {
      status = false;
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE
         << " ERROR: rsmpca array not equal to section count!" << std::endl;
   }
   if ( ( m_ida.m_grndd != 'G' ) && ( m_ida.m_grndd != 'H' ) && ( m_ida.m_grndd != 'R' ) )
   {
      status = false;
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE
         << " ERROR: rsmida grndd Ground Domain Form not set!" << std::endl;
   }
   if ( m_ida.m_grndd == 'R' )
   {
      status = false;
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE
         << " ERROR: rsmida grndd Rectangular Ground Domain not supported!" << std::endl;
   }
   if ( m_pia.m_rssiz == 0 )
   {
      status = false; // divide by zero.
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE
         << " ERROR: rsmpia rrsiz Section row size cannot be zero!" << std::endl;
   }
   if ( m_pia.m_cssiz == 0 )
   {
      status = false; // divide by zero.
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE
         << " ERROR: rsmpia cssiz Section column size cannot be zero!" << std::endl;
   }

   return status;
}



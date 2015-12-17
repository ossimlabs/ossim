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

static std::string MODEL_TYPE_KW  = "ossimRsmModel";
static std::string PCA_IID_KW     = "pca.iid";
static std::string PCA_EDITION_KW = "pca.edition";
static std::string PCA_RNRMO_KW   = "pca.rnrmo";
static std::string PCA_CNRMO_KW   = "pca.cnrmo";
static std::string PCA_XNRMO_KW   = "pca.xnrmo";
static std::string PCA_YNRMO_KW   = "pca.ynrmo";
static std::string PCA_ZNRMO_KW   = "pca.znrmo";
static std::string PCA_RNRMSF_KW  = "pca.rnrmsf";
static std::string PCA_CNRMSF_KW  = "pca.cnrmsf";
static std::string PCA_XNRMSF_KW  = "pca.xnrmsf";
static std::string PCA_YNRMSF_KW  = "pca.ynrmsf";
static std::string PCA_ZNRMSF_KW  = "pca.znrmsf";
static std::string PCA_RNPWRX_KW  = "pca.rnpwrx";
static std::string PCA_RNPWRY_KW  = "pca.rnpwry";
static std::string PCA_RNPWRZ_KW  = "pca.rnpwrz";
static std::string PCA_RNTRMS_KW  = "pca.rntrms";
static std::string PCA_RNPCF_KW   = "pca.rnpcf";
static std::string PCA_RDPWRX_KW  = "pca.rdpwrx";
static std::string PCA_RDPWRY_KW  = "pca.rdpwry";
static std::string PCA_RDPWRZ_KW  = "pca.rdpwrz";
static std::string PCA_RDTRMS_KW  = "pca.rdtrms";
static std::string PCA_RDPCF_KW   = "pca.rdpcf";
static std::string PCA_CNPWRX_KW  = "pca.cnpwrx";
static std::string PCA_CNPWRY_KW  = "pca.cnpwry";
static std::string PCA_CNPWRZ_KW  = "pca.cnpwrz";
static std::string PCA_CNTRMS_KW  = "pca.cntrms";
static std::string PCA_CNPCF_KW   = "pca.cnpcf";
static std::string PCA_CDPWRX_KW  = "pca.cdpwrx";
static std::string PCA_CDPWRY_KW  = "pca.cdpwry";
static std::string PCA_CDPWRZ_KW  = "pca.cdpwrz";
static std::string PCA_CDTRMS_KW  = "pca.cdtrms";
static std::string PCA_CDPCF_KW   = "pca.cdpcf";

static std::string PIA_IID_KW     = "pia.iid";
static std::string PIA_EDITION_KW = "pia.edition";
static std::string PIA_R0_KW      = "pia.r0";
static std::string PIA_RX_KW      = "pia.rx";
static std::string PIA_RY_KW      = "pia.ry";
static std::string PIA_RZ_KW      = "pia.rz";
static std::string PIA_RXX_KW     = "pia.rxx";
static std::string PIA_RXY_KW     = "pia.rxy";
static std::string PIA_RXZ_KW     = "pia.rxz";
static std::string PIA_RYY_KW     = "pia.ryy";
static std::string PIA_RYZ_KW     = "pia.ryz";
static std::string PIA_RZZ_KW     = "pia.rzz";
static std::string PIA_C0_KW      = "pia.c0";
static std::string PIA_CX_KW      = "pia.cx";
static std::string PIA_CY_KW      = "pia.cy";
static std::string PIA_CZ_KW      = "pia.cz";
static std::string PIA_CXX_KW     = "pia.cxx";
static std::string PIA_CXY_KW     = "pia.cxy";
static std::string PIA_CXZ_KW     = "pia.cxz";
static std::string PIA_CYY_KW     = "pia.cyy";
static std::string PIA_CYZ_KW     = "pia.cyz";
static std::string PIA_CZZ_KW     = "pia.czz";
static std::string PIA_RNIS_KW    = "pia.rnis";
static std::string PIA_CNIS_KW    = "pia.cnis";
static std::string PIA_TNIS_KW    = "pia.tnis";
static std::string PIA_RSSIZ_KW   = "pia.rssiz";
static std::string PIA_CSSIZ_KW   = "pia.cssiz";


ossimRsmModel::ossimRsmModel()
   :
   ossimSensorModel(),

   m_pca_iid(),
   m_pca_edition(),

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

   m_cdpcf(),

   m_pia_iid(),
   m_pia_edition(),
   m_pia_r0(0.0),
   m_pia_rx(0.0),
   m_pia_ry(0.0),
   m_pia_rz(0.0),
   m_pia_rxx(0.0),
   m_pia_rxy(0.0),
   m_pia_rxz(0.0),
   m_pia_ryy(0.0),
   m_pia_ryz(0.0),
   m_pia_rzz(0.0),
   m_pia_c0(0.0),
   m_pia_cx(0.0),
   m_pia_cy(0.0),
   m_pia_cz(0.0),
   m_pia_cxx(0.0),
   m_pia_cxy(0.0),
   m_pia_cxz(0.0),   
   m_pia_cyy(0.0),
   m_pia_cyz(0.0),
   m_pia_czz(0.0),
   m_pia_rnis(0),
   m_pia_cnis(0),
   m_pia_tnis(0),
   m_pia_rssiz(0.0),
   m_pia_cssiz(0.0)
{
   initAdjustableParameters();
   
} // End: ossimRsmModel::ossimRsmModel()

ossimRsmModel::ossimRsmModel( const ossimRsmModel& obj )
   :
   ossimSensorModel( obj ),

   m_pca_iid( obj.m_pca_iid ),
   m_pca_edition( obj.m_pca_edition ),

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

   m_cdpcf( obj.m_cdpcf ),

   m_pia_iid( obj.m_pia_iid ),
   m_pia_edition( obj.m_pia_edition ),
   m_pia_r0( obj.m_pia_r0 ),
   m_pia_rx( obj.m_pia_rx ),
   m_pia_ry( obj.m_pia_ry ),
   m_pia_rz( obj.m_pia_rz ),
   m_pia_rxx( obj.m_pia_rxx ),
   m_pia_rxy( obj.m_pia_rxy ),
   m_pia_rxz( obj.m_pia_rxz ),
   m_pia_ryy( obj.m_pia_ryy ),
   m_pia_ryz( obj.m_pia_ryz ),
   m_pia_rzz( obj.m_pia_rzz ),
   m_pia_c0( obj.m_pia_c0 ),
   m_pia_cx( obj.m_pia_cx ),
   m_pia_cy( obj.m_pia_cy ),
   m_pia_cz( obj.m_pia_cz ),
   m_pia_cxx( obj.m_pia_cxx ),
   m_pia_cxy( obj.m_pia_cxy ),
   m_pia_cxz( obj.m_pia_cxz ),   
   m_pia_cyy( obj.m_pia_cyy ),
   m_pia_cyz( obj.m_pia_cyz ),   
   m_pia_czz( obj.m_pia_czz ),
   m_pia_rnis( obj.m_pia_rnis ),
   m_pia_cnis( obj.m_pia_cnis ),
   m_pia_tnis( obj.m_pia_tnis ),
   m_pia_rssiz( obj.m_pia_rssiz ),
   m_pia_cssiz( obj.m_pia_cssiz )
{
   
} // End: ossimRsmModel::ossimRsmModel( const ossimRsmModel& obj )

const ossimRsmModel& ossimRsmModel::operator=( const ossimRsmModel& rhs )
{
   if (this != &rhs)
   {
      ossimSensorModel::operator=(rhs);

      m_pca_iid = rhs.m_pca_iid;
      m_pca_edition = rhs.m_pca_edition;
      
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

      m_pia_iid = rhs.m_pia_iid    ;
      m_pia_edition = rhs.m_pia_edition;
      m_pia_r0 = rhs.m_pia_r0;
      m_pia_rx = rhs.m_pia_rx;
      m_pia_ry = rhs.m_pia_ry;
      m_pia_rz = rhs.m_pia_rz;
      m_pia_rxx = rhs.m_pia_rxx;
      m_pia_rxy = rhs.m_pia_rxy;
      m_pia_rxz = rhs.m_pia_rxz;
      m_pia_ryy = rhs.m_pia_ryy;
      m_pia_ryz = rhs.m_pia_ryz;
      m_pia_rzz = rhs.m_pia_rzz;
      m_pia_c0 = rhs.m_pia_c0;
      m_pia_cx = rhs.m_pia_cx;
      m_pia_cy = rhs.m_pia_cy;
      m_pia_cz = rhs.m_pia_cz;
      m_pia_cxx = rhs.m_pia_cxx;
      m_pia_cxy = rhs.m_pia_cxy;
      m_pia_cxz = rhs.m_pia_cxz;      
      m_pia_cyy = rhs.m_pia_cyy;
      m_pia_cyz = rhs.m_pia_cyz;      
      m_pia_czz = rhs.m_pia_czz;
      m_pia_rnis = rhs.m_pia_rnis;
      m_pia_cnis = rhs.m_pia_cnis;
      m_pia_tnis = rhs.m_pia_tnis;
      m_pia_rssiz = rhs.m_pia_rssiz;
      m_pia_cssiz = rhs.m_pia_cssiz;
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

   kwl.add(prefix, ossimKeywordNames::TYPE_KW, MODEL_TYPE_KW.c_str());

   //---
   // Hand off to base class for common stuff:
   //---
   ossimSensorModel::saveState(kwl, prefix);

   kwl.add(prefix, PCA_IID_KW.c_str(), m_pca_iid.c_str());
   kwl.add(prefix, PCA_EDITION_KW.c_str(), m_pca_edition.c_str());
   
   //---
   // Save off offsets and scales:
   //---
   
   kwl.add(prefix, PCA_RNRMO_KW.c_str(), m_rnrmo);
   kwl.add(prefix, PCA_CNRMO_KW.c_str(), m_cnrmo);
   kwl.add(prefix, PCA_XNRMO_KW.c_str(), m_xnrmo);
   kwl.add(prefix, PCA_YNRMO_KW.c_str(), m_ynrmo);
   kwl.add(prefix, PCA_ZNRMO_KW.c_str(), m_znrmo);
   kwl.add(prefix, PCA_RNRMSF_KW.c_str(), m_rnrmsf);
   kwl.add(prefix, PCA_CNRMSF_KW.c_str(), m_cnrmsf);
   kwl.add(prefix, PCA_XNRMSF_KW.c_str(), m_xnrmsf);
   kwl.add(prefix, PCA_YNRMSF_KW.c_str(), m_ynrmsf);
   kwl.add(prefix, PCA_ZNRMSF_KW.c_str(), m_znrmsf);

   kwl.add(prefix, PCA_RNPWRX_KW.c_str(), m_rnpwrx);
   kwl.add(prefix, PCA_RNPWRY_KW.c_str(), m_rnpwry);
   kwl.add(prefix, PCA_RNPWRZ_KW.c_str(), m_rnpwrz);
   kwl.add(prefix, PCA_RNTRMS_KW.c_str(), m_rntrms);
   for (ossim_uint32 i=0; i<m_rntrms; ++i)
   {
      ossimString key;
      key = PCA_RNPCF_KW.c_str();
      key += ossimString::toString(i);
      kwl.add(prefix, key.c_str(), m_rnpcf[i]);
   }

   kwl.add(prefix, PCA_RDPWRX_KW.c_str(), m_rdpwrx);
   kwl.add(prefix, PCA_RDPWRY_KW.c_str(), m_rdpwry);
   kwl.add(prefix, PCA_RDPWRZ_KW.c_str(), m_rdpwrz);
   kwl.add(prefix, PCA_RDTRMS_KW.c_str(), m_rdtrms);
   for (ossim_uint32 i=0; i<m_rdtrms; ++i)
   {
	
      ossimString key;
      key = PCA_RDPCF_KW.c_str();
      key += ossimString::toString(i);
      kwl.add(prefix, key.c_str(), m_rdpcf[i]);
   }

   kwl.add(prefix, PCA_CNPWRX_KW.c_str(), m_cnpwrx);
   kwl.add(prefix, PCA_CNPWRY_KW.c_str(), m_cnpwry);
   kwl.add(prefix, PCA_CNPWRZ_KW.c_str(), m_cnpwrz);
   kwl.add(prefix, PCA_CNTRMS_KW.c_str(), m_cntrms);
   for (ossim_uint32 i=0; i<m_cntrms; ++i)
   {
      ossimString key;
      key = PCA_CNPCF_KW.c_str();
      key += ossimString::toString(i);
      kwl.add(prefix, key.c_str(), m_cnpcf[i]);
   }

   kwl.add(prefix, PCA_CDPWRX_KW.c_str(), m_cdpwrx);
   kwl.add(prefix, PCA_CDPWRY_KW.c_str(), m_cdpwry);
   kwl.add(prefix, PCA_CDPWRZ_KW.c_str(), m_cdpwrz);
   kwl.add(prefix, PCA_CDTRMS_KW.c_str(), m_cdtrms);
   for (ossim_uint32 i=0; i<m_cdtrms; ++i)
   {
      ossimString key;
      key = PCA_CDPCF_KW.c_str();
      key += ossimString::toString(i);
      kwl.add(prefix, key.c_str(), m_cdpcf[i]);
   }

   // PIA:
   kwl.add(prefix, PIA_IID_KW.c_str(), m_pia_iid.c_str());
   kwl.add(prefix, PIA_EDITION_KW.c_str(), m_pia_edition.c_str());
   kwl.add(prefix, PIA_R0_KW.c_str(), m_pia_r0);
   kwl.add(prefix, PIA_RX_KW.c_str(), m_pia_rx);
   kwl.add(prefix, PIA_RY_KW.c_str(), m_pia_ry);
   kwl.add(prefix, PIA_RZ_KW.c_str(), m_pia_rz);
   kwl.add(prefix, PIA_RXX_KW.c_str(), m_pia_rxx);
   kwl.add(prefix, PIA_RXY_KW.c_str(), m_pia_rxy);
   kwl.add(prefix, PIA_RXZ_KW.c_str(), m_pia_rxz);
   kwl.add(prefix, PIA_RYY_KW.c_str(), m_pia_ryy);
   kwl.add(prefix, PIA_RYZ_KW.c_str(), m_pia_ryz);
   kwl.add(prefix, PIA_RZZ_KW.c_str(), m_pia_rzz);
   kwl.add(prefix, PIA_C0_KW.c_str(), m_pia_c0);
   kwl.add(prefix, PIA_CX_KW.c_str(), m_pia_cx);
   kwl.add(prefix, PIA_CY_KW.c_str(), m_pia_cy);
   kwl.add(prefix, PIA_CZ_KW.c_str(), m_pia_cz);
   kwl.add(prefix, PIA_CXX_KW.c_str(), m_pia_cxx);
   kwl.add(prefix, PIA_CXY_KW.c_str(), m_pia_cxy);
   kwl.add(prefix, PIA_CXZ_KW.c_str(), m_pia_cxz);
   kwl.add(prefix, PIA_CYY_KW.c_str(), m_pia_cyy);
   kwl.add(prefix, PIA_CYZ_KW.c_str(), m_pia_cyz);
   kwl.add(prefix, PIA_CZZ_KW.c_str(), m_pia_czz);
   kwl.add(prefix, PIA_RNIS_KW.c_str(), m_pia_rnis);
   kwl.add(prefix, PIA_CNIS_KW.c_str(), m_pia_cnis);
   kwl.add(prefix, PIA_TNIS_KW.c_str(), m_pia_tnis);
   kwl.add(prefix, PIA_RSSIZ_KW.c_str(), m_pia_rssiz);
   kwl.add(prefix, PIA_CSSIZ_KW.c_str(), m_pia_cssiz);

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
   static const char MODULE[] = "ossimRsmModel::loadState";
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   bool result = false;
   
   // Check for type match before preceeding:
   std::string myPrefix = ( prefix ? prefix : "" );
   std::string type = kwl.findKey( myPrefix, std::string(ossimKeywordNames::TYPE_KW) );
   if ( (type == "ossimNitfRsmModel" ) || ( type == MODEL_TYPE_KW ) )
   {
      // Pass on to the base-class for parsing first:
      result = ossimSensorModel::loadState(kwl, prefix);
   }
   
   if ( result )
   {
      std::string pfx = (prefix != 0) ? prefix : "";
      std::string key;
      std::string value;

      result = false; // Set to true on last key.
      
      while( 1 ) // Break out on error.
      {
         //---
         // Continue parsing for local members:
         //---
         key = PCA_RNRMO_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_rnrmo = ossimString(value).toFloat64();
         }
         else
         {
            break;
         }

         key = PCA_CNRMO_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_cnrmo = ossimString(value).toFloat64();
         }
         else
         {
            break;
         }

         key = PCA_XNRMO_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_xnrmo = ossimString(value).toFloat64();
         }
         else
         {
            break;
         }

         key = PCA_YNRMO_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_ynrmo = ossimString(value).toFloat64();
         }
         else
         {
            break;
         }

         key = PCA_ZNRMO_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_znrmo = ossimString(value).toFloat64();
         }
         else
         {
            break;
         }

         key = PCA_RNRMSF_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_rnrmsf = ossimString(value).toFloat64();
         }
         else
         {
            break;
         }

         key = PCA_CNRMSF_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_cnrmsf = ossimString(value).toFloat64();
         }
         else
         {
            break;
         }

         key = PCA_XNRMSF_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_xnrmsf = ossimString(value).toFloat64();
         }
         else
         {
            break;
         }

         key = PCA_YNRMSF_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_ynrmsf = ossimString(value).toFloat64();
         }
         else
         {
            break;
         }

         key = PCA_ZNRMSF_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_znrmsf = ossimString(value).toFloat64();
         }
         else
         {
            break;
         }

         key = PCA_RNPWRX_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_rnpwrx = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_RNPWRY_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_rnpwry = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_RNPWRZ_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_rnpwrz = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_RNTRMS_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_rntrms = ossimString(value).toUInt32();
            m_rnpcf.resize(m_rntrms);
            bool keysParsed = true; // Set to false if not found.
            for (ossim_uint32 i=0; i<m_rntrms; ++i)
            {
               key = PCA_RNPCF_KW;
               key += ossimString::toString(i).string();
               value = kwl.findKey(pfx, key);
               if ( !value.size() )
               {
                  keysParsed = false;
                  break; // Break from for loop.
               }
               m_rnpcf[i] = ossimString(value).toFloat64();
            }
            if ( keysParsed == false )
            {
               break; // Break from while (FOREVER) loop.
            }
         }

         key = PCA_RDPWRX_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_rdpwrx = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_RDPWRY_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_rdpwry = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_RDPWRZ_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_rdpwrz = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_RDTRMS_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_rdtrms = ossimString(value).toUInt32();
            m_rdpcf.resize(m_rdtrms);
            bool keysParsed = true; // Set to false if not found.
            for (ossim_uint32 i=0; i<m_rdtrms; ++i)
            {
               key = PCA_RDPCF_KW;
               key += ossimString::toString(i).string();
               value = kwl.findKey(pfx, key);
               if ( !value.size() )
               {
                  keysParsed = false;
                  break; // Break from for loop.
               }
               m_rdpcf[i] = ossimString(value).toFloat64();
            }
            if ( keysParsed == false )
            {
               break; // Break from while (FOREVER) loop.
            }
         }

         key = PCA_CNPWRX_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_cnpwrx = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_CNPWRY_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_cnpwry = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_CNPWRZ_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_cnpwrz = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_CNTRMS_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_cntrms = ossimString(value).toUInt32();
            m_cnpcf.resize(m_cntrms);
            bool keysParsed = true; // Set to false if not found.
            for (ossim_uint32 i=0; i<m_cntrms; ++i)
            {
               key = PCA_CNPCF_KW;
               key += ossimString::toString(i).string();
               value = kwl.findKey(pfx, key);
               if ( !value.size() )
               {
                  keysParsed = false;
                  break; // Break from for loop.
               }
               m_cnpcf[i] = ossimString(value).toFloat64();
            }
            if ( keysParsed == false )
            {
               break; // Break from while (FOREVER) loop.
            }
         }
         else
         {
            break;
         }

         key = PCA_CDPWRX_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_cdpwrx = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_CDPWRY_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_cdpwry = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_CDPWRZ_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_cdpwrz = ossimString(value).toUInt32();
         }
         else
         {
            break;
         }

         key = PCA_CDTRMS_KW;
         value = kwl.findKey(pfx, key);
         if ( value.size() )
         {
            m_cdtrms = ossimString(value).toUInt32();
            m_cdpcf.resize(m_cdtrms);
            bool keysParsed = true; // Set to false if not found.
            for (ossim_uint32 i=0; i<m_cdtrms; ++i)
            {
               key = PCA_CDPCF_KW;
               key += ossimString::toString(i).string();
               value = kwl.findKey(pfx, key);
               if ( !value.size() )
               {
                  keysParsed = false;
                  break; // Break from for loop.
               }
               m_cdpcf[i] = ossimString(value).toFloat64();
            }
             if ( keysParsed == false )
            {
               break; // Break from while (FOREVER) loop.
            }
         }
         else
         {
            break;
         }

         // If we get here we're good, so set the status for downstream code.
         result = true;

         // Final break from while forever loop.
         break;
         
      } // Matches while( FOREVER ){ ...

      if ( result )
      {  
         updateModel();
      }
      else // Find on key failed...
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << MODULE << " WARNING:\n"
            << "Error encountered parsing the following required keyword: "
            << "<" << key << ">. Check the keywordlist for proper syntax."
            << std::endl;
      }
      
   } // Matches: if ( result ){ ...

   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exit status = " << (result?"true":"false\n")
         << std::endl;
   }
   
   return result;
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

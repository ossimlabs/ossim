//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/projection/ossimRpcSolver.h>
#include <ossim/matrix/newmatnl.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/support_data/ossimNitfRpcBTag.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>

using namespace ossim;
using namespace std;

static const ossim_uint32 STARTING_GRID_SIZE = 8;
static const ossim_uint32 ENDING_GRID_SIZE = 64;

ossimRpcSolver::ossimRpcSolver(bool useElevation, bool useHeightAboveMSLFlag)
:  theUseElevationFlag(useElevation),
   theHeightAboveMSLFlag(useHeightAboveMSLFlag),
   theMeanResidual(0),
   theMaxResidual(0)
{
}

void ossimRpcSolver::solveCoefficients(const ossimDrect& imageBounds,
                                       ossimProjection* proj,
                                       ossim_uint32 xSamples,
                                       ossim_uint32 ySamples)
{
   ossimRefPtr<ossimImageGeometry> geom = new ossimImageGeometry();
   geom->setProjection(proj);
   solveCoefficients(imageBounds, geom.get(), xSamples, ySamples);
}

void ossimRpcSolver::solveCoefficients(const ossimDrect& imageBounds,
                                       ossimImageGeometry* geom,
                                       ossim_uint32 xSamples,
                                       ossim_uint32 ySamples)
{
   if (!geom || !(geom->getProjection()))
      return;
   theRefGeom = geom;

   std::vector<ossimGpt> groundPoints;
   std::vector<ossimDpt> imagePoints;
   ossim_uint32 x,y;
   ossimGpt gpt;
   ossimGpt defaultGround;
   if (ySamples <= 1)
      ySamples = STARTING_GRID_SIZE;
   if (xSamples <= 1)
      xSamples = STARTING_GRID_SIZE;
   srand(time(0));
   double Dx = imageBounds.width()/(xSamples-1);
   double Dy = imageBounds.height()/(ySamples-1);
   ossimDpt dpt;
   for(y = 0; y < ySamples; ++y)
   {
      dpt.y = y*Dy + imageBounds.ul().y;
      for(x = 0; x < xSamples; ++x)
      {
         dpt.x = x*Dx + imageBounds.ul().x;
         if (theUseElevationFlag)
            geom->localToWorld(dpt, gpt);
         else
            geom->localToWorld(dpt, 0, gpt);

         if (gpt.isLatLonNan())
            continue;

         if(gpt.isHgtNan())
            gpt.height(0.0);

         gpt.changeDatum(defaultGround.datum());
         if(theHeightAboveMSLFlag)
         {
            double h = ossimElevManager::instance()->getHeightAboveMSL(gpt);
            if(ossim::isnan(h) == false)
               gpt.height(h);
         }

         imagePoints.push_back(dpt);
         groundPoints.push_back(gpt);
      }
   }
   solveCoefficients(imagePoints, groundPoints);
}

void ossimRpcSolver::solveCoefficients(const std::vector<ossimDpt>& imagePoints,
                                       const std::vector<ossimGpt>& groundControlPoints)
{
   if((imagePoints.size() != groundControlPoints.size()))
      return;

   // we will first create f which holds the result of f(x,y,z).
   // This basically holds the cooresponding image point for each
   // ground control point.  One for x and a second array for y
   int numPoints = imagePoints.size();
   std::vector<double> fx, fy;

   //  Holds the x, y, z vectors
   //
   std::vector<double> x;
   std::vector<double> y;
   std::vector<double> z;
   ossim_uint32 c = 0;
   fx.resize(imagePoints.size());
   fy.resize(imagePoints.size());
   x.resize(imagePoints.size());
   y.resize(imagePoints.size());
   z.resize(imagePoints.size());

   // compute the image bounds for the given image points
   ossimDrect rect(imagePoints);
   ossimDpt centerImagePoint  = rect.midPoint();

   // get the width and height that will be used in data normalization
   ossim_float64 w = rect.width();
   ossim_float64 h = rect.height();

   double latSum=0.0;
   double lonSum=0.0;
   double heightSum=0.0;

   // find the center ground  Use elevation only if its enabled
   for(c = 0; c < groundControlPoints.size();++c)
   {
      if(ossim::isnan(groundControlPoints[c].latd()) == false)
      {
         latSum += groundControlPoints[c].latd();
      }
      if(ossim::isnan(groundControlPoints[c].lond()) == false)
      {
         lonSum += groundControlPoints[c].lond();
      }
      if(!groundControlPoints[c].isHgtNan())
      {
         if(theUseElevationFlag)
         {
            heightSum += groundControlPoints[c].height();
         }
      }
   }

   // set the center ground for the offset
   //
   ossimGpt centerGround(latSum/groundControlPoints.size(),
                         lonSum/groundControlPoints.size(),
                         heightSum/groundControlPoints.size());

   // set up ground scales and deltas for normalization
   ossim_float64 deltaLat       = 0.0;
   ossim_float64 deltaLon       = 0.0;
   ossim_float64 deltaHeight    = 0.0;
   ossim_float64 maxDeltaLat    = 0.0;
   ossim_float64 maxDeltaLon    = 0.0;
   ossim_float64 maxDeltaHeight = 0.0;
   ossim_float64 heightTest       = 0.0;
   for(c = 0; c < groundControlPoints.size(); ++c)
   {
      deltaLat = (groundControlPoints[c].latd()-centerGround.latd());
      deltaLon = (groundControlPoints[c].lond()-centerGround.lond());
      if(!groundControlPoints[c].isHgtNan())
      {
         if(theUseElevationFlag)
         {
            deltaHeight = groundControlPoints[c].height() - centerGround.height();
            heightTest  = groundControlPoints[c].height();
         }
         else
         {
            deltaHeight = 0.0;
            heightTest  = 0.0;
         }
      }
      else
      {
         deltaHeight = 0.0;
      }
      fx[c] = (imagePoints[c].x - centerImagePoint.x)/(w/2.0);
      fy[c] = (imagePoints[c].y - centerImagePoint.y)/(h/2.0);
      
      x[c] = deltaLon;
      y[c] = deltaLat;
      z[c] = deltaHeight;

      if(fabs(deltaLat) > maxDeltaLat)
         maxDeltaLat = fabs(deltaLat);
      if(fabs(deltaLon) > maxDeltaLon)
         maxDeltaLon = fabs(deltaLon);
      if(fabs(heightTest) > maxDeltaHeight)
         maxDeltaHeight = fabs(heightTest);
   }

   bool elevationEnabled = theUseElevationFlag;

   // if max delta is less than a degree set it to 1 degree.
//   if(maxDeltaLat < 1.0)
//      maxDeltaLat = 1.0;
//   if(maxDeltaLon < 1.0)
//      maxDeltaLon = 1.0;

   if(maxDeltaHeight < FLT_EPSILON)
      elevationEnabled = false;
   if(maxDeltaHeight < 1.0)
      maxDeltaHeight = 1.0;

   // set the height scale to something pretty large
   if(!elevationEnabled)
   {
      maxDeltaHeight = 1.0/DBL_EPSILON;
      maxDeltaHeight = 10000;
      centerGround.height(0.0);
   }
   // normalize the ground points
   for(c = 0; c < groundControlPoints.size(); ++c)
   {
      x[c] /= maxDeltaLon;
      y[c] /= maxDeltaLat;
      z[c] /= maxDeltaHeight;
   }

   theRpcModel = new ossimRpcModel;
   theRpcModel->thePolyType = ossimRpcModel::B;
   theRpcModel->theLineOffset = centerImagePoint.y;
   theRpcModel->theSampOffset = centerImagePoint.x;
   theRpcModel->theLineScale  = h/2.0;
   theRpcModel->theSampScale  = w/2.0;

   theRpcModel->theLatScale = maxDeltaLat;
   theRpcModel->theLonScale = maxDeltaLon;
   theRpcModel->theHgtScale = maxDeltaHeight;

   theRpcModel->theLatOffset = centerGround.lat;
   theRpcModel->theLonOffset = centerGround.lon;
   theRpcModel->theHgtOffset = centerGround.hgt;

   if(ossim::isnan(theRpcModel->theHgtOffset))
      theRpcModel->theHgtOffset = 0.0;

   NEWMAT::ColumnVector coeffxVec;
   NEWMAT::ColumnVector coeffyVec;

   // perform a least squares fit for sample values found in f
   // given the world values with variables x, y, z
   solveCoefficients(coeffxVec, fx, x, y, z);

   // perform a least squares fit for line values found in f
   // given the world values with variables x, y, z
   solveCoefficients(coeffyVec, fy, x, y, z);

   // there are 20 numerator coefficients and 19 denominator coefficients.
   // I believe that the very first one for the denominator coefficients is fixed at 1.
   theRpcModel->theLineNumCoef[0] = coeffyVec[0];
   theRpcModel->theLineDenCoef[0] = 1.0;
   theRpcModel->theSampNumCoef[0] = coeffxVec[0];
   theRpcModel->theSampDenCoef[0] = 1.0;
   for (int i=1; i<20; i++)
   {
      theRpcModel->theLineNumCoef[i] = coeffyVec[i];
      theRpcModel->theLineDenCoef[i] = coeffyVec[i+19];
      theRpcModel->theSampNumCoef[i] = coeffxVec[i];
      theRpcModel->theSampDenCoef[i] = coeffxVec[i+19];
   }

   // now lets compute the RMSE for the given control points by feeding it
   // back through the modeled RPC
   ossim_float64  sumSquareError = 0.0;
   ossim_uint32 idx = 0;

   theMaxResidual = 0;
   for (idx = 0; idx<imagePoints.size(); idx++)
   {
      ossimDpt evalPt;
      evalPoint(groundControlPoints[idx], evalPt);
      ossim_float64 len = (evalPt - imagePoints[idx]).length();
      if (len > theMaxResidual)
         theMaxResidual = len;
      sumSquareError += (len*len);
   }

   // set the error
   theMeanResidual = sqrt(sumSquareError/imagePoints.size());
}

bool ossimRpcSolver::solve(const ossimDrect& imageBounds,
                           ossimImageGeometry* geom,
                           const double& tolerance)
{
   static const char* MODULE = "ossimRpcSolver::solve()  ";

   if (!geom)
      return false;

   theRefGeom = geom;
   ossimDpt ul = imageBounds.ul();
   ossim_float64 w = imageBounds.width();
   ossim_float64 h = imageBounds.height();
   ossimDpt ipt, irpc;
   ossimGpt gpt;

   // Start at the minimum grid size:
   ossim_uint32 xSamples = STARTING_GRID_SIZE;
   ossim_uint32 ySamples = STARTING_GRID_SIZE;

   // Loop until error is below threshold:
   bool converged = false;
   while (!converged)
   {
      double residual = 0;
      double sumResiduals = 0;
      int numResiduals = 0;
      theMaxResidual = 0;

      converged = true; // hope for the best and get proved otherwise below
      solveCoefficients(imageBounds, geom, xSamples, ySamples);

      // Sample along x and y directions to accumulate errors:
      double deltaX = w/(xSamples-1);
      double deltaY = h/(ySamples-1);

      // Sample the midpoints between image grid used to compute RPC:
      for (ossim_uint32 y=0; y<ySamples-1; ++y)
      {
         ipt.y = deltaY*((double)y + 0.5) + ul.y;
         for (ossim_uint32 x=0; x<xSamples-1; ++x)
         {
            // Forward projection using input model:
            ipt.x = deltaX*((double)x + 0.5) + ul.x;
            if (theUseElevationFlag)
               geom->localToWorld(ipt, gpt);
            else
               geom->localToWorld(ipt, 0, gpt);
            if(theHeightAboveMSLFlag)
            {
               double h = ossimElevManager::instance()->getHeightAboveMSL(gpt);
               if(ossim::isnan(h) == false)
                  gpt.height(h);
            }

            // Reverse projection using RPC:
            evalPoint(gpt, irpc);

            // Compute residual and accumulate:
            residual = (ipt-irpc).length();
            if (residual > theMaxResidual)
               theMaxResidual = residual;
            sumResiduals += residual;
            ++numResiduals;
         }
      }

      theMeanResidual = sumResiduals/numResiduals;
      if (theMaxResidual > tolerance)
         converged = false;

#if 1
      { //### DEBUG BLOCK ###
         ossimNotify(ossimNotifyLevel_INFO)<<MODULE
               <<"\n   sampling grid size: ("<<xSamples<<", "<<ySamples<<")"
               <<"\n        mean residual: "<<theMeanResidual
               <<"\n         max residual: "<<theMaxResidual
               <<"\n            converged: "<<ossimString::toString(converged)<<endl;
      }
#endif

      if ((xSamples >= ENDING_GRID_SIZE) && (ySamples >= ENDING_GRID_SIZE))
         break;

      if (!converged)
      {
         xSamples *= 2;
         if (xSamples > ENDING_GRID_SIZE)
            xSamples = ENDING_GRID_SIZE;
         ySamples *= 2;
         if (ySamples > ENDING_GRID_SIZE)
            ySamples = ENDING_GRID_SIZE;
      }
   }

   // Was testing the max residual. But use the mean for final test:
   if (theMeanResidual <= tolerance)
         converged = true;

   if (!converged)
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "WARNING: Unable to converge on desired error tolerance ("<<tolerance<<" p).\n"
            <<"    RMS residual error: " << theMeanResidual << "\n"
            <<"    Max residual error: " << theMaxResidual<<std::endl;
   }
   else if (theMaxResidual > tolerance)
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "WARNING: While the RPC solution did converge, at least one residual ("
            <<theMaxResidual<<") is larger than the desired error tolerance ("<<tolerance<<" p)."
            << std::endl;
   }
   return converged;
}

bool ossimRpcSolver::solve(const ossimFilename& imageFilename,
                             const double& pixel_tolerance)
{
   // Establish input geometry:
   ossimRefPtr<ossimImageHandler> h = ossimImageHandlerRegistry::instance()->open(imageFilename);
   if(!h.valid())
      return false;

   ossimRefPtr<ossimImageGeometry> geom = h->getImageGeometry();
   ossimDrect imageRect (h->getBoundingRect());

   return solve(imageRect, geom.get(), pixel_tolerance);
}


double ossimRpcSolver::getRmsError()const
{
   return theMeanResidual;
}

double ossimRpcSolver::getMaxError()const
{
   return theMaxResidual;
}

void ossimRpcSolver::solveInitialCoefficients(NEWMAT::ColumnVector& coeff,
                                              const std::vector<double>& f,
                                              const std::vector<double>& x,
                                              const std::vector<double>& y,
                                              const std::vector<double>& z)const
{
   ossim_uint32 idx = 0;
   NEWMAT::Matrix m;
   NEWMAT::ColumnVector r((int)f.size());
   for(idx = 0; idx < f.size(); ++idx)
   {
      r[idx] = f[idx];
   }
   setupSystemOfEquations(m, r, x, y, z);
   
   coeff = invert(m.t()*m)*m.t()*r;
}

void ossimRpcSolver::solveCoefficients(NEWMAT::ColumnVector& coeff,
                                       const std::vector<double>& f,
                                       const std::vector<double>& x,
                                       const std::vector<double>& y,
                                       const std::vector<double>& z)const
{
   // this is an iterative  linear least square fit.  We really pobably need
   // a nonlinear fit instead
   //
   ossim_uint32 idx = 0;
   NEWMAT::Matrix m;

   NEWMAT::ColumnVector r((int)f.size());

   for(idx = 0; idx < f.size(); ++idx)
   {
      r[idx] = f[idx];
   }

   NEWMAT::ColumnVector tempCoeff;
   NEWMAT::DiagonalMatrix weights((int)f.size());
   NEWMAT::ColumnVector denominator(20);

   // initialize the weight matrix to the identity
   //
   for(idx = 0; idx < f.size(); ++idx)
   {
      weights[idx] = 1.0;
   }

   double residualValue = 1.0/FLT_EPSILON;
   ossim_uint32 iterations = 0;
   NEWMAT::Matrix w2;
   do
   {
      w2 = weights*weights;

#if 0
      { //### DEBUG ###
         cout<<"\nw2 = \n"<<w2<<endl;
         cout<<"\nr = "<<r<<endl;
      }
#endif

      // sets up the matrix to hold the system of equations
      setupSystemOfEquations(m, r, x, y, z);

      // solve the least squares solution.  Note: the invert is used
      // to do a Singular Value Decomposition for the inverse since the
      // matrix is more than likely singular.  Slower but more robust
#if 0
      { //### DEBUG ###
         NEWMAT::Matrix mt = m.t();
         cout<<"\nm = "<<m<<endl;
         cout<<"\nmt = "<<mt<<endl;

         NEWMAT::Matrix mtw2 = m.t()*w2;
         cout<<"\nmtw2 = "<<mtw2<<endl;
         NEWMAT::Matrix mtw2m = mtw2*m;
         cout<<"\nmtw2m = "<<mtw2m<<endl;
         NEWMAT::Matrix mtw2r = mtw2*r;
         cout<<"\nmtw2r = "<<mtw2r<<endl;

         NEWMAT::Matrix mtw2m_inv = invert(mtw2m);
         cout<<"\nmtw2m_inv = "<<mtw2m_inv<<endl;
         tempCoeff = mtw2m_inv * mtw2r;
         cout<<"\ntempCoeff = "<<tempCoeff<<endl;
      }
#else
      tempCoeff = invert(m.t()*w2*m)*m.t()*w2*r;
#endif

      // set up the weight matrix by using the denominator
      for(idx = 0; idx < 19; ++idx)
      {
         denominator[idx+1] = tempCoeff[20+idx];
      }
      denominator[0] = 1.0;
      
      setupWeightMatrix(weights, denominator, r, x, y, z);

      // compute the residual
      NEWMAT::ColumnVector residual = m.t()*w2*(m*tempCoeff-r);

      // now get the innerproduct
      NEWMAT::Matrix tempRes = (residual.t()*residual);
      residualValue = sqrt(tempRes[0][0]);

      ++iterations;

   } while ((residualValue > FLT_EPSILON) && (iterations < 10));
   coeff = tempCoeff;

}

NEWMAT::Matrix ossimRpcSolver::invert(const NEWMAT::Matrix& m)const
{
   ossim_uint32 idx = 0;
   NEWMAT::DiagonalMatrix d;
   NEWMAT::Matrix u;
   NEWMAT::Matrix v;

   // decompose m.t*m which is stored in Temp into the singular values and vectors.
   //
   NEWMAT::SVD(m, d, u, v, true, true);

   // invert the diagonal
   // this is just doing the reciprical fo all diagonal components and store back int
   // d.  ths compute d inverse.
   //
   for(idx=0; idx < (ossim_uint32)d.Ncols(); ++idx)
   {
      if(d[idx] > FLT_EPSILON)
      {
         d[idx] = 1.0/d[idx];
      }
      else
      {
         d[idx] = 0.0;
      }
   }

   //compute inverse of decomposed m;
   return v*d*u.t();
}


void ossimRpcSolver::setupSystemOfEquations(NEWMAT::Matrix& equations,
                                            const NEWMAT::ColumnVector& f,
                                            const std::vector<double>& x,
                                            const std::vector<double>& y,
                                            const std::vector<double>& z)const
{
   ossim_uint32 idx;
   equations.ReSize(f.Nrows(), 39);
   
   for(idx = 0; idx < (ossim_uint32)f.Nrows();++idx)
   {
      equations[idx][0]  = 1;
      equations[idx][1]  = x[idx];
      equations[idx][2]  = y[idx];
      equations[idx][3]  = z[idx];
      equations[idx][4]  = x[idx]*y[idx];
      equations[idx][5]  = x[idx]*z[idx];
      equations[idx][6]  = y[idx]*z[idx];
      equations[idx][7]  = x[idx]*x[idx];
      equations[idx][8]  = y[idx]*y[idx];
      equations[idx][9]  = z[idx]*z[idx];
      equations[idx][10] = x[idx]*y[idx]*z[idx];
      equations[idx][11] = x[idx]*x[idx]*x[idx];
      equations[idx][12] = x[idx]*y[idx]*y[idx];
      equations[idx][13] = x[idx]*z[idx]*z[idx];
      equations[idx][14] = x[idx]*x[idx]*y[idx];
      equations[idx][15] = y[idx]*y[idx]*y[idx];
      equations[idx][16] = y[idx]*z[idx]*z[idx];
      equations[idx][17] = x[idx]*x[idx]*z[idx];
      equations[idx][18] = y[idx]*y[idx]*z[idx];
      equations[idx][19] = z[idx]*z[idx]*z[idx];
      equations[idx][20] = -f[idx]*x[idx];
      equations[idx][21] = -f[idx]*y[idx];
      equations[idx][22] = -f[idx]*z[idx];
      equations[idx][23] = -f[idx]*x[idx]*y[idx];
      equations[idx][24] = -f[idx]*x[idx]*z[idx];
      equations[idx][25] = -f[idx]*y[idx]*z[idx];
      equations[idx][26] = -f[idx]*x[idx]*x[idx];
      equations[idx][27] = -f[idx]*y[idx]*y[idx];
      equations[idx][28] = -f[idx]*z[idx]*z[idx];
      equations[idx][29] = -f[idx]*x[idx]*y[idx]*z[idx];
      equations[idx][30] = -f[idx]*x[idx]*x[idx]*x[idx];
      equations[idx][31] = -f[idx]*x[idx]*y[idx]*y[idx];
      equations[idx][32] = -f[idx]*x[idx]*z[idx]*z[idx];
      equations[idx][33] = -f[idx]*x[idx]*x[idx]*y[idx];
      equations[idx][34] = -f[idx]*y[idx]*y[idx]*y[idx];
      equations[idx][35] = -f[idx]*y[idx]*z[idx]*z[idx];
      equations[idx][36] = -f[idx]*x[idx]*x[idx]*z[idx];
      equations[idx][37] = -f[idx]*y[idx]*y[idx]*z[idx];
      equations[idx][38] = -f[idx]*z[idx]*z[idx]*z[idx];
   }
}

void ossimRpcSolver::setupWeightMatrix(NEWMAT::DiagonalMatrix& result, // holds the resulting weights
                                       const NEWMAT::ColumnVector& coefficients,
                                       const NEWMAT::ColumnVector& f,
                                       const std::vector<double>& x,
                                       const std::vector<double>& y,
                                       const std::vector<double>& z)const
{
   result.ReSize(f.Nrows());
   ossim_uint32 idx = 0;
   ossim_uint32 idx2 = 0;
   NEWMAT::RowVector row(coefficients.Nrows());
   
    for(idx = 0; idx < (ossim_uint32)f.Nrows(); ++idx)
    {
       row[0]  = 1;
       row[1]  = x[idx];
       row[2]  = y[idx];
       row[3]  = z[idx];
       row[4]  = x[idx]*y[idx];
       row[5]  = x[idx]*z[idx];
       row[6]  = y[idx]*z[idx];
       row[7]  = x[idx]*x[idx];
       row[8]  = y[idx]*y[idx];
       row[9]  = z[idx]*z[idx];
       row[10] = x[idx]*y[idx]*z[idx];
       row[11] = x[idx]*x[idx]*x[idx];
       row[12] = x[idx]*y[idx]*y[idx];
       row[13] = x[idx]*z[idx]*z[idx];
       row[14] = x[idx]*x[idx]*y[idx];
       row[15] = y[idx]*y[idx]*y[idx];
       row[16] = y[idx]*z[idx]*z[idx];
       row[17] = x[idx]*x[idx]*z[idx];
       row[18] = y[idx]*y[idx]*z[idx];
       row[19] = z[idx]*z[idx]*z[idx];

      result[idx] = 0.0;
      for(idx2 = 0; idx2 < (ossim_uint32)row.Ncols(); ++idx2)
      {
         result[idx] += row[idx2]*coefficients[idx2];
      }

      if(result[idx] > FLT_EPSILON)
      {
         result[idx] = 1.0/result[idx];
      }
    }
}

void ossimRpcSolver::evalPoint(const ossimGpt& gpt, ossimDpt& ipt) const
{
   if (!theRpcModel)
   {
      ipt.makeNan();
      return;
   }

   theRpcModel->worldToLineSample(gpt, ipt);
}


ossimRefPtr<ossimNitfRegisteredTag> ossimRpcSolver::getNitfRpcBTag() const
{
   ossimNitfRpcBTag* rpcbTag = new ossimNitfRpcBTag();
   rpcbTag->setRpcModelParams(theRpcModel);
   
   // Return it as an ossimRefPtr<ossimNitfRegisteredTag>...
   ossimRefPtr<ossimNitfRegisteredTag> tag = rpcbTag;
   
   return tag;
}

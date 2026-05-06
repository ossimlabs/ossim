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
#include <algorithm>
#include <cfloat>
#include <cmath>

using namespace ossim;
using namespace std;

static const ossim_uint32 STARTING_GRID_SIZE = 8;
static const ossim_uint32 ENDING_GRID_SIZE = 64;
static const ossim_uint32 MAX_SOLVE_ITERATIONS = 6;
static const ossim_uint32 RPC_COEFFICIENT_COUNT = 39;
static const ossim_uint32 RPC_POLYNOMIAL_TERM_COUNT = 20;
static const ossim_float64 MIN_RESIDUAL_IMPROVEMENT_FRACTION = 0.01;
static const ossim_float64 MIN_RESIDUAL_IMPROVEMENT_PIXELS = 1.0e-4;
// Lightly damps denominator terms to avoid rational poles in sparse/flat fits.
static const ossim_float64 RPC_DENOMINATOR_REGULARIZATION_WEIGHT = 1.0e-4;
static const ossim_float64 RPC_DENOMINATOR_EPSILON = 1.0e-6;
static const ossim_uint32 RPC_NONLINEAR_MAX_ITERATIONS = 10;
static const ossim_float64 RPC_AUTO_HEIGHT_LAYER_DELTA_FRACTION = 0.25;
static const ossim_float64 RPC_MIN_AUTO_HEIGHT_LAYER_DELTA = 0.1;
static const ossim_float64 RPC_MAX_AUTO_HEIGHT_LAYER_DELTA = 1000.0;
static const ossim_float64 RPC_AUTO_HEIGHT_LAYER_PROBE_DELTA = 10.0;
static const ossim_float64 RPC_AUTO_HEIGHT_LAYER_TARGET_PIXELS = 0.25;

namespace
{
   bool isFinite(ossim_float64 value)
   {
      return (ossim::isnan(value) == false) && std::isfinite(value);
   }

   void setupRpcTerms(double x, double y, double z, double* terms)
   {
      terms[0]  = 1.0;
      terms[1]  = x;
      terms[2]  = y;
      terms[3]  = z;
      terms[4]  = x*y;
      terms[5]  = x*z;
      terms[6]  = y*z;
      terms[7]  = x*x;
      terms[8]  = y*y;
      terms[9]  = z*z;
      terms[10] = x*y*z;
      terms[11] = x*x*x;
      terms[12] = x*y*y;
      terms[13] = x*z*z;
      terms[14] = x*x*y;
      terms[15] = y*y*y;
      terms[16] = y*z*z;
      terms[17] = x*x*z;
      terms[18] = y*y*z;
      terms[19] = z*z*z;
   }

   double evaluateRpcNumerator(const NEWMAT::ColumnVector& coeff,
                               const double* terms)
   {
      double numerator = 0.0;
      for (int term = 0; term < (int)RPC_POLYNOMIAL_TERM_COUNT; ++term)
         numerator += coeff[term] * terms[term];
      return numerator;
   }

   double evaluateRpcDenominator(const NEWMAT::ColumnVector& coeff,
                                 const double* terms)
   {
      double denominator = terms[0];
      for (int term = 1; term < (int)RPC_POLYNOMIAL_TERM_COUNT; ++term)
         denominator += coeff[RPC_POLYNOMIAL_TERM_COUNT + term - 1] * terms[term];
      if (std::fabs(denominator) < RPC_DENOMINATOR_EPSILON)
         denominator = (denominator < 0.0) ? -RPC_DENOMINATOR_EPSILON : RPC_DENOMINATOR_EPSILON;
      return denominator;
   }

   double evaluateRpcRational(const NEWMAT::ColumnVector& coeff,
                              const double* terms)
   {
      return evaluateRpcNumerator(coeff, terms) / evaluateRpcDenominator(coeff, terms);
   }

   double robustLoss(double residual, bool useHuber, double huberDelta)
   {
      const double absResidual = std::fabs(residual);
      if (!useHuber || (absResidual <= huberDelta))
         return residual * residual;
      return 2.0 * huberDelta * absResidual - huberDelta * huberDelta;
   }

   double robustWeight(double residual, bool useHuber, double huberDelta)
   {
      const double absResidual = std::fabs(residual);
      if (!useHuber || (absResidual <= huberDelta) || (absResidual <= DBL_EPSILON))
         return 1.0;
      return std::sqrt(huberDelta / absResidual);
   }

   double computeRationalObjective(const NEWMAT::ColumnVector& coeff,
                                   const std::vector<double>& f,
                                   const std::vector<double>& x,
                                   const std::vector<double>& y,
                                   const std::vector<double>& z,
                                   bool useHuber,
                                   double huberDelta)
   {
      double objective = 0.0;
      double terms[RPC_POLYNOMIAL_TERM_COUNT];
      for (ossim_uint32 idx = 0; idx < f.size(); ++idx)
      {
         setupRpcTerms(x[idx], y[idx], z[idx], terms);
         const double residual = evaluateRpcRational(coeff, terms) - f[idx];
         if (!isFinite(residual))
            return DBL_MAX;
         objective += robustLoss(residual, useHuber, huberDelta);
      }

      for (int term = 1; term < (int)RPC_POLYNOMIAL_TERM_COUNT; ++term)
      {
         const double regularizedCoeff =
               RPC_DENOMINATOR_REGULARIZATION_WEIGHT *
               coeff[RPC_POLYNOMIAL_TERM_COUNT + term - 1];
         objective += regularizedCoeff * regularizedCoeff;
      }
      return objective / std::max<size_t>(1, f.size());
   }

   double computeRationalRms(const NEWMAT::ColumnVector& coeff,
                             const std::vector<double>& f,
                             const std::vector<double>& x,
                             const std::vector<double>& y,
                             const std::vector<double>& z)
   {
      double sumSquareResidual = 0.0;
      double terms[RPC_POLYNOMIAL_TERM_COUNT];
      for (ossim_uint32 idx = 0; idx < f.size(); ++idx)
      {
         setupRpcTerms(x[idx], y[idx], z[idx], terms);
         const double residual = evaluateRpcRational(coeff, terms) - f[idx];
         if (!isFinite(residual))
            return DBL_MAX;
         sumSquareResidual += residual * residual;
      }
      return std::sqrt(sumSquareResidual / std::max<size_t>(1, f.size()));
   }

   bool computeHeightStatistics(ossimImageGeometry* geom,
                                const ossimDrect& imageBounds,
                                ossim_uint32 xSamples,
                                ossim_uint32 ySamples,
                                bool useHeightAboveMSLFlag,
                                ossim_float64& nominalHeight,
                                ossim_float64& heightDelta)
   {
      if (!geom || (xSamples <= 1) || (ySamples <= 1) ||
          (imageBounds.width() <= DBL_EPSILON) ||
          (imageBounds.height() <= DBL_EPSILON))
         return false;

      const double dx = imageBounds.width()/(xSamples-1);
      const double dy = imageBounds.height()/(ySamples-1);
      ossimDpt dpt;
      ossimGpt gpt;
      ossim_float64 heightSum = 0.0;
      ossim_uint32 heightCount = 0;
      ossim_float64 minHeight = DBL_MAX;
      ossim_float64 maxHeight = -DBL_MAX;
      const ossimDpt metersPerPixel = geom->getMetersPerPixel();
      const ossim_float64 meanMetersPerPixel =
            (std::fabs(metersPerPixel.x) + std::fabs(metersPerPixel.y)) * 0.5;
      ossim_float64 maxPixelsPerMeterHeight = 0.0;

      for(ossim_uint32 y = 0; y < ySamples; ++y)
      {
         dpt.y = y*dy + imageBounds.ul().y;
         for(ossim_uint32 x = 0; x < xSamples; ++x)
         {
            dpt.x = x*dx + imageBounds.ul().x;
            geom->localToWorld(dpt, gpt);
            if (gpt.isLatLonNan() || gpt.isHgtNan())
               continue;
            if (!isFinite(gpt.height()))
               continue;

            if(useHeightAboveMSLFlag)
            {
               double h = ossimElevManager::instance()->getHeightAboveMSL(gpt);
               if(isFinite(h))
                  gpt.height(h);
            }

            heightSum += gpt.height();
            minHeight = std::min(minHeight, gpt.height());
            maxHeight = std::max(maxHeight, gpt.height());
            ++heightCount;
         }
      }

      if (heightCount == 0)
         return false;

      nominalHeight = heightSum / heightCount;

      if (isFinite(meanMetersPerPixel) && (meanMetersPerPixel > DBL_EPSILON))
      {
         ossimGpt gptAtNominalHeight;
         ossimGpt gptAtProbeHeight;
         for(ossim_uint32 y = 0; y < ySamples; ++y)
         {
            dpt.y = y*dy + imageBounds.ul().y;
            for(ossim_uint32 x = 0; x < xSamples; ++x)
            {
               dpt.x = x*dx + imageBounds.ul().x;
               geom->localToWorld(dpt, nominalHeight, gptAtNominalHeight);
               geom->localToWorld(dpt,
                                  nominalHeight + RPC_AUTO_HEIGHT_LAYER_PROBE_DELTA,
                                  gptAtProbeHeight);
               if (gptAtNominalHeight.isLatLonNan() || gptAtProbeHeight.isLatLonNan())
                  continue;

               gptAtProbeHeight.height(gptAtNominalHeight.height());
               const ossim_float64 horizontalMeters =
                     gptAtNominalHeight.distanceTo(gptAtProbeHeight);
               const ossim_float64 pixelsPerMeterHeight =
                     horizontalMeters /
                     (RPC_AUTO_HEIGHT_LAYER_PROBE_DELTA * meanMetersPerPixel);
               if (isFinite(pixelsPerMeterHeight) && (pixelsPerMeterHeight > 0.0))
                  maxPixelsPerMeterHeight =
                        std::max(maxPixelsPerMeterHeight, pixelsPerMeterHeight);
            }
         }
      }

      heightDelta = (maxHeight > minHeight) ?
            ((maxHeight - minHeight) * RPC_AUTO_HEIGHT_LAYER_DELTA_FRACTION) : 0.0;
      if (heightDelta > 0.0)
      {
         heightDelta = std::max(heightDelta, RPC_MIN_AUTO_HEIGHT_LAYER_DELTA);
         heightDelta = std::min(heightDelta, RPC_MAX_AUTO_HEIGHT_LAYER_DELTA);
      }
      if (maxPixelsPerMeterHeight > DBL_EPSILON)
      {
         const ossim_float64 sensitivityDelta =
               RPC_AUTO_HEIGHT_LAYER_TARGET_PIXELS / maxPixelsPerMeterHeight;
         if (isFinite(sensitivityDelta) && (sensitivityDelta > 0.0))
         {
            heightDelta = (heightDelta > 0.0) ? std::min(heightDelta, sensitivityDelta) :
                  sensitivityDelta;
         }
      }
      return true;
   }

   /**
    * @brief Solves an overdetermined least-squares system with an explicit SVD pseudo-inverse.
    *
    * @details The RPC coefficient system is commonly rank deficient, especially for single-height
    * fits where all z-dependent columns are zero. Solving the tall weighted system directly avoids
    * squaring the condition number through normal equations.
    *
    * @param a Weighted design matrix.
    * @param b Weighted observation vector.
    * @return Minimum-norm coefficient vector.
    */
   NEWMAT::ColumnVector solveLeastSquaresSvd(const NEWMAT::Matrix& a,
                                             const NEWMAT::ColumnVector& b)
   {
      NEWMAT::DiagonalMatrix singularValues;
      NEWMAT::Matrix u;
      NEWMAT::Matrix v;

      NEWMAT::SVD(a, singularValues, u, v, true, true);

      ossim_float64 maxSingularValue = 0.0;
      for (int i = 0; i < singularValues.Ncols(); ++i)
         maxSingularValue = std::max(maxSingularValue, std::fabs(singularValues[i]));

      const ossim_float64 tolerance =
            maxSingularValue * std::max(a.Nrows(), a.Ncols()) * DBL_EPSILON;

      NEWMAT::DiagonalMatrix inverseSingularValues(singularValues.Ncols());
      inverseSingularValues = 0.0;
      for (int i = 0; i < singularValues.Ncols(); ++i)
      {
         if (std::fabs(singularValues[i]) > tolerance)
         {
            inverseSingularValues[i] = 1.0 / singularValues[i];
         }
      }

      return v * inverseSingularValues * u.t() * b;
   }

   void refineRationalLm(NEWMAT::ColumnVector& coeff,
                         const std::vector<double>& f,
                         const std::vector<double>& x,
                         const std::vector<double>& y,
                         const std::vector<double>& z,
                         bool useHuber)
   {
      if (coeff.Nrows() != RPC_COEFFICIENT_COUNT)
         return;

      double currentRms = computeRationalRms(coeff, f, x, y, z);
      double huberDelta = std::max(1.0e-8, 1.5 * currentRms);
      double currentObjective = computeRationalObjective(coeff, f, x, y, z, useHuber, huberDelta);
      if (!isFinite(currentRms) || !isFinite(currentObjective) ||
          (currentRms >= DBL_MAX) || (currentObjective >= DBL_MAX))
         return;
      double lambda = 1.0e-3;

      const int observationRows = static_cast<int>(f.size());
      const int regularizationRows = RPC_POLYNOMIAL_TERM_COUNT - 1;
      const int dampingRows = RPC_COEFFICIENT_COUNT;
      const int totalRows = observationRows + regularizationRows + dampingRows;

      for (ossim_uint32 iteration = 0; iteration < RPC_NONLINEAR_MAX_ITERATIONS; ++iteration)
      {
         NEWMAT::Matrix jacobian(totalRows, RPC_COEFFICIENT_COUNT);
         NEWMAT::ColumnVector rhs(totalRows);
         jacobian = 0.0;
         rhs = 0.0;

         double terms[RPC_POLYNOMIAL_TERM_COUNT];
         for (int row = 0; row < observationRows; ++row)
         {
            setupRpcTerms(x[row], y[row], z[row], terms);
            const double numerator = evaluateRpcNumerator(coeff, terms);
            const double denominator = evaluateRpcDenominator(coeff, terms);
            const double residual = numerator / denominator - f[row];
            if (!isFinite(residual))
               continue;
            const double weight = robustWeight(residual, useHuber, huberDelta);

            rhs[row] = -weight * residual;
            for (int term = 0; term < (int)RPC_POLYNOMIAL_TERM_COUNT; ++term)
               jacobian[row][term] = weight * terms[term] / denominator;

            const double denominatorSquared = denominator * denominator;
            for (int term = 1; term < (int)RPC_POLYNOMIAL_TERM_COUNT; ++term)
            {
               jacobian[row][RPC_POLYNOMIAL_TERM_COUNT + term - 1] =
                     -weight * numerator * terms[term] / denominatorSquared;
            }
         }

         for (int term = 1; term < (int)RPC_POLYNOMIAL_TERM_COUNT; ++term)
         {
            const int row = observationRows + term - 1;
            const int coefficientIndex = RPC_POLYNOMIAL_TERM_COUNT + term - 1;
            jacobian[row][coefficientIndex] = RPC_DENOMINATOR_REGULARIZATION_WEIGHT;
            rhs[row] = -RPC_DENOMINATOR_REGULARIZATION_WEIGHT * coeff[coefficientIndex];
         }

         const double dampingWeight = std::sqrt(lambda);
         for (int coefficientIndex = 0; coefficientIndex < (int)RPC_COEFFICIENT_COUNT; ++coefficientIndex)
         {
            const int row = observationRows + regularizationRows + coefficientIndex;
            jacobian[row][coefficientIndex] = dampingWeight;
         }

         NEWMAT::ColumnVector delta = solveLeastSquaresSvd(jacobian, rhs);
         NEWMAT::ColumnVector candidate = coeff + delta;
         const double candidateObjective =
               computeRationalObjective(candidate, f, x, y, z, useHuber, huberDelta);

         if (isFinite(candidateObjective) && (candidateObjective < currentObjective))
         {
            coeff = candidate;
            const double previousObjective = currentObjective;
            currentObjective = candidateObjective;
            currentRms = computeRationalRms(coeff, f, x, y, z);
            if (!isFinite(currentRms))
               break;
            huberDelta = std::max(1.0e-8, 1.5 * currentRms);
            lambda = std::max(1.0e-12, lambda * 0.3);

            if (std::fabs(previousObjective - currentObjective) <= 1.0e-14)
               break;
         }
         else
         {
            lambda *= 10.0;
            if (lambda > 1.0e8)
               break;
         }
      }
   }
}

ossimRpcSolver::ossimRpcSolver(bool useElevation, bool useHeightAboveMSLFlag)
:  theUseElevationFlag(useElevation),
   theHeightAboveMSLFlag(useHeightAboveMSLFlag),
   theHeightLayerDelta(0.0),
   theHeightLayerRadius(0),
   theMaxIterations(MAX_SOLVE_ITERATIONS),
   theResidualImprovementTolerance(-1.0),
   theFitOptimizer(RPC_FIT_WEIGHTED_SVD),
   theMeanResidual(0),
   theMaxResidual(0),
   theFitBiasError(ossim::nan()),
   theFitRandError(ossim::nan()),
   theFitBiasErrorPixels(ossim::nan()),
   theFitRandErrorPixels(ossim::nan())
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

/**
 * @brief Samples an image geometry over imageBounds and solves RPC coefficients.
 *
 * @details Each image grid point is projected through the source geometry to establish ground
 * observations. If height layering is enabled with elevation, elevation samples first establish one
 * nominal height for the fit area and all layers are sampled as fixed-height planes around that
 * nominal height. If layering is enabled without elevation, the nominal height is 0. The resulting
 * image/ground observation vectors are then passed to the direct observation solve.
 *
 * @param imageBounds Image-space area over which the RPC approximation is fit.
 * @param geom Source image geometry to approximate.
 * @param xSamples Number of grid samples in the sample direction.
 * @param ySamples Number of grid samples in the line direction.
 */
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
   if ((imageBounds.width() <= DBL_EPSILON) ||
       (imageBounds.height() <= DBL_EPSILON))
      return;
   double Dx = imageBounds.width()/(xSamples-1);
   double Dy = imageBounds.height()/(ySamples-1);
   ossimDpt dpt;
   std::vector<ossim_float64> heightOffsets;
   ossim_float64 nominalLayerHeight = 0.0;
   ossim_float64 layerDelta = theHeightLayerDelta;
   const ossim_uint32 layerRadius = theHeightLayerRadius;
   if ((layerRadius > 0) && theUseElevationFlag)
   {
      ossim_float64 autoLayerDelta = 0.0;
      if (computeHeightStatistics(geom,
                                  imageBounds,
                                  xSamples,
                                  ySamples,
                                  theHeightAboveMSLFlag,
                                  nominalLayerHeight,
                                  autoLayerDelta) &&
          (layerDelta <= DBL_EPSILON))
      {
         layerDelta = autoLayerDelta;
      }
   }
   if ((layerRadius > 0) && (layerDelta <= DBL_EPSILON))
      layerDelta = 0.0;
   const ossim_uint32 effectiveLayerRadius =
         ((layerRadius > 0) && (layerDelta > DBL_EPSILON)) ? layerRadius : 0;
   for (int layer = -static_cast<int>(effectiveLayerRadius);
        layer <= static_cast<int>(effectiveLayerRadius);
        ++layer)
      heightOffsets.push_back(layer * layerDelta);
   const bool useNominalLayerHeight = (effectiveLayerRadius > 0);

   for(y = 0; y < ySamples; ++y)
   {
      dpt.y = y*Dy + imageBounds.ul().y;
      for(x = 0; x < xSamples; ++x)
      {
         dpt.x = x*Dx + imageBounds.ul().x;
         ossim_float64 baseHeight = 0.0;
         if (theUseElevationFlag)
            geom->localToWorld(dpt, gpt);
         else
            geom->localToWorld(dpt, 0, gpt);

         if (gpt.isLatLonNan())
            continue;

         if(gpt.isHgtNan())
            gpt.height(0.0);

         if(theHeightAboveMSLFlag)
         {
            double h = ossimElevManager::instance()->getHeightAboveMSL(gpt);
            if(isFinite(h))
               gpt.height(h);
         }
         baseHeight = useNominalLayerHeight ? nominalLayerHeight : gpt.height();
         if (!isFinite(baseHeight))
            continue;

         for (std::vector<ossim_float64>::const_iterator offset = heightOffsets.begin();
              offset != heightOffsets.end(); ++offset)
         {
            const ossim_float64 layerHeight = baseHeight + *offset;
            if (!isFinite(layerHeight))
               continue;

            geom->localToWorld(dpt, layerHeight, gpt);
            if (gpt.isLatLonNan())
               continue;
            if(gpt.isHgtNan())
               gpt.height(layerHeight);
            if (!isFinite(gpt.height()))
               continue;
            gpt.changeDatum(defaultGround.datum());

            imagePoints.push_back(dpt);
            groundPoints.push_back(gpt);
         }
      }
   }
   solveCoefficients(imagePoints, groundPoints);
   if (theRpcModel)
   {
      std::vector<ossimDpt> imageResiduals;
      imageResiduals.reserve(imagePoints.size());
      for (ossim_uint32 idx = 0; idx < imagePoints.size(); ++idx)
      {
         ossimDpt evalPt;
         evalPoint(groundPoints[idx], evalPt);
         const ossimDpt residual = evalPt - imagePoints[idx];
         if (isFinite(residual.x) && isFinite(residual.y))
            imageResiduals.push_back(residual);
      }
      theRpcModel->setMetersPerPixel(geom->getMetersPerPixel());
      updateFitErrorEstimates(imageResiduals, geom->getMetersPerPixel());
   }
}

/**
 * @brief Fits RPC coefficients from paired image and ground observations.
 *
 * @details The observations are normalized into the RPC coordinate domain, then the sample and line
 * rational functions are solved independently. The method guards underconstrained and degenerate
 * inputs, estimates RPC offsets/scales from the observation extents, and records residuals against
 * the supplied observations after fitting.
 *
 * @param imagePoints Image-space observations.
 * @param groundControlPoints Matching ground observations.
 */
void ossimRpcSolver::solveCoefficients(const std::vector<ossimDpt>& imagePoints,
                                       const std::vector<ossimGpt>& groundControlPoints)
{
   theRpcModel = 0;
   theMeanResidual = ossim::nan();
   theMaxResidual = ossim::nan();
   clearFitErrorEstimates();

   if((imagePoints.size() != groundControlPoints.size()))
      return;
   std::vector<ossimDpt> validImagePoints;
   std::vector<ossimGpt> validGroundControlPoints;
   validImagePoints.reserve(imagePoints.size());
   validGroundControlPoints.reserve(groundControlPoints.size());
   for (ossim_uint32 idx = 0; idx < imagePoints.size(); ++idx)
   {
      const bool validImage =
            isFinite(imagePoints[idx].x) &&
            isFinite(imagePoints[idx].y);
      const bool validGround =
            isFinite(groundControlPoints[idx].latd()) &&
            isFinite(groundControlPoints[idx].lond()) &&
            (groundControlPoints[idx].isHgtNan() || isFinite(groundControlPoints[idx].height()));
      if (validImage && validGround)
      {
         validImagePoints.push_back(imagePoints[idx]);
         validGroundControlPoints.push_back(groundControlPoints[idx]);
      }
   }
   if (validImagePoints.size() != imagePoints.size())
   {
      solveCoefficients(validImagePoints, validGroundControlPoints);
      return;
   }
   if (imagePoints.size() < RPC_COEFFICIENT_COUNT)
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "ossimRpcSolver::solveCoefficients WARNING: Need at least "
            << RPC_COEFFICIENT_COUNT << " observations to solve RPC coefficients. Got "
            << imagePoints.size() << "." << std::endl;
      return;
   }

   // we will first create f which holds the result of f(x,y,z).
   // This basically holds the cooresponding image point for each
   // ground control point.  One for x and a second array for y
   // int numPoints = imagePoints.size();
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
   if ((std::fabs(w) <= DBL_EPSILON) || (std::fabs(h) <= DBL_EPSILON))
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "ossimRpcSolver::solveCoefficients WARNING: Degenerate image observation bounds."
            << std::endl;
      return;
   }

   double latSum=0.0;
   double lonSum=0.0;
   double heightSum=0.0;
   ossim_uint32 validLatCount = 0;
   ossim_uint32 validLonCount = 0;
   ossim_uint32 validHeightCount = 0;

   // find the center ground  Use elevation only if its enabled
   for(c = 0; c < groundControlPoints.size();++c)
   {
      if(isFinite(groundControlPoints[c].latd()))
      {
         latSum += groundControlPoints[c].latd();
         ++validLatCount;
      }
      if(isFinite(groundControlPoints[c].lond()))
      {
         lonSum += groundControlPoints[c].lond();
         ++validLonCount;
      }
      if(!groundControlPoints[c].isHgtNan() && isFinite(groundControlPoints[c].height()))
      {
         if(theUseElevationFlag)
         {
            heightSum += groundControlPoints[c].height();
            ++validHeightCount;
         }
      }
   }
   if ((validLatCount == 0) || (validLonCount == 0))
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "ossimRpcSolver::solveCoefficients WARNING: No valid ground observations."
            << std::endl;
      return;
   }

   // set the center ground for the offset
   //
   ossimGpt centerGround(latSum/validLatCount,
                         lonSum/validLonCount,
                         (validHeightCount > 0) ? heightSum/validHeightCount : 0.0);

   // set up ground scales and deltas for normalization
   ossim_float64 deltaLat       = 0.0;
   ossim_float64 deltaLon       = 0.0;
   ossim_float64 deltaHeight    = 0.0;
   ossim_float64 maxDeltaLat    = 0.0;
   ossim_float64 maxDeltaLon    = 0.0;
   ossim_float64 maxDeltaHeight = 0.0;
   for(c = 0; c < groundControlPoints.size(); ++c)
   {
      deltaLat = (groundControlPoints[c].latd()-centerGround.latd());
      deltaLon = (groundControlPoints[c].lond()-centerGround.lond());
      if(!groundControlPoints[c].isHgtNan())
      {
         if(theUseElevationFlag)
         {
            deltaHeight = groundControlPoints[c].height() - centerGround.height();
         }
         else
         {
            deltaHeight = 0.0;
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
      if(fabs(deltaHeight) > maxDeltaHeight)
         maxDeltaHeight = fabs(deltaHeight);
   }

   bool elevationEnabled = theUseElevationFlag;

   if ((maxDeltaLat <= DBL_EPSILON) || (maxDeltaLon <= DBL_EPSILON))
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "ossimRpcSolver::solveCoefficients WARNING: Degenerate ground observation bounds."
            << std::endl;
      return;
   }

   if(maxDeltaHeight < FLT_EPSILON)
      elevationEnabled = false;
   if(maxDeltaHeight < 1.0)
      maxDeltaHeight = 1.0;

   // set the height scale to something pretty large
   if(!elevationEnabled)
   {
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
   ossim_uint32 residualCount = 0;
   std::vector<ossimDpt> imageResiduals;
   imageResiduals.reserve(imagePoints.size());

   theMaxResidual = 0;
   for (idx = 0; idx<imagePoints.size(); idx++)
   {
      ossimDpt evalPt;
      evalPoint(groundControlPoints[idx], evalPt);
      const ossimDpt residual = evalPt - imagePoints[idx];
      ossim_float64 len = residual.length();
      if (!isFinite(len))
         continue;
      if (len > theMaxResidual)
         theMaxResidual = len;
      sumSquareError += (len*len);
      imageResiduals.push_back(residual);
      ++residualCount;
   }

   // set the error
   theMeanResidual = residualCount ?
         sqrt(sumSquareError/residualCount) :
         ossim::nan();
   updateFitErrorEstimates(imageResiduals, ossimDpt(ossim::nan(), ossim::nan()));
}

/**
 * @brief Iteratively fits and validates an RPC over an image-space area.
 *
 * @details The solver starts at the minimum sampling grid, fits coefficients, and validates the
 * result at midpoint samples between the fitting grid nodes. If the maximum pixel residual exceeds
 * tolerance, the grid is refined until convergence, the max iteration count is reached, the grid
 * ceiling is reached, or max-residual improvement stalls.
 *
 * @param imageBounds Image-space area over which the RPC approximation is valid.
 * @param geom Source geometry to approximate.
 * @param tolerance Maximum allowed image residual, in pixels.
 * @return true if the maximum residual satisfies tolerance.
 */
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
   if ((w <= DBL_EPSILON) || (h <= DBL_EPSILON))
      return false;
   ossimDpt ipt, irpc;
   ossimGpt gpt;
   std::vector<ossim_float64> heightOffsets;
   const ossim_uint32 layerRadius = theHeightLayerRadius;
   ossim_float64 layerDelta = theHeightLayerDelta;
   ossim_float64 nominalLayerHeight = 0.0;
   ossim_float64 autoLayerDelta = 0.0;
   const bool haveHeightStatistics =
         (layerRadius > 0) &&
         theUseElevationFlag &&
         computeHeightStatistics(geom,
                                 imageBounds,
                                 STARTING_GRID_SIZE,
                                 STARTING_GRID_SIZE,
                                 theHeightAboveMSLFlag,
                                 nominalLayerHeight,
                                 autoLayerDelta);
   if ((layerRadius > 0) && (layerDelta <= DBL_EPSILON) && haveHeightStatistics)
      layerDelta = autoLayerDelta;
   if ((layerRadius > 0) && (layerDelta <= DBL_EPSILON))
      layerDelta = 0.0;
   const ossim_uint32 effectiveLayerRadius =
         ((layerRadius > 0) && (layerDelta > DBL_EPSILON)) ? layerRadius : 0;
   for (int layer = -static_cast<int>(effectiveLayerRadius);
        layer <= static_cast<int>(effectiveLayerRadius);
        ++layer)
      heightOffsets.push_back(layer * layerDelta);
   const bool useNominalLayerHeight = (effectiveLayerRadius > 0);

   // Start at the minimum grid size:
   ossim_uint32 xSamples = STARTING_GRID_SIZE;
   ossim_uint32 ySamples = STARTING_GRID_SIZE;

   // Loop until error is below threshold:
   bool converged = false;
   ossim_uint32 iterationCount = 0;
   ossim_float64 previousMaxResidual = DBL_MAX;
   const ossim_float64 minResidualImprovement =
         (theResidualImprovementTolerance > 0.0) ?
         theResidualImprovementTolerance :
         std::max(std::fabs(tolerance) * MIN_RESIDUAL_IMPROVEMENT_FRACTION,
                  MIN_RESIDUAL_IMPROVEMENT_PIXELS);
   const ossim_uint32 maxIterations = std::max<ossim_uint32>(1, theMaxIterations);
   while (!converged)
   {
      ++iterationCount;
      double residual = 0;
      double sumResiduals = 0;
      int numResiduals = 0;
      std::vector<ossimDpt> imageResiduals;
      theMaxResidual = 0;

      converged = true; // hope for the best and get proved otherwise below
      solveCoefficients(imageBounds, geom, xSamples, ySamples);
      if (!theRpcModel)
         return false;

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
            if (gpt.isLatLonNan())
               continue;
            if(theHeightAboveMSLFlag)
            {
               double h = ossimElevManager::instance()->getHeightAboveMSL(gpt);
               if(isFinite(h))
                  gpt.height(h);
            }

            const ossim_float64 baseHeight =
                  useNominalLayerHeight ? nominalLayerHeight : (gpt.isHgtNan() ? 0.0 : gpt.height());
            if (!isFinite(baseHeight))
               continue;
            for (std::vector<ossim_float64>::const_iterator offset = heightOffsets.begin();
                 offset != heightOffsets.end(); ++offset)
            {
               const ossim_float64 layerHeight = baseHeight + *offset;
               if (!isFinite(layerHeight))
                  continue;

               geom->localToWorld(ipt, layerHeight, gpt);
               if (gpt.isLatLonNan())
                  continue;
               if(gpt.isHgtNan())
                  gpt.height(layerHeight);
               if (!isFinite(gpt.height()))
                  continue;

               // Reverse projection using RPC:
               evalPoint(gpt, irpc);

               // Compute residual and accumulate:
               const ossimDpt imageResidual = irpc - ipt;
               residual = imageResidual.length();
               if (!isFinite(residual))
                  continue;
               if (residual > theMaxResidual)
                  theMaxResidual = residual;
               sumResiduals += residual;
               imageResiduals.push_back(imageResidual);
               ++numResiduals;
            }
         }
      }

      if (numResiduals == 0)
         return false;
      theMeanResidual = sumResiduals/numResiduals;
      updateFitErrorEstimates(imageResiduals, geom->getMetersPerPixel());
      if (theMaxResidual > tolerance)
         converged = false;

      const ossim_float64 residualImprovement = previousMaxResidual - theMaxResidual;
      const bool residualImprovementStalled =
            (previousMaxResidual != DBL_MAX) &&
            (residualImprovement >= 0.0) &&
            (residualImprovement <= minResidualImprovement);

#if 1
      { //### DEBUG BLOCK ###
         ossimNotify(ossimNotifyLevel_INFO)<<MODULE
               <<"\n            iteration: "<<iterationCount
               <<"\n   sampling grid size: ("<<xSamples<<", "<<ySamples<<")"
               <<"\n        mean residual: "<<theMeanResidual
               <<"\n         max residual: "<<theMaxResidual
               <<"\n max residual change: "
               <<((previousMaxResidual == DBL_MAX) ? 0.0 : residualImprovement)
               <<"\n            converged: "<<ossimString::toString(converged)<<endl;
      }
#endif

      if (converged)
         break;

      if (iterationCount >= maxIterations)
         break;

      if ((xSamples >= ENDING_GRID_SIZE) && (ySamples >= ENDING_GRID_SIZE))
         break;

      if (residualImprovementStalled)
         break;

      previousMaxResidual = theMaxResidual;

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

   if (!converged)
   {
      ossimNotify(ossimNotifyLevel_WARN)
            << "WARNING: Unable to converge on desired error tolerance ("<<tolerance<<" p).\n"
            <<"    Mean residual error: " << theMeanResidual << "\n"
            <<"    Max residual error: " << theMaxResidual<<std::endl;
   }

   // Initialize metadata:
   theRpcModel->setMetersPerPixel(geom->getMetersPerPixel());
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

double ossimRpcSolver::getRpcFitBiasError() const
{
   return theFitBiasError;
}

double ossimRpcSolver::getRpcFitRandError() const
{
   return theFitRandError;
}

double ossimRpcSolver::getRpcFitBiasErrorInPixels() const
{
   return theFitBiasErrorPixels;
}

double ossimRpcSolver::getRpcFitRandErrorInPixels() const
{
   return theFitRandErrorPixels;
}

void ossimRpcSolver::clearFitErrorEstimates()
{
   theFitBiasError = ossim::nan();
   theFitRandError = ossim::nan();
   theFitBiasErrorPixels = ossim::nan();
   theFitRandErrorPixels = ossim::nan();
}

void ossimRpcSolver::updateFitErrorEstimates(const std::vector<ossimDpt>& imageResiduals,
                                             const ossimDpt& metersPerPixel)
{
   if (imageResiduals.empty())
   {
      clearFitErrorEstimates();
      return;
   }

   ossim_float64 meanSampleResidual = 0.0;
   ossim_float64 meanLineResidual = 0.0;
   for (std::vector<ossimDpt>::const_iterator residual = imageResiduals.begin();
        residual != imageResiduals.end();
        ++residual)
   {
      meanSampleResidual += residual->x;
      meanLineResidual += residual->y;
   }
   meanSampleResidual /= imageResiduals.size();
   meanLineResidual /= imageResiduals.size();

   ossim_float64 sumPixelScatterSquared = 0.0;
   for (std::vector<ossimDpt>::const_iterator residual = imageResiduals.begin();
        residual != imageResiduals.end();
        ++residual)
   {
      const ossim_float64 sampleScatter = residual->x - meanSampleResidual;
      const ossim_float64 lineScatter = residual->y - meanLineResidual;
      sumPixelScatterSquared += sampleScatter*sampleScatter + lineScatter*lineScatter;
   }

   theFitBiasErrorPixels =
         std::sqrt(meanSampleResidual*meanSampleResidual +
                   meanLineResidual*meanLineResidual);
   theFitRandErrorPixels =
         std::sqrt(sumPixelScatterSquared / imageResiduals.size());

   const bool haveMetersPerPixel =
         isFinite(metersPerPixel.x) &&
         isFinite(metersPerPixel.y) &&
         (std::fabs(metersPerPixel.x) > DBL_EPSILON) &&
         (std::fabs(metersPerPixel.y) > DBL_EPSILON);
   if (!haveMetersPerPixel)
   {
      theFitBiasError = ossim::nan();
      theFitRandError = ossim::nan();
      return;
   }

   const ossim_float64 sampleMetersPerPixel = std::fabs(metersPerPixel.x);
   const ossim_float64 lineMetersPerPixel = std::fabs(metersPerPixel.y);
   const ossim_float64 meanSampleResidualMeters =
         meanSampleResidual * sampleMetersPerPixel;
   const ossim_float64 meanLineResidualMeters =
         meanLineResidual * lineMetersPerPixel;
   ossim_float64 sumMeterScatterSquared = 0.0;
   for (std::vector<ossimDpt>::const_iterator residual = imageResiduals.begin();
        residual != imageResiduals.end();
        ++residual)
   {
      const ossim_float64 sampleScatterMeters =
            (residual->x - meanSampleResidual) * sampleMetersPerPixel;
      const ossim_float64 lineScatterMeters =
            (residual->y - meanLineResidual) * lineMetersPerPixel;
      sumMeterScatterSquared +=
            sampleScatterMeters*sampleScatterMeters +
            lineScatterMeters*lineScatterMeters;
   }

   static const ossim_float64 MAX_NITF_RPC_ERROR_METERS = 9999.99;
   theFitBiasError =
         std::min(MAX_NITF_RPC_ERROR_METERS,
                  std::sqrt(meanSampleResidualMeters*meanSampleResidualMeters +
                            meanLineResidualMeters*meanLineResidualMeters));
   theFitRandError =
         std::min(MAX_NITF_RPC_ERROR_METERS,
                  std::sqrt(sumMeterScatterSquared / imageResiduals.size()));

   if (theRpcModel.valid() &&
       isFinite(theFitBiasError) &&
       isFinite(theFitRandError))
   {
      theRpcModel->setPositionError(theFitBiasError, theFitRandError, true);
   }
}

void ossimRpcSolver::setHeightLayerDelta(ossim_float64 delta)
{
   theHeightLayerDelta = delta;
}

ossim_float64 ossimRpcSolver::getHeightLayerDelta() const
{
   return theHeightLayerDelta;
}

void ossimRpcSolver::setHeightLayerRadius(ossim_uint32 radius)
{
   theHeightLayerRadius = radius;
}

ossim_uint32 ossimRpcSolver::getHeightLayerRadius() const
{
   return theHeightLayerRadius;
}

void ossimRpcSolver::setMaxIterations(ossim_uint32 iterations)
{
   theMaxIterations = std::max<ossim_uint32>(1, iterations);
}

ossim_uint32 ossimRpcSolver::getMaxIterations() const
{
   return theMaxIterations;
}

void ossimRpcSolver::setResidualImprovementTolerance(ossim_float64 tolerance)
{
   theResidualImprovementTolerance = (tolerance > 0.0) ? tolerance : -1.0;
}

ossim_float64 ossimRpcSolver::getResidualImprovementTolerance() const
{
   return theResidualImprovementTolerance;
}

void ossimRpcSolver::setFitOptimizer(RpcFitOptimizer optimizer)
{
   theFitOptimizer = optimizer;
}

ossimRpcSolver::RpcFitOptimizer ossimRpcSolver::getFitOptimizer() const
{
   return theFitOptimizer;
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

/**
 * @brief Solves one RPC rational expression from normalized observations.
 *
 * @details This method solves either the sample or line rational polynomial. It iteratively updates
 * denominator-based weights, solves the weighted SVD system, and uses light Tikhonov damping on the
 * denominator coefficients. The damping favors denominators near 1 unless denominator terms are
 * needed by the data, which reduces the chance of sparse-grid rational poles.
 *
 * @param coeff Output 39-element coefficient vector. Elements 0-19 are numerator coefficients and
 * elements 20-38 are denominator coefficients 1-19. Denominator coefficient 0 is fixed to 1.
 * @param f Normalized image coordinate observations, either sample or line.
 * @param x Normalized longitude observations.
 * @param y Normalized latitude observations.
 * @param z Normalized height observations.
 */
void ossimRpcSolver::solveCoefficients(NEWMAT::ColumnVector& coeff,
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

   NEWMAT::ColumnVector tempCoeff;
   NEWMAT::DiagonalMatrix weights((int)f.size());
   NEWMAT::ColumnVector denominator(20);
   
   // sets up the matrix to hold the system of equations
   setupSystemOfEquations(m, r, x, y, z);

   // initialize the weight matrix to the identity
   //
   for(idx = 0; idx < f.size(); ++idx)
   {
      weights[idx] = 1.0;
   }

   double previousResidualValue = 1.0/FLT_EPSILON;
   double residualValue = previousResidualValue;
   ossim_uint32 iterations = 0;
   NEWMAT::ColumnVector previousCoeff;
   const double denominatorEpsilon = 1.0e-6;
   const double residualTolerance = 1.0e-12;
   const double coefficientTolerance = 1.0e-12;
   do
   {
      NEWMAT::Matrix weightedM(
            m.Nrows() + RPC_POLYNOMIAL_TERM_COUNT - 1,
            m.Ncols());
      NEWMAT::ColumnVector weightedR(
            r.Nrows() + RPC_POLYNOMIAL_TERM_COUNT - 1);
      weightedM = 0.0;
      weightedR = 0.0;
      for (int row = 0; row < m.Nrows(); ++row)
      {
         weightedR[row] = weights[row] * r[row];
         for (int col = 0; col < m.Ncols(); ++col)
            weightedM[row][col] = weights[row] * m[row][col];
      }
      for (int term = 1; term < RPC_POLYNOMIAL_TERM_COUNT; ++term)
      {
         const int row = m.Nrows() + term - 1;
         weightedM[row][RPC_POLYNOMIAL_TERM_COUNT + term - 1] =
               RPC_DENOMINATOR_REGULARIZATION_WEIGHT;
      }

      previousCoeff = tempCoeff;
      tempCoeff = solveLeastSquaresSvd(weightedM, weightedR);

      // set up the weight matrix by using the denominator
      for(idx = 0; idx < RPC_POLYNOMIAL_TERM_COUNT-1; ++idx)
      {
         denominator[idx+1] = tempCoeff[RPC_POLYNOMIAL_TERM_COUNT+idx];
      }
      denominator[0] = 1.0;
      
      setupWeightMatrix(weights, denominator, r, x, y, z);

      double sumSquareResidual = 0.0;
      for (idx = 0; idx < f.size(); ++idx)
      {
         double numerator = 0.0;
         double denom = 0.0;
         for (int term = 0; term < RPC_POLYNOMIAL_TERM_COUNT; ++term)
         {
            numerator += m[idx][term] * tempCoeff[term];
            denom += m[idx][term] * denominator[term];
         }
         if (std::fabs(denom) < denominatorEpsilon)
            denom = (denom < 0.0) ? -denominatorEpsilon : denominatorEpsilon;
         const double estimate = numerator / denom;
         const double delta = estimate - f[idx];
         if (!isFinite(delta))
         {
            sumSquareResidual = DBL_MAX;
            break;
         }
         sumSquareResidual += delta * delta;
      }
      residualValue = isFinite(sumSquareResidual) ?
            sqrt(sumSquareResidual/f.size()) :
            DBL_MAX;

      double coefficientDelta = 1.0/FLT_EPSILON;
      if (previousCoeff.Nrows() == tempCoeff.Nrows())
      {
         coefficientDelta = 0.0;
         for (idx = 0; idx < (ossim_uint32)tempCoeff.Nrows(); ++idx)
         {
            const double delta = tempCoeff[idx] - previousCoeff[idx];
            if (!isFinite(delta))
            {
               coefficientDelta = DBL_MAX;
               break;
            }
            coefficientDelta += delta * delta;
         }
         if (coefficientDelta != DBL_MAX)
            coefficientDelta = sqrt(coefficientDelta);
      }

      ++iterations;

      if ((residualValue <= residualTolerance) ||
          (coefficientDelta <= coefficientTolerance) ||
          (std::fabs(previousResidualValue-residualValue) <= residualTolerance))
      {
         break;
      }
      previousResidualValue = residualValue;

   } while (iterations < 15);

   if (theFitOptimizer == RPC_FIT_LM)
   {
      refineRationalLm(tempCoeff, f, x, y, z, false);
   }
   else if (theFitOptimizer == RPC_FIT_LM_HUBER)
   {
      refineRationalLm(tempCoeff, f, x, y, z, true);
   }

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


/**
 * @brief Builds the linearized rational polynomial design matrix.
 *
 * @details Each row represents N(x,y,z) - f*D'(x,y,z) = f, where D' omits the fixed denominator
 * coefficient 0. This yields 39 unknowns: 20 numerator coefficients and 19 denominator
 * coefficients.
 */
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

/**
 * @brief Computes reciprocal denominator weights for the iterative rational fit.
 *
 * @details Weighting by 1 / abs(D) converts the linearized rational fit toward the nonlinear
 * residual being minimized. The denominator is clamped to avoid infinite weights when an
 * intermediate solution approaches a pole.
 */
void ossimRpcSolver::setupWeightMatrix(NEWMAT::DiagonalMatrix& result, // holds the resulting weights
                                       const NEWMAT::ColumnVector& coefficients,
                                       const NEWMAT::ColumnVector& f,
                                       const std::vector<double>& x,
                                       const std::vector<double>& y,
                                       const std::vector<double>& z)const
{
   result.ReSize(f.Nrows());
   const double denominatorEpsilon = 1.0e-6;
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

      if (!isFinite(result[idx]))
      {
         result[idx] = 1.0;
         continue;
      }
      result[idx] = 1.0 / std::max(std::fabs(result[idx]), denominatorEpsilon);
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

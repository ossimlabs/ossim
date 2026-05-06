//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************
#ifndef ossimRpcSolver_HEADER
#define ossimRpcSolver_HEADER

#include <vector>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/matrix/newmat.h>
#include <ossim/projection/ossimRpcModel.h>
#include <ossim/projection/ossimRpcProjection.h>
#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <ossim/imaging/ossimImageGeometry.h>

/**
 * @brief Fits an ossimRpcModel approximation to an image geometry or observation set.
 *
 * @details The solver samples image-space points, projects them through the source geometry to
 * ground, normalizes image and ground coordinates to the RPC offset/scale domain, and solves the
 * RPC00B rational polynomial coefficients for line and sample. The coefficient solve uses weighted
 * SVD least squares with light denominator regularization to avoid unstable rational poles in
 * sparse or nearly flat-height fits.
 *
 * The public solve() method validates the fit at midpoint samples, refines the sampling grid as
 * needed, and stops when one of these conditions is met:
 * - The maximum pixel residual satisfies the requested tolerance.
 * - The configured iteration limit is reached.
 * - The configured grid limit is reached.
 * - The maximum residual improvement stalls.
 *
 * @par Common fitting modes
 * - Elevation-aware fitting: enable elevation and use the source geometry/DEM heights. This is the
 *   normal path for RPC generation when elevation is available.
 * - Single-height fitting: disable elevation and all observations are fit at height 0. This is
 *   primarily useful for diagnostics and flat image-plane comparisons.
 * - Layered-height fitting: enable a height layer delta/radius to fit several constant-height
 *   planes about a nominal center height. With elevation enabled, the nominal height is estimated
 *   from the fit area first, and the layer delta can be estimated from sampled height variation
 *   when only a radius is supplied. The automatic delta is capped by image height sensitivity so
 *   small-GSD or oblique images do not receive an overly large height slab. Without elevation, the
 *   nominal height is 0 and an explicit delta is required for layered fitting. This helps produce
 *   an RPC that remains stable over height changes instead of only matching the terrain height
 *   sampled at each image point. It also reduces downstream dependence on using the exact same
 *   elevation database used during fitting, since the generated RPC has already been fit across a
 *   small height slab around the scene.
 * - Optimizer comparison: use setFitOptimizer() to compare the default weighted SVD solve against
 *   nonlinear LM refinement and a Huber-weighted LM refinement.
 *
 * @par Polynomial format
 * This class currently emits RPC00B polynomial ordering. For each coordinate, the numerator has
 * 20 terms and the denominator has 20 terms with denominator coefficient 0 fixed to 1, producing
 * the standard line numerator, line denominator, sample numerator, and sample denominator sets.
 *
 * @par Solver-derived RPC error estimates
 * The solver tracks signed residuals between the source geometry and the fitted RPC. These
 * residuals describe only the RPC approximation error relative to the input model. They are not an
 * independent absolute geolocation accuracy statement unless the input model is itself truth. The
 * reported bias error is the magnitude of the mean residual vector, and the reported random error
 * is the RMS scatter after removing that mean residual. When meters-per-pixel is available from an
 * input image geometry, the meter-valued estimates are stored on the solved ossimRpcModel and are
 * written to the RPC00B ERR_BIAS and ERR_RAND fields.
 *
 * @note In the polynomial term listing below, x=longitude, y=latitude, z=height after
 * normalization to RPC coordinates.
 *
 * @code
 * coeff[ 0]       + coeff[ 1]*x     + coeff[ 2]*y     + coeff[ 3]*z     +
 * coeff[ 4]*x*y   + coeff[ 5]*x*z   + coeff[ 6]*y*z   + coeff[ 7]*x*x   +
 * coeff[ 8]*y*y   + coeff[ 9]*z*z   + coeff[10]*x*y*z + coeff[11]*x*x*x +
 * coeff[12]*x*y*y + coeff[13]*x*z*z + coeff[14]*x*x*y + coeff[15]*y*y*y +
 * coeff[16]*y*z*z + coeff[17]*x*x*z + coeff[18]*y*y*z + coeff[19]*z*z*z
 * @endcode
 *
 * @par Example: fit from an existing image geometry
 * @code
 * ossimRpcSolver solver(useElevation);
 * solver.setHeightLayerRadius(1);  // Radius 1 means 3 total planes.
 * solver.setHeightLayerDelta(0.0); // <= 0 requests automatic delta when elevation is enabled.
 *
 * if (solver.solve(imageBounds, geom, 0.5))
 * {
 *    ossimRefPtr<ossimRpcModel> rpc = solver.getRpcModel();
 * }
 * @endcode
 *
 * @par Example: auto layered fit from an existing image geometry
 * @code
 * ossimRpcSolver solver(true);
 * solver.setHeightLayerRadius(1);  // Estimate nominal height and auto delta from the fit area.
 * solver.solve(imageBounds, geom, 0.5);
 * @endcode
 *
 * @par Example: fit from explicit observations
 * @code
 * solver.solveCoefficients(imagePoints, groundPoints);
 * ossimRefPtr<ossimRpcModel> rpc = solver.getRpcModel();
 * @endcode
 *
 * @note The image and ground observation vectors must have equal size. At least 39 observations
 * are required to solve the 39 unknown coefficients for each rational expression.
 *
 * @note A sub-image bounding rect constrains the solution to that area, but sampled image
 * coordinates remain in the source image coordinate system. If the resulting RPC will accompany
 * a chip whose local upper-left image coordinate is (0,0), call
 * rpcModel->setImageOffset(chip_offset) before writing the chip geometry.
 */
class OSSIM_DLL ossimRpcSolver : public ossimReferenced
{
public:
   enum RpcFitOptimizer
   {
      RPC_FIT_WEIGHTED_SVD = 0,
      RPC_FIT_LM           = 1,
      RPC_FIT_LM_HUBER     = 2
   };

   /**
    * @brief Constructs an RPC solver.
    *
    * @param useElevation If true, ground observation heights are used when solving the RPC.
    * If false, observations are fit as a single height-0 surface.
    * @param useHeightAboveMSLFlag If true, sampled heights are converted to height above MSL
    * when the elevation manager can provide a value.
    *
    * @note NaN heights are replaced with 0.0 before fitting.
    */
   ossimRpcSolver(bool useElevation=false,
                  bool useHeightAboveMSLFlag=false);

   virtual ~ossimRpcSolver(){}

   /**
    * @brief Samples an image projection and solves an RPC model.
    *
    * @param imageBounds Image-space area to sample.
    * @param imageProj Source projection to approximate.
    * @param xSamples Number of grid samples in the sample direction.
    * @param ySamples Number of grid samples in the line direction.
    */
   void solveCoefficients(const ossimDrect& imageBounds,
                          ossimProjection* imageProj,
                          ossim_uint32 xSamples=8,
                          ossim_uint32 ySamples=8);

   /**
    * @brief Samples an image geometry and solves an RPC model.
    *
    * @details This overload honors the solver's elevation and height-layer settings when producing
    * image/ground observations for the coefficient fit.
    *
    * @param imageBounds Image-space area to sample.
    * @param geom Source image geometry to approximate.
    * @param xSamples Number of grid samples in the sample direction.
    * @param ySamples Number of grid samples in the line direction.
    */
   void solveCoefficients(const ossimDrect& imageBounds,
                          ossimImageGeometry* geom,
                          ossim_uint32 xSamples=8,
                          ossim_uint32 ySamples=8);

   /**
    * @brief Iteratively solves an RPC model and validates midpoint residuals.
    *
    * @details The RPC is computed for the specified image bounds only. When imageBounds is smaller
    * than the full valid image rect, this is intended for generating an RPC to accompany a subimage
    * written to disk.
    *
    * @param aoiBounds The AOI in image space for the RPC computation.
    * @param geom Geometry of the input image.
    * @param pixel_tolerance Maximum image-space residual in pixels.
    * @return true if solution converged below pixel tolerance.
    */
   bool solve(const ossimDrect& aoiBounds,
              ossimImageGeometry* geom,
              const double& pixel_tolerance=0.5);

   /**
    * @brief Iteratively solves an RPC model for an image file's full image rectangle.
    *
    * @param imageFilename Image file used to initialize the source geometry.
    * @param pixel_tolerance Maximum image-space residual in pixels.
    * @return true if solution converged below pixel tolerance.
    */
   bool solve(const ossimFilename& imageFilename,
              const double& pixel_tolerance=0.5);

   /**
    * @brief Solves RPC coefficients from paired image and ground observations.
    *
    * @details All data is normalized to the RPC offset/scale domain for numerical robustness.
    *
    * @param imagePoints Image-space observations.
    * @param groundControlPoints Matching ground observations.
    */ 
   void solveCoefficients(const std::vector<ossimDpt>& imagePoints,
                          const std::vector<ossimGpt>& groundControlPoints);

   /**
    * Fetches the solved-for RPC model. See note in header above on setting the image offset
    * if this model will be applied to a sub-image chip.
    */
   const ossimRefPtr<ossimRpcModel> getRpcModel() const { return theRpcModel; }

   double getRmsError()const;
   double getMaxError()const;

   /**
    * @return Solver-derived RPC approximation bias error in meters.
    *
    * @details This is the magnitude of the mean signed validation residual converted to meters.
    * It maps to RPC00B ERR_BIAS when a solved RPC model is serialized. Returns NaN when no
    * meter-scale validation residuals have been computed.
    */
   double getRpcFitBiasError() const;

   /**
    * @return Solver-derived RPC approximation random error in meters.
    *
    * @details This is the RMS signed validation residual scatter after removing the mean residual,
    * converted to meters. It maps to RPC00B ERR_RAND when a solved RPC model is serialized.
    * Returns NaN when no meter-scale validation residuals have been computed.
    */
   double getRpcFitRandError() const;

   /**
    * @return Solver-derived RPC approximation bias error in pixels.
    *
    * @details This is available for explicit observation solves where no meters-per-pixel value is
    * known. It is not written directly to RPC00B.
    */
   double getRpcFitBiasErrorInPixels() const;

   /**
    * @return Solver-derived RPC approximation random error in pixels.
    *
    * @details This is available for explicit observation solves where no meters-per-pixel value is
    * known. It is not written directly to RPC00B.
    */
   double getRpcFitRandErrorInPixels() const;

   /**
    * @brief Sets the height-layer spacing, in meters, used for layered RPC fitting and validation.
    *
    * @details Layered fitting is controlled primarily by setHeightLayerRadius(). When radius is
    * positive, observations are sampled at nominal center height plus integer multiples of delta.
    * With elevation enabled, the nominal center height is estimated from the fit area and a
    * delta <= 0 requests automatic delta estimation from sampled height variation and image height
    * sensitivity. With elevation disabled, provide a positive delta if layered fitting is desired.
    */
   void setHeightLayerDelta(ossim_float64 delta);
   ossim_float64 getHeightLayerDelta() const;

   /**
    * @brief Sets the number of height layers to sample on each side of the nominal center height.
    *
    * @details A radius of 0 disables layered fitting. A radius of 1 samples 3 total planes
    * (-delta, 0, +delta), radius 2 samples 5 total planes, etc. If elevation is enabled and no
    * positive delta is supplied, the solver estimates a delta from fit-area height variation and
    * caps it by image height sensitivity.
    */
   void setHeightLayerRadius(ossim_uint32 radius);
   ossim_uint32 getHeightLayerRadius() const;

   /**
    * Sets the maximum number of outer fit/validate iterations. Each iteration fits an RPC
    * at the current sampling grid size, validates midpoint residuals, and may refine the grid.
    */
   void setMaxIterations(ossim_uint32 iterations);
   ossim_uint32 getMaxIterations() const;

   /**
    * Sets the minimum max-residual improvement, in pixels, needed to continue refining.
    * A value <= 0 uses the solver default derived from the requested pixel tolerance.
    */
   void setResidualImprovementTolerance(ossim_float64 tolerance);
   ossim_float64 getResidualImprovementTolerance() const;

   /**
    * @brief Selects the coefficient optimizer used for each rational expression.
    *
    * @details RPC_FIT_WEIGHTED_SVD is the default linearized, iteratively reweighted SVD solve.
    * RPC_FIT_LM starts from that solution and refines the actual nonlinear rational residual with
    * a damped Levenberg-Marquardt style step. RPC_FIT_LM_HUBER applies the same nonlinear
    * refinement with Huber residual weighting.
    */
   void setFitOptimizer(RpcFitOptimizer optimizer);
   RpcFitOptimizer getFitOptimizer() const;

   /**
    * @return ossimRefPtr<ossimNitfRegisteredTag>
    *
    * @note one of the solve methods should have been called prior to this.
    */
   ossimRefPtr<ossimNitfRegisteredTag> getNitfRpcBTag() const;
   
   /**
    * Sets the image rect over which to compute the RPCs. The Resulting RPC will only be valid
    * over that range of image space. */
   void setValidImageRect(const ossimIrect& imageRect);

protected:
   virtual void solveInitialCoefficients(NEWMAT::ColumnVector& coeff,
                                         const std::vector<double>& f,
                                         const std::vector<double>& x,
                                         const std::vector<double>& y,
                                         const std::vector<double>& z)const;
                                         
   virtual void solveCoefficients(NEWMAT::ColumnVector& coeff,
                                  const std::vector<double>& f,
                                  const std::vector<double>& x,
                                  const std::vector<double>& y,
                                  const std::vector<double>& z)const;
   
   double eval(const std::vector<double>& coeff,
               const double& x, const double& y, const double& z)const;

   void evalPoint(const ossimGpt& gpt, ossimDpt& ipt) const;

   /**
    * Updates solver and RPC model error estimates from signed image residuals. When metersPerPixel
    * contains finite positive values, meter estimates are also computed and stored on the RPC model.
    */
   void updateFitErrorEstimates(const std::vector<ossimDpt>& imageResiduals,
                                const ossimDpt& metersPerPixel);

   void clearFitErrorEstimates();

   /**
    * Inverts using the SVD method
    */
   NEWMAT::Matrix invert(const NEWMAT::Matrix& m)const;
   
   void setupSystemOfEquations(NEWMAT::Matrix& equations,
                               const NEWMAT::ColumnVector& f,
                               const std::vector<double>& x,
                               const std::vector<double>& y,
                               const std::vector<double>& z)const;

   void setupWeightMatrix(NEWMAT::DiagonalMatrix& result, // holds the resulting weights
                          const NEWMAT::ColumnVector& coefficients,
                          const NEWMAT::ColumnVector& f,
                          const std::vector<double>& x,
                          const std::vector<double>& y,
                          const std::vector<double>& z)const;

   bool theUseElevationFlag;
   bool theHeightAboveMSLFlag;
   ossim_float64 theHeightLayerDelta;
   ossim_uint32 theHeightLayerRadius;
   ossim_uint32 theMaxIterations;
   ossim_float64 theResidualImprovementTolerance;
   RpcFitOptimizer theFitOptimizer;
   ossim_float64 theMeanResidual;
   ossim_float64 theMaxResidual;
   ossim_float64 theFitBiasError;
   ossim_float64 theFitRandError;
   ossim_float64 theFitBiasErrorPixels;
   ossim_float64 theFitRandErrorPixels;
   ossimRefPtr<ossimImageGeometry> theRefGeom;
   ossimRefPtr<ossimRpcModel> theRpcModel;

};

#endif

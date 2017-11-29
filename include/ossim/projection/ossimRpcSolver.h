//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************
#ifndef ossimRpcSolver_HEADER
#define ossimRpcSolver_HEADER

#include <vector>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/matrix/newmat.h>
#include <ossim/projection/ossimRpcModel.h>
#include <ossim/projection/ossimRpcProjection.h>

class ossimProjection;
class ossimImageGeometry;
class ossimNitfRegisteredTag;

/**
 * This currently only support Rational poilynomial B format.  This can be
 * found in the NITF registered commercial tag document.
 *
 * @note x=longitude, y=latitude, z=height
 * 
 * <pre>
 * Format is:
 *  coeff[ 0]       + coeff[ 1]*x     + coeff[ 2]*y     + coeff[ 3]*z     +
 *  coeff[ 4]*x*y   + coeff[ 5]*x*z   + coeff[ 6]*y*z   + coeff[ 7]*x*x   +
 *  coeff[ 8]*y*y   + coeff[ 9]*z*z   + coeff[10]*x*y*z + coeff[11]*x*x*x +
 *  coeff[12]*x*y*y + coeff[13]*x*z*z + coeff[14]*x*x*y + coeff[15]*y*y*y +
 *  coeff[16]*y*z*z + coeff[17]*x*x*z + coeff[18]*y*y*z + coeff[19]*z*z*z;
 *
 *       where coeff is one of XNum, XDen, YNum, and YDen.  So there are 80
 *       coefficients all together.
 *
 *       
 * Currently we use a linear least squares fit to solve the coefficients.
 * This is the simplest to implement.  We probably relly need a nonlinear
 * minimizer to fit the coefficients but I don't have time to experiment.
 * Levenberg Marquardt might be a solution to look into.
 *
 *
 * 
 * HOW TO USE:
 * 
 *        ossimRpcSolver solver;
 *        solver.solveCoefficients(rect,
 *                                 *proj.get());
 *                                 
 * We can also call solve coefficients with a list of ground control points.
 * First is the list of image points followed by the ground points.
 * NOTE: Thes must be equal in size.
 * 
 *        solver.solveCoefficients(imagePoints,
 *                                 groundPoints);
 *                                 
 *                                 
 * Once you call solveCoefficients you can create the projector:
 *                                 
 *        ossimRefPtr<ossimRpcProjection> rpc = solver.createRpcProjection();
 *
 * </pre>
 * 
 */ 
class OSSIM_DLL ossimRpcSolver : public ossimReferenced
{
public:
   /**
    * The use elvation flag will deterimne if we force the height t be 0.
    * If the elevation is enabled then we use the height field of the control
    * points to determine the coefficients of the RPC00 polynomial.  If its
    * false then we will ignore the height by setting the height field to 0.0.
    *
    * Note:  even if the elevation is enabled all NAN heights are set to 0.0.
    */
   ossimRpcSolver(bool useElevation=false,
                  bool useHeightAboveMSLFlag=false);

   
   /**
    * This will convert any projector to an RPC model
    */
   void solveCoefficients(const ossimDrect& imageBounds,
                          ossimProjection* imageProj,
                          ossim_uint32 xSamples=8,
                          ossim_uint32 ySamples=8);
   
   void solveCoefficients(const ossimDrect& imageBounds,
                          ossimImageGeometry* geom,
                          ossim_uint32 xSamples=8,
                          ossim_uint32 ySamples=8);

   /**
    * Similar to the other solve methods except that the final grid size is established
    * iteratively so that the error at the midpoints between grid nodes falls below tolerance.
    * The RPC is computed for the specified image bounds range only, not the full image. The
    * expectation here (when the imageBounds is less than the full valid image rect) is to
    * generate an RPC to accompany a subimage that will be written to disk.
    * @param imageBounds The AOI in image space for the RPC computation.
    * @param geom Represents the geometry of the input image
    * @param pixel_tolerance Maximum error in pixels (typically fraction of a pixel) to achieve.
    * @return true if solution converged below pixel tolerance.
    */
   bool solve(const ossimDrect& aoiBounds,
              ossimImageGeometry* geom,
              const double& pixel_tolerance=0.5);

   /**
    * takes associated image points and ground points
    * and solves the coefficents for the rational polynomial for
    * line and sample calculations from world points.
    *
    * Note: All data will be normalized between -1 and 1 for
    *       numerical robustness.
    */ 
   void solveCoefficients(const std::vector<ossimDpt>& imagePoints,
                          const std::vector<ossimGpt>& groundControlPoints);

   /**
    * Creates and Rpc model from the coefficients
    */
   const ossimRefPtr<ossimRpcModel> getRpcModel() const { return theRpcModel; }

   double getRmsError()const;
   double getMaxError()const;

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
	virtual ~ossimRpcSolver(){}
   
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
   ossim_float64 theMeanResidual;
   ossim_float64 theMaxResidual;
   ossimRefPtr<ossimImageGeometry> theRefGeom;
   ossimRefPtr<ossimRpcModel> theRpcModel;


};

#endif

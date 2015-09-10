//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts (gpotts@imagelinks.com
//
// Description: Source code produced by Dave Knopp
//
//*******************************************************************
//  $Id: ossimLeastSquaresPlane.h 23167 2015-02-24 22:07:14Z okramer $
#ifndef    ossimLeastSquaresPlane_INCLUDE
#define    ossimLeastSquaresPlane_INCLUDE
#include <ossim/base/ossimConstants.h>
#include <ossim/matrix/newmat.h>

/** 
 * @brief Provide 2D Least Squares Plane model fitting
 * The math model is that of a plane of the form:
@code
z(x,y) = ax + b*y + c
@endcode
 * The getLSParms() method returns parameter values which are the least
 * squares solution associated with the samples added via addSample(). Note
 * that it is necessary to add at least three sample to obtain a solution.
 */

class OSSIMDLLEXPORT ossimLeastSquaresPlane
{
public:

   ossimLeastSquaresPlane(const ossimLeastSquaresPlane &);
   /** 
    * Instantiate as zero surface.
    */
   ossimLeastSquaresPlane();
   
   ossimLeastSquaresPlane & operator = (const ossimLeastSquaresPlane &);
   
   /**
    * Free internal storage.
    */
   virtual ~ossimLeastSquaresPlane();

   /**
    * Will clear everything and set it up to
    * for another solve.  Just add points
    * and call the solve method.
    */
   virtual void clear();

   /**
    * add a single data sample.
    *
    * @param x coordinate of sample location.
    * @param y coordinate of sample location.
    * @param zmea sample value measured at (x,y)
    */
   virtual void addSample(double x, double y, double z_mea);
   
   /**
    * return LS solution parameters.
    *
    * @param pa set to x coefficient.
    * @param pb set to y coefficient
    * @param pc set to constant term
    */
   virtual bool getLSParms(double& pa, double& pb, double& pc) const;

   /**
    * @param pa set to x coefficient.
    * @param pb set to y coefficient
    * @param pc set to constant term
    */
   virtual void setLSParams(double pa, double pb, double pc);
                           
   /**
    * interpolate LS-fit value at location (xx,yy) - returns z(xx,yy).
    *
    * @param xx "x" coordinate at which to interpolate.
    * @param yy "y" "y" coordinate at which to interpolate.
    * 
    */
   virtual inline double lsFitValue(double xx, double yy) const { return (m_a*xx + m_b*yy + m_c); }
   
   /**
    * compute least squares parameter solution - true if succesfull.
    */
   bool solveLS();
   
private:

   /**
    * linear-X term.
    */
   double m_a;

   /**
    * linear-Y term.
    */
   double m_b;

   /**
    * constant term.
    */
   double m_c;

   /**
    * Normal system coefficient matrix.
    */
   NEWMAT::Matrix*  AtA;

   /**
    * Normal system RHS vector
    */
   NEWMAT::Matrix*  Atb;

   ossim_uint32 m_numSamples;
};

#endif


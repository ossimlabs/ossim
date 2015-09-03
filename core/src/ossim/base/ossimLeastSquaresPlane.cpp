//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Description: Source code produced by Dave Knopp
//
//*******************************************************************
//  $Id: ossimLeastSquaresPlane.cpp 23167 2015-02-24 22:07:14Z okramer $

#include <ossim/base/ossimLeastSquaresPlane.h>
#include <iostream>  // for debugging
#include <ossim/base/ossimNotifyContext.h>

using namespace std;

ossimLeastSquaresPlane::ossimLeastSquaresPlane()
: m_a(0.0),
  m_b(0.0),
  m_c(0.0),
  AtA(NULL),
  Atb(NULL),
  m_numSamples(0)
{
   // allocate normal system accumulation matrices
   AtA = new NEWMAT::Matrix(3,3);
   Atb = new NEWMAT::Matrix(3,1);
   
   // ensure initilization to zero
   *AtA = 0.0;
   *Atb = 0.0;
   
   return;
}

ossimLeastSquaresPlane::ossimLeastSquaresPlane(const ossimLeastSquaresPlane &rhs)
{
   // allocate normal system accumulation matrices
   AtA = new NEWMAT::Matrix(3,3);
   Atb = new NEWMAT::Matrix(3,1);

   m_a = rhs.m_a;
   m_b = rhs.m_b;
   m_c = rhs.m_c;
   
   *AtA  = *rhs.AtA;
   *Atb  = *rhs.Atb;
}

ossimLeastSquaresPlane::~ossimLeastSquaresPlane()
{
   if(AtA)
   {
      delete AtA;
      AtA = NULL;
   }
   if(Atb)
   {
      delete Atb;
      Atb = NULL;
   }
}
ossimLeastSquaresPlane & ossimLeastSquaresPlane::operator = (const ossimLeastSquaresPlane &rhs)
{
   m_a    = rhs.m_a;
   m_b    = rhs.m_b;
   m_c    = rhs.m_c;
   
   *AtA    = *rhs.AtA;
   *Atb    = *rhs.Atb;
   
   return *this;
}

void ossimLeastSquaresPlane::clear()
{
   *AtA    = 0.0;
   *Atb    = 0.0;  
   m_a    = 0.0;
   m_b    = 0.0;
   m_c    = 0.0;
}

void ossimLeastSquaresPlane::addSample(double xx, double yy, double zmea)
{
   // form normal system layer
   NEWMAT::Matrix AtA_layer(3,1);
   AtA_layer(1,1) = xx;
   AtA_layer(2,1) = yy;
   AtA_layer(3,1) = 1.0;
   
   // accumulate layer into normal system
   *AtA += AtA_layer * AtA_layer.t();
   *Atb += AtA_layer * zmea;

   ++m_numSamples;
}

bool ossimLeastSquaresPlane::solveLS()
{
   if (m_numSamples < 3)
      return false;

   NEWMAT::Matrix Soln(3,1);
   Soln = AtA->i() * (*Atb);
   m_a = Soln(1,1);
   m_b = Soln(2,1);
   m_c = Soln(3,1);
   
   return true;
}

bool ossimLeastSquaresPlane::getLSParms(double& pa, double& pb, double& pc) const
{
   pa = m_a;
   pb = m_b;
   pc = m_c;
   
   return true;
}

void ossimLeastSquaresPlane::setLSParams(double pa, double pb, double pc)
{
   m_a = pa;
   m_b = pb;
   m_c = pc;
}

                 

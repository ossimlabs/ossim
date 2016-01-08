#include <ossim/projection/ossimTangentialRadialLensDistortion.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimKeywordlist.h>

RTTI_DEF1(ossimTangentialRadialLensDistortion, "ossimTangentialRadialLensDistortion",
          ossimLensDistortion);
ossimTangentialRadialLensDistortion::ossimTangentialRadialLensDistortion()
   :ossimLensDistortion()
{
  m_k.resize(3);
  m_p.resize(2);
  m_k[0] = m_k[1] = m_k[2] = 0.0;
  m_p[0] = m_p[1];
}
ossimTangentialRadialLensDistortion::ossimTangentialRadialLensDistortion(const ossimDpt& calibratedCenter,
                               double k1, double k2, double k3,
                               double p1, double p2)
    :ossimLensDistortion(calibratedCenter)
 {
 
    m_k.resize(3);
    m_p.resize(2);

    m_k[0] = k1;
    m_k[1] = k2;
    m_k[2] = k3;

    m_p[0] = p1;
    m_p[1] = p2;


}
ossimTangentialRadialLensDistortion::ossimTangentialRadialLensDistortion(const ossimDpt& calibratedCenter,
                                     std::vector<double> k,
                                     std::vector<double> p)
  :ossimLensDistortion(calibratedCenter),
  m_k(k),
  m_p(p)
{
}

void ossimTangentialRadialLensDistortion::undistort(const ossimDpt& input, 
         ossimDpt& output)const
{
   // make relative to calibrated center
   //ossimDpt deltaPosition(input.x - theCenter.x,
   //                       input.y - theCenter.y);

   double r = sqrt(input.x*input.x + 
                   input.y*input.y);
   double r2 = r*r;
   double rMultiplier = r2;
   ossim_uint32 idx = 0;
   // comnpute the radial portion of the equation of the form
   // 1 + k0*r^2 + k1*r^4 + .......
   double tempRadial = 1.0;
   for(idx = 0; idx < m_k.size();++idx)
   {
      tempRadial += m_k[idx]*rMultiplier;
      rMultiplier*=r2;
   }
   // now apply the radial to the input point and then 
   // shift and apply the tangential coefficients along x and y.
   output.x = (input.x*tempRadial) + 
              (2.0*m_p[0]*input.x*input.y) + 
              (m_p[1]*(r2 + 2*input.x*input.x)); 

   output.y = (input.y*tempRadial) + 
              (2.0*m_p[1]*input.x*input.y) + 
              (m_p[0]*(r2 + 2*input.y*input.y));

//   output.x += theCenter.x;
//   output.y += theCenter.y;
}
bool ossimTangentialRadialLensDistortion::saveState(ossimKeywordlist& kwl,
                      const char* prefix )const
{
   bool result = ossimLensDistortion::saveState(kwl, prefix);
   ossimString kValues;
   ossimString pValues;
   ossim::toSimpleStringList(kValues, m_k);
   ossim::toSimpleStringList(pValues, m_p);

   kwl.add(prefix, "k", kValues);
   kwl.add(prefix, "p", pValues);

   return result;
}

bool ossimTangentialRadialLensDistortion::loadState(const ossimKeywordlist& kwl,
                      const char* prefix)
{
   bool result = ossimLensDistortion::loadState(kwl, prefix);
   ossimString k = kwl.find(prefix, "k");
   ossimString p = kwl.find(prefix, "p");

   m_k.clear();
   m_p.clear();
   ossim::toSimpleVector(m_k, k);
   ossim::toSimpleVector(m_p, p);

   return result;
}

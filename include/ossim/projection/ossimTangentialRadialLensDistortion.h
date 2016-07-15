#ifndef ossimTangentialRadialLensDistortion_HEADER
#define ossimTangentialRadialLensDistortion_HEADER 1
#include <vector>
#include <ossim/base/ossimDpt.h>
#include <ossim/projection/ossimLensDistortion.h>

/***
This is taken from the site:

http://www.mathworks.com/products/symbolic/code-examples.html?file=/products/demos/symbolictlbx/Pixel_location/Camera_Lens_Undistortion.html

Developing an Algorithm to Undistort Pixel Locations of an Image
This example uses Symbolic Math Toolbox to develop an algorithm that undistorts pixel locations of an image.

Background

When a camera captures an image, it does not precisely capture 
the real points, but rather a slightly distorted version of the real points that can be denoted (x2, y2). The distorted pixel locations can be described using the following equations:

x2 = x1(1 + k1*r^2 + k2*r^4 + k3*r^6) +2*p1*x1*y1 + p2*(r^2 + 2*x1^2)
y2 = y1(1 + k1*r^2 + k2*r^4 + k3*r^6) +2*p2*x1*y1 + p1*(r^2 + 2*y1^2)
 

where:
x1, y2     = undistorted pixel locations
k1, k2, k3 = radial distortion coefficients of the lens
p1, p2     = tangential distortion coefficients of the lens
r = sqrt(x1^2 + y1^2)


***/

class ossimTangentialRadialLensDistortion : public ossimLensDistortion
{
public:
   ossimTangentialRadialLensDistortion();

   ossimTangentialRadialLensDistortion(const ossimDpt& calibratedCenter,
                                 double k1 = 0.0, double k2 = 0.0, double k3 = 0.0,
                                 double p1 = 0.0, double p2 = 0.0);
   ossimTangentialRadialLensDistortion(const ossimDpt& calibratedCenter,
                                 std::vector<double> k,
                                 std::vector<double> p);

//  ossimDpt adjustPoint(const ossimDpt &position)const;
  virtual std::ostream& print(std::ostream& out) const
  {return out;}

  virtual void undistort(const ossimDpt& input, ossimDpt& output)const;
  
  
  virtual bool saveState(ossimKeywordlist& kwl,
                         const char* prefix = 0)const;
  
  virtual bool loadState(const ossimKeywordlist& kwl,
                         const char* prefix = 0);
  
protected:

  std::vector<ossim_float64> m_k;
  std::vector<ossim_float64> m_p;
  
TYPE_DATA  
};

#endif

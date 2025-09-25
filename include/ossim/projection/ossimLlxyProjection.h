//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Contains class declaration for ossimLlxy.  This is a simple "latitude /
// longitude to x / y" projection.  Note that "origin" and "tie_point" are
// synonymous in this projection.
// 
//*******************************************************************
//  $Id: ossimLlxyProjection.h 19651 2011-05-25 18:49:22Z gpotts $

#ifndef ossimLlxyProjection_HEADER
#define ossimLlxyProjection_HEADER

#include <ossim/projection/ossimMapProjection.h>

class OSSIMDLLEXPORT ossimLlxyProjection : public ossimMapProjection
{
public:
   ossimLlxyProjection();
   ossimLlxyProjection(const ossimLlxyProjection& rhs);
   ossimLlxyProjection(const ossimEllipsoid& ellipsoid,
                       const ossimGpt& origin);
   ossimLlxyProjection(const ossimGpt& origin,
                       double latSpacing,  // decimal degrees
                       double lonSpacing);
   
   ~ossimLlxyProjection() override;

   ossimObject *dup()const override;

   bool isGeographic()const override;

   ossimDpt forward(const ossimGpt &worldPoint) const override;
   ossimGpt inverse(const ossimDpt &projectedPoint) const override;
   
   /*!
    *  METHOD:  getLatSpacing() 
    *  Returns the latitude spacing as a double representing decimal degrees.
    */
   virtual double getLatSpacing() const;

   /*!
    *  METHOD:  getLonSpacing() 
    *  Returns the longitude spacing as a double representing decimal degrees.
    */
   virtual double getLonSpacing() const;

   /*!
    *  METHOD:  setLatSpacing(double spacing)
    *  Set the latitude spacing as a double representing decimal degrees.
    */
   virtual void   setLatSpacing(double spacing);

    /*!
    *  METHOD:  setLonSpacing(double spacing)
    *  Set the latitude spacing as a double representing decimal degrees.
    */
   virtual void   setLonSpacing(double spacing);

   /*!
    *  METHOD: worldToLineSample()
    * Performs the forward projection from ground point to line, sample.
    */
   void worldToLineSample(const ossimGpt& worldPoint,
                          ossimDpt&       lineSampPt) const override;

   /*!
    * Performs the inverse projection from line, sample to ground (world):
    */
   void lineSampleHeightToWorld(const ossimDpt& lineSampPt,
                                const double&  hgtEllipsoid,
                                ossimGpt&       worldPt) const override;
   
   /*!
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   bool saveState(ossimKeywordlist& kwl,
                  const char* prefix=0)const override;

   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   bool loadState(const ossimKeywordlist& kwl,
                  const char* prefix=0) override;

   std::ostream& print(std::ostream& out) const override;

   bool operator==(const ossimProjection& projection) const override;

  //   virtual ossimDpt getMetersPerPixel() const;
   void setMetersPerPixel(const ossimDpt& pt) override;
   /*!
    * This will go from the ground point and give
    * you an approximate lat and lon per pixel. the Delta Lat
    * and delta lon will be in degrees.
    */
   virtual void computeDegreesPerPixel(const ossimGpt& ground,
                                       const ossimDpt& metersPerPixel,
                                       double &deltaLat,
                                       double &deltaLon);

   
   /*!
    * This will go from the ground point and give
    * you an approximate meters per pixel. the Delta Lat
    * and delta lon will be in degrees.
    */
   virtual void computeMetersPerPixel(const ossimGpt& center,
                                      double deltaDegreesPerPixelLat,
                                      double deltaDegreesPerPixelLon,
                                      ossimDpt &metersPerPixel);
   
private:
   void updateTransform() override;

TYPE_DATA
};

#endif

//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc. 
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Kathy Minear
//
// Description:
// Takes in DNs for any number of bands
// Converts DNs to Radiance at the satellite values Lsat
// Converts Lsat to Surface Reflectance values
//
//*************************************************************************
// $Id: ossimAtCorrRemapper.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef ossimAtCorrRemapper_HEADER
#define ossimAtCorrRemapper_HEADER

#include <ossim/imaging/ossimImageSourceFilter.h>

class ossimAtCorrRemapper : public ossimImageSourceFilter
{
public:

   ossimAtCorrRemapper(ossimObject* owner = NULL,
                       ossimImageSource* inputSource =  NULL,
                       const ossimString& sensorType = "");


   virtual ossimString getShortName() const;

   virtual void initialize();

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tile_rect,
                                               ossim_uint32 resLevel=0);
   
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   
   virtual std::ostream& print(std::ostream& os) const;
   friend std::ostream& operator << (std::ostream& os,  const ossimAtCorrRemapper& hr);

   std::vector<double> getNormMinPixelValues() const;
   std::vector<double> getNormMaxPixelValues() const;

   void getNormMinPixelValues(std::vector<double>& v) const;
   void getNormMaxPixelValues(std::vector<double>& v) const;

   const ossimString& getSensorType() const;

   void setSensorType(const ossimString& sensorType);

protected:
   virtual ~ossimAtCorrRemapper();

   /*!
    * Method to set unset the enable flag.
    */
   void verifyEnabled();
   virtual void interpolate(const ossimDpt& pt,
                            int band,
                            double& a,
                            double& b,
                            double& c)const;
                            
   ossimRefPtr<ossimImageData> theTile;
   double*            theSurfaceReflectance;
   bool               theUseInterpolationFlag;
   std::vector<double>     theMinPixelValue;
   std::vector<double>     theMaxPixelValue;
   std::vector<double>     theXaArray;
   std::vector<double>     theXbArray;
   std::vector<double>     theXcArray;
   std::vector<double>     theBiasArray;
   std::vector<double>     theGainArray;
   std::vector<double>     theCalCoefArray;
   std::vector<double>     theBandWidthArray;
   ossimString        theSensorType;

TYPE_DATA
};

#endif /* #ifndef ossimAtCorrRemapper_HEADER */

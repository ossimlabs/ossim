//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: ossimHistoMatchRemapper.h 23353 2015-06-01 19:19:10Z dburken $
#ifndef ossimHistoMatchRemapper_HEADER
#define ossimHistoMatchRemapper_HEADER 1
#include <vector>
#include <ossim/base/ossimConstants.h>
#include <ossim/imaging/ossimImageSourceFilter.h>

class OSSIM_DLL ossimHistoMatchRemapper: public ossimImageSourceFilter
{
public:
   ossimHistoMatchRemapper();
   ossimHistoMatchRemapper(ossimImageSource* inputSource,
                           const std::vector<double>& targetMeanPerBand  = std::vector<double>(),
                           const std::vector<double>& targetSigmaPerBand = std::vector<double>(),
                           const std::vector<double>& inputMeanPerBand   = std::vector<double>(),
                           const std::vector<double>& inputSigmaPerBand  = std::vector<double>()
                           );

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tileRect,
                                               ossim_uint32 resLevel=0);
   virtual void initialize();
   
   const std::vector<double>& getInputMeanValues()const;
   const std::vector<double>& getInputSigmaValues()const;
   const std::vector<double>& getTargetMeanValues()const;
   const std::vector<double>& getTargetSigmaValues()const;

   void setInputMeanValues(const std::vector<double>& newValues);
   void setInputSigmaValues(const std::vector<double>& newValues);
   void setTargetMeanValues(const std::vector<double>& newValues);
   void setTargetSigmaValues(const std::vector<double>& newValues);
   

   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=NULL);
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=NULL);

protected:
   virtual ~ossimHistoMatchRemapper();
   
   ossimRefPtr<ossimImageData> theBlankTile;
   
   std::vector<double> theTargetMeanPerBand;
   std::vector<double> theTargetSigmaPerBand;
   std::vector<double> theInputMeanPerBand;
   std::vector<double> theInputSigmaPerBand;

   /**
    * transLean
    * @param vIn input value to be transformed
    * @param vBias bias value to be removed
    * @param vTarget value that will replace bias
    * @param vMin minimum valid value of vIn (inclusive)
    * @param vMax maximum valid value of vIn (inclusive)
    * @return vOut
    */
   double transLean(double vIn,
                    double vBias,
                    double vTarget,
                    double vMin,
                    double vMax);

   /**
    * transLeanStretch
    * @param vin Input value to be transformed.
    * @param vBias Bias value to be removed.
    * @param vBiasStretch dispersion (+/-) about vBias
    * @param vTarget value that will replace bias
    * @param vTargetStretch dispersion (+/-) about vTarget
    * @param vMin minimum valid value of vIn (inclusive)
    * @param vMax maximum valid value of vIn (inclusive)
    * @return vOut
    */
   double transLeanStretch(double vIn,
                           double vBias,
                           double vBiasStretch,
                           double vTarget,
                           double vTargetStretch,
                           double vMin,
                           double vMax);
   
};

#endif /* #ifndef ossimHistoMatchRemapper_HEADER */
